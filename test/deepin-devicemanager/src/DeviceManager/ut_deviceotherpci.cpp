// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Mode 2 重写：DeviceOtherPCI 单元测试（旧版断言空洞/无分支映射，评级 D，整体重写）
// 覆盖方法：ctor/setInfoFromTomlOneByOne/name/vendor/driver/subTitle/getOverviewInfo/
//          initFilterKey/loadBaseDeviceInfo/loadOtherDeviceInfo/loadTableData
// 缺陷闭环：DEF-DM-OTHERPCI-CAPABILITIES-VERSION 已在上游修复
//   （PR #848 / commit a684f10b，DeviceOtherPCI.cpp:39/102 m_Version -> m_Capabilities），
//   本文件用例已按修复后行为断言（Capabilities 写入/展示 m_Capabilities）
//
// 用例计数声明（min 为分级下限，actual 为实际用例数，均 ≤ 现有用例规模）
// | method                  | level | min | actual |
// | ctor/initFilterKey      | low   |   1 |      1 |
// | setInfoFromTomlOneByOne | low   |   4 |      4 |
// | getOverviewInfo/subTitle| low   |   1 |      1 |
// | getters                 | low   |   1 |      1 |
// | loadBaseDeviceInfo      | low   |   2 |      2 |
// | loadOtherDeviceInfo     | low   |   1 |      1 |
// | loadTableData           | low   |   1 |      1 |
//
// 打桩依据（dependency-tracer）：全类无外部依赖（无 DBus/网络/文件/进程），无需打桩

#include "DeviceOtherPCI.h"
#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

static bool ut_containsPair(const QList<QPair<QString, QString>> &lst, const QString &k, const QString &v)
{
    for (const auto &p : lst) {
        if (p.first == k && p.second == v)
            return true;
    }
    return false;
}

class UT_DeviceOtherPCI : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_pPCI = new DeviceOtherPCI;
    }
    void TearDown() override
    {
        delete m_pPCI;
        m_pPCI = nullptr;
    }
    DeviceOtherPCI *m_pPCI = nullptr;
};

// ===================== ctor =====================
// Branch: DeviceOtherPCI.cpp:11-25 成员初始化；ctor 未调用 initFilterKey 且其实现为空 -> m_FilterKey 恒空
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_ctor_default)
{
    // Arrange: SetUp 已构造默认 PCI 设备（构造即被测路径）
    EXPECT_NE(nullptr, m_pPCI);
    // Act: 读取构造后的初始状态；直接调用空实现 initFilterKey（白盒，构造器未调用）
    const QString name = m_pPCI->name();
    m_pPCI->initFilterKey();
    // Assert: 基础属性为空
    EXPECT_EQ(QString(""), name);
    EXPECT_EQ(QString(""), m_pPCI->vendor());
    EXPECT_EQ(QString(""), m_pPCI->driver());
    EXPECT_EQ(QString(""), m_pPCI->subTitle());
    EXPECT_EQ(QString(""), m_pPCI->m_BusInfo);
    EXPECT_EQ(QString(""), m_pPCI->m_Capabilities);
    EXPECT_EQ(QString(""), m_pPCI->m_Irq);
    EXPECT_EQ(QString(""), m_pPCI->m_Memory);
    EXPECT_EQ(QString(""), m_pPCI->m_Latency);
    EXPECT_EQ(QString(""), m_pPCI->m_InputOutput);
    // Assert: 过滤键为空
    EXPECT_TRUE(m_pPCI->m_FilterKey.isEmpty());
    // Assert: 其它信息为空
    EXPECT_EQ(0, m_pPCI->getOtherAttribs().size());
    // Assert: 表格数据惰性加载：loadTableData 追加三个空成员 -> ["","",""]
    const QStringList &emptyData = m_pPCI->getTableData();
    EXPECT_EQ(3, emptyData.size());
    EXPECT_EQ(QString(""), emptyData[0]);
    EXPECT_EQ(QString(""), emptyData[1]);
    EXPECT_EQ(QString(""), emptyData[2]);
}

// ===================== setInfoFromTomlOneByOne =====================
// Branch: DeviceOtherPCI.cpp:27-44 Model/Bus Info/Input-Output/Memory/IRQ/Latency 写入;
//         39 "Capabilities" 值写入 m_Capabilities（缺陷修复后行为）
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_setInfoFromToml_cover)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Model", "PCI123");
    map.insert("Bus Info", "pci@0000:00:1f.3");
    map.insert("Input/Output", "4000");
    map.insert("Memory", "a1300000-a131ffff");
    map.insert("IRQ", "16");
    map.insert("Latency", "32");
    map.insert("Capabilities", "msi pm");
    // Act
    TomlFixMethod ret = m_pPCI->setInfoFromTomlOneByOne(map);
    // Assert: 返回最后处理的 Capabilities 结果
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_EQ(QString("PCI123"), m_pPCI->m_Model);
    EXPECT_EQ(QString("pci@0000:00:1f.3"), m_pPCI->m_BusInfo);
    EXPECT_EQ(QString("4000"), m_pPCI->m_InputOutput);
    EXPECT_EQ(QString("a1300000-a131ffff"), m_pPCI->m_Memory);
    EXPECT_EQ(QString("16"), m_pPCI->m_Irq);
    EXPECT_EQ(QString("32"), m_pPCI->m_Latency);
    // Assert: Capabilities 写入 m_Capabilities；m_Version 不被 toml 数据触碰
    EXPECT_EQ(QString("msi pm"), m_pPCI->m_Capabilities);
    EXPECT_EQ(QString(""), m_pPCI->m_Version);
    // Assert: 过滤键为空 -> map 数据不收集；但 loadOtherDeviceInfo 的成员项仍会追加
    const QList<QPair<QString, QString>> &lst = m_pPCI->getOtherAttribs();
    EXPECT_EQ(5, lst.size());
    EXPECT_TRUE(ut_containsPair(lst, "Input/Output", "4000"));
    EXPECT_TRUE(ut_containsPair(lst, "Memory", "a1300000-a131ffff"));
    EXPECT_TRUE(ut_containsPair(lst, "IRQ", "16"));
    EXPECT_TRUE(ut_containsPair(lst, "Latency", "32"));
    EXPECT_TRUE(ut_containsPair(lst, "Capabilities", "msi pm"));
    EXPECT_FALSE(ut_containsPair(lst, "Driver", ""));
}

// Branch: DeviceInfo.cpp:988-998 键缺失/值为空 -> TOML_None
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_setInfoFromToml_none)
{
    // Arrange: 缺 Capabilities 键
    QMap<QString, QString> mapNoKey;
    mapNoKey.insert("Model", "PCI123");
    // Act: 缺键调用
    TomlFixMethod ret1 = m_pPCI->setInfoFromTomlOneByOne(mapNoKey);
    // Assert: 其余属性写入，Capabilities 缺失 -> TOML_None（m_Capabilities 保持空）
    EXPECT_EQ(TOML_None, ret1);
    EXPECT_EQ(QString("PCI123"), m_pPCI->m_Model);
    EXPECT_EQ(QString(""), m_pPCI->m_Capabilities);
    EXPECT_EQ(QString(""), m_pPCI->m_Version);

    // Arrange: Capabilities 为空值
    QMap<QString, QString> mapEmpty;
    mapEmpty.insert("Model", "PCI123");
    mapEmpty.insert("Capabilities", "");
    // Act: 空值调用
    TomlFixMethod ret2 = m_pPCI->setInfoFromTomlOneByOne(mapEmpty);
    // Assert
    EXPECT_EQ(TOML_None, ret2);
    EXPECT_EQ(QString(""), m_pPCI->m_Capabilities);
    EXPECT_EQ(QString(""), m_pPCI->m_Version);
}

// Branch: DeviceInfo.cpp:1000-1003 值含 nouse -> 清空 m_Capabilities 返回 TOML_nouse
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_setInfoFromToml_nouse)
{
    // Arrange: 预置 m_Capabilities/m_Version，Capabilities 含 nouse
    m_pPCI->m_Capabilities = "1.00";
    m_pPCI->m_Version = "2.00";
    QMap<QString, QString> map;
    map.insert("Capabilities", "nouse");
    // Act
    TomlFixMethod ret = m_pPCI->setInfoFromTomlOneByOne(map);
    // Assert: nouse 只清空 m_Capabilities，不影响 m_Version
    EXPECT_EQ(TOML_nouse, ret);
    EXPECT_EQ(QString(""), m_pPCI->m_Capabilities);
    EXPECT_EQ(QString("2.00"), m_pPCI->m_Version);
}

// Branch: DeviceOtherPCI.cpp:29-38 部分键缺失 -> 各属性独立处理互不影响
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_setInfoFromToml_partial)
{
    // Arrange: 仅 Model 与 IRQ
    QMap<QString, QString> map;
    map.insert("Model", "PCI123");
    map.insert("IRQ", "16");
    // Act
    TomlFixMethod ret = m_pPCI->setInfoFromTomlOneByOne(map);
    // Assert: 有值成员写入，无值成员保持空
    EXPECT_EQ(TOML_None, ret);
    EXPECT_EQ(QString("PCI123"), m_pPCI->m_Model);
    EXPECT_EQ(QString("16"), m_pPCI->m_Irq);
    EXPECT_EQ(QString(""), m_pPCI->m_BusInfo);
    EXPECT_EQ(QString(""), m_pPCI->m_Memory);
    EXPECT_EQ(QString(""), m_pPCI->m_Latency);
}

// ===================== 概况 / 副标题 / getter =====================
// Branch: DeviceOtherPCI.cpp:63-68 m_Name 为空 -> m_Model; 非空 -> m_Name
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_getOverviewInfo)
{
    // Arrange: 名称空、型号有值
    m_pPCI->m_Model = "PCI123";
    // Act: 名称空时概况
    const QString overviewEmptyName = m_pPCI->getOverviewInfo();
    // Assert: 回退返回型号
    EXPECT_EQ(QString("PCI123"), overviewEmptyName);

    // Arrange: 名称非空
    m_pPCI->m_Name = "Audio Controller";
    // Act: 名称非空时概况与副标题
    const QString overview = m_pPCI->getOverviewInfo();
    const QString sub = m_pPCI->subTitle();
    // Assert: 返回名称；副标题恒为型号
    EXPECT_EQ(QString("Audio Controller"), overview);
    EXPECT_EQ(QString("PCI123"), sub);
}

// Branch: DeviceOtherPCI.cpp:52-61 name/vendor/driver getter 直接返回成员
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_getters)
{
    // Arrange
    m_pPCI->m_Name = "Audio Controller";
    m_pPCI->m_Vendor = "Intel";
    m_pPCI->m_Driver = "snd_hda_intel";
    // Act: 逐一读取
    const QString name = m_pPCI->name();
    const QString vendor = m_pPCI->vendor();
    const QString driver = m_pPCI->driver();
    // Assert
    EXPECT_EQ(QString("Audio Controller"), name);
    EXPECT_EQ(QString("Intel"), vendor);
    EXPECT_EQ(QString("snd_hda_intel"), driver);
}

// ===================== loadBaseDeviceInfo =====================
// Branch: DeviceOtherPCI.cpp:90-97 Name 空时回退 Vendor+Model 拼接（无分隔符）;
//         DeviceInfo.cpp:896-904 空值跳过
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_loadBaseDeviceInfo_nameConcat)
{
    // Arrange: m_Name 为空 -> Name 项回退为 Vendor+Model 直接拼接
    m_pPCI->m_Vendor = "Intel";
    m_pPCI->m_Model = "PCI123";
    m_pPCI->m_Version = "1.00";
    // Act
    const QList<QPair<QString, QString>> &lst = m_pPCI->getBaseAttribs();
    // Assert
    EXPECT_TRUE(ut_containsPair(lst, "Name", "IntelPCI123"));
    EXPECT_TRUE(ut_containsPair(lst, "Vendor", "Intel"));
    EXPECT_TRUE(ut_containsPair(lst, "Model", "PCI123"));
    EXPECT_TRUE(ut_containsPair(lst, "Version", "1.00"));
    EXPECT_FALSE(ut_containsPair(lst, "Bus Info", ""));
    EXPECT_EQ(4, lst.size());
}

// Branch: DeviceOtherPCI.cpp:90 m_Name 非空 -> Name 项使用 m_Name
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_loadBaseDeviceInfo_nameSet)
{
    // Arrange
    m_pPCI->m_Name = "Audio Controller";
    m_pPCI->m_Vendor = "Intel";
    m_pPCI->m_Model = "PCI123";
    // Act
    const QList<QPair<QString, QString>> &lst = m_pPCI->getBaseAttribs();
    // Assert: 不再拼接回退
    EXPECT_TRUE(ut_containsPair(lst, "Name", "Audio Controller"));
    EXPECT_EQ(3, lst.size());
}

// ===================== loadOtherDeviceInfo =====================
// Branch: DeviceOtherPCI.cpp:99-113 Input-Output/Memory/IRQ/Latency/Driver/Capabilities(m_Capabilities)
//         + mapInfoToList(); 过滤键为空 -> map 数据恒不收集
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_loadOtherDeviceInfo)
{
    // Arrange: 成员赋值（Capabilities 列取 m_Capabilities）
    m_pPCI->m_InputOutput = "4000";
    m_pPCI->m_Memory = "a1300000-a131ffff";
    m_pPCI->m_Irq = "16";
    m_pPCI->m_Latency = "32";
    m_pPCI->m_Driver = "snd_hda_intel";
    m_pPCI->m_Capabilities = "msi pm";
    QMap<QString, QString> map;
    map.insert("IRQ", "999");          // 过滤键为空，map 值不收集
    m_pPCI->getOtherMapInfo(map);
    // Act
    const QList<QPair<QString, QString>> &lst = m_pPCI->getOtherAttribs();
    // Assert
    EXPECT_EQ(6, lst.size());
    EXPECT_TRUE(ut_containsPair(lst, "Input/Output", "4000"));
    EXPECT_TRUE(ut_containsPair(lst, "Memory", "a1300000-a131ffff"));
    EXPECT_TRUE(ut_containsPair(lst, "IRQ", "16"));
    EXPECT_TRUE(ut_containsPair(lst, "Latency", "32"));
    EXPECT_TRUE(ut_containsPair(lst, "Driver", "snd_hda_intel"));
    // Assert: Capabilities 列取的是 m_Capabilities（修复后行为）
    EXPECT_TRUE(ut_containsPair(lst, "Capabilities", "msi pm"));
    // Assert: map 的 IRQ 因过滤键为空不覆盖成员值
    EXPECT_FALSE(ut_containsPair(lst, "IRQ", "999"));
}

// ===================== loadTableData =====================
// Branch: DeviceOtherPCI.cpp:115-123 [m_Name, m_Vendor, m_Model]
TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_loadTableData)
{
    // Arrange
    m_pPCI->m_Name = "Audio Controller";
    m_pPCI->m_Vendor = "Intel";
    m_pPCI->m_Model = "PCI123";
    // Act
    const QStringList &data = m_pPCI->getTableData();
    const QStringList &header = m_pPCI->getTableHeader();
    // Assert: 数据与表头（m_CanEnable 默认 false -> "no" 列）
    EXPECT_EQ(QStringList() << "Audio Controller" << "Intel" << "PCI123", data);
    EXPECT_EQ(QStringList() << "Name" << "Vendor" << "Model" << "no", header);
}
