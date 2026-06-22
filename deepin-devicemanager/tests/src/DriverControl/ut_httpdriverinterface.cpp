// Copyright (C) 2019 ~ 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

#define private public
#define protected public
#include "HttpDriverInterface.h"

class UT_HttpDriverInterface : public UT_HEAD
{
public:
    HttpDriverInterface *m_iface = nullptr;
    void SetUp() override
    {
        m_iface = HttpDriverInterface::getInstance();
    }
};

// 单例：多次获取应返回同一指针
TEST_F(UT_HttpDriverInterface, UT_HttpDriverInterface_getInstance)
{
    HttpDriverInterface *a = HttpDriverInterface::getInstance();
    HttpDriverInterface *b = HttpDriverInterface::getInstance();
    EXPECT_EQ(a, b);
    EXPECT_EQ(a, m_iface);
}

// 空字符串
TEST_F(UT_HttpDriverInterface, UT_HttpDriverInterface_convertJson_empty)
{
    QList<RepoDriverInfo> lst;
    EXPECT_FALSE(m_iface->convertJsonToDeviceList("", lst));
}

// 非法 JSON
TEST_F(UT_HttpDriverInterface, UT_HttpDriverInterface_convertJson_invalid)
{
    QList<RepoDriverInfo> lst;
    EXPECT_FALSE(m_iface->convertJsonToDeviceList("{not a valid json", lst));
}

// msg != success
TEST_F(UT_HttpDriverInterface, UT_HttpDriverInterface_convertJson_badMsg)
{
    QList<RepoDriverInfo> lst;
    QString json = R"({"msg":"failed","data":{"list":[]}})";
    EXPECT_FALSE(m_iface->convertJsonToDeviceList(json, lst));
}

// list 为空
TEST_F(UT_HttpDriverInterface, UT_HttpDriverInterface_convertJson_emptyList)
{
    QList<RepoDriverInfo> lst;
    QString json = R"({"msg":"success","data":{"list":[]}})";
    EXPECT_FALSE(m_iface->convertJsonToDeviceList(json, lst));
}

// 合法 JSON：解析字段、KB/MB 单位、ppds
TEST_F(UT_HttpDriverInterface, UT_HttpDriverInterface_convertJson_valid)
{
    QList<RepoDriverInfo> lst;
    QString json = R"({"msg":"success","data":{"list":[)"
                   R"({"arch":"amd64","manufacturer":"Intel","deb_manufacturer":"Intel Corp",)"
                   R"("version":"1.0","deb_version":"1.0","packages":"intel-driver","class_p":"display",)"
                   R"("class":"gpu","models":["modelA","modelB"],"products":"prod123",)"
                   R"("deb":"http://example.com/d.deb","level":3,"system":"x86",)"
                   R"("desc":"Intel GPU driver","adaptation":"V20","source":"repo",)"
                   R"("download_url":"http://example.com/dl","size":524288,)"
                   R"("ppds":[{"desc":"ppd1","manufacturer":"HP","source":"src1"}]},)"
                   R"({"arch":"amd64","manufacturer":"NVIDIA","size":5242880}]}})";
    EXPECT_TRUE(m_iface->convertJsonToDeviceList(json, lst));
    ASSERT_EQ(2, lst.size());

    // 第一项
    const RepoDriverInfo &d0 = lst.at(0);
    EXPECT_STREQ("amd64", d0.strArch.toStdString().c_str());
    EXPECT_STREQ("Intel", d0.strManufacturer.toStdString().c_str());
    EXPECT_STREQ("Intel Corp", d0.strDebManufacturer.toStdString().c_str());
    EXPECT_STREQ("1.0", d0.strVersion.toStdString().c_str());
    EXPECT_STREQ("intel-driver", d0.strPackages.toStdString().c_str());
    EXPECT_STREQ("display", d0.strClass_p.toStdString().c_str());
    EXPECT_STREQ("gpu", d0.strClass.toStdString().c_str());
    EXPECT_EQ(2, d0.strModels.size());
    EXPECT_STREQ("modelA", d0.strModels.at(0).toStdString().c_str());
    EXPECT_STREQ("modelB", d0.strModels.at(1).toStdString().c_str());
    EXPECT_EQ(3, d0.iLevel);
    EXPECT_STREQ("Intel GPU driver", d0.strDesc.toStdString().c_str());
    EXPECT_STREQ("http://example.com/d.deb", d0.strDeb.toStdString().c_str());
    // size 524288 < 1MB => KB
    EXPECT_STREQ("512.00KB", d0.strSize.toStdString().c_str());
    ASSERT_EQ(1, d0.lstPpds.size());
    EXPECT_STREQ("ppd1", d0.lstPpds.at(0).strDesc.toStdString().c_str());
    EXPECT_STREQ("HP", d0.lstPpds.at(0).strManufacturer.toStdString().c_str());

    // 第二项：size 5242880 < 1GB => MB
    const RepoDriverInfo &d1 = lst.at(1);
    EXPECT_STREQ("5.00MB", d1.strSize.toStdString().c_str());
}

// GB 单位分支
TEST_F(UT_HttpDriverInterface, UT_HttpDriverInterface_convertJson_sizeGB)
{
    QList<RepoDriverInfo> lst;
    // 1GiB = 1073741824 bytes，可放入 int，触发 GB 分支
    QString json = R"({"msg":"success","data":{"list":[{"manufacturer":"X","size":1073741824}]}})";
    EXPECT_TRUE(m_iface->convertJsonToDeviceList(json, lst));
    ASSERT_EQ(1, lst.size());
    EXPECT_STREQ("1.00GB", lst.at(0).strSize.toStdString().c_str());
}
