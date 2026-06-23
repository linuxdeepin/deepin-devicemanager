// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 HttpDriverInterface 的 protected 方法 getRequestBoard/Printer/Camera(桩 getRequestJson)
// + checkDriverInfo(桩 packageInstall 避免 apt)。-fno-access-control 下可访问 protected。

#include "HttpDriverInterface.h"
#include "MacroDefinition.h"
#include "ut_Head.h"
#include "stub.h"

#include <QString>
#include <QList>

#include <gtest/gtest.h>

// 桩 getRequestJson(protected 成员,首参 this):直接返回 URL,避免真实网络
static QString ut_req_getJson(HttpDriverInterface * /*self*/, QString strUrl)
{
    return strUrl;
}

// 桩 packageInstall(成员,首参 this):避免 apt 子进程
static int ut_req_packageInstall(HttpDriverInterface * /*self*/, const QString & /*a*/, const QString & /*b*/)
{
    return 2;   // 已安装最新
}

class UT_HttpDriverInterfaceReq : public UT_HEAD
{
public:
    void SetUp() override
    {
        h = HttpDriverInterface::getInstance();
    }
    HttpDriverInterface *h = nullptr;
};

TEST_F(UT_HttpDriverInterfaceReq, getRequestBoard)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getRequestJson), ut_req_getJson);
    EXPECT_NO_FATAL_FAILURE(h->getRequestBoard("Intel", "GMA", 3, 1));
    EXPECT_NO_FATAL_FAILURE(h->getRequestBoard());
}

TEST_F(UT_HttpDriverInterfaceReq, getRequestPrinter)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getRequestJson), ut_req_getJson);
    EXPECT_NO_FATAL_FAILURE(h->getRequestPrinter("Hewlett-Packard", "LaserJet"));
    EXPECT_NO_FATAL_FAILURE(h->getRequestPrinter());
}

TEST_F(UT_HttpDriverInterfaceReq, getRequestCamera)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getRequestJson), ut_req_getJson);
    EXPECT_NO_FATAL_FAILURE(h->getRequestCamera("Logitech C920"));
    EXPECT_NO_FATAL_FAILURE(h->getRequestCamera());
}

TEST_F(UT_HttpDriverInterfaceReq, checkDriverInfo_emptyJson)
{
    DriverInfo di;
    di.m_Type = DR_Gpu;
    EXPECT_NO_FATAL_FAILURE(h->checkDriverInfo("", &di));
}

TEST_F(UT_HttpDriverInterfaceReq, checkDriverInfo_validList)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, packageInstall), ut_req_packageInstall);
    DriverInfo di;
    di.m_Type = DR_Gpu;
    QString json = R"({"msg":"ok","data":{"list":[)"
                   R"({"packages":"gpu-driver","deb_version":"1.0","level":3,"size":1024}]}})";
    EXPECT_NO_FATAL_FAILURE(h->checkDriverInfo(json, &di));
}

TEST_F(UT_HttpDriverInterfaceReq, checkDriverInfo_badJson)
{
    DriverInfo di;
    di.m_Type = DR_Gpu;
    EXPECT_NO_FATAL_FAILURE(h->checkDriverInfo("{invalid json", &di));
}

// getRequest: 桩 getRequestBoard(返回非 error)+packageInstall,覆盖主体分发逻辑
static QString ut_req_getReqBoard(HttpDriverInterface * /*self*/, const QString & /*a*/, const QString & /*b*/, int /*c*/, int /*d*/)
{
    return "";
}

TEST_F(UT_HttpDriverInterfaceReq, getRequest_gpu)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getRequestBoard), ut_req_getReqBoard);
    stub.set(ADDR(HttpDriverInterface, packageInstall), ut_req_packageInstall);
    DriverInfo di;
    di.m_Type = DR_Gpu;
    EXPECT_NO_FATAL_FAILURE(h->getRequest(&di));
}

TEST_F(UT_HttpDriverInterfaceReq, getRequest_nullType)
{
    Stub stub;
    stub.set(ADDR(HttpDriverInterface, getRequestBoard), ut_req_getReqBoard);
    DriverInfo di;
    di.m_Type = DR_Null;
    EXPECT_NO_FATAL_FAILURE(h->getRequest(&di));
}
