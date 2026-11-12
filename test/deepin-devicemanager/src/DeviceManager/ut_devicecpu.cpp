// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * UT_DeviceCpu — DeviceCpu 单元测试（qt-autotest-generator Mode 2 重写增强）
 *
 * 打桩决策（依据 skill 依赖矩阵）：
 *  - DeviceCpu 全部方法为纯内存逻辑（map 读写、字符串处理），不涉及 DBus、
 *    网络、文件系统、子进程（lscpu/lshw/dmidecode 的输出已由测试以 map 形式
 *    直接注入 setInfoFromXxx），无需打桩，全部使用真实实现。
 *  - Common::isHwPlatform / Common::specialComType 为环境常量，测试环境下
 *    取默认值（非特殊机型、非 HW 平台），分支确定可复现。
 *
 * 用例计数声明（min 按技能 §4 level/factors 下限；actual 为本文件实际用例数）：
 *
 * | 方法 | level | factors | min | actual |
 * |----------------------------|------|---------------------|-----|--------|
 * | DeviceCpu | mid | in_degree:90 | 2 | 2 |
 * | available | low | - | 1 | 1 |
 * | driver | low | - | 1 | 1 |
 * | frequencyIsRange | mid | in_degree:1 | 2 | 2 |
 * | getOverviewInfo | low | - | 1 | 1 |
 * | initFilterKey | mid | in_degree:1 | 2 | 2 |
 * | loadBaseDeviceInfo | low | - | 1 | 1 |
 * | loadOtherDeviceInfo | low | - | 1 | 1 |
 * | loadTableData | low | - | 1 | 1 |
 * | loadTableHeader | low | - | 1 | 2 |
 * | name | low | - | 1 | 1 |
 * | setCpuInfo | low | - | 1 | 1 |
 * | setCurFreq | mid | in_degree:2 | 3 | 3 |
 * | setFrequencyIsCur | mid | in_degree:1 | 2 | 2 |
 * | setInfoFromDmidecode | mid | in_degree:1 | 2 | 2 |
 * | setInfoFromLscpu | mid | in_degree:1;lines:66 | 3 | 3 |
 * | setInfoFromLshw | mid | in_degree:1 | 2 | 2 |
 * | setInfoFromTomlOneByOne | mid | in_degree:2 | 3 | 3 |
 * | subTitle | low | - | 1 | 1 |
 * | vendor | mid | in_degree:1 | 2 | 2 |
 */

// 分支清单（来源：DeviceCpu::setInfoFromLscpu）
// B1: min && max 同时存在 → 进入频率范围计算
// B2: min 与 max 相等（fabs 差 < 0.001）→ m_FrequencyIsRange=false，单值显示
// B3: min 与 max 不等 → m_Frequency 为 "min-max GHz" 范围
// B4: 仅有 CPU MHz → 单值频率
// B5: CPU MHz 值含 "MHz" → 原样使用，否则追加 " MHz"
// B6: Flags 含扩展指令集 → 逐项追加 m_Extensions
// B7: getOtherMapInfo 汇集其他属性

// 分支清单（来源：DeviceCpu::setInfoFromDmidecode）
// B1: m_Name 含 Loongson（忽略大小写）→ product 以不覆盖模式写入（名称保持）
// B2: 非 Loongson → product 覆盖写入 m_Name
// B3: isHwPlatform 且 map 含 Version → m_Name 取 Version
// B4: Manufacturer/Max Speed/Current Speed/Family 以不覆盖模式赋值
// B5: getOtherMapInfo 汇集其他属性

#include "DeviceCpu.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

#define private public
#define protected public
#include "DeviceCpu.h"

class UT_DeviceCpu : public ::testing::Test
{
public:
    void SetUp()
    {
        m_deviceCpu = new DeviceCpu;
    }
    void TearDown()
    {
        delete m_deviceCpu;
    }
    DeviceCpu *m_deviceCpu;
};

void setLscpuData(QMap<QString, QString> &mapLscpu)
{
    mapLscpu.insert("model name", "Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz");
    mapLscpu.insert("vendor_id", "GenuineIntel");
    mapLscpu.insert("Thread(s) per core", "1");
    mapLscpu.insert("bogomips", "7200.00");
    mapLscpu.insert("Architecture", "x86_64");
    mapLscpu.insert("cpu family", "6");
    mapLscpu.insert("CPU MHz", "4085.639");
    mapLscpu.insert("model", "158");
    mapLscpu.insert("stepping", "10");
    mapLscpu.insert("L1d cache", "128 KiB");
    mapLscpu.insert("L1i cache", "128 KiB");
    mapLscpu.insert("L2 cache", "1 MiB");
    mapLscpu.insert("L3 cache", "6 MiB");
    mapLscpu.insert("Flags", "MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T");
    mapLscpu.insert("flags", "MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T");
    mapLscpu.insert("Virtualization", "VT-x");
    mapLscpu.insert("processor", "0");
    mapLscpu.insert("core id", "0");
}

void setLshwData(QMap<QString, QString> &mapLshw)
{
    mapLshw.insert("product", "Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz");
    mapLshw.insert("version", "Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz");
    mapLshw.insert("vendor", "Intel Corp.");
}

void setDmidecodeData(QMap<QString, QString> &mapDmidecode)
{
    mapDmidecode.insert("product", "Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz");
    mapDmidecode.insert("Manufacturer", "Intel(R) Corporation");
    mapDmidecode.insert("Max Speed", "8300 MHz");
    mapDmidecode.insert("Family", "Core i3");
}

// 构造等价类：新建设备为空状态
TEST_F(UT_DeviceCpu, UT_DeviceCpu_ctor_Defaults)
{
    // Arrange：新建 DeviceCpu
    ASSERT_NE(nullptr, m_deviceCpu);

    // Act
    QString name = m_deviceCpu->name();

    // Assert：名称为空、默认可用
    EXPECT_TRUE(name.isEmpty());
    EXPECT_EQ(m_deviceCpu->m_Name, name);
    EXPECT_TRUE(m_deviceCpu->available());
}

// 构造等价类：默认 subTitle 无物理 ID 后缀值
TEST_F(UT_DeviceCpu, UT_DeviceCpu_ctor_SubTitleEmptyPhysicalID)
{
    // Arrange：新建 DeviceCpu
    ASSERT_NE(nullptr, m_deviceCpu);

    // Act
    QString title = m_deviceCpu->subTitle();

    // Assert：m_PhysicalID 为空，副标题仅含前缀
    EXPECT_TRUE(m_deviceCpu->m_PhysicalID.isEmpty());
    EXPECT_STREQ("Processor ", title.toStdString().c_str());
}

// 分支清单（来源：DeviceCpu::setCpuInfo）
// B1: 依次调用 setInfoFromLscpu/setInfoFromLshw/setInfoFromDmidecode
// B2: m_Name 去除尾部 "/[0-9]*$" 与 "x [0-9]*$"（正则）
// B3: 特殊机型（kSpecialType5/6/7）→ 频率字符串替换（测试环境非特殊机型，不触发）
// B4: 记录逻辑核数与 core 数
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setCpuInfo)
{
    // Arrange：三路数据齐备
    QMap<QString, QString> mapLscpu;
    QMap<QString, QString> mapLshw;
    QMap<QString, QString> mapDmidecode;

    setLscpuData(mapLscpu);
    setLshwData(mapLshw);
    setDmidecodeData(mapDmidecode);

    // Act
    m_deviceCpu->setCpuInfo(mapLscpu, mapLshw, mapDmidecode, 4, 4);

    // Assert：名称、核数与厂商（dmidecode 最后覆盖）
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->name().toStdString().c_str());
    EXPECT_EQ(4, m_deviceCpu->m_LogicalCPUNum);
    EXPECT_EQ(4, m_deviceCpu->m_CPUCoreNum);
    EXPECT_STREQ("Intel(R) Corporation", m_deviceCpu->m_Vendor.toStdString().c_str());
}

// 等价类：过滤键初始化（5 个 CPU 键）
TEST_F(UT_DeviceCpu, UT_DeviceCpu_initFilterKey)
{
    // Arrange：调用前调用后键数不变（幂等）
    m_deviceCpu->initFilterKey();
    const int firstSize = m_deviceCpu->m_FilterKey.size();

    // Act
    m_deviceCpu->initFilterKey();

    // Assert
    EXPECT_EQ(5, m_deviceCpu->m_FilterKey.size());
    EXPECT_EQ(firstSize, m_deviceCpu->m_FilterKey.size());
}

// 等价类：过滤键内容
TEST_F(UT_DeviceCpu, UT_DeviceCpu_initFilterKey_KeyContents)
{
    // Arrange：新建设备
    ASSERT_NE(nullptr, m_deviceCpu);

    // Act
    m_deviceCpu->initFilterKey();

    // Assert：五个键全部存在，总数为 5
    EXPECT_EQ(5, m_deviceCpu->m_FilterKey.size());
    EXPECT_TRUE(m_deviceCpu->m_FilterKey.contains("CPU implementer"));
    EXPECT_TRUE(m_deviceCpu->m_FilterKey.contains("CPU architecture"));
    EXPECT_TRUE(m_deviceCpu->m_FilterKey.contains("CPU variant"));
    EXPECT_TRUE(m_deviceCpu->m_FilterKey.contains("CPU part"));
    EXPECT_TRUE(m_deviceCpu->m_FilterKey.contains("CPU revision"));
}

// 等价类：概览信息格式（核数/逻辑数英文翻译）
TEST_F(UT_DeviceCpu, UT_DeviceCpu_getOverviewInfo)
{
    // Arrange：三路数据齐备
    QMap<QString, QString> mapLscpu;
    QMap<QString, QString> mapLshw;
    QMap<QString, QString> mapDmidecode;

    setLscpuData(mapLscpu);
    setLshwData(mapLshw);
    setDmidecodeData(mapDmidecode);
    m_deviceCpu->setCpuInfo(mapLscpu, mapLshw, mapDmidecode, 4, 4);

    // Act
    QString overview = m_deviceCpu->getOverviewInfo();

    // Assert
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz (FourCore(s) / FourProcessor)", overview.toStdString().c_str());
    EXPECT_TRUE(overview.contains("Core(s)"));
}

// 等价类：基本信息列表（Max Speed 未注入时跳过，共 6 项）
TEST_F(UT_DeviceCpu, UT_DeviceCpu_loadBaseDeviceInfo)
{
    // Arrange：注入 lscpu 数据
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    m_deviceCpu->setInfoFromLscpu(mapLscpu);

    // Act
    m_deviceCpu->loadBaseDeviceInfo();

    // Assert：顺序 Name, Vendor, Threads, Architecture, CPU Family, Model
    ASSERT_EQ(6, m_deviceCpu->m_LstBaseInfo.size());
    QPair<QString, QString> value0 = m_deviceCpu->m_LstBaseInfo.at(0);
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", value0.second.toStdString().c_str());
    QPair<QString, QString> value1 = m_deviceCpu->m_LstBaseInfo.at(1);
    EXPECT_STREQ("GenuineIntel", value1.second.toStdString().c_str());
    QPair<QString, QString> value2 = m_deviceCpu->m_LstBaseInfo.at(2);
    EXPECT_STREQ("1", value2.second.toStdString().c_str());
    QPair<QString, QString> value3 = m_deviceCpu->m_LstBaseInfo.at(3);
    EXPECT_STREQ("x86_64", value3.second.toStdString().c_str());
    QPair<QString, QString> value4 = m_deviceCpu->m_LstBaseInfo.at(4);
    EXPECT_STREQ("6", value4.second.toStdString().c_str());
    QPair<QString, QString> value5 = m_deviceCpu->m_LstBaseInfo.at(5);
    EXPECT_STREQ("158", value5.second.toStdString().c_str());
}

// 等价类：CPU 无其他信息来源 → 其他信息列表为空
TEST_F(UT_DeviceCpu, UT_DeviceCpu_loadOtherDeviceInfo)
{
    // Arrange：新建设备
    ASSERT_NE(nullptr, m_deviceCpu);

    // Act
    m_deviceCpu->loadOtherDeviceInfo();

    // Assert
    EXPECT_EQ(0, m_deviceCpu->m_LstOtherInfo.size());
    EXPECT_TRUE(m_deviceCpu->m_LstOtherInfo.isEmpty());
}

// 分支清单（来源：DeviceCpu::loadTableHeader）
// B1: frequencyIsRange()==true → 第 3 列为 "Frequency"
// B2: frequencyIsRange()==false → 第 3 列为 "Max Frequency"
TEST_F(UT_DeviceCpu, UT_DeviceCpu_loadTableHeader)
{
    // Arrange：默认非范围频率
    ASSERT_FALSE(m_deviceCpu->m_FrequencyIsRange);

    // Act
    m_deviceCpu->loadTableHeader();

    // Assert：4 列，第 3 列为 Max Frequency
    EXPECT_EQ(4, m_deviceCpu->m_TableHeader.size());
    EXPECT_STREQ("Max Frequency", m_deviceCpu->m_TableHeader.at(2).toStdString().c_str());
    EXPECT_STREQ("Architecture", m_deviceCpu->m_TableHeader.at(3).toStdString().c_str());
}

// 分支 B1：范围频率 → 表头第 3 列为 Frequency
TEST_F(UT_DeviceCpu, UT_DeviceCpu_loadTableHeader_RangeFlag)
{
    // Arrange：标记为范围频率
    m_deviceCpu->m_FrequencyIsRange = true;

    // Act
    m_deviceCpu->loadTableHeader();

    // Assert：第 3 列为 Frequency，列名序列完整
    ASSERT_EQ(4, m_deviceCpu->m_TableHeader.size());
    EXPECT_STREQ("Frequency", m_deviceCpu->m_TableHeader.at(2).toStdString().c_str());
    EXPECT_STREQ("Name", m_deviceCpu->m_TableHeader.at(0).toStdString().c_str());
    EXPECT_STREQ("Architecture", m_deviceCpu->m_TableHeader.at(3).toStdString().c_str());
}

// 等价类：表格数据 4 列（空设备为空串）
TEST_F(UT_DeviceCpu, UT_DeviceCpu_loadTableData)
{
    // Arrange：新建设备
    ASSERT_NE(nullptr, m_deviceCpu);

    // Act
    m_deviceCpu->loadTableData();

    // Assert
    EXPECT_EQ(4, m_deviceCpu->m_TableData.size());
    EXPECT_TRUE(m_deviceCpu->m_TableData.at(0).isEmpty());
}

// 等价类：name() 与 m_Name 一致
TEST_F(UT_DeviceCpu, UT_DeviceCpu_name)
{
    // Arrange：注入 lscpu 数据
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    m_deviceCpu->setInfoFromLscpu(mapLscpu);

    // Act
    QString name = m_deviceCpu->name();

    // Assert
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", name.toStdString().c_str());
    EXPECT_EQ(m_deviceCpu->m_Name, name);
}

// 等价类：vendor() 与 m_Vendor 一致
TEST_F(UT_DeviceCpu, UT_DeviceCpu_vendor)
{
    // Arrange：注入 lscpu 数据
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    m_deviceCpu->setInfoFromLscpu(mapLscpu);

    // Act
    const QString &vendor = m_deviceCpu->vendor();

    // Assert
    EXPECT_STREQ("GenuineIntel", vendor.toStdString().c_str());
    EXPECT_EQ(m_deviceCpu->m_Vendor, vendor);
}

// 等价类：dmidecode 路径后厂商取 Manufacturer（覆盖式）
TEST_F(UT_DeviceCpu, UT_DeviceCpu_vendor_AfterDmidecode)
{
    // Arrange：三路数据齐备
    QMap<QString, QString> mapLscpu;
    QMap<QString, QString> mapLshw;
    QMap<QString, QString> mapDmidecode;
    setLscpuData(mapLscpu);
    setLshwData(mapLshw);
    setDmidecodeData(mapDmidecode);

    // Act
    m_deviceCpu->setCpuInfo(mapLscpu, mapLshw, mapDmidecode, 4, 4);

    // Assert：dmidecode 最后执行，厂商为 Manufacturer，与成员一致
    EXPECT_STREQ("Intel(R) Corporation", m_deviceCpu->vendor().toStdString().c_str());
    EXPECT_EQ(m_deviceCpu->m_Vendor, m_deviceCpu->vendor());
}

// 等价类：预置驱动后返回预置值
TEST_F(UT_DeviceCpu, UT_DeviceCpu_driver)
{
    // Arrange：预置驱动
    m_deviceCpu->m_Driver = "processor";

    // Act
    QString driver = m_deviceCpu->driver();

    // Assert：返回预置值，与成员一致
    EXPECT_STREQ("processor", driver.toStdString().c_str());
    EXPECT_EQ(m_deviceCpu->m_Driver, driver);
}

// 等价类：subTitle 为 Processor + 物理 ID
TEST_F(UT_DeviceCpu, UT_DeviceCpu_subTitle)
{
    // Arrange：注入 lscpu 数据（processor=0）
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    m_deviceCpu->setInfoFromLscpu(mapLscpu);

    // Act
    QString title = m_deviceCpu->subTitle();

    // Assert
    EXPECT_STREQ("Processor 0", title.toStdString().c_str());
    m_deviceCpu->m_PhysicalID = "2";
    EXPECT_STREQ("Processor 2", m_deviceCpu->subTitle().toStdString().c_str());
}

// 等价类：默认可用（m_Available 默认值，驱动为空时 available() 强制不可用已由子类覆盖）
TEST_F(UT_DeviceCpu, UT_DeviceCpu_available)
{
    // Arrange：预置可用状态与非空驱动
    m_deviceCpu->m_Available = true;
    m_deviceCpu->m_Driver = "processor";

    // Act
    bool ret = m_deviceCpu->available();

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(m_deviceCpu->m_Available, ret);
}

// 分支等价类：min/max 缺失 → 非范围频率
TEST_F(UT_DeviceCpu, UT_DeviceCpu_frequencyIsRange_SingleValue)
{
    // Arrange：注入 lscpu 数据（无 min/max MHz）
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    m_deviceCpu->setInfoFromLscpu(mapLscpu);

    // Act
    bool ret = m_deviceCpu->frequencyIsRange();

    // Assert：非范围，与成员一致
    EXPECT_FALSE(ret);
    EXPECT_EQ(m_deviceCpu->m_FrequencyIsRange, ret);
}

// 分支等价类：min/max 同时存在且不等 → 范围频率
TEST_F(UT_DeviceCpu, UT_DeviceCpu_frequencyIsRange_RangeValue)
{
    // Arrange：注入 lscpu 数据 + min/max MHz
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    mapLscpu.insert("CPU min MHz", "800.0000");
    mapLscpu.insert("CPU max MHz", "4200.0000");
    m_deviceCpu->setInfoFromLscpu(mapLscpu);

    // Act
    bool ret = m_deviceCpu->frequencyIsRange();

    // Assert：范围态，与成员一致
    EXPECT_TRUE(ret);
    EXPECT_EQ(m_deviceCpu->m_FrequencyIsRange, ret);
}

// 分支清单（来源：DeviceCpu::setInfoFromLshw）
// B1: product 非 null/ARMv → m_Name 取 product
// B2: product 含 null/ARMv → m_Name 取 version 兑底（bug-108166）
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromLshw_LoongsonKeep)
{
    // Arrange：预置名称（lshw product 不含 null → 应被覆盖）
    QMap<QString, QString> mapLshw;
    setLshwData(mapLshw);

    // Act
    m_deviceCpu->setInfoFromLshw(mapLshw);

    // Assert：product 覆盖 m_Name，vendor 写入
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corp.", m_deviceCpu->m_Vendor.toStdString().c_str());
}

// 分支 B2：product 含 "null" → m_Name 取 version 兑底（bug-108166）
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromLshw_ProductNull)
{
    // Arrange：product 为 null，version 为正确 CPU 信息
    QMap<QString, QString> mapLshw;
    setLshwData(mapLshw);
    mapLshw.insert("product", "null");

    // Act
    m_deviceCpu->setInfoFromLshw(mapLshw);

    // Assert：名称取 version，vendor 正常写入
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corp.", m_deviceCpu->m_Vendor.toStdString().c_str());
}

// 分支清单（来源：DeviceCpu::setInfoFromLscpu）——见文件头
// 分支清单（来源：DeviceCpu::setInfoFromDmidecode）——见文件头

// 分支等价类：标准 lscpu 数据，全属性赋值
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromLscpu_Normal)
{
    // Arrange：标准 lscpu 数据
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);

    // Act
    m_deviceCpu->setInfoFromLscpu(mapLscpu);

    // Assert：逐项核对
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("GenuineIntel", m_deviceCpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("1", m_deviceCpu->m_ThreadNum.toStdString().c_str());
    EXPECT_STREQ("7200.00", m_deviceCpu->m_BogoMIPS.toStdString().c_str());
    EXPECT_STREQ("x86_64", m_deviceCpu->m_Architecture.toStdString().c_str());
    EXPECT_STREQ("6", m_deviceCpu->m_Familly.toStdString().c_str());
    EXPECT_STREQ("4085.639", m_deviceCpu->m_CurFrequency.toStdString().c_str());
    EXPECT_STREQ("158", m_deviceCpu->m_Model.toStdString().c_str());
    EXPECT_STREQ("10", m_deviceCpu->m_Step.toStdString().c_str());
    EXPECT_STREQ("MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T", m_deviceCpu->m_Flags.toStdString().c_str());
    EXPECT_STREQ("VT-x", m_deviceCpu->m_HardwareVirtual.toStdString().c_str());
    EXPECT_STREQ("0", m_deviceCpu->m_PhysicalID.toStdString().c_str());
    EXPECT_STREQ("0", m_deviceCpu->m_CoreID.toStdString().c_str());
    EXPECT_STREQ("4085.639 MHz", m_deviceCpu->m_Frequency.toStdString().c_str());
    EXPECT_STREQ("MMX SSE SSE2 SSE3 3D Now SSE4 SSSE3 SSE4_1 SSE4_2 AMD64 EM64T ", m_deviceCpu->m_Extensions.toStdString().c_str());
    EXPECT_FALSE(m_deviceCpu->m_FrequencyIsRange);
}

// 分支 B3：min/max 存在且不等 → 频率为范围字符串
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromLscpu_RangeFreq)
{
    // Arrange：lscpu 数据 + min/max MHz
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    mapLscpu.insert("CPU min MHz", "800.0000");
    mapLscpu.insert("CPU max MHz", "4200.0000");

    // Act
    m_deviceCpu->setInfoFromLscpu(mapLscpu);

    // Assert：范围格式 0.80-4.20 GHz
    EXPECT_STREQ("0.80-4.20 GHz", m_deviceCpu->m_Frequency.toStdString().c_str());
    EXPECT_STREQ("4200.0000", m_deviceCpu->m_MaxFrequency.toStdString().c_str());
    EXPECT_TRUE(m_deviceCpu->m_FrequencyIsRange);
}

// 分支 B2：min/max 相等 → 单值显示，非范围
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromLscpu_EqualMinMax)
{
    // Arrange：lscpu 数据 + 相等 min/max MHz
    QMap<QString, QString> mapLscpu;
    setLscpuData(mapLscpu);
    mapLscpu.insert("CPU min MHz", "800.0000");
    mapLscpu.insert("CPU max MHz", "800.0000");

    // Act
    m_deviceCpu->setInfoFromLscpu(mapLscpu);

    // Assert：800 MHz 单值，非范围
    EXPECT_STREQ("800.00 MHz", m_deviceCpu->m_Frequency.toStdString().c_str());
    EXPECT_FALSE(m_deviceCpu->m_FrequencyIsRange);
}

// 分支 B1：非 Loongson → product 覆盖 m_Name，Max Speed 写入频率
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromDmidecode_Normal)
{
    // Arrange：标准 dmidecode 数据
    QMap<QString, QString> mapDmidecode;
    setDmidecodeData(mapDmidecode);

    // Act
    m_deviceCpu->setInfoFromDmidecode(mapDmidecode);

    // Assert：product 覆盖名称，Manufacturer/Max Speed/Family 写入
    EXPECT_STREQ("Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel(R) Corporation", m_deviceCpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("8300 MHz", m_deviceCpu->m_Frequency.toStdString().c_str());
    EXPECT_STREQ("Core i3", m_deviceCpu->m_Familly.toStdString().c_str());
}

// 分支 B1：名称含 Loongson（忽略大小写）→ product 不覆盖
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromDmidecode_LoongsonKeep)
{
    // Arrange：预置 Loongson 名称
    QMap<QString, QString> mapDmidecode;
    setDmidecodeData(mapDmidecode);
    m_deviceCpu->m_Name = "Loongson";

    // Act
    m_deviceCpu->setInfoFromDmidecode(mapDmidecode);

    // Assert：名称保持，其余属性正常写入
    EXPECT_STREQ("Loongson", m_deviceCpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel(R) Corporation", m_deviceCpu->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("8300 MHz", m_deviceCpu->m_Frequency.toStdString().c_str());
    EXPECT_STREQ("Core i3", m_deviceCpu->m_Familly.toStdString().c_str());
}

// 分支等价类：Stepping 键存在 → 返回 TOML_Cover 且成员赋值
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromTomlOneByOne_SteppingKey)
{
    // Arrange：map 含 Stepping 与 Frequency
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Stepping", "10");
    mapInfo.insert("Frequency", "2.4 GHz");

    // Act
    TomlFixMethod ret = m_deviceCpu->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_STREQ("10", m_deviceCpu->m_Step.toStdString().c_str());
    EXPECT_STREQ("2.4 GHz", m_deviceCpu->m_Frequency.toStdString().c_str());
}

// 分支等价类：空 map → 返回 TOML_None，成员不变
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromTomlOneByOne_EmptyMap)
{
    // Arrange：空 map
    QMap<QString, QString> mapInfo;

    // Act
    TomlFixMethod ret = m_deviceCpu->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_None, ret);
    EXPECT_TRUE(m_deviceCpu->m_PhysicalID.isEmpty());
    EXPECT_TRUE(m_deviceCpu->m_Step.isEmpty());
}

// 分支等价类：多键注入 → 各成员逐一赋值
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setInfoFromTomlOneByOne_MultiKeys)
{
    // Arrange：多键 map
    QMap<QString, QString> mapInfo;
    mapInfo.insert("CPU ID", "0");
    mapInfo.insert("Threads", "8");
    mapInfo.insert("Architecture", "x86_64");
    mapInfo.insert("BogoMIPS", "7200.00");
    mapInfo.insert("Virtualization", "VT-x");

    // Act
    TomlFixMethod ret = m_deviceCpu->setInfoFromTomlOneByOne(mapInfo);

    // Assert：无 Stepping 键 → TOML_None；各成员赋值
    EXPECT_EQ(TOML_None, ret);
    EXPECT_STREQ("0", m_deviceCpu->m_PhysicalID.toStdString().c_str());
    EXPECT_STREQ("8", m_deviceCpu->m_ThreadNum.toStdString().c_str());
    EXPECT_STREQ("x86_64", m_deviceCpu->m_Architecture.toStdString().c_str());
    EXPECT_STREQ("7200.00", m_deviceCpu->m_BogoMIPS.toStdString().c_str());
    EXPECT_STREQ("VT-x", m_deviceCpu->m_HardwareVirtual.toStdString().c_str());
}

// 分支等价类：非空频率 → 覆盖当前频率
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setCurFreq_Overwrite)
{
    // Arrange：预置当前频率
    m_deviceCpu->m_CurFrequency = "100 MHz";

    // Act
    m_deviceCpu->setCurFreq("1800 MHz");

    // Assert：覆盖为新值
    EXPECT_STREQ("1800 MHz", m_deviceCpu->m_CurFrequency.toStdString().c_str());
    EXPECT_NE(QString("100 MHz"), m_deviceCpu->m_CurFrequency);
}

// 分支等价类：空频率 → 忽略，保持原值
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setCurFreq_Empty_Ignored)
{
    // Arrange：预置当前频率
    m_deviceCpu->m_CurFrequency = "100 MHz";

    // Act
    m_deviceCpu->setCurFreq("");

    // Assert：不被清空，仍为旧值
    EXPECT_STREQ("100 MHz", m_deviceCpu->m_CurFrequency.toStdString().c_str());
    EXPECT_NE(QString(""), m_deviceCpu->m_CurFrequency);
}

// 分支等价类：多次设置 → 以最后一次为准
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setCurFreq_LastWriteWins)
{
    // Arrange：新建设备
    ASSERT_NE(nullptr, m_deviceCpu);

    // Act
    m_deviceCpu->setCurFreq("1.0 GHz");
    m_deviceCpu->setCurFreq("2.0 GHz");

    // Assert：以最后一次为准
    EXPECT_STREQ("2.0 GHz", m_deviceCpu->m_CurFrequency.toStdString().c_str());
    EXPECT_NE(QString("1.0 GHz"), m_deviceCpu->m_CurFrequency);
}

// 等价类：标记当前频率为范围
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setFrequencyIsCur_True)
{
    // Arrange：构造默认为 true，先预置为 false
    m_deviceCpu->m_FrequencyIsCur = false;

    // Act
    m_deviceCpu->setFrequencyIsCur(true);

    // Assert：置为 true，与入参一致
    EXPECT_TRUE(m_deviceCpu->m_FrequencyIsCur);
    EXPECT_EQ(true, m_deviceCpu->m_FrequencyIsCur);
}

// 等价类：取消当前频率标记
TEST_F(UT_DeviceCpu, UT_DeviceCpu_setFrequencyIsCur_False)
{
    // Arrange：先置为 true
    m_deviceCpu->setFrequencyIsCur(true);

    // Act
    m_deviceCpu->setFrequencyIsCur(false);

    // Assert：置为 false，与入参一致
    EXPECT_FALSE(m_deviceCpu->m_FrequencyIsCur);
    EXPECT_EQ(false, m_deviceCpu->m_FrequencyIsCur);
}

// 等价类：翻译数字表规模
TEST_F(UT_DeviceCpu, UT_DeviceCpu_getTrNumber)
{
    // Arrange：静态常量表
    ASSERT_NE(nullptr, m_deviceCpu);

    // Act
    const int size = m_deviceCpu->m_trNumber.size();

    // Assert：表规模与预期一致且非空
    EXPECT_EQ(130, size);
    EXPECT_GT(size, 0);
}
