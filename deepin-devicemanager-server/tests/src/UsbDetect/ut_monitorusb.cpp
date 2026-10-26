// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * 用例计数声明（method | level | factors | min | actual）
 * | MonitorUsb::MonitorUsb | low | - | 1 | 3 |
 * | MonitorUsb::monitor | high | complexity:9,cognitive:18,lines:67,in_degree:81 | 3 | 9 |
 * | MonitorUsb::setWorkingFlag | low | - | 1 | 1 |
 * | MonitorUsb::slotTimeout | low | - | 1 | 4 |
 */

#include <gtest/gtest.h>

#include <QDBusConnection>
#include <QDateTime>
#include <QProcess>
#include <QSignalSpy>

#include <sys/select.h>
#include <sys/time.h>
#include <libudev.h>

#include "stubext.h"
#include "monitorusb.h"
#include "controlinterface.h"

using namespace stub_ext;

namespace {
// 伪造的 udev 上下文 / monitor / device 指针，仅用于非空判断与桩间传递
constexpr intptr_t kFakeUdev = 0x1001;
constexpr intptr_t kFakeMonitor = 0x2002;
constexpr intptr_t kFakeDevice = 0x3003;
constexpr int kFakeFd = 100;

// monitor() 监听循环的 fd 值即 udev_monitor_get_fd 的返回值
void stubUdevEnv(stub_ext::StubExt &stub)
{
    stub.set_lamda(udev_new,
                   []() -> struct udev * { return reinterpret_cast<struct udev *>(kFakeUdev); });
    stub.set_lamda(udev_monitor_new_from_netlink,
                   [](struct udev *, const char *) -> struct udev_monitor * {
                       return reinterpret_cast<struct udev_monitor *>(kFakeMonitor);
                   });
    stub.set_lamda(udev_monitor_filter_add_match_subsystem_devtype,
                   [](struct udev_monitor *, const char *, const char *) -> int { return 0; });
    stub.set_lamda(udev_monitor_enable_receiving,
                   [](struct udev_monitor *) -> int { return 0; });
    stub.set_lamda(udev_monitor_get_fd,
                   [](struct udev_monitor *) -> int { return kFakeFd; });
}

// select 打桩：模拟 fd 就绪；第二次调用后清空工作标志以终止监听循环，避免死循环
// selectCalls 为用例内局部计数，由调用方持有
void stubSelectReady(stub_ext::StubExt &stub, MonitorUsb *monitor, int &selectCalls,
                     bool withFdSet = true)
{
    stub.set_lamda(select,
                   [&selectCalls, monitor, withFdSet](int nfds, fd_set *readfds, fd_set *, fd_set *,
                                                      struct timeval *) -> int {
                       ++selectCalls;
                       if (selectCalls >= 2)
                           monitor->m_workingFlag = false;
                       if (withFdSet && selectCalls == 1)
                           FD_SET(nfds - 1, readfds);
                       return 1;
                   });
}

// udev 设备事件打桩：第一次返回伪设备，之后返回空并终止循环
void stubUdevDeviceEvent(stub_ext::StubExt &stub, MonitorUsb *monitor, int &receiveCalls,
                         const char *devType, unsigned long long devNum, const char *action)
{
    stub.set_lamda(udev_monitor_receive_device,
                   [&receiveCalls, monitor](struct udev_monitor *) -> struct udev_device * {
                       ++receiveCalls;
                       if (receiveCalls >= 2) {
                           monitor->m_workingFlag = false;
                           return nullptr;
                       }
                       return reinterpret_cast<struct udev_device *>(kFakeDevice);
                   });
    stub.set_lamda(udev_device_get_devtype,
                   [devType](struct udev_device *) -> const char * { return devType; });
    stub.set_lamda(udev_device_get_devnum,
                   [devNum](struct udev_device *) -> unsigned long long { return devNum; });
    stub.set_lamda(udev_device_get_action,
                   [action](struct udev_device *) -> const char * { return action; });
}

// ControlInterface 单例的 DBus 依赖全部打桩，保证测试不触达真实系统总线
void stubControlInterface(stub_ext::StubExt &stub, int *disableOutCalls, int *updateWakeupCalls)
{
    stub.set_lamda(static_cast<QDBusConnection (*)()>(&QDBusConnection::systemBus),
                   []() -> QDBusConnection { return QDBusConnection(QString()); });
    stub.set_lamda(static_cast<void (ControlInterface::*)(const QString &)>(&ControlInterface::disableOutDevice),
                   [disableOutCalls](ControlInterface *, const QString &) { ++(*disableOutCalls); });
    stub.set_lamda(static_cast<void (ControlInterface::*)(const QString &)>(&ControlInterface::updateWakeup),
                   [updateWakeupCalls](ControlInterface *, const QString &) { ++(*updateWakeupCalls); });
}

// add/remove 事件路径中的 hwinfo 探测打桩（输出为空即可，结果只被透传）
void stubQProcessEmpty(stub_ext::StubExt &stub)
{
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) {});
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                   [](QProcess *, int) -> bool { return true; });
    stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                   [](QProcess *) -> QByteArray { return QByteArray(); });
}
} // namespace

class MonitorUsbTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stubUdevEnv(stub);
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// ---------- MonitorUsb::MonitorUsb (monitorusb.cpp:17-43) ----------
// 分支：udev_new / monitor 创建 / 过滤器 / enable_receiving / get_fd 全部初始化
TEST_F(MonitorUsbTest, Ctor_WithStubbedUdev_InitializesContextMonitorAndTimer)
{
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    // Act
    MonitorUsb monitor;

    // Assert
    EXPECT_EQ(reinterpret_cast<void *>(kFakeUdev), reinterpret_cast<void *>(monitor.m_Udev));
    EXPECT_EQ(kFakeFd, monitor.fd);
    EXPECT_TRUE(monitor.m_workingFlag);
    EXPECT_FALSE(monitor.m_UsbChanged);
    ASSERT_NE(nullptr, monitor.mp_Timer);
}

// 分支：udev_new 失败（返回空）-> 记录警告并继续构造，不崩溃
TEST_F(MonitorUsbTest, Ctor_UdevNewReturnsNull_NoCrash)
{
    // Arrange
    stub.set_lamda(&udev_new, []() -> struct udev* { return nullptr; });

    // Act
    MonitorUsb monitor;
    monitor.setWorkingFlag(false);

    // Assert
    EXPECT_NO_THROW(monitor.monitor());
    EXPECT_FALSE(monitor.m_UsbChanged);
    EXPECT_EQ(nullptr, monitor.m_Udev);   // udev_new 失败时上下文保持为空
}

// ---------- MonitorUsb::monitor (monitorusb.cpp:45-111) ----------
// 分支：m_workingFlag 为 false 时直接退出，未进入 select 循环
TEST_F(MonitorUsbTest, Monitor_WorkingFlagDisabled_ExitsBeforeSelectLoop)
{
    // Arrange
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    MonitorUsb monitor;
    monitor.setWorkingFlag(false);
    int selectCalls = 0;
    stub.set_lamda(select,
                   [&selectCalls](int, fd_set *, fd_set *, fd_set *, struct timeval *) -> int {
                       ++selectCalls;
                       return 0;
                   });

    monitor.monitor();   // 正常返回即代表循环退出

    // Assert
    EXPECT_EQ(0, selectCalls);
    EXPECT_FALSE(monitor.m_workingFlag);
}

// 分支：select 超时返回 0 -> continue 继续监听，随后标志被清退出
TEST_F(MonitorUsbTest, Monitor_SelectTimeout_ContinuesThenExitsOnFlagCleared)
{
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    // Act
    MonitorUsb monitor;
    int selectCalls = 0;
    stub.set_lamda(select,
                   [&selectCalls, &monitor](int, fd_set *, fd_set *, fd_set *,
                                            struct timeval *) -> int {
                       ++selectCalls;
                       if (selectCalls >= 2)
                           monitor.m_workingFlag = false;
                       return 0;   // 超时
                   });
    int receiveCalls = 0;
    stub.set_lamda(udev_monitor_receive_device,
                   [&receiveCalls](struct udev_monitor *) -> struct udev_device * {
                       ++receiveCalls;
                       return nullptr;
                   });

    monitor.monitor();

    // Assert
    EXPECT_GE(selectCalls, 2);   // 至少经历一次超时继续
    EXPECT_EQ(0, receiveCalls);  // 超时路径不会取设备
}

// 分支：select 就绪但 fd 未置位 -> FD_ISSET 为假，继续循环且不取设备
TEST_F(MonitorUsbTest, Monitor_SelectReadyButFdNotSet_ContinuesLoop)
{
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    // Act
    MonitorUsb monitor;
    int selectCalls = 0;
    // 监听循环自身先 FD_SET(fd) 再调 select；此处模拟"就绪但 fd 未置位"
    // 必须用 FD_CLR 清掉，才能走到 FD_ISSET 为假的 continue 分支
    stub.set_lamda(select,
                   [&selectCalls, &monitor](int nfds, fd_set *readfds, fd_set *, fd_set *,
                                            struct timeval *) -> int {
                       ++selectCalls;
                       if (selectCalls >= 2)
                           monitor.m_workingFlag = false;
                       FD_CLR(nfds - 1, readfds);   // fd = nfds-1
                       return 1;
                   });
    int receiveCalls = 0;
    stub.set_lamda(udev_monitor_receive_device,
                   [&receiveCalls](struct udev_monitor *) -> struct udev_device * {
                       ++receiveCalls;
                       return nullptr;
                   });

    monitor.monitor();

    // Assert
    EXPECT_GE(selectCalls, 2);   // 就绪但 fd 未置位 -> continue 后再次进入 select
    EXPECT_EQ(0, receiveCalls);
}

// 分支：取到的 udev 设备为空 -> continue，不发射信号
TEST_F(MonitorUsbTest, Monitor_ReceiveDeviceNull_ContinuesLoop)
{
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    // Act
    MonitorUsb monitor;
    int selectCalls = 0;
    stubSelectReady(stub, &monitor, selectCalls);
    int receiveCalls = 0;
    stub.set_lamda(udev_monitor_receive_device,
                   [&receiveCalls, &monitor](struct udev_monitor *) -> struct udev_device * {
                       ++receiveCalls;
                       if (receiveCalls >= 2)
                           monitor.m_workingFlag = false;
                       return nullptr;
                   });

    QSignalSpy spy(&monitor, &MonitorUsb::usbChanged);
    monitor.monitor();

    // Assert
    EXPECT_EQ(2, receiveCalls);
    EXPECT_EQ(0, spy.count());
}

// 分支：devtype 为 "link"（蓝牙）且工作中 -> 发射 usbChanged 并继续循环
TEST_F(MonitorUsbTest, Monitor_BluetoothLinkDevice_EmitsUsbChanged)
{
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    // Act
    MonitorUsb monitor;
    int selectCalls = 0;
    stubSelectReady(stub, &monitor, selectCalls);
    int receiveCalls = 0;
    stubUdevDeviceEvent(stub, &monitor, receiveCalls, "link", 42, "add");
    stub.set_lamda(udev_device_unref, [](struct udev_device *dev) -> struct udev_device * { return dev; });
    int disableOutCalls = 0;
    int updateWakeupCalls = 0;
    stubControlInterface(stub, &disableOutCalls, &updateWakeupCalls);
    stubQProcessEmpty(stub);

    QSignalSpy spy(&monitor, &MonitorUsb::usbChanged);
    monitor.monitor();

    // Assert
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(2, selectCalls);   // 就绪处理一次后，下一轮因标志清空退出循环
}

// 分支：devnum 为 0 的设备被忽略并 unref，不更新缓存
TEST_F(MonitorUsbTest, Monitor_DevnumZero_UnrefsDeviceAndContinues)
{
    // Arrange
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    MonitorUsb monitor;
    int selectCalls = 0;
    stubSelectReady(stub, &monitor, selectCalls);
    int receiveCalls = 0;
    stubUdevDeviceEvent(stub, &monitor, receiveCalls, "usb", 0, "add");
    int unrefCalls = 0;
    stub.set_lamda(udev_device_unref,
                   [&unrefCalls](struct udev_device *dev) -> struct udev_device * {
                       ++unrefCalls;
                       return dev;
                   });
    int disableOutCalls = 0;
    int updateWakeupCalls = 0;
    stubControlInterface(stub, &disableOutCalls, &updateWakeupCalls);
    stubQProcessEmpty(stub);

    QSignalSpy spy(&monitor, &MonitorUsb::usbChanged);
    monitor.monitor();

    // Assert
    EXPECT_EQ(1, unrefCalls);
    EXPECT_FALSE(monitor.m_UsbChanged);
    EXPECT_EQ(0, spy.count());
}

// 分支：add 事件 -> 探测设备信息并通知 ControlInterface，缓存标记更新
TEST_F(MonitorUsbTest, Monitor_AddEvent_TriggersHwinfoAndControlNotifies)
{
    // Arrange
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    MonitorUsb monitor;
    int selectCalls = 0;
    stubSelectReady(stub, &monitor, selectCalls);
    int receiveCalls = 0;
    stubUdevDeviceEvent(stub, &monitor, receiveCalls, "usb", 42, "add");
    stub.set_lamda(udev_device_unref, [](struct udev_device *dev) -> struct udev_device * { return dev; });
    int disableOutCalls = 0;
    int updateWakeupCalls = 0;
    stubControlInterface(stub, &disableOutCalls, &updateWakeupCalls);
    stubQProcessEmpty(stub);

    QSignalSpy spy(&monitor, &MonitorUsb::usbChanged);
    monitor.monitor();

    // Assert
    EXPECT_EQ(1, disableOutCalls);   // add 事件额外调用 disableOutDevice
    EXPECT_EQ(1, updateWakeupCalls);
    EXPECT_TRUE(monitor.m_UsbChanged);
    EXPECT_EQ(0, spy.count());       // usbChanged 由 slotTimeout 防抖后发射
}

// 分支：remove 事件 -> 只更新唤醒信息，不调用 disableOutDevice
TEST_F(MonitorUsbTest, Monitor_RemoveEvent_UpdatesCacheWithoutDisableOut)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    MonitorUsb monitor;
    int selectCalls = 0;
    stubSelectReady(stub, &monitor, selectCalls);
    int receiveCalls = 0;
    stubUdevDeviceEvent(stub, &monitor, receiveCalls, "usb", 42, "remove");
    stub.set_lamda(udev_device_unref, [](struct udev_device *dev) -> struct udev_device * { return dev; });
    int disableOutCalls = 0;
    int updateWakeupCalls = 0;
    stubControlInterface(stub, &disableOutCalls, &updateWakeupCalls);
    stubQProcessEmpty(stub);

    monitor.monitor();

    // Assert
    EXPECT_EQ(0, disableOutCalls);
    EXPECT_EQ(1, updateWakeupCalls);
    EXPECT_TRUE(monitor.m_UsbChanged);
}

// 分支：change 事件不在 add/remove 之列 -> 不更新缓存
TEST_F(MonitorUsbTest, Monitor_ChangeEvent_DoesNotUpdateCache)
{
    // Arrange
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    MonitorUsb monitor;
    int selectCalls = 0;
    stubSelectReady(stub, &monitor, selectCalls);
    int receiveCalls = 0;
    stubUdevDeviceEvent(stub, &monitor, receiveCalls, "usb", 42, "change");
    stub.set_lamda(udev_device_unref, [](struct udev_device *dev) -> struct udev_device * { return dev; });
    int disableOutCalls = 0;
    int updateWakeupCalls = 0;
    stubControlInterface(stub, &disableOutCalls, &updateWakeupCalls);
    stubQProcessEmpty(stub);

    monitor.monitor();

    // Assert
    EXPECT_EQ(0, disableOutCalls);
    EXPECT_EQ(0, updateWakeupCalls);
    EXPECT_FALSE(monitor.m_UsbChanged);
}

// ---------- MonitorUsb::setWorkingFlag (monitorusb.cpp:113-117) ----------
TEST_F(MonitorUsbTest, SetWorkingFlag_NewFlagValue_UpdatesWorkingState)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    MonitorUsb monitor;

    monitor.setWorkingFlag(false);
    // Assert
    EXPECT_FALSE(monitor.m_workingFlag);

    monitor.setWorkingFlag(true);
    EXPECT_TRUE(monitor.m_workingFlag);
    EXPECT_FALSE(monitor.m_UsbChanged);   // setWorkingFlag 不改变插拔记录
}

// ---------- MonitorUsb::slotTimeout (monitorusb.cpp:119-128) ----------
// 分支：无未决变更（m_UsbChanged=false）不发射
TEST_F(MonitorUsbTest, SlotTimeout_NoPendingChange_DoesNotEmit)
{
    // Arrange
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    MonitorUsb monitor;

    QSignalSpy spy(&monitor, &MonitorUsb::usbChanged);
    monitor.slotTimeout();

    // Assert
    EXPECT_EQ(0, spy.count());
    EXPECT_FALSE(monitor.m_UsbChanged);   // 无待处理插拔记录
}

// 分支：有未决变更但工作标志已关闭 -> 依旧不发射
TEST_F(MonitorUsbTest, SlotTimeout_WorkingFlagDisabled_DoesNotEmit)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    MonitorUsb monitor;
    monitor.m_UsbChanged = true;
    monitor.setWorkingFlag(false);

    QSignalSpy spy(&monitor, &MonitorUsb::usbChanged);
    monitor.slotTimeout();

    // Assert
    EXPECT_EQ(0, spy.count());
    EXPECT_TRUE(monitor.m_UsbChanged);   // 停止工作时插拔记录保持不清空
}

// 分支：距上次变更不足 1s（防抖窗口内）不发射
TEST_F(MonitorUsbTest, SlotTimeout_WithinDebounceWindow_DoesNotEmit)
{
    // Act
    // Arrange
    // （fixture 已打桩默认环境，无额外前置条件）
    MonitorUsb monitor;
    monitor.m_UsbChanged = true;
    monitor.m_UsbChangeTime = QDateTime::currentMSecsSinceEpoch();

    QSignalSpy spy(&monitor, &MonitorUsb::usbChanged);
    monitor.slotTimeout();

    // Assert
    EXPECT_EQ(0, spy.count());
    EXPECT_TRUE(monitor.m_UsbChanged);   // 防抖窗口内不重置标志
}

// 分支：超过 1s 防抖窗口 -> 发射一次并重置标志，再次调用不再发射
TEST_F(MonitorUsbTest, SlotTimeout_AfterDebounceWindow_EmitsOnceAndResetsFlag)
{
    // Arrange
    // （fixture 已打桩默认环境，无额外前置条件）
    // Act
    MonitorUsb monitor;
    monitor.m_UsbChanged = true;
    monitor.m_UsbChangeTime = QDateTime::currentMSecsSinceEpoch() - 2000;

    QSignalSpy spy(&monitor, &MonitorUsb::usbChanged);
    monitor.slotTimeout();

    // Assert
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(monitor.m_UsbChanged);

    monitor.slotTimeout();
    EXPECT_EQ(1, spy.count());   // 标志已复位，不会重复发射
}

TEST_F(MonitorUsbTest, Ctor_DefaultState_TimerActiveAndFlagsInitialized)
{
    // Arrange
    // （fixture 已打桩默认 udev 环境，无额外前置条件）
    // Act
    MonitorUsb monitor;
    // Assert
    EXPECT_TRUE(monitor.m_workingFlag);           // 构造后处于工作状态
    EXPECT_FALSE(monitor.m_UsbChanged);
    EXPECT_NE(nullptr, monitor.mp_Timer);
    EXPECT_TRUE(monitor.mp_Timer->isActive());    // 1s 轮询定时器已启动
}
