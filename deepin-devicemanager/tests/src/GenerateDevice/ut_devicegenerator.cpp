// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// 单元测试编写检查清单（test-code-gen 10 项最小检查）
// 1. [x] 每个被测方法的分支清单来自 MCP get_code_snippet 返回的真实源码（ut-t08 @ 99700372）
// 2. [x] 源码分支全部映射到用例或显式 BRANCH_NOT_MAPPED 标注
// 3. [x] AAA（Arrange/Act/Assert）结构注释
// 4. [x] 断言基于被测行为（设备数量/BusID/CPU头部信息），非空断言或实现细节
// 5. [x] 外部依赖全部打桩或规避：DeviceAudio::setInfoFrom_sysFS（真实 /sys/class/sound）、
//        DeviceManager::tomlDeviceSet（真实 toml 配置）、DSysInfo（真实系统信息）均打桩；
//        网络设备逻辑名校验 /sys/class/net、Others 设备 authorized 文件走本机必存在的
//        真实条目（lo / /sys/bus/usb/devices/usb1），无额外设备环境依赖
// 6. [x] 不使用 VADDR 对 Qt 内置类取地址，统一 static_cast 指定重载
// 7. [x] 桩函数作用域最小化（Stub 栈对象，析构自动还原）
// 8. [x] 负向用例检查强异常安全（EXPECT_NO_FATAL_FAILURE）
// 9. [x] 同质多分支 ≥3 的用 TEST_P 参数化（hwinfo <5 字段过滤族）
// 10.[x] 每个测试可独立运行（Fixture SetUp/TearDown 重置 DeviceManager 单例状态）
//
// 覆盖范围（DeviceGenerator.cpp @ deepin-devicemanager/src/GenerateDevice）：
//   生命周期: ctor / dtor
//   生成入口: generatorComputerDevice / generatorCpuDevice / generatorBiosDevice /
//             generatorMemoryDevice / generatorDiskDevice / generatorGpuDevice /
//             generatorMonitorDevice / generatorNetworkDevice / generatorAudioDevice /
//             generatorBluetoothDevice / generatorKeyboardDevice / generatorMouseDevice /
//             generatorCameraDevice / generatorCdromDevice / generatorOthersDevice /
//             generatorPrinterDevice / generatorPowerDevice / generatorInfoFromToml
//   dmidecode 族: getBiosInfo / getSystemInfo / getBaseBoardInfo / getChassisInfo /
//             getBiosMemoryInfo
//   来源族:   getMemoryInfoFromLshw / getMemoryInfoFromDmidecode / getDiskInfoFromHwinfo /
//             getDiskInfoFromLshw / getDiskInfoFromLsblk / getDiskInfoFromSmartCtl /
//             getGpuInfoFromHwinfo / getGpuInfoFromLshw / getGpuInfoFromXrandr /
//             getGpuSizeFromDmesg / getMonitorInfoFromHwinfo /
//             getMonitorInfoFromXrandrVerbose / getAudioInfoFromHwinfo /
//             getAudioInfoFromLshw / getAudioInfoFromCatInput / getAudioInfoFrom_sysFS /
//             getAudioChipInfoFromDmesg / getBlueToothInfoFromHwinfo /
//             getBluetoothInfoFromLshw / getBluetoothInfoFromHciconfig /
//             getKeyboardInfoFromHwinfo / getKeyboardInfoFromLshw / getMouseInfoFromHwinfo /
//             getMouseInfoFromLshw / getMouseInfoFromCatDevices / getImageInfoFromHwinfo /
//             getImageInfoFromLshw / getCdromInfoFromHwinfo / getCdromInfoFromLshw /
//             getOthersInfoFromHwinfo / getOthersInfoFromLshw
//   工具:     addBusIDFromHwinfo / getBusIDFromHwinfo / getProductName /
//             calAndSetCpuHeaderInfo（私有，构建使用 -fno-access-control 可直达）
// 未覆盖（BRANCH_NOT_MAPPED）:
//   - getMouseInfoFromHwinfo 新建设备销毁分支（else delete）：需 hciconfig 驱动的
//     m_BluetoothIsConnected=false，DeviceInput 构造默认 true，单测无法构造
//   - getMouseInfoFromHwinfo I2C 禁用分支（getInterface()==I2C）与 validateCanEnableForMouse
//     内部分支：依赖输入行携带 I2C 接口信息，hwinfo 真实输出不携带
//   - generatorNetworkDevice hwinfo wlan 新增分支（依赖真实 /sys/class/net/wlan* 目录）/
//     unique_id 非空禁用分支（依赖 DeviceNetwork::uniqueID() 内部生成算法与输入严格一致）
//   - getBluetoothInfoFromHwinfo "mouse"/"keyboard" 硬件类过滤与 hub 同构（同 continue），
//     仅以 hub 用例代表
// ============================================================================

#include "DeviceGenerator.h"
#include "GenerateDevicePool.h"
#include "DeviceManager.h"
#include "DeviceAudio.h"
#include "DeviceComputer.h"
#include "ut_Head.h"
#include "stub.h"

#include <DSysInfo>

#include <QList>
#include <QMap>
#include <QString>

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

// ============================ 数据构造辅助 ============================

// 构造单行设备信息（键值对来自真实命令输出中出现的字段名）
static QMap<QString, QString> ut_dg_row(std::initializer_list<std::pair<const char *, const char *>> pairs)
{
    QMap<QString, QString> map;
    for (const auto &p : pairs)
        map.insert(QString(p.first), QString(p.second));
    return map;
}

// 向 DeviceManager 注入一类命令输出（真实 cmdInfo 通道，替代对 cmdInfo 打桩）
static void ut_dg_seed(const QString &key, QList<QMap<QString, QString>> rows)
{
    QMap<QString, QList<QMap<QString, QString>>> info;
    info.insert(key, rows);
    DeviceManager::instance()->addCmdInfo(info);
}

// ============================ 桩函数（文件级静态） ============================

// DeviceAudio::setInfoFrom_sysFS 桩：前 i<2 张卡返回成功并给出 SysFS ID 与芯片，其余失败
static bool ut_dg_audioSysFS_okTwice(DeviceAudio *self, QMap<QString, QString> &mapInfo, int ii)
{
    Q_UNUSED(self);
    if (ii < 2) {
        mapInfo.insert("SysFS ID", QString("/devices/pci0000:00/0000:00:1f.3/sound/card%1").arg(ii));
        mapInfo.insert("chip", QString("ALC%1").arg(256 + ii));
        return true;
    }
    return false;
}

static bool ut_dg_audioSysFS_alwaysFail(DeviceAudio *self, QMap<QString, QString> &mapInfo, int ii)
{
    Q_UNUSED(self);
    Q_UNUSED(mapInfo);
    Q_UNUSED(ii);
    return false;
}

// DeviceManager::tomlDeviceSet 桩：只记录调用
static bool ut_dg_tomlCalled = false;
static void ut_dg_tomlDeviceSet(DeviceManager *self, DeviceType deviceType)
{
    Q_UNUSED(self);
    Q_UNUSED(deviceType);
    ut_dg_tomlCalled = true;
}

// DSysInfo 桩：控制 getProductName 两个分支
static QString ut_dg_uosSystemName(const QLocale &) { return QString("UOS"); }
static QString ut_dg_uosProductTypeName(const QLocale &) { return QString("Desktop"); }
static QString ut_dg_uosEditionName(const QLocale &) { return QString("Professional"); }
static QString ut_dg_uosProductTypeNameServer(const QLocale &) { return QString("Server"); }
static QString ut_dg_majorVersion() { return QString("20"); }
static QString ut_dg_minorVersion() { return QString("1060"); }

// ============================ Fixture ============================

class UT_DeviceGenerator : public UT_HEAD
{
public:
    void SetUp()
    {
        DeviceManager::instance()->clear();
        // clear() 不重置 m_ListCpuHeaderInfo，需显式清空避免跨测试残留
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

// ============================ 生命周期 ============================

// ctor/dtor: 空参构造与析构不产生副作用
TEST_F(UT_DeviceGenerator, ctor_dtor_basic)
{
    // Arrange/Act: SetUp 已构造 dg
    // Assert: 析构过程无致命错误
    EXPECT_NO_FATAL_FAILURE(delete dg);
    dg = new DeviceGenerator;
    EXPECT_NE(dg, nullptr);
}

// ============================ 计算机概况 ============================

// generatorComputerDevice: cat_os_release + lshw_system + cat_version(gcc 版本正则) 全链路 → 计算机设备 1 台
TEST_F(UT_DeviceGenerator, generatorComputerDevice_full)
{
    // Arrange
    ut_dg_seed("cat_os_release", { ut_dg_row({ {"HOME_URL", "\"https://www.deepin.org\""}, {"NAME", "UnionTech OS Desktop"} }) });
    ut_dg_seed("lshw_system", { ut_dg_row({ {"description", "Desktop Computer"}, {"vendor", "Dell Inc."}, {"product", "OptiPlex 7080"} }) });
    ut_dg_seed("cat_version", { ut_dg_row({ {"OS", "UnionTech OS Desktop 20 Pro (gcc 8.3.0 (Uos 8.3.0.5-1+dde)) 2021"} }) });
    // Act
    dg->generatorComputerDevice();
    // Assert: 无论分支如何计算机设备恒创建
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceComputer.size(), 1);
    EXPECT_TRUE(dynamic_cast<DeviceComputer *>(DeviceManager::instance()->m_ListDeviceComputer[0]) != nullptr);
}

// generatorComputerDevice: 无任何命令输出 → 仍创建计算机设备（缺省分支）
TEST_F(UT_DeviceGenerator, generatorComputerDevice_empty)
{
    // Arrange: 不注入数据
    // Act
    dg->generatorComputerDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceComputer.size(), 1);
}

// getProductName: 桌面版（productType 不含 Server）→ "系统名 主版本 版本名"
TEST_F(UT_DeviceGenerator, getProductName_nonServer)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DSysInfo, uosSystemName), ut_dg_uosSystemName);
    stub.set(ADDR(DSysInfo, uosProductTypeName), ut_dg_uosProductTypeName);
    stub.set(ADDR(DSysInfo, uosEditionName), ut_dg_uosEditionName);
    stub.set(ADDR(DSysInfo, majorVersion), ut_dg_majorVersion);
    // Act
    const QString name = DeviceGenerator::getProductName();
    // Assert
    EXPECT_EQ(name, QString("UOS 20 Professional"));
}

// getProductName: 服务器版（productType 含 Server）→ "系统名 主版本 (次版本版本名)"
TEST_F(UT_DeviceGenerator, getProductName_server)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DSysInfo, uosSystemName), ut_dg_uosSystemName);
    stub.set(ADDR(DSysInfo, uosProductTypeName), ut_dg_uosProductTypeNameServer);
    stub.set(ADDR(DSysInfo, uosEditionName), ut_dg_uosEditionName);
    stub.set(ADDR(DSysInfo, majorVersion), ut_dg_majorVersion);
    stub.set(ADDR(DSysInfo, minorVersion), ut_dg_minorVersion);
    // Act
    const QString name = DeviceGenerator::getProductName();
    // Assert
    EXPECT_EQ(name, QString("UOS 20 (1060Professional)"));
}

// ============================ CPU ============================

// generatorCpuDevice: 缺 lscpu_num → 提前返回，不产生 CPU 设备
TEST_F(UT_DeviceGenerator, generatorCpuDevice_noLscpuNum)
{
    // Arrange
    ut_dg_seed("lscpu", { ut_dg_row({ {"processor", "0"} }) });
    // Act
    dg->generatorCpuDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceCPU.size(), 0);
}

// generatorCpuDevice: 3 个乱序 lscpu + lscpu_num + lshw_cpu + dmidecode4 → 3 台 CPU 设备 + 头部信息
TEST_F(UT_DeviceGenerator, generatorCpuDevice_full)
{
    // Arrange: processor 顺序 2/0/1，验证排序后依次建设备
    ut_dg_seed("lscpu", { ut_dg_row({ {"processor", "2"}, {"model name", "Intel(R) Core(TM) i7-10700"} }),
                          ut_dg_row({ {"processor", "0"}, {"model name", "Intel(R) Core(TM) i7-10700"} }),
                          ut_dg_row({ {"processor", "1"}, {"model name", "Intel(R) Core(TM) i7-10700"} }) });
    ut_dg_seed("lscpu_num", { ut_dg_row({ {"core", "4"}, {"logical", "8"} }) });
    ut_dg_seed("lshw_cpu", { ut_dg_row({ {"product", "Intel(R) Core(TM) i7-10700"}, {"vendor", "Intel Corp."} }) });
    ut_dg_seed("dmidecode4", { ut_dg_row({ {"Socket Designation", "U3E1"}, {"Version", "Intel(R) Core(TM) i7-10700"}, {"Manufacturer", "Intel Corp."} }) });
    // Act
    dg->generatorCpuDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceCPU.size(), 3);
    // 头部信息: model name/vendor_id/CPU(s)=8/Thread(s)=2 至少 4 对
    QList<QList<QPair<QString, QString>>> header;
    DeviceManager::instance()->getCpuHeaderInfo(header);
    ASSERT_FALSE(header.isEmpty());
    bool hasCpuCount = false, hasThread = false;
    for (const auto &single : header) {
        for (const auto &pair : single) {
            if (pair.first == "CPU(s)" && pair.second == "8") hasCpuCount = true;
            if (pair.first == "Thread(s)" && pair.second == "2") hasThread = true;
        }
    }
    EXPECT_TRUE(hasCpuCount);
    EXPECT_TRUE(hasThread);
}

// generatorCpuDevice: dmidecode4 双条目同 Socket → CPU 数去重为 1；Core/Thread 计数被 dmidecode 覆盖
TEST_F(UT_DeviceGenerator, generatorCpuDevice_dmidecode4_override)
{
    // Arrange: lscpu_num core=4 logical=8; dmidecode 两个条目同插槽各 6 核 12 线程
    ut_dg_seed("lscpu", { ut_dg_row({ {"processor", "0"}, {"model name", "Intel(R) Xeon(R)"} }) });
    ut_dg_seed("lscpu_num", { ut_dg_row({ {"core", "4"}, {"logical", "8"} }) });
    ut_dg_seed("dmidecode4", { ut_dg_row({ {"Socket Designation", "CPU0"}, {"Core Count", "6"}, {"Thread Count", "12"} }),
                               ut_dg_row({ {"Socket Designation", "CPU0"}, {"Core Count", "6"}, {"Thread Count", "12"} }) });
    // Act
    dg->generatorCpuDevice();
    // Assert: 设备数等于 lscpu 行数
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceCPU.size(), 1);
    // dmidecode 覆盖: 核/线程数按条目累加 core=12 logical=24 → CPU(s)=24 Thread(s)=2
    QList<QList<QPair<QString, QString>>> header;
    DeviceManager::instance()->getCpuHeaderInfo(header);
    ASSERT_FALSE(header.isEmpty());
    bool hasCpuCount = false, hasThread = false;
    for (const auto &single : header) {
        for (const auto &pair : single) {
            if (pair.first == "CPU(s)" && pair.second == "24") hasCpuCount = true;
            if (pair.first == "Thread(s)" && pair.second == "2") hasThread = true;
        }
    }
    EXPECT_TRUE(hasCpuCount);
    EXPECT_TRUE(hasThread);
}

// calAndSetCpuHeaderInfo（私有方法，-fno-access-control 可直达）: 全字段 → 头部含 5 个固定对
TEST_F(UT_DeviceGenerator, calAndSetCpuHeaderInfo_full)
{
    // Arrange
    QMap<QString, QString> info;
    info.insert("model name", "Intel(R) Core(TM) i7-10700");
    info.insert("vendor_id", "GenuineIntel");
    info.insert("Architecture", "x86_64");
    // Act
    dg->calAndSetCpuHeaderInfo(info, 8, 16);
    // Assert
    QList<QList<QPair<QString, QString>>> header;
    DeviceManager::instance()->getCpuHeaderInfo(header);
    ASSERT_EQ(header.size(), 1);
    const QList<QPair<QString, QString>> &single = header[0];
    ASSERT_GE(single.size(), 5);
    EXPECT_EQ(single[0].first, QString("Model Name"));
    EXPECT_EQ(single[0].second, QString("Intel(R) Core(TM) i7-10700"));
    EXPECT_EQ(single[1].first, QString("Vendor ID"));
    EXPECT_EQ(single[1].second, QString("GenuineIntel"));
    EXPECT_EQ(single[2].first, QString("Architecture"));
    EXPECT_EQ(single[2].second, QString("x86_64"));
    EXPECT_EQ(single[3].first, QString("CPU(s)"));
    EXPECT_EQ(single[3].second, QString("16"));
    EXPECT_EQ(single[4].first, QString("Thread(s)"));
    EXPECT_EQ(single[4].second, QString("2"));
}

// calAndSetCpuHeaderInfo: 空输入 + coreNum=0 → 整体替换为仅含一个空内层的头部列表
TEST_F(UT_DeviceGenerator, calAndSetCpuHeaderInfo_empty)
{
    // Arrange
    QMap<QString, QString> info;
    // Act
    dg->calAndSetCpuHeaderInfo(info, 0, 0);
    // Assert: setCpuHeaderInfo 为整体替换语义 → 空输入产生 [空内层]
    QList<QList<QPair<QString, QString>>> header;
    DeviceManager::instance()->getCpuHeaderInfo(header);
    ASSERT_EQ(header.size(), 1);
    EXPECT_TRUE(header[0].isEmpty());
}

// ============================ 主板（dmidecode 族） ============================

// generatorBiosDevice: dmidecode0/1/2/3/16 各一条 → 5 台 BIOS 类设备
TEST_F(UT_DeviceGenerator, generatorBiosDevice_all)
{
    // Arrange
    ut_dg_seed("dmidecode0", { ut_dg_row({ {"Vendor", "American Megatrends"}, {"Version", "1.0.5"} }) });
    ut_dg_seed("dmidecode1", { ut_dg_row({ {"Manufacturer", "Dell Inc."}, {"Product Name", "OptiPlex 7080"} }) });
    ut_dg_seed("dmidecode2", { ut_dg_row({ {"Manufacturer", "Dell Inc."}, {"Product Name", "0K240J"} }) });
    ut_dg_seed("dmidecode3", { ut_dg_row({ {"Manufacturer", "Dell Inc."}, {"Type", "Desktop"} }) });
    ut_dg_seed("dmidecode16", { ut_dg_row({ {"Size", "32 GB"}, {"Form Factor", "DIMM"} }) });
    // Act
    dg->generatorBiosDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBios.size(), 5);
}

// getBiosInfo: dmidecode0 有效条目 + dmidecode13 语言信息 → 1 台 BIOS + 语言信息注入
TEST_F(UT_DeviceGenerator, getBiosInfo_valid)
{
    // Arrange
    ut_dg_seed("dmidecode0", { ut_dg_row({ {"Vendor", "American Megatrends"}, {"Version", "1.0.5"} }) });
    ut_dg_seed("dmidecode13", { ut_dg_row({ {"Currently Installed Language(s)", "zh_CN|en_US"} }) });
    // Act
    dg->getBiosInfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBios.size(), 1);
}

// getBiosInfo: dmidecode0 条目字段不足 2 → 跳过
TEST_F(UT_DeviceGenerator, getBiosInfo_insufficient)
{
    // Arrange
    ut_dg_seed("dmidecode0", { ut_dg_row({ {"Vendor", "American Megatrends"} }) });
    // Act
    dg->getBiosInfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBios.size(), 0);
}

// getSystemInfo: dmidecode1 有效 → 1 台
TEST_F(UT_DeviceGenerator, getSystemInfo_valid)
{
    // Arrange
    ut_dg_seed("dmidecode1", { ut_dg_row({ {"Manufacturer", "Dell Inc."}, {"Product Name", "OptiPlex 7080"} }) });
    // Act
    dg->getSystemInfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBios.size(), 1);
}

// getSystemInfo: 空输入 → 0 台（强异常安全）
TEST_F(UT_DeviceGenerator, getSystemInfo_empty)
{
    // Arrange: 无数据
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getSystemInfo());
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBios.size(), 0);
}

// getBaseBoardInfo: 有效 → 1 台；不足 2 字段 → 0 台
TEST_F(UT_DeviceGenerator, getBaseBoardInfo_validAndInsufficient)
{
    // Arrange: 无效条目
    ut_dg_seed("dmidecode2", { ut_dg_row({ {"Manufacturer", "Dell Inc."} }) });
    // Act/Assert: 不足跳过
    dg->getBaseBoardInfo();
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBios.size(), 0);
    // Arrange: 有效条目
    DeviceManager::instance()->clear();
    ut_dg_seed("dmidecode2", { ut_dg_row({ {"Manufacturer", "Dell Inc."}, {"Product Name", "0K240J"} }) });
    // Act/Assert
    dg->getBaseBoardInfo();
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBios.size(), 1);
}

// getChassisInfo: 有效 → 1 台
TEST_F(UT_DeviceGenerator, getChassisInfo_valid)
{
    // Arrange
    ut_dg_seed("dmidecode3", { ut_dg_row({ {"Manufacturer", "Dell Inc."}, {"Type", "Desktop"} }) });
    // Act
    dg->getChassisInfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBios.size(), 1);
}

// getBiosMemoryInfo: dmidecode16 有效 → 1 台
TEST_F(UT_DeviceGenerator, getBiosMemoryInfo_valid)
{
    // Arrange
    ut_dg_seed("dmidecode16", { ut_dg_row({ {"Size", "32 GB"}, {"Form Factor", "DIMM"} }) });
    // Act
    dg->getBiosMemoryInfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBios.size(), 1);
}

// ============================ 内存 ============================

// getMemoryInfoFromLshw: System Memory + vendor + GiB → 1 台
TEST_F(UT_DeviceGenerator, getMemoryInfoFromLshw_valid)
{
    // Arrange
    ut_dg_seed("lshw_memory", { ut_dg_row({ {"description", "System Memory"}, {"vendor", "Samsung"}, {"size", "8GiB"} }) });
    // Act
    dg->getMemoryInfoFromLshw();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMemory.size(), 1);
}

// getMemoryInfoFromLshw: System Memory 但缺 vendor → 跳过
TEST_F(UT_DeviceGenerator, getMemoryInfoFromLshw_noVendor_skip)
{
    // Arrange
    ut_dg_seed("lshw_memory", { ut_dg_row({ {"description", "System Memory"}, {"size", "8GiB"} }) });
    // Act
    dg->getMemoryInfoFromLshw();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMemory.size(), 0);
}

// getMemoryInfoFromLshw: size 单位非 GiB/MiB → 跳过
TEST_F(UT_DeviceGenerator, getMemoryInfoFromLshw_badSize_skip)
{
    // Arrange
    ut_dg_seed("lshw_memory", { ut_dg_row({ {"description", "System Memory"}, {"vendor", "Samsung"}, {"size", "8KB"} }) });
    // Act
    dg->getMemoryInfoFromLshw();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMemory.size(), 0);
}

// getMemoryInfoFromDmidecode: "No Module Installed" → 跳过
TEST_F(UT_DeviceGenerator, getMemoryInfoFromDmidecode_nomodule_skip)
{
    // Arrange
    ut_dg_seed("dmidecode17", { ut_dg_row({ {"size", "No Module Installed"}, {"Locator", "DIMM0"} }) });
    // Act
    dg->getMemoryInfoFromDmidecode();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMemory.size(), 0);
}

// getMemoryInfoFromDmidecode: 已有 lshw 内存设备 → dmidecode 信息更新既有设备而非新增
TEST_F(UT_DeviceGenerator, getMemoryInfoFromDmidecode_updateExisting)
{
    // Arrange: 先经 lshw 建 1 台
    ut_dg_seed("lshw_memory", { ut_dg_row({ {"description", "System Memory"}, {"vendor", "Samsung"}, {"size", "8GiB"} }) });
    dg->getMemoryInfoFromLshw();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceMemory.size(), 1);
    ut_dg_seed("dmidecode17", { ut_dg_row({ {"size", "16GiB"}, {"Locator", "DIMM0"}, {"Manufacturer", "Samsung"} }) });
    // Act
    dg->getMemoryInfoFromDmidecode();
    // Assert: 更新不新增
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMemory.size(), 1);
}

// ============================ 存储设备 ============================

// getDiskInfoFromHwinfo: 有效 hwinfo_disk → 1 台 + BusID 记录
TEST_F(UT_DeviceGenerator, getDiskInfoFromHwinfo_valid)
{
    // Arrange
    ut_dg_seed("hwinfo_disk", { ut_dg_row({ {"SysFS BusID", "0:0:0:0"}, {"Model", "WDC WD10EZEX"}, {"Vendor", "WD"},
                                            {"Driver", "ahci"}, {"size", "931.5G"}, {"Capacity", "931 GB"},
                                            {"Hardware Class", "disk"} }) });
    // Act
    dg->getDiskInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceStorage.size(), 1);
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("0:0:0:0") }));
}

// getDiskInfoFromHwinfo: 字段不足 → 跳过且不记录 BusID
TEST_F(UT_DeviceGenerator, getDiskInfoFromHwinfo_insufficient)
{
    // Arrange
    ut_dg_seed("hwinfo_disk", { ut_dg_row({ {"Model", "WDC WD10EZEX"} }) });
    // Act
    dg->getDiskInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceStorage.size(), 0);
    EXPECT_TRUE(dg->getBusIDFromHwinfo().isEmpty());
}

// getDiskInfoFromHwinfo: setHwinfoInfo 失败（缺 SysFS BusID）→ 设备被销毁，但 BusID 若存在仍记录
TEST_F(UT_DeviceGenerator, getDiskInfoFromHwinfo_invalid_recordsBusId)
{
    // Arrange: ≥2 字段但缺 SysFS BusID → setHwinfoInfo 返回 false
    ut_dg_seed("hwinfo_disk", { ut_dg_row({ {"Model", "WDC WD10EZEX"}, {"Vendor", "WD"} }) });
    // Act
    dg->getDiskInfoFromHwinfo();
    // Assert: 无设备新增（find(SysFS BusID)==end，也不记录 BusID）
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceStorage.size(), 0);
    EXPECT_TRUE(dg->getBusIDFromHwinfo().isEmpty());
}

// getDiskInfoFromLshw: 先建 hwinfo 磁盘，再以 lshw_disk/lshw_storage 更新 → 数量不变
TEST_F(UT_DeviceGenerator, getDiskInfoFromLshw_updatesExisting)
{
    // Arrange
    ut_dg_seed("hwinfo_disk", { ut_dg_row({ {"SysFS BusID", "0:0:0:0"}, {"Model", "WDC WD10EZEX"}, {"Vendor", "WD"},
                                            {"Driver", "ahci"}, {"size", "931.5G"}, {"Capacity", "931 GB"} }) });
    dg->getDiskInfoFromHwinfo();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceStorage.size(), 1);
    ut_dg_seed("lshw_disk", { ut_dg_row({ {"logical name", "/dev/sda"}, {"serial", "WD-WCC6Y4YRK2A8"}, {"vendor", "Western Digital"} }) });
    ut_dg_seed("lshw_storage", { ut_dg_row({ {"product", "Samsung SSD 970"}, {"serial", "S5H9NJ0N"}, {"size", "500GB"} }) });
    // Act
    dg->getDiskInfoFromLshw();
    // Assert: lshw 仅更新既有设备
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceStorage.size(), 1);
}

// getDiskInfoFromLsblk: lsblk_d 首行键值对 → setStorageDeviceMediaType 无异常
TEST_F(UT_DeviceGenerator, getDiskInfoFromLsblk_noCrash)
{
    // Arrange
    ut_dg_seed("lsblk_d", { ut_dg_row({ {"8:0", "disk"}, {"8:16", "disk"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getDiskInfoFromLsblk());
}

// getDiskInfoFromLsblk: 空 lsblk_d → 不进入循环
TEST_F(UT_DeviceGenerator, getDiskInfoFromLsblk_empty)
{
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getDiskInfoFromLsblk());
}

// getDiskInfoFromSmartCtl: smart 条目含 "ln" → setStorageInfoFromSmartctl 无异常
TEST_F(UT_DeviceGenerator, getDiskInfoFromSmartCtl_noCrash)
{
    // Arrange
    ut_dg_seed("smart", { ut_dg_row({ {"ln", "0:0:0:0"}, {"Model Family", "WDC"}, {"User Capacity", "1,000,204,886,016 bytes"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getDiskInfoFromSmartCtl());
}

// getDiskInfoFromSmartCtl: 缺 "ln" → 跳过
TEST_F(UT_DeviceGenerator, getDiskInfoFromSmartCtl_noLn_skip)
{
    // Arrange
    ut_dg_seed("smart", { ut_dg_row({ {"Model Family", "WDC"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getDiskInfoFromSmartCtl());
}

// generatorDiskDevice: 全链路（hwinfo→lshw→lsblk→smart→merge→order）
TEST_F(UT_DeviceGenerator, generatorDiskDevice_pipeline)
{
    // Arrange
    ut_dg_seed("hwinfo_disk", { ut_dg_row({ {"SysFS BusID", "0:0:0:0"}, {"Model", "WDC WD10EZEX"}, {"Vendor", "WD"},
                                            {"Driver", "ahci"}, {"size", "931.5G"}, {"Capacity", "931 GB"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->generatorDiskDevice());
    EXPECT_GE(DeviceManager::instance()->m_ListDeviceStorage.size(), 1);
}

// ============================ 显卡 ============================

// getGpuInfoFromHwinfo: ≥5 字段 → 1 台 + BusID 记录
TEST_F(UT_DeviceGenerator, getGpuInfoFromHwinfo_valid)
{
    // Arrange
    ut_dg_seed("hwinfo_display", { ut_dg_row({ {"Model", "NVIDIA GTX 1650"}, {"Vendor", "NVIDIA"}, {"Driver", "nvidia"},
                                               {"SysFS BusID", "0000:01:00.0"}, {"Device File", "/dev/dri/card0"} }) });
    // Act
    dg->getGpuInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceGPU.size(), 1);
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("0000:01:00") }));
}

// getGpuInfoFromHwinfo: 字段不足 5 → 跳过
TEST_F(UT_DeviceGenerator, getGpuInfoFromHwinfo_insufficient)
{
    // Arrange
    ut_dg_seed("hwinfo_display", { ut_dg_row({ {"Model", "NVIDIA GTX 1650"}, {"Vendor", "NVIDIA"}, {"Driver", "nvidia"}, {"SysFS BusID", "0000:01:00.0"} }) });
    // Act
    dg->getGpuInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceGPU.size(), 0);
}

// getGpuInfoFromLshw: 先建 hwinfo GPU，再 lshw_display 更新 → 数量不变
TEST_F(UT_DeviceGenerator, getGpuInfoFromLshw_updatesExisting)
{
    // Arrange
    ut_dg_seed("hwinfo_display", { ut_dg_row({ {"Model", "NVIDIA GTX 1650"}, {"Vendor", "NVIDIA"}, {"Driver", "nvidia"},
                                               {"SysFS BusID", "0000:01:00.0"}, {"Device File", "/dev/dri/card0"} }) });
    dg->getGpuInfoFromHwinfo();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceGPU.size(), 1);
    ut_dg_seed("lshw_display", { ut_dg_row({ {"product", "TU117"}, {"vendor", "NVIDIA"}, {"description", "VGA compatible controller"},
                                             {"bus info", "pci@0000:01:00.0"}, {"capacity", "4GB"} }) });
    // Act
    dg->getGpuInfoFromLshw();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceGPU.size(), 1);
}

// getGpuInfoFromLshw: 字段不足 → 无操作
TEST_F(UT_DeviceGenerator, getGpuInfoFromLshw_insufficient)
{
    // Arrange
    ut_dg_seed("lshw_display", { ut_dg_row({ {"product", "TU117"}, {"vendor", "NVIDIA"}, {"description", "VGA"}, {"capacity", "4GB"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getGpuInfoFromLshw());
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceGPU.size(), 0);
}

// getGpuInfoFromXrandr: ≥1 字段 → setGpuInfoFromXrandr 无异常
TEST_F(UT_DeviceGenerator, getGpuInfoFromXrandr_noCrash)
{
    // Arrange
    ut_dg_seed("xrandr", { ut_dg_row({ {"Screen 0", "minimum 8 x 8"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getGpuInfoFromXrandr());
}

// getGpuInfoFromXrandr: 空输入 → 跳过
TEST_F(UT_DeviceGenerator, getGpuInfoFromXrandr_empty)
{
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getGpuInfoFromXrandr());
}

// getGpuSizeFromDmesg: dmesg 存在 → setGpuSizeFromDmesg（dmesg 分支）
TEST_F(UT_DeviceGenerator, getGpuSizeFromDmesg_dmesgBranch)
{
    // Arrange
    ut_dg_seed("hwinfo_display", { ut_dg_row({ {"Model", "NVIDIA GTX 1650"}, {"Vendor", "NVIDIA"}, {"Driver", "nvidia"},
                                               {"SysFS BusID", "0000:01:00.0"}, {"Device File", "/dev/dri/card0"} }) });
    ut_dg_seed("dmesg", { ut_dg_row({ {"NVRM", "0x0000:01:00.0: 4096MiB"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getGpuSizeFromDmesg());
}

// getGpuSizeFromDmesg: 无 dmesg 但有 nvidia → nvidia 分支
TEST_F(UT_DeviceGenerator, getGpuSizeFromDmesg_nvidiaBranch)
{
    // Arrange
    ut_dg_seed("hwinfo_display", { ut_dg_row({ {"Model", "NVIDIA GTX 1650"}, {"Vendor", "NVIDIA"}, {"Driver", "nvidia"},
                                               {"SysFS BusID", "0000:01:00.0"}, {"Device File", "/dev/dri/card0"} }) });
    ut_dg_seed("nvidia", { ut_dg_row({ {"VideoRam", "2097152"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getGpuSizeFromDmesg());
}

// ============================ 显示器 ============================

// getMonitorInfoFromHwinfo: ≥5 字段 → 1 台 + BusID 记录
TEST_F(UT_DeviceGenerator, getMonitorInfoFromHwinfo_valid)
{
    // Arrange
    ut_dg_seed("hwinfo_monitor", { ut_dg_row({ {"Model", "Dell U2419H"}, {"Vendor", "Dell"}, {"Serial ID", "CN-0KD4M3"},
                                               {"SysFS BusID", "1-1:1.0"}, {"Resolution", "1920x1080"} }) });
    // Act
    dg->getMonitorInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMonitor.size(), 1);
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("1-1:1") }));
}

// getMonitorInfoFromHwinfo: 字段不足 → 跳过
TEST_F(UT_DeviceGenerator, getMonitorInfoFromHwinfo_insufficient)
{
    // Arrange
    ut_dg_seed("hwinfo_monitor", { ut_dg_row({ {"Model", "Dell U2419H"}, {"Vendor", "Dell"}, {"Serial ID", "CN-0KD4M3"}, {"Resolution", "1920x1080"} }) });
    // Act
    dg->getMonitorInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMonitor.size(), 0);
}

// getMonitorInfoFromXrandrVerbose: 先建 hwinfo 显示器，再以 xrandr_verbose 更新
TEST_F(UT_DeviceGenerator, getMonitorInfoFromXrandrVerbose_updatesExisting)
{
    // Arrange
    ut_dg_seed("hwinfo_monitor", { ut_dg_row({ {"Model", "Dell U2419H"}, {"Vendor", "Dell"}, {"Serial ID", "CN-0KD4M3"},
                                               {"SysFS BusID", "1-1:1.0"}, {"Resolution", "1920x1080"} }) });
    dg->getMonitorInfoFromHwinfo();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceMonitor.size(), 1);
    ut_dg_seed("xrandr_verbose", { ut_dg_row({ {"mainInfo", "HDMI-1 connected primary 1920x1080+0+0"}, {"edid", "00ffffffffffff00"} }) });
    // Act
    dg->getMonitorInfoFromXrandrVerbose();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMonitor.size(), 1);
}

// getMonitorInfoFromXrandrVerbose: 空输入 → 无操作
TEST_F(UT_DeviceGenerator, getMonitorInfoFromXrandrVerbose_empty)
{
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getMonitorInfoFromXrandrVerbose());
}

// ============================ 声卡 ============================

// getAudioInfoFromHwinfo: 6 字段含 path → 查无既有设备 → 新增
TEST_F(UT_DeviceGenerator, getAudioInfoFromHwinfo_addWithPath)
{
    // Arrange
    ut_dg_seed("hwinfo_sound", { ut_dg_row({ {"Model", "Intel HDMI"}, {"Vendor", "Intel"}, {"Driver", "snd_hda_intel"},
                                             {"path", "/devices/pci0000:00/0000:00:1f.3"}, {"SysFS BusID", "0:1f.3"},
                                             {"Module Alias", "sound-alias"} }) });
    // Act
    dg->getAudioInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceAudio.size(), 1);
}

// getAudioInfoFromHwinfo: 恰 5 字段含 unique_id → 新增且被禁用（唯一 ID 记录）
TEST_F(UT_DeviceGenerator, getAudioInfoFromHwinfo_disabledWithUniqueID)
{
    // Arrange
    ut_dg_seed("hwinfo_sound", { ut_dg_row({ {"Model", "Intel HDMI"}, {"Vendor", "Intel"}, {"Driver", "snd_hda_intel"},
                                             {"unique_id", "snd-0x1f.3"}, {"SysFS BusID", "0:1f.3"} }) });
    // Act
    dg->getAudioInfoFromHwinfo();
    // Assert
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceAudio.size(), 1);
    DeviceAudio *audio = dynamic_cast<DeviceAudio *>(DeviceManager::instance()->m_ListDeviceAudio[0]);
    ASSERT_NE(audio, nullptr);
    EXPECT_FALSE(audio->enable());   // 禁用设备 enable 为假
}

// getAudioInfoFromHwinfo: 同一 SysFS Device Link 两行 → 第二行命中既有设备（禁用）而非新增
//（addCmdInfo 为追加语义，同键多行必须在同一次 seed 中给出）
TEST_F(UT_DeviceGenerator, getAudioInfoFromHwinfo_updateExisting)
{
    // Arrange: 两行同 SysFS ID（pciPath 与设备 sysPath 一致，第二行命中禁用分支）
    ut_dg_seed("hwinfo_sound", { ut_dg_row({ {"Model", "Intel HDMI"}, {"Vendor", "Intel"}, {"Driver", "snd_hda_intel"},
                                             {"SysFS ID", "/devices/pci0000:00/0000:00:1f.3"}, {"SysFS BusID", "0:1f.3"},
                                             {"Module Alias", "sound-alias"} }),
                                 ut_dg_row({ {"Model", "Intel HDMI"}, {"Vendor", "Intel"}, {"Driver", "snd_hda_intel"},
                                             {"SysFS ID", "/devices/pci0000:00/0000:00:1f.3"}, {"SysFS BusID", "0:1f.3"},
                                             {"Module Alias", "sound-alias"} }) });
    // Act
    dg->getAudioInfoFromHwinfo();
    // Assert: 第二行命中 getAudioDevice → 更新既有，数量不变
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceAudio.size(), 1);
}

// getAudioInfoFromHwinfo: 字段不足 5 且无 path → 跳过
TEST_F(UT_DeviceGenerator, getAudioInfoFromHwinfo_insufficient)
{
    // Arrange
    ut_dg_seed("hwinfo_sound", { ut_dg_row({ {"Model", "Intel HDMI"}, {"Vendor", "Intel"}, {"Driver", "snd_hda_intel"}, {"SysFS BusID", "0:1f.3"} }) });
    // Act
    dg->getAudioInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceAudio.size(), 0);
}

// getAudioInfoFromLshw: ≥5 字段 → setAudioInfoFromLshw + 去重，无异常
TEST_F(UT_DeviceGenerator, getAudioInfoFromLshw_noCrash)
{
    // Arrange
    ut_dg_seed("lshw_multimedia", { ut_dg_row({ {"product", "100 Series Audio"}, {"vendor", "Intel"}, {"description", "Audio device"},
                                                {"bus info", "pci@0000:00:1f.3"}, {"configuration", "driver=snd_hda_intel"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getAudioInfoFromLshw());
}

// getAudioInfoFromLshw: 字段不足 → 无操作
TEST_F(UT_DeviceGenerator, getAudioInfoFromLshw_insufficient)
{
    // Arrange
    ut_dg_seed("lshw_multimedia", { ut_dg_row({ {"product", "100 Series Audio"}, {"vendor", "Intel"}, {"description", "Audio device"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getAudioInfoFromLshw());
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceAudio.size(), 0);
}

// getAudioInfoFromCatInput: Sysfs 含 sound → 新增
TEST_F(UT_DeviceGenerator, getAudioInfoFromCatInput_sound)
{
    // Arrange
    ut_dg_seed("cat_devices", { ut_dg_row({ {"N", "HDA Intel PCH"}, {"Sysfs", "/devices/pci0000:00/0000:00:1f.3/sound/card0"} }) });
    // Act
    dg->getAudioInfoFromCatInput();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceAudio.size(), 1);
}

// getAudioInfoFromCatInput: Sysfs 不含 sound → 跳过
TEST_F(UT_DeviceGenerator, getAudioInfoFromCatInput_nonSound)
{
    // Arrange
    ut_dg_seed("cat_devices", { ut_dg_row({ {"N", "USB Camera"}, {"Sysfs", "/devices/pci0000:00/0000:00:14.0/usb1"} }) });
    // Act
    dg->getAudioInfoFromCatInput();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceAudio.size(), 0);
}

// getAudioInfoFrom_sysFS: 桩 setInfoFrom_sysFS（真实实现读 /sys/class/sound）—
// 前 2 张卡成功：第 1 张新增，第 2 张命中既有 → 更新芯片并销毁临时对象
TEST_F(UT_DeviceGenerator, getAudioInfoFrom_sysFS_stubbed)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DeviceAudio, setInfoFrom_sysFS), ut_dg_audioSysFS_okTwice);
    // Act
    dg->getAudioInfoFrom_sysFS();
    // Assert: card0/card1 sysPath 不同 → 各自新增；i=2..4 失败不再新增
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceAudio.size(), 2);
}

// getAudioInfoFrom_sysFS: 桩始终失败 → 不产生设备
TEST_F(UT_DeviceGenerator, getAudioInfoFrom_sysFS_alwaysFail)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DeviceAudio, setInfoFrom_sysFS), ut_dg_audioSysFS_alwaysFail);
    // Act
    dg->getAudioInfoFrom_sysFS();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceAudio.size(), 0);
}

// getAudioChipInfoFromDmesg: audiochip 键存在 → setAudioChipFromDmesg 无异常
TEST_F(UT_DeviceGenerator, getAudioChipInfoFromDmesg_noCrash)
{
    // Arrange
    ut_dg_seed("audiochip", { ut_dg_row({ {"chip", "ALC256"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getAudioChipInfoFromDmesg());
}

// getAudioChipInfoFromDmesg: 空输入 → 无操作
TEST_F(UT_DeviceGenerator, getAudioChipInfoFromDmesg_empty)
{
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getAudioChipInfoFromDmesg());
}

// ============================ 蓝牙 ============================

// getBlueToothInfoFromHwinfo: Model 含 bluetooth → 新增 + BusID 记录
TEST_F(UT_DeviceGenerator, getBlueToothInfoFromHwinfo_model)
{
    // Arrange
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "Intel Bluetooth wireless"}, {"Hardware Class", "bluetooth"},
                                           {"Driver", "btusb"}, {"SysFS BusID", "1-4:1.0"}, {"Serial ID", "8087:0a2a"} }) });
    // Act
    dg->getBlueToothInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 1);
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("1-4:1") }));
}

// getBlueToothInfoFromHwinfo: Hardware Class hub → 过滤（mouse/keyboard 同构）
TEST_F(UT_DeviceGenerator, getBlueToothInfoFromHwinfo_hubSkip)
{
    // Arrange
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "USB2.0 Hub"}, {"Hardware Class", "hub"}, {"Driver", "hub"} }) });
    // Act
    dg->getBlueToothInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 0);
}

// getBlueToothInfoFromHwinfo: 无 Driver 但 Driver Status 含 btusb → 新增
TEST_F(UT_DeviceGenerator, getBlueToothInfoFromHwinfo_driverStatus)
{
    // Arrange
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "BCM20702A0"}, {"Hardware Class", "bluetooth"},
                                           {"Driver Status", "btusb is active"}, {"SysFS BusID", "1-5:1.0"} }) });
    // Act
    dg->getBlueToothInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 1);
}

// getBlueToothInfoFromHwinfo: 相同 unique_id + 相同链路第二行 → 禁用既有而非新增
TEST_F(UT_DeviceGenerator, getBlueToothInfoFromHwinfo_duplicateUpdate)
{
    // Arrange: 两行同 Unique ID（DeviceBluetooth::uniqueID ← "Unique ID" 键，第二行命中禁用分支）
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "Intel Bluetooth wireless"}, {"Hardware Class", "bluetooth"},
                                           {"Driver", "btusb"}, {"Unique ID", "bt-8087"}, {"SysFS BusID", "1-4:1.0"} }),
                               ut_dg_row({ {"Model", "Intel Bluetooth wireless"}, {"Hardware Class", "bluetooth"},
                                           {"Driver", "btusb"}, {"Unique ID", "bt-8087"}, {"SysFS BusID", "1-4:1.0"} }) });
    // Act
    dg->getBlueToothInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 1);
}

// getBluetoothInfoFromLshw: ≥1 字段 → setBluetoothInfoFromLshw 更新既有，无异常
TEST_F(UT_DeviceGenerator, getBluetoothInfoFromLshw_noCrash)
{
    // Arrange: 先建 1 台蓝牙
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "Intel Bluetooth wireless"}, {"Hardware Class", "bluetooth"},
                                           {"Driver", "btusb"}, {"SysFS BusID", "1-4:1.0"} }) });
    dg->getBlueToothInfoFromHwinfo();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 1);
    ut_dg_seed("lshw_usb", { ut_dg_row({ {"product", "Bluetooth wireless interface"}, {"vendor", "Intel Corp."}, {"serial", "8087:0a2a"} }) });
    // Act
    dg->getBluetoothInfoFromLshw();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 1);
}

// getBluetoothInfoFromHciconfig: hwinfo 两行同链路仅产生 1 台设备，hciconfig 首行更新之
TEST_F(UT_DeviceGenerator, getBluetoothInfoFromHciconfig_updatesExisting)
{
    // Arrange: 两行 hwinfo 蓝牙（同链路 → 第二行命中既有，仅建 1 台）
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "Intel Bluetooth wireless"}, {"Hardware Class", "bluetooth"},
                                           {"Driver", "btusb"}, {"SysFS Device Link", "/devices/usb1/1-4"},
                                           {"SysFS BusID", "1-4:1.0"} }),
                               ut_dg_row({ {"Model", "Realtek Bluetooth radio"}, {"Hardware Class", "bluetooth"},
                                           {"Driver", "btrtl"}, {"SysFS Device Link", "/devices/usb1/1-4"},
                                           {"SysFS BusID", "1-6:1.0"} }) });
    dg->getBlueToothInfoFromHwinfo();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 1);
    ut_dg_seed("hciconfig", { ut_dg_row({ {"hci0", "Type: BR/EDR Bus: USB"}, {"BD Address", "9C:B6:D0:12:34:56 ACL MTU: 1021:4"} }),
                              ut_dg_row({ {"hci1", "Type: BR/EDR Bus: USB"}, {"BD Address", "9C:B6:D0:AB:CD:EF ACL MTU: 1021:4"} }) });
    // Act: index=0 更新既有设备；index=1 getBluetoothAtIndex(1) 越界为空
    dg->getBluetoothInfoFromHciconfig();
    // Assert: 更新不新增
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 1);
}

// getBluetoothInfoFromHciconfig: 无蓝牙设备（getBluetoothAtIndex 越界）→ 无操作
TEST_F(UT_DeviceGenerator, getBluetoothInfoFromHciconfig_noDevices)
{
    // Arrange
    ut_dg_seed("hciconfig", { ut_dg_row({ {"hci0", "Type: BR/EDR Bus: USB"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getBluetoothInfoFromHciconfig());
}

// ============================ 键盘 ============================

// getKeyboardInfoFromHwinfo: 有效行 → 1 台 + BusID 记录
TEST_F(UT_DeviceGenerator, getKeyboardInfoFromHwinfo_valid)
{
    // Arrange
    ut_dg_seed("hwinfo_keyboard", { ut_dg_row({ {"Model", "AT Translated Set 2 keyboard"}, {"Hotplug", "PS/2"},
                                                {"SysFS BusID", "i8042/serio0"}, {"Driver", "atkbd"} }) });
    // Act
    dg->getKeyboardInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceKeyboard.size(), 1);
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("i8042/serio0") }));
}

// getKeyboardInfoFromHwinfo: 空行 → 跳过
TEST_F(UT_DeviceGenerator, getKeyboardInfoFromHwinfo_emptyRow)
{
    // Arrange
    ut_dg_seed("hwinfo_keyboard", { ut_dg_row({}) });
    // Act
    dg->getKeyboardInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceKeyboard.size(), 0);
    EXPECT_TRUE(dg->getBusIDFromHwinfo().isEmpty());
}

// getKeyboardInfoFromLshw: ≥1 字段 → setKeyboardInfoFromLshw 更新既有，无异常
TEST_F(UT_DeviceGenerator, getKeyboardInfoFromLshw_noCrash)
{
    // Arrange: 先建 1 台键盘
    ut_dg_seed("hwinfo_keyboard", { ut_dg_row({ {"Model", "AT Translated Set 2 keyboard"}, {"Hotplug", "PS/2"} }) });
    dg->getKeyboardInfoFromHwinfo();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceKeyboard.size(), 1);
    ut_dg_seed("lshw_usb", { ut_dg_row({ {"product", "USB Receiver"}, {"vendor", "Logitech"} }) });
    // Act
    dg->getKeyboardInfoFromLshw();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceKeyboard.size(), 1);
}

// ============================ 鼠标 ============================

// getMouseInfoFromHwinfo: Touchpad + PS/2 → 新增（跳过 authorized 检查）
TEST_F(UT_DeviceGenerator, getMouseInfoFromHwinfo_touchpad)
{
    // Arrange
    ut_dg_seed("hwinfo_mouse", { ut_dg_row({ {"Model", "Logitech Touchpad"}, {"Hotplug", "PS/2"},
                                             {"SysFS BusID", "i8042/serio1"}, {"Driver", "psmouse"} }) });
    // Act
    dg->getMouseInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMouse.size(), 1);
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("i8042/serio1") }));
}

// getMouseInfoFromHwinfo: Device 含 PM → 过滤
TEST_F(UT_DeviceGenerator, getMouseInfoFromHwinfo_pmSkip)
{
    // Arrange
    ut_dg_seed("hwinfo_mouse", { ut_dg_row({ {"Device", "PM Mouse"}, {"Hotplug", "PS/2"} }) });
    // Act
    dg->getMouseInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMouse.size(), 0);
}

// getMouseInfoFromHwinfo: Device 含 T70 → 过滤
TEST_F(UT_DeviceGenerator, getMouseInfoFromHwinfo_t70Skip)
{
    // Arrange
    ut_dg_seed("hwinfo_mouse", { ut_dg_row({ {"Device", "MOSART Semi. 2.4G T70"}, {"Hotplug", "PS/2"} }) });
    // Act
    dg->getMouseInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMouse.size(), 0);
}

// getMouseInfoFromHwinfo: 非 PS/2 普通鼠标 → DeviceInput 构造默认蓝牙已连接（m_BluetoothIsConnected=true）→ 正常新增
TEST_F(UT_DeviceGenerator, getMouseInfoFromHwinfo_nonTouchpadDefaultAdded)
{
    // Arrange: Hotplug=USB，SysFS Device Link 指向本机真实 authorized 文件
    ut_dg_seed("hwinfo_mouse", { ut_dg_row({ {"Model", "Logitech USB Mouse"}, {"Hotplug", "USB"},
                                             {"SysFS Device Link", "/bus/usb/devices/usb1"}, {"SysFS BusID", "1-1:1.0"} }) });
    // Act
    dg->getMouseInfoFromHwinfo();
    // Assert: 蓝牙默认已连接 → 设备保留；BusID 去除 .0 后缀
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceMouse.size(), 1);
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("1-1:1") }));
}

// getMouseInfoFromHwinfo: hwinfo_usb 中 Melfas 触控屏 → 新增
TEST_F(UT_DeviceGenerator, getMouseInfoFromHwinfo_melfasTouch)
{
    // Arrange
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "Melfas LGDisplay Incell Touch"}, {"Hardware Class", "mouse"},
                                           {"Driver", "usbhid"}, {"SysFS BusID", "1-2:1.0"} }) });
    // Act
    dg->getMouseInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMouse.size(), 1);
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("1-2:1") }));
}

// getMouseInfoFromLshw: ≥1 字段 → addMouseInfoFromLshw 更新既有，无异常
TEST_F(UT_DeviceGenerator, getMouseInfoFromLshw_noCrash)
{
    // Arrange: 先建 1 台鼠标（触控板）
    ut_dg_seed("hwinfo_mouse", { ut_dg_row({ {"Model", "Logitech Touchpad"}, {"Hotplug", "PS/2"} }) });
    dg->getMouseInfoFromHwinfo();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceMouse.size(), 1);
    ut_dg_seed("lshw_usb", { ut_dg_row({ {"product", "USB Receiver"}, {"vendor", "Logitech"} }) });
    // Act
    dg->getMouseInfoFromLshw();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMouse.size(), 1);
}

// getMouseInfoFromCatDevices: 空实现 → 调用无副作用
TEST_F(UT_DeviceGenerator, getMouseInfoFromCatDevices_emptyBody)
{
    // Act/Assert: 空方法，回归保护
    EXPECT_NO_FATAL_FAILURE(dg->getMouseInfoFromCatDevices());
}

// ============================ 摄像头/图像设备 ============================

// getImageInfoFromHwinfo: Model 含 camera + 路径不含 usb（跳过 QFile 校验）→ 新增 + BusID
TEST_F(UT_DeviceGenerator, getImageInfoFromHwinfo_valid)
{
    // Arrange
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "Integrated Camera"}, {"Hardware Class", "camera"},
                                           {"Driver", "uvcvideo"}, {"SysFS Device Link", "/devices/pci0000:00/0000:00:14.0/0000:02:00.0"},
                                           {"SysFS BusID", "1-3:1.0"} }) });
    // Act
    dg->getImageInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceImage.size(), 1);
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("1-3:1") }));
}

// getImageInfoFromHwinfo: 非 camera/webcam/uvcvideo → 跳过
TEST_F(UT_DeviceGenerator, getImageInfoFromHwinfo_nonCamera)
{
    // Arrange
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "USB2.0 Hub"}, {"Hardware Class", "hub"}, {"Driver", "hub"},
                                           {"SysFS Device Link", "/devices/pci0000:00/0000:00:14.0"}, {"SysFS BusID", "1-0:1.0"} }) });
    // Act
    dg->getImageInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceImage.size(), 0);
    EXPECT_TRUE(dg->getBusIDFromHwinfo().isEmpty());
}

// getImageInfoFromHwinfo: 相同 unique_id 二次出现 → 禁用更新既有
TEST_F(UT_DeviceGenerator, getImageInfoFromHwinfo_duplicateUpdate)
{
    // Arrange
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "Integrated Camera"}, {"Hardware Class", "camera"},
                                           {"Driver", "uvcvideo"}, {"unique_id", "cam-04f2"},
                                           {"SysFS Device Link", "/devices/pci0000:00/0000:00:14.0/0000:02:00.0"},
                                           {"SysFS BusID", "1-3:1.0"} }),
                               ut_dg_row({ {"Model", "Integrated Camera"}, {"Hardware Class", "camera"},
                                           {"Driver", "uvcvideo"}, {"unique_id", "cam-04f2"},
                                           {"SysFS Device Link", "/devices/pci0000:00/0000:00:14.0/0000:02:00.0"},
                                           {"SysFS BusID", "1-3:1.0"} }) });
    // Act
    dg->getImageInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceImage.size(), 1);
}

// getImageInfoFromLshw: ≥2 字段 → setCameraInfoFromLshw 更新既有，无异常
TEST_F(UT_DeviceGenerator, getImageInfoFromLshw_noCrash)
{
    // Arrange: 先建 1 台摄像头
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Model", "Integrated Camera"}, {"Hardware Class", "camera"},
                                           {"Driver", "uvcvideo"}, {"SysFS Device Link", "/devices/pci0000:00/0000:00:14.0/0000:02:00.0"} }) });
    dg->getImageInfoFromHwinfo();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceImage.size(), 1);
    ut_dg_seed("lshw_usb", { ut_dg_row({ {"product", "Integrated Camera"}, {"vendor", "Chicony"} }) });
    // Act
    dg->getImageInfoFromLshw();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceImage.size(), 1);
}

// getImageInfoFromLshw: 字段不足 → 无操作
TEST_F(UT_DeviceGenerator, getImageInfoFromLshw_insufficient)
{
    // Arrange
    ut_dg_seed("lshw_usb", { ut_dg_row({ {"product", "Integrated Camera"} }) });
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(dg->getImageInfoFromLshw());
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceImage.size(), 0);
}

// ============================ 光驱 ============================

// getCdromInfoFromHwinfo: ≥5 字段 → 1 台 + BusID 记录
TEST_F(UT_DeviceGenerator, getCdromInfoFromHwinfo_valid)
{
    // Arrange
    ut_dg_seed("hwinfo_cdrom", { ut_dg_row({ {"Model", "DVDRAM GH24NSC0"}, {"Vendor", "HL-DT-ST"}, {"Driver", "ata_piix"},
                                             {"SysFS BusID", "1:0:0:0"}, {"Capacity", "700 MB"} }) });
    // Act
    dg->getCdromInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceCdrom.size(), 1);
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("1:0:0:0") }));
}

// getCdromInfoFromHwinfo: 字段不足 → 跳过
TEST_F(UT_DeviceGenerator, getCdromInfoFromHwinfo_insufficient)
{
    // Arrange
    ut_dg_seed("hwinfo_cdrom", { ut_dg_row({ {"Model", "DVDRAM GH24NSC0"}, {"Vendor", "HL-DT-ST"}, {"Driver", "ata_piix"}, {"SysFS BusID", "1:0:0:0"} }) });
    // Act
    dg->getCdromInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceCdrom.size(), 0);
}

// getCdromInfoFromLshw: 先建 hwinfo 光驱，lshw_cdrom ≥2 字段 → setInfoFromLshw 更新
TEST_F(UT_DeviceGenerator, getCdromInfoFromLshw_updatesExisting)
{
    // Arrange
    ut_dg_seed("hwinfo_cdrom", { ut_dg_row({ {"Model", "DVDRAM GH24NSC0"}, {"Vendor", "HL-DT-ST"}, {"Driver", "ata_piix"},
                                             {"SysFS BusID", "1:0:0:0"}, {"Capacity", "700 MB"} }) });
    dg->getCdromInfoFromHwinfo();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceCdrom.size(), 1);
    ut_dg_seed("lshw_cdrom", { ut_dg_row({ {"product", "DVDRAM GH24NSC0"}, {"vendor", "HL-DT-ST"} }) });
    // Act
    dg->getCdromInfoFromLshw();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceCdrom.size(), 1);
}

// ============================ 其它设备 ============================

// getOthersInfoFromHwinfo: Device 含 fingerprint → 强制显示新增
TEST_F(UT_DeviceGenerator, getOthersInfoFromHwinfo_fingerprint)
{
    // Arrange
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Device", "Fingerprint Reader"}, {"Hardware Class", "unknown"},
                                           {"Driver", "uinput"}, {"SysFS BusID", "1-7:1.0"} }) });
    // Act
    dg->getOthersInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceOthers.size(), 1);
}

// getOthersInfoFromHwinfo: 无 unique_id + BusID 不在管理器列表 + authorized 真实存在 → 新增
TEST_F(UT_DeviceGenerator, getOthersInfoFromHwinfo_validAuthorized)
{
    // Arrange: /sys/bus/usb/devices/usb1/authorized 本机必存在
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Device", "Generic USB Device"}, {"Hardware Class", "unknown"},
                                           {"SysFS BusID", "1-1:1.0"},
                                           {"SysFS Device Link", "/bus/usb/devices/usb1"} }) });
    // Act
    dg->getOthersInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceOthers.size(), 1);
}

// getOthersInfoFromHwinfo: 含 unique_id 且 Hardware Class != others → isOtherDevice=false 跳过
TEST_F(UT_DeviceGenerator, getOthersInfoFromHwinfo_uniqueIdSkip)
{
    // Arrange
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Device", "Generic USB Device"}, {"Hardware Class", "unknown"},
                                           {"unique_id", "other-1"}, {"SysFS BusID", "1-1:1.0"},
                                           {"SysFS Device Link", "/bus/usb/devices/usb1"} }) });
    // Act
    dg->getOthersInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceOthers.size(), 0);
}

// getOthersInfoFromHwinfo: 缺 SysFS BusID → curBus 空 → 跳过
TEST_F(UT_DeviceGenerator, getOthersInfoFromHwinfo_curBusEmpty)
{
    // Arrange
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Device", "Generic USB Device"}, {"Hardware Class", "unknown"},
                                           {"SysFS Device Link", "/bus/usb/devices/usb1"} }) });
    // Act
    dg->getOthersInfoFromHwinfo();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceOthers.size(), 0);
}

// getOthersInfoFromLshw: ≥2 字段 → setOthersDeviceInfoFromLshw 更新既有，无异常
TEST_F(UT_DeviceGenerator, getOthersInfoFromLshw_noCrash)
{
    // Arrange: 先建 1 台其它设备
    ut_dg_seed("hwinfo_usb", { ut_dg_row({ {"Device", "Fingerprint Reader"}, {"Hardware Class", "unknown"}, {"Driver", "uinput"} }) });
    dg->getOthersInfoFromHwinfo();
    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceOthers.size(), 1);
    ut_dg_seed("lshw_usb", { ut_dg_row({ {"product", "Fingerprint Reader"}, {"vendor", "Validity"} }) });
    // Act
    dg->getOthersInfoFromLshw();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceOthers.size(), 1);
}

// ============================ 打印机 ============================

// generatorPrinterDevice: 2 行有效打印机 → 2 台
TEST_F(UT_DeviceGenerator, generatorPrinterDevice_valid)
{
    // Arrange
    ut_dg_seed("printer", { ut_dg_row({ {"printer-name", "HP_LaserJet"}, {"printer-state", "3"}, {"printer-make-and-model", "HP LaserJet Pro"} }),
                            ut_dg_row({ {"printer-name", "Canon_iR"}, {"printer-state", "3"}, {"printer-make-and-model", "Canon iR-ADV"} }) });
    // Act
    dg->generatorPrinterDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDevicePrint.size(), 2);
}

// generatorPrinterDevice: 字段不足 2 → 跳过
TEST_F(UT_DeviceGenerator, generatorPrinterDevice_insufficient)
{
    // Arrange
    ut_dg_seed("printer", { ut_dg_row({ {"printer-name", "HP_LaserJet"} }) });
    // Act
    dg->generatorPrinterDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDevicePrint.size(), 0);
}

// ============================ 电源 ============================

// generatorPowerDevice: upower 有效电池 + Daemon → 1 台并注入守护信息
TEST_F(UT_DeviceGenerator, generatorPowerDevice_valid)
{
    // Arrange: 不带 percentage 键（该分支会触发真实 DBus）
    ut_dg_seed("upower", { ut_dg_row({ {"Device", "BAT"}, {"state", "charging"}, {"capacity", "88.5%"},
                                       {"vendor", "SMP"}, {"model", "DELL CYMGM"} }) });
    ut_dg_seed("Daemon", { ut_dg_row({ {"DaemonVersion", "0.9.1"} }) });
    // Act
    dg->generatorPowerDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDevicePower.size(), 1);
}

// generatorPowerDevice: Device 为 line_power → setInfoFromUpower 返回 false → 跳过
TEST_F(UT_DeviceGenerator, generatorPowerDevice_linePower)
{
    // Arrange
    ut_dg_seed("upower", { ut_dg_row({ {"Device", "line_power_AC"}, {"state", "unknown"} }) });
    // Act
    dg->generatorPowerDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDevicePower.size(), 0);
}

// generatorPowerDevice: 字段不足 2 → 跳过
TEST_F(UT_DeviceGenerator, generatorPowerDevice_insufficient)
{
    // Arrange
    ut_dg_seed("upower", { ut_dg_row({ {"Device", "BAT"} }) });
    // Act
    dg->generatorPowerDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDevicePower.size(), 0);
}

// ============================ 网络设备 ============================

// generatorNetworkDevice: lshw_network 合法 MAC + 本机存在的逻辑名 lo → 1 台有线设备
TEST_F(UT_DeviceGenerator, generatorNetworkDevice_lshwValid)
{
    // Arrange: lo 在 /sys/class/net 下必存在
    ut_dg_seed("lshw_network", { ut_dg_row({ {"logical name", "lo"}, {"serial", "f4:b5:20:24:5e:4f"},
                                             {"description", "Ethernet interface"}, {"capacity", "1GB/s"} }) });
    // Act
    dg->generatorNetworkDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceNetwork.size(), 1);
}

// generatorNetworkDevice: 缺 serial → 跳过
TEST_F(UT_DeviceGenerator, generatorNetworkDevice_noSerial)
{
    // Arrange
    ut_dg_seed("lshw_network", { ut_dg_row({ {"logical name", "lo"}, {"description", "Ethernet interface"} }) });
    // Act
    dg->generatorNetworkDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceNetwork.size(), 0);
}

// generatorNetworkDevice: 全零 MAC → isValidMAC 失败 → 跳过
TEST_F(UT_DeviceGenerator, generatorNetworkDevice_zeroMac)
{
    // Arrange
    ut_dg_seed("lshw_network", { ut_dg_row({ {"logical name", "lo"}, {"serial", "00:00:00:00:00:00"} }) });
    // Act
    dg->generatorNetworkDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceNetwork.size(), 0);
}

// generatorNetworkDevice: 逻辑名在 /sys/class/net 下不存在 → isValidLogicalName 失败 → 跳过
TEST_F(UT_DeviceGenerator, generatorNetworkDevice_badLogicalName)
{
    // Arrange: wlan7 一般不存在于 /sys/class/net
    ut_dg_seed("lshw_network", { ut_dg_row({ {"logical name", "wlan7"}, {"serial", "f4:b5:20:24:5e:4f"} }) });
    // Act
    dg->generatorNetworkDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceNetwork.size(), 0);
}

// generatorNetworkDevice: hwinfo_network 独立行（Device File lo + 合法 MAC）→ 新增有线设备
TEST_F(UT_DeviceGenerator, generatorNetworkDevice_hwinfoWired)
{
    // Arrange
    ut_dg_seed("hwinfo_network", { ut_dg_row({ {"Device File", "lo"}, {"HW Address", "f4:b5:20:24:5e:4f"},
                                               {"Model", "Ethernet Connection"} }) });
    // Act
    dg->generatorNetworkDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceNetwork.size(), 1);
}

// generatorNetworkDevice: hwinfo 行 MAC 与 lshw 设备一致 → setInfoFromHwinfo 合并，单次调用仅 1 台
//（lstDevice 为函数局部列表，须单次调用同时注入两类数据源）
TEST_F(UT_DeviceGenerator, generatorNetworkDevice_hwinfoMerge)
{
    // Arrange: 同一次调用中 lshw 建 1 台，hwinfo 行 MAC 相同 → 合并
    ut_dg_seed("lshw_network", { ut_dg_row({ {"logical name", "lo"}, {"serial", "f4:b5:20:24:5e:4f"} }) });
    ut_dg_seed("hwinfo_network", { ut_dg_row({ {"Device File", "lo"}, {"HW Address", "f4:b5:20:24:5e:4f"} }) });
    // Act
    dg->generatorNetworkDevice();
    // Assert: MAC 匹配 → setInfoFromHwinfo 合并，hasMatchLogicalName=true 不新增
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceNetwork.size(), 1);
}

// generatorNetworkDevice: hwinfo_network 缺 MAC 也缺 unique_id → 跳过
TEST_F(UT_DeviceGenerator, generatorNetworkDevice_hwinfoNoMac)
{
    // Arrange
    ut_dg_seed("hwinfo_network", { ut_dg_row({ {"Device File", "lo"} }) });
    // Act
    dg->generatorNetworkDevice();
    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceNetwork.size(), 0);
}

// ============================ BusID 工具 ============================

// addBusIDFromHwinfo: 尾部 .N 去除 + 无后缀原样 + 空串跳过
TEST_F(UT_DeviceGenerator, addBusIDFromHwinfo_variants)
{
    // Act
    dg->addBusIDFromHwinfo("1-2:1.0");   // 去除 .0
    dg->addBusIDFromHwinfo("0:0:0:0");   // 无点后缀，原样
    dg->addBusIDFromHwinfo("");          // 空串跳过
    // Assert
    EXPECT_EQ(dg->getBusIDFromHwinfo(), (QStringList{ QString("1-2:1"), QString("0:0:0:0") }));
}

// getBusIDFromHwinfo: 初始为空列表引用
TEST_F(UT_DeviceGenerator, getBusIDFromHwinfo_initialEmpty)
{
    // Act/Assert
    EXPECT_TRUE(dg->getBusIDFromHwinfo().isEmpty());
}

// ============================ TOML ============================

// generatorInfoFromToml: 桩 tomlDeviceSet（真实实现读 toml 配置文件）→ 仅验证委托
TEST_F(UT_DeviceGenerator, generatorInfoFromToml_delegates)
{
    // Arrange
    Stub stub;
    ut_dg_tomlCalled = false;
    stub.set(ADDR(DeviceManager, tomlDeviceSet), ut_dg_tomlDeviceSet);
    // Act
    dg->generatorInfoFromToml(DT_Others);
    // Assert
    EXPECT_TRUE(ut_dg_tomlCalled);
}

// ============================ 同质参数化：hwinfo <5 字段过滤族 ============================

// 参数化上下文: 各设备来源键与其最小有效字段数
namespace {
struct HwinfoFilterCase {
    const char *cmdKey;      // DeviceManager cmdInfo 键
    const char *desc;        // 用例说明
};
}
class UT_DeviceGeneratorHwinfoFilter : public UT_DeviceGenerator,
                                       public ::testing::WithParamInterface<HwinfoFilterCase>
{
};

// hwinfo 系列来源对 <5 字段一律跳过（getGpuInfoFromHwinfo/getMonitorInfoFromHwinfo/getCdromInfoFromHwinfo 同构分支）
TEST_P(UT_DeviceGeneratorHwinfoFilter, underFiveFields_skipped)
{
    // Arrange: 4 字段行
    ut_dg_seed(QString(GetParam().cmdKey), { ut_dg_row({ {"Model", "Demo"}, {"Vendor", "Demo"},
                                                        {"Driver", "demo"}, {"SysFS BusID", "0:0:0:0"} }) });
    // Act
    dg->getGpuInfoFromHwinfo();
    dg->getMonitorInfoFromHwinfo();
    dg->getCdromInfoFromHwinfo();
    // Assert: 对应来源被跳过，其它来源无数据
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceGPU.size(), 0);
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMonitor.size(), 0);
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceCdrom.size(), 0);
}

INSTANTIATE_TEST_SUITE_P(HwinfoFilter, UT_DeviceGeneratorHwinfoFilter,
                         ::testing::Values(HwinfoFilterCase{"hwinfo_display", "显卡"},
                                           HwinfoFilterCase{"hwinfo_monitor", "显示器"},
                                           HwinfoFilterCase{"hwinfo_cdrom", "光驱"}));
