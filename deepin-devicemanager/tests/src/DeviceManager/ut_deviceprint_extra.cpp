// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 增量补全：覆盖 DevicePrint::vendor / makeAndeModel 访问器。
// 通过 setInfo 填充打印机信息后调用对应 getter 校验返回值。

#include "DevicePrint.h"
#include "DeviceInfo.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>

#include <gtest/gtest.h>

class UT_DevicePrintVendor : public UT_HEAD
{
public:
    void SetUp()
    {
        m_devicePrint = new DevicePrint;
    }
    void TearDown()
    {
        delete m_devicePrint;
    }
    DevicePrint *m_devicePrint = nullptr;
};

static void ut_print_setmap_vendor(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("printer-info", "Canon iR-ADV C3720 22.21");
    mapinfo.insert("Name", "Canon-iR-ADV-C3720-UFR");
    mapinfo.insert("device-uri", "socket://10.4.12.241");
    mapinfo.insert("printer-state", "3");
}

// const QString &vendor() const;
TEST_F(UT_DevicePrintVendor, Vendor_AfterSetInfo_ReturnsParsedVendor)
{
    QMap<QString, QString> mapinfo;
    ut_print_setmap_vendor(mapinfo);
    m_devicePrint->setInfo(mapinfo);

    // setInfo 解析 "printer-info" 首个单词作为厂商（"Canon"）
    EXPECT_STREQ("Canon", m_devicePrint->vendor().toStdString().c_str());
}

// const QString makeAndeModel() const;
TEST_F(UT_DevicePrintVendor, MakeAndeModel_MemberSet_ReturnsSameValue)
{
    // -fno-access-control 允许直接设置受保护成员，确保 getter 被覆盖且断言稳定
    m_devicePrint->m_MakeAndModel = "Canon iR-ADV C3720";
    EXPECT_STREQ("Canon iR-ADV C3720", m_devicePrint->makeAndeModel().toStdString().c_str());
}
