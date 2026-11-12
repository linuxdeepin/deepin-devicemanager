// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// 重写说明(Mode 2/t17): 原文件仅 1 个无断言用例(EMPTY_ASSERT/MISSING_AAA)。
// 本文件覆盖 src/Tool/LoadCpuInfoThread.cpp 全部 5 个方法:
//   LoadCpuInfoThread() / run / runCmd / loadCpuInfo / getMapInfoFromCmd /
//   getCpuInfoFromLscpu
// usecase_count: 9
// 打桩矩阵(方法体私有, 经 #define private public 直测):
//   QProcess::start / waitForFinished / readAllStandardOutput
//       -> stub.set(static_cast 成员指针, ...) 注入 canned lscpu 输出,
//          不启动真实进程
//   DeviceManager::cmdInfo             -> stub 返回受控列表
//   DeviceManager::setCpuRefreshInfoFromlscpu -> stub 捕获刷新数据
//   DeviceManager::instance() 为项目既有测试范式(单例真实构造一次)。
// 无真实设备/DBus/文件依赖。
// ============================================================================

#include <gtest/gtest.h>

#include <QProcess>
#include <QThread>

#define private public
#include "LoadCpuInfoThread.h"
#undef private

#include "DeviceManager.h"
#include "ut_Head.h"
#include "stub.h"

// ---- 打桩辅助: canned lscpu 输出 ----
static const char *ut_lscpuOutput =
    "Architecture:        x86_64\n"
    "CPU(s):              8\n"
    "Model name:          Fake CPU @ 2.0GHz\n"
    "Socket(s):           1\n"
    "Core(s) per socket:  4\n"
    "bad line without separator\n"
    "Thread(s) per core:  2: with: colon\n";

static QByteArray ut_qprocess_stdout;

static void ut_qprocess_start(QProcess *self, const QString &program,
                              const QStringList &arguments, QIODevice::OpenMode mode)
{
    Q_UNUSED(self);
    Q_UNUSED(program);
    Q_UNUSED(arguments);
    Q_UNUSED(mode);
}

static bool ut_qprocess_waitForFinished_true(QProcess *self, int msecs)
{
    Q_UNUSED(self);
    Q_UNUSED(msecs);
    return true;
}

static QByteArray ut_qprocess_readAll_canned(QProcess *self)
{
    Q_UNUSED(self);
    return ut_qprocess_stdout;
}

// ---- 打桩辅助: DeviceManager 数据 ----
static QList<QMap<QString, QString>> ut_cmdInfoList;

static const QList<QMap<QString, QString>> &ut_devicemanager_cmdInfo(DeviceManager *self, const QString &key)
{
    Q_UNUSED(self);
    Q_UNUSED(key);
    return ut_cmdInfoList;
}

static QMap<QString, QString> ut_capturedCpuRefresh;
static int ut_cpuRefreshCallCount = 0;

static void ut_devicemanager_setCpuRefreshInfoFromlscpu(DeviceManager *self, const QMap<QString, QString> &mapInfo)
{
    Q_UNUSED(self);
    ut_capturedCpuRefresh = mapInfo;
    ++ut_cpuRefreshCallCount;
}

class UT_LoadCpuInfoThread : public UT_HEAD
{
public:
    void SetUp() override
    {
        // Arrange: 每个 Case 重置桩数据并创建被测线程对象
        ut_qprocess_stdout.clear();
        ut_cmdInfoList.clear();
        ut_capturedCpuRefresh.clear();
        ut_cpuRefreshCallCount = 0;
        m_thread = new LoadCpuInfoThread();
    }

    void TearDown() override
    {
        delete m_thread;
        m_thread = nullptr;
    }

    LoadCpuInfoThread *m_thread = nullptr;
    Stub stub;

    void stubQProcess()
    {
        // 三个成员指针均用 static_cast 消歧(QProcess::start 存在多个重载)
        void (QProcess::*startFunc)(const QString &, const QStringList &, QIODevice::OpenMode) =
            static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start);
        bool (QProcess::*waitFunc)(int) = static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished);
        QByteArray (QProcess::*readFunc)() = static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput);
        stub.set(startFunc, ut_qprocess_start);
        stub.set(waitFunc, ut_qprocess_waitForFinished_true);
        stub.set(readFunc, ut_qprocess_readAll_canned);
    }

    void stubDeviceManager()
    {
        stub.set(ADDR(DeviceManager, cmdInfo), ut_devicemanager_cmdInfo);
        stub.set(ADDR(DeviceManager, setCpuRefreshInfoFromlscpu), ut_devicemanager_setCpuRefreshInfoFromlscpu);
    }
};

// 构造: 对象创建成功且未在运行态
TEST_F(UT_LoadCpuInfoThread, Constructor_Default_ThreadNotRunning)
{
    // Arrange: SetUp 中已构造 LoadCpuInfoThread 实例
    // Act: 直接读取构造后的线程状态
    const bool running = m_thread->isRunning();

    // Assert: 实例有效且未在运行态
    EXPECT_NE(m_thread, nullptr);
    EXPECT_FALSE(running);
}

// getMapInfoFromCmd: 以 ": " 分隔的合法行被解析进 map
TEST_F(UT_LoadCpuInfoThread, GetMapInfoFromCmd_ValidLines_ParsesIntoMap)
{
    // Arrange
    QMap<QString, QString> mapInfo;

    // Act
    m_thread->getMapInfoFromCmd(QString::fromLocal8Bit(ut_lscpuOutput), mapInfo, ": ");

    // Assert
    EXPECT_EQ(QString("x86_64"), mapInfo.value("Architecture"));
    EXPECT_EQ(QString("8"), mapInfo.value("CPU(s)"));
    EXPECT_EQ(QString("Fake CPU @ 2.0GHz"), mapInfo.value("Model name"));
    EXPECT_EQ(QString("1"), mapInfo.value("Socket(s)"));
    EXPECT_EQ(5, mapInfo.size());
}

// getMapInfoFromCmd: 空输入产生空 map
TEST_F(UT_LoadCpuInfoThread, GetMapInfoFromCmd_EmptyInput_EmptyMap)
{
    // Arrange
    QMap<QString, QString> mapInfo;

    // Act
    m_thread->getMapInfoFromCmd(QString(), mapInfo);

    // Assert
    EXPECT_EQ(0, mapInfo.size());
    EXPECT_TRUE(mapInfo.isEmpty());
}

// getMapInfoFromCmd: 值中含 ": " 时 split 产生 3 段被跳过, 键不写入
TEST_F(UT_LoadCpuInfoThread, GetMapInfoFromCmd_MoreThanTwoFields_SkipsLine)
{
    // Arrange: "A: B: C" split 后 size==3, 不插入; 单段行同样跳过
    QMap<QString, QString> mapInfo;

    // Act
    m_thread->getMapInfoFromCmd("A: B: C\nsingle-no-separator\n", mapInfo, ": ");

    // Assert
    EXPECT_EQ(0, mapInfo.size());
    EXPECT_FALSE(mapInfo.contains("A"));
    EXPECT_TRUE(mapInfo.isEmpty());
}

// runCmd: 进程正常结束时返回标准输出
TEST_F(UT_LoadCpuInfoThread, RunCmd_ProcessFinishes_ReturnsStdout)
{
    // Arrange
    stubQProcess();
    ut_qprocess_stdout = QByteArray(ut_lscpuOutput);
    QString info;

    // Act
    m_thread->runCmd(info, "lscpu");

    // Assert
    EXPECT_EQ(QString::fromLocal8Bit(ut_lscpuOutput), info);
    EXPECT_FALSE(info.isEmpty());
}

// loadCpuInfo: runCmd + getMapInfoFromCmd 组合输出结构化 map
TEST_F(UT_LoadCpuInfoThread, LoadCpuInfo_WithCannedOutput_PopulatesMap)
{
    // Arrange
    stubQProcess();
    ut_qprocess_stdout = QByteArray(ut_lscpuOutput);
    QMap<QString, QString> mapInfo;

    // Act
    m_thread->loadCpuInfo(mapInfo, "lscpu");

    // Assert
    EXPECT_EQ(QString("8"), mapInfo.value("CPU(s)"));
    EXPECT_EQ(QString("4"), mapInfo.value("Core(s) per socket"));
    EXPECT_FALSE(mapInfo.isEmpty());
}

// getCpuInfoFromLscpu: cmdInfo 为空时直接返回, 不触发刷新
TEST_F(UT_LoadCpuInfoThread, GetCpuInfoFromLscpu_EmptyCmdInfo_SkipsRefresh)
{
    // Arrange: DeviceManager 数据桩返回空列表
    stubDeviceManager();
    stubQProcess();

    // Act
    m_thread->getCpuInfoFromLscpu();

    // Assert: 未调用 setCpuRefreshInfoFromlscpu
    EXPECT_EQ(0, ut_cpuRefreshCallCount);
    EXPECT_TRUE(ut_capturedCpuRefresh.isEmpty());
}

// getCpuInfoFromLscpu: cmdInfo 非空时解析 lscpu 并刷新 CPU 信息
TEST_F(UT_LoadCpuInfoThread, GetCpuInfoFromLscpu_NonEmptyCmdInfo_RefreshesCpu)
{
    // Arrange
    stubDeviceManager();
    stubQProcess();
    ut_qprocess_stdout = QByteArray(ut_lscpuOutput);
    QMap<QString, QString> seed;
    seed.insert("placeholder", "1");
    ut_cmdInfoList.append(seed);

    // Act
    m_thread->getCpuInfoFromLscpu();

    // Assert: 刷新被调用一次且携带解析结果
    EXPECT_EQ(1, ut_cpuRefreshCallCount);
    EXPECT_EQ(QString("8"), ut_capturedCpuRefresh.value("CPU(s)"));
    EXPECT_EQ(QString("Fake CPU @ 2.0GHz"), ut_capturedCpuRefresh.value("Model name"));
}

// run: 直接调用线程体完成信息采集(不 start 线程, 同步执行)
TEST_F(UT_LoadCpuInfoThread, Run_ExecutedDirectly_CompletesWithRefresh)
{
    // Arrange
    stubDeviceManager();
    stubQProcess();
    ut_qprocess_stdout = QByteArray(ut_lscpuOutput);
    QMap<QString, QString> seed;
    seed.insert("placeholder", "1");
    ut_cmdInfoList.append(seed);

    // Act
    m_thread->run();

    // Assert
    EXPECT_EQ(1, ut_cpuRefreshCallCount);
    EXPECT_EQ(QString("x86_64"), ut_capturedCpuRefresh.value("Architecture"));
}
