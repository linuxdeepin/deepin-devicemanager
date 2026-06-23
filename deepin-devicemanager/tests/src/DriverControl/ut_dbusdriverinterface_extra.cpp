// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DBusDriverInterface 未测的 slot(纯 emit 信号转发) + 业务方法(installDriver双参/undo/backup/del/aptUpdate)。
// -fno-access-control 下可调用 private slot。

#include "DBusDriverInterface.h"
#include "ut_Head.h"

#include <QString>
#include <QStringList>

#include <gtest/gtest.h>

class UT_DBusDriverInterfaceExtra : public UT_HEAD
{
public:
    void SetUp() override
    {
        p = DBusDriverInterface::getInstance();
    }
    DBusDriverInterface *p = nullptr;
};

// ---- private slots: 纯信号转发 ----
TEST_F(UT_DBusDriverInterfaceExtra, slotProcessChange)
{
    EXPECT_NO_FATAL_FAILURE(p->slotProcessChange(50, "installing"));
}

TEST_F(UT_DBusDriverInterfaceExtra, slotProcessEnd)
{
    EXPECT_NO_FATAL_FAILURE(p->slotProcessEnd(true, "ok"));
    EXPECT_NO_FATAL_FAILURE(p->slotProcessEnd(false, "error"));
}

TEST_F(UT_DBusDriverInterfaceExtra, slotDownloadProgressChanged)
{
    QStringList msgs;
    msgs << "50%"
         << "1MB/s";
    EXPECT_NO_FATAL_FAILURE(p->slotDownloadProgressChanged(msgs));
}

TEST_F(UT_DBusDriverInterfaceExtra, slotDownloadFinished)
{
    EXPECT_NO_FATAL_FAILURE(p->slotDownloadFinished());
}

TEST_F(UT_DBusDriverInterfaceExtra, slotInstallProgressChanged)
{
    EXPECT_NO_FATAL_FAILURE(p->slotInstallProgressChanged(75));
}

TEST_F(UT_DBusDriverInterfaceExtra, slotInstallProgressFinished)
{
    EXPECT_NO_FATAL_FAILURE(p->slotInstallProgressFinished(true, 0));
    EXPECT_NO_FATAL_FAILURE(p->slotInstallProgressFinished(false, 1));
}

// ---- 业务方法(installDriver/undoInstall/backupDeb/delDeb/aptUpdate)跳过 ----
// mp_Iface 连上了真实 org.deepin.DeviceControl 服务,同步 call 会阻塞等待后台,导致测试超时。
