// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// DriverScanner::run 主流程用例(任务06, cx=8):
// - Common::executeClientCmd(apt policy) 打桩, 不触真实 apt;
// - QThread::sleep 打桩(免等待);
// - HttpDriverInterface::getRequest 打桩(不触真实网络), 网络中断场景模拟其 sigRequestFinished 信号。
// 注: run() 中未限定 sleep 在 QThread 派生类作用域内解析为静态 QThread::sleep(unsigned long)。

#include "DriverScanner.h"
#include "HttpDriverInterface.h"
#include "MacroDefinition.h"
#include "commonfunction.h"
#include "ut_Head.h"
#include "stub.h"

#include <QSignalSpy>

#include <gtest/gtest.h>

// ---------------- stub 状态 ----------------
static QByteArray ut_sc_aptOut;              // executeClientCmd 桩返回的 apt policy 文本
static bool ut_sc_notInstalled = false;      // 桩返回未安装占位符
static int ut_sc_execCalls = 0;
static bool ut_sc_emitNetworkError = false;  // getRequest 桩是否模拟网络错误信号
static int ut_sc_getRequestCalls = 0;
static QString ut_sc_lastPkg;                // getRequest 桩捕获的包名

static void ut_reset_sc_stubs()
{
    ut_sc_aptOut.clear();
    ut_sc_notInstalled = false;
    ut_sc_execCalls = 0;
    ut_sc_emitNetworkError = false;
    ut_sc_getRequestCalls = 0;
    ut_sc_lastPkg.clear();
}

// ---------------- 桩函数 ----------------
// 返回与请求包名匹配的 apt policy 文本(保证 startsWith 匹配命中)
static QByteArray ut_stub_executeClientCmd(const QString &, const QStringList &args, const QString &, int, bool)
{
    ++ut_sc_execCalls;
    const QString pkg = args.size() > 1 ? args.at(1) : QStringLiteral("ut-pkg");
    const QString out = ut_sc_notInstalled
        ? pkg + ":\n  Installed: (none)\n  Candidate: 1.2.3\n"
        : pkg + ":\n  Installed: 1.2.3\n  Candidate: 1.2.3\n";
    return out.toUtf8();
}

static void ut_stub_qthread_sleep(unsigned long) { }

static void ut_stub_hdi_getRequest(void *self, DriverInfo *info)
{
    ++ut_sc_getRequestCalls;
    ut_sc_lastPkg = info->packages();
    if (ut_sc_emitNetworkError)
        emit static_cast<HttpDriverInterface *>(self)->sigRequestFinished(false, QStringLiteral("network error"));
}

class UT_DriverScannerRun : public UT_HEAD
{
public:
    void SetUp() override
    {
        ut_reset_sc_stubs();
        m_scanner = new DriverScanner;
    }
    void TearDown() override
    {
        delete m_scanner;
        m_scanner = nullptr;
    }

    // 造一个驱动信息(堆上, scanner 不接管所有权)
    DriverInfo *makeInfo(const QString &pkg)
    {
        DriverInfo *info = new DriverInfo;
        info->m_Name = pkg;
        info->m_Packages = pkg;
        return info;
    }

    DriverScanner *m_scanner = nullptr;
};

TEST_F(UT_DriverScannerRun, normalScan_success)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_stub_executeClientCmd);
    stub.set(static_cast<void (*)(unsigned long)>(&QThread::sleep), ut_stub_qthread_sleep);
    stub.set(ADDR(HttpDriverInterface, getRequest), ut_stub_hdi_getRequest);

    QList<DriverInfo *> lst;
    lst << makeInfo("pkg-1") << makeInfo("pkg-2");
    m_scanner->setDriverList(lst);

    QSignalSpy spyScan(m_scanner, &DriverScanner::scanInfo);
    QSignalSpy spyEnd(m_scanner, &DriverScanner::scanFinished);
    m_scanner->run();

    EXPECT_EQ(2, ut_sc_getRequestCalls);
    EXPECT_EQ(2, ut_sc_execCalls);
    EXPECT_EQ(2, spyScan.count());
    EXPECT_EQ(QStringLiteral("pkg-2"), ut_sc_lastPkg);
    // 本地安装版本解析
    EXPECT_EQ(QStringLiteral("1.2.3"), lst.at(0)->m_Version);
    ASSERT_EQ(1, spyEnd.count());
    EXPECT_EQ(SR_SUCESS, spyEnd.at(0).at(0).toInt());

    qDeleteAll(lst);
}

TEST_F(UT_DriverScannerRun, aptPolicyNotInstalled_versionEmpty)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_stub_executeClientCmd);
    stub.set(static_cast<void (*)(unsigned long)>(&QThread::sleep), ut_stub_qthread_sleep);
    stub.set(ADDR(HttpDriverInterface, getRequest), ut_stub_hdi_getRequest);
    ut_sc_notInstalled = true;

    QList<DriverInfo *> lst;
    lst << makeInfo("pkg-1");
    lst.at(0)->m_Version = "orig";
    m_scanner->setDriverList(lst);

    QSignalSpy spyEnd(m_scanner, &DriverScanner::scanFinished);
    m_scanner->run();
    // 未安装 -> 版本保持原样
    EXPECT_EQ(QStringLiteral("orig"), lst.at(0)->m_Version);
    ASSERT_EQ(1, spyEnd.count());
    EXPECT_EQ(SR_SUCESS, spyEnd.at(0).at(0).toInt());

    qDeleteAll(lst);
}

TEST_F(UT_DriverScannerRun, emptyPackages_skipsApt)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_stub_executeClientCmd);
    stub.set(static_cast<void (*)(unsigned long)>(&QThread::sleep), ut_stub_qthread_sleep);
    stub.set(ADDR(HttpDriverInterface, getRequest), ut_stub_hdi_getRequest);

    QList<DriverInfo *> lst;
    DriverInfo *info = makeInfo("pkg-1");
    info->m_Packages.clear();
    lst << info;
    m_scanner->setDriverList(lst);

    QSignalSpy spyEnd(m_scanner, &DriverScanner::scanFinished);
    m_scanner->run();
    EXPECT_EQ(1, ut_sc_getRequestCalls);
    EXPECT_EQ(0, ut_sc_execCalls);
    ASSERT_EQ(1, spyEnd.count());
    EXPECT_EQ(SR_SUCESS, spyEnd.at(0).at(0).toInt());

    delete info;
}

TEST_F(UT_DriverScannerRun, emptyList_successNoScan)
{
    Stub stub;
    stub.set(static_cast<void (*)(unsigned long)>(&QThread::sleep), ut_stub_qthread_sleep);
    stub.set(ADDR(HttpDriverInterface, getRequest), ut_stub_hdi_getRequest);

    m_scanner->setDriverList(QList<DriverInfo *>());
    QSignalSpy spyScan(m_scanner, &DriverScanner::scanInfo);
    QSignalSpy spyEnd(m_scanner, &DriverScanner::scanFinished);
    m_scanner->run();
    EXPECT_EQ(0, ut_sc_getRequestCalls);
    EXPECT_EQ(0, spyScan.count());
    ASSERT_EQ(1, spyEnd.count());
    EXPECT_EQ(SR_SUCESS, spyEnd.at(0).at(0).toInt());
}

TEST_F(UT_DriverScannerRun, networkError_midScan_interrupted)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_stub_executeClientCmd);
    stub.set(static_cast<void (*)(unsigned long)>(&QThread::sleep), ut_stub_qthread_sleep);
    stub.set(ADDR(HttpDriverInterface, getRequest), ut_stub_hdi_getRequest);
    ut_sc_emitNetworkError = true;

    QList<DriverInfo *> lst;
    lst << makeInfo("pkg-1") << makeInfo("pkg-2");
    m_scanner->setDriverList(lst);

    QSignalSpy spyEnd(m_scanner, &DriverScanner::scanFinished);
    m_scanner->run();
    // 第一个驱动请求即触发 sigRequestFinished -> m_IsStop=true
    EXPECT_EQ(1, ut_sc_getRequestCalls);
    // else 分支与循环末尾各发一次 SR_NETWORD_ERR
    ASSERT_EQ(2, spyEnd.count());
    EXPECT_EQ(SR_NETWORD_ERR, spyEnd.at(0).at(0).toInt());
    EXPECT_EQ(SR_NETWORD_ERR, spyEnd.at(1).at(0).toInt());

    qDeleteAll(lst);
}
