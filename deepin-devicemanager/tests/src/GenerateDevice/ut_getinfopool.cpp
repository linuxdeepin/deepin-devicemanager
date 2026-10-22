// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// 分支覆盖清单（基于 worktree ut-t09 @ f2d46ee9 源码逐分支核对）
// 被测: src/GenerateDevice/GetInfoPool.cpp (GetInfoPool + CmdTask)
//
// CmdTask::CmdTask: m_Key/m_File/m_Info/mp_Parent 赋值
// CmdTask::run(33-41):
//   B1 CmdTool::loadCmdInfo(m_Key, m_File) 装载命令信息;
//   B2 mp_Parent->finishedCmd(m_Info, tool.cmdInfo()) 转发结果
// GetInfoPool::GetInfoPool(43-49): initCmd() 构造即装载 29 条命令
// GetInfoPool::getAllInfo(51-63):
//   B1 DeviceManager::instance()->clear() 清空;
//   B2 遍历 m_CmdList: 每条 new CmdTask(key[0], file[1], info[2], this) + start + deleteLater
// GetInfoPool::finishedCmd(65-77):
//   B1 DeviceManager::addCmdInfo(cmdInfo) 合并;
//   B2 QMutexLocker + m_FinishedNum++;
//   B3 m_FinishedNum == m_CmdList.size() -> emit finishedAll(info) + 归零
// GetInfoPool::setFramework(78-83): m_Arch = arch
// GetInfoPool::initCmd(85-123): 逐条 append 29 条命令(不先清空)
// Stub: CmdTool::loadCmdInfo 打桩为空操作 —— 测试不执行真实命令(lshw/hwinfo 等),
//       不依赖真实设备环境。原实现的 getDeviceInfo 桩保留为参考(见注释)。
// ============================================================================

#include "GetInfoPool.h"
#include "DeviceManager.h"
#include "CmdTool.h"
#include "stub.h"
#include "ut_Head.h"

#include <QThreadPool>
#include <gtest/gtest.h>

// ---------- 打桩状态 ----------
static QString s_cmdKey;      // loadCmdInfo 捕获: 命令名
static QString s_cmdFile;     // loadCmdInfo 捕获: 信息文件名

// ---------- CmdTool 打桩: 不执行真实命令 ----------
static void ut_ct_loadCmdInfo(void *obj, const QString &cmd, const QString &file)
{
    Q_UNUSED(obj);
    s_cmdKey = cmd;
    s_cmdFile = file;
}

class UT_GetInfoPool : public UT_HEAD
{
public:
    Stub m_stub;
    GetInfoPool *m_readFilePool = nullptr;

    void SetUp()
    {
        DeviceManager::instance()->clear();
        s_cmdKey.clear();
        s_cmdFile.clear();
        m_stub.set(ADDR(CmdTool, loadCmdInfo), ut_ct_loadCmdInfo);
        m_readFilePool = new GetInfoPool;
    }
    void TearDown()
    {
        m_readFilePool->waitForDone(-1);
        delete m_readFilePool;
        m_readFilePool = nullptr;
        DeviceManager::instance()->clear();
        // m_stub 析构自动还原所有被替换函数
    }
};

class UT_CmdTask : public UT_HEAD
{
public:
    Stub m_stub;
    GetInfoPool *readFilePool = nullptr;
    CmdTask *m_cmdTask = nullptr;

    void SetUp()
    {
        DeviceManager::instance()->clear();
        s_cmdKey.clear();
        s_cmdFile.clear();
        m_stub.set(ADDR(CmdTool, loadCmdInfo), ut_ct_loadCmdInfo);
        readFilePool = new GetInfoPool;
        m_cmdTask = new CmdTask("", "", "", readFilePool);
    }
    void TearDown()
    {
        readFilePool->waitForDone(-1);
        delete readFilePool;
        delete m_cmdTask;
        m_cmdTask = nullptr;
        readFilePool = nullptr;
        DeviceManager::instance()->clear();
    }
};

// ============================================================================
// GetInfoPool
// ============================================================================

/// 构造: initCmd 装载 29 条命令, 状态归零
TEST_F(UT_GetInfoPool, Constructor_LoadsCommandList)
{
    EXPECT_EQ(m_readFilePool->m_CmdList.size(), 29);
    EXPECT_EQ(m_readFilePool->m_Arch, QString());
    EXPECT_EQ(m_readFilePool->m_FinishedNum, 0);
    // 首条命令为 lshw
    EXPECT_EQ(m_readFilePool->m_CmdList[0][0], "lshw");
    EXPECT_EQ(m_readFilePool->m_CmdList[0][1], "lshw.txt");
}

/// initCmd: 清空后重新装载仍为 29(append 语义, 重复调用会累加)
TEST_F(UT_GetInfoPool, InitCmd_RebuildAfterClear_29Entries)
{
    m_readFilePool->m_CmdList.clear();
    m_readFilePool->initCmd();
    EXPECT_EQ(m_readFilePool->m_CmdList.size(), 29);
    EXPECT_EQ(m_readFilePool->m_CmdList[1][0], "printer");
    EXPECT_EQ(m_readFilePool->m_CmdList[6][0], "dmidecode4");
}

/// setFramework: 记录架构
TEST_F(UT_GetInfoPool, SetFramework_StoresArch)
{
    m_readFilePool->setFramework("x86_64");
    EXPECT_EQ(m_readFilePool->m_Arch, "x86_64");

    m_readFilePool->setFramework("mips64");
    EXPECT_EQ(m_readFilePool->m_Arch, "mips64");
}

/// getAllInfo: 清空 DeviceManager + 每条命令派发一个 CmdTask
TEST_F(UT_GetInfoPool, GetAllInfo_ClearsManagerAndDispatches29Tasks)
{
    // 预置 DeviceManager 内容, 验证 getAllInfo 清空行为
    QMap<QString, QList<QMap<QString, QString>>> seed;
    seed["cat_os_release"] = QList<QMap<QString, QString>>() << QMap<QString, QString> {{"HOME_URL", "x"}};
    DeviceManager::instance()->addCmdInfo(seed);
    EXPECT_FALSE(DeviceManager::instance()->cmdInfo("cat_os_release").isEmpty());

    m_readFilePool->getAllInfo();
    m_readFilePool->waitForDone(-1);

    // DeviceManager 已被清空
    EXPECT_TRUE(DeviceManager::instance()->cmdInfo("cat_os_release").isEmpty());
    // 29 条命令各执行一次(经 CmdTask::run -> loadCmdInfo); 多线程竞争, 不检查最后一次 key
    EXPECT_EQ(m_readFilePool->m_FinishedNum, 0); // finishedAll 触发后归零
}

/// getAllInfo: m_CmdList 为空时为空操作
TEST_F(UT_GetInfoPool, GetAllInfo_EmptyCommandList_NoDispatch)
{
    m_readFilePool->m_CmdList.clear();
    m_readFilePool->getAllInfo();
    m_readFilePool->waitForDone(-1);
    EXPECT_EQ(m_readFilePool->m_FinishedNum, 0);
}

/// finishedCmd: 合并 cmdInfo 到 DeviceManager + 计数
TEST_F(UT_GetInfoPool, FinishedCmd_MergesInfoAndCounts)
{
    QMap<QString, QList<QMap<QString, QString>>> data;
    data["lshw"] = QList<QMap<QString, QString>>() << QMap<QString, QString> {{"vendor", "Lenovo"}};
    m_readFilePool->finishedCmd("audio", data);

    EXPECT_EQ(m_readFilePool->m_FinishedNum, 1);
    EXPECT_FALSE(DeviceManager::instance()->cmdInfo("lshw").isEmpty());
}

/// finishedCmd: 计满 m_CmdList.size() -> emit finishedAll(info) 并归零
TEST_F(UT_GetInfoPool, FinishedCmd_ReachesListSize_EmitsFinishedAll)
{
    int emits = 0;
    QString lastInfo;
    QObject::connect(m_readFilePool, &GetInfoPool::finishedAll, [&](const QString &info) {
        ++emits;
        lastInfo = info;
    });

    // 28 次: 不触发
    for (int i = 0; i < 28; ++i)
        m_readFilePool->finishedCmd(QString("cmd-%1").arg(i), QMap<QString, QList<QMap<QString, QString>>>());
    EXPECT_EQ(emits, 0);
    EXPECT_EQ(m_readFilePool->m_FinishedNum, 28);

    // 第 29 次: 触发并归零
    m_readFilePool->finishedCmd("last-cmd", QMap<QString, QList<QMap<QString, QString>>>());
    EXPECT_EQ(emits, 1);
    EXPECT_EQ(lastInfo, "last-cmd");
    EXPECT_EQ(m_readFilePool->m_FinishedNum, 0);

    // 归零后重新计数: 不再触发
    m_readFilePool->finishedCmd("next-round", QMap<QString, QList<QMap<QString, QString>>>());
    EXPECT_EQ(emits, 1);
    EXPECT_EQ(m_readFilePool->m_FinishedNum, 1);
}

// ============================================================================
// CmdTask
// ============================================================================

/// 构造: 参数正确落到成员
TEST_F(UT_CmdTask, Constructor_StoresMembers)
{
    CmdTask task("audio-key", "audio-file.txt", "audio-info", readFilePool);
    EXPECT_EQ(task.m_Key, "audio-key");
    EXPECT_EQ(task.m_File, "audio-file.txt");
    EXPECT_EQ(task.m_Info, "audio-info");
    EXPECT_EQ(task.mp_Parent, readFilePool);
}

/// run: 调用 loadCmdInfo(m_Key, m_File) 并将结果转发给 finishedCmd(m_Info, ...)
TEST_F(UT_CmdTask, Run_LoadsAndForwardsToParent)
{
    CmdTask task("audio-key", "audio-file.txt", "audio-info", readFilePool);
    task.run();

    // loadCmdInfo 收到的参数来自 CmdTask 成员
    EXPECT_EQ(s_cmdKey, "audio-key");
    EXPECT_EQ(s_cmdFile, "audio-file.txt");
    // finishedCmd 已被真实调用(空 cmdInfo 合并 + 计数)
    EXPECT_EQ(readFilePool->m_FinishedNum, 1);
}

/// run: 空参 CmdTask 也能安全执行
TEST_F(UT_CmdTask, Run_EmptyMembers_SafeExecution)
{
    m_cmdTask->run();
    EXPECT_EQ(s_cmdKey, QString());
    EXPECT_EQ(s_cmdFile, QString());
    EXPECT_EQ(readFilePool->m_FinishedNum, 1);
}
