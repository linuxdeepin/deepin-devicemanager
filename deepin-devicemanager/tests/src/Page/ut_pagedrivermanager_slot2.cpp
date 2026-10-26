// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QSignalSpy>

#include "ut_Head.h"
#include "stub.h"

#define private public
#include "PageDriverManager.h"
#include "PageDriverInstallInfo.h"
#include "PageDriverBackupInfo.h"
#include "PageDriverRestoreInfo.h"
#include "DriverBackupThread.h"
#include "DriverScanner.h"
#include "DBusDriverInterface.h"
#include "commontools.h"
#include "ut_mock_thread.h"

static QString ut_pdmp_backupPath;
static QString ut_pdmp_getBackupPath()
{
    return ut_pdmp_backupPath;
}

static void ut_pdmp_threadRun()
{
}

// 重新安装桩:记录 2 参数 installDriver 调用
static int ut_pdmp_reinstallCnt = 0;
static void ut_pdmp_installDriver2(const QString &, const QString &)
{
    ut_pdmp_reinstallCnt++;
}

// 反馈入口桩:记录调用
static int ut_pdmp_feedbackCnt = 0;
static void ut_pdmp_feedback()
{
    ut_pdmp_feedbackCnt++;
}

static DriverInfo *ut_pdmp_makeInfo(const QString &name, const QString &packages,
                                    const QString &debVersion, const QString &version,
                                    Status status)
{
    DriverInfo *info = new DriverInfo();
    info->m_Name = name;
    info->m_Packages = packages;
    info->m_DebVersion = debVersion;
    info->m_Version = version;
    info->m_Size = "23.2";
    info->m_Byte = 1048576;
    info->m_Status = status;
    return info;
}

class UT_PageDriverManager_Progress : public UT_HEAD
{
public:
    void SetUp()
    {
        ut_pdmp_backupPath.clear();
        ut_pdmp_reinstallCnt = 0;
        ut_pdmp_feedbackCnt = 0;
        stub.set(ADDR(CommonTools, getBackupPath), ut_pdmp_getBackupPath);
        m_dPageDriverManager = new PageDriverManager;
    }
    void TearDown()
    {
        // 线程可能被测试换绑为 MOCK;先等自然退出,避免 PDM 析构 terminate 留下脏线程状态
        m_dPageDriverManager->mp_BackupThread->wait(2000);
        m_dPageDriverManager->mp_scanner->wait(2000);
        delete m_dPageDriverManager;
    }
    PageDriverManager *m_dPageDriverManager;
    Stub stub;
};

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotInstallProgressChanged_noCur)
{
    // 无当前驱动:直接返回,不崩溃
    m_dPageDriverManager->slotInstallProgressChanged(30);
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurDriverInfo);
    SUCCEED();
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotInstallProgressChanged_download)
{
    // 进度 <= 50 为下载阶段:进度不超过 45 时保持下载状态
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DOWNLOADING);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;

    m_dPageDriverManager->slotInstallProgressChanged(30);
    EXPECT_EQ(ST_DOWNLOADING, info->m_Status);

    // 45 < 进度 <= 50:下载即将完成,状态置为安装中
    m_dPageDriverManager->slotInstallProgressChanged(47);
    EXPECT_EQ(ST_INSTALL, info->m_Status);

    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->m_CurIndex = -1;
    delete info;
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotInstallProgressChanged_install)
{
    // 进度 > 50:安装阶段,状态置为安装中
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DOWNLOADING);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;

    m_dPageDriverManager->slotInstallProgressChanged(80);
    EXPECT_EQ(ST_INSTALL, info->m_Status);

    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->m_CurIndex = -1;
    delete info;
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotInstallProgressFinished_success)
{
    // 单项安装成功:状态置为成功,队列空则重置安装上下文
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_INSTALL);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;
    m_dPageDriverManager->m_ListUpdateIndex.append(0);

    m_dPageDriverManager->slotInstallProgressFinished(true, EC_NULL);

    EXPECT_EQ(ST_SUCESS, info->m_Status);
    EXPECT_TRUE(m_dPageDriverManager->m_ListDriverIndex.isEmpty());
    EXPECT_TRUE(m_dPageDriverManager->m_ListUpdateIndex.isEmpty());
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurDriverInfo);
    EXPECT_EQ(-1, m_dPageDriverManager->m_CurIndex);
    EXPECT_EQ(-1, m_dPageDriverManager->m_CancelIndex);
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotInstallProgressFinished_fail_generic)
{
    // 普通失败:状态置为失败
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_INSTALL);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;

    m_dPageDriverManager->slotInstallProgressFinished(false, EC_NOTFOUND);

    EXPECT_EQ(ST_FAILED, info->m_Status);
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurDriverInfo);
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotInstallProgressFinished_fail_networkNotify)
{
    // 网络异常通知:仅提示,不改变当前安装上下文(提前 return)
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_INSTALL);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;

    m_dPageDriverManager->slotInstallProgressFinished(false, EC_NOTIFY_NETWORK);

    EXPECT_EQ(ST_INSTALL, info->m_Status);
    EXPECT_EQ(info, m_dPageDriverManager->mp_CurDriverInfo);
    EXPECT_EQ(0, m_dPageDriverManager->m_CurIndex);

    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->m_CurIndex = -1;
    delete info;
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotInstallProgressFinished_fail_reinstall)
{
    // 需要重装:再次发起后台安装后直接返回
    stub.set((void (DBusDriverInterface::*)(const QString &, const QString &))ADDR(DBusDriverInterface, installDriver),
             ut_pdmp_installDriver2);
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_INSTALL);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;

    m_dPageDriverManager->slotInstallProgressFinished(false, EC_REINSTALL);

    EXPECT_EQ(1, ut_pdmp_reinstallCnt);
    EXPECT_EQ(info, m_dPageDriverManager->mp_CurDriverInfo);

    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->m_CurIndex = -1;
    delete info;
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotInstallProgressFinished_fail_network)
{
    // 网络错误:所有等待项全部置失败并清空队列
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_INSTALL);
    DriverInfo *info2 = ut_pdmp_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_NOT_INSTALL);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->addDriverInfo(info2);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;
    m_dPageDriverManager->m_ListDriverIndex.append(1);

    m_dPageDriverManager->slotInstallProgressFinished(false, EC_NETWORK);

    EXPECT_EQ(ST_FAILED, info->m_Status);
    EXPECT_TRUE(m_dPageDriverManager->m_ListDriverIndex.isEmpty());
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurDriverInfo);
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotInstallProgressFinished_noCur)
{
    // 无当前驱动:直接返回
    m_dPageDriverManager->slotInstallProgressFinished(true, EC_NULL);
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurDriverInfo);
    SUCCEED();
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotBackupProgressChanged)
{
    // 进度回调为空实现,调用不崩溃即可
    m_dPageDriverManager->slotBackupProgressChanged(42);
    SUCCEED();
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotRestoreProgress_noCur)
{
    // 无当前还原驱动:直接返回
    m_dPageDriverManager->slotRestoreProgress(50, "detail");
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurRestoreDriverInfo);
    SUCCEED();
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotRestoreProgress_doing)
{
    // 还原中:进度 < 100 展示还原进度
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DRIVER_CAN_RESTORE);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurRestoreDriverInfo = info;

    m_dPageDriverManager->slotRestoreProgress(50, "detail");
    EXPECT_EQ(info, m_dPageDriverManager->mp_CurRestoreDriverInfo);

    // 进度 >= 100 展示还原驱动完成界面
    m_dPageDriverManager->slotRestoreProgress(100, "detail");
    EXPECT_EQ(info, m_dPageDriverManager->mp_CurRestoreDriverInfo);

    m_dPageDriverManager->mp_CurRestoreDriverInfo = nullptr;
    delete info;
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotRestoreFinished_success)
{
    // 还原成功:当前项从可还原列表移除,表格刷新后保留剩余项
    QTemporaryDir tmpDir;
    ut_pdmp_backupPath = tmpDir.path();
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DRIVER_CAN_RESTORE);
    info->m_BackupFileName = tmpDir.filePath("gpu0_0.9.deb");
    DriverInfo *info2 = ut_pdmp_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_DRIVER_CAN_RESTORE);
    info2->m_BackupFileName = tmpDir.filePath("net0_1.0.deb");
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->addDriverInfo(info2);
    m_dPageDriverManager->m_ListRestorableIndex << 0 << 1;
    m_dPageDriverManager->mp_CurRestoreDriverInfo = info;

    m_dPageDriverManager->slotRestoreFinished(true, "");

    EXPECT_EQ(QList<int>() << 1, m_dPageDriverManager->m_ListRestorableIndex);
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurRestoreDriverInfo);
    EXPECT_EQ(0, ut_pdmp_feedbackCnt);
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotRestoreFinished_fail_okBtn)
{
    // 还原失败:弹窗确认(按钮 0),不触发反馈
    QTemporaryDir tmpDir;
    ut_pdmp_backupPath = tmpDir.path();
    stub.set(ADDR(CommonTools, feedback), ut_pdmp_feedback);
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DRIVER_CAN_RESTORE);
    info->m_BackupFileName = tmpDir.filePath("gpu0_0.9.deb");
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->m_ListRestorableIndex.append(0);
    m_dPageDriverManager->mp_CurRestoreDriverInfo = info;
    // 失败弹窗换绑 MOCK(exec 为虚函数,stub-c 无法打桩,见 ut_mock_thread.h)
    delete m_dPageDriverManager->mp_FailedDialog;
    MockFailedDialog *dlg = new MockFailedDialog(m_dPageDriverManager);
    dlg->m_execReturn = 0;
    m_dPageDriverManager->mp_FailedDialog = dlg;

    m_dPageDriverManager->slotRestoreFinished(false, "error");

    // 失败项仍保留在可还原列表
    EXPECT_EQ(QList<int>() << 0, m_dPageDriverManager->m_ListRestorableIndex);
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurRestoreDriverInfo);
    EXPECT_EQ(1, dlg->m_execCount);
    EXPECT_EQ(0, ut_pdmp_feedbackCnt);
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotRestoreFinished_fail_feedbackBtn)
{
    // 还原失败:弹窗点击反馈(按钮 1)触发反馈入口
    QTemporaryDir tmpDir;
    ut_pdmp_backupPath = tmpDir.path();
    stub.set(ADDR(CommonTools, feedback), ut_pdmp_feedback);
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DRIVER_CAN_RESTORE);
    info->m_BackupFileName = tmpDir.filePath("gpu0_0.9.deb");
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->m_ListRestorableIndex.append(0);
    m_dPageDriverManager->mp_CurRestoreDriverInfo = info;
    // 失败弹窗换绑 MOCK,返回 1 表示点击反馈
    delete m_dPageDriverManager->mp_FailedDialog;
    MockFailedDialog *dlg = new MockFailedDialog(m_dPageDriverManager);
    dlg->m_execReturn = 1;
    m_dPageDriverManager->mp_FailedDialog = dlg;

    m_dPageDriverManager->slotRestoreFinished(false, "error");

    EXPECT_EQ(1, dlg->m_execCount);
    EXPECT_EQ(1, ut_pdmp_feedbackCnt);
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurRestoreDriverInfo);
}

TEST_F(UT_PageDriverManager_Progress, UT_PageDriverManager_slotRestoreFinished_noCur)
{
    // 无当前还原驱动:仅复位重检按钮与表格,不崩溃
    DriverInfo *info = ut_pdmp_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DRIVER_CAN_RESTORE);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->m_ListRestorableIndex.append(0);

    m_dPageDriverManager->slotRestoreFinished(false, "error");

    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurRestoreDriverInfo);
    EXPECT_EQ(QList<int>() << 0, m_dPageDriverManager->m_ListRestorableIndex);
    delete info;
}
