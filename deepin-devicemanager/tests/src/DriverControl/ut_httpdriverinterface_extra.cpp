// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 HttpDriverInterface 未测的纯逻辑方法: getOsBuild/getVersion(读 /etc/os-version)。
// checkDriverInfo 因 DriverInfo 类型依赖暂不覆盖。

#include "HttpDriverInterface.h"
#include "ut_Head.h"

#include <QString>

#include <gtest/gtest.h>

class UT_HttpDriverInterfaceExtra : public UT_HEAD
{
};

// getOsBuild: 读 /etc/os-version(文件存在)
TEST_F(UT_HttpDriverInterfaceExtra, getOsBuild)
{
    EXPECT_NO_FATAL_FAILURE(HttpDriverInterface::getInstance()->getOsBuild());
}

// getVersion: 读版本文件
TEST_F(UT_HttpDriverInterfaceExtra, getVersion)
{
    QString major, minor;
    EXPECT_NO_FATAL_FAILURE(HttpDriverInterface::getInstance()->getVersion(major, minor));
}

// convertJsonToDeviceList: 有效 json 解析路径(纯逻辑)
// 重写(Mode 6): 原用例 msg:"ok" 无法通过源码 msg=="success" 门检, 实际只走了失败早退分支;
// 改为真实有效载荷并断言字段映射。
TEST_F(UT_HttpDriverInterfaceExtra, convertJsonToDeviceList_valid)
{
    QList<RepoDriverInfo> lst;
    QString json = "{\"msg\":\"success\",\"data\":{\"list\":[{\"arch\":\"amd64\","
                   "\"manufacturer\":\"ut-man\",\"packages\":\"ut-pkg\",\"deb_version\":\"1.0\","
                   "\"level\":5,\"size\":2048}]}}";
    bool r = HttpDriverInterface::getInstance()->convertJsonToDeviceList(json, lst);
    EXPECT_TRUE(r);
    EXPECT_EQ(1, lst.size());
    EXPECT_EQ(QStringLiteral("amd64"), lst.at(0).strArch);
    EXPECT_EQ(QStringLiteral("ut-man"), lst.at(0).strManufacturer);
    EXPECT_EQ(QStringLiteral("ut-pkg"), lst.at(0).strPackages);
    EXPECT_EQ(QStringLiteral("1.0"), lst.at(0).strDebVersion);
    EXPECT_EQ(5, lst.at(0).iLevel);
    EXPECT_EQ(2048, lst.at(0).bytes);
    EXPECT_EQ(QStringLiteral("2.00KB"), lst.at(0).strSize);
}
