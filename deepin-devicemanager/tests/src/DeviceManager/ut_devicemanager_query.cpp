// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DeviceManager 遍历查找 (deepin-devicemanager/src/DeviceManager/DeviceManager.cpp)
// 覆盖: getMouseDevice/getBluetoothDevice/getAudioDevice/getNetworkDevice/
//       getImageDevice/getOthersDevice/getBluetoothAtIndex/
//       deleteDisableDuplicate_AudioDevice/getCpuHeaderInfo。
// 注入设备到各 list 后查找, 纯逻辑遍历, 无外部依赖。
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                            | level | factors | min | actual |
// |-----------------------------------|-------|---------|-----|--------|
// | getMouseDevice                    | low   | -       | 1   | 1      |
// | getBluetoothDevice                | low   | -       | 1   | 1      |
// | getBluetoothAtIndex               | low   | -       | 1   | 1      |
// | getAudioDevice                    | low   | -       | 1   | 1      |
// | getNetworkDevice                  | low   | -       | 1   | 1      |
// | getImageDevice                    | low   | -       | 1   | 1      |
// | getOthersDevice                   | low   | -       | 1   | 1      |
// | deleteDisableDuplicate_AudioDevice| mid   | -       | 2   | 2      |
// | getCpuHeaderInfo                  | low   | -       | 1   | 1      |
// ============================================================

#include "DeviceManager.h"
#include "DeviceInput.h"
#include "DeviceBluetooth.h"
#include "DeviceAudio.h"
#include "DeviceNetwork.h"
#include "DeviceImage.h"
#include "DeviceOthers.h"
#include "ut_Head.h"

#include <QList>
#include <QPair>
#include <QString>

#include <gtest/gtest.h>

class UT_DeviceManagerQuery : public ::testing::Test
{
public:
    void SetUp() override
    {
        ut_disable_system_dbus();  // 隔离真实系统服务, 避免运行期触发系统总线鉴权
        dm = DeviceManager::instance();
        dm->clear();
        dm->addMouseDevice(new DeviceInput);
        dm->addKeyboardDevice(new DeviceInput);
        dm->addBluetoothDevice(new DeviceBluetooth);
        dm->addAudioDevice(new DeviceAudio);
        dm->addNetworkDevice(new DeviceNetwork);
        dm->addImageDevice(new DeviceImage);
        dm->addOthersDevice(new DeviceOthers);
    }
    void TearDown() override
    {
        dm->clear();
    }
    DeviceManager *dm = nullptr;
};

// getMouseDevice: 不存在的 uid 返回空指针
TEST_F(UT_DeviceManagerQuery, UT_DeviceManagerQuery_getMouseDevice_notFound)
{
    // Arrange: 鼠标列表已有设备但 uid 不匹配
    // Act: 以不存在的 uid 查找
    DeviceBaseInfo *ret = dm->getMouseDevice("nonexist_uid");

    // Assert: 查找失败返回空指针, 且列表本身非空(确认是匹配失败而非无数据)
    EXPECT_EQ(ret, nullptr);
    EXPECT_EQ(1, dm->m_ListDeviceMouse.size());
}

// getBluetoothDevice: 不存在的 uid 返回空指针
TEST_F(UT_DeviceManagerQuery, UT_DeviceManagerQuery_getBluetoothDevice_notFound)
{
    // Arrange: 蓝牙列表已有设备但 uid 不匹配
    // Act: 以不存在的 uid 查找
    DeviceBaseInfo *ret = dm->getBluetoothDevice("nonexist_uid");

    // Assert: 查找失败返回空指针, 且列表本身非空
    EXPECT_EQ(ret, nullptr);
    EXPECT_EQ(1, dm->m_ListDeviceBluetooth.size());
}

// getBluetoothAtIndex: 有效下标返回设备, 越界下标返回空指针
TEST_F(UT_DeviceManagerQuery, UT_DeviceManagerQuery_getBluetoothAtIndex)
{
    // Arrange: 蓝牙列表含 1 个设备
    // Act: 执行被测调用
    // Assert: 下标 0 命中, 越界下标 99 返回空
    EXPECT_NE(dm->getBluetoothAtIndex(0), nullptr);
    EXPECT_EQ(dm->getBluetoothAtIndex(99), nullptr);
    EXPECT_EQ(dm->getBluetoothAtIndex(0), dm->m_ListDeviceBluetooth.at(0));
}

// getAudioDevice: 不存在的路径返回空指针
TEST_F(UT_DeviceManagerQuery, UT_DeviceManagerQuery_getAudioDevice_notFound)
{
    // Arrange: 音频列表已有设备但路径不匹配
    // Act: 以不存在的路径查找
    DeviceBaseInfo *ret = dm->getAudioDevice("/nonexist/path");

    // Assert: 查找失败返回空指针, 且列表本身非空
    EXPECT_EQ(ret, nullptr);
    EXPECT_EQ(1, dm->m_ListDeviceAudio.size());
}

// getNetworkDevice: 不存在的 uid 返回空指针
TEST_F(UT_DeviceManagerQuery, UT_DeviceManagerQuery_getNetworkDevice_notFound)
{
    // Arrange: 网卡列表已有设备但 uid 不匹配
    // Act: 以不存在的 uid 查找
    DeviceBaseInfo *ret = dm->getNetworkDevice("nonexist_uid");

    // Assert: 查找失败返回空指针, 且列表本身非空
    EXPECT_EQ(ret, nullptr);
    EXPECT_EQ(1, dm->m_ListDeviceNetwork.size());
}

// getImageDevice: 不存在的 uid 返回空指针
TEST_F(UT_DeviceManagerQuery, UT_DeviceManagerQuery_getImageDevice_notFound)
{
    // Arrange: 摄像头列表已有设备但 uid 不匹配
    // Act: 以不存在的 uid 查找
    DeviceBaseInfo *ret = dm->getImageDevice("nonexist_uid");

    // Assert: 查找失败返回空指针, 且列表本身非空
    EXPECT_EQ(ret, nullptr);
    EXPECT_EQ(1, dm->m_ListDeviceImage.size());
}

// getOthersDevice: 不存在的 uid 返回空指针
TEST_F(UT_DeviceManagerQuery, UT_DeviceManagerQuery_getOthersDevice_notFound)
{
    // Arrange: 其他设备列表已有设备但 uid 不匹配
    // Act: 以不存在的 uid 查找
    DeviceBaseInfo *ret = dm->getOthersDevice("nonexist_uid");

    // Assert: 查找失败返回空指针, 且列表本身非空
    EXPECT_EQ(ret, nullptr);
    EXPECT_EQ(1, dm->m_ListDeviceOthers.size());
}

// deleteDisableDuplicate_AudioDevice: 单个默认音频设备(空驱动)优先级为 2, 清理后保留
TEST_F(UT_DeviceManagerQuery, UT_DeviceManagerQuery_deleteDisableDuplicate_keepSingle)
{
    // Arrange: 单个默认构造音频设备(先释放 SetUp 注入的设备避免泄漏)
    DeviceAudio *audio = new DeviceAudio;
    qDeleteAll(dm->m_ListDeviceAudio);
    dm->m_ListDeviceAudio.clear();
    dm->m_ListDeviceAudio.append(audio);

    // Act: 执行禁用重复音频清理
    dm->deleteDisableDuplicate_AudioDevice();

    // Assert: 唯一设备(非 USB 禁用场景)被保留, 指针不变
    EXPECT_EQ(1, dm->m_ListDeviceAudio.size());
    EXPECT_EQ(audio, dm->m_ListDeviceAudio.at(0));
}

// deleteDisableDuplicate_AudioDevice: 空列表安全完成
TEST_F(UT_DeviceManagerQuery, UT_DeviceManagerQuery_deleteDisableDuplicate_emptyList)
{
    // Arrange: 清空音频列表(先释放 SetUp 注入的设备避免泄漏)
    qDeleteAll(dm->m_ListDeviceAudio);
    dm->m_ListDeviceAudio.clear();

    // Act: 执行禁用重复音频清理
    dm->deleteDisableDuplicate_AudioDevice();

    // Assert: 空列表遍历安全, 结果仍为空
    EXPECT_EQ(0, dm->m_ListDeviceAudio.size());
    EXPECT_TRUE(dm->m_ListDeviceAudio.isEmpty());
}

// getCpuHeaderInfo: 返回内部 CPU 头信息表的拷贝
TEST_F(UT_DeviceManagerQuery, UT_DeviceManagerQuery_getCpuHeaderInfo)
{
    // Arrange: 注入一行 CPU 头信息
    QList<QPair<QString, QString> > row;
    row.append(QPair<QString, QString>("key", "value"));
    dm->m_ListCpuHeaderInfo.clear();
    dm->m_ListCpuHeaderInfo.append(row);

    // Act: 读取 CPU 头信息
    QList<QList<QPair<QString, QString> > > info;
    dm->getCpuHeaderInfo(info);

    // Assert: 内容与内部表一致
    EXPECT_EQ(1, info.size());
    EXPECT_EQ(row, info.at(0));
}
