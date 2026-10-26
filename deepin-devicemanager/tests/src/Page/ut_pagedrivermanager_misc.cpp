// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QtTest>

#include "ut_Head.h"
#include "stub.h"

#define private public
#include "PageDriverManager.h"
#include "DriverBackupThread.h"
#include "DriverScanner.h"
#include "commontools.h"
#include "DeviceManager.h"
#include <QNetworkInformation>

static QString ut_pdmm_backupPath;
static QString ut_pdmm_getBackupPath()
{
    return ut_pdmm_backupPath;
}

// 扫描设备桩:避免触发真实硬件遍历
static void ut_pdmm_scanDevices()
{
}

// 设备列表桩:返回空列表,让 scanDevices/scanDevicesInfo 走空设备分支
static bool ut_pdmm_fakeGetDeviceList(const QString &, QList<DeviceBaseInfo *> &)
{
    return false;
}

// QThread::start 计数桩:非虚函数可安全打桩,替换后不再产生真实线程与异步事件
static int ut_pdmm_startCnt = 0;
static void ut_pdmm_fakeStart()
{
    ut_pdmm_startCnt++;
}

// QNetworkInformation 单例/可达性桩:instance() 返回假指针,reachability() 返回受控状态
// (桩体不访问成员,假指针安全)
static QNetworkInformation::Reachability ut_pdmm_reach = QNetworkInformation::Reachability::Online;
static QNetworkInformation::Reachability ut_pdmm_reachability()
{
    return ut_pdmm_reach;
}
static QNetworkInformation *ut_pdmm_netInstance()
{
    return reinterpret_cast<QNetworkInformation *>(0x1);
}
static QNetworkInformation *ut_pdmm_nullInstance()
{
    return nullptr;
}

static DriverInfo *ut_pdmm_makeInfo(const QString &name, const QString &packages,
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

class UT_PageDriverManager_Misc : public UT_HEAD
{
public:
    void SetUp()
    {
        ut_pdmm_backupPath.clear();
        ut_pdmm_reach = QNetworkInformation::Reachability::Online;
        stub.set(ADDR(CommonTools, getBackupPath), ut_pdmm_getBackupPath);
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

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_getDebBackupInfo_noDir)
{
    // 备份目录不存在:原有备份版本保持不变
    QTemporaryDir tmpDir;
    ut_pdmm_backupPath = tmpDir.path();
    DriverInfo *info = ut_pdmm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    info->m_DebBackupVersion = "";

    m_dPageDriverManager->getDebBackupInfo(info);

    EXPECT_TRUE(info->debBackupVersion().isEmpty());
    EXPECT_TRUE(info->backupFileName().isEmpty());
    delete info;
}

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_getDebBackupInfo_found)
{
    // 备份目录存在且含同名 deb:解析出备份版本与备份文件路径
    QTemporaryDir tmpDir;
    QString pkgDir = tmpDir.path() + "/driver/pkg-gpu";
    ASSERT_TRUE(QDir().mkpath(pkgDir));
    QFile debFile(pkgDir + "/pkg-gpu_2.5.0.deb");
    ASSERT_TRUE(debFile.open(QIODevice::WriteOnly));
    debFile.close();
    ut_pdmm_backupPath = tmpDir.path();

    DriverInfo *info = ut_pdmm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    m_dPageDriverManager->getDebBackupInfo(info);

    EXPECT_STREQ("2.5.0", info->m_DebBackupVersion.toStdString().c_str());
    EXPECT_STREQ(debFile.fileName().toStdString().c_str(), info->m_BackupFileName.toStdString().c_str());
    delete info;
}

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_getDownloadInfo_kb)
{
    // 下载量小于 1MB:按 KB 展示
    QString speed, size;
    m_dPageDriverManager->getDownloadInfo(0, 512, speed, size);
    EXPECT_TRUE(size.contains("KB"));
    EXPECT_FALSE(speed.isEmpty());
}

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_getDownloadInfo_mb)
{
    // 下载量介于 1MB 与 1GB 之间:按 MB 展示
    qint64 twoMB = 2 * 1024 * 1024;
    QString speed, size;
    m_dPageDriverManager->getDownloadInfo(50, twoMB, speed, size);
    EXPECT_TRUE(size.contains("MB"));
    EXPECT_FALSE(speed.isEmpty());
}

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_failAllIndex)
{
    // 所有等待安装项统一置为失败状态(空列表与有数据列表均不崩溃)
    DriverInfo *info = ut_pdmm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DOWNLOADING);
    DriverInfo *info2 = ut_pdmm_makeInfo("net0", "pkg-net", "2.0", "1.0", ST_DOWNLOADING);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->addDriverInfo(info2);
    m_dPageDriverManager->m_ListDriverIndex.append(0);
    m_dPageDriverManager->m_ListDriverIndex.append(1);

    m_dPageDriverManager->failAllIndex();

    // failAllIndex 仅更新表格状态,不改列表内容与驱动状态
    EXPECT_EQ(QList<int>() << 0 << 1, m_dPageDriverManager->m_ListDriverIndex);
    EXPECT_EQ(ST_DOWNLOADING, info->m_Status);
}

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_networkIsOnline)
{
    // Qt6 分支:单例可达性受控,验证当前行为
    stub.set(ADDR(QNetworkInformation, instance), ut_pdmm_netInstance);
    stub.set((QNetworkInformation::Reachability (QNetworkInformation::*)() const)ADDR(QNetworkInformation, reachability),
             ut_pdmm_reachability);

    // 注意:当前实现为 reachability == Disconnected 时返回 true,逻辑疑似反转(Mode 5 缺陷候选)
    // 本用例固化当前行为:Online -> false
    ut_pdmm_reach = QNetworkInformation::Reachability::Online;
    EXPECT_FALSE(m_dPageDriverManager->networkIsOnline());

    // Disconnected -> true(同样为反转后的当前行为)
    ut_pdmm_reach = QNetworkInformation::Reachability::Disconnected;
    EXPECT_TRUE(m_dPageDriverManager->networkIsOnline());

    // Unknown: != Disconnected -> false
    ut_pdmm_reach = QNetworkInformation::Reachability::Unknown;
    EXPECT_FALSE(m_dPageDriverManager->networkIsOnline());
}

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_networkIsOffline_null)
{
    // 单例为空:返回 false
    stub.set(ADDR(QNetworkInformation, instance), ut_pdmm_nullInstance);
    EXPECT_FALSE(m_dPageDriverManager->networkIsOnline());
}

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_scanDevices_empty)
{
    // scanDevices 直扫:设备管理器返回空列表,不创建扫描线程,不修改状态
    stub.set(ADDR(DeviceManager, getDeviceList), ut_pdmm_fakeGetDeviceList);
    stub.set(ADDR(DriverScanner, start), ut_pdmm_fakeStart);
    ut_pdmm_startCnt = 0;
    m_dPageDriverManager->m_Scanning = false;

    m_dPageDriverManager->scanDevices();

    EXPECT_FALSE(m_dPageDriverManager->m_Scanning);
    EXPECT_EQ(0, ut_pdmm_startCnt);
}

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_scanDriverInfo_first)
{
    // 扫描设备桩 + 线程 start 桩:进入扫描状态且线程被启动
    stub.set(ADDR(PageDriverManager, scanDevices), ut_pdmm_scanDevices);
    stub.set(ADDR(DriverScanner, start), ut_pdmm_fakeStart);
    ut_pdmm_startCnt = 0;
    DriverInfo *info = ut_pdmm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_CAN_UPDATE);
    m_dPageDriverManager->addDriverInfo(info);

    m_dPageDriverManager->m_IsFirstScan = true;
    m_dPageDriverManager->m_Scanning = false;

    m_dPageDriverManager->scanDriverInfo();

    EXPECT_TRUE(m_dPageDriverManager->m_Scanning);
    EXPECT_FALSE(m_dPageDriverManager->m_IsFirstScan);
    // 扫描线程 start 被调用
    EXPECT_EQ(1, ut_pdmm_startCnt);
}

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_scanDriverInfo_busy)
{
    // 正在扫描时再次发起:直接返回,不重复启动线程
    stub.set(ADDR(PageDriverManager, scanDevices), ut_pdmm_scanDevices);
    stub.set(ADDR(DriverScanner, start), ut_pdmm_fakeStart);
    ut_pdmm_startCnt = 0;
    m_dPageDriverManager->m_IsFirstScan = false;
    m_dPageDriverManager->m_Scanning = true;

    m_dPageDriverManager->scanDriverInfo();

    EXPECT_TRUE(m_dPageDriverManager->m_Scanning);
    EXPECT_EQ(0, ut_pdmm_startCnt);
}

TEST_F(UT_PageDriverManager_Misc, UT_PageDriverManager_scanDriverInfo_inOperation)
{
    // 安装过程中发起扫描:置扫描标志后直接返回,不启动线程
    stub.set(ADDR(PageDriverManager, scanDevices), ut_pdmm_scanDevices);
    stub.set(ADDR(DriverScanner, start), ut_pdmm_fakeStart);
    ut_pdmm_startCnt = 0;
    DriverInfo *info = ut_pdmm_makeInfo("gpu0", "pkg-gpu", "1.0", "0.9", ST_DOWNLOADING);
    m_dPageDriverManager->addDriverInfo(info);
    m_dPageDriverManager->mp_CurDriverInfo = info;
    m_dPageDriverManager->m_CurIndex = 0;
    m_dPageDriverManager->m_IsFirstScan = false;
    m_dPageDriverManager->m_Scanning = false;

    m_dPageDriverManager->scanDriverInfo();

    EXPECT_TRUE(m_dPageDriverManager->m_Scanning);
    EXPECT_EQ(0, ut_pdmm_startCnt);

    m_dPageDriverManager->mp_CurDriverInfo = nullptr;
    m_dPageDriverManager->m_CurIndex = -1;
    delete info;
}
