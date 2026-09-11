// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include "drivercontrol/drivermanager.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

// 沙箱加固（PMS: BUG-376053）：installDriver(路径版) 仅接受守护进程备份目录
// /var/lib/deepin-devicemanager/driver 下的路径（还原流程），
// 其它路径（含前端 /home、/tmp 下的文件）一律拒绝
class UT_DriverManagerSandbox : public UT_HEAD
{
public:
    void SetUp() override
    {
        ASSERT_TRUE(QDir().mkpath(uniqueWorkDir()));
    }

    static QString uniqueWorkDir()
    {
        static QTemporaryDir dir(QDir::tempPath() + "/ut_drivermgr_" + QString::number(::getpid()) + "_XXXXXX");
        return dir.path();
    }
};

TEST_F(UT_DriverManagerSandbox, UT_DriverManagerSandbox_installDriver_RejectForeignPath)
{
    // 在守护进程备份目录之外放置一个真实存在的文件（避免命中“文件不存在”分支，
    // 确保走到沙箱路径守卫）
    const QString foreignPath = uniqueWorkDir() + "/fake_driver.deb";
    {
        QFile f(foreignPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("not-a-real-deb");
    }

    DriverManager manager;
    int finishedCount = 0;
    bool lastSuccess = true;
    QObject::connect(&manager, &DriverManager::sigFinished, [&](bool bsuccess, QString) {
        ++finishedCount;
        lastSuccess = bsuccess;
    });

    EXPECT_FALSE(manager.installDriver(foreignPath));
    EXPECT_EQ(finishedCount, 1);
    EXPECT_FALSE(lastSuccess);
}

TEST_F(UT_DriverManagerSandbox, UT_DriverManagerSandbox_installDriver_RejectNonexistentPath)
{
    DriverManager manager;
    int finishedCount = 0;
    QObject::connect(&manager, &DriverManager::sigFinished, [&](bool, QString) { ++finishedCount; });

    EXPECT_FALSE(manager.installDriver(uniqueWorkDir() + "/no_such_file.deb"));
    EXPECT_EQ(finishedCount, 1);
}

TEST_F(UT_DriverManagerSandbox, UT_DriverManagerSandbox_backupDebFd_RejectBadDebname)
{
    DriverManager manager;

    // debname 仅作备份目录名，白名单外的名字直接拒绝（不触发任何磁盘操作）
    EXPECT_FALSE(manager.backupDebFd(QDBusUnixFileDescriptor(), "a/b"));
    EXPECT_FALSE(manager.backupDebFd(QDBusUnixFileDescriptor(), ".."));
    EXPECT_FALSE(manager.backupDebFd(QDBusUnixFileDescriptor(), QString()));

    // 无效 fd：名字合法也拒绝
    EXPECT_FALSE(manager.backupDebFd(QDBusUnixFileDescriptor(), "nvidia-535"));
}
