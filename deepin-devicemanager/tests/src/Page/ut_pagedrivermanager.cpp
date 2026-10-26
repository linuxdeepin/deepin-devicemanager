// SPDX-FileCopyrightText: 2019-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QSignalSpy>
#include <QThread>
#include <QTest>

#include "ut_Head.h"
#include "stub.h"

#define private public
#include "PageDriverManager.h"
#include "PageListView.h"
#include "PageDriverInstallInfo.h"
#include "PageDriverBackupInfo.h"
#include "PageDriverRestoreInfo.h"
#include "DriverBackupThread.h"
#include "DriverScanner.h"
#include "DetectedStatusWidget.h"
#include "commontools.h"

// 备份路径指向临时目录,避免访问真实系统路径 /var/lib/deepin-devicemanager
static QString ut_pdm_backupPath;
static QString ut_pdm_getBackupPath()
{
    return ut_pdm_backupPath;
}

// 网络离线桩
static bool ut_pdm_netOffline()
{
    return false;
}

// QThread::start 计数桩:非虚函数可安全打桩,替换后不再产生真实线程与异步事件
static int ut_pdm_startCnt = 0;
static void ut_pdm_fakeStart()
{
    ut_pdm_startCnt++;
}

// 头部状态区 UI 桩:仅做展示刷新,不参与逻辑断言,避免重复布局项堆积
static void ut_pdm_setNoBackupUI(int, int)
{
}
static void ut_pdm_setBackableUI(int, int)
{
}
static void ut_pdm_setReDetectEnable(bool)
{
}
static void ut_pdm_setDownloadUI(const DriverType &, const QString &, const QString &,
                                 const QString &, int)
{
}
static void ut_pdm_setNoUpdateUI(const QString &)
{
}
static void ut_pdm_setDetectFinishUI(const QString &, const QString &, bool)
{
}
static void ut_pdm_setRestoreUI(int)
{
}

static DriverInfo *ut_pdm_makeInfo(const QString &name, const QString &packages,
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

class UT_PageDriverManager : public UT_HEAD
{
public:
    void SetUp()
    {
        ut_pdm_backupPath.clear();
        ut_pdm_startCnt = 0;
        // 备份路径统一指向临时目录,避免读写真实系统目录
        stub.set(ADDR(CommonTools, getBackupPath), ut_pdm_getBackupPath);
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

TEST_F(UT_PageDriverManager, UT_PageDriverManager_ctor)
{
    // 构造后:左侧列表类型为驱动安装,堆栈页含 3 个页面,当前显示第一页
    EXPECT_STREQ("Driver Install", m_dPageDriverManager->mp_ListView->currentType().toStdString().c_str());
    ASSERT_EQ(3, m_dPageDriverManager->mp_StackWidget->count());
    EXPECT_EQ(0, m_dPageDriverManager->mp_StackWidget->currentIndex());
    // 初始状态:无任何进行中的操作
    EXPECT_EQ(-1, m_dPageDriverManager->m_CurIndex);
    EXPECT_EQ(-1, m_dPageDriverManager->m_CurBackupIndex);
    EXPECT_EQ(-1, m_dPageDriverManager->m_CancelIndex);
    EXPECT_FALSE(m_dPageDriverManager->isInstalling());
    EXPECT_FALSE(m_dPageDriverManager->isBackingup());
    EXPECT_FALSE(m_dPageDriverManager->isRestoring());
    EXPECT_FALSE(m_dPageDriverManager->isScanning());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_updateListView)
{
    QList<QPair<QString, QString> > list;
    list.append(QPair<QString, QString>("Driver Install", "driverinstall##Overview"));
    list.append(QPair<QString, QString>("Driver Backup", "driverbackup##Overview"));
    list.append(QPair<QString, QString>("Driver Restore", "driverrestore##Overview"));
    m_dPageDriverManager->updateListView(list);
    EXPECT_STREQ("Driver Install", m_dPageDriverManager->mp_ListView->currentType().toStdString().c_str());

    // 点击备份项,右侧堆栈切换到备份页
    m_dPageDriverManager->mp_ListView->setCurType("Driver Backup");
    m_dPageDriverManager->mp_ListView->itemClicked("Driver Backup");
    EXPECT_EQ(1, m_dPageDriverManager->mp_StackWidget->currentIndex());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_addDriverInfo)
{
    ASSERT_TRUE(m_dPageDriverManager->m_ListDriverInfo.isEmpty());
    DriverInfo *info = ut_pdm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info);
    ASSERT_EQ(1, m_dPageDriverManager->m_ListDriverInfo.size());
    EXPECT_EQ(info, m_dPageDriverManager->m_ListDriverInfo.first());
    EXPECT_STREQ("gpu0", info->name().toStdString().c_str());
    // addDriverInfo 仅追加不负责释放,测试自行清理避免 TearDown 二次析构
    m_dPageDriverManager->m_ListDriverInfo.clear();
    delete info;
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_isFirstScan_first)
{
    // 首次扫描标志为 true 时恒返回 true
    m_dPageDriverManager->m_IsFirstScan = true;
    m_dPageDriverManager->m_Scanning = true;
    EXPECT_TRUE(m_dPageDriverManager->isFirstScan());

    // 非首次且正在扫描 -> false(与网络状态无关)
    m_dPageDriverManager->m_IsFirstScan = false;
    stub.set(ADDR(PageDriverManager, networkIsOnline), ut_pdm_netOffline);
    EXPECT_FALSE(m_dPageDriverManager->isFirstScan());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_isFirstScan_offline)
{
    // 非首次、未扫描、离线 -> 视为首次扫描
    m_dPageDriverManager->m_IsFirstScan = false;
    m_dPageDriverManager->m_Scanning = false;
    stub.set(ADDR(PageDriverManager, networkIsOnline), ut_pdm_netOffline);
    EXPECT_TRUE(m_dPageDriverManager->isFirstScan());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_isStateFlags)
{
    DriverInfo *info = ut_pdm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    EXPECT_FALSE(m_dPageDriverManager->isInstalling());
    EXPECT_FALSE(m_dPageDriverManager->isBackingup());
    EXPECT_FALSE(m_dPageDriverManager->isRestoring());
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->mp_CurBackupDriverInfo = info;
    m_dPageDriverManager->mp_CurRestoreDriverInfo = info;
    m_dPageDriverManager->m_Scanning = true;
    EXPECT_TRUE(m_dPageDriverManager->isInstalling());
    EXPECT_TRUE(m_dPageDriverManager->isBackingup());
    EXPECT_TRUE(m_dPageDriverManager->isRestoring());
    EXPECT_TRUE(m_dPageDriverManager->isScanning());
    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->mp_CurBackupDriverInfo = nullptr;
    m_dPageDriverManager->mp_CurRestoreDriverInfo = nullptr;
    m_dPageDriverManager->m_Scanning = false;
    delete info;
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_addToDriverIndex)
{
    m_dPageDriverManager->addToDriverIndex(0);
    ASSERT_EQ(1, m_dPageDriverManager->m_ListDriverIndex.size());
    EXPECT_EQ(0, m_dPageDriverManager->m_ListDriverIndex.first());

    // 重复添加同一索引不生效
    m_dPageDriverManager->addToDriverIndex(0);
    EXPECT_EQ(1, m_dPageDriverManager->m_ListDriverIndex.size());

    m_dPageDriverManager->addToDriverIndex(2);
    ASSERT_EQ(2, m_dPageDriverManager->m_ListDriverIndex.size());
    EXPECT_EQ(2, m_dPageDriverManager->m_ListDriverIndex.last());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_removeFromDriverIndex)
{
    m_dPageDriverManager->addToDriverIndex(0);
    m_dPageDriverManager->addToDriverIndex(1);
    m_dPageDriverManager->removeFromDriverIndex(0);
    ASSERT_EQ(1, m_dPageDriverManager->m_ListDriverIndex.size());
    EXPECT_EQ(1, m_dPageDriverManager->m_ListDriverIndex.first());

    // 移除不存在的索引无副作用
    m_dPageDriverManager->removeFromDriverIndex(99);
    EXPECT_EQ(1, m_dPageDriverManager->m_ListDriverIndex.size());

    // 清空后一键安装按钮置灰(队列空)
    m_dPageDriverManager->removeFromDriverIndex(1);
    EXPECT_TRUE(m_dPageDriverManager->m_ListDriverIndex.isEmpty());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_addToBackupIndex)
{
    m_dPageDriverManager->addToBackupIndex(3);
    ASSERT_EQ(1, m_dPageDriverManager->m_ListBackupIndex.size());
    EXPECT_EQ(3, m_dPageDriverManager->m_ListBackupIndex.first());

    m_dPageDriverManager->addToBackupIndex(3);
    EXPECT_EQ(1, m_dPageDriverManager->m_ListBackupIndex.size());

    m_dPageDriverManager->addToBackupIndex(4);
    ASSERT_EQ(2, m_dPageDriverManager->m_ListBackupIndex.size());
    EXPECT_EQ(4, m_dPageDriverManager->m_ListBackupIndex.last());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_removeFromBackupIndex)
{
    m_dPageDriverManager->addToBackupIndex(3);
    m_dPageDriverManager->addToBackupIndex(4);
    m_dPageDriverManager->removeFromBackupIndex(3);
    ASSERT_EQ(1, m_dPageDriverManager->m_ListBackupIndex.size());
    EXPECT_EQ(4, m_dPageDriverManager->m_ListBackupIndex.first());

    m_dPageDriverManager->removeFromBackupIndex(99);
    EXPECT_EQ(1, m_dPageDriverManager->m_ListBackupIndex.size());

    m_dPageDriverManager->removeFromBackupIndex(4);
    EXPECT_TRUE(m_dPageDriverManager->m_ListBackupIndex.isEmpty());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_installNextDriver)
{
    // 待安装队列 [2],installNextDriver 取队首并调用后台安装(DBus 已被全局桩拦截)
    DriverInfo *info0 = ut_pdm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    DriverInfo *info1 = ut_pdm_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_NOT_INSTALL);
    DriverInfo *info2 = ut_pdm_makeInfo("snd0", "pkg-snd", "3.0", "2.0", ST_NOT_INSTALL);
    m_dPageDriverManager->addDriverInfo(info0);
    m_dPageDriverManager->addDriverInfo(info1);
    m_dPageDriverManager->addDriverInfo(info2);
    m_dPageDriverManager->m_ListDriverIndex.append(2);

    ASSERT_FALSE(m_dPageDriverManager->isInstalling());
    m_dPageDriverManager->installNextDriver();

    EXPECT_EQ(2, m_dPageDriverManager->m_CurIndex);
    EXPECT_EQ(info2, m_dPageDriverManager->mp_CurDriverInfo);
    EXPECT_TRUE(m_dPageDriverManager->m_ListDriverIndex.isEmpty());
    EXPECT_EQ(ST_DOWNLOADING, info2->m_Status);
    EXPECT_TRUE(m_dPageDriverManager->isInstalling());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_backupNextDriver)
{
    // 备份线程 start 打桩,避免真实备份动作
    QTemporaryDir tmpDir;
    ut_pdm_backupPath = tmpDir.path();
    DriverInfo *info0 = ut_pdm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    DriverInfo *info1 = ut_pdm_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info0);
    m_dPageDriverManager->addDriverInfo(info1);
    m_dPageDriverManager->m_ListBackupIndex.append(1);

    stub.set(ADDR(DriverBackupThread, start), ut_pdm_fakeStart);

    ASSERT_FALSE(m_dPageDriverManager->isBackingup());
    m_dPageDriverManager->backupNextDriver();

    EXPECT_EQ(1, m_dPageDriverManager->m_CurBackupIndex);
    EXPECT_EQ(info1, m_dPageDriverManager->mp_CurBackupDriverInfo);
    EXPECT_TRUE(m_dPageDriverManager->m_ListBackupIndex.isEmpty());
    EXPECT_EQ(ST_DRIVER_BACKING_UP, info1->m_Status);
    EXPECT_TRUE(m_dPageDriverManager->isBackingup());
    // 备份线程 start 被调用且不再产生真实线程
    EXPECT_EQ(1, ut_pdm_startCnt);
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_clearAllData)
{
    DriverInfo *info = ut_pdm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->m_ListNewIndex.append(0);
    m_dPageDriverManager->m_ListDriverIndex.append(0);
    m_dPageDriverManager->m_ListUpdateIndex.append(0);
    m_dPageDriverManager->m_ListInstallIndex.append(0);
    m_dPageDriverManager->m_ListBackupIndex.append(0);
    m_dPageDriverManager->m_ListBackableIndex.append(0);
    m_dPageDriverManager->m_ListBackedupeIndex.append(0);
    m_dPageDriverManager->m_ListRestorableIndex.append(0);

    m_dPageDriverManager->clearAllData();

    EXPECT_TRUE(m_dPageDriverManager->m_ListNewIndex.isEmpty());
    EXPECT_TRUE(m_dPageDriverManager->m_ListDriverIndex.isEmpty());
    EXPECT_TRUE(m_dPageDriverManager->m_ListUpdateIndex.isEmpty());
    EXPECT_TRUE(m_dPageDriverManager->m_ListInstallIndex.isEmpty());
    EXPECT_TRUE(m_dPageDriverManager->m_ListBackupIndex.isEmpty());
    EXPECT_TRUE(m_dPageDriverManager->m_ListBackableIndex.isEmpty());
    EXPECT_TRUE(m_dPageDriverManager->m_ListBackedupeIndex.isEmpty());
    EXPECT_TRUE(m_dPageDriverManager->m_ListRestorableIndex.isEmpty());
    EXPECT_TRUE(m_dPageDriverManager->m_ListDriverInfo.isEmpty());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_slotScanFinished_success)
{
    QTemporaryDir tmpDir;
    ut_pdm_backupPath = tmpDir.path();
    // info0:未安装,无 deb 包 -> 仅进入安装列表
    // info1:可更新,有 deb 包,无备份 -> 安装 + 可备份
    // info2:已最新,有 deb 包且有备份(版本与当前不同) -> 各备份/还原列表
    DriverInfo *info0 = ut_pdm_makeInfo("gpu0", "pkg-gpu", "", "0.9", ST_NOT_INSTALL);
    DriverInfo *info1 = ut_pdm_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_CAN_UPDATE);
    DriverInfo *info2 = ut_pdm_makeInfo("snd0", "pkg-snd", "3.0", "2.0", ST_DRIVER_IS_NEW);
    info2->m_DebBackupVersion = "2.5";
    m_dPageDriverManager->addDriverInfo(info0);
    m_dPageDriverManager->addDriverInfo(info1);
    m_dPageDriverManager->addDriverInfo(info2);
    m_dPageDriverManager->m_Scanning = true;

    QSignalSpy spy(m_dPageDriverManager, &PageDriverManager::scanFinished);
    // 头部状态区 UI 打桩:仅展示刷新,不参与本用例断言
    stub.set(ADDR(DetectedStatusWidget, setNoUpdateDriverUI), ut_pdm_setNoUpdateUI);
    stub.set(ADDR(DetectedStatusWidget, setDetectFinishUI), ut_pdm_setDetectFinishUI);
    stub.set(ADDR(DetectedStatusWidget, setNoBackupDriverUI), ut_pdm_setNoBackupUI);
    stub.set(ADDR(DetectedStatusWidget, setBackableDriverUI), ut_pdm_setBackableUI);
    stub.set(ADDR(DetectedStatusWidget, setRestoreDriverUI), ut_pdm_setRestoreUI);
    stub.set(ADDR(DetectedStatusWidget, setReDetectEnable), ut_pdm_setReDetectEnable);
    m_dPageDriverManager->slotScanFinished(SR_SUCESS);

    // 状态归类
    EXPECT_EQ(QList<int>() << 0, m_dPageDriverManager->m_ListInstallIndex);
    EXPECT_EQ(QList<int>() << 1, m_dPageDriverManager->m_ListUpdateIndex);
    EXPECT_EQ(QList<int>() << 2, m_dPageDriverManager->m_ListNewIndex);
    // info1 与 info2 均有 deb 包且备份版本 != deb 版本 -> 可备份
    EXPECT_EQ(QList<int>() << 1 << 2, m_dPageDriverManager->m_ListBackableIndex);
    // 仅 info2 有备份文件
    EXPECT_EQ(QList<int>() << 2, m_dPageDriverManager->m_ListBackedupeIndex);
    // 仅 info2 备份版本非空且 != 当前版本 -> 可还原
    EXPECT_EQ(QList<int>() << 2, m_dPageDriverManager->m_ListRestorableIndex);
    // 扫描结束标志复位并发送信号
    EXPECT_FALSE(m_dPageDriverManager->isScanning());
    ASSERT_EQ(1, spy.count());
    EXPECT_EQ(SR_SUCESS, spy.first().first().value<ScanResult>());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_slotScanFinished_netErr)
{
    m_dPageDriverManager->m_Scanning = true;
    DriverInfo *info0 = ut_pdm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info0);

    QSignalSpy spy(m_dPageDriverManager, &PageDriverManager::scanFinished);
    // 头部状态区 UI 打桩:仅展示刷新,不参与本用例断言
    stub.set(ADDR(DetectedStatusWidget, setNoUpdateDriverUI), ut_pdm_setNoUpdateUI);
    stub.set(ADDR(DetectedStatusWidget, setDetectFinishUI), ut_pdm_setDetectFinishUI);
    stub.set(ADDR(DetectedStatusWidget, setNoBackupDriverUI), ut_pdm_setNoBackupUI);
    stub.set(ADDR(DetectedStatusWidget, setBackableDriverUI), ut_pdm_setBackableUI);
    stub.set(ADDR(DetectedStatusWidget, setRestoreDriverUI), ut_pdm_setRestoreUI);
    stub.set(ADDR(DetectedStatusWidget, setReDetectEnable), ut_pdm_setReDetectEnable);
    m_dPageDriverManager->slotScanFinished(SR_NETWORD_ERR);

    // 网络错误:不归入任何列表,仅复位扫描状态并发送信号
    EXPECT_TRUE(m_dPageDriverManager->m_ListInstallIndex.isEmpty());
    EXPECT_TRUE(m_dPageDriverManager->m_ListUpdateIndex.isEmpty());
    EXPECT_FALSE(m_dPageDriverManager->isScanning());
    ASSERT_EQ(1, spy.count());
    EXPECT_EQ(SR_NETWORD_ERR, spy.first().first().value<ScanResult>());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_slotScanFinished_failed)
{
    m_dPageDriverManager->m_Scanning = true;
    DriverInfo *info0 = ut_pdm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info0);

    QSignalSpy spy(m_dPageDriverManager, &PageDriverManager::scanFinished);
    m_dPageDriverManager->slotScanFinished(SR_Failed);

    EXPECT_TRUE(m_dPageDriverManager->m_ListInstallIndex.isEmpty());
    EXPECT_FALSE(m_dPageDriverManager->isScanning());
    ASSERT_EQ(1, spy.count());
    EXPECT_EQ(SR_Failed, spy.first().first().value<ScanResult>());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_slotBackupFinished_success)
{
    QTemporaryDir tmpDir;
    ut_pdm_backupPath = tmpDir.path();
    DriverInfo *info0 = ut_pdm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    DriverInfo *info1 = ut_pdm_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info0);
    m_dPageDriverManager->addDriverInfo(info1);
    // 通过真实 backupNextDriver 建立当前备份上下文(start 已打桩);头部状态区 UI 一并打桩
    stub.set(ADDR(DetectedStatusWidget, setNoBackupDriverUI), ut_pdm_setNoBackupUI);
    stub.set(ADDR(DetectedStatusWidget, setBackableDriverUI), ut_pdm_setBackableUI);
    stub.set(ADDR(DetectedStatusWidget, setReDetectEnable), ut_pdm_setReDetectEnable);
    stub.set(ADDR(DriverBackupThread, start), ut_pdm_fakeStart);
    m_dPageDriverManager->m_ListBackupIndex.append(1);
    m_dPageDriverManager->backupNextDriver();
    ASSERT_EQ(info1, m_dPageDriverManager->mp_CurBackupDriverInfo);
    ASSERT_EQ(1, m_dPageDriverManager->m_CurBackupIndex);
    m_dPageDriverManager->m_ListBackableIndex.append(1);

    m_dPageDriverManager->slotBackupFinished(true);

    // 备份成功:当前项进入已备份列表,且备份版本("") != 当前版本 -> 进入可还原列表
    EXPECT_EQ(QList<int>() << 1, m_dPageDriverManager->m_ListBackedupeIndex);
    EXPECT_EQ(QList<int>() << 1, m_dPageDriverManager->m_ListRestorableIndex);
    EXPECT_TRUE(m_dPageDriverManager->m_ListBackableIndex.isEmpty());
    EXPECT_EQ(ST_DRIVER_BACKUP_SUCCESS, info1->m_Status);
    // 备份队列已空:重置全部备份状态
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurBackupDriverInfo);
    EXPECT_EQ(-1, m_dPageDriverManager->m_CurBackupIndex);
    EXPECT_EQ(0, m_dPageDriverManager->m_backupSuccessNum);
    EXPECT_EQ(0, m_dPageDriverManager->m_backupFailedNum);
    EXPECT_FALSE(m_dPageDriverManager->isBackingup());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_slotBackupFinished_fail)
{
    QTemporaryDir tmpDir;
    ut_pdm_backupPath = tmpDir.path();
    DriverInfo *info0 = ut_pdm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info0);
    stub.set(ADDR(DetectedStatusWidget, setNoBackupDriverUI), ut_pdm_setNoBackupUI);
    stub.set(ADDR(DetectedStatusWidget, setBackableDriverUI), ut_pdm_setBackableUI);
    stub.set(ADDR(DetectedStatusWidget, setReDetectEnable), ut_pdm_setReDetectEnable);
    stub.set(ADDR(DriverBackupThread, start), ut_pdm_fakeStart);
    m_dPageDriverManager->m_ListBackupIndex.append(0);
    m_dPageDriverManager->backupNextDriver();
    ASSERT_EQ(info0, m_dPageDriverManager->mp_CurBackupDriverInfo);

    m_dPageDriverManager->slotBackupFinished(false);

    // 备份失败:计数与状态更新,队列已空则重置状态
    EXPECT_EQ(ST_DRIVER_BACKUP_FAILED, info0->m_Status);
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurBackupDriverInfo);
    EXPECT_EQ(-1, m_dPageDriverManager->m_CurBackupIndex);
    EXPECT_TRUE(m_dPageDriverManager->m_ListBackedupeIndex.isEmpty());
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_slotBackupFinished_success_thenNext)
{
    QTemporaryDir tmpDir;
    ut_pdm_backupPath = tmpDir.path();
    DriverInfo *info0 = ut_pdm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    DriverInfo *info1 = ut_pdm_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info0);
    m_dPageDriverManager->addDriverInfo(info1);
    m_dPageDriverManager->m_ListBackupIndex.append(0);
    m_dPageDriverManager->m_ListBackupIndex.append(1);
    m_dPageDriverManager->m_ListBackableIndex << 0 << 1;

    // 备份队列还有一项:成功后应继续下一项而不是重置状态;头部状态区 UI 一并打桩
    stub.set(ADDR(DetectedStatusWidget, setNoBackupDriverUI), ut_pdm_setNoBackupUI);
    stub.set(ADDR(DetectedStatusWidget, setBackableDriverUI), ut_pdm_setBackableUI);
    stub.set(ADDR(DetectedStatusWidget, setReDetectEnable), ut_pdm_setReDetectEnable);
    stub.set(ADDR(DriverBackupThread, start), ut_pdm_fakeStart);
    m_dPageDriverManager->backupNextDriver();
    ASSERT_EQ(info0, m_dPageDriverManager->mp_CurBackupDriverInfo);
    m_dPageDriverManager->slotBackupFinished(true);
    EXPECT_EQ(ST_DRIVER_BACKUP_SUCCESS, info0->m_Status);
    // 队列剩余项被消费,当前备份对象切换为下一项,状态不重置
    EXPECT_EQ(info1, m_dPageDriverManager->mp_CurBackupDriverInfo);
    EXPECT_EQ(1, m_dPageDriverManager->m_CurBackupIndex);
    EXPECT_NE(nullptr, m_dPageDriverManager->mp_CurBackupDriverInfo);
}

TEST_F(UT_PageDriverManager, UT_PageDriverManager_slotBackupFinished_noCur)
{
    // 无当前备份驱动时直接返回,不崩溃、不改状态
    m_dPageDriverManager->slotBackupFinished(true);
    EXPECT_EQ(nullptr, m_dPageDriverManager->mp_CurBackupDriverInfo);
    EXPECT_TRUE(m_dPageDriverManager->m_ListBackedupeIndex.isEmpty());
    SUCCEED();
}
