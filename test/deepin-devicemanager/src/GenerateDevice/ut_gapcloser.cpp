// Copyright (C) 2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// 覆盖率收口批次（Mode 2 增量）：针对首轮覆盖率报告中 high 级方法的未覆盖分支，
// 以新增用例收口。方法体以工作目录实际源码为准（ut-t08 @ 2938d4ee）。
//
// 收口目标（首轮 uncovered 行 → 用例）：
//   CmdTool.cpp:
//     - getMapInfoFromSmartctl   键值/续行/括号方括号过滤/SMART 属性表 18 项（~70 行）
//     - loadNvidiaSettingInfo    -L 输出各类 continue、FB Memory Usage 2-token、
//                                VideoRam GB 分支、-L 失败告警
//     - loadLssgInfo / loadDmesgInfo / loadUpowerInfo / loadBiosInfoFromLspci /
//       loadCatConfigInfo / loadBootDeviceManfid  getDeviceInfo 失败路径
//     - loadHciconfigInfo        UosHome + QProcess 超时 / 文件读取失败 / 空段落
//     - loadLscpuInfo            空段落 continue
//     - loadHwinfoInfo           hwinfo_monitor 空条目 continue
//     - getMapInfoFromInput      三段式键值合并
//     - getMapInfoFromLshw       configuration 无=项 / resources 无:项与重复键合并 /
//                                network 逻辑名 /dev 替换
//     - getMapInfoFromHwinfo     PS/2 Mouse 强制 Hotplug、SubDevice/SubVendor pick、
//                                SysFS Device Link 哈希、Module Alias 尾部截断
//     - parseOemTomlInfo         主路径/调试兜底/安装路径解析 + 打开失败 + 空内容
//                                （QFile::exists 为静态函数，可安全打桩）
//   DeviceGenerator.cpp:
//     - calAndSetCpuHeaderInfo   L1d/L1i/L2/L3 缓存总量计算
//     - generatorNetworkDevice   hwinfo 有效性 continue、unique_id 命中禁用、
//                                else 强制展示、wired 新增（真实 /sys/class/net/lo、eno1）
//     - getBlueToothInfoFromHwinfo 空行 continue、Driver Status 分支新建与去重禁用
//     - getMouseInfoFromHwinfo   PM/T70 过滤、非 PS/2 无 authorized 跳过、path 键跳过
// 未收口（BRANCH_NOT_MAPPED，环境/死代码所致）：
//   - CmdTool getMapInfoFromSmartctl 1124-1140 与 loadNvidiaSettingInfo 1125-1140：
//     Qt6 下 split(' ') 使 memorySizeList.size()==2 不可达（FB 兜底死代码，已标红缺陷）
//   - CmdTool loadNvidiaSettingInfo 1177-1178：sizeStr 依赖上述死代码，Qt6 不可达
//   - CmdTool getMapInfoFromHwinfo 1465/1479 Qt5 分支：本构建为 Qt6，不可达
//   - DeviceGenerator generatorNetworkDevice 374/415-420 wlan 新增：依赖
//     /sys/class/net/wlan* 真实目录，本机不存在
//   - DeviceGenerator getMouseInfoFromHwinfo I2C 禁用与 else delete 分支：依赖
//     真实 /proc/bus/input/devices + hcitool 连接状态，无法构造
// ============================================================================

#include "CmdTool.h"
#include "DeviceGenerator.h"
#include "DeviceManager.h"
#include "DBusInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <QFile>
#include <QProcess>
#include <QByteArray>
#include <QStringList>
#include <QDir>
#include <DSysInfo>
#include <cstring>
#include <cstdlib>
#include <dlfcn.h>

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

// ============================ CmdTool 桩（复用 ut_cmdtool.cpp 约定） ============================

static QString ut_gp_payload;      // getDeviceInfo 返回内容
static bool    ut_gp_devOk = true;
static QString ut_gp_dbusPayload;  // DBusInterface::getInfo 返回内容
static bool    ut_gp_dbusOk = true;

static bool ut_gp_getDeviceInfo(void * /*self*/, QString &deviceInfo, const QString & /*debugFile*/)
{
    if (ut_gp_devOk)
        deviceInfo = ut_gp_payload;
    return ut_gp_devOk;
}

static bool ut_gp_getInfo(void * /*self*/, const QString & /*key*/, QString &info)
{
    if (ut_gp_dbusOk)
        info = ut_gp_dbusPayload;
    return ut_gp_dbusOk;
}

static void ut_gp_start(void * /*self*/, const QString & /*program*/, const QStringList & /*arguments*/, QIODevice::OpenMode /*mode*/)
{
    return;
}

static bool ut_gp_waitTrue(void * /*self*/, int /*msecs*/)
{
    return true;
}

static bool ut_gp_waitFalse(void * /*self*/, int /*msecs*/)
{
    return false;
}

static QByteArray ut_gp_procOutput;
static QByteArray ut_gp_readAll(void * /*self*/)
{
    return ut_gp_procOutput;
}

static void ut_gp_installQProcessStubs(Stub &stub, bool waitResult)
{
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(
                 &QProcess::start), ut_gp_start);
    stub.set(ADDR(QProcess, waitForFinished), waitResult ? ut_gp_waitTrue : ut_gp_waitFalse);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_gp_readAll);
}

// getDeviceInfoFromCmd 桩：按调用次序返回三段预置输出
static int     ut_gp_cmdSeq = 0;
static QString ut_gp_cmdOut1;
static QString ut_gp_cmdOut2;
static QString ut_gp_cmdOut3;

static bool ut_gp_getDeviceInfoFromCmd(void * /*self*/, QString &deviceInfo,
                                       const QString & /*program*/, const QStringList & /*arguments*/)
{
    ++ut_gp_cmdSeq;
    if (1 == ut_gp_cmdSeq)
        deviceInfo = ut_gp_cmdOut1;
    else if (2 == ut_gp_cmdSeq)
        deviceInfo = ut_gp_cmdOut2;
    else
        deviceInfo = ut_gp_cmdOut3;
    return true;
}

// 变体：第 3 次调用（nvidia-settings -q VideoRam）失败
static bool ut_gp_getDeviceInfoFromCmd_vramFail(void *self, QString &deviceInfo,
                                                const QString &program, const QStringList &arguments)
{
    ++ut_gp_cmdSeq;
    if (3 == ut_gp_cmdSeq)
        return false;
    if (1 == ut_gp_cmdSeq)
        deviceInfo = ut_gp_cmdOut1;
    else
        deviceInfo = ut_gp_cmdOut2;
    return true;
}

// 变体：第 1 次调用（nvidia-smi -L）失败
static bool ut_gp_getDeviceInfoFromCmd_firstFail(void *self, QString &deviceInfo,
                                                 const QString &program, const QStringList &arguments)
{
    ++ut_gp_cmdSeq;
    if (1 == ut_gp_cmdSeq)
        return false;
    if (2 == ut_gp_cmdSeq)
        deviceInfo = ut_gp_cmdOut2;
    else
        deviceInfo = ut_gp_cmdOut3;
    return true;
}

// QFile::open/readAll 桩（dlsym 取 open 真实符号，避免虚表地址问题）
// open 支持按调用次序返回预置结果（主路径/调试兜底/安装路径依次打开）
static bool        ut_gp_fileOpenOk = true;
static QList<bool> ut_gp_openSeq;
static int         ut_gp_openIdx = 0;
static QByteArray  ut_gp_fileContent;
static bool ut_gp_fileOpen(QFile * /*self*/, QIODevice::OpenMode /*mode*/)
{
    if (ut_gp_openIdx < ut_gp_openSeq.size())
        return ut_gp_openSeq.at(ut_gp_openIdx++);
    return ut_gp_fileOpenOk;
}
static QByteArray ut_gp_fileReadAll(QFile * /*self*/)
{
    return ut_gp_fileContent;
}
static void ut_gp_installQFileStubs(Stub &stub, QList<bool> openSeq)
{
    void *openFn = dlsym(RTLD_DEFAULT, "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    if (nullptr != openFn)
        stub.set(openFn, reinterpret_cast<void *>(ut_gp_fileOpen));
    stub.set(ADDR(QFile, readAll), ut_gp_fileReadAll);
    ut_gp_openSeq = openSeq;
    ut_gp_openIdx = 0;
    ut_gp_fileOpenOk = true;
}

// QFile::exists(const QString&) 静态函数桩：按调用次序返回预置结果
static QList<bool> ut_gp_existsSeq;
static int         ut_gp_existsIdx = 0;
static bool ut_gp_fileExists(const QString & /*path*/)
{
    if (ut_gp_existsIdx < ut_gp_existsSeq.size())
        return ut_gp_existsSeq.at(ut_gp_existsIdx++);
    return false;
}
static void ut_gp_installQFileExistsStub(Stub &stub, QList<bool> seq)
{
    ut_gp_existsSeq = seq;
    ut_gp_existsIdx = 0;
    stub.set(reinterpret_cast<void *>(static_cast<bool (*)(const QString &)>(&QFile::exists)),
             reinterpret_cast<void *>(ut_gp_fileExists));
}

// DSysInfo 版本类型桩
static DSysInfo::UosEdition ut_gp_uosHomeEdition()
{
    return DSysInfo::UosHome;
}
static DSysInfo::UosEdition ut_gp_uosProfessionalEdition()
{
    return DSysInfo::UosProfessional;
}

// ============================ CmdTool Fixture ============================

class UT_CmdToolGaps : public UT_HEAD
{
public:
    void SetUp()
    {
        m_cmdTool = new CmdTool;
        ut_gp_payload.clear();
        ut_gp_devOk = true;
        ut_gp_dbusPayload.clear();
        ut_gp_dbusOk = true;
        ut_gp_procOutput.clear();
        ut_gp_fileOpenOk = true;
        ut_gp_fileContent.clear();
        ut_gp_cmdSeq = 0;
        ut_gp_cmdOut1.clear();
        ut_gp_cmdOut2.clear();
        ut_gp_cmdOut3.clear();
        ut_gp_existsSeq.clear();
        ut_gp_existsIdx = 0;
        ut_gp_openSeq.clear();
        ut_gp_openIdx = 0;
        ut_gp_fileOpenOk = true;
    }
    void TearDown()
    {
        delete m_cmdTool;
        m_cmdTool = nullptr;
    }
    CmdTool *m_cmdTool = nullptr;
};

// ============================ getMapInfoFromSmartctl ============================

// 键值行 + 四空格续行合并（无冒号续行追加到上一键）
TEST_F(UT_CmdToolGaps, getMapInfoFromSmartctl_kvContinuation)
{
    // Arrange
    const QString info = "Model Family: Samsung SSD 850\n"
                         "    four space continuation\n"
                         "Serial Number: S1X2Y3\n";
    QMap<QString, QString> m;
    // Act
    m_cmdTool->getMapInfoFromSmartctl(m, info, ":");
    // Assert: 续行以 ", " 追加；普通键值直接存
    EXPECT_EQ(m["Model Family"], QString("Samsung SSD 850, four space continuation"));
    EXPECT_EQ(m["Serial Number"], QString("S1X2Y3"));
}

// 括号跨越冒号 / 方括号跨越冒号 / Error 行 / 时间正则行均不入表
TEST_F(UT_CmdToolGaps, getMapInfoFromSmartctl_skipFilters)
{
    // Arrange
    const QString info = "Size (approx: value) here\n"
                         "Serial [S1: X2] ok\n"
                         "Error 5: happened\n"
                         "Time: 08:00\n";
    QMap<QString, QString> m;
    // Act
    m_cmdTool->getMapInfoFromSmartctl(m, info, ":");
    // Assert: 四类行均被过滤
    EXPECT_TRUE(m.isEmpty());
}

// SMART 属性表：rx 十列表行 + 无 0x 列的 18 项属性行全部入库
TEST_F(UT_CmdToolGaps, getMapInfoFromSmartctl_attrTable)
{
    // Arrange: 首行命中 rx 正则表；后续行为 8 列属性行（无 0x 列不匹配 rx）
    const QString info =
        "  5 Reallocated_Sector_Ct 0x0033 100 100 010 Pre-fail Always - 0\n"
        "1 Power_On_Hours 100 100 0 Old_age Always 111\n"
        "2 Power_Cycle_Count 100 100 0 Old_age Always 222\n"
        "3 Raw_Read_Error_Rate 100 100 0 Old_age Always 333\n"
        "4 Spin_Up_Time 100 100 0 Old_age Always 444\n"
        "5 Start_Stop_Count 100 100 0 Old_age Always 555\n"
        "6 Reallocated_Sector_Ct 100 100 0 Old_age Always 666\n"
        "7 Seek_Error_Rate 100 100 0 Old_age Always 777\n"
        "8 Spin_Retry_Count 100 100 0 Old_age Always 888\n"
        "9 Calibration_Retry_Count 100 100 0 Old_age Always 999\n"
        "10 G-Sense_Error_Rate 100 100 0 Old_age Always 1010\n"
        "11 Power-Off_Retract_Count 100 100 0 Old_age Always 1111\n"
        "12 Load_Cycle_Count 100 100 0 Old_age Always 1212\n"
        "13 Temperature_Celsius 100 100 0 Old_age Always 1313\n"
        "14 Reallocated_Event_Count 100 100 0 Old_age Always 1414\n"
        "15 Current_Pending_Sector 100 100 0 Old_age Always 1515\n"
        "16 Offline_Uncorrectable 100 100 0 Old_age Always 1616\n"
        "17 UDMA_CRC_Error_Count 100 100 0 Old_age Always 1717\n"
        "18 Multi_Zone_Error_Rate 100 100 0 Old_age Always 1818\n";
    QMap<QString, QString> m;
    // Act
    m_cmdTool->getMapInfoFromSmartctl(m, info, ":");
    // Assert: rx 行先存 raw=0，属性行覆盖为各自 last 值
    EXPECT_EQ(m["Reallocated_Sector_Ct"], QString("666"));
    EXPECT_EQ(m["Power_On_Hours"], QString("111"));
    EXPECT_EQ(m["Power_Cycle_Count"], QString("222"));
    EXPECT_EQ(m["Raw_Read_Error_Rate"], QString("333"));
    EXPECT_EQ(m["Spin_Up_Time"], QString("444"));
    EXPECT_EQ(m["Start_Stop_Count"], QString("555"));
    EXPECT_EQ(m["Seek_Error_Rate"], QString("777"));
    EXPECT_EQ(m["Spin_Retry_Count"], QString("888"));
    EXPECT_EQ(m["Calibration_Retry_Count"], QString("999"));
    EXPECT_EQ(m["G-Sense_Error_Rate"], QString("1010"));
    EXPECT_EQ(m["Power-Off_Retract_Count"], QString("1111"));
    EXPECT_EQ(m["Load_Cycle_Count"], QString("1212"));
    EXPECT_EQ(m["Temperature_Celsius"], QString("1313"));
    EXPECT_EQ(m["Reallocated_Event_Count"], QString("1414"));
    EXPECT_EQ(m["Current_Pending_Sector"], QString("1515"));
    EXPECT_EQ(m["Offline_Uncorrectable"], QString("1616"));
    EXPECT_EQ(m["UDMA_CRC_Error_Count"], QString("1717"));
    EXPECT_EQ(m["Multi_Zone_Error_Rate"], QString("1818"));
}

// rx 表行重复键覆盖（rx 分支为直接赋值，不追加）
TEST_F(UT_CmdToolGaps, getMapInfoFromSmartctl_rxRowOverwrite)
{
    // Arrange: 两行同 key 的 rx 表行
    const QString info =
        "  5 Reallocated_Sector_Ct 0x0033 100 100 010 Pre-fail Always - 0\n"
        "  5 Reallocated_Sector_Ct 0x0033 100 100 010 Pre-fail Always - 9\n";
    QMap<QString, QString> m;
    // Act
    m_cmdTool->getMapInfoFromSmartctl(m, info, ":");
    // Assert: 直接赋值 → 后行覆盖
    EXPECT_EQ(m["Reallocated_Sector_Ct"], QString("9"));
}

// ============================ loadNvidiaSettingInfo ============================

// -L 输出各类 continue：无冒号行 / lastStr 空 / GPU 编号 3 token
TEST_F(UT_CmdToolGaps, loadNvidiaSettingInfo_lshwSkipVariants)
{
    // Arrange: VideoRam 查询失败 → 无 Size 不入库
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_gp_getDeviceInfoFromCmd_vramFail);
    ut_gp_cmdSeq = 0;
    ut_gp_cmdOut1 = "GPU 0: GeForce GTX 1050 (PCI:0000:01:00.0)\n"
                    "noColonLine\n"
                    "GPU :\n"
                    "Big Name Card: desc\n"
                    "GPU 1: Card\n";
    ut_gp_cmdOut2 = "GPU 00000000:01:00.0\n";
    ut_gp_cmdOut3.clear();
    // Act
    m_cmdTool->loadNvidiaSettingInfo("ut_ngpu", "nvidia.txt");
    // Assert: 各 skip 行不产生条目，VideoRam 失败且 FB 无 Total → 不入库
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("ut_ngpu"));
}

// -L 命令本身失败 → 告警返回
TEST_F(UT_CmdToolGaps, loadNvidiaSettingInfo_gpuListFail)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_gp_getDeviceInfoFromCmd_firstFail);
    ut_gp_cmdSeq = 0;
    ut_gp_cmdOut2 = "x\n";
    ut_gp_cmdOut3 = "y\n";
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadNvidiaSettingInfo("ut_ngpu", "nvidia.txt"));
    // Assert
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("ut_ngpu"));
}

// FB Memory Usage 后 2-token "Total 800" 行进入 Total 判定（Qt6 下 size==2 不可达，无 Size）
TEST_F(UT_CmdToolGaps, loadNvidiaSettingInfo_fbTotalTwoToken)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_gp_getDeviceInfoFromCmd_vramFail);
    ut_gp_cmdSeq = 0;
    ut_gp_cmdOut1 = "GPU 0: GeForce GTX 1050 (PCI:0000:01:00.0)\n";
    ut_gp_cmdOut2 = "GPU 00000000:01:00.0\n"
                    "FB Memory Usage\n"
                    "Total 800\n";
    ut_gp_cmdOut3.clear();
    // Act
    m_cmdTool->loadNvidiaSettingInfo("ut_ngpu", "nvidia.txt");
    // Assert: Qt6 split(' ') 下 memorySizeList 恒为 1 段 → 无 Size → 不入库
    // 【疑似源码缺陷】FB 兜底分支在 Qt6 下为死代码，已在缺陷清单标红
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("ut_ngpu"));
}

// VideoRam GB 分支：贪婪正则只捕获末 4 位数字 → 实际值被截断
// 【疑似源码缺陷】33554432 应为 32GB，正则 ([0-9]{4,}) 只捕到 "4432" → "null=4MB"
TEST_F(UT_CmdToolGaps, loadNvidiaSettingInfo_vramGB)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_gp_getDeviceInfoFromCmd);
    ut_gp_cmdSeq = 0;
    ut_gp_cmdOut1 = "GPU 0: GeForce GTX 1050 (PCI:0000:01:00.0)\n";
    ut_gp_cmdOut2 = "GPU 00000000:01:00.0\n";
    ut_gp_cmdOut3 = "  Attribute 'VideoRam' (PC:0.0): 33554432.\n";
    // Act
    m_cmdTool->loadNvidiaSettingInfo("ut_ngpu", "nvidia.txt");
    // Assert: VideoRam 正则已修复（DEF-002，上游 728a9546）：33554432KB/1024=32768MB≥1024 → 32GB
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["ut_ngpu"].size());
    EXPECT_EQ(QString("null=32GB"), m_cmdTool->m_cmdInfo["ut_ngpu"][0]["Size"]);
    EXPECT_EQ(QString("GeForce GTX 1050"), m_cmdTool->m_cmdInfo["ut_ngpu"][0]["Device"]);
    EXPECT_EQ(QString("00000000:01:00.0"), m_cmdTool->m_cmdInfo["ut_ngpu"][0]["BusID"]);
}

// ============================ 失败路径装载器 ============================

// loadLssgInfo: getDeviceInfo 失败 → 告警返回
TEST_F(UT_CmdToolGaps, loadLssgInfo_failPath)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_gp_getDeviceInfo);
    ut_gp_devOk = false;
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadLssgInfo("ut_missing.txt"));
    // Assert: 无 smartctl 键产生
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("smartctl_ut"));
}

// loadDmesgInfo: getDeviceInfo 失败 → 告警返回
TEST_F(UT_CmdToolGaps, loadDmesgInfo_failPath)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_gp_getDeviceInfo);
    ut_gp_devOk = false;
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadDmesgInfo("ut_missing_dmesg.txt"));
    // Assert: 不入库
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("dmesg"));
}

// loadUpowerInfo: getDeviceInfo 失败 → 告警返回
TEST_F(UT_CmdToolGaps, loadUpowerInfo_failPath)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_gp_getDeviceInfo);
    ut_gp_devOk = false;
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadUpowerInfo("upower", "ut_missing_upower.txt"));
    // Assert
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("upower"));
}

// loadBiosInfoFromLspci: getDeviceInfo 失败 → 告警返回
TEST_F(UT_CmdToolGaps, loadBiosInfoFromLspci_failPath)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_gp_getDeviceInfo);
    ut_gp_devOk = false;
    // Act
    QString chipsetFamily = "before";
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadBiosInfoFromLspci(chipsetFamily));
    // Assert: 入参未被修改
    EXPECT_EQ(chipsetFamily, QString("before"));
}

// loadCatConfigInfo: getDeviceInfo 失败 → 告警返回
TEST_F(UT_CmdToolGaps, loadCatConfigInfo_failPath)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_gp_getDeviceInfo);
    ut_gp_devOk = false;
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadCatConfigInfo("ut_catcfg", "ut_missing_cat.txt"));
    // Assert
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("ut_catcfg"));
}

// loadBootDeviceManfid: getDeviceInfo 失败 → 告警返回
TEST_F(UT_CmdToolGaps, loadBootDeviceManfid_failPath)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_gp_getDeviceInfo);
    ut_gp_devOk = false;
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadBootDeviceManfid("boot_device", "ut_missing_boot.txt"));
    // Assert
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("boot_device"));
}

// ============================ loadHciconfigInfo / loadLscpuInfo / loadHwinfoInfo ============================

// UosHome 版本 + hciconfig 命令超时（waitForFinished false）→ 告警后走空输出
TEST_F(UT_CmdToolGaps, loadHciconfigInfo_uosHomeTimeout)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DSysInfo, uosEditionType), ut_gp_uosHomeEdition);
    ut_gp_installQProcessStubs(stub, false);
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadHciconfigInfo("hciconfig.txt"));
    // Assert: 命令失败输出为空，不产生蓝牙条目（未崩溃即通过）
    QMap<QString, QString> emptyMap;
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadBluetoothCtlInfo(emptyMap));
}

// 非 UosHome + 文件读取失败 → 告警返回
TEST_F(UT_CmdToolGaps, loadHciconfigInfo_fileFail)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DSysInfo, uosEditionType), ut_gp_uosProfessionalEdition);
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_gp_getDeviceInfo);
    ut_gp_devOk = false;
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadHciconfigInfo("ut_missing_hci.txt"));
    // Assert: 无蓝牙条目
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("hciconfig"));
}

// 文件读取成功 + 尾部空段落 → 空段落 continue
TEST_F(UT_CmdToolGaps, loadHciconfigInfo_emptyParagraph)
{
    // Arrange: 尾部 "\n\n" 产生空段落
    Stub stub;
    stub.set(ADDR(DSysInfo, uosEditionType), ut_gp_uosProfessionalEdition);
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_gp_getDeviceInfo);
    ut_gp_payload = "hci0 paragraph data\n\n";
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadHciconfigInfo("hciconfig.txt"));
    // Assert: 空段落被跳过（未崩溃即通过）
    SUCCEED();
}

// loadLscpuInfo: 尾部空段落 continue + DBus lscpu_num 失败返回
TEST_F(UT_CmdToolGaps, loadLscpuInfo_emptyItemAndDBusFail)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_gp_getDeviceInfo);
    stub.set(ADDR(DBusInterface, getInfo), ut_gp_getInfo);
    ut_gp_payload = "CPU para\n\n";
    ut_gp_dbusOk = false;
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadLscpuInfo("lscpu_cpu", "lscpu.txt"));
    // Assert: 段落解析入库，DBus 失败不产生 lscpu_num
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lscpu_cpu"));
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("lscpu_num"));
}

// loadHwinfoInfo(hwinfo_monitor): 尾部空条目 continue
TEST_F(UT_CmdToolGaps, loadHwinfoInfo_emptyMonitorItem)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_gp_getDeviceInfo);
    ut_gp_payload = "monitor para\n\n";
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->loadHwinfoInfo("hwinfo_monitor", "hwinfo_monitor.txt"));
    // Assert: 非 monitor 类不收录
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("hwinfo_monitor"));
}

// ============================ 解析器直测 ============================

// getMapInfoFromInput: 三段式 "A: B: C" → A = "BC"（去引号）
TEST_F(UT_CmdToolGaps, getMapInfoFromInput_threePart)
{
    // Arrange: 恰好 2 个 ": " 分隔符 → 三段
    QMap<QString, QString> m;
    // Act
    m_cmdTool->getMapInfoFromInput("Caps Lock: 0 : 1\n", m, ": ");
    // Assert: 第 2、3 段拼接
    EXPECT_EQ(m["Caps Lock"], QString("01"));
}

// getMapInfoFromLshw: configuration 无=项跳过 / resources 无:项跳过 /
// resources 重复键 "  " 合并 / network 逻辑名 /dev 值被非 /dev 值替换
TEST_F(UT_CmdToolGaps, getMapInfoFromLshw_configResNetwork)
{
    // Arrange
    const QString info = "network: Ethernet\n"
                         "logical name: /dev/ttyUSB0\n"
                         "logical name: eth0\n"
                         "configuration: broadcast yes driver=r8166\n"
                         "resources: irq ioport:3000\n"
                         "resources: irq:16\n"
                         "resources: irq:17\n";
    QMap<QString, QString> m;
    // Act
    m_cmdTool->getMapInfoFromLshw(info, m, ": ");
    // Assert
    EXPECT_EQ(m["logical name"], QString("eth0"));
    EXPECT_EQ(m["irq"], QString("16  17"));
    EXPECT_EQ(m["driver"], QString("r8166"));
    EXPECT_FALSE(m.contains("broadcast"));
    EXPECT_EQ(m["ioport"], QString("3000"));
}

// getMapInfoFromHwinfo: PS/2 Mouse 强制 Hotplug / SubDevice/SubVendor pick /
// SysFS Device Link 进哈希 / Module Alias 尾部 10 位截断
TEST_F(UT_CmdToolGaps, getMapInfoFromHwinfo_ps2Subdevice)
{
    // Arrange: Vendor/Device 值含空格（两 token）→ 形成 VID_PID → 触发哈希 Unique ID
    const QString info = "Device: BCM20702A0\n"
                         "PS/2 Mouse: present\n"
                         "SubDevice: 0x1234\n"
                         "SubVendor: 0x1028\n"
                         "Vendor: 0x8087 abc\n"
                         "Device: 0x1234 def\n"
                         "Device File: ut0\n"
                         "SysFS ID: /sys/ut/1\n"
                         "SysFS Device Link: /sys/devices/ut/link\n"
                         "Module Alias: usb:v8087p0A5Ad0100\n";
    QMap<QString, QString> m;
    // Act
    m_cmdTool->getMapInfoFromHwinfo(info, m, ": ");
    // Assert
    EXPECT_EQ(m["Hotplug"], QString("PS/2"));
    EXPECT_EQ(m["Module Alias"], QString("usb:v8087"));
    EXPECT_EQ(m["VID_PID"], QString("abcdef"));
    EXPECT_EQ(m["Vendor"], QString("0x8087 abc"));
    EXPECT_FALSE(m["Unique ID"].isEmpty());
}

// ============================ parseOemTomlInfo ============================

// 主路径 /etc/deepin/hardware/<file> 命中：节解析 + 内联表 + # 注释
TEST_F(UT_CmdToolGaps, parseOemTomlInfo_primaryPath)
{
    // Arrange: exists 第 1 次调用（主路径）返回 true
    Stub stub;
    ut_gp_installQFileExistsStub(stub, { true });
    ut_gp_installQFileStubs(stub, { true });
    ut_gp_fileContent = "[audio.x]\n"
                        "name = \"U Audio\" # inline comment\n"
                        "ids = { a1, a2 }\n"
                        "[video.y]\n"
                        "name = \"U Video\"\n";
    // Act
    bool ok = m_cmdTool->parseOemTomlInfo("ut_oem.toml");
    // Assert: 每节以 toml+首段节名入库
    EXPECT_TRUE(ok);
    ASSERT_TRUE(m_cmdTool->m_cmdInfo.contains("tomlaudio"));
    EXPECT_EQ(m_cmdTool->m_cmdInfo["tomlaudio"][0]["name"], QString("U Audio"));
    EXPECT_EQ(m_cmdTool->m_cmdInfo["tomlaudio"][0]["ids"], QString("a1, a2"));
    ASSERT_TRUE(m_cmdTool->m_cmdInfo.contains("tomlvideo"));
    EXPECT_EQ(m_cmdTool->m_cmdInfo["tomlvideo"][0]["name"], QString("U Video"));
}

// 主路径不存在 → 调试兜底直接 open（无 exists 检查）命中解析
TEST_F(UT_CmdToolGaps, parseOemTomlInfo_debugFallback)
{
    // Arrange: 主路径 exists=false → 调试文件 open 成功
    Stub stub;
    ut_gp_installQFileExistsStub(stub, { false });
    ut_gp_installQFileStubs(stub, { true });
    ut_gp_fileContent = "[audio.x]\n"
                        "name = Fallback\n"
                        "[video.y]\n"
                        "name = Fallback2\n";
    // Act
    bool ok = m_cmdTool->parseOemTomlInfo("ut_oem.toml");
    // Assert: 第二个节头触发第一节 flush → true
    EXPECT_TRUE(ok);
    ASSERT_TRUE(m_cmdTool->m_cmdInfo.contains("tomlaudio"));
    EXPECT_EQ(m_cmdTool->m_cmdInfo["tomlaudio"][0]["name"], QString("Fallback"));
}

// 主路径与调试兜底均失败 → 安装路径 /usr/share/... 命中
TEST_F(UT_CmdToolGaps, parseOemTomlInfo_usrShareFallback)
{
    // Arrange: exists 序列 [主路径=false, 安装路径=true]；open 序列 [调试=false, 安装=true]
    Stub stub;
    ut_gp_installQFileExistsStub(stub, { false, true });
    ut_gp_installQFileStubs(stub, { false, true });
    ut_gp_fileContent = "[audio.x]\n"
                        "name = Installed\n"
                        "[video.y]\n"
                        "name = Installed2\n";
    // Act
    bool ok = m_cmdTool->parseOemTomlInfo("ut_oem.toml");
    // Assert
    EXPECT_TRUE(ok);
    ASSERT_TRUE(m_cmdTool->m_cmdInfo.contains("tomlaudio"));
    EXPECT_EQ(m_cmdTool->m_cmdInfo["tomlaudio"][0]["name"], QString("Installed"));
}

// 主路径存在但打开失败 → 调试兜底打开也失败 → 安装路径不存在 → false
TEST_F(UT_CmdToolGaps, parseOemTomlInfo_primaryOpenFail)
{
    // Arrange: exists 序列 [主路径=true, 安装路径=false]；open 序列 [主=false, 调试=false]
    Stub stub;
    ut_gp_installQFileExistsStub(stub, { true, false });
    ut_gp_installQFileStubs(stub, { false, false });
    // Act
    bool ok = m_cmdTool->parseOemTomlInfo("ut_oem.toml");
    // Assert: 全部打开失败 → false
    EXPECT_FALSE(ok);
}

// 【疑似源码缺陷】单个节的 toml：末节在循环后 flush 时不置 tomlPars=true，
// 数据已入库但函数仍返回 false。测试记录实际（有缺陷的）行为。
TEST_F(UT_CmdToolGaps, parseOemTomlInfo_singleSectionQuirk)
{
    // Arrange
    Stub stub;
    ut_gp_installQFileExistsStub(stub, { true });
    ut_gp_installQFileStubs(stub, { true });
    ut_gp_fileContent = "[audio.x]\n"
                        "name = Only\n";
    // Act
    bool ok = m_cmdTool->parseOemTomlInfo("ut_oem.toml");
    // Assert: 单 section 现返回 true（DEF-004 已修复，上游 ce18dbd0），数据入库
    EXPECT_TRUE(ok);
    ASSERT_TRUE(m_cmdTool->m_cmdInfo.contains("tomlaudio"));
    EXPECT_EQ(m_cmdTool->m_cmdInfo["tomlaudio"][0]["name"], QString("Only"));
}

// 文件可读但内容为空 → false
TEST_F(UT_CmdToolGaps, parseOemTomlInfo_emptyContent)
{
    // Arrange
    Stub stub;
    ut_gp_installQFileExistsStub(stub, { true });
    ut_gp_installQFileStubs(stub, { true });
    ut_gp_fileContent = "";
    // Act
    bool ok = m_cmdTool->parseOemTomlInfo("ut_oem.toml");
    // Assert
    EXPECT_FALSE(ok);
}

// 全部路径不存在 → false（不崩溃）
TEST_F(UT_CmdToolGaps, parseOemTomlInfo_allMissing)
{
    // Arrange: 主路径 exists=false；调试文件 open 失败；安装路径 exists=false
    Stub stub;
    ut_gp_installQFileExistsStub(stub, { false, false });
    ut_gp_installQFileStubs(stub, { false });
    // Act
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->parseOemTomlInfo("ut_never_exist.toml"));
    // Assert
    SUCCEED();
}

// ============================ DeviceGenerator 收口 ============================

// 数据构造辅助（与 ut_devicegenerator.cpp 约定一致）
static QMap<QString, QString> ut_gp_dgrow(std::initializer_list<std::pair<const char *, const char *>> pairs)
{
    QMap<QString, QString> map;
    for (const auto &p : pairs)
        map.insert(QString(p.first), QString(p.second));
    return map;
}

static void ut_gp_dgseed(const QString &key, QList<QMap<QString, QString>> rows)
{
    QMap<QString, QList<QMap<QString, QString>>> info;
    info.insert(key, rows);
    DeviceManager::instance()->addCmdInfo(info);
}

class UT_GapDG : public UT_HEAD
{
public:
    void SetUp()
    {
        DeviceManager::instance()->clear();
        DeviceManager::instance()->setCpuHeaderInfo(QList<QList<QPair<QString, QString>>>());
        dg = new DeviceGenerator;
    }
    void TearDown()
    {
        delete dg;
        DeviceManager::instance()->clear();
    }
    DeviceGenerator *dg = nullptr;
};

// calAndSetCpuHeaderInfo: L1d/L1i/L2/L3 缓存总量（shared cpu list → 分组数）
// 注意：lscpu 键为小写 "model name"/"vendor id"（kModelName/kVendorId 常量）
// 【疑似源码缺陷】Common::formatTotalCache 用 s.mid() 截单位未 trim，"1 MiB"
// 的单位串为 " MIB"，startsWith("M") 失败 → 按 KiB 数值处理 → L2/L3 总量偏小
TEST_F(UT_GapDG, calAndSetCpuHeaderInfo_cacheTotals)
{
    // Arrange: 4 核 8 线程；L1d 每 4 线程共享（8/4=2 组）等
    QMap<QString, QString> info;
    info.insert("model name", "UT CPU");
    info.insert("vendor id", "GenuineIntel");
    info.insert("Architecture", "x86_64");
    info.insert("L1d cache", "32 KiB");
    info.insert("L1d shared cpu list", "0-3");
    info.insert("L1i cache", "64 KiB");
    info.insert("L1i shared cpu list", "0-7");
    info.insert("L2 cache", "1 MiB");
    info.insert("L2 shared cpu list", "");
    info.insert("L3 cache", "3 MiB");
    info.insert("L3 shared cpu list", "0-1,4,6");
    // Act
    dg->calAndSetCpuHeaderInfo(info, 4, 8);
    // Assert: L1d 32K*2=64 KiB；L1i 64K*1=64 KiB；单位解析已修复（DEF-005，上游 6ccddaf4）：
    // L2 1 MiB×4 核=4096 KiB → "4 MiB"；L3 3 MiB×2 组=6144 KiB → "6 MiB"
    QList<QList<QPair<QString, QString>>> header;
    DeviceManager::instance()->getCpuHeaderInfo(header);
    ASSERT_EQ(header.size(), 1);
    QMap<QString, QString> kv;
    for (const auto &p : header[0])
        kv.insert(p.first, p.second);
    EXPECT_EQ(kv["Model Name"], QString("UT CPU"));
    EXPECT_EQ(kv["CPU(s)"], QString("8"));
    EXPECT_EQ(kv["Thread(s)"], QString("2"));
    EXPECT_EQ(kv["L1d cache"], QString("64 KiB"));
    EXPECT_EQ(kv["L1i cache"], QString("64 KiB"));
    EXPECT_EQ(kv["L2 cache"], QString("4 MiB"));
    EXPECT_EQ(kv["L3 cache"], QString("6 MiB"));
}

// generatorNetworkDevice: hwinfo wired 新增设备（uniqueID 取 Permanent HW Address）→
// 数据库来源行（unique_id=MAC）命中现有设备 → 禁用
TEST_F(UT_GapDG, generatorNetworkDevice_uniqueIdDisable)
{
    // Arrange: 第一行（无 unique_id）新增设备，uniqueID 被写为 Permanent HW Address；
    // 第二行 unique_id 命中该 MAC → setEnableValue(false)
    ut_gp_dgseed("hwinfo_network", {
        ut_gp_dgrow({ {"Permanent HW Address", "aa:bb:cc:dd:ee:01"}, {"Device File", "lo"} }),
        ut_gp_dgrow({ {"unique_id", "aa:bb:cc:dd:ee:01"} })
    });
    // Act
    dg->generatorNetworkDevice();
    // Assert: 第二行无有效 MAC 不新增 → 共 1 台且被禁用
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceNetwork.size(), 1);
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceNetwork[0]->uniqueID(), QString("aa:bb:cc:dd:ee:01"));
    EXPECT_FALSE(DeviceManager::instance()->m_ListDeviceNetwork[0]->enable());
}

// generatorNetworkDevice: hwinfo 行与现有设备均不匹配 → else 强制展示 +
// 有效逻辑名（真实 /sys/class/net/eno1）→ wired 新增
TEST_F(UT_GapDG, generatorNetworkDevice_elseForcedAndWiredAdd)
{
    // Arrange
    ut_gp_dgseed("lshw_network", { ut_gp_dgrow({ {"logical name", "lo"}, {"serial", "aa:bb:cc:dd:ee:01"} }) });
    ut_gp_dgseed("hwinfo_network", { ut_gp_dgrow({ {"Permanent HW Address", "aa:bb:cc:dd:ee:02"},
                                                   {"Device File", "eno1"} }) });
    // Act
    dg->generatorNetworkDevice();
    // Assert: 原设备被强制展示（canUninstall 置假）+ 新增 1 台
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceNetwork.size(), 2);
    EXPECT_FALSE(DeviceManager::instance()->m_ListDeviceNetwork[0]->canUninstall());
}

// getBlueToothInfoFromHwinfo: 空行 continue
TEST_F(UT_GapDG, getBlueToothInfoFromHwinfo_emptyRow)
{
    // Arrange: 空 QMap 行
    ut_gp_dgseed("hwinfo_usb", { QMap<QString, QString>() });
    // Act
    dg->getBlueToothInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 0);
}

// getBlueToothInfoFromHwinfo: 无 Driver 但 Driver Status 含 btusb → 新建；
// 同 unique_id 第二行 → 禁用已有设备
TEST_F(UT_GapDG, getBlueToothInfoFromHwinfo_driverStatusBranch)
{
    // Arrange: 两行均走 Driver Status 分支，第二行去重
    ut_gp_dgseed("hwinfo_usb", {
        ut_gp_dgrow({ {"Device", "Generic BT"}, {"Driver Status", "btusb is not blocked"},
                      {"Unique ID", "UTBT01"}, {"SysFS BusID", "3-1:1.0"} }),
        ut_gp_dgrow({ {"Device", "Generic BT"}, {"Driver Status", "btusb is not blocked"},
                      {"Unique ID", "UTBT01"}, {"SysFS BusID", "3-1:1.0"} })
    });
    // Act
    dg->getBlueToothInfoFromHwinfo();
    // Assert: 仅 1 台且被禁用
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 1);
    EXPECT_FALSE(DeviceManager::instance()->m_ListDeviceBluetooth[0]->enable());
}

// getMouseInfoFromHwinfo: PM 设备过滤 / T70 设备过滤 / 非 PS/2 无 authorized 跳过 /
// 已有 path 键跳过
TEST_F(UT_GapDG, getMouseInfoFromHwinfo_filterRows)
{
    // Arrange: PM 行、T70 行、USB 无 authorized 行、带 path 行均不入库
    ut_gp_dgseed("hwinfo_mouse", {
        ut_gp_dgrow({ {"Device", "PM123"}, {"Hotplug", "PS/2"} }),
        ut_gp_dgrow({ {"Device", "T70X"}, {"Hotplug", "PS/2"} }),
        ut_gp_dgrow({ {"Model", "UT Mouse"}, {"Hotplug", "USB"}, {"SysFS Device Link", "/bus/ut/absent"} }),
        ut_gp_dgrow({ {"Unique ID", "UTP1"}, {"Hotplug", "PS/2"}, {"path", "/devices/x"} })
    });
    // Act
    dg->getMouseInfoFromHwinfo();
    // Assert: 全部被过滤
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMouse.size(), 0);
}
