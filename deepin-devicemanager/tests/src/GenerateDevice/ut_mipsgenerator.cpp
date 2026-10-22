// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// 分支覆盖清单（基于 worktree ut-t09 @ f2d46ee9 源码逐分支核对）
// 被测: src/GenerateDevice/MipsGenerator.cpp
//
// MipsGenerator::MipsGenerator: 空构造(仅日志)
// MipsGenerator::generatorComputerDevice(28-91, complexity:7/cognitive:8):
//   B1 cat_os_release 非空 -> setHomeUrl(HOME_URL 去除引号);
//   B2 lshw_system 非空 -> setType(description)/setVendor(vendor)/setName(product);
//   B3 dmidecode1 size>1(严格大于) -> setName(dmiInfo[1]["Product Name"]);
//   B4 setOsDescription(DeviceGenerator::getProductName()) 无条件执行;
//   B5 cat_version 非空 -> OS 字段正则 "\\(gcc [\\s\\S]*(\\([\\s\\S]*\\))\\)" 命中 ->
//     剔除匹配段并在原位插入内层括号内容(如 "UOS 20 (gcc 8.3.0 (Uos 8.3.0-6))"
//     -> "UOS 20 (Uos 8.3.0-6)"); B6 未命中 -> setOS 原文;
//   B7 addComputerDevice 无条件追加
// 说明: 数据全部经 DeviceManager::addCmdInfo 注入, 无 QProcess/文件/DBus 依赖;
// getProductName 为 Dtk 库调用, 直接执行本机真实返回(非空即可)。
// ============================================================================

#include "MipsGenerator.h"
#include "DeviceGenerator.h"
#include "DeviceManager.h"
#include "DeviceComputer.h"
#include "ut_Head.h"

#include <gtest/gtest.h>

class UT_MipsGenerator : public UT_HEAD
{
public:
    MipsGenerator *m_generator = nullptr;

    void SetUp() override
    {
        DeviceManager::instance()->clear();
        m_generator = new MipsGenerator;
    }

    void TearDown() override
    {
        delete m_generator;
        m_generator = nullptr;
        DeviceManager::instance()->clear();
    }

    static DeviceManager *dm() { return DeviceManager::instance(); }
};

/// 构造: 可实例化且为 DeviceGenerator 派生
TEST_F(UT_MipsGenerator, Constructor_Polymorphic)
{
    DeviceGenerator *base = m_generator;
    EXPECT_NE(dynamic_cast<MipsGenerator *>(base), nullptr);
}

/// B1: HOME_URL 引号剥离
TEST_F(UT_MipsGenerator, Computer_HomeUrlQuotesStripped)
{
    QMap<QString, QList<QMap<QString, QString>>> data;
    data["cat_os_release"] = QList<QMap<QString, QString>>()
                             << QMap<QString, QString> {{"HOME_URL", "\"https://www.deepin.org\""}};
    dm()->addCmdInfo(data);

    m_generator->generatorComputerDevice();

    ASSERT_EQ(dm()->m_ListDeviceComputer.size(), 1);
    DeviceComputer *computer = dynamic_cast<DeviceComputer *>(dm()->m_ListDeviceComputer[0]);
    ASSERT_NE(computer, nullptr);
    EXPECT_EQ(computer->m_HomeUrl, "https://www.deepin.org");
}

/// B2: lshw_system -> type/vendor/name
TEST_F(UT_MipsGenerator, Computer_SystemInfoSetsFields)
{
    QMap<QString, QList<QMap<QString, QString>>> data;
    data["lshw_system"] = QList<QMap<QString, QString>>()
                          << QMap<QString, QString> {{"description", "Notebook"},
                                                     {"vendor", "Loongson"},
                                                     {"product", "3A5000"}};
    dm()->addCmdInfo(data);

    m_generator->generatorComputerDevice();

    ASSERT_EQ(dm()->m_ListDeviceComputer.size(), 1);
    DeviceComputer *computer = dynamic_cast<DeviceComputer *>(dm()->m_ListDeviceComputer[0]);
    ASSERT_NE(computer, nullptr);
    EXPECT_EQ(computer->name(), "3A5000");
    EXPECT_EQ(computer->vendor(), "Loongson");
    EXPECT_EQ(computer->m_Type, "Notebook");
}

/// B3: dmidecode1 取第二条目(index 1)的 Product Name
TEST_F(UT_MipsGenerator, Computer_DmiSecondEntryOverridesName)
{
    QMap<QString, QList<QMap<QString, QString>>> data;
    data["lshw_system"] = QList<QMap<QString, QString>>()
                          << QMap<QString, QString> {{"description", "Desktop"}, {"vendor", "Loongson"}, {"product", "LC-3A5000"}};
    data["dmidecode1"] = QList<QMap<QString, QString>>()
                         << QMap<QString, QString> {{"Manufacturer", "Loongson"}}
                         << QMap<QString, QString> {{"Product Name", "Loongson-3A5000-7A1000-PC"}};
    dm()->addCmdInfo(data);

    m_generator->generatorComputerDevice();

    ASSERT_EQ(dm()->m_ListDeviceComputer.size(), 1);
    DeviceComputer *computer = dynamic_cast<DeviceComputer *>(dm()->m_ListDeviceComputer[0]);
    ASSERT_NE(computer, nullptr);
    EXPECT_EQ(computer->name(), "Loongson-3A5000-7A1000-PC");
}

/// B3(边界): dmidecode1 仅一条(size==1) -> name 不被覆盖, 保持 lshw product
TEST_F(UT_MipsGenerator, Computer_DmiSingleEntry_KeepsLshwName)
{
    QMap<QString, QList<QMap<QString, QString>>> data;
    data["lshw_system"] = QList<QMap<QString, QString>>()
                          << QMap<QString, QString> {{"description", "Desktop"}, {"vendor", "Loongson"}, {"product", "LC-3A5000"}};
    data["dmidecode1"] = QList<QMap<QString, QString>>()
                         << QMap<QString, QString> {{"Product Name", "Should-Not-Use"}};
    dm()->addCmdInfo(data);

    m_generator->generatorComputerDevice();

    ASSERT_EQ(dm()->m_ListDeviceComputer.size(), 1);
    DeviceComputer *computer = dynamic_cast<DeviceComputer *>(dm()->m_ListDeviceComputer[0]);
    ASSERT_NE(computer, nullptr);
    EXPECT_EQ(computer->name(), "LC-3A5000");
}

/// B4: OsDescription 无条件取 getProductName(非空)
TEST_F(UT_MipsGenerator, Computer_OsDescriptionAlwaysSet)
{
    m_generator->generatorComputerDevice();

    ASSERT_EQ(dm()->m_ListDeviceComputer.size(), 1);
    DeviceComputer *computer = dynamic_cast<DeviceComputer *>(dm()->m_ListDeviceComputer[0]);
    ASSERT_NE(computer, nullptr);
    EXPECT_FALSE(computer->m_OsDescription.isEmpty());
}

/// B5: cat_version 含 gcc 括号 -> 归一化为内层括号内容
TEST_F(UT_MipsGenerator, Computer_OsVersionGccNormalized)
{
    QMap<QString, QList<QMap<QString, QString>>> data;
    data["cat_version"] = QList<QMap<QString, QString>>()
                          << QMap<QString, QString> {{"OS", "UOS 20 (gcc 8.3.0 (Uos 8.3.0-6))"}};
    dm()->addCmdInfo(data);

    m_generator->generatorComputerDevice();

    ASSERT_EQ(dm()->m_ListDeviceComputer.size(), 1);
    DeviceComputer *computer = dynamic_cast<DeviceComputer *>(dm()->m_ListDeviceComputer[0]);
    ASSERT_NE(computer, nullptr);
    EXPECT_EQ(computer->m_OS, "UOS 20 (Uos 8.3.0-6)");
}

/// B6: cat_version 不含 gcc -> setOS 原文(已 trim)
TEST_F(UT_MipsGenerator, Computer_OsVersionNoGcc_KeepsRaw)
{
    QMap<QString, QList<QMap<QString, QString>>> data;
    data["cat_version"] = QList<QMap<QString, QString>>()
                          << QMap<QString, QString> {{"OS", "  20 community  "}};
    dm()->addCmdInfo(data);

    m_generator->generatorComputerDevice();

    ASSERT_EQ(dm()->m_ListDeviceComputer.size(), 1);
    DeviceComputer *computer = dynamic_cast<DeviceComputer *>(dm()->m_ListDeviceComputer[0]);
    ASSERT_NE(computer, nullptr);
    EXPECT_EQ(computer->m_OS, "20 community");
}

/// 空数据: 仍创建一台默认计算机设备(B7 无条件追加)
TEST_F(UT_MipsGenerator, Computer_EmptyCmdInfo_StillAddsDevice)
{
    m_generator->generatorComputerDevice();
    ASSERT_EQ(dm()->m_ListDeviceComputer.size(), 1);
    DeviceComputer *computer = dynamic_cast<DeviceComputer *>(dm()->m_ListDeviceComputer[0]);
    ASSERT_NE(computer, nullptr);
    EXPECT_FALSE(computer->m_OsDescription.isEmpty()); // B4 仍执行
}
