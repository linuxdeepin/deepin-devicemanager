// Copyright (C) 2019 ~ 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QDomDocument>
#include <QTextStream>
#include <QStringList>

#define private public
#define protected public
#include "DeviceCpu.h"
#include "DeviceInfo.h"

// 用 DeviceCpu 作为 DeviceBaseInfo 的具体子类来补充测试基类方法
class UT_DeviceInfoBase : public UT_HEAD
{
public:
    void SetUp() override
    {
        m_cpu = new DeviceCpu;
    }
    void TearDown() override
    {
        delete m_cpu;
    }
    DeviceCpu *m_cpu = nullptr;
};

// ===================== isValueValid：覆盖所有无效分支 + 有效值 =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_isValueValid_invalid)
{
    EXPECT_FALSE(m_cpu->isValueValid(""));
    EXPECT_FALSE(m_cpu->isValueValid("Unknown"));
    EXPECT_FALSE(m_cpu->isValueValid("N/A"));
    EXPECT_FALSE(m_cpu->isValueValid("n/a"));
    EXPECT_FALSE(m_cpu->isValueValid("Null"));
    EXPECT_FALSE(m_cpu->isValueValid("none"));
    EXPECT_FALSE(m_cpu->isValueValid("Not Provided"));
    EXPECT_FALSE(m_cpu->isValueValid("Not Specified"));
    EXPECT_FALSE(m_cpu->isValueValid("Default string"));
    EXPECT_FALSE(m_cpu->isValueValid("Unspecified"));
    EXPECT_FALSE(m_cpu->isValueValid("Not Present"));
    EXPECT_FALSE(m_cpu->isValueValid("<OUT OF SPEC>"));
    EXPECT_FALSE(m_cpu->isValueValid("Other"));
    EXPECT_FALSE(m_cpu->isValueValid("TBD"));
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_isValueValid_valid)
{
    EXPECT_TRUE(m_cpu->isValueValid("Intel Core i3"));
    EXPECT_TRUE(m_cpu->isValueValid("GenuineIntel"));
    EXPECT_TRUE(m_cpu->isValueValid("1024 MB"));
}

// ===================== displayWidth（静态） =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_displayWidth)
{
    EXPECT_DOUBLE_EQ(0.0, DeviceBaseInfo::displayWidth(""));
    EXPECT_DOUBLE_EQ(3.0, DeviceBaseInfo::displayWidth("abc"));
    // 两个 CJK 字符，各占 1.5 => 3.0
    EXPECT_DOUBLE_EQ(3.0, DeviceBaseInfo::displayWidth(QString::fromUtf8("中文")));
    // 混合：1 ASCII(1) + 1 CJK(1.5) = 2.5
    EXPECT_DOUBLE_EQ(2.5, DeviceBaseInfo::displayWidth(QString::fromUtf8("a中")));
}

// ===================== 简单 set/get 对 =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_hardwareClass)
{
    m_cpu->setHardwareClass("cpu");
    EXPECT_STREQ("cpu", m_cpu->hardwareClass().toStdString().c_str());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_uniqueID)
{
    m_cpu->setUniqueID("unique-123");
    EXPECT_STREQ("unique-123", m_cpu->uniqueID().toStdString().c_str());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_sysPath)
{
    m_cpu->setSysPath("/sys/devices/cpu");
    EXPECT_STREQ("/sys/devices/cpu", m_cpu->sysPath().toStdString().c_str());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_canEnable)
{
    EXPECT_FALSE(m_cpu->canEnable());
    m_cpu->setCanEnable(true);
    EXPECT_TRUE(m_cpu->canEnable());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_canUninstall)
{
    EXPECT_FALSE(m_cpu->canUninstall());
    m_cpu->setCanUninstall(true);
    EXPECT_TRUE(m_cpu->canUninstall());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setEnableValue)
{
    m_cpu->setEnableValue(false);
    EXPECT_FALSE(m_cpu->m_Enable);
    m_cpu->setEnableValue(true);
    EXPECT_TRUE(m_cpu->m_Enable);
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setForcedDisplay)
{
    m_cpu->setForcedDisplay(true);
    EXPECT_TRUE(m_cpu->m_forcedDisplay);
}

// ===================== VID / PID / VID_PID / Modalias =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_VID_PID_Modalias)
{
    m_cpu->m_VID = "8086";
    m_cpu->m_PID = "1234";
    m_cpu->m_VID_PID = "8086:1234";
    m_cpu->m_Modalias = "pci:v1234";

    EXPECT_STREQ("8086", m_cpu->getVID().toStdString().c_str());
    EXPECT_STREQ("1234", m_cpu->getPID().toStdString().c_str());
    EXPECT_STREQ("8086:1234", m_cpu->getVIDAndPID().toStdString().c_str());
    EXPECT_STREQ("pci:v1234", m_cpu->getModalias().toStdString().c_str());
}

// ===================== addFilterKey =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_addFilterKey)
{
    // DeviceCpu 构造已预置部分 filter key，先清空以独立验证
    m_cpu->m_FilterKey.clear();
    m_cpu->addFilterKey("Name");
    m_cpu->addFilterKey("Vendor");
    m_cpu->addFilterKey("Name");  // 重复不增加
    EXPECT_EQ(2, m_cpu->m_FilterKey.size());
}

// ===================== addOtherDeviceInfo（头插） =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_addOtherDeviceInfo)
{
    m_cpu->addOtherDeviceInfo("K1", "V1");
    m_cpu->addOtherDeviceInfo("K2", "");  // 空值不添加
    m_cpu->addOtherDeviceInfo("K3", "V3");  // 头插，排最前
    EXPECT_EQ(2, m_cpu->m_LstOtherInfo.size());
    EXPECT_STREQ("K3", m_cpu->m_LstOtherInfo.at(0).first.toStdString().c_str());
    EXPECT_STREQ("K1", m_cpu->m_LstOtherInfo.at(1).first.toStdString().c_str());
}

// ===================== setOtherDeviceInfo =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setOtherDeviceInfo)
{
    m_cpu->setOtherDeviceInfo("Key1", "Value1");
    EXPECT_STREQ("Value1", m_cpu->m_MapOtherInfo["Key1"].toStdString().c_str());
}

// ===================== getOtherMapInfo =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getOtherMapInfo)
{
    m_cpu->addFilterKey("Speed");
    QMap<QString, QString> map;
    map.insert("Speed", "2667");
    map.insert("Junk", "xyz");  // 不在 FilterKey 中，忽略
    m_cpu->getOtherMapInfo(map);
    EXPECT_STREQ("2667", m_cpu->m_MapOtherInfo["Speed"].toStdString().c_str());
    EXPECT_FALSE(m_cpu->m_MapOtherInfo.contains("Junk"));
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getOtherMapInfo_nouse)
{
    m_cpu->addFilterKey("Status");
    QMap<QString, QString> map;
    map.insert("Status", "this is NOUSE value");
    m_cpu->getOtherMapInfo(map);
    // 含 "nouse"（忽略大小写）会被 remove
    EXPECT_FALSE(m_cpu->m_MapOtherInfo.contains("Status"));
}

// ===================== setAttribute =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setAttribute)
{
    QMap<QString, QString> map;
    map.insert("Name", "Intel CPU");
    QString var;
    m_cpu->setAttribute(map, "Name", var);
    EXPECT_STREQ("Intel CPU", var.toStdString().c_str());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setAttribute_noKey)
{
    QMap<QString, QString> map;
    QString var = "old";
    m_cpu->setAttribute(map, "Missing", var);
    EXPECT_STREQ("old", var.toStdString().c_str());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setAttribute_overwriteFalse)
{
    QMap<QString, QString> map;
    map.insert("Name", "NewValue");
    QString var = "Existing";
    // overwrite=false 且 var 非空 => 不覆盖
    m_cpu->setAttribute(map, "Name", var, false);
    EXPECT_STREQ("Existing", var.toStdString().c_str());
}

// ===================== nameTr =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_nameTr)
{
    m_cpu->m_Name = "Intel";
    // 无翻译文件时 tr 返回原文
    EXPECT_STREQ("Intel", m_cpu->nameTr().toStdString().c_str());
}

// ===================== getBaseAttribs / getOtherAttribs（走 DeviceCpu 实现） =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getBaseAttribs)
{
    const auto &base = m_cpu->getBaseAttribs();
    EXPECT_TRUE(base.isEmpty());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getOtherAttribs)
{
    const auto &other = m_cpu->getOtherAttribs();
    EXPECT_TRUE(other.isEmpty());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getBaseTranslationAttribs)
{
    // getBaseAttribs 会清空并按空数据加载 => 0 项，仍会执行翻译循环
    const auto &tr = m_cpu->getBaseTranslationAttribs();
    EXPECT_EQ(0, tr.size());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getOtherTranslationAttribs)
{
    const auto &tr = m_cpu->getOtherTranslationAttribs();
    EXPECT_EQ(0, tr.size());
}

// ===================== getTableHeader / getTableData =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getTableHeader)
{
    const auto &header = m_cpu->getTableHeader();
    EXPECT_FALSE(header.isEmpty());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getTableData)
{
    const auto &data = m_cpu->getTableData();
    EXPECT_FALSE(data.isEmpty());
}

// ===================== toHtmlString / subTitleToHTML =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_toHtmlString)
{
    m_cpu->addBaseDeviceInfo("Name", "Intel");
    m_cpu->getBaseAttribs();
    m_cpu->getBaseTranslationAttribs();

    QDomDocument doc;
    m_cpu->toHtmlString(doc);
    EXPECT_FALSE(doc.isNull());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_subTitleToHTML)
{
    QDomDocument doc;
    // DeviceCpu 默认 subTitle() 为空 => 不添加 h3
    m_cpu->subTitleToHTML(doc);
    SUCCEED();
}

// ===================== readDeviceInfoKeyValue =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_readDeviceInfoKeyValue_emptyKey)
{
    EXPECT_STREQ("", m_cpu->readDeviceInfoKeyValue("").toStdString().c_str());
}

// ===================== getDriverVersion（调用 modinfo，空驱动返回空） =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getDriverVersion)
{
    // DeviceCpu::driver() 返回空 => modinfo 无输出 => 返回 ""
    QString v = m_cpu->getDriverVersion();
    EXPECT_STREQ("", v.toStdString().c_str());
}

// ===================== tableInfoToTxt / tableHeaderToTxt =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_tableToTxt)
{
    QString out;
    QTextStream stream(&out);
    QList<double> widths;
    widths << 20 << 20;
    m_cpu->tableHeaderToTxt(stream, widths);
    m_cpu->tableInfoToTxt(stream, widths);
    EXPECT_FALSE(out.isEmpty());
}
