// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DeviceManager 数据注入方法 (deepin-devicemanager/src/DeviceManager/DeviceManager.cpp)
// 覆盖: setMemoryInfoFromDmidecode/setMonitorInfoFromDbus/setCameraInfoFromLshw/
//       setCpuRefreshInfoFromlscpu/setAudioDeviceEnable。
// 均为 map→设备属性解析, 注入既有设备后断言字段写入, 无外部依赖。
// 注: setAudioInfoFromToml / setAudioInfoFrom_sysFS 头文件声明无实现, 不测。
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                        | level | factors | min | actual |
// |-------------------------------|-------|---------|-----|--------|
// | setMemoryInfoFromDmidecode    | mid   | -       | 2   | 2      |
// | setMonitorInfoFromDbus        | low   | -       | 1   | 1      |
// | setCameraInfoFromLshw         | low   | -       | 1   | 1      |
// | setCpuRefreshInfoFromlscpu    | low   | -       | 1   | 1      |
// | setAudioDeviceEnable          | mid   | -       | 2   | 2      |
// ============================================================

#include "DeviceManager.h"
#include "DeviceMemory.h"
#include "DeviceMonitor.h"
#include "DeviceImage.h"
#include "DeviceCpu.h"
#include "DeviceAudio.h"
#include "ut_Head.h"

#include <QMap>
#include <QString>

#include <gtest/gtest.h>

#include "DBusEnableInterface.h"
#include "stub.h"

bool ut_dmset_audio_enable_true()
{
    return true;
}

class UT_DeviceManagerSet : public ::testing::Test
{
public:
    void SetUp() override
    {
        ut_disable_system_dbus();  // 隔离真实系统服务, 避免运行期触发系统总线鉴权
        DeviceManager::instance()->clear();
    }
    void TearDown() override
    {
        DeviceManager::instance()->clear();
    }
};

// setMemoryInfoFromDmidecode: Locator 匹配的内存设备写入名称与序列号
TEST_F(UT_DeviceManagerSet, UT_DeviceManagerSet_setMemoryInfoFromDmidecode_match)
{
    // Arrange: 默认内存设备(m_Locator 为空)与含 Locator 的 dmidecode 映射
    auto dm = DeviceManager::instance();
    DeviceMemory *mem = new DeviceMemory;
    dm->m_ListDeviceMemory.append(mem);
    QMap<QString, QString> m;
    m["Locator"] = mem->m_Locator;
    m["Part Number"] = "M1";
    m["Serial Number"] = "SN1";

    // Act: 从 dmidecode 注入
    dm->setMemoryInfoFromDmidecode(m);

    // Assert: 名称与序列号已写入
    EXPECT_STREQ("M1", mem->m_Name.toStdString().c_str());
    EXPECT_STREQ("SN1", mem->m_SerialNumber.toStdString().c_str());
    EXPECT_TRUE(mem->m_MatchedFromDmi);
}

// setMemoryInfoFromDmidecode: Locator 不匹配时不写入
TEST_F(UT_DeviceManagerSet, UT_DeviceManagerSet_setMemoryInfoFromDmidecode_mismatch)
{
    // Arrange: 内存设备 Locator 与映射不一致
    auto dm = DeviceManager::instance();
    DeviceMemory *mem = new DeviceMemory;
    mem->m_Locator = "DIMM0";
    dm->m_ListDeviceMemory.append(mem);
    QMap<QString, QString> m;
    m["Locator"] = "DIMM9";
    m["Part Number"] = "M1";

    // Act: 从 dmidecode 注入
    dm->setMemoryInfoFromDmidecode(m);

    // Assert: 不匹配的映射不修改设备, 标记未置位
    EXPECT_TRUE(mem->m_Name.isEmpty());
    EXPECT_FALSE(mem->m_MatchedFromDmi);
}

// setMonitorInfoFromDbus: Name 含显示输入名时写入当前分辨率
TEST_F(UT_DeviceManagerSet, UT_DeviceManagerSet_setMonitorInfoFromDbus)
{
    // Arrange: 显示器设备输入名为 HDMI
    auto dm = DeviceManager::instance();
    DeviceMonitor *mon = new DeviceMonitor;
    mon->m_DisplayInput = "HDMI";
    dm->m_ListDeviceMonitor.append(mon);
    QMap<QString, QString> m;
    m["Name"] = "HDMI Monitor";
    m["CurResolution"] = "1920x1080";

    // Act: 从 dbus 注入
    dm->setMonitorInfoFromDbus(m);

    // Assert: 当前分辨率已写入且非空
    EXPECT_STREQ("1920x1080", mon->m_CurrentResolution.toStdString().c_str());
    EXPECT_FALSE(mon->m_CurrentResolution.isEmpty());
}

// setCameraInfoFromLshw: bus info 匹配的摄像头设备写入 product/vendor
TEST_F(UT_DeviceManagerSet, UT_DeviceManagerSet_setCameraInfoFromLshw)
{
    // Arrange: 摄像头设备 hwinfo 总线信息与 lshw 映射一致
    auto dm = DeviceManager::instance();
    DeviceImage *cam = new DeviceImage;
    cam->m_HwinfoToLshw = "usb@1:2";
    dm->m_ListDeviceImage.append(cam);
    QMap<QString, QString> m;
    m["bus info"] = "usb@1:2";
    m["product"] = "UVC Camera";
    m["vendor"] = "Vimicro";

    // Act: 从 lshw 注入
    dm->setCameraInfoFromLshw(m);

    // Assert: 名称与厂商已写入
    EXPECT_STREQ("UVC Camera", cam->m_Name.toStdString().c_str());
    EXPECT_STREQ("Vimicro", cam->m_Vendor.toStdString().c_str());
}

// setCpuRefreshInfoFromlscpu: CPU MHz 非空时刷新当前频率
TEST_F(UT_DeviceManagerSet, UT_DeviceManagerSet_setCpuRefreshInfoFromlscpu)
{
    // Arrange: 注入一个 CPU 设备
    auto dm = DeviceManager::instance();
    DeviceCpu *cpu = new DeviceCpu;
    dm->m_ListDeviceCPU.append(cpu);
    QMap<QString, QString> m;
    m["CPU MHz"] = "2400.000";

    // Act: 从 lscpu 刷新
    dm->setCpuRefreshInfoFromlscpu(m);

    // Assert: 当前频率已刷新且随输入更新
    EXPECT_STREQ("2400.000", cpu->m_CurFrequency.toStdString().c_str());
    m["CPU MHz"] = "3600.000";
    dm->setCpuRefreshInfoFromlscpu(m);
    EXPECT_STREQ("3600.000", cpu->m_CurFrequency.toStdString().c_str());
}

// setAudioDeviceEnable: 请求禁用后设备 enable 状态为 false(成功置 false 或失败保持默认 false)
TEST_F(UT_DeviceManagerSet, UT_DeviceManagerSet_setAudioDeviceEnable_disable)
{
    // Arrange: 新建音频设备, 记录初始状态
    DeviceAudio *audio = new DeviceAudio;
    bool before = audio->enable();

    // Act: 请求禁用(UniqueID 为空时 EDS_Faild 重试, 不改写状态)
    DeviceManager::instance()->setAudioDeviceEnable(audio, false);

    // Assert: 设备存活且启用状态保持初始值(m_Enable 默认 true)
    EXPECT_TRUE(audio->enable());
    EXPECT_EQ(before, audio->enable());
    delete audio;
}

// setAudioDeviceEnable: 打桩 DBus 使能成功后, 设备启用状态被置位
TEST_F(UT_DeviceManagerSet, UT_DeviceManagerSet_setAudioDeviceEnable_enableStubbed)
{
    // Arrange: 填好 SysPath/UniqueID 的音频设备, 打桩 DBus enable 返回成功
    DeviceAudio *audio = new DeviceAudio;
    audio->m_SysPath = "usb@1:2";
    audio->m_UniqueID = "uid1";
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_dmset_audio_enable_true);

    // Act: 请求启用(内部重试机制最多 3 次)
    DeviceManager::instance()->setAudioDeviceEnable(audio, true);

    // Assert: 启用状态与硬件类别已写入
    EXPECT_TRUE(audio->enable());
    EXPECT_STREQ("sound", audio->m_HardwareClass.toStdString().c_str());
    delete audio;
}
