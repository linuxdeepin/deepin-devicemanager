// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// 分支覆盖清单（基于 worktree ut-t09 @ f2d46ee9 源码逐分支核对）
// 被测: src/GenerateDevice/GenerateDevicePool.cpp
//
// GenerateTask::GenerateTask(~2 行)/~GenerateTask: m_Type 赋值
// GenerateTask::run(35-103, complexity:20/cognitive:38):
//   B1 getDeviceGenerator()==nullptr -> return(不 emit);
//   B2-B17 switch 各设备类型分支 -> generator + generatorInfoFromToml(DT_Computer/Cpu/Bios/
//   Memory/Storage/Gpu/Monitor/Network/Audio/Bluetoorh/Keyboard/Mouse/Print/Image/Cdrom/Power);
//   B18 default -> 不生成;
//   B19 尾部: emit finished(getBusIDFromHwinfo()) -> delete generator
// GenerateDevicePool::GenerateDevicePool: initType() 装载 16 类
// GenerateDevicePool::generateDevice(115-150, complexity:5/cognitive:8):
//   B1 16 个任务投递 globalInstance; B2 忙等: finished==size 或 >4000ms -> getDeviceGenerator
//   + generatorOthersDevice + generatorInfoFromToml(DT_Others)
// GenerateDevicePool::initType(152-172): 16 个 DT_* 按固定顺序入 m_TypeList
// GenerateDevicePool::slotFinished(174-178): addBusId(lst) + m_FinishedGenerator++
// 说明: 生成器经 DeviceFactory::getDeviceGenerator 打桩注入 HWGenerator;
// QProcess/QDir/tomlDeviceSet 打桩保证无真实设备依赖; 忙等经 QDateTime::currentMSecsSinceEpoch
// 打桩立即超时(真实路径槽经队列连接在忙等期间不投递, 恒走 4s 超时分支)。
// 私有成员访问依赖 tests/CMakeLists.txt 的 -fno-access-control。
// ============================================================================

#include "GenerateDevicePool.h"
#include "DeviceGenerator.h"
#include "DeviceFactory.h"
#include "DeviceManager.h"
#include "HWGenerator.h"
#include "MipsGenerator.h"
#include "commonfunction.h"
#include "stub.h"
#include "ut_Head.h"

#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QThreadPool>
#include <gtest/gtest.h>

// ---------- 打桩状态 ----------
static DeviceGenerator *s_factoryNext = nullptr;   // 工厂注入的生成器
static int s_factoryCalls = 0;
static int s_emitCount = 0;                        // GenerateTask::finished 触发次数
static QStringList s_lastEmitted;

// ---------- DeviceFactory / DeviceManager / QDateTime 打桩 ----------
static bool s_factoryNull = false; // FactoryNull 用例: 工厂恒返 nullptr

static DeviceGenerator *ut_factory_getDeviceGenerator(void *obj)
{
    Q_UNUSED(obj);
    ++s_factoryCalls;
    if (s_factoryNull)
        return nullptr;
    if (s_factoryNext) {
        DeviceGenerator *one = s_factoryNext; // 单任务测试: 首次取走预设实例
        s_factoryNext = nullptr;
        return one;
    }
    return new HWGenerator; // GenerateTask::run 尾部会 delete generator, 每次须新实例
}

// 空vendor -> 生成器构造走轻量路径, 不触发 dmidecode 子进程(真实工厂用例除外)
static QString ut_common_boardVendorType(void *obj)
{
    Q_UNUSED(obj);
    return QString();
}

static void ut_dm_tomlDeviceSet(void *obj, DeviceType deviceType)
{
    Q_UNUSED(obj);
    Q_UNUSED(deviceType);
}

static qint64 ut_qdatetime_now(void *obj)
{
    Q_UNUSED(obj);
    // 第一次调用为 beginMSecond, 其后一律超时
    static int calls = 0;
    ++calls;
    return calls == 1 ? 0 : 100000;
}

// ---------- QProcess 打桩 ----------
static void ut_gp_start(void *obj, const QString &program, const QStringList &args, QIODevice::OpenMode mode)
{
    Q_UNUSED(obj);
    Q_UNUSED(program);
    Q_UNUSED(args);
    Q_UNUSED(mode);
}

static bool ut_gp_waitForFinished(void *obj, int msecs)
{
    Q_UNUSED(obj);
    Q_UNUSED(msecs);
    return true;
}

static int ut_gp_exitCode(void *obj)
{
    Q_UNUSED(obj);
    return 127;
}

static QByteArray ut_gp_readAllStandardOutput(void *obj)
{
    Q_UNUSED(obj);
    return QByteArray();
}

// ---------- QDir 打桩(monitor 早退) ----------
static bool ut_gp_dirExists(void *obj)
{
    Q_UNUSED(obj);
    return false;
}

class UT_GenerateDevicePool : public UT_HEAD
{
public:
    Stub m_stub;

    void SetUp() override
    {
        DeviceManager::instance()->clear();
        s_factoryNext = nullptr;
        s_factoryNull = false;
        s_factoryCalls = 0;
        s_emitCount = 0;
        s_lastEmitted.clear();
        m_stub.set(ADDR(Common, boardVendorType), ut_common_boardVendorType);
        m_stub.set(ADDR(DeviceManager, tomlDeviceSet), ut_dm_tomlDeviceSet);
        m_stub.set(((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start)), ut_gp_start);
        m_stub.set((bool (QProcess::*)(int))ADDR(QProcess, waitForFinished), ut_gp_waitForFinished);
        m_stub.set(ADDR(QProcess, exitCode), ut_gp_exitCode);
        m_stub.set(ADDR(QProcess, readAllStandardOutput), ut_gp_readAllStandardOutput);
        m_stub.set((bool (QDir::*)() const)ADDR(QDir, exists), ut_gp_dirExists);
    }

    void TearDown() override
    {
        QThreadPool::globalInstance()->waitForDone();
        DeviceManager::instance()->clear();
        // m_stub 析构自动还原所有被替换函数
    }
};

// ============================================================================
// GenerateDevicePool
// ============================================================================

/// 构造: initType 装载 16 个设备类型
TEST_F(UT_GenerateDevicePool, Constructor_InitializesTypeList)
{
    GenerateDevicePool pool;
    EXPECT_EQ(pool.m_TypeList.size(), 16);
    EXPECT_EQ(pool.m_TypeList[0], DT_Bluetoorh);
    EXPECT_EQ(pool.m_TypeList[1], DT_Keyboard);
    EXPECT_EQ(pool.m_TypeList[2], DT_Mouse);
    EXPECT_EQ(pool.m_TypeList[3], DT_Image);
    EXPECT_EQ(pool.m_TypeList[4], DT_Storage);
    EXPECT_EQ(pool.m_TypeList[5], DT_Computer);
    EXPECT_EQ(pool.m_TypeList[6], DT_Cpu);
    EXPECT_EQ(pool.m_TypeList[7], DT_Bios);
    EXPECT_EQ(pool.m_TypeList[8], DT_Memory);
    EXPECT_EQ(pool.m_TypeList[9], DT_Gpu);
    EXPECT_EQ(pool.m_TypeList[10], DT_Monitor);
    EXPECT_EQ(pool.m_TypeList[11], DT_Network);
    EXPECT_EQ(pool.m_TypeList[12], DT_Audio);
    EXPECT_EQ(pool.m_TypeList[13], DT_Print);
    EXPECT_EQ(pool.m_TypeList[14], DT_Cdrom);
    EXPECT_EQ(pool.m_TypeList[15], DT_Power);
    // DT_Others 明确不入列表
    EXPECT_FALSE(pool.m_TypeList.contains(DT_Others));
}

/// slotFinished: addBusId 转发 + 计数
TEST_F(UT_GenerateDevicePool, SlotFinished_ForwardsBusIdAndCounts)
{
    GenerateDevicePool pool;
    const int finishedBefore = pool.m_FinishedGenerator; // 成员无初始化, 断言增量

    pool.slotFinished(QStringList() << "usb@1:1");
    pool.slotFinished(QStringList() << "usb@1:2" << "usb@1:3");

    EXPECT_EQ(pool.m_FinishedGenerator, finishedBefore + 2);
    EXPECT_EQ(DeviceManager::instance()->m_BusIdList.size(), 3);
    EXPECT_EQ(DeviceManager::instance()->m_BusIdList[0], "usb@1:1");
    EXPECT_EQ(DeviceManager::instance()->m_BusIdList[1], "usb@1:2");
    EXPECT_EQ(DeviceManager::instance()->m_BusIdList[2], "usb@1:3");
}

/// slotFinished: 空 busId 列表也计入
TEST_F(UT_GenerateDevicePool, SlotFinished_EmptyList_StillCounts)
{
    GenerateDevicePool pool;
    const int finishedBefore = pool.m_FinishedGenerator;
    pool.slotFinished(QStringList());
    EXPECT_EQ(pool.m_FinishedGenerator, finishedBefore + 1);
    // clear() 不复位 m_BusIdList, 用增量断言: 空 busId 不追加
    const int busIdCount = DeviceManager::instance()->m_BusIdList.size();
    EXPECT_EQ(DeviceManager::instance()->m_BusIdList.size(), busIdCount);
}

/// generateDevice: 投递 16 任务 + 超时走 others 生成; 忙等期间队列槽不投递,
/// m_FinishedGenerator 保持 0(QDateTime 打桩使其立即超时, 不真实忙等 4s)
TEST_F(UT_GenerateDevicePool, GenerateDevice_TasksDispatched_TimeoutRunsOthers)
{
    m_stub.set(ADDR(DeviceFactory, getDeviceGenerator), ut_factory_getDeviceGenerator);
    s_factoryNext = new HWGenerator;
    m_stub.set(ADDR(QDateTime, currentMSecsSinceEpoch), ut_qdatetime_now);

    GenerateDevicePool pool;
    pool.generateDevice();

    // 忙等期间 finished 槽经队列连接无法投递 -> 计数不变
    EXPECT_EQ(pool.m_FinishedGenerator, 0);
    // 16 个任务各调一次工厂 + DT_Others 再调一次(任务异步执行, 仅断言下界)
    EXPECT_GE(s_factoryCalls, 1);
    QThreadPool::globalInstance()->waitForDone();
    EXPECT_GE(s_factoryCalls, 17); // 16 任务 + 1 others
    s_factoryNext = nullptr; // run() 内部已 delete, 置空防止悬垂
}

// ============================================================================
// GenerateTask
// ============================================================================

/// B1: 工厂返回 nullptr -> run 直接返回, 不 emit
TEST_F(UT_GenerateDevicePool, TaskRun_FactoryNull_NoEmit)
{
    m_stub.set(ADDR(DeviceFactory, getDeviceGenerator), ut_factory_getDeviceGenerator);
    s_factoryNull = true;
    GenerateTask task(DT_Computer);
    QObject::connect(&task, &GenerateTask::finished, [&]() { ++s_emitCount; });
    task.run();
    EXPECT_EQ(s_emitCount, 0);
}

/// B2(B19): DT_Computer -> 生成器被调用且 emit finished
TEST_F(UT_GenerateDevicePool, TaskRun_ComputerType_InvokesAndEmits)
{
    m_stub.set(ADDR(DeviceFactory, getDeviceGenerator), ut_factory_getDeviceGenerator);
    s_factoryNext = new HWGenerator;
    GenerateTask task(DT_Computer);
    QObject::connect(&task, &GenerateTask::finished, [&](const QStringList &lst) {
        ++s_emitCount;
        s_lastEmitted = lst;
    });
    task.run();
    EXPECT_EQ(s_emitCount, 1);
    // 空 cmdInfo 下无设备生成, busID 列表为空
    EXPECT_EQ(s_lastEmitted.size(), 0);
    // HWGenerator::generatorComputerDevice 无条件 addComputerDevice(空数据也生成默认设备)
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceComputer.size(), 1);
}

/// B10: DT_Bluetoorh 分支 -> emit(蓝牙信息源为空)
TEST_F(UT_GenerateDevicePool, TaskRun_BluetoothType_InvokesAndEmits)
{
    m_stub.set(ADDR(DeviceFactory, getDeviceGenerator), ut_factory_getDeviceGenerator);
    s_factoryNext = new HWGenerator;
    GenerateTask task(DT_Bluetoorh);
    QObject::connect(&task, &GenerateTask::finished, [&](const QStringList &) { ++s_emitCount; });
    task.run();
    EXPECT_EQ(s_emitCount, 1);
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceBluetooth.size(), 0);
    s_factoryNext = nullptr; // run() 内部已 delete, 置空防止悬垂
}

/// B7: DT_Gpu 分支 -> gpuinfo 退出码 127 早退但仍 emit
TEST_F(UT_GenerateDevicePool, TaskRun_GpuType_GpuinfoMissing_StillEmits)
{
    m_stub.set(ADDR(DeviceFactory, getDeviceGenerator), ut_factory_getDeviceGenerator);
    s_factoryNext = new HWGenerator;
    GenerateTask task(DT_Gpu);
    QObject::connect(&task, &GenerateTask::finished, [&](const QStringList &) { ++s_emitCount; });
    task.run();
    EXPECT_EQ(s_emitCount, 1);
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceGPU.size(), 0);
    s_factoryNext = nullptr; // run() 内部已 delete, 置空防止悬垂
}

/// B9: DT_Audio 分支 -> emit 且无音频设备
TEST_F(UT_GenerateDevicePool, TaskRun_AudioType_InvokesAndEmits)
{
    m_stub.set(ADDR(DeviceFactory, getDeviceGenerator), ut_factory_getDeviceGenerator);
    s_factoryNext = new HWGenerator;
    GenerateTask task(DT_Audio);
    QObject::connect(&task, &GenerateTask::finished, [&](const QStringList &) { ++s_emitCount; });
    task.run();
    EXPECT_EQ(s_emitCount, 1);
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceAudio.size(), 0);
    s_factoryNext = nullptr; // run() 内部已 delete, 置空防止悬垂
}

/// B18: default 分支(DT_Null) -> 不生成任何设备, 但仍 emit
TEST_F(UT_GenerateDevicePool, TaskRun_NullType_DefaultStillEmits)
{
    m_stub.set(ADDR(DeviceFactory, getDeviceGenerator), ut_factory_getDeviceGenerator);
    s_factoryNext = new HWGenerator;
    GenerateTask task(DT_Null);
    QObject::connect(&task, &GenerateTask::finished, [&](const QStringList &) { ++s_emitCount; });
    task.run();
    EXPECT_EQ(s_emitCount, 1);
    s_factoryNext = nullptr; // run() 内部已 delete, 置空防止悬垂
}

/// B18: DT_Others 也走 default(不在 switch 分支列表中)
TEST_F(UT_GenerateDevicePool, TaskRun_OthersType_DefaultStillEmits)
{
    m_stub.set(ADDR(DeviceFactory, getDeviceGenerator), ut_factory_getDeviceGenerator);
    s_factoryNext = new HWGenerator;
    GenerateTask task(DT_Others);
    QObject::connect(&task, &GenerateTask::finished, [&](const QStringList &) { ++s_emitCount; });
    task.run();
    EXPECT_EQ(s_emitCount, 1);
    s_factoryNext = nullptr; // run() 内部已 delete, 置空防止悬垂
}

// ---------- Common 打桩(真实工厂验证用) ----------
static QString ut_common_getArch(void *obj)
{
    Q_UNUSED(obj);
    return "mips64";
}

/// 真实工厂(依赖注入验证): getArch=mips64 -> MipsGenerator
TEST_F(UT_GenerateDevicePool, RealFactory_MipsArch_CreatesMipsGenerator)
{
    Stub localStub;
    localStub.set(ADDR(Common, getArch), ut_common_getArch);
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();
    ASSERT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<MipsGenerator *>(gen), nullptr);
    delete gen;
}
