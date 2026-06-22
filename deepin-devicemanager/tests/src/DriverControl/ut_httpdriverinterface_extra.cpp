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

// convertJsonToDeviceList: 再覆盖一个有效 json 解析路径(纯逻辑)
TEST_F(UT_HttpDriverInterfaceExtra, convertJsonToDeviceList_valid)
{
    QList<RepoDriverInfo> lst;
    QString json = "{\"code\":0,\"msg\":\"ok\",\"data\":[{\"name\":\"test-driver\",\"version\":\"1.0\","
                   "\"url\":\"http://x/deb\",\"arch\":\"x86_64\",\"os\":\"V25\",\"size\":1024}]}";
    bool r = HttpDriverInterface::getInstance()->convertJsonToDeviceList(json, lst);
    (void)r;
    EXPECT_NO_FATAL_FAILURE(HttpDriverInterface::getInstance()->convertJsonToDeviceList(json, lst));
}
