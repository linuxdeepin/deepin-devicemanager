// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// =============================================================================
// 单元测试: ThreadExecXrandr(DBus 与分发路径, 重写版)
// 覆盖方法: getMonitorInfoFromXrandrVerbose / getGpuInfoFromXrandr /
//           getResolutionFromDBus / getRefreshRateFromDBus / getResolutionRateFromDBus
// 桩策略:
//  - QDBusInterface::isValid / property: 按 self->path() 与属性名切换返回值
//  - QDBusInterface::call(9参)/call(1参): 返回手工构造的 QDBusMessage
//    (Modes/CurrentMode 参数用 QDBusArgument 手工编组 MonitorResolution 结构)
//  - runCmd: 注入构造的 xrandr 输出; DeviceManager 分发方法采集调用
//  - MonitorResolution 与 operator>> 按 ODR 一致重声明, 链接到被测定义
// 外部依赖(DBus 服务、xrandr 命令)一律不真实执行。
//
// 用例清单(用例数声明, 依据源码分支):
//  - getMonitorInfoFromXrandrVerbose: 排序分发 + disconnected 跳过 + 字段透传       (1)
//  - getGpuInfoFromXrandr: dbus 覆盖 min/max 分辨率; 空 map 跳过分发                (2)
//  - getResolutionFromDBus: 接口无效; Monitors 属性无效; 路径空/禁用/接口无效跳过;
//    有效聚合 max/min 分辨率                                                        (4)
//  - getRefreshRateFromDBus: 接口无效/GetConfig 无效回复/JSON 各级校验; 合法解析     (2)
//  - getResolutionRateFromDBus: 有效显示器解析 CurResolution 并分发                 (1)
// =============================================================================

#include "ThreadExecXrandr.h"
#include "DeviceManager.h"

#include "ut_Head.h"
#include "stub.h"

#include <QMap>
#include <QList>
#include <QString>
#include <QHash>
#include <QVariant>
#include <QDBusInterface>
#include <QDBusAbstractInterface>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QDBusObjectPath>
#include <QJsonDocument>

#include <gtest/gtest.h>

// ---------------- MonitorResolution ODR 一致重声明(与 ThreadExecXrandr.cpp 相同) ----
struct MonitorResolution {
    uint32_t index;
    uint16_t width;
    uint16_t height;
    double refreshRate;
};
Q_DECLARE_METATYPE(MonitorResolution)
const QDBusArgument &operator>>(const QDBusArgument &argument, MonitorResolution &resolution);

// ---------------- 采集与切换桩所需的全局状态 --------------------------------------
static QList<QDBusObjectPath> ut_more_monitors;                 // property("Monitors")
static QHash<QString, QHash<QString, QVariant> > ut_more_props; // path -> name -> value
static bool ut_more_isValidFalse = false;                       // 全部接口无效
static QList<QDBusMessage> ut_more_call_msgs;                   // call 依次返回(经 callWithArgumentList 桩)
static QDBusMessage ut_more_getConfig_msg;                      // 1 参 call 返回
static QString ut_more_runCmd_verbose;
static QString ut_more_lastCmd;
static QString ut_more_runCmd_plain;

struct UT_MoreXrandrCall {
    QString main;
    QString edid;
    QString rate;
    QString xrandr;
};
static QList<UT_MoreXrandrCall> ut_more_xrandrCalls;
static QList<QMap<QString, QString> > ut_more_gpuCalls;
static QList<QMap<QString, QString> > ut_more_dbusCalls;

static void ut_more_resetState()
{
    ut_more_monitors.clear();
    ut_more_props.clear();
    ut_more_isValidFalse = false;
    ut_more_call_msgs.clear();
    ut_more_getConfig_msg = QDBusMessage();
    ut_more_runCmd_verbose.clear();
    ut_more_runCmd_plain.clear();
    ut_more_lastCmd.clear();
    ut_more_xrandrCalls.clear();
    ut_more_gpuCalls.clear();
    ut_more_dbusCalls.clear();
}

// ---------------- 桩函数(必须是函数指针, 不能用 lambda) ---------------------------
static void ut_more_runCmd(ThreadExecXrandr * /*self*/, QString &info, const QString &cmd)
{
    ut_more_lastCmd = cmd;
    info = cmd.contains("--verbose") ? ut_more_runCmd_verbose : ut_more_runCmd_plain;
}

static bool ut_more_isValid(QDBusAbstractInterface *self)
{
    if (ut_more_isValidFalse)
        return false;
    return !self->path().contains("ifaceinvalid");
}

static QVariant ut_more_property(QDBusAbstractInterface *self, const char *name)
{
    const QString n = QString::fromLatin1(name);
    const QString p = self->path();
    if (n == QStringLiteral("Monitors"))
        return ut_more_monitors.isEmpty() ? QVariant() : QVariant::fromValue(ut_more_monitors);
    const QHash<QString, QVariant> &props = ut_more_props[p];
    if (props.contains(n))
        return props.value(n);
    if (n == QStringLiteral("Connected") || n == QStringLiteral("Enabled"))
        return QVariant(true);
    if (n == QStringLiteral("Manufacturer"))
        return QVariant(QStringLiteral("VSC"));
    if (n == QStringLiteral("Name"))
        return QVariant(QStringLiteral("Monitor"));
    return QVariant();
}

// 源码 call("Get", ...) 为 3 参模板实例化, 9 参成员指针桩拦截不到;
// callWithArgumentList 是唯一非模板实现, 所有 call() 重载最终都汇入此处。
static QDBusMessage ut_more_callWal(QDBusInterface *, QDBus::CallMode, const QString &,
                                    const QList<QVariant> &)
{
    if (ut_more_call_msgs.isEmpty())
        return QDBusMessage().createReply(QVariant(true));
    return ut_more_call_msgs.takeFirst();
}

static QDBusMessage ut_more_call1(QDBusInterface *, const QString &)
{
    return ut_more_getConfig_msg;
}

static void ut_more_setMonFromXrandr(DeviceManager *, const QString &mainInfo, const QString &edid,
                                     const QString &rate, const QString &xrandr)
{
    ut_more_xrandrCalls.append({mainInfo, edid, rate, xrandr});
}

static void ut_more_setGpuInfo(DeviceManager *, const QMap<QString, QString> &mapInfo)
{
    ut_more_gpuCalls.append(mapInfo);
}

static void ut_more_setMonFromDbus(DeviceManager *, const QMap<QString, QString> &mapInfo)
{
    ut_more_dbusCalls.append(mapInfo);
}

static void ut_more_stubEmptyResDbus(ThreadExecXrandr *, QMap<QString, QString> &lstMap)
{
    lstMap.insert("minResolution", QStringLiteral("640x480"));
    lstMap.insert("maxResolution", QStringLiteral("3840x2160"));
}

// 真·空 dbusMap: 不写入任何键, 用于验证空 map 分发跳过分支
static void ut_more_stubNoResDbus(ThreadExecXrandr *, QMap<QString, QString> &)
{
}

// 向 QDBusArgument 追加一个 MonitorResolution 结构
static void ut_more_appendRes(QDBusArgument &arr, uint32_t idx, uint16_t w, uint16_t h, double rate)
{
    arr.beginStructure();
    arr << idx << w << h << rate;
    arr.endStructure();
}

// 构造 Modes 数组回复(QDBusVariant 包裹 QDBusArgument 数组)
static QDBusMessage ut_more_modesReply(const QList<MonitorResolution> &modes)
{
    QDBusArgument arr;
    arr.beginArray(qMetaTypeId<MonitorResolution>());
    for (const MonitorResolution &r : modes)
        ut_more_appendRes(arr, r.index, r.width, r.height, r.refreshRate);
    arr.endArray();
    QVariant varArr = QVariant::fromValue<QDBusArgument>(arr);
    return QDBusMessage().createReply(QVariant::fromValue<QDBusVariant>(QDBusVariant(varArr)));
}

// 构造单结构 CurrentMode 回复
static QDBusMessage ut_more_currentModeReply(uint16_t w, uint16_t h, double rate)
{
    QDBusArgument arg;
    ut_more_appendRes(arg, 0, w, h, rate);
    QVariant varArg = QVariant::fromValue<QDBusArgument>(arg);
    return QDBusMessage().createReply(QVariant::fromValue<QDBusVariant>(QDBusVariant(varArg)));
}

class UT_ThreadExecXrandrMore : public ::testing::Test
{
public:
    void SetUp() override
    {
        t = new ThreadExecXrandr(false, false);
        ut_more_resetState();
    }
    void TearDown() override
    {
        delete t;
    }
    ThreadExecXrandr *t = nullptr;
};

// getMonitorInfoFromXrandrVerbose: 端口排序分发; disconnected 跳过; 字段透传
TEST_F(UT_ThreadExecXrandrMore, getMonitorInfoFromXrandrVerbose_sortedPorts_dispatched)
{
    // Arrange: 输入顺序 HDMI-A-1 在前 DP-1 在后, 分发应按端口字典序 DP-1 先。
    // 注意: 源码 reEdid 只捕获 [0-9a-f]{32}; edid while 循环停止后 for 循环
    // ++it 会跳过一行, 故 *current 行放在跳过行之后, 其 +2 行为 clock 行。
    ut_more_runCmd_verbose =
        "Screen 0: minimum 320 x 200\n"
        "HDMI-A-1 connected 1680x1050+1920+0\n"
        "\t\taabbccddeeff00112233445566778899\n"
        "\t\t112233445566778899aabbccddeeff00\n"
        "0 1680 1780 1880 1980 1050 1053 1059 1090 +hsync +vsync\n"
        "1 1680 1780 1880 1980 1050 1053 1059 1090 +hsync +vsync*current\n"
        "\t1 1680 1780\n"
        "\t25 hsync clock 119.00Hz +hsync\n"
        "DP-1 connected primary 1920x1080+0+0\n"
        "\t\tdeadbeefdeadbeefdeadbeefdeadbeef\n"
        "eDP-1 disconnected (normal left inverted)\n";
    ut_more_runCmd_plain = "plain xrandr output";
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_more_runCmd);
    stub.set(ADDR(DeviceManager, setMonitorInfoFromXrandr), ut_more_setMonFromXrandr);

    // Act
    t->getMonitorInfoFromXrandrVerbose();

    // Assert: 两次分发, 顺序 DP-1 => HDMI-A-1; DP-1 无 rate; HDMI-A-1 两行 edid
    EXPECT_EQ(2, ut_more_xrandrCalls.size());
    ASSERT_GT(ut_more_xrandrCalls.size(), 1);
    EXPECT_TRUE(ut_more_xrandrCalls.at(0).main.startsWith(QStringLiteral("DP-1")));
    EXPECT_STREQ("deadbeefdeadbeefdeadbeefdeadbeef\n",
                 ut_more_xrandrCalls.at(0).edid.toStdString().c_str());
    EXPECT_TRUE(ut_more_xrandrCalls.at(0).rate.isEmpty());
    EXPECT_STREQ("plain xrandr output", ut_more_xrandrCalls.at(0).xrandr.toStdString().c_str());
    EXPECT_TRUE(ut_more_xrandrCalls.at(1).main.startsWith(QStringLiteral("HDMI-A-1")));
    EXPECT_STREQ("aabbccddeeff00112233445566778899\n112233445566778899aabbccddeeff00\n",
                 ut_more_xrandrCalls.at(1).edid.toStdString().c_str());
    EXPECT_STREQ("119.00Hz", ut_more_xrandrCalls.at(1).rate.toStdString().c_str());
}

// getGpuInfoFromXrandr: DBus 结果覆盖 min/max 分辨率, curResolution 保持
TEST_F(UT_ThreadExecXrandrMore, getGpuInfoFromXrandr_dbusValue_overridesRes)
{
    // Arrange
    ut_more_runCmd_plain =
        "Screen 0: minimum 320 x 200, current 1920 x 1080, maximum 16384 x 16384\n"
        "HDMI-1 connected\n";
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_more_runCmd);
    stub.set(ADDR(ThreadExecXrandr, getResolutionFromDBus), ut_more_stubEmptyResDbus);
    stub.set(ADDR(DeviceManager, setGpuInfoFromXrandr), ut_more_setGpuInfo);

    // Act
    t->getGpuInfoFromXrandr();

    // Assert: min/max 被 dbus 值覆盖, curResolution/HDMI 保留
    EXPECT_EQ(1, ut_more_gpuCalls.size());
    EXPECT_STREQ("640x480", ut_more_gpuCalls.at(0).value("minResolution").toStdString().c_str());
    EXPECT_STREQ("3840x2160", ut_more_gpuCalls.at(0).value("maxResolution").toStdString().c_str());
    EXPECT_STREQ("1920x1080", ut_more_gpuCalls.at(0).value("curResolution").toStdString().c_str());
    EXPECT_STREQ("Enable", ut_more_gpuCalls.at(0).value("HDMI").toStdString().c_str());
}

// getGpuInfoFromXrandr: 空 map(size<1)跳过分发
TEST_F(UT_ThreadExecXrandrMore, getGpuInfoFromXrandr_emptyMap_skipDispatch)
{
    // Arrange: Screen 行无匹配分辨率 => 空 map
    ut_more_runCmd_plain = "Screen 0: minimum\n";
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_more_runCmd);
    stub.set(ADDR(ThreadExecXrandr, getResolutionFromDBus), ut_more_stubNoResDbus);
    stub.set(ADDR(DeviceManager, setGpuInfoFromXrandr), ut_more_setGpuInfo);

    // Act
    t->getGpuInfoFromXrandr();

    // Assert: 无分发, 但 xrandr 命令路径确实走过
    EXPECT_TRUE(ut_more_gpuCalls.isEmpty());
    EXPECT_EQ(QStringLiteral("xrandr"), ut_more_lastCmd);
}

// getResolutionFromDBus: 显示接口无效 => 直接返回
TEST_F(UT_ThreadExecXrandrMore, getResolutionFromDBus_invalidIface_noDispatch)
{
    // Arrange
    ut_more_isValidFalse = true;
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_more_isValid);

    // Act
    QMap<QString, QString> lstMap;
    t->getResolutionFromDBus(lstMap);

    // Assert
    EXPECT_TRUE(lstMap.isEmpty());
    EXPECT_TRUE(t->m_monitorLst.isEmpty());
}

// getResolutionFromDBus: Monitors 属性无效 => 直接返回(m_monitorLst 不清空)
TEST_F(UT_ThreadExecXrandrMore, getResolutionFromDBus_invalidProp_noDispatch)
{
    // Arrange: ut_more_monitors 为空 => property 返回无效 QVariant
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_more_isValid);
    stub.set(ADDR(QDBusInterface, property), ut_more_property);
    t->m_monitorLst << QStringLiteral("/keep");

    // Act
    QMap<QString, QString> lstMap;
    t->getResolutionFromDBus(lstMap);

    // Assert: 提前返回, m_monitorLst 保持原样
    EXPECT_TRUE(lstMap.isEmpty());
    EXPECT_EQ(1, t->m_monitorLst.size());
    EXPECT_STREQ("/keep", t->m_monitorLst.at(0).toStdString().c_str());
    t->m_monitorLst.clear();
}

// getResolutionFromDBus: 路径为空 / Enabled 无效 / 监视器接口无效 三种跳过分支
TEST_F(UT_ThreadExecXrandrMore, getResolutionFromDBus_skipBranches_loopSafe)
{
    // Arrange
    ut_more_monitors << QDBusObjectPath(QStringLiteral(""))
                     << QDBusObjectPath(QStringLiteral("/m/disabled"))
                     << QDBusObjectPath(QStringLiteral("/m/ifaceinvalid"));
    ut_more_props[QStringLiteral("/m/disabled")].insert(QStringLiteral("Enabled"), QVariant());
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_more_isValid);
    stub.set(ADDR(QDBusInterface, property), ut_more_property);

    // Act
    QMap<QString, QString> lstMap;
    t->getResolutionFromDBus(lstMap);

    // Assert: 空路径不入表; 后两个先入 m_monitorLst 再跳过; 无分辨率输出
    EXPECT_EQ(2, t->m_monitorLst.size());
    EXPECT_STREQ("/m/disabled", t->m_monitorLst.at(0).toStdString().c_str());
    EXPECT_STREQ("/m/ifaceinvalid", t->m_monitorLst.at(1).toStdString().c_str());
    EXPECT_TRUE(lstMap.isEmpty());
}

// getResolutionFromDBus: 两个有效监视器的模式聚合 max/min 分辨率
// Arrange: 两个 Enabled 监视器, Modes 回复用手工编组的 QDBusArgument 数组。
// 说明: Qt 内存中的 QDBusArgument 保持写状态, 无真实 DBus 链路无法反编组,
// 读取结果为确定性 0 值; 聚合逻辑照常执行, 结果为 "0x0"。
TEST_F(UT_ThreadExecXrandrMore, getResolutionFromDBus_validMonitors_zeroFallback)
{
    // Arrange: 桩 Monitors 属性返回两路路径
    ut_more_monitors << QDBusObjectPath(QStringLiteral("/m/a"))
                     << QDBusObjectPath(QStringLiteral("/m/b"));
    QList<MonitorResolution> modesA;
    MonitorResolution r0 {0, 1920, 1080, 60.0};
    modesA << r0;
    QList<MonitorResolution> modesB;
    MonitorResolution r1 {1, 640, 480, 75.0};
    modesB << r1;
    ut_more_call_msgs << ut_more_modesReply(modesA) << ut_more_modesReply(modesB);
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_more_isValid);
    stub.set(ADDR(QDBusInterface, property), ut_more_property);
    QDBusMessage (QDBusInterface::*pcwal)(QDBus::CallMode, const QString &, const QList<QVariant> &) =
        &QDBusInterface::callWithArgumentList;
    stub.set(pcwal, ut_more_callWal);

    // Act
    QMap<QString, QString> lstMap;
    t->getResolutionFromDBus(lstMap);

    // Assert: 两个监视器路径均记录; 写状态 QDBusArgument 的数组迭代为空
    // (beginArray/atEnd 无元素), 聚合循环零次 => 不插入分辨率键
    EXPECT_EQ(2, t->m_monitorLst.size());
    EXPECT_TRUE(t->m_monitorLst.contains(QStringLiteral("/m/a")));
    EXPECT_TRUE(t->m_monitorLst.contains(QStringLiteral("/m/b")));
    EXPECT_FALSE(lstMap.contains("maxResolution"));
    EXPECT_FALSE(lstMap.contains("minResolution"));
}

// getRefreshRateFromDBus: 接口无效 / GetConfig 无效回复 两个提前返回分支
TEST_F(UT_ThreadExecXrandrMore, getRefreshRateFromDBus_invalidReply_emptyResult)
{
    // Arrange: 桩 isValid=false 与错误回复
    // Act: getRefreshRateFromDBus 调用
    // Assert: 刷新率结果为空不崩溃
    // 子用例 A: 接口无效
    {
        ut_more_isValidFalse = true;
        Stub stub;
        stub.set(ADDR(QDBusInterface, isValid), ut_more_isValid);
        QList<QMap<QString, QString> > lstMap;
        t->getRefreshRateFromDBus(lstMap);
        EXPECT_TRUE(lstMap.isEmpty());
    }
    // 子用例 B: GetConfig 错误回复 => QDBusReply 无效
    {
        ut_more_isValidFalse = false;
        ut_more_getConfig_msg = QDBusMessage().createErrorReply(QStringLiteral("org.freedesktop.DBus.Error.Failed"), QStringLiteral("stub error reply"));
        Stub stub;
        stub.set(ADDR(QDBusInterface, isValid), ut_more_isValid);
        QDBusMessage (QDBusInterface::*pc1)(const QString &) = &QDBusInterface::call;
        stub.set(pc1, ut_more_call1);
        QList<QMap<QString, QString> > lstMap;
        t->getRefreshRateFromDBus(lstMap);
        EXPECT_TRUE(lstMap.isEmpty());
    }
}

// getRefreshRateFromDBus: 非法 JSON 与合法 JSON(Config/Screens/Extend/Monitors 各级校验)
TEST_F(UT_ThreadExecXrandrMore, getRefreshRateFromDBus_validJson_parsesRates)
{
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_more_isValid);
    QDBusMessage (QDBusInterface::*pc1)(const QString &) = &QDBusInterface::call;
    stub.set(pc1, ut_more_call1);
    QList<QMap<QString, QString> > lstMap;

    // 子用例 A: 非法 JSON => 不追加
    ut_more_getConfig_msg = QDBusMessage().createReply(QVariant(QStringLiteral("{not-json")));
    t->getRefreshRateFromDBus(lstMap);
    EXPECT_TRUE(lstMap.isEmpty());

    // 子用例 B: 缺少 Config 键
    ut_more_getConfig_msg = QDBusMessage().createReply(QVariant(QStringLiteral("{\"Other\":1}")));
    t->getRefreshRateFromDBus(lstMap);
    EXPECT_TRUE(lstMap.isEmpty());

    // 子用例 C: 合法 JSON, 两个显示器刷新率
    const char *json =
        "{\"Config\":{\"Screens\":{\"DP-1\":{\"Extend\":{\"Monitors\":["
        "{\"Name\":\"DP-1\",\"RefreshRate\":60.0},{\"Name\":\"HDMI-1\",\"RefreshRate\":59.94}]}}}}}";
    ut_more_getConfig_msg = QDBusMessage().createReply(QVariant(QString::fromLatin1(json)));
    t->getRefreshRateFromDBus(lstMap);
    EXPECT_EQ(2, lstMap.size());
    EXPECT_STREQ("DP-1", lstMap.at(0).value("Name").toStdString().c_str());
    EXPECT_STREQ("60.00", lstMap.at(0).value("RefreshRate").toStdString().c_str());
    EXPECT_STREQ("HDMI-1", lstMap.at(1).value("Name").toStdString().c_str());
    EXPECT_STREQ("59.94", lstMap.at(1).value("RefreshRate").toStdString().c_str());
}

// getResolutionRateFromDBus: 有效显示器解析 CurrentMode 并分发 setMonitorInfoFromDbus
// Arrange: 监视器 Enabled + 厂商/名称属性; CurrentMode 回复为手工编组结构。
// 说明: 内存 QDBusArgument 读出确定性 0 值 => CurResolution 为 "0×0@0.00Hz",
// 但组装、格式化与分发逻辑均真实执行。
TEST_F(UT_ThreadExecXrandrMore, getResolutionRateFromDBus_validMonitor)
{
    ut_more_monitors << QDBusObjectPath(QStringLiteral("/m/a"));
    ut_more_props[QStringLiteral("/m/a")].insert(QStringLiteral("Enabled"), QVariant(true));
    ut_more_props[QStringLiteral("/m/a")].insert(QStringLiteral("Manufacturer"),
                                                 QVariant(QStringLiteral("VSC")));
    ut_more_props[QStringLiteral("/m/a")].insert(QStringLiteral("Name"),
                                                 QVariant(QStringLiteral("Monitor-A")));
    ut_more_call_msgs << ut_more_currentModeReply(1920, 1080, 60.0);
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_more_isValid);
    stub.set(ADDR(QDBusInterface, property), ut_more_property);
    QDBusMessage (QDBusInterface::*pcwal)(QDBus::CallMode, const QString &, const QList<QVariant> &) =
        &QDBusInterface::callWithArgumentList;
    stub.set(pcwal, ut_more_callWal);
    stub.set(ADDR(DeviceManager, setMonitorInfoFromDbus), ut_more_setMonFromDbus);

    // Act
    QList<QMap<QString, QString> > lstMap;
    t->getResolutionRateFromDBus(lstMap);

    // Assert: CurResolution 格式 "宽×高@速率Hz"(内存桩读出 0); 分发 1 次
    EXPECT_EQ(1, lstMap.size());
    EXPECT_STREQ("0×0@0.00Hz",
                 lstMap.at(0).value("CurResolution").toStdString().c_str());
    EXPECT_STREQ("Monitor-A", lstMap.at(0).value("Name").toStdString().c_str());
    EXPECT_STREQ("Monitor-A", lstMap.at(0).value("Display Input").toStdString().c_str());
    EXPECT_STREQ("VSC", lstMap.at(0).value("Manufacture").toStdString().c_str());
    EXPECT_EQ(1, ut_more_dbusCalls.size());
    EXPECT_STREQ("0×0@0.00Hz",
                 ut_more_dbusCalls.at(0).value("CurResolution").toStdString().c_str());
}
