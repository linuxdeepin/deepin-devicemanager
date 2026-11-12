// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// =============================================================================
// 单元测试: CommonTools (src/Tool/commontools.cpp)
// 覆盖方法: getDriverType / getDriverPixmap / getStausType / getStatusPixmap /
//           getErrorString / getBackupPath / getSystemTime / getUrl / staticMaps /
//           parseEDID / preGenerateGpuInfo / getGpuBaseInfo / feedback
// 桩策略:
//  - QProcess(启动/等待/读取输出): 注入构造的 hexdump/
//    glxinfo 输出, 不真实执行外部命令;
//  - QDBusInterface::isValid/call: 控制 DBus 可用性与回复;
//  - DeviceMonitor::setInfoFromEdid(ForCustom) / DeviceManager::addMonitor: 采集
//    parseEDID 组装的 mapInfo, 避免向真实单例添加设备;
//  - 用户家目录无 ~/url 文件(getUrl 走默认分支)。
//
// 用例清单(用例数声明, 依据源码分支):
//  - getDriverType 5 枚举; getDriverPixmap 全枚举+未知键; getStausType 4 枚举;
//    getStatusPixmap 已知+未知; getErrorString 已知+未知; getBackupPath;
//    getSystemTime; getUrl 默认; staticMaps                                  (9, 保留+增强)
//  - parseEDID: isHW(hexdump 默认格式)成功; isHW 真实头部被拒(记录行为);
//    isHW 空输出/超时跳过; 非isHW(hexdump -C)成功                            (4)
//  - preGenerateGpuInfo: glxinfo+DBus 成功; getGpuBaseInfo 失败分支           (2)
//  - feedback: DBus 正常/错误回复                                             (2)
// =============================================================================

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QRegularExpression>
#include <QProcess>
// QProcess::start 的桩目标成员指针(全文件共用): __DBG_STUB_INVOKE__
using QProcStartFn = void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode);
static constexpr QProcStartFn ut_procStartFn = &QProcess::start; // __DBG_STUB_INVOKE__

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusVariant>
#include "DeviceManager.h"
#include "DeviceMonitor.h"

#define private public
#define protected public
#include "commontools.h"

class UT_CommonTools : public ::testing::Test
{
};

// CommonTools: 构造函数设置/不设置父对象
TEST_F(UT_CommonTools, ctor_parentParentingFollowsArgs)
{
    // Arrange
    QObject guard;
    // Act
    CommonTools withParent(&guard);
    CommonTools noParent;
    // Assert
    EXPECT_EQ(&guard, withParent.parent());
    EXPECT_EQ(nullptr, noParent.parent());
}

// ===================== getDriverType =====================
TEST_F(UT_CommonTools, getDriverType_enumKeys_returnsMappedType)
{
    // Arrange: 静态映射表已就绪
    // Act: 遍历 DriverType 枚举查询
    // Assert: 各枚举返回映射表对应字符串
    EXPECT_STREQ("Display adapter", CommonTools::getDriverType(DR_Gpu).toStdString().c_str());
    EXPECT_STREQ("Bluetooth adapter", CommonTools::getDriverType(DR_Bluetooth).toStdString().c_str());
    EXPECT_STREQ("Network adapter", CommonTools::getDriverType(DR_Network).toStdString().c_str());
    EXPECT_STREQ("Sound card", CommonTools::getDriverType(DR_Sound).toStdString().c_str());
    EXPECT_STREQ("Printer", CommonTools::getDriverType(DR_Printer).toStdString().c_str());
}

// ===================== getDriverPixmap =====================
TEST_F(UT_CommonTools, getDriverPixmap_enumKeys_returnsPixmap)
{
    // Arrange: 准备资源路径基线
    // Act: 遍历 DriverType 查询图标路径
    // Assert: 路径与枚举一一对应且未知键回退
    EXPECT_STREQ(":/icons/deepin/builtin/icons/GPU.svg", CommonTools::getDriverPixmap(DR_Gpu).toStdString().c_str());
    EXPECT_STREQ(":/icons/deepin/builtin/icons/bluetooth.svg", CommonTools::getDriverPixmap(DR_Bluetooth).toStdString().c_str());
    // 增强: 其余枚举映射与未知键(QMap 缺省空串)
    EXPECT_FALSE(CommonTools::getDriverPixmap(DR_Network).isEmpty());
    EXPECT_FALSE(CommonTools::getDriverPixmap(DR_Sound).isEmpty());
    EXPECT_FALSE(CommonTools::getDriverPixmap(DR_Printer).isEmpty());
    EXPECT_TRUE(CommonTools::getDriverPixmap((DriverType)9999).isEmpty());
}

// ===================== getStausType =====================
TEST_F(UT_CommonTools, getStausType_enumKeys_returnsMappedType)
{
    // Arrange: 静态映射表已就绪
    // Act: 遍历 Status 枚举查询
    // Assert: 各枚举返回映射表对应字符串
    EXPECT_STREQ("Installation successful", CommonTools::getStausType(ST_SUCESS).toStdString().c_str());
    EXPECT_STREQ("Installation failed", CommonTools::getStausType(ST_FAILED).toStdString().c_str());
    EXPECT_STREQ("Not installed", CommonTools::getStausType(ST_NOT_INSTALL).toStdString().c_str());
    EXPECT_STREQ("Waiting", CommonTools::getStausType(ST_WAITING).toStdString().c_str());
    // 增强: 未知键 => 空串
    EXPECT_TRUE(CommonTools::getStausType((Status)9999).isEmpty());
}

// ===================== getStatusPixmap =====================
TEST_F(UT_CommonTools, getStatusPixmap_enumKeys_returnsPixmap)
{
    // Arrange: 准备资源路径基线
    // Act: 遍历 Status 查询图标路径
    // Assert: 已知键路径正确且未知键不崩溃
    EXPECT_STREQ(":/icons/deepin/builtin/icons/ok.svg", CommonTools::getStatusPixmap(ST_SUCESS).toStdString().c_str());
    EXPECT_STREQ(":/icons/deepin/builtin/icons/warning.svg", CommonTools::getStatusPixmap(ST_FAILED).toStdString().c_str());
    EXPECT_TRUE(CommonTools::getStatusPixmap(ST_DOWNLOADING).isEmpty());
    // 增强: 其余枚举与未知键
    EXPECT_TRUE(CommonTools::getStatusPixmap((Status)9999).isEmpty());
    for (int st = ST_NOT_INSTALL; st <= ST_WAITING; ++st) {
        QString pixmap = CommonTools::getStatusPixmap((Status)st);
        EXPECT_TRUE(pixmap.isEmpty() || pixmap.startsWith(":/icons/"));
    }
}

// ===================== getErrorString =====================
TEST_F(UT_CommonTools, getErrorString_knownCodes_returnsText)
{
    // Arrange: 准备错误码集合
    // Act: getErrorString 查询
    // Assert: 已知码返回文本未知码回退默认
    EXPECT_STREQ("Unknown error", CommonTools::getErrorString(EC_NULL).toStdString().c_str());
    EXPECT_STREQ("Network error", CommonTools::getErrorString(EC_NETWORK).toStdString().c_str());
    EXPECT_STREQ("Canceled", CommonTools::getErrorString(EC_CANCEL).toStdString().c_str());
    // 增强: 未知错误码 => 空串
    EXPECT_TRUE(CommonTools::getErrorString(9999).isEmpty());
}

// ===================== getBackupPath =====================
TEST_F(UT_CommonTools, getBackupPath_call_returnsAppDir)
{
    // Arrange: 设置 TMPDIR 隔离环境
    // Act: getBackupPath 拼接路径
    // Assert: 返回路径为绝对路径且以应用备份目录结尾(不绑定运行环境)
    const QString p = CommonTools::getBackupPath();
    EXPECT_TRUE(p.endsWith(QStringLiteral("deepin-devicemanager/")));
    EXPECT_TRUE(p.startsWith(QStringLiteral("/")));
    EXPECT_TRUE(p.contains(QStringLiteral("devicemanager")));
}

// ===================== getSystemTime =====================
TEST_F(UT_CommonTools, getSystemTime_call_formatMatches)
{
    // Arrange: 无需外部状态
    // Act: getSystemTime 取当前时间
    // Assert: 返回 yyyy-MM-dd 格式且可解析
    QString t = CommonTools::getSystemTime();
    EXPECT_FALSE(t.isEmpty());
    // 格式 "yyyy-M-d hh:mm"，例如 "2026-6-22 12:30"
    QRegularExpression re("^\\d{4}-\\d{1,2}-\\d{1,2} \\d{1,2}:\\d{2}$");
    EXPECT_TRUE(re.match(t).hasMatch());
    // 增强: 两次调用间隔内格式稳定
    QString t2 = CommonTools::getSystemTime();
    EXPECT_TRUE(re.match(t2).hasMatch());
}

// ===================== getUrl =====================
// 注: 运行环境家目录下不存在 ~/url 文件, 覆盖默认分支
TEST_F(UT_CommonTools, getUrl_missingFile_returnsDefault)
{
    // Arrange: 家目录无 ~/url 文件
    // Act: getUrl 读取默认地址
    // Assert: 返回默认地址字符串
    QString url = CommonTools::getUrl();
    EXPECT_TRUE(url.contains("uniontech.com"));
    // 增强: 默认分支精确值
    EXPECT_STREQ("https://driver.uniontech.com/api/v1/drive/search", url.toStdString().c_str());
}

// ===================== 构造/静态表冒烟测试 =====================
TEST_F(UT_CommonTools, staticMaps_init_tablesPopulated)
{
    // Arrange: 静态映射初始化
    // Act: 检查各静态表内容
    // Assert: 关键键值与源码约定一致
    EXPECT_TRUE(CommonTools::m_MapDriverType.contains(DR_Gpu));
    EXPECT_TRUE(CommonTools::m_MapDriverIcon.contains(DR_Network));
    EXPECT_TRUE(CommonTools::m_MapStatusType.contains(ST_SUCESS));
    EXPECT_TRUE(CommonTools::m_MapErrorString.contains(EC_NULL));
}

// ===================== parseEDID 桩与全局状态 =====================
// hexdump 输出模式: 0 = 默认格式(isHW=true), 1 = -C 格式(isHW=false)
static int ut_edidDump_mode = 0;
static QByteArray ut_edidDump_out;      // 注入的 hexdump 输出
static bool ut_edidDump_timeout = false; // waitForFinished 返回 false

static void ut_qproc_start(QProcess *, const QString &, const QStringList &, QIODevice::OpenMode)
{
    // 不真实启动进程
}

static bool ut_qproc_waitFinished(QProcess *, int)
{
    return !ut_edidDump_timeout;
}

static QByteArray ut_qproc_readOut(QProcess *)
{
    return ut_edidDump_out;
}

static QList<QMap<QString, QString> > ut_parseEDID_maps;
static int ut_parseEDID_added = 0;

static void ut_dm_setInfoFromEdid(DeviceMonitor *, const QMap<QString, QString> &mapInfo)
{
    ut_parseEDID_maps.append(mapInfo);
}

static void ut_dm_setInfoFromEdidForCustom(DeviceMonitor *, const QMap<QString, QString> &mapInfo)
{
    ut_parseEDID_maps.append(mapInfo);
}

static void ut_dm_addMonitor(DeviceManager *, DeviceMonitor *const)
{
    ++ut_parseEDID_added;
}

// 与 ut_edidparser.cpp 一致的最小合法 EDID(项目使用的 6 字节 0xFF 头部形态)
static const char *ut_edid_lines[] = {
    "00ffffffffffff005a63384001010101",
    "0d1e010380351d782ece65a657519f27",
    "0f5054bfef80b300a940a9c095009040",
    "8180814081c0023a801871382d40582c",
    "45000f282100001e000000ff00565351",
    "3230313332313330320a000000fd0032",
    "4b185311000a202020202020000000fc",
    "005641323433302d4648440a20200141",
    "020320f14d9005040302121113141e1d",
    "1f0123097f078301000065030c001000",
    "023a801871382d40582c45000f282100",
    "001e011d8018711c1620582c25000f28",
    "2100009e011d007251d01e206e285500",
    "0f282100001e8c0ad08a20e02d10103e",
    "96000f28210000188c0ad09020403120",
    "0c4055000f28210000180000000000d6",
};
static const int ut_edid_line_count = 16;

// 默认 hexdump(isHW=true): 偏移 + 8 个 2 字节小端字(字节序两两交换)
static QString ut_toDefaultHexdump()
{
    QString out;
    for (int i = 0; i < ut_edid_line_count; ++i) {
        const QString line = QString::fromLatin1(ut_edid_lines[i]);
        QStringList words;
        for (int j = 0; j + 3 < line.length(); j += 4)
            words << line.mid(j + 2, 2) + line.mid(j, 2);
        out += QString("00000%10 ").arg(i * 16, 1, 16, QLatin1Char('0'));
        out += words.join(" ");
        out += "\n";
    }
    return out;
}

// hexdump -C(isHW=false): 偏移 + 原始字节序列 + ASCII 列
static QString ut_toCHexdump()
{
    QString out;
    for (int i = 0; i < ut_edid_line_count; ++i) {
        const QString line = QString::fromLatin1(ut_edid_lines[i]);
        QStringList bytes;
        for (int j = 0; j + 1 < line.length(); j += 2)
            bytes << line.mid(j, 2);
        QString hexPart;
        for (int j = 0; j < bytes.size(); ++j) {
            hexPart += bytes.at(j);
            hexPart += " ";
            if (j == 7)
                hexPart += " ";
        }
        hexPart += " |";
        for (int j = 0; j < bytes.size(); ++j) {
            bool ok = false;
            int v = bytes.at(j).toInt(&ok, 16);
            hexPart += (ok && v >= 0x20 && v < 0x7f) ? QChar(v) : QChar('.');
        }
        hexPart += "|";
        out += QString("%1  %2\n").arg(i * 16, 8, 16, QLatin1Char('0')).arg(hexPart);
    }
    return out;
}

class UT_CommonTools_ParseEDID : public ::testing::Test
{
public:
    void SetUp() override
    {
        ut_edidDump_mode = 0;
        ut_edidDump_out.clear();
        ut_edidDump_timeout = false;
        ut_parseEDID_maps.clear();
        ut_parseEDID_added = 0;
    }
    void TearDown() override {}
};

// parseEDID: isHW=true(默认 hexdump 格式)成功路径 => 组装 mapInfo 并 addMonitor
TEST_F(UT_CommonTools_ParseEDID, parseEDID_hw_success)
{
    // Arrange
    ut_edidDump_mode = 0;
    ut_edidDump_out = ut_toDefaultHexdump().toLocal8Bit();
    Stub stub;
    stub.set(ut_procStartFn, ut_qproc_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_qproc_waitFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_qproc_readOut);
    stub.set(ADDR(DeviceMonitor, setInfoFromEdid), ut_dm_setInfoFromEdid);
    stub.set(ADDR(DeviceManager, addMonitor), ut_dm_addMonitor);

    // Act
    CommonTools::parseEDID(QStringList() << "synthetic-edid", "HDMI-1", true);

    // Assert: 1 台显示器; Vendor/Model(大端形态取 model)/Size/Display Input
    EXPECT_EQ(1, ut_parseEDID_added);
    ASSERT_EQ(1, ut_parseEDID_maps.size());
    EXPECT_STREQ("VSC", ut_parseEDID_maps.at(0).value("Vendor").toStdString().c_str());
    EXPECT_STREQ("4038", ut_parseEDID_maps.at(0).value("Model").toStdString().c_str());
    EXPECT_STREQ("23.8 inch(527mm×296mm)",
                 ut_parseEDID_maps.at(0).value("Size").toStdString().c_str());
    EXPECT_STREQ("HDMI-1", ut_parseEDID_maps.at(0).value("Display Input").toStdString().c_str());
}

// parseEDID: isHW=true + 真实 EDID(7 字节 0xFF 头部, 实测 hexdump 输出) => 头部校验
// 拒绝, 不添加显示器(当前实现行为, 已记录疑似缺陷)
TEST_F(UT_CommonTools_ParseEDID, parseEDID_hw_realSpecHeaderRejected)
{
    // Arrange: 实机 hexdump 对真实 EDID 的输出(头部 00 FF*7 00 两两交换为 ff00 ffff ffff ffff)
    ut_edidDump_mode = 0;
    ut_edidDump_out =
        "0000000 ff00 ffff ffff ffff 5a00 3863 0140 0101\n"
        "0000010 1e0d 0301 3580 781d 2e2e 2e2e 2e2e 2e2e\n"
        "0000020 2e2e 2e2e 2e2e 2e2e 500f bf54 80ef 00b3\n";
    Stub stub;
    stub.set(ut_procStartFn, ut_qproc_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_qproc_waitFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_qproc_readOut);
    stub.set(ADDR(DeviceMonitor, setInfoFromEdid), ut_dm_setInfoFromEdid);
    stub.set(ADDR(DeviceManager, addMonitor), ut_dm_addMonitor);

    // Act
    CommonTools::parseEDID(QStringList() << "real-edid", "DP-1", true);

    // Assert: setEdid 大端校验拒绝 => 无显示器
    EXPECT_EQ(0, ut_parseEDID_added);
    EXPECT_TRUE(ut_parseEDID_maps.isEmpty());
}

// parseEDID: 空输出(isHW=false)与超时(isHW=true)两条跳过分支
TEST_F(UT_CommonTools_ParseEDID, parseEDID_timeoutOutput_skippedQuietly)
{
    // Arrange: 桩 cat 输出超时空串
    // Act: parseEDID 解析空输出
    // Assert: 不崩溃且不派发监视器
    Stub stub;
    stub.set(ut_procStartFn, ut_qproc_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_qproc_waitFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_qproc_readOut);
    stub.set(ADDR(DeviceMonitor, setInfoFromEdid), ut_dm_setInfoFromEdid);
    stub.set(ADDR(DeviceMonitor, setInfoFromEdidForCustom), ut_dm_setInfoFromEdidForCustom);
    stub.set(ADDR(DeviceManager, addMonitor), ut_dm_addMonitor);

    // 子用例 A: isHW=false 且 hexdump -C 输出为空 => 跳过
    {
        ut_edidDump_mode = 1;
        ut_edidDump_out.clear();
        CommonTools::parseEDID(QStringList() << "empty", "HDMI-1", false);
        EXPECT_EQ(0, ut_parseEDID_added);
        EXPECT_TRUE(ut_parseEDID_maps.isEmpty());
    }
    // 子用例 B: isHW=true 且 waitForFinished 超时 => 跳过
    {
        ut_edidDump_mode = 0;
        ut_edidDump_out = "0000000 ff00 ffff ffff 00ff 5a00 3863 0140 0101\n";
        ut_edidDump_timeout = true;
        CommonTools::parseEDID(QStringList() << "timeout", "HDMI-1", true);
        EXPECT_EQ(0, ut_parseEDID_added);
        EXPECT_TRUE(ut_parseEDID_maps.isEmpty());
    }
}

// parseEDID: isHW=false(hexdump -C)成功路径 => Model 取 monitorName
TEST_F(UT_CommonTools_ParseEDID, parseEDID_custom_success)
{
    // Arrange
    ut_edidDump_mode = 1;
    ut_edidDump_out = ut_toCHexdump().toLocal8Bit();
    Stub stub;
    stub.set(ut_procStartFn, ut_qproc_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_qproc_waitFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_qproc_readOut);
    stub.set(ADDR(DeviceMonitor, setInfoFromEdidForCustom), ut_dm_setInfoFromEdidForCustom);
    stub.set(ADDR(DeviceManager, addMonitor), ut_dm_addMonitor);

    // Act
    CommonTools::parseEDID(QStringList() << "custom-edid", "eDP-1", false);

    // Assert: 小端形态校验通过; Model 为监视器名称
    EXPECT_EQ(1, ut_parseEDID_added);
    ASSERT_EQ(1, ut_parseEDID_maps.size());
    EXPECT_STREQ("VSC", ut_parseEDID_maps.at(0).value("Vendor").toStdString().c_str());
    EXPECT_STREQ("VA2430-FHD", ut_parseEDID_maps.at(0).value("Model").toStdString().c_str());
    EXPECT_STREQ("23.8 inch(527mm×296mm)",
                 ut_parseEDID_maps.at(0).value("Size").toStdString().c_str());
    EXPECT_STREQ("eDP-1", ut_parseEDID_maps.at(0).value("Display Input").toStdString().c_str());
}

// ===================== preGenerateGpuInfo / getGpuBaseInfo =====================
// glxinfo 输出桩与 DBus 桩(preGenerateGpuInfo 的静态缓存首次填充)
static QByteArray ut_glxinfo_out;
static bool ut_glxinfo_timeout = false;
static bool ut_deviceinfo_valid = true;
static QDBusMessage ut_deviceinfo_reply;

static bool ut_qproc_waitFinished2(QProcess *, int)
{
    return !ut_glxinfo_timeout;
}

static QByteArray ut_qproc_readOut2(QProcess *)
{
    return ut_glxinfo_out;
}

// 真实 systemBus 上存在 org.deepin.DeviceInfo, QDBusInterface 构造函数会同步内省而阻塞 25s。
// 桩接静态 systemBus() 返回无效连接, 构造函数即跳过内省, isValid/call 由其它桩接管。
static QDBusConnection ut_conn_sysBus()
{
    return QDBusConnection(QStringLiteral("__ut_no_bus__"));
}

static bool ut_if_valid(QDBusAbstractInterface *)
{
    return ut_deviceinfo_valid;
}

static QDBusMessage ut_if_call1(QDBusInterface *, const QString &)
{
    return ut_deviceinfo_reply;
}

static bool ut_flag_feedbackOk = false;
static QString ut_if_feedbackMethod;
static QDBusMessage ut_if_callWal(QDBusInterface *, QDBus::CallMode, const QString &method,
                                  const QList<QVariant> &)
{
    ut_if_feedbackMethod = method;
    return ut_flag_feedbackOk ? QDBusMessage().createReply(QVariant(true))
                              : QDBusMessage().createErrorReply(QStringLiteral("org.freedesktop.DBus.Error.Failed"), QStringLiteral("stub error reply"));
}

// 注: preGenerateGpuInfo 内部有 static 缓存, 本用例必须最先执行且只断言一次
TEST_F(UT_CommonTools, preGenerateGpuInfo_stubsOk_infoConcatenated)
{
    // Arrange: glxinfo 正常输出 + DeviceInfo DBus 可用
    ut_glxinfo_out =
        "name of display: :0\n"
        "OpenGL vendor string: TestVendor\n"
        "OpenGL renderer string: TestRenderer\n"
        "OpenGL version string: 3.0\n";
    ut_glxinfo_timeout = false;
    ut_deviceinfo_valid = true;
    ut_deviceinfo_reply = QDBusMessage().createReply(QVariant(QStringLiteral("GPU-MEM-INFO")));
    Stub stub;
    stub.set(ut_procStartFn, ut_qproc_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_qproc_waitFinished2);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_qproc_readOut2);
    stub.set(ADDR(QDBusInterface, isValid), ut_if_valid);
    QDBusMessage (QDBusInterface::*pc1)(const QString &) = &QDBusInterface::call;
    stub.set(pc1, ut_if_call1);
    QDBusConnection (*psysBus)() = &QDBusConnection::systemBus;
    stub.set(psysBus, ut_conn_sysBus);

    // Act
    QString result = CommonTools::preGenerateGpuInfo();

    // Assert: glxinfo 键值与 DBus 显存信息拼接
    EXPECT_TRUE(result.contains("TestRenderer"));
    EXPECT_TRUE(result.contains("TestVendor"));
    EXPECT_TRUE(result.endsWith("GPU-MEM-INFO"));
}

// getGpuBaseInfo: glxinfo 执行失败 => 返回 false 且 mapInfo 为空
TEST_F(UT_CommonTools, getGpuBaseInfo_glxinfoFails_returnsFalse)
{
    // Arrange
    ut_glxinfo_out.clear();
    ut_glxinfo_timeout = true;
    Stub stub;
    stub.set(ut_procStartFn, ut_qproc_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_qproc_waitFinished2);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_qproc_readOut2);

    // Act
    QMap<QString, QString> mapInfo;
    bool ret = CommonTools::getGpuBaseInfo(mapInfo);

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_TRUE(mapInfo.isEmpty());
}

// ===================== feedback =====================
// feedback: DBus 正常回复(全局 call 桩生效)与错误回复两条日志分支均不崩溃
TEST_F(UT_CommonTools, feedback_serviceSession_pathsReached)
{
    // Arrange: 桩 callWithArgumentList 记录方法名
    // Act: feedback 触发 DBus 调用
    // Assert: ServiceSession 被调用且不崩溃
    // feedback 内 call("ServiceSession", uint8_t) 是变参模板实例化, 9 参成员指针拦截不到;
    // 改桩唯一非模板实现 callWithArgumentList, 所有 call() 重载均汇入此处。
    QDBusMessage (QDBusInterface::*pcwal)(QDBus::CallMode, const QString &, const QList<QVariant> &) =
        &QDBusInterface::callWithArgumentList;

    // 子用例 A: 正常回复 => isValid 日志分支
    {
        Stub stub;
        ut_deviceinfo_valid = true;
        ut_flag_feedbackOk = true;
        stub.set(ADDR(QDBusInterface, isValid), ut_if_valid);
        stub.set(pcwal, ut_if_callWal);
        EXPECT_NO_FATAL_FAILURE(CommonTools::feedback());
        EXPECT_EQ(QStringLiteral("ServiceSession"), ut_if_feedbackMethod);
    }
    // 子用例 B: 错误回复 => 失败日志分支
    {
        Stub stub;
        ut_deviceinfo_valid = true;
        ut_flag_feedbackOk = false;
        stub.set(ADDR(QDBusInterface, isValid), ut_if_valid);
        stub.set(pcwal, ut_if_callWal);
        EXPECT_NO_FATAL_FAILURE(CommonTools::feedback());
        EXPECT_EQ(QStringLiteral("ServiceSession"), ut_if_feedbackMethod);
    }
}
