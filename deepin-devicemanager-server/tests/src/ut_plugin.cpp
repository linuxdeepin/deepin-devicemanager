// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * 用例计数声明（method | level | factors | min | actual）
 * | DSMRegister | low | - | 1 | 2 |
 * | DSMUnRegister | low | - | 1 | 1 |
 */

#include <gtest/gtest.h>

#include <QApplication>
#include <QDBusConnection>
#include <QProcess>

#include <libudev.h>

#include "stubext.h"
#include "mainjob.h"
#include "controlinterface.h"

using namespace stub_ext;

// plugin.cpp 未提供头文件，此处按源码声明插件入口（extern "C"）
extern "C" {
int DSMRegister(const char *name, void *data);
int DSMUnRegister(const char *name, void *data);
}

class PluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        registered = false;
        stubDependencies();
    }

    void TearDown() override
    {
        // 先在本测试内注销，随后刷新事件队列让 deleteLater 落地
        // 注意：processEvents 必须在 stub.clear() 之前执行，
        // 这样 MainJob 延迟 1s 的初始化 lambda 若被触发，其内部
        // initDriverRepoSource / sqlCopytoKernel 仍处于打桩状态，不会写真实仓库配置
        if (registered) {
            DSMUnRegister("org.deepin.DeviceInfo.ut", nullptr);
            registered = false;
            QApplication::processEvents();
        }
        stub.clear();
    }

    // MainJob 构造链上的全部外部依赖打桩：hwinfo 探测、udev 监听、DBus、驱动仓库写操作
    void stubDependencies()
    {
        // hwinfo 探测（DetectThread 构造会执行一次）
        stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                       [](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) {});
        stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                       [](QProcess *, int) -> bool { return true; });
        stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                       [](QProcess *) -> QByteArray { return QByteArray(); });

        // udev 环境（MonitorUsb 构造依赖）
        stub.set_lamda(udev_new,
                       []() -> struct udev * { return reinterpret_cast<struct udev *>(0x1001); });
        stub.set_lamda(udev_monitor_new_from_netlink,
                       [](struct udev *, const char *) -> struct udev_monitor * {
                           return reinterpret_cast<struct udev_monitor *>(0x2002);
                       });
        stub.set_lamda(udev_monitor_filter_add_match_subsystem_devtype,
                       [](struct udev_monitor *, const char *, const char *) -> int { return 0; });
        stub.set_lamda(udev_monitor_enable_receiving,
                       [](struct udev_monitor *) -> int { return 0; });
        stub.set_lamda(udev_monitor_get_fd, [](struct udev_monitor *) -> int { return 100; });

        // 系统总线与监控开关：不依赖真实 DBus 服务
        stub.set_lamda(static_cast<QDBusConnection (*)()>(&QDBusConnection::systemBus),
                       []() -> QDBusConnection { return QDBusConnection(QString()); });
        stub.set_lamda(static_cast<bool (ControlInterface::*)()>(&ControlInterface::monitorWorkingDBFlag),
                       [](ControlInterface *) -> bool { return false; });

        // MainJob 初始化路径中的重操作打桩：设备加载与延迟加载的驱动仓库/sql 操作
        stub.set_lamda(static_cast<void (MainJob::*)()>(&MainJob::updateAllDevice),
                       [this](MainJob *) { ++updateAllDeviceCalls; });
        stub.set_lamda(static_cast<void (MainJob::*)()>(&MainJob::initDriverRepoSource),
                       [](MainJob *) {});
        stub.set_lamda(static_cast<void (MainJob::*)()>(&MainJob::sqlCopytoKernel),
                       [](MainJob *) {});
    }

    stub_ext::StubExt stub;
    int updateAllDeviceCalls = 0;
    bool registered = false;
};

// ---------- DSMRegister (plugin.cpp:14-20) ----------
// 行为：创建 MainJob（其中会加载一次全量设备信息）并返回 0
TEST_F(PluginTest, DSMRegister_ValidServiceName_CreatesMainJobAndReturnsZero)
{
    // Arrange
    updateAllDeviceCalls = 0;

    // Act
    int ret = DSMRegister("org.deepin.DeviceInfo.ut", nullptr);
    registered = true;

    // Assert
    EXPECT_EQ(0, ret);
    EXPECT_EQ(1, updateAllDeviceCalls);   // MainJob 构造中执行了一次全量加载
}

// name 为空时同样正常注册，不崩溃
TEST_F(PluginTest, DSMRegister_NullName_StillReturnsZero)
{
    // Act
    // Arrange
    // （fixture 已打桩默认环境，无额外前置条件）
    int ret = DSMRegister(nullptr, nullptr);
    registered = true;

    // Assert
    EXPECT_EQ(0, ret);
    EXPECT_EQ(1, updateAllDeviceCalls);
}

// ---------- DSMUnRegister (plugin.cpp:22-30) ----------
// 行为：注销返回 0（deleteLater 需要事件循环处理，TearDown 中已刷新）
TEST_F(PluginTest, DSMUnRegister_AfterRegister_ReturnsZero)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Assert
    // Arrange
    ASSERT_EQ(0, DSMRegister("org.deepin.DeviceInfo.ut", nullptr));
    updateAllDeviceCalls = 0;

    int ret = DSMUnRegister("org.deepin.DeviceInfo.ut", nullptr);

    EXPECT_EQ(0, ret);
    // 注销后不再触发新的设备加载
    EXPECT_EQ(0, updateAllDeviceCalls);
}
