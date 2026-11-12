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

static QString ut_pdmo_backupPath;
// 撤销安装 DBus 调用桩:避免真实 system bus 同步调用(setTimeout 1000s)阻塞
static void ut_pdmo_fakeUndo()
{
}

static QString ut_pdmo_getBackupPath()
{
    return ut_pdmo_backupPath;
}

// 安装/备份推进桩:仅记录调用次数
static int ut_pdmo_installNextCnt = 0;
static void ut_pdmo_installNext()
{
    ut_pdmo_installNextCnt++;
}
static int ut_pdmo_backupNextCnt = 0;
static void ut_pdmo_backupNext()
{
    ut_pdmo_backupNextCnt++;
}

// 备份线程空跑桩
static void ut_pdmo_threadRun()
{
}

static DriverInfo *ut_pdmo_makeInfo(const QString &name, const QString &packages,
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

class UT_PageDriverManager_Ops : public UT_HEAD
{
public:
    void SetUp()
    {
        ut_pdmo_backupPath.clear();
        ut_pdmo_installNextCnt = 0;
        ut_pdmo_backupNextCnt = 0;
        stub.set(ADDR(CommonTools, getBackupPath), ut_pdmo_getBackupPath);
        // 撤销安装走 DBusDriverInterface::undoInstallDriver → mp_Iface->call("method")
        // 单参内联重载,真实 system bus 同步调用会阻塞 1000s(setTimeout),打桩跳过
        stub.set(ADDR(DBusDriverInterface, undoInstallDriver), ut_pdmo_fakeUndo);
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

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotDriverOperationClicked_install)
{
    // INSTALL:首次点击 -> 入队并推进安装
    stub.set(ADDR(PageDriverManager, installNextDriver), ut_pdmo_installNext);
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_NOT_INSTALL);
    m_dPageDriverManager->addDriverInfo(info);
    // 安装前三个页面的重检按钮先置灰
    m_dPageDriverManager->slotDriverOperationClicked(0, 0, DriverOperationItem::INSTALL);
    EXPECT_EQ(QList<int>() << 0, m_dPageDriverManager->m_ListDriverIndex);
    EXPECT_EQ(1, ut_pdmo_installNextCnt);
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotDriverOperationClicked_install_inProgress)
{
    // 安装过程中再次点击 INSTALL:只入队等待,不推进
    stub.set(ADDR(PageDriverManager, installNextDriver), ut_pdmo_installNext);
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DOWNLOADING);
    DriverInfo *info2 = ut_pdmo_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_NOT_INSTALL);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->addDriverInfo(info2);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;

    m_dPageDriverManager->slotDriverOperationClicked(1, 0, DriverOperationItem::INSTALL);
    EXPECT_EQ(QList<int>() << 1, m_dPageDriverManager->m_ListDriverIndex);
    EXPECT_EQ(0, ut_pdmo_installNextCnt);

    // 清理上下文,避免影响用例内断言
    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->m_CurIndex = -1;
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotDriverOperationClicked_update)
{
    // UPDATE 与 INSTALL 同一分支
    stub.set(ADDR(PageDriverManager, installNextDriver), ut_pdmo_installNext);
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info);

    m_dPageDriverManager->slotDriverOperationClicked(0, 0, DriverOperationItem::UPDATE);
    EXPECT_EQ(QList<int>() << 0, m_dPageDriverManager->m_ListDriverIndex);
    EXPECT_EQ(1, ut_pdmo_installNextCnt);
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotDriverOperationClicked_backup)
{
    // BACKUP:首次点击 -> 入队并推进备份
    stub.set(ADDR(PageDriverManager, backupNextDriver), ut_pdmo_backupNext);
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info);

    m_dPageDriverManager->slotDriverOperationClicked(0, 0, DriverOperationItem::BACKUP);
    EXPECT_EQ(QList<int>() << 0, m_dPageDriverManager->m_ListBackupIndex);
    EXPECT_EQ(1, ut_pdmo_backupNextCnt);
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotDriverOperationClicked_backup_inProgress)
{
    // 备份过程中再次点击 BACKUP:只入队等待
    stub.set(ADDR(PageDriverManager, backupNextDriver), ut_pdmo_backupNext);
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DRIVER_BACKING_UP);
    DriverInfo *info2 = ut_pdmo_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->addDriverInfo(info2);
    m_dPageDriverManager->mp_CurBackupDriverInfo = info;
    m_dPageDriverManager->m_CurBackupIndex = 0;

    m_dPageDriverManager->slotDriverOperationClicked(1, 0, DriverOperationItem::BACKUP);
    EXPECT_EQ(QList<int>() << 1, m_dPageDriverManager->m_ListBackupIndex);
    EXPECT_EQ(0, ut_pdmo_backupNextCnt);

    m_dPageDriverManager->mp_CurBackupDriverInfo = nullptr;
    m_dPageDriverManager->m_CurBackupIndex = -1;
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotDriverOperationClicked_restore)
{
    // RESTORE:设置当前还原对象并发起后台还原,其它可还原项置灰
    QTemporaryDir tmpDir;
    ut_pdmo_backupPath = tmpDir.path();
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    info->m_BackupFileName = tmpDir.filePath("gpu0_0.9.deb");
    DriverInfo *info2 = ut_pdmo_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_CAN_UPDATE);
    info2->m_BackupFileName = tmpDir.filePath("net0_1.0.deb");
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->addDriverInfo(info2);
    m_dPageDriverManager->m_ListRestorableIndex << 0 << 1;

    m_dPageDriverManager->slotDriverOperationClicked(0, 0, DriverOperationItem::RESTORE);

    EXPECT_EQ(info, m_dPageDriverManager->mp_CurRestoreDriverInfo);
    EXPECT_TRUE(m_dPageDriverManager->isRestoring());
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotInstallAllDrivers)
{
    stub.set(ADDR(PageDriverManager, installNextDriver), ut_pdmo_installNext);
    m_dPageDriverManager->slotInstallAllDrivers();
    EXPECT_EQ(1, ut_pdmo_installNextCnt);
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotBackupAllDrivers)
{
    stub.set(ADDR(PageDriverManager, backupNextDriver), ut_pdmo_backupNext);
    m_dPageDriverManager->slotBackupAllDrivers();
    EXPECT_EQ(1, ut_pdmo_backupNextCnt);
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotUndoInstall_noCurrent)
{
    // 无正在安装项:不调用后台撤销,仅恢复等待项状态并清空队列
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_NOT_INSTALL);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->m_ListDriverIndex.append(0);

    m_dPageDriverManager->slotUndoInstall();

    EXPECT_TRUE(m_dPageDriverManager->m_ListDriverIndex.isEmpty());
    // m_CancelIndex == m_CurIndex(-1)时未发起撤销
    EXPECT_EQ(-1, m_dPageDriverManager->m_CancelIndex);
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotUndoInstall_withCurrent)
{
    // 有正在安装项(m_CancelIndex != m_CurIndex):调用后台撤销并记录取消索引
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DOWNLOADING);
    DriverInfo *info2 = ut_pdmo_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_NOT_INSTALL);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->addDriverInfo(info2);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;
    m_dPageDriverManager->m_CancelIndex = -1;
    m_dPageDriverManager->m_ListDriverIndex.append(1);

    m_dPageDriverManager->slotUndoInstall();

    EXPECT_EQ(0, m_dPageDriverManager->m_CancelIndex);
    EXPECT_TRUE(m_dPageDriverManager->m_ListDriverIndex.isEmpty());

    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->m_CurIndex = -1;
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotUndoBackup)
{
    // 撤销备份:等待项状态复位,备份队列清空
    stub.set(ADDR(DriverBackupThread, undoBackup), ut_pdmo_threadRun);
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DRIVER_BACKING_UP);
    DriverInfo *info2 = ut_pdmo_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_DRIVER_NOT_BACKUP);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->addDriverInfo(info2);
    m_dPageDriverManager->m_ListBackupIndex.append(1);

    m_dPageDriverManager->slotUndoBackup();

    EXPECT_TRUE(m_dPageDriverManager->m_ListBackupIndex.isEmpty());
    EXPECT_EQ(ST_DRIVER_BACKING_UP, info->m_Status);
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotItemCheckedClicked)
{
    // 勾选 -> 入队;取消 -> 出队
    m_dPageDriverManager->slotItemCheckedClicked(0, true);
    EXPECT_EQ(QList<int>() << 0, m_dPageDriverManager->m_ListDriverIndex);
    m_dPageDriverManager->slotItemCheckedClicked(1, true);
    EXPECT_EQ(QList<int>() << 0 << 1, m_dPageDriverManager->m_ListDriverIndex);
    m_dPageDriverManager->slotItemCheckedClicked(0, false);
    EXPECT_EQ(QList<int>() << 1, m_dPageDriverManager->m_ListDriverIndex);
    m_dPageDriverManager->slotItemCheckedClicked(1, false);
    EXPECT_TRUE(m_dPageDriverManager->m_ListDriverIndex.isEmpty());
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotItemCheckedClicked_inProgress)
{
    // 安装过程中勾选:入队且全选框置灰(无空闲表格项,不崩溃即通过)
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DOWNLOADING);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;

    m_dPageDriverManager->slotItemCheckedClicked(1, true);
    EXPECT_EQ(QList<int>() << 1, m_dPageDriverManager->m_ListDriverIndex);

    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->m_CurIndex = -1;
    delete info;
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotBackupItemCheckedClicked)
{
    m_dPageDriverManager->slotBackupItemCheckedClicked(0, true);
    EXPECT_EQ(QList<int>() << 0, m_dPageDriverManager->m_ListBackupIndex);
    m_dPageDriverManager->slotBackupItemCheckedClicked(1, true);
    EXPECT_EQ(QList<int>() << 0 << 1, m_dPageDriverManager->m_ListBackupIndex);
    m_dPageDriverManager->slotBackupItemCheckedClicked(0, false);
    EXPECT_EQ(QList<int>() << 1, m_dPageDriverManager->m_ListBackupIndex);
    m_dPageDriverManager->slotBackupItemCheckedClicked(1, false);
    EXPECT_TRUE(m_dPageDriverManager->m_ListBackupIndex.isEmpty());
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotListViewWidgetItemClicked)
{
    // 左侧列表点击切换右侧页面
    m_dPageDriverManager->slotListViewWidgetItemClicked("Driver Backup");
    EXPECT_EQ(1, m_dPageDriverManager->mp_StackWidget->currentIndex());
    m_dPageDriverManager->slotListViewWidgetItemClicked("Driver Restore");
    EXPECT_EQ(2, m_dPageDriverManager->mp_StackWidget->currentIndex());
    m_dPageDriverManager->slotListViewWidgetItemClicked("Driver Install");
    EXPECT_EQ(0, m_dPageDriverManager->mp_StackWidget->currentIndex());
    // 未知项:索引不变
    m_dPageDriverManager->slotListViewWidgetItemClicked("Unknown");
    EXPECT_EQ(0, m_dPageDriverManager->mp_StackWidget->currentIndex());
    EXPECT_STREQ("Unknown", m_dPageDriverManager->m_CurItemStr.toStdString().c_str());
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotDownloadFinished)
{
    // 无当前驱动:直接返回
    m_dPageDriverManager->slotDownloadFinished();
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurDriverInfo);

    // 有当前驱动:更新安装页下载完成状态
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DOWNLOADING);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;
    m_dPageDriverManager->slotDownloadFinished();
    EXPECT_EQ(info, m_dPageDriverManager->mp_CurDriverInfo);

    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->m_CurIndex = -1;
    delete info;
}

TEST_F(UT_PageDriverManager_Ops, UT_PageDriverManager_slotDownloadProgressChanged)
{
    // 无当前驱动:直接返回
    m_dPageDriverManager->slotDownloadProgressChanged(QStringList() << "30" << "1.5MB" << "512KB");
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurDriverInfo);

    // 有当前驱动:把下载进度刷新到安装页表头
    DriverInfo *info = ut_pdmo_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DOWNLOADING);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;
    m_dPageDriverManager->slotDownloadProgressChanged(QStringList() << "30" << "1.5MB" << "512KB");
    EXPECT_EQ(info, m_dPageDriverManager->mp_CurDriverInfo);

    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->m_CurIndex = -1;
    delete info;
}
