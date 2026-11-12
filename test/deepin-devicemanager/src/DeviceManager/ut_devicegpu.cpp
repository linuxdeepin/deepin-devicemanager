// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * UT_DeviceGpu — DeviceGpu 单元测试（qt-autotest-generator Mode 2 重写增强）
 *
 * 打桩决策（依据 skill 依赖矩阵：外部 IO / 子进程 / 设备环境一律 stub）：
 *  - Common::executeClientCmd —— 真实实现会起 modinfo 子进程，一律 stub（返回空=核内驱动）
 *  - QFile::open（dlsym 符号地址打桩）/ QIODevice::readAll —— setHwinfoInfo 读 /sys、/proc 文件，一律 stub
 *  - QFile::exists —— /proc/gpuinfo_0 探测，一律 stub
 *  - Common::boardVendorType —— 真实实现会读 DMI（checkBoardVendorFlag），一律 stub
 *  - DeviceMonitor::available —— 虚函数无法用本项目 stub.h 打桩（vcall thunk 段错误），
 *    不可用分支用 UT_GpuUnavailable 子类多态重写 available() 验证
 * 其余（setAttribute/setHwinfoLshwKey/matchToLshw/mapInfoToList/getOtherMapInfo 等）
 * 均为纯内存逻辑，使用真实实现。
 *
 * 用例计数声明（min 按技能 §4 level/factors 下限；actual 为本文件实际用例数）：
 *
 * | 方法 | level | factors | min | actual |
 * |----------------------------|------|----------------------------------------------------|-----|--------|
 * | DeviceGpu | mid | in_degree:1 | 2 | 2 |
 * | appendExtraInfoToOtherInfo | mid | alloc_in_loop:1;in_degree:1 | 3 | 3 |
 * | driver | low | - | 1 | 1 |
 * | getOverviewInfo | low | - | 1 | 2 |
 * | initFilterKey | mid | in_degree:1 | 2 | 2 |
 * | loadBaseDeviceInfo | low | - | 1 | 1 |
 * | loadOtherDeviceInfo | high | transitive_loop_depth:3 | 4 | 4 |
 * | loadTableData | low | - | 1 | 2 |
 * | name | low | - | 1 | 1 |
 * | setDmesgInfo | mid | in_degree:1 | 2 | 5 |
 * | setGpuInfo | mid | in_degree:1 | 2 | 2 |
 * | setGpuInfoByCustom | high | complexity:6;cognitive:21;in_degree:1 | 3 | 4 |
 * | setHwinfoInfo | high | complexity:16;cognitive:37;lines:119;transitive_loop_depth:3 | 4 | 13 |
 * | setInfoFromTomlOneByOne | mid | in_degree:2 | 2 | 3 |
 * | setLshwInfo | mid | in_degree:1 | 2 | 3 |
 * | setXrandrInfo | mid | complexity:6;in_degree:1 | 2 | 2 |
 * | subTitle | low | - | 1 | 1 |
 * | vendor | low | - | 1 | 1 |
 */

// 分支清单（来源：DeviceGpu::setHwinfoInfo）
// B1: m_Name 为空，或(SubDevice 存在且 Device 以 pci 开头且 SubDevice 非 pci) → 名称改取 SubDevice
// B2: m_Model 为空且 m_Name 非空且不以 pci 开头 → m_Model = m_Name
// B3: 驱动为空或 driverIsKernelIn 为 true → m_CanUninstall = false（核内驱动不可卸载）
// B4: Unique ID 匹配 [a-zA-Z0-9_+-]{4}\.(.*) → m_UniqueID 取捕获组
// B5: SysFS ID 非空 → 读 /sys<SysFS ID>/gpu-info
// B6: gpu-info 打开失败 → 跳过显存解析，m_GraphicsMemory 保持原值
// B7: 文件行为空 → continue
// B8: 行按 ":" 分段数 != 2 → continue
// B9: 键为 "VRAM total size" → 尝试十六进制解析显存
// B10: 解析成功且 vramSize >= 1048576 → 进入换算；否则忽略
// B11: 换算后 curSize > 1 → 单位 GB；否则 → 单位 MB
// B12: VID_PID 含 0731 且 /proc/gpuinfo_0 存在 → 读 JJW 显存文件
// B13: JJW 文件打开成功 → 逐行解析；打开失败 → 跳过
// B14: JJW 文件行为空 → continue
// B15: JJW 行分段数 != 2 → continue
// B16: 键为 "Memory Size" → m_GraphicsMemory 直接赋值并 break

// 分支清单（来源：DeviceGpu::setGpuInfoByCustom）
// B1: 遍历自定义 map 全部键值（for）
// B2: key=="Name" → m_Name
// B3: key=="Vendor" → m_Vendor
// B4: key=="Model" → m_Model
// B5: key=="Version" → m_Version；key=="Graphics Memory" → m_GraphicsMemory
// B6: 其他 key → 存入 m_extraInfo（追加到其他信息）

// 分支清单（来源：DeviceGpu::setLshwInfo）
// B1: matchToLshw 匹配失败（非同一 GPU）→ return，不设置任何属性
// B2: (m_Name 为空或以 pci 开头) 且 map 含非 pci 的 product → m_Model 取 product
// B3: 驱动为核内驱动（driverIsKernelIn）或驱动为空 → m_CanUninstall = false

// 分支清单（来源：DeviceGpu::setDmesgInfo）
// B1: map 含 BusID 且后缀 != m_HwinfoToLshw → return（非同一总线设备，早退）
// B2: map["Size"] 含 "null" → 显存取 "null=" 之后内容（Bug-85049 JJW 特殊处理）
// B3: map 含 Device 且非空且 m_Name 以 pci 开头 → m_Name/m_Model 取 Device 并写入其他信息
// B4: map["Size"] 含 m_HwinfoToLshw → 显存取该总线 ID 前缀之后内容

// 分支清单（来源：DeviceGpu::setXrandrInfo）
// B1: map 含 "HDMI" → m_HDMI
// B2: map 含 "VGA" → m_VGA
// B3: map 含 "DP" → m_DisplayPort
// B4: map 含 "eDP" → m_eDP
// B5: map 含 "DVI" → m_DVI
// B6: map 含 "DigitalOutput" → m_Digital（bug-105482 添加新接口类型）

#include "DeviceGpu.h"
#include "commonfunction.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QFile>
#include <QIODevice>
#include <QPaintEvent>
#include <QPainter>

#include <dlfcn.h>

#include <gtest/gtest.h>

/*
 * QFile::open 为 Qt 虚函数，本项目 stub.h 无法通过成员函数指针（vcall thunk）打桩
 * （实测必段错误）。改用 dlsym 取 QFile::open 导出符号地址，对真实函数体打桩：
 *   _ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE
 */
static char *ut_gpu_qfileOpenAddr()
{
    static char *addr = (char *)dlsym(RTLD_DEFAULT, "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    return addr;
}

class UT_DeviceGpu : public ::testing::Test
{
public:
    void SetUp()
    {
        m_deviceGpu = new DeviceGpu;
    }
    void TearDown()
    {
        delete m_deviceGpu;
    }
    DeviceGpu *m_deviceGpu;
};

/* ===================== 打桩辅助 ===================== */

// Common::executeClientCmd 桩：返回值由用例控制（空=核内驱动；含 filename:=核外驱动）
static QByteArray ut_gpu_execCmdRet;
static QByteArray ut_gpu_execCmd_stub(const QString &, const QStringList &, const QString &, int, bool)
{
    return ut_gpu_execCmdRet;
}

// QIODevice::open 桩（成员函数，替换为零参自由函数，与框架约定一致）
static bool ut_gpu_fileOpenRet = false;
static bool ut_gpu_open_stub()
{
    return ut_gpu_fileOpenRet;
}

// QIODevice::readAll 桩：返回用例预设内容
static QByteArray ut_gpu_fileContent;
static QByteArray ut_gpu_readAll_stub()
{
    return ut_gpu_fileContent;
}

// QFile::exists 桩（静态函数，带同型签名）
static bool ut_gpu_fileExistsRet = false;
static bool ut_gpu_exists_stub(const QString &)
{
    return ut_gpu_fileExistsRet;
}

// Common::boardVendorType 桩：避免真实 DMI 读取
static QString ut_gpu_boardVendorRet;
static QString ut_gpu_boardVendor_stub()
{
    return ut_gpu_boardVendorRet;
}

// DeviceMonitor::available 为虚函数，本项目 stub.h 无法打桩（vcall thunk 段错误，
// 同 QIODevice::open）。不可用分支改用多态：子类重写 available() 返回 false。
class UT_GpuUnavailable : public DeviceGpu
{
public:
    bool available() override { return false; }
};

/* ===================== 测试数据填充 ===================== */

void ut_gpu_fill_lshw(QMap<QString, QString> &mapLshw)
{
    mapLshw.insert("product", "GK208B [GeForce GT 730]");
    mapLshw.insert("vendor", "NVIDIA Corporation");
    mapLshw.insert("bus info", "pci@0000:01:00.0");
    mapLshw.insert("version", "a1");
    mapLshw.insert("width", "64 bits");
    mapLshw.insert("clock", "33MHz");
    mapLshw.insert("irq", "126");
    mapLshw.insert("capabilities", "pm msi pciexpress vga_controller bus_master cap_list rom");
    mapLshw.insert("description", "VGA compatible controller");
    mapLshw.insert("driver", "nouveau");
    mapLshw.insert("ioport", "e000(size=128)");
    mapLshw.insert("memory", "de000000-deffffff  d0000000-d7ffffff  d8000000-d9ffffff  c0000-dffff");
    mapLshw.insert("physical id", "0");
}

void ut_gpu_fill_hwinfo(QMap<QString, QString> &mapHwinfo)
{
    mapHwinfo.insert("Vendor", "nVidia Corporation");
    mapHwinfo.insert("Device", "GK208B [GeForce GT 730]");
    mapHwinfo.insert("Model", "nVidia GK208B [GeForce GT 730]");
    mapHwinfo.insert("Revision", "0xa1");
    mapHwinfo.insert("IRQ", "126 (681 events)");
    mapHwinfo.insert("Driver", "nouveau");
    mapHwinfo.insert("Width", "64 bits");
    mapHwinfo.insert("SysFS BusID", "0000:01:00.0");
    mapHwinfo.insert("SysFS ID", "/devices/pci0000:00/0000:00:01.0/0000:01:00.0");
}

void ut_gpu_fill_gpuinfo(QMap<QString, QString> &mapGpu)
{
    mapGpu.insert("Name", "GK208B");
}

/* ===================== 基础方法 ===================== */

// 等价类：过滤键初始化（白盒：initFilterKey 固定添加 13 个键）
TEST_F(UT_DeviceGpu, UT_DeviceGpu_initFilterKey)
{
    // Arrange：构造后已含初始键集
    const int initialSize = m_deviceGpu->m_FilterKey.size();

    // Act：重复调用保证幂等
    m_deviceGpu->initFilterKey();

    // Assert：键数量与代表键内容（源码实际键集，不含 Vendor）
    EXPECT_EQ(13, m_deviceGpu->m_FilterKey.size());
    EXPECT_EQ(initialSize, 13);
    EXPECT_TRUE(m_deviceGpu->m_FilterKey.contains("Device"));
    EXPECT_TRUE(m_deviceGpu->m_FilterKey.contains("GL version"));
}

// 等价类：构造默认状态（构造即完成 initFilterKey，名称为空）
TEST_F(UT_DeviceGpu, UT_DeviceGpu_DeviceGpu_InitialDefaults)
{
    // Arrange：SetUp 已构造新对象
    ASSERT_NE(m_deviceGpu, nullptr);

    // Act
    QString name = m_deviceGpu->name();

    // Assert：默认名称为空，过滤键已初始化
    EXPECT_STREQ("", name.toStdString().c_str());
    EXPECT_EQ(13, m_deviceGpu->m_FilterKey.size());
}

// 等价类：构造后动态成员为空
TEST_F(UT_DeviceGpu, UT_DeviceGpu_DeviceGpu_EmptyContainers)
{
    // Arrange：SetUp 已构造新对象
    ASSERT_NE(m_deviceGpu, nullptr);

    // Act：读取初始状态
    const bool extraEmpty = m_deviceGpu->m_extraInfo.isEmpty();

    // Assert：容器成员均为空
    EXPECT_TRUE(extraEmpty);
    EXPECT_STREQ("", m_deviceGpu->m_HwinfoToLshw.toStdString().c_str());
    EXPECT_EQ(0, m_deviceGpu->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadBaseDeviceInfo)
{
    // Arrange：hwinfo + dmesg 显存组合
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_hwinfo(mapinfo);
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);   // 阻断 modinfo 子进程
    ut_gpu_execCmdRet = QByteArray();
    m_deviceGpu->setHwinfoInfo(mapinfo);
    QMap<QString, QString> DmesgInfoMap;
    DmesgInfoMap.insert("Size", "0000:01:00.0=2GB");
    m_deviceGpu->setDmesgInfo(DmesgInfoMap);

    // Act
    m_deviceGpu->loadBaseDeviceInfo();

    // Assert：基础信息按序填充
    ASSERT_GE(m_deviceGpu->m_LstBaseInfo.size(), 5);
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_LstBaseInfo.at(0).second.toStdString().c_str());
    EXPECT_STREQ("nVidia Corporation", m_deviceGpu->m_LstBaseInfo.at(1).second.toStdString().c_str());
    EXPECT_STREQ("nVidia GK208B [GeForce GT 730]", m_deviceGpu->m_LstBaseInfo.at(2).second.toStdString().c_str());
    EXPECT_STREQ("0xa1", m_deviceGpu->m_LstBaseInfo.at(3).second.toStdString().c_str());
    EXPECT_STREQ("2GB", m_deviceGpu->m_LstBaseInfo.at(4).second.toStdString().c_str());
}

/* ===================== setLshwInfo ===================== */

// 分支 B1~B3：同 GPU 匹配，属性全量设置；核内驱动 → 不可卸载
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setLshwInfo_001)
{
    // Arrange：m_HwinfoToLshw 与 lshw bus info 后缀一致（同 GPU）；modinfo 桩返回空 → 核内驱动
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_lshw(mapinfo);
    m_deviceGpu->m_HwinfoToLshw = "0000:01:00.0";
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();

    // Act
    m_deviceGpu->setLshwInfo(mapinfo);

    // Assert
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("NVIDIA Corporation", m_deviceGpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("a1", m_deviceGpu->m_Version.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceGpu->m_Width.toStdString().c_str());
    EXPECT_STREQ("33MHz", m_deviceGpu->m_Clock.toStdString().c_str());
    EXPECT_STREQ("126", m_deviceGpu->m_IRQ.toStdString().c_str());
    EXPECT_STREQ("pm msi pciexpress vga_controller bus_master cap_list rom", m_deviceGpu->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("VGA compatible controller", m_deviceGpu->m_Description.toStdString().c_str());
    EXPECT_STREQ("nouveau", m_deviceGpu->m_Driver.toStdString().c_str());
    EXPECT_STREQ("pci@0000:01:00.0", m_deviceGpu->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("e000(size=128)", m_deviceGpu->m_IOPort.toStdString().c_str());
    EXPECT_STREQ("de000000-deffffff  d0000000-d7ffffff  d8000000-d9ffffff  c0000-dffff", m_deviceGpu->m_MemAddress.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_PhysID.toStdString().c_str());
    EXPECT_FALSE(m_deviceGpu->m_CanUninstall);   // B3：核内驱动
}

// 分支 B1：非同一 GPU → 早退，所有属性保持为空
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setLshwInfo_002)
{
    // Arrange：m_HwinfoToLshw 与 bus info 不一致
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_lshw(mapinfo);
    m_deviceGpu->m_HwinfoToLshw = "02:00.0";

    // Act
    m_deviceGpu->setLshwInfo(mapinfo);

    // Assert：早退，全部为空
    EXPECT_STREQ("", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Driver.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_MemAddress.toStdString().c_str());
}

// 等价类：m_Name 已有非 pci 名称时，product 不覆盖 m_Name（setAttribute overwrite=false 语义）
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setLshwInfo_NameKept_WhenAlreadySet)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_lshw(mapinfo);
    m_deviceGpu->m_HwinfoToLshw = "0000:01:00.0";
    m_deviceGpu->m_Name = "Existing GPU Name";
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();

    // Act
    m_deviceGpu->setLshwInfo(mapinfo);

    // Assert：名称保留原值，其余属性正常设置（lshw 路径厂商已规范化为大写 NVIDIA）
    EXPECT_STREQ("Existing GPU Name", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("NVIDIA Corporation", m_deviceGpu->m_Vendor.toStdString().c_str());
}

/* ===================== setHwinfoInfo ===================== */

// 常规路径：各属性按 map 设置
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo)
{
    // Arrange：SysFS ID 预置 → 必须阻断 gpu-info 文件打开
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_hwinfo(mapinfo);
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    stub.set(ut_gpu_qfileOpenAddr(), ut_gpu_open_stub);
    ut_gpu_execCmdRet = QByteArray();
    ut_gpu_fileOpenRet = false;   // 文件打开失败 → 显存解析跳过

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert
    EXPECT_STREQ("nVidia Corporation", m_deviceGpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("nVidia GK208B [GeForce GT 730]", m_deviceGpu->m_Model.toStdString().c_str());
    EXPECT_STREQ("0xa1", m_deviceGpu->m_Version.toStdString().c_str());
    EXPECT_STREQ("126 (681 events)", m_deviceGpu->m_IRQ.toStdString().c_str());
    EXPECT_STREQ("nouveau", m_deviceGpu->m_Driver.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceGpu->m_Width.toStdString().c_str());
    EXPECT_STREQ("0000:01:00.0", m_deviceGpu->m_HwinfoToLshw.toStdString().c_str());
    EXPECT_FALSE(m_deviceGpu->m_CanUninstall);   // B3：核内驱动
    EXPECT_TRUE(m_deviceGpu->setHwinfoInfo(mapinfo));   // 返回值恒为 true
}

// 分支 B1：Device 为 pci 占位且 SubDevice 有效 → 名称取 SubDevice
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_SubDeviceOverridesPciName)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "pci1024:1b38");
    mapinfo.insert("SubDevice", "GK208B [GeForce GT 730]");
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：B1 命中，名称来自 SubDevice；B2 随之命中，Model 取新名称
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_Model.toStdString().c_str());
}

// 分支 B2：Model 为空且名称非 pci → m_Model = m_Name
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_ModelFromName_WhenModelEmpty)
{
    // Arrange：不提供 Model
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "GK208B [GeForce GT 730]");
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：B2 命中
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_Model.toStdString().c_str());
}

// 分支 B5/B9/B10/B11：gpu-info 显存十六进制 0x80000000 = 2048MiB → curSize=2.0 → "2GB"
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_VramGB_FromGpuInfoFile)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "Test GPU");
    mapinfo.insert("SysFS ID", "/devices/pci0000:00/0000:00:01.0/0000:01:00.0");
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    stub.set(ut_gpu_qfileOpenAddr(), ut_gpu_open_stub);
    stub.set(static_cast<QByteArray (QIODevice::*)()>(&QIODevice::readAll), ut_gpu_readAll_stub);
    ut_gpu_execCmdRet = QByteArray();
    ut_gpu_fileOpenRet = true;
    ut_gpu_fileContent = QByteArray("VRAM total size: 80000000\n");

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：B11 GB 分支；名称已按 Device 设置
    EXPECT_STREQ("2GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("Test GPU", m_deviceGpu->m_Name.toStdString().c_str());
}

// 分支 B5/B9/B10/B11：0x1400000 = 20MiB → curSize<1 → "20MB"
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_VramMB_FromGpuInfoFile)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "Test GPU");
    mapinfo.insert("SysFS ID", "/devices/pci0000:00/0000:00:01.0/0000:01:00.0");
    Stub stub;
    stub.set(ut_gpu_qfileOpenAddr(), ut_gpu_open_stub);
    stub.set(static_cast<QByteArray (QIODevice::*)()>(&QIODevice::readAll), ut_gpu_readAll_stub);
    ut_gpu_fileOpenRet = true;
    ut_gpu_fileContent = QByteArray("VRAM total size: 1400000\n");

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：B11 MB 分支；名称已按 Device 设置
    EXPECT_STREQ("20MB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("Test GPU", m_deviceGpu->m_Name.toStdString().c_str());
}

// 分支 B10 边界：解析出的值 < 1048576 → 忽略，显存保持为空
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_VramTooSmall_Ignored)
{
    // Arrange：0x100 = 256 字节，低于阈值
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "Test GPU");
    mapinfo.insert("SysFS ID", "/devices/pci0000:00/0000:00:01.0/0000:01:00.0");
    Stub stub;
    stub.set(ut_gpu_qfileOpenAddr(), ut_gpu_open_stub);
    stub.set(static_cast<QByteArray (QIODevice::*)()>(&QIODevice::readAll), ut_gpu_readAll_stub);
    ut_gpu_fileOpenRet = true;
    ut_gpu_fileContent = QByteArray("VRAM total size: 100\n");

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：未达阈值不设置；名称不受影响
    EXPECT_STREQ("", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("Test GPU", m_deviceGpu->m_Name.toStdString().c_str());
}

// 分支 B6：gpu-info 打开失败 → 已有显存值不被破坏（强异常安全）
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_GpuInfoOpenFail_KeepsMemory)
{
    // Arrange：先用 dmesg 设置显存，再执行文件打开失败的 hwinfo 流程
    QMap<QString, QString> dmesgMap;
    dmesgMap.insert("Size", "0000:01:00.0=4GB");
    m_deviceGpu->m_HwinfoToLshw = "0000:01:00.0";
    m_deviceGpu->setDmesgInfo(dmesgMap);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "Test GPU");
    mapinfo.insert("SysFS ID", "/devices/pci0000:00/0000:00:01.0/0000:01:00.0");
    Stub stub;
    stub.set(ut_gpu_qfileOpenAddr(), ut_gpu_open_stub);
    ut_gpu_fileOpenRet = false;   // B6

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：显存保持 dmesg 设置值；名称已按 Device 设置
    EXPECT_STREQ("4GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("Test GPU", m_deviceGpu->m_Name.toStdString().c_str());
}

// 分支 B12/B13/B16：JJW 设备（VID_PID 含 0731）+ /proc/gpuinfo_0 存在 → "Memory Size" 直接赋值
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_JJWFile_MemorySize)
{
    // Arrange：SysFS ID 留空 → 只走 JJW 路径
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "JJW GPU");
    mapinfo.insert("VID_PID", "0731:0001");
    Stub stub;
    stub.set(ut_gpu_qfileOpenAddr(), ut_gpu_open_stub);
    stub.set(static_cast<QByteArray (QIODevice::*)()>(&QIODevice::readAll), ut_gpu_readAll_stub);
    stub.set(static_cast<bool (*)(const QString &)>(&QFile::exists), ut_gpu_exists_stub);
    ut_gpu_fileOpenRet = true;
    ut_gpu_fileExistsRet = true;
    ut_gpu_fileContent = QByteArray("Memory Size: 8GB\n");

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：B16 命中；名称已按 Device 设置
    EXPECT_STREQ("8GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("JJW GPU", m_deviceGpu->m_Name.toStdString().c_str());
}

// 分支 B12 逆条件：VID_PID 含 0731 但文件不存在 → 显存保持原值
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_JJWFileMissing_KeepsMemory)
{
    // Arrange：先经 dmesg 设置显存
    QMap<QString, QString> dmesgMap;
    dmesgMap.insert("Size", "0000:01:00.0=6GB");
    m_deviceGpu->m_HwinfoToLshw = "0000:01:00.0";
    m_deviceGpu->setDmesgInfo(dmesgMap);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "JJW GPU");
    mapinfo.insert("VID_PID", "0731:0001");
    Stub stub;
    stub.set(static_cast<bool (*)(const QString &)>(&QFile::exists), ut_gpu_exists_stub);
    ut_gpu_fileExistsRet = false;

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：文件不存在，不覆盖；名称已按 Device 设置
    EXPECT_STREQ("6GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("JJW GPU", m_deviceGpu->m_Name.toStdString().c_str());
}

// 分支 B12 逆条件：VID_PID 不含 0731 → 不探测 JJW 文件
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_VidPidNotJJW_NoProbe)
{
    // Arrange：VID_PID 无 0731，exists 桩返回 true 也不应被读取（open 桩返回 false 兜底）
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "Normal GPU");
    mapinfo.insert("VID_PID", "10de:1b38");
    Stub stub;
    stub.set(ut_gpu_qfileOpenAddr(), ut_gpu_open_stub);
    stub.set(static_cast<QByteArray (QIODevice::*)()>(&QIODevice::readAll), ut_gpu_readAll_stub);
    stub.set(static_cast<bool (*)(const QString &)>(&QFile::exists), ut_gpu_exists_stub);
    ut_gpu_fileOpenRet = false;
    ut_gpu_fileExistsRet = true;
    ut_gpu_fileContent = QByteArray("Memory Size: 999GB\n");

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：非 JJW 设备不读 JJW 文件；名称已按 Device 设置
    EXPECT_STREQ("", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("Normal GPU", m_deviceGpu->m_Name.toStdString().c_str());
}

// 分支 B4：Unique ID 匹配正则 → m_UniqueID 取捕获组
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_UniqueID_Captured)
{
    // Arrange："usb1" 4 字符 + "." + "abc"
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "Test GPU");
    mapinfo.insert("Unique ID", "usb1.abc");
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：捕获 "." 之后内容；名称已按 Device 设置
    EXPECT_STREQ("abc", m_deviceGpu->m_UniqueID.toStdString().c_str());
    EXPECT_STREQ("Test GPU", m_deviceGpu->m_Name.toStdString().c_str());
}

// 分支 B4 边界：Unique ID 不匹配正则 → m_UniqueID 保持为空
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_UniqueID_NoMatch_KeepsEmpty)
{
    // Arrange："pci" 仅 3 个合规字符，无法满足 {4}\. 结构
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "Test GPU");
    mapinfo.insert("Unique ID", "pci.1234");
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：不捕获；名称已按 Device 设置
    EXPECT_STREQ("", m_deviceGpu->m_UniqueID.toStdString().c_str());
    EXPECT_STREQ("Test GPU", m_deviceGpu->m_Name.toStdString().c_str());
}

// 等价类：驱动为空 → m_CanUninstall = false（B3 另一析取支）
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setHwinfoInfo_EmptyDriver_NotUninstallable)
{
    // Arrange：不提供 Driver 键
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "NoDriver GPU");

    // Act
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Assert：驱动为空按不可卸载处理
    EXPECT_FALSE(m_deviceGpu->m_CanUninstall);
    EXPECT_STREQ("", m_deviceGpu->m_Driver.toStdString().c_str());
}

/* ===================== setDmesgInfo ===================== */

// 分支 B4：Size 含 m_HwinfoToLshw → 显存取前缀后内容
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setDmesgInfo_001)
{
    // Arrange
    m_deviceGpu->m_HwinfoToLshw = "01:00.0";
    QMap<QString, QString> DmesgInfoMap;
    DmesgInfoMap.insert("Size", "01:00.0=2GB");

    // Act
    m_deviceGpu->setDmesgInfo(DmesgInfoMap);

    // Assert：显存取 "=" 右侧内容
    EXPECT_STREQ("2GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());

    // Act：再次赋值验证为覆盖而非追加
    DmesgInfoMap["Size"] = "01:00.0=4GB";
    m_deviceGpu->setDmesgInfo(DmesgInfoMap);

    // Assert：显存更新为新值
    EXPECT_STREQ("4GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
}

// 分支 B2：Size 含 null → Bug-85049 JJW 特殊处理
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setDmesgInfo_002)
{
    // Arrange
    m_deviceGpu->m_HwinfoToLshw = "01:00.0";
    QMap<QString, QString> DmesgInfoMap;
    DmesgInfoMap.insert("Size", "null=3GB");

    // Act
    m_deviceGpu->setDmesgInfo(DmesgInfoMap);

    // Assert：null 前缀特殊处理取 "=" 右侧；名称不受影响
    EXPECT_STREQ("3GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Name.toStdString().c_str());
}

// 分支 B1：BusID 后缀与 m_HwinfoToLshw 不一致 → 早退，Size 也不再处理
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setDmesgInfo_BusIdMismatch_EarlyReturn)
{
    // Arrange：BusID 指向另一总线设备
    m_deviceGpu->m_HwinfoToLshw = "01:00.0";
    QMap<QString, QString> DmesgInfoMap;
    DmesgInfoMap.insert("BusID", "pci@0000:02:00.0");
    DmesgInfoMap.insert("Size", "01:00.0=2GB");

    // Act
    m_deviceGpu->setDmesgInfo(DmesgInfoMap);

    // Assert：早退，显存未被设置，名称也未处理
    EXPECT_STREQ("", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Name.toStdString().c_str());
}

// 分支 B3：m_Name 以 pci 开头且 Device 有效 → m_Name/m_Model 取 Device
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setDmesgInfo_DevicePciBranch)
{
    // Arrange：经 hwinfo 流程把 m_Name 置为 pci 占位
    QMap<QString, QString> hwinfoMap;
    hwinfoMap.insert("Device", "pci1024:1b38");
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();
    m_deviceGpu->setHwinfoInfo(hwinfoMap);
    ASSERT_TRUE(m_deviceGpu->m_Name.startsWith("pci"));

    QMap<QString, QString> dmesgMap;
    dmesgMap.insert("Device", "GK208B [GeForce GT 730]");

    // Act
    m_deviceGpu->setDmesgInfo(dmesgMap);

    // Assert：B3 命中
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_Model.toStdString().c_str());
}

// 边界：Size 缺失（QMap operator[] 返回空串）→ 不崩溃、不设置
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setDmesgInfo_NoSizeKey_NoCrash)
{
    // Arrange：仅含无关键
    QMap<QString, QString> DmesgInfoMap;
    DmesgInfoMap.insert("Other", "value");

    // Act
    EXPECT_NO_FATAL_FAILURE(m_deviceGpu->setDmesgInfo(DmesgInfoMap));

    // Assert：不崩溃、显存与名称均保持空
    EXPECT_STREQ("", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Name.toStdString().c_str());
}

/* ===================== setGpuInfo ===================== */

TEST_F(UT_DeviceGpu, UT_DeviceGpu_setGpuInfo)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_gpuinfo(mapinfo);

    // Act
    m_deviceGpu->setGpuInfo(mapinfo);

    // Assert：接口信息保持空，名称被覆盖
    EXPECT_STREQ("", m_deviceGpu->m_HDMI.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_VGA.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_DisplayPort.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_eDP.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_DVI.toStdString().c_str());
    EXPECT_STREQ("GK208B", m_deviceGpu->m_Name.toStdString().c_str());
}

// 等价类：空 map → 名称与接口成员均保持空
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setGpuInfo_EmptyMap_KeepsEmpty)
{
    // Arrange：空 map
    QMap<QString, QString> emptyMap;

    // Act
    m_deviceGpu->setGpuInfo(emptyMap);

    // Assert：空 map → 名称与接口成员均保持空
    EXPECT_STREQ("", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_HDMI.toStdString().c_str());
}

/* ===================== setGpuInfoByCustom ===================== */

// 分支 B2~B5：标准键直接写入成员
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setGpuInfoByCustom_FullMap)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "Custom GPU");
    mapInfo.insert("Vendor", "Custom Vendor");
    mapInfo.insert("Model", "Custom Model");
    mapInfo.insert("Version", "v1.0");
    mapInfo.insert("Graphics Memory", "16GB");

    // Act
    m_deviceGpu->setGpuInfoByCustom(mapInfo);

    // Assert：成员逐一命中
    EXPECT_STREQ("Custom GPU", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("Custom Vendor", m_deviceGpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Custom Model", m_deviceGpu->m_Model.toStdString().c_str());
    EXPECT_STREQ("v1.0", m_deviceGpu->m_Version.toStdString().c_str());
    EXPECT_STREQ("16GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_TRUE(m_deviceGpu->m_extraInfo.isEmpty());   // 无额外键
}

// 分支 B6：非标准键进入 m_extraInfo
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setGpuInfoByCustom_UnknownKeysToExtraInfo)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Custom Key", "Custom Value");
    mapInfo.insert("Notes", "some notes");

    // Act
    m_deviceGpu->setGpuInfoByCustom(mapInfo);

    // Assert
    EXPECT_EQ(2, m_deviceGpu->m_extraInfo.size());
    EXPECT_STREQ("Custom Value", m_deviceGpu->m_extraInfo["Custom Key"].toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Name.toStdString().c_str());   // 标准成员不受影响
}

// 分支 B1 前置：调用前先 clear 历史 m_extraInfo
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setGpuInfoByCustom_ClearsPreviousExtraInfo)
{
    // Arrange：预置历史额外信息，再次调用时只含新键
    m_deviceGpu->m_extraInfo.insert("Old Key", "Old Value");
    QMap<QString, QString> mapInfo;
    mapInfo.insert("New Key", "New Value");

    // Act
    m_deviceGpu->setGpuInfoByCustom(mapInfo);

    // Assert：旧键清除，仅剩新键
    EXPECT_FALSE(m_deviceGpu->m_extraInfo.contains("Old Key"));
    EXPECT_EQ(1, m_deviceGpu->m_extraInfo.size());
    EXPECT_STREQ("New Value", m_deviceGpu->m_extraInfo["New Key"].toStdString().c_str());
}

// 等价类：空 map → 全部保持为空
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setGpuInfoByCustom_EmptyMap_NoChange)
{
    // Arrange：空 map
    QMap<QString, QString> emptyMap;

    // Act
    m_deviceGpu->setGpuInfoByCustom(emptyMap);

    // Assert
    EXPECT_STREQ("", m_deviceGpu->m_Name.toStdString().c_str());
    EXPECT_TRUE(m_deviceGpu->m_extraInfo.isEmpty());
}

/* ===================== appendExtraInfoToOtherInfo ===================== */

// 等价类：有效值追加、无效值（Unknown/空）过滤
TEST_F(UT_DeviceGpu, UT_DeviceGpu_appendExtraInfoToOtherInfo_ValidOnly)
{
    // Arrange：QMap 按键排序遍历
    m_deviceGpu->m_extraInfo.insert("Good", "value");
    m_deviceGpu->m_extraInfo.insert("Bad", "Unknown");
    m_deviceGpu->m_extraInfo.insert("Empty", "");

    // Act
    m_deviceGpu->appendExtraInfoToOtherInfo();

    // Assert：仅有效项追加
    ASSERT_EQ(1, m_deviceGpu->m_LstOtherInfo.size());
    EXPECT_STREQ("Good", m_deviceGpu->m_LstOtherInfo.at(0).first.toStdString().c_str());
    EXPECT_STREQ("value", m_deviceGpu->m_LstOtherInfo.at(0).second.toStdString().c_str());
}

// 等价类：无额外信息 → 不产生任何条目
TEST_F(UT_DeviceGpu, UT_DeviceGpu_appendExtraInfoToOtherInfo_EmptyExtra_NoEntries)
{
    // Arrange：m_extraInfo 为空
    ASSERT_TRUE(m_deviceGpu->m_extraInfo.isEmpty());

    // Act
    m_deviceGpu->appendExtraInfoToOtherInfo();

    // Assert：列表保持空
    EXPECT_EQ(0, m_deviceGpu->m_LstOtherInfo.size());
    EXPECT_TRUE(m_deviceGpu->m_LstOtherInfo.isEmpty());
}

// 等价类：多个有效项 → QMap 键序输出
TEST_F(UT_DeviceGpu, UT_DeviceGpu_appendExtraInfoToOtherInfo_MultipleValid_Ordered)
{
    // Arrange：两个有效项（QMap 按键排序：A 在 B 前）
    m_deviceGpu->m_extraInfo.insert("B Beta", "value-b");
    m_deviceGpu->m_extraInfo.insert("A Alpha", "value-a");

    // Act
    m_deviceGpu->appendExtraInfoToOtherInfo();

    // Assert：两个条目且按键序排列
    ASSERT_EQ(2, m_deviceGpu->m_LstOtherInfo.size());
    EXPECT_STREQ("A Alpha", m_deviceGpu->m_LstOtherInfo.at(0).first.toStdString().c_str());
    EXPECT_STREQ("B Beta", m_deviceGpu->m_LstOtherInfo.at(1).first.toStdString().c_str());
}

/* ===================== loadOtherDeviceInfo ===================== */

// 空设备：无有效值 → 其他信息为空（boardVendorType 打桩避免真实 DMI 读取）
TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadOtherDeviceInfo)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(Common, boardVendorType), ut_gpu_boardVendor_stub);
    ut_gpu_boardVendorRet = "Generic";

    // Act
    m_deviceGpu->loadOtherDeviceInfo();

    // Assert
    EXPECT_EQ(0, m_deviceGpu->m_LstOtherInfo.size());
    EXPECT_TRUE(m_deviceGpu->m_LstOtherInfo.isEmpty());
}

// 等价类：有值设备 → 分辨率等键进入其他信息（普通厂商含 Max/Min）
TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadOtherDeviceInfo_WithValues_IncludesResolutions)
{
    // Arrange：lshw + xrandr 组合；厂商打桩为普通厂商
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_lshw(mapinfo);
    m_deviceGpu->m_HwinfoToLshw = "0000:01:00.0";
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    stub.set(ADDR(Common, boardVendorType), ut_gpu_boardVendor_stub);
    ut_gpu_execCmdRet = QByteArray();
    ut_gpu_boardVendorRet = "Generic";
    m_deviceGpu->setLshwInfo(mapinfo);

    QMap<QString, QString> xrandrMap;
    xrandrMap.insert("minResolution", "640x480");
    xrandrMap.insert("curResolution", "1920x1080");
    xrandrMap.insert("maxResolution", "3840x2160");
    m_deviceGpu->setXrandrInfo(xrandrMap);

    // Act
    m_deviceGpu->loadOtherDeviceInfo();

    // Assert：普通厂商包含最大/最小/当前分辨率与驱动（其余项来自基础信息，共 10 项）
    EXPECT_EQ(10, m_deviceGpu->m_LstOtherInfo.size());
    EXPECT_TRUE(m_deviceGpu->m_LstOtherInfo.contains(QPair<QString, QString>("Driver", "nouveau")));
    EXPECT_TRUE(m_deviceGpu->m_LstOtherInfo.contains(QPair<QString, QString>("Maximum Resolution", "3840x2160")));
    EXPECT_TRUE(m_deviceGpu->m_LstOtherInfo.contains(QPair<QString, QString>("Minimum Resolution", "640x480")));
    EXPECT_TRUE(m_deviceGpu->m_LstOtherInfo.contains(QPair<QString, QString>("Current Resolution", "1920x1080")));
}

// 分支：specialHString 厂商 → 不含 Max/Min Resolution，仍含 Current
TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadOtherDeviceInfo_SpecialVendor_ExcludesResolutions)
{
    // Arrange：厂商打桩为 specialHString 值
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_lshw(mapinfo);
    m_deviceGpu->m_HwinfoToLshw = "0000:01:00.0";
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    stub.set(ADDR(Common, boardVendorType), ut_gpu_boardVendor_stub);
    ut_gpu_execCmdRet = QByteArray();
    ut_gpu_boardVendorRet = Common::specialHString();
    m_deviceGpu->setLshwInfo(mapinfo);

    QMap<QString, QString> xrandrMap;
    xrandrMap.insert("minResolution", "640x480");
    xrandrMap.insert("curResolution", "1920x1080");
    xrandrMap.insert("maxResolution", "3840x2160");
    m_deviceGpu->setXrandrInfo(xrandrMap);

    // Act
    m_deviceGpu->loadOtherDeviceInfo();

    // Assert：Max/Min 被排除，剩余 8 项为基础信息 + Current + Driver
    EXPECT_EQ(8, m_deviceGpu->m_LstOtherInfo.size());
    EXPECT_FALSE(m_deviceGpu->m_LstOtherInfo.contains(QPair<QString, QString>("Maximum Resolution", "3840x2160")));
    EXPECT_FALSE(m_deviceGpu->m_LstOtherInfo.contains(QPair<QString, QString>("Minimum Resolution", "640x480")));
    EXPECT_TRUE(m_deviceGpu->m_LstOtherInfo.contains(QPair<QString, QString>("Current Resolution", "1920x1080")));
}

// 分支：m_extraInfo 经 loadOtherDeviceInfo 追加到其他信息
TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadOtherDeviceInfo_AppendsExtraInfo)
{
    // Arrange：自定义键进入 m_extraInfo，再统一加载
    QMap<QString, QString> customMap;
    customMap.insert("Extra Key", "extra value");
    m_deviceGpu->setGpuInfoByCustom(customMap);
    Stub stub;
    stub.set(ADDR(Common, boardVendorType), ut_gpu_boardVendor_stub);
    ut_gpu_boardVendorRet = "Generic";

    // Act
    m_deviceGpu->loadOtherDeviceInfo();

    // Assert
    EXPECT_EQ(1, m_deviceGpu->m_LstOtherInfo.size());
    EXPECT_TRUE(m_deviceGpu->m_LstOtherInfo.contains(QPair<QString, QString>("Extra Key", "extra value")));
}

/* ===================== loadTableHeader / loadTableData ===================== */

TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadTableHeader)
{
    // Arrange：空设备即可
    ASSERT_NE(m_deviceGpu, nullptr);

    // Act
    m_deviceGpu->loadTableHeader();

    // Assert：三列（名称/厂商/型号）且无空表头
    EXPECT_EQ(3, m_deviceGpu->m_TableHeader.size());
    EXPECT_FALSE(m_deviceGpu->m_TableHeader.isEmpty());
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadTableData)
{
    // Arrange：设备可用（真实 available() 返回 true）
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_hwinfo(mapinfo);
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Act
    m_deviceGpu->loadTableData();

    // Assert：名称/厂商/型号三列，可用时名称不加前缀
    ASSERT_EQ(3, m_deviceGpu->m_TableData.size());
    EXPECT_STREQ("GK208B [GeForce GT 730]", m_deviceGpu->m_TableData.at(0).toStdString().c_str());
    EXPECT_STREQ("nVidia Corporation", m_deviceGpu->m_TableData.at(1).toStdString().c_str());
    EXPECT_STREQ("nVidia GK208B [GeForce GT 730]", m_deviceGpu->m_TableData.at(2).toStdString().c_str());
}

// 分支：不可用设备 → 名称列加 "(Unavailable)" 前缀（翻译存在与否不影响 contains 判定）
TEST_F(UT_DeviceGpu, UT_DeviceGpu_loadTableData_Unavailable_MarksName)
{
    // Arrange：子类多态重写 available() = false；exec 桩阻断 modinfo 子进程
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_hwinfo(mapinfo);
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();
    UT_GpuUnavailable unavailableGpu;
    unavailableGpu.setHwinfoInfo(mapinfo);

    // Act
    unavailableGpu.loadTableData();

    // Assert：名称列含原名称且与原名称不同（前缀已加）
    ASSERT_EQ(3, unavailableGpu.m_TableData.size());
    EXPECT_TRUE(unavailableGpu.m_TableData.at(0).contains("GK208B [GeForce GT 730]"));
    EXPECT_NE(unavailableGpu.m_TableData.at(0), QString("GK208B [GeForce GT 730]"));
}

/* ===================== setXrandrInfo ===================== */

// 分支 B1~B6：全部接口键存在
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setXrandrInfo)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("minResolution", "minResolution");
    mapInfo.insert("curResolution", "curResolution");
    mapInfo.insert("maxResolution", "maxResolution");
    mapInfo.insert("HDMI", "Unable");
    mapInfo.insert("VGA", "Unable");
    mapInfo.insert("DP", "Unable");
    mapInfo.insert("eDP", "Unable");
    mapInfo.insert("DVI", "Unable");
    mapInfo.insert("DigitalOutput", "DigitalOut Value");

    // Act
    m_deviceGpu->setXrandrInfo(mapInfo);

    // Assert
    EXPECT_STREQ("minResolution", m_deviceGpu->m_MinimumResolution.toStdString().c_str());
    EXPECT_STREQ("curResolution", m_deviceGpu->m_CurrentResolution.toStdString().c_str());
    EXPECT_STREQ("maxResolution", m_deviceGpu->m_MaximumResolution.toStdString().c_str());
    EXPECT_STREQ("Unable", m_deviceGpu->m_HDMI.toStdString().c_str());
    EXPECT_STREQ("Unable", m_deviceGpu->m_VGA.toStdString().c_str());
    EXPECT_STREQ("Unable", m_deviceGpu->m_DisplayPort.toStdString().c_str());
    EXPECT_STREQ("Unable", m_deviceGpu->m_eDP.toStdString().c_str());
    EXPECT_STREQ("Unable", m_deviceGpu->m_DVI.toStdString().c_str());
    EXPECT_STREQ("DigitalOut Value", m_deviceGpu->m_Digital.toStdString().c_str());
}

// 分支边界：键缺失 → 对应成员保持为空（默认赋值语义）
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setXrandrInfo_MissingKeys_KeepsEmpty)
{
    // Arrange：仅含分辨率，无任何接口键
    QMap<QString, QString> mapInfo;
    mapInfo.insert("minResolution", "640x480");

    // Act
    m_deviceGpu->setXrandrInfo(mapInfo);

    // Assert：接口成员保持空
    EXPECT_STREQ("640x480", m_deviceGpu->m_MinimumResolution.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_CurrentResolution.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_HDMI.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_DisplayPort.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Digital.toStdString().c_str());
}

/* ===================== name / driver / subTitle / getOverviewInfo ===================== */

TEST_F(UT_DeviceGpu, UT_DeviceGpu_name)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_hwinfo(mapinfo);
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Act
    QString name = m_deviceGpu->name();

    // Assert：与成员值一致
    EXPECT_STREQ("GK208B [GeForce GT 730]", name.toStdString().c_str());
    EXPECT_EQ(m_deviceGpu->m_Name, name);
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_driver)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_hwinfo(mapinfo);
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Act
    QString driver = m_deviceGpu->driver();

    // Assert：与成员值一致
    EXPECT_STREQ("nouveau", driver.toStdString().c_str());
    EXPECT_EQ(m_deviceGpu->m_Driver, driver);
}

// 等价类：vendor getter 与成员值一致
TEST_F(UT_DeviceGpu, UT_DeviceGpu_vendor)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_hwinfo(mapinfo);
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Act
    QString vendor = m_deviceGpu->vendor();

    // Assert：与成员值一致
    EXPECT_STREQ("nVidia Corporation", vendor.toStdString().c_str());
    EXPECT_EQ(m_deviceGpu->m_Vendor, vendor);
}

TEST_F(UT_DeviceGpu, UT_DeviceGpu_subTitle)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_hwinfo(mapinfo);
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Act
    QString title = m_deviceGpu->subTitle();

    // Assert：副标题即 m_Model
    EXPECT_STREQ("nVidia GK208B [GeForce GT 730]", title.toStdString().c_str());
    EXPECT_EQ(m_deviceGpu->m_Model, title);
}

// 等价类：名称非空 → 概览返回名称
TEST_F(UT_DeviceGpu, UT_DeviceGpu_getOverviewInfo)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_gpu_fill_hwinfo(mapinfo);
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_gpu_execCmd_stub);
    ut_gpu_execCmdRet = QByteArray();
    m_deviceGpu->setHwinfoInfo(mapinfo);

    // Act
    QString overview = m_deviceGpu->getOverviewInfo();

    // Assert：名称非空 → 概览即名称
    EXPECT_STREQ("GK208B [GeForce GT 730]", overview.toStdString().c_str());
    EXPECT_EQ(m_deviceGpu->name(), overview);
}

// 边界：名称为空 → 概览回退为型号
TEST_F(UT_DeviceGpu, UT_DeviceGpu_getOverviewInfo_EmptyName_ReturnsModel)
{
    // Arrange：名称空、型号非空
    m_deviceGpu->m_Model = "Model X";

    // Act
    QString overview = m_deviceGpu->getOverviewInfo();

    // Assert：名称为空 → 概览回退为型号
    EXPECT_STREQ("Model X", overview.toStdString().c_str());
    EXPECT_EQ(m_deviceGpu->m_Model, overview);
}

/* ===================== setInfoFromTomlOneByOne ===================== */

// 等价类：TOML 配置值写入成员；返回值恒为 TOML_None
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setInfoFromTomlOneByOne_ValuesApplied)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", "Toml Model");
    mapInfo.insert("Driver", "toml_drv");
    mapInfo.insert("Graphics Memory", "4GB");
    mapInfo.insert("HDMI", "Connected");

    // Act
    TomlFixMethod ret = m_deviceGpu->setInfoFromTomlOneByOne(mapInfo);

    // Assert：成员更新
    EXPECT_STREQ("Toml Model", m_deviceGpu->m_Model.toStdString().c_str());
    EXPECT_STREQ("toml_drv", m_deviceGpu->m_Driver.toStdString().c_str());
    EXPECT_STREQ("4GB", m_deviceGpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_STREQ("Connected", m_deviceGpu->m_HDMI.toStdString().c_str());
    EXPECT_EQ(TOML_None, ret);
}

// 边界：空 map → 成员保持为空、不崩溃
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setInfoFromTomlOneByOne_EmptyMap_NoChange)
{
    // Arrange：空 map
    QMap<QString, QString> emptyMap;

    // Act
    TomlFixMethod ret = m_deviceGpu->setInfoFromTomlOneByOne(emptyMap);

    // Assert
    EXPECT_STREQ("", m_deviceGpu->m_Model.toStdString().c_str());
    EXPECT_STREQ("", m_deviceGpu->m_Driver.toStdString().c_str());
    EXPECT_EQ(TOML_None, ret);
}

// 分支：值含 "nouse" → 成员被清空（TOML_nouse 语义）
TEST_F(UT_DeviceGpu, UT_DeviceGpu_setInfoFromTomlOneByOne_NouseClearsValue)
{
    // Arrange：预置型号，再传 nouse 值
    m_deviceGpu->m_Model = "Old Model";
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", "nouse_model");

    // Act
    TomlFixMethod ret = m_deviceGpu->setInfoFromTomlOneByOne(mapInfo);

    // Assert：值被清空，返回 TOML_None
    EXPECT_STREQ("", m_deviceGpu->m_Model.toStdString().c_str());
    EXPECT_EQ(TOML_None, ret);
}
