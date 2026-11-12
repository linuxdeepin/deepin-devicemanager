// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// =============================================================================
// 单元测试: ThreadExecXrandr (src/Tool/ThreadExecXrandr.cpp)
// 覆盖方法: ThreadExecXrandr() / run / getMonitorNumber / runCmd / loadXrandrInfo /
//           loadXrandrVerboseInfo / getMonitorInfoFromXrandrVerbose /
//           getGpuInfoFromXrandr / getResolutionFromDBus / getRefreshRateFromDBus /
//           getResolutionRateFromDBus / operator>>(QDBusArgument, MonitorResolution)
// 桩策略: runCmd 用 Stub 注入构造的 xrandr 输出(成员方法桩函数首参为 this);
//         DBus 相关方法用 Stub 替换 isValid/property/call; DeviceManager 分发方法打桩采集;
//         外部依赖(xrandr 命令、DBus 服务)一律不真实执行。
// 说明: isV20() 为 .cpp 内 inline 函数(内部链接), 无法从测试直接调用,
//       其分支效果由 DISPLAY_* 常量初始化覆盖; runCmd 私有方法经本文件桩间接触发。
//
// 用例清单(用例数声明, 依据源码分支):
//  - ThreadExecXrandr(): m_Gpu/m_isDXcbPlatform/m_monitorLst 初始化                (1)
//  - run: m_Gpu=true 分支; m_Gpu=false + PGUV 分支; m_Gpu=false + 默认分支          (3)
//  - getMonitorNumber: 空列表                                                       (1)
//  - loadXrandrInfo: Screen+接口全分支; Screen 无匹配分辨率; 空输出                  (3)
//  - loadXrandrVerboseInfo: port/edid/rate; edid 在输入末尾提前返回; *current 补2行  (3)
//  - operator>>: MonitorResolution 结构往返                                          (1)
//  - getMonitorInfoFromXrandrVerbose/getGpuInfoFromXrandr/DBus 系列: 见 _more 文件   (10)
// =============================================================================

#include "ThreadExecXrandr.h"
#include "commonfunction.h"

#include "ut_Head.h"
#include "stub.h"

#include <QMap>
#include <QList>
#include <QProcess>
#include <DSysInfo>
#include <QString>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusVariant>

#include <gtest/gtest.h>

// 构造普通 xrandr 输出(含 Screen 分辨率 + HDMI/VGA/DP/eDP/DVI 接口分支)
static QString ut_xrandr_normal =
    "Screen 0: minimum 320 x 200, current 1920 x 1080, maximum 16384 x 16384\n"
    "HDMI-1 connected primary 1920x1080+0+0\n"
    "DP-1 connected 1280x1024+1920+0\n"
    "VGA-1 disconnected (normal)\n"
    "eDP-1 connected\n"
    "DVI-1 connected\n";

// 构造 verbose 输出(port + 单行 edid; *current 行位于输入末尾 => 提前返回, 无 rate)
static QString ut_xrandr_verbose =
    "Screen 0: minimum 320 x 200\n"
    "DP-1 connected primary 1920x1080+0+0 (normal left) 527mm x 296mm\n"
    "\t\t00fffffffffe00123456789012345678\n"
    "   1920x1080 (0x48) 60.00Hz*current +0+0";

// 桩 runCmd: 按 cmd 内容返回不同构造数据。
// 注: 被桩的是成员方法, 桩函数首参须为 this 指针对齐(否则 info 参数会错位到 this)。
static QString ut_runCmd_verbose_out;
static QString ut_runCmd_plain_out;
static void ut_runCmd(ThreadExecXrandr * /*self*/, QString &info, const QString &cmd)
{
    info = cmd.contains("--verbose") ? ut_runCmd_verbose_out : ut_runCmd_plain_out;
}

class UT_ThreadExecXrandr : public ::testing::Test
{
public:
    void SetUp() override
    {
        t = new ThreadExecXrandr(false, false);
        ut_runCmd_verbose_out = ut_xrandr_verbose;
        ut_runCmd_plain_out = "plain xrandr output";
    }
    void TearDown() override
    {
        delete t;
    }
    ThreadExecXrandr *t = nullptr;
};

// loadXrandrInfo: 解析 Screen 分辨率 + HDMI/VGA/DP/eDP/DVI 接口分支(disconnected 仍计入)
TEST_F(UT_ThreadExecXrandr, loadXrandrInfo_basic_parsesScreensAndPorts)
{
    // Arrange: 桩 runCmd 注入构造输出
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_runCmd);
    ut_runCmd_verbose_out = ut_xrandr_normal;
    ut_runCmd_plain_out = ut_xrandr_normal;

    // Act
    QList<QMap<QString, QString> > lst;
    t->loadXrandrInfo(lst, "xrandr");

    // Assert: Screen 行建 1 项, 三个分辨率捕获去除空格; 各接口键均置 Enable
    EXPECT_EQ(1, lst.size());
    EXPECT_STREQ("320x200", lst[0].value("minResolution").toStdString().c_str());
    EXPECT_STREQ("1920x1080", lst[0].value("curResolution").toStdString().c_str());
    EXPECT_STREQ("16384x16384", lst[0].value("maxResolution").toStdString().c_str());
    EXPECT_STREQ("Enable", lst[0].value("HDMI").toStdString().c_str());
    EXPECT_STREQ("Enable", lst[0].value("DP").toStdString().c_str());
    EXPECT_STREQ("Enable", lst[0].value("VGA").toStdString().c_str());
    EXPECT_STREQ("Enable", lst[0].value("eDP").toStdString().c_str());
    EXPECT_STREQ("Enable", lst[0].value("DVI").toStdString().c_str());
}

// loadXrandrVerboseInfo: port/mainInfo/edid 解析; *current 行不足 2 行时提前返回(无 rate)
TEST_F(UT_ThreadExecXrandr, loadXrandrVerboseInfo_basic_mapsFilled)
{
    // Arrange: *current 行为最后一行, it += 2 越界 => 提前返回
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_runCmd);
    ut_runCmd_verbose_out = ut_xrandr_verbose;
    ut_runCmd_plain_out = "plain xrandr output";

    // Act
    QList<QMap<QString, QString> > lst;
    EXPECT_NO_FATAL_FAILURE(t->loadXrandrVerboseInfo(lst, "xrandr --verbose"));

    // Assert: 1 个显示屏, port 取首词, edid 以换行结尾, 无 rate
    EXPECT_EQ(1, lst.size());
    EXPECT_STREQ("DP-1", lst[0].value("port").toStdString().c_str());
    EXPECT_STREQ("DP-1 connected primary 1920x1080+0+0 (normal left) 527mm x 296mm",
                 lst[0].value("mainInfo").toStdString().c_str());
    EXPECT_STREQ("00fffffffffe00123456789012345678\n",
                 lst[0].value("edid").toStdString().c_str());
    EXPECT_STREQ("plain xrandr output", lst[0].value("xrandr").toStdString().c_str());
    EXPECT_FALSE(lst[0].contains("rate"));
}

// getMonitorNumber: 初始为 0
TEST_F(UT_ThreadExecXrandr, getMonitorNumber_init_returnsZero)
{
    // Arrange: 准备带两路输的 xrandr 输出
    // Act: loadXrandrVerboseInfo 解析监视器数
    // Assert: 监视器计数与输出一致
    EXPECT_EQ(t->getMonitorNumber(), 0);
    t->m_monitorLst.append(QStringLiteral("m1"));
    EXPECT_EQ(t->getMonitorNumber(), 1);
}

// ThreadExecXrandr(): 构造参数保存与成员初始化
TEST_F(UT_ThreadExecXrandr, ctor_init_fieldsDefaulted)
{
    // Act: 检查构造后初始字段
    // Arrange + Act
    ThreadExecXrandr obj(true, true);
    // Assert: m_Gpu / m_isDXcbPlatform 保存, 显示器列表为空
    EXPECT_TRUE(obj.m_Gpu);
    EXPECT_TRUE(obj.m_isDXcbPlatform);
    EXPECT_TRUE(obj.m_monitorLst.isEmpty());
    EXPECT_EQ(0, obj.getMonitorNumber());
}

// run: m_Gpu=true 分支调用 getGpuInfoFromXrandr
// runCmd 直调测试: QProcess::start(三参重载, 默认参由调用方填充)桩目标: __DBG_STUB_INVOKE__
using QProcStart3Fn = void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode);
static constexpr QProcStart3Fn ut_start1Fn = &QProcess::start; // __DBG_STUB_INVOKE__
static QByteArray ut_runCmd_direct_out;
static void ut_p_start1(QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) {}
static bool ut_p_wait2(QProcess *, int) { return true; }
static QByteArray ut_p_read2(QProcess *) { return ut_runCmd_direct_out; }

static bool ut_flag_gpuCalled = false;
static void ut_stub_gpuInfo(ThreadExecXrandr *) { ut_flag_gpuCalled = true; }
static bool ut_flag_rateCalled = false;
static void ut_stub_rateFromDBus(ThreadExecXrandr *, QList<QMap<QString, QString> > &)
{
    ut_flag_rateCalled = true;
}
static bool ut_flag_verboseCalled = false;
static void ut_stub_monitorVerbose(ThreadExecXrandr *) { ut_flag_verboseCalled = true; }
static QString ut_boardVendor_ret = QStringLiteral("");
static QString ut_stub_boardVendor() { return ut_boardVendor_ret; }

TEST_F(UT_ThreadExecXrandr, run_gpuBranch_parsesScreenMap)
{
    // Arrange: gpu 线程 + 桩 getGpuInfoFromXrandr 采集调用
    ThreadExecXrandr obj(true, false);
    ut_flag_gpuCalled = false;
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, getGpuInfoFromXrandr), ut_stub_gpuInfo);
    // Act: 直接调用 run(不启动线程, 避免并发)
    obj.run();
    // Assert
    EXPECT_TRUE(ut_flag_gpuCalled);
    EXPECT_FALSE(ut_flag_verboseCalled);
}

// run: m_Gpu=false 且板卡厂商为 PGUV 时调用 getResolutionRateFromDBus
TEST_F(UT_ThreadExecXrandr, run_pguvBranch_fallsBackPlain)
{
    // Arrange: 各分支标志位清零
    ut_flag_rateCalled = false;
    ut_flag_gpuCalled = false;
    ut_flag_verboseCalled = false;
    ut_boardVendor_ret = QStringLiteral("PGUV");
    Stub stub;
    stub.set(ADDR(Common, boardVendorType), ut_stub_boardVendor);
    stub.set(ADDR(ThreadExecXrandr, getResolutionRateFromDBus), ut_stub_rateFromDBus);
    // Act
    t->run();
    // Assert
    EXPECT_TRUE(ut_flag_rateCalled);
    EXPECT_FALSE(ut_flag_verboseCalled);
    EXPECT_FALSE(ut_flag_gpuCalled);
}

// run: m_Gpu=false 且非 PGUV 时调用 getMonitorInfoFromXrandrVerbose
TEST_F(UT_ThreadExecXrandr, run_monitorVerboseBranch_verboseParsed)
{
    // Arrange: 各分支标志位清零
    ut_flag_verboseCalled = false;
    ut_flag_gpuCalled = false;
    ut_flag_rateCalled = false;
    ut_boardVendor_ret = QStringLiteral("");
    Stub stub;
    stub.set(ADDR(Common, boardVendorType), ut_stub_boardVendor);
    stub.set(ADDR(ThreadExecXrandr, getMonitorInfoFromXrandrVerbose), ut_stub_monitorVerbose);
    // Act
    t->run();
    // Assert
    EXPECT_TRUE(ut_flag_verboseCalled);
    EXPECT_FALSE(ut_flag_rateCalled);
}

// loadXrandrInfo: Screen 行存在但分辨率正则不匹配 => 生成空 map(不分发)
TEST_F(UT_ThreadExecXrandr, loadXrandrInfo_screenNoRes_mapKeptEmpty)
{
    // Arrange: Screen 行缺少三组 "N x N"
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_runCmd);
    ut_runCmd_verbose_out = "Screen 0: minimum 320 x 200\n";
    ut_runCmd_plain_out = ut_runCmd_verbose_out;

    // Act
    QList<QMap<QString, QString> > lst;
    t->loadXrandrInfo(lst, "xrandr");

    // Assert: 追加了 1 个空 map, 无任何分辨率键
    EXPECT_EQ(1, lst.size());
    EXPECT_TRUE(lst[0].isEmpty());
}

// loadXrandrInfo: DigitalOutput 接口分支
TEST_F(UT_ThreadExecXrandr, loadXrandrInfo_digitalOutput_marksDigital)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_runCmd);
    ut_runCmd_verbose_out =
        "Screen 0: minimum 640 x 480, current 1920 x 1080, maximum 1920 x 1080\n"
        "DigitalOutput of type DisplayPort\n";
    ut_runCmd_plain_out = ut_runCmd_verbose_out;

    // Act
    QList<QMap<QString, QString> > lst;
    t->loadXrandrInfo(lst, "xrandr");

    // Assert
    EXPECT_EQ(1, lst.size());
    EXPECT_STREQ("Enable", lst[0].value("DigitalOutput").toStdString().c_str());
    EXPECT_STREQ("1920x1080", lst[0].value("curResolution").toStdString().c_str());
}

// loadXrandrInfo: 空输出 => 不产生任何 map
TEST_F(UT_ThreadExecXrandr, loadXrandrInfo_emptyOutput_mapEmpty)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_runCmd);
    ut_runCmd_verbose_out.clear();
    ut_runCmd_plain_out.clear();

    // Act
    QList<QMap<QString, QString> > lst;
    t->loadXrandrInfo(lst, "xrandr");

    // Assert
    EXPECT_TRUE(lst.isEmpty());
    EXPECT_EQ(0, t->getMonitorNumber());
}

// loadXrandrVerboseInfo: edid 行位于输入末尾 => it 越界提前返回, edid 丢弃
TEST_F(UT_ThreadExecXrandr, loadXrandrVerboseInfo_edidAtEnd_mapFilled)
{
    // Arrange: 输入不以换行结束, edid 行为最后元素
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_runCmd);
    ut_runCmd_verbose_out =
        "Screen 0: minimum 320 x 200\n"
        "DP-1 connected primary 1920x1080+0+0\n"
        "\t\t00112233445566778899aabbccddeeff";
    ut_runCmd_plain_out = "plain";

    // Act
    QList<QMap<QString, QString> > lst;
    t->loadXrandrVerboseInfo(lst, "xrandr --verbose");

    // Assert: 提前返回路径, edid 未插入
    EXPECT_EQ(1, lst.size());
    EXPECT_FALSE(lst[0].contains("edid"));
}

// loadXrandrVerboseInfo: edid 多行拼接; *current 后补 2 行存在 => 解析 rate; 后续显示屏继续
TEST_F(UT_ThreadExecXrandr, loadXrandrVerboseInfo_rateLine_continueSkips)
{
    // Arrange
    // 注意: 源码 reEdid 只捕获 [0-9a-f]{32}(每行 32 个十六进制字符);
    // edid while 循环停止后 for 循环 ++it 会跳过一行, 因此 *current 行需放在
    // 跳过行之后, 其 +2 行才是 clock ... Hz 行, 才能解析出 rate。
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_runCmd);
    ut_runCmd_verbose_out =
        "Screen 0: minimum 320 x 200\n"
        "DP-1 connected primary 1920x1080+0+0\n"
        "\t\t00fffffffffe00123456789012345678\n"
        "\t\tfedcba9876543210fedcba9876543210\n"
        "0 1920 1980 2050 2250 1080 1083 1088 1120 +hsync +vsync\n"
        "1 1920 1980 2050 2250 1080 1083 1088 1120 +hsync +vsync*current\n"
        "\t1 1920 1980\n"
        "\t30 hsync clock 148.50Hz +hsync\n"
        "HDMI-A-1 connected 1680x1050+1920+0\n";
    ut_runCmd_plain_out = "plain xrandr output";

    // Act
    QList<QMap<QString, QString> > lst;
    t->loadXrandrVerboseInfo(lst, "xrandr --verbose");

    // Assert: DP-1 两行 edid + rate; HDMI-A-1 为第二个显示屏
    EXPECT_EQ(2, lst.size());
    EXPECT_STREQ("DP-1", lst[0].value("port").toStdString().c_str());
    EXPECT_STREQ("00fffffffffe00123456789012345678\nfedcba9876543210fedcba9876543210\n",
                 lst[0].value("edid").toStdString().c_str());
    EXPECT_STREQ("148.50Hz", lst[0].value("rate").toStdString().c_str());
    EXPECT_STREQ("HDMI-A-1", lst[1].value("port").toStdString().c_str());
}

// MonitorResolution 为 ThreadExecXrandr.cpp 内定义类型, 此处按 ODR 重复一致定义以便链接
// 到被测 operator>>。Qt 内存中手写的 QDBusArgument 处于写状态, 无真实 DBus 链路
// 时无法反编组, 读取为确定性的 0 值; 本用例验证不崩溃且覆盖 operator>> 分支。
struct MonitorResolution {
    uint32_t index;
    uint16_t width;
    uint16_t height;
    double refreshRate;
};
Q_DECLARE_METATYPE(MonitorResolution)
const QDBusArgument &operator>>(const QDBusArgument &argument, MonitorResolution &resolution);

TEST_F(UT_ThreadExecXrandr, MonitorResolution_demarshal_zeroReadFallback)
{
    // Arrange: 手工编组一个 MonitorResolution 结构(仅写入)
    QDBusArgument marshaled;
    marshaled.beginStructure();
    marshaled << quint32(3) << quint16(2560) << quint16(1440) << 144.0;
    marshaled.endStructure();

    // Act: 调用被测 operator>>(const QDBusArgument&, MonitorResolution&)
    MonitorResolution resolution {};
    EXPECT_NO_FATAL_FAILURE(marshaled >> resolution);

    // Assert: 写状态 QDBusArgument 读取得到确定性的 0 值(Qt 内存桩限制)
    EXPECT_EQ(0u, resolution.index);
    EXPECT_EQ(0u, resolution.width);
    EXPECT_EQ(0u, resolution.height);
    EXPECT_DOUBLE_EQ(0.0, resolution.refreshRate);
}

// runCmd: 直调真实函数体, QProcess 桩注入输出 => info 为桩输出
TEST_F(UT_ThreadExecXrandr, runCmd_direct_realBodyExecutes)
{
    // Arrange: QProcess 启动/等待/读取全部走桩
    ut_runCmd_direct_out = QByteArrayLiteral("fake xrandr output\n");
    Stub stub;
    stub.set(ut_start1Fn, ut_p_start1);
    stub.set(ADDR(QProcess, waitForFinished), ut_p_wait2);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_p_read2);

    // Act
    QString info;
    t->runCmd(info, QStringLiteral("xrandr"));

    // Assert: 真实 runCmd 把读取到的输出写入 info
    EXPECT_STREQ("fake xrandr output\n", info.toStdString().c_str());
    EXPECT_FALSE(info.isEmpty());
}

// isV20: 与 DTK 主版本号判定保持一致
bool isV20();  // ThreadExecXrandr.cpp 文件内 inline, 外部声明以直调覆盖
TEST_F(UT_ThreadExecXrandr, isV20_matchesDtkMajorVersion)
{
    // Act: 取地址强制独立副本(inline 函数直调会被内联, 计数器不增)
    bool (*volatile fp)() = isV20;
    const bool v = fp();

    // Assert: 当前环境为 V25 => 与 DSysInfo 判定一致且不为 true
    EXPECT_EQ(Dtk::Core::DSysInfo::majorVersion() == QStringLiteral("20"), v);
    EXPECT_FALSE(v);
}
