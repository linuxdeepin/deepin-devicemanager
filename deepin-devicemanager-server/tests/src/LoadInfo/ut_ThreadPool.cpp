// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "../stub.h"
#include "threadpool.h"
#include "threadpooltask.h"
#include "DDLog.h"

#include <QString>
#include <QList>
#include <QDir>

/*
 * Branch list for ThreadPool:
 * Cmd default ctor:
 *   B1: cmd == "" / file == "" / canNotReplace == false / waitingTime == -1
 * initCmd (m_ListCmd population):
 *   B2: lshw appended to both m_ListCmd and m_ListUpdate, canNotReplace=false
 *   B3: dmidecode_spn appended to m_ListCmd only, canNotReplace=true
 *   B4: lscpu appended to both, canNotReplace=true
 *   B5: bt_device waitingTime set to 500 (default is -1)
 *   B6: dr_config appended to m_ListCmd only (not m_ListUpdate)
 * constructor:
 *   B7: initCmd called → m_ListCmd/m_ListUpdate non-empty after construction
 *   B8: mkdir(PATH) called → device info directory exists after construction
 */

// Expose private members for white-box inspection (test binary built with -fno-access-control)
#define private public
#define protected public
#include "threadpool.h"
#undef private
#undef protected

class ThreadPool_UT : public UT_HEAD
{
public:
    ThreadPool *pool = nullptr;
    void SetUp() override
    {
        pool = new ThreadPool();
    }
    void TearDown() override
    {
        delete pool;
        pool = nullptr;
    }
};

// ===== Cmd struct =====

TEST_F(ThreadPool_UT, Cmd_DefaultConstructor_AllFieldsInitialized)
{
    // Arrange / Act
    Cmd cmd;
    // Assert
    EXPECT_TRUE(cmd.cmd.isEmpty());
    EXPECT_TRUE(cmd.file.isEmpty());
    EXPECT_FALSE(cmd.canNotReplace);
    EXPECT_EQ(cmd.waitingTime, -1);
}

TEST_F(ThreadPool_UT, Cmd_FieldsAssignable_RoundTripValues)
{
    // Arrange
    Cmd cmd;
    // Act
    cmd.cmd = "lscpu";
    cmd.file = "lscpu.txt";
    cmd.canNotReplace = true;
    cmd.waitingTime = 500;
    // Assert
    EXPECT_EQ(cmd.cmd, "lscpu");
    EXPECT_EQ(cmd.file, "lscpu.txt");
    EXPECT_TRUE(cmd.canNotReplace);
    EXPECT_EQ(cmd.waitingTime, 500);
}

// ===== initCmd — m_ListCmd membership =====

TEST_F(ThreadPool_UT, InitCmd_AfterConstruction_ListCmdNotEmpty)
{
    // Arrange / Act — constructor already called initCmd in SetUp
    // Assert
    ASSERT_NE(pool, nullptr);
    EXPECT_GT(pool->m_ListCmd.size(), 0);
    EXPECT_GT(pool->m_ListCmd.size(), 0);
}

TEST_F(ThreadPool_UT, InitCmd_AfterConstruction_ListUpdateNotEmpty)
{
    // Arrange / Act — constructor already called initCmd in SetUp
    // Assert
    ASSERT_NE(pool, nullptr);
    EXPECT_GT(pool->m_ListUpdate.size(), 0);
    EXPECT_GT(pool->m_ListCmd.size(), pool->m_ListUpdate.size());
}

TEST_F(ThreadPool_UT, InitCmd_AfterInit_ListCmdLargerThanListUpdate)
{
    // Arrange / Act — m_ListCmd contains dmidecode-only entries not in m_ListUpdate
    // Assert
    EXPECT_GT(pool->m_ListCmd.size(), pool->m_ListUpdate.size());
    EXPECT_GE(pool->m_ListCmd.size(), pool->m_ListUpdate.size());
}

TEST_F(ThreadPool_UT, InitCmd_LshwInBothLists_CanReplace)
{
    // Arrange / Act — find lshw entry in m_ListCmd
    bool foundInCmd = false;
    bool foundInUpdate = false;
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "lshw.txt") {
            foundInCmd = true;
            EXPECT_FALSE(c.canNotReplace);  // lshw canNotReplace = false
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "lshw.txt") {
            foundInUpdate = true;
        }
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_TRUE(foundInUpdate);
}

TEST_F(ThreadPool_UT, InitCmd_DmidecodeSpnInCmdOnly_CannotReplace)
{
    // Arrange / Act
    bool foundInCmd = false;
    bool foundInUpdate = false;
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "dmidecode_spn.txt") {
            foundInCmd = true;
            EXPECT_TRUE(c.canNotReplace);  // dmidecode_spn canNotReplace = true
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "dmidecode_spn.txt") {
            foundInUpdate = true;
        }
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_FALSE(foundInUpdate);
}

TEST_F(ThreadPool_UT, InitCmd_LscpuInBothLists_CannotReplace)
{
    // Arrange / Act
    bool foundInCmd = false;
    bool foundInUpdate = false;
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "lscpu.txt") {
            foundInCmd = true;
            EXPECT_TRUE(c.canNotReplace);  // lscpu canNotReplace = true
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "lscpu.txt") {
            foundInUpdate = true;
        }
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_TRUE(foundInUpdate);
}

TEST_F(ThreadPool_UT, InitCmd_BtDeviceEntry_HasCustomWaitingTime)
{
    // Arrange / Act — bt_device is the only entry with waitingTime = 500
    bool found = false;
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "bt_device.txt") {
            found = true;
            EXPECT_EQ(c.waitingTime, 500);
            EXPECT_FALSE(c.canNotReplace);
        }
    }
    // Assert
    EXPECT_TRUE(found);
}

TEST_F(ThreadPool_UT, InitCmd_NonBtDeviceEntries_HaveDefaultWaitingTime)
{
    // Arrange / Act — every entry except bt_device should have waitingTime == -1
    int checkedCount = 0;
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "bt_device.txt") {
            continue;
        }
        // Assert
        EXPECT_EQ(c.waitingTime, -1) << "Unexpected waitingTime for " << c.file.toStdString();
        checkedCount++;
    }
    // Assert
    EXPECT_GT(checkedCount, 0);
}

TEST_F(ThreadPool_UT, InitCmd_DrConfigEntry_InCmdListOnly)
{
    // Arrange / Act — dr_config appended to m_ListCmd only
    bool foundInCmd = false;
    bool foundInUpdate = false;
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "dr_config.txt") {
            foundInCmd = true;
            EXPECT_TRUE(c.canNotReplace);
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "dr_config.txt") {
            foundInUpdate = true;
        }
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_FALSE(foundInUpdate);
}

TEST_F(ThreadPool_UT, InitCmd_HwinfoInBothLists_CanReplace)
{
    // Arrange / Act
    bool foundInCmd = false;
    bool foundInUpdate = false;
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "hwinfo.txt") {
            foundInCmd = true;
            EXPECT_FALSE(c.canNotReplace);
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "hwinfo.txt") {
            foundInUpdate = true;
        }
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_TRUE(foundInUpdate);
}

TEST_F(ThreadPool_UT, InitCmd_HwinfoMonitorEntry_InBothLists)
{
    // Arrange / Act
    bool foundInCmd = false;
    bool foundInUpdate = false;
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "hwinfo_monitor.txt") {
            foundInCmd = true;
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "hwinfo_monitor.txt") {
            foundInUpdate = true;
        }
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_TRUE(foundInUpdate);
}

// ===== initCmd — command string content =====

TEST_F(ThreadPool_UT, InitCmd_LscpuCmdString_IsPlainCommand)
{
    // Arrange / Act — lscpu cmd is just "lscpu" (no redirect)
    bool found = false;
    QString cmdStr;
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "lscpu.txt") {
            found = true;
            cmdStr = c.cmd;
        }
    }
    // Assert
    EXPECT_TRUE(found);
    EXPECT_EQ(cmdStr, "lscpu");
}

TEST_F(ThreadPool_UT, InitCmd_LshwCmdString_ContainsRedirect)
{
    // Arrange / Act — lshw cmd redirects to PATH/lshw.txt
    bool found = false;
    QString cmdStr;
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "lshw.txt") {
            found = true;
            cmdStr = c.cmd;
        }
    }
    // Assert
    EXPECT_TRUE(found);
    EXPECT_TRUE(cmdStr.contains("lshw"));
    EXPECT_TRUE(cmdStr.contains(">"));
}

// ===== initCmd — entry count =====

TEST_F(ThreadPool_UT, InitCmd_AfterInit_ListCmdHas22Entries)
{
    // Arrange / Act — m_ListCmd: lshw + 9 dmidecode + upower + lscpu + lsblk + ls_sg + lspci +
    //            lpstat + dmesg + hciconfig + bt_device + dr_config + hwinfo + hwinfo_monitor
    // Assert
    EXPECT_EQ(pool->m_ListCmd.size(), 22);
    EXPECT_GT(pool->m_ListCmd.size(), pool->m_ListUpdate.size());
}

TEST_F(ThreadPool_UT, InitCmd_AfterInit_ListUpdateHas12Entries)
{
    // Arrange / Act — m_ListUpdate: lshw + upower + lscpu + lsblk + ls_sg + lspci + lpstat +
    //               dmesg + hciconfig + bt_device + hwinfo + hwinfo_monitor
    // Assert
    EXPECT_EQ(pool->m_ListUpdate.size(), 12);
    EXPECT_LT(pool->m_ListUpdate.size(), pool->m_ListCmd.size());
}

// ===== constructor — side effects =====

TEST_F(ThreadPool_UT, Constructor_AfterInit_CreatesDeviceInfoDirectory)
{
    // Arrange / Act — constructor calls mkdir(PATH) to create device info directory
    // Assert
    QDir dir(PATH);
    EXPECT_TRUE(dir.exists());
    EXPECT_TRUE(dir.isReadable());
}

TEST_F(ThreadPool_UT, Constructor_TypeCheck_InheritsQThreadPool)
{
    // Arrange / Act
    // Assert
    EXPECT_NE(dynamic_cast<QThreadPool *>(pool), nullptr);
    EXPECT_NE(pool, nullptr);
}
