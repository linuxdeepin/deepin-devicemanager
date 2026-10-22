// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DeviceManager (deepin-devicemanager/src/DeviceManager/DeviceManager.cpp)
// 覆盖用例与被测方法对照表:
//   clear()                          -> UT_DeviceManager_clear
//   getDeviceTypes()                 -> UT_DeviceManager_getDeviceTypes
//   setDeviceListClass()             -> UT_DeviceManager_setDeviceListClass
//   getDeviceList()                  -> UT_DeviceManager_getDeviceList_001/002
//   addMouseDevice()                 -> UT_DeviceManager_addMouseDevice
//   addMouseInfoFromLshw()           -> UT_DeviceManager_addMouseInfoFromLshw_001/002
//   addCpuDevice()                   -> UT_DeviceManager_addCpuDevice
//   addStorageDeivce()               -> UT_DeviceManager_addStorageDeivce
//   addLshwinfoIntoStorageDevice()   -> UT_DeviceManager_addLshwinfoIntoStorageDevice
//   addLshwinfoIntoNVMEStorageDevice()-> UT_DeviceManager_addLshwinfoIntoNVMEStorageDevice
//   setStorageInfoFromSmartctl()     -> UT_DeviceManager_setStorageInfoFromSmartctl
//   setStorageDeviceMediaType()      -> UT_DeviceManager_setStorageDeviceMediaType
//   addGpuDevice()                   -> UT_DeviceManager_addGpuDevice
//   setGpuInfoFromXrandr()           -> UT_DeviceManager_setGpuInfoFromXrandr
//   setGpuInfoFromLshw()             -> UT_DeviceManager_setGpuInfoFromLshw
//   networkDriver()                  -> UT_DeviceManager_networkDriver
//   correctPowerInfo()               -> UT_DeviceManager_correctPowerInfo
//   setGpuSizeFromDmesg()            -> UT_DeviceManager_setGpuSizeFromDmesg
//   addMemoryDevice()                -> UT_DeviceManager_addMemoryDevice
//   addMonitor()                     -> UT_DeviceManager_addMonitor
//   setMonitorInfoFromXrandr()       -> UT_DeviceManager_setMonitorInfoFromXrandr
//   addBiosDevice()                  -> UT_DeviceManager_addBiosDevice
//   setLanguageInfo()                -> UT_DeviceManager_setLanguageInfo
//   addBluetoothDevice()             -> UT_DeviceManager_addBluetoothDevice
//   setBluetoothInfoFromHwinfo()     -> UT_DeviceManager_setBluetoothInfoFromHwinfo
//   setBluetoothInfoFromLshw()       -> UT_DeviceManager_setBluetoothInfoFromLshw
//   addAudioDevice()                 -> UT_DeviceManager_addAudioDevice
//   setAudioInfoFromLshw()           -> UT_DeviceManager_setAudioInfoFromLshw
//   setAudioChipFromDmesg()          -> UT_DeviceManager_setAudioChipFromDmesg
//   addNetworkDevice()               -> UT_DeviceManager_addNetworkDevice
//   addImageDevice()                 -> UT_DeviceManager_addImageDevice
//   addKeyboardDevice()              -> UT_DeviceManager_addKeyboardDevice
//   setKeyboardInfoFromLshw()        -> UT_DeviceManager_setKeyboardInfoFromLshw
//   correctNetworkLinkStatus()       -> UT_DeviceManager_correctNetworkLinkStatus
//   addOthersDevice()                -> UT_DeviceManager_addOthersDevice
//   setOthersDeviceInfoFromLshw()    -> UT_DeviceManager_setOthersDeviceInfoFromLshw
//   addPowerDevice()                 -> UT_DeviceManager_addPowerDevice
//   addPrintDevice()                 -> UT_DeviceManager_addPrintDevice
//   addOtherPCIDevice()              -> UT_DeviceManager_addOtherPCIDevice
//   addComputerDevice()              -> UT_DeviceManager_addComputerDevice
//   addCdromDevice()                 -> UT_DeviceManager_addCdromDevice
//   addLshwinfoIntoCdromDevice()     -> UT_DeviceManager_addLshwinfoIntoCdromDevice
//   addBusId()                       -> UT_DeviceManager_addBusId
//   getBusId()                       -> UT_DeviceManager_getBusId
//   addCmdInfo()                     -> UT_DeviceManager_addCmdInfo_001/002
//   cmdInfo()                        -> UT_DeviceManager_cmdInfo (依赖 addCmdInfo_002 的状态)
//   getDeviceOverview()              -> UT_DeviceManager_getDeviceOverview
//   getDeviceDriverPool()            -> UT_DeviceManager_getDeviceDriverPool
//   addInputInfo()                   -> UT_DeviceManager_addInputInfo
//   inputInfo()                      -> UT_DeviceManager_inputInfo
//   isDeviceExistInPairedDevice()    -> UT_DeviceManager_isDeviceExistInPairedDevice_001/002
//   setCpuNum()                      -> UT_DeviceManager_setCpuNum
// 说明: DeviceManager::clear() 会 delete 全部 m_ListDevice* 中指针,
//       使用 clear() 清理后严禁再手动 delete, 防止双重释放。
// ============================================================

#include "DeviceManager.h"
#include "DeviceInput.h"
#include "DeviceCpu.h"
#include "DeviceGpu.h"
#include "DeviceStorage.h"
#include "DeviceMemory.h"
#include "DeviceMonitor.h"
#include "DeviceBios.h"
#include "DeviceBluetooth.h"
#include "DeviceAudio.h"
#include "DeviceNetwork.h"
#include "DeviceImage.h"
#include "DeviceOthers.h"
#include "DevicePower.h"
#include "DevicePrint.h"
#include "DeviceOtherPCI.h"
#include "DeviceComputer.h"
#include "DeviceCdrom.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QIODevice>

#include <gtest/gtest.h>

class UT_DeviceManager : public ::testing::Test
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

TEST_F(UT_DeviceManager, UT_DeviceManager_clear)
{
    // Arrange: 清空单例, 确保初始状态无残留设备
    DeviceManager::instance()->clear();

    // Act: 再次调用 clear(), 验证空状态下重复清理不崩溃
    DeviceManager::instance()->clear();

    // Assert: 全部设备列表与命令缓存均为空
    EXPECT_EQ(0, DeviceManager::instance()->m_cmdInfo.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceMouse.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceStorage.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceMonitor.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceBluetooth.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceAudio.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceImage.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceKeyboard.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceOthers.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDevicePower.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDevicePrint.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceOtherPCI.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceCdrom.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceComputer.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceNetwork.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceBios.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceGPU.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceMemory.size());
    EXPECT_EQ(0, DeviceManager::instance()->m_ListDeviceCPU.size());
}


TEST_F(UT_DeviceManager, UT_DeviceManager_getDeviceTypes)
{
    // Arrange: 清空设备管理器, 添加一个 CPU 设备
    DeviceManager *dm = DeviceManager::instance();
    dm->clear();
    dm->m_CpuNum = 0;
    DeviceCpu *cpu = new DeviceCpu;
    dm->addCpuDevice(cpu);

    // Act: 获取设备类型导航列表
    const QList<QPair<QString, QString>> &types = dm->getDeviceTypes();

    // Assert: 概况 + CPU + 分隔符 共三项, 各项跳转链接正确
    EXPECT_EQ(3, types.size());
    EXPECT_STREQ("overview##Overview", types.at(0).second.toStdString().c_str());
    EXPECT_STREQ("cpu##CPU", types.at(1).second.toStdString().c_str());
    EXPECT_STREQ("Separator##Separator", types.at(2).second.toStdString().c_str());

    // Act2: 多核 CPU(m_CpuNum>1) 时重新获取
    dm->m_CpuNum = 2;
    const QList<QPair<QString, QString>> &types2 = dm->getDeviceTypes();

    // Assert2: 追加 "CPU quantity" 项(共四项), 该项无跳转链接
    EXPECT_EQ(4, types2.size());
    EXPECT_STREQ("", types2.at(2).second.toStdString().c_str());

    // Cleanup: clear() 负责释放 cpu 指针, 不可再手动 delete
    dm->m_CpuNum = 0;
    dm->clear();
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setDeviceListClass)
{
    // Arrange: 清空单例, 确保类别映射从零开始构建
    DeviceManager::instance()->clear();

    // Act: 构建 17 类设备类别映射
    DeviceManager::instance()->setDeviceListClass();

    // Assert: 映射大小为 17, 且包含 CPU 类别键
    EXPECT_EQ(17, DeviceManager::instance()->m_DeviceClassMap.size());
    EXPECT_TRUE(DeviceManager::instance()->m_DeviceClassMap.contains(QObject::tr("CPU")));

    // Cleanup
    DeviceManager::instance()->m_DeviceClassMap.clear();
}

TEST_F(UT_DeviceManager, UT_DeviceManager_getDeviceList_001)
{
    // Arrange: 空的类别映射与空结果列表
    QList<DeviceBaseInfo *> lst;

    // Act: 以 "Overview" 类别查询设备列表
    bool ret = DeviceManager::instance()->getDeviceList(QObject::tr("Overview"), lst);

    // Assert: 概况类别不返回设备列表, 结果列表保持为空
    EXPECT_FALSE(ret);
    EXPECT_TRUE(lst.isEmpty());
}

TEST_F(UT_DeviceManager, UT_DeviceManager_getDeviceList_002)
{
    // Arrange: 向类别映射注入两个输入设备
    DeviceInput *device1 = new DeviceInput;
    DeviceInput *device2 = new DeviceInput;
    QList<DeviceBaseInfo *> lstInfo;
    lstInfo.append(device1);
    lstInfo.append(device2);
    DeviceManager::instance()->m_DeviceClassMap.insert("input", lstInfo);

    // Act: 以 "over"(截断匹配) 查询设备列表
    bool ret = DeviceManager::instance()->getDeviceList("over", lstInfo);

    // Assert: 前缀匹配命中, 返回全部两个设备
    EXPECT_EQ(2, lstInfo.size());
    EXPECT_TRUE(ret);

    // Cleanup
    DeviceManager::instance()->m_DeviceClassMap.clear();
    delete device1;
    delete device2;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addMouseDevice)
{
    // Arrange: 构造鼠标设备
    DeviceInput *device = new DeviceInput;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addMouseDevice(device);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceMouse.size());
    EXPECT_EQ(device, DeviceManager::instance()->m_ListDeviceMouse.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceMouse.clear();
    delete device;
}

void ut_manager_setLshwMap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("bus info", "usb@1:8");
    mapinfo.insert("vendor", "Cherry Zhuhai");
    mapinfo.insert("version", "1.07");
    mapinfo.insert("capabilities", "usb-2.00");
    mapinfo.insert("description", "Keyboard");
    mapinfo.insert("driver", "usbhid");
    mapinfo.insert("maxpower", "350mA");
    mapinfo.insert("speed", "12Mbit/s");
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addMouseInfoFromLshw_001)
{
    // Arrange: 鼠标设备 businfo 匹配键与 lshw "bus info" 一致
    DeviceManager::instance()->m_ListDeviceMouse.clear();
    DeviceInput *device = new DeviceInput;
    device->m_HwinfoToLshw = "usb@1:8";
    DeviceManager::instance()->m_ListDeviceMouse.append(device);

    QMap<QString, QString> mapinfo;
    ut_manager_setLshwMap(mapinfo);

    // Act: 从 lshw 匹配填充鼠标信息
    bool ret = DeviceManager::instance()->addMouseInfoFromLshw(mapinfo);

    // Assert: 匹配成功, 厂商与版本号已被填充
    EXPECT_TRUE(ret);
    EXPECT_STREQ("Cherry Zhuhai", device->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("1.07", device->m_Version.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceMouse.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addMouseInfoFromLshw_002)
{
    // Arrange: 鼠标列表为空, 无可匹配设备
    DeviceManager::instance()->m_ListDeviceMouse.clear();
    QMap<QString, QString> mapinfo;
    ut_manager_setLshwMap(mapinfo);

    // Act: 从 lshw 匹配填充鼠标信息
    bool ret = DeviceManager::instance()->addMouseInfoFromLshw(mapinfo);

    // Assert: 无设备可匹配, 返回 false
    EXPECT_FALSE(ret);
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceMouse.isEmpty());
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addCpuDevice)
{
    // Arrange: 构造 CPU 设备
    DeviceCpu *cpu = new DeviceCpu;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addCpuDevice(cpu);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceCPU.size());
    EXPECT_EQ(cpu, DeviceManager::instance()->m_ListDeviceCPU.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceCPU.clear();
    delete cpu;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addStorageDeivce)
{
    // Arrange: 构造存储设备
    DeviceStorage *st = new DeviceStorage;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addStorageDeivce(st);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceStorage.size());
    EXPECT_EQ(st, DeviceManager::instance()->m_ListDeviceStorage.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete st;
}

void ut_manager_setLshwstorage(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("bus info", "scsi@2:0.0.0");
    mapinfo.insert("interface", "sata");
    mapinfo.insert("capabilities", "gpt-1.00 partitioned partitioned:gpt");
    mapinfo.insert("version", "R013");
    mapinfo.insert("serial", "2002E3E0B393");
    mapinfo.insert("product", "CT240BX500SSD1");
    mapinfo.insert("description", "ATA Disk");
    mapinfo.insert("size", "223GiB (240GB)");
    mapinfo.insert("vendor", "vendor");
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addLshwinfoIntoStorageDevice)
{
    // Arrange: 存储设备匹配键为 scsi 通道号
    DeviceStorage *device = new DeviceStorage;
    device->m_KeyToLshw = "2:0:0:0";
    DeviceManager::instance()->m_ListDeviceStorage.append(device);

    QMap<QString, QString> mapinfo;
    ut_manager_setLshwstorage(mapinfo);

    // Act: 从 lshw 填充存储设备信息
    DeviceManager::instance()->addLshwinfoIntoStorageDevice(mapinfo);

    // Assert: 接口类型与存储通道键均被正确设置
    EXPECT_STREQ("sata", device->m_Interface.toStdString().c_str());
    EXPECT_STREQ("scsi2", device->m_KeyFromStorage.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addLshwinfoIntoNVMEStorageDevice)
{
    // Arrange: NVME 存储设备, 匹配键为 sysfs 路径
    DeviceStorage *device = new DeviceStorage;
    device->m_NvmeKey = "/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0.0.0";
    DeviceManager::instance()->m_ListDeviceStorage.append(device);

    QMap<QString, QString> mapinfo;
    ut_manager_setLshwstorage(mapinfo);

    // Act: 从 lshw 填充 NVME 存储设备信息
    DeviceManager::instance()->addLshwinfoIntoNVMEStorageDevice(mapinfo);

    // Assert: 厂商被填充, 且设备指针保留在管理器列表中
    EXPECT_STREQ("vendor", device->m_Vendor.toStdString().c_str());
    EXPECT_EQ(device, DeviceManager::instance()->m_ListDeviceStorage.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

void ut_manager_setSmartCtlInfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Firmware Version", "M6CR013");
    mapinfo.insert("SATA Version is", "SATA 3.2, 6.0 Gb/s (current: 6.0 Gb/s)");
    mapinfo.insert("Rotation Rate", "Solid State Device");
    mapinfo.insert("Power_On_Hours", "3717");
    mapinfo.insert("Power_Cycle_Count", "306");
    mapinfo.insert("User Capacity", "240,057,409,536 bytes [240 GB]");
    mapinfo.insert("Device Model", "CT240BX500SSD1");
    mapinfo.insert("Serial Number", "2002E3E0B393");
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setStorageInfoFromSmartctl)
{
    // Arrange: 存储设备文件为 /dev/sda
    DeviceStorage *device = new DeviceStorage;
    device->m_DeviceFile = "/dev/sda";
    DeviceManager::instance()->m_ListDeviceStorage.append(device);

    QMap<QString, QString> mapinfo;
    ut_manager_setSmartCtlInfo(mapinfo);

    // Act: 从 smartctl 输出填充存储设备信息
    DeviceManager::instance()->setStorageInfoFromSmartctl("sda", mapinfo);

    // Assert: 固件版本被填充, 设备序列号同步更新
    EXPECT_STREQ("M6CR013", device->m_FirmwareVersion.toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", device->m_SerialNumber.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setStorageDeviceMediaType)
{
    // Arrange: 存储设备文件为 /dev/sda
    DeviceStorage *device = new DeviceStorage;
    device->m_DeviceFile = "/dev/sda";
    DeviceManager::instance()->m_ListDeviceStorage.append(device);

    // Act: 按介质类型码 1 设置介质类型
    DeviceManager::instance()->setStorageDeviceMediaType("sda", "1");

    // Assert: 介质类型映射为 HDD, 类型标识非空
    EXPECT_STREQ("HDD", device->m_MediaType.toStdString().c_str());
    EXPECT_FALSE(device->m_MediaType.isEmpty());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceStorage.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addGpuDevice)
{
    // Arrange: 构造显卡设备
    DeviceGpu *gpu = new DeviceGpu;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addGpuDevice(gpu);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceGPU.size());
    EXPECT_EQ(gpu, DeviceManager::instance()->m_ListDeviceGPU.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceGPU.clear();
    delete gpu;
}

void ut_manager_setgpuxrandrmap(QMap<QString, QString> &mapInfo)
{
    mapInfo.insert("minResolution", "minResolution");
    mapInfo.insert("curResolution", "curResolution");
    mapInfo.insert("maxResolution", "maxResolution");
    mapInfo.insert("HDMI", "Unable");
    mapInfo.insert("VGA", "Unable");
    mapInfo.insert("DP", "Unable");
    mapInfo.insert("eDP", "Unable");
    mapInfo.insert("DVI", "Unable");
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setGpuInfoFromXrandr)
{
    // Arrange: 构造 xrandr 输出映射
    QMap<QString, QString> mapinfo;
    ut_manager_setgpuxrandrmap(mapinfo);

    DeviceGpu *gpu = new DeviceGpu;
    DeviceManager::instance()->m_ListDeviceGPU.append(gpu);

    // Act: 从 xrandr 填充显卡信息
    DeviceManager::instance()->setGpuInfoFromXrandr(mapinfo);

    // Assert: 最小分辨率与最大分辨率均被填充
    EXPECT_STREQ("minResolution", gpu->m_MinimumResolution.toStdString().c_str());
    EXPECT_STREQ("maxResolution", gpu->m_MaximumResolution.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceGPU.clear();
    delete gpu;
}

void ut_manager_setgpuLshwInfo(QMap<QString, QString> &mapLshw)
{
    mapLshw.insert("product", "GK208B [GeForce GT 730]");
    mapLshw.insert("vendor", "NVIDIA Corporation");
    mapLshw.insert("bus info", "pci@0000:01:00.0");
    mapLshw.insert("version", "a1");
    mapLshw.insert("width", "64 bits");
    mapLshw.insert("clock", "33MHz");
    mapLshw.insert("irq", "126");
    mapLshw.insert("capabilities", "pm msi pciexpress vga_controller bus_master cap_list rom");
    mapLshw.insert("description", "VGA compatible controller");
    mapLshw.insert("driver", "nouveau");
    mapLshw.insert("ioport", "e000(size=128)");
    mapLshw.insert("memory", "de000000-deffffff  d0000000-d7ffffff  d8000000-d9ffffff  c0000-dffff");
    mapLshw.insert("physical id", "0");
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setGpuInfoFromLshw)
{
    // Arrange: 显卡匹配键为 PCI 总线地址
    QMap<QString, QString> mapinfo;
    ut_manager_setgpuLshwInfo(mapinfo);

    DeviceGpu *gpu = new DeviceGpu;
    gpu->m_HwinfoToLshw = "0000:01:00.0";
    DeviceManager::instance()->m_ListDeviceGPU.append(gpu);

    // Act: 从 lshw 填充显卡信息
    DeviceManager::instance()->setGpuInfoFromLshw(mapinfo);

    // Assert: 设备名与厂商均被填充
    EXPECT_STREQ("GK208B [GeForce GT 730]", gpu->m_Name.toStdString().c_str());
    EXPECT_STREQ("NVIDIA Corporation", gpu->m_Vendor.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceGPU.clear();
    delete gpu;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_networkDriver)
{
    // Arrange: 网卡列表为空
    DeviceManager::instance()->m_ListDeviceNetwork.clear();

    // Act: 获取全部网卡逻辑名称
    QStringList ret = DeviceManager::instance()->networkDriver();

    // Assert: 空列表返回空结果
    EXPECT_EQ(0, ret.size());
    EXPECT_TRUE(ret.isEmpty());
}

TEST_F(UT_DeviceManager, UT_DeviceManager_correctPowerInfo)
{
    // Arrange: 电源设备与 upower 命令输出
    QMap<QString, QString> map;
    map.insert("name", "name");
    map.insert("Device", "Device");
    map.insert("serial", "serial");
    QMap<QString, QMap<QString, QString>> info;
    info.insert("upower", map);

    DevicePower *p = new DevicePower;
    DeviceManager::instance()->m_ListDevicePower.append(p);

    // Act: 用 upower 输出校正电源信息
    DeviceManager::instance()->correctPowerInfo(info);

    // Assert: 序列号被填充, 未提供的字段(厂商/型号)保持空
    EXPECT_STREQ("serial", p->m_SerialNumber.toStdString().c_str());
    EXPECT_TRUE(p->m_Vendor.isEmpty());
    EXPECT_TRUE(p->m_Model.isEmpty());

    // Cleanup
    DeviceManager::instance()->m_ListDevicePower.clear();
    delete p;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setGpuSizeFromDmesg)
{
    // Arrange: 显卡匹配键为 PCI 地址, dmesg 中带显存大小
    DeviceGpu *gpu = new DeviceGpu;
    gpu->m_HwinfoToLshw = "01:00.0";
    DeviceManager::instance()->m_ListDeviceGPU.append(gpu);

    QMap<QString, QString> dmesgInfoMap;
    dmesgInfoMap.insert("Size", "01:00.0=2GB");

    // Act: 从 dmesg 填充显存大小
    DeviceManager::instance()->setGpuSizeFromDmesg(dmesgInfoMap);

    // Assert: 显存大小被填充且非空
    EXPECT_STREQ("2GB", gpu->m_GraphicsMemory.toStdString().c_str());
    EXPECT_FALSE(gpu->m_GraphicsMemory.isEmpty());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceGPU.clear();
    delete gpu;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addMemoryDevice)
{
    // Arrange: 构造内存设备
    DeviceMemory *m = new DeviceMemory;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addMemoryDevice(m);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceMemory.size());
    EXPECT_EQ(m, DeviceManager::instance()->m_ListDeviceMemory.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceMemory.clear();
    delete m;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addMonitor)
{
    // Arrange: 构造显示器设备
    DeviceMonitor *m = new DeviceMonitor;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addMonitor(m);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceMonitor.size());
    EXPECT_EQ(m, DeviceManager::instance()->m_ListDeviceMonitor.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceMonitor.clear();
    delete m;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setMonitorInfoFromXrandr)
{
    // Arrange: 显示器带生产周期, 构造 xrandr 主屏与 EDID 输出
    DeviceMonitor *monitor = new DeviceMonitor;
    DeviceManager::instance()->addMonitor(monitor);
    monitor->m_Interface = "";
    monitor->m_ProductionWeek = "2020-03";
    QString main = "HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";
    QString edid = "00ffffffffffff005a63384001010101\n0d1e010380351d782ece65a657519f27\n";
    QString rate = "60.00Hz";

    // Act: 从 xrandr 填充显示器信息
    DeviceManager::instance()->setMonitorInfoFromXrandr(main, edid, rate);

    // Assert: 当前分辨率与刷新率组合正确
    EXPECT_STREQ("1920x1080@60.00Hz", monitor->m_CurrentResolution.toStdString().c_str());
    EXPECT_FALSE(monitor->m_CurrentResolution.isEmpty());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceMonitor.clear();
    delete monitor;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addBiosDevice)
{
    // Arrange: 构造主板(BIOS) 设备
    DeviceBios *bios = new DeviceBios;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addBiosDevice(bios);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceBios.size());
    EXPECT_EQ(bios, DeviceManager::instance()->m_ListDeviceBios.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceBios.clear();
    delete bios;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setLanguageInfo)
{
    // Arrange: 主板设备与不匹配的语言描述格式
    DeviceBios *bios = new DeviceBios;
    DeviceManager::instance()->m_ListDeviceBios.append(bios);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("Language Description Format", "language");

    // Act: 设置语言信息
    DeviceManager::instance()->setLanguageInfo(mapinfo);

    // Assert: 格式不匹配时不追加其他信息, 列表保持为空
    EXPECT_EQ(0, bios->m_LstOtherInfo.size());
    EXPECT_TRUE(bios->m_LstOtherInfo.isEmpty());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceBios.clear();
    delete bios;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addBluetoothDevice)
{
    // Arrange: 构造蓝牙设备
    DeviceBluetooth *b = new DeviceBluetooth;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addBluetoothDevice(b);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceBluetooth.size());
    EXPECT_EQ(b, DeviceManager::instance()->m_ListDeviceBluetooth.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceBluetooth.clear();
    delete b;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setBluetoothInfoFromHwinfo)
{
    // Arrange: 蓝牙设备带厂商, hwinfo 输出含版本号
    DeviceBluetooth *bth = new DeviceBluetooth;
    bth->m_Vendor = "Vendor";
    DeviceManager::instance()->m_ListDeviceBluetooth.append(bth);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("Vendor", "Vendor");
    mapinfo.insert("Revision", "Revision");

    // Act: 从 hwinfo 填充蓝牙信息
    DeviceManager::instance()->setBluetoothInfoFromHwinfo(mapinfo);

    // Assert: 版本号被填充, 厂商保持一致
    EXPECT_STREQ("Revision", bth->m_Version.toStdString().c_str());
    EXPECT_STREQ("Vendor", bth->m_Vendor.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceBluetooth.clear();
    delete bth;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setBluetoothInfoFromLshw)
{
    // Arrange: 蓝牙设备匹配键与 lshw "bus info" 一致
    DeviceBluetooth *bth = new DeviceBluetooth;
    bth->m_HwinfoToLshw = "unique";
    DeviceManager::instance()->m_ListDeviceBluetooth.append(bth);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "unique");
    mapinfo.insert("vendor", "Vendor");

    // Act: 从 lshw 填充蓝牙信息
    DeviceManager::instance()->setBluetoothInfoFromLshw(mapinfo);

    // Assert: 厂商被填充, 总线信息同步更新
    EXPECT_STREQ("Vendor", bth->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("unique", bth->m_BusInfo.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceBluetooth.clear();
    delete bth;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addAudioDevice)
{
    // Arrange: 构造音频设备
    DeviceAudio *a = new DeviceAudio;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addAudioDevice(a);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceAudio.size());
    EXPECT_EQ(a, DeviceManager::instance()->m_ListDeviceAudio.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceAudio.clear();
    delete a;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setAudioInfoFromLshw)
{
    // Arrange: 音频设备匹配键为 PCI 地址, lshw "bus info" 带 pci@ 前缀
    DeviceAudio *audio = new DeviceAudio;
    audio->m_HwinfoToLshw = "0000:00:1f.3";
    DeviceManager::instance()->m_ListDeviceAudio.append(audio);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "pci@0000:00:1f.3");
    mapinfo.insert("product", "Cannon Lake PCH cAVS");
    mapinfo.insert("vendor", "Intel Corporation");

    // Act: 从 lshw 填充音频设备信息
    DeviceManager::instance()->setAudioInfoFromLshw(mapinfo);

    // Assert: 设备名与厂商均被填充
    EXPECT_STREQ("Cannon Lake PCH cAVS", audio->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", audio->m_Vendor.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceAudio.clear();
    delete audio;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setAudioChipFromDmesg)
{
    // Arrange: 音频设备加入管理器
    DeviceAudio *audio = new DeviceAudio;
    DeviceManager::instance()->m_ListDeviceAudio.append(audio);

    // Act: 从 dmesg 填充音频芯片型号
    DeviceManager::instance()->setAudioChipFromDmesg("st");

    // Assert: 芯片型号被填充且非空
    EXPECT_STREQ("st", audio->m_Chip.toStdString().c_str());
    EXPECT_FALSE(audio->m_Chip.isEmpty());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceAudio.clear();
    delete audio;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addNetworkDevice)
{
    // Arrange: 构造网卡设备
    DeviceNetwork *n = new DeviceNetwork;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addNetworkDevice(n);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceNetwork.size());
    EXPECT_EQ(n, DeviceManager::instance()->m_ListDeviceNetwork.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceNetwork.clear();
    delete n;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addImageDevice)
{
    // Arrange: 构造摄像头设备
    DeviceImage *image = new DeviceImage;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addImageDevice(image);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceImage.size());
    EXPECT_EQ(image, DeviceManager::instance()->m_ListDeviceImage.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceImage.clear();
    delete image;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addKeyboardDevice)
{
    // Arrange: 构造键盘设备
    DeviceInput *device = new DeviceInput;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addKeyboardDevice(device);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceKeyboard.size());
    EXPECT_EQ(device, DeviceManager::instance()->m_ListDeviceKeyboard.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceKeyboard.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setKeyboardInfoFromLshw)
{
    // Arrange: 键盘设备匹配键与 lshw "bus info" 一致
    DeviceInput *device = new DeviceInput;
    device->m_HwinfoToLshw = "usb@1:8";
    DeviceManager::instance()->m_ListDeviceKeyboard.append(device);

    QMap<QString, QString> mapinfo;
    ut_manager_setLshwMap(mapinfo);

    // Act: 从 lshw 填充键盘设备信息
    DeviceManager::instance()->setKeyboardInfoFromLshw(mapinfo);

    // Assert: 厂商与描述均被填充
    EXPECT_STREQ("Cherry Zhuhai", device->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Keyboard", device->m_Description.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceKeyboard.clear();
    delete device;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_correctNetworkLinkStatus)
{
    // Arrange: 网卡逻辑名 en2sp0, 链路状态 no
    DeviceNetwork *net = new DeviceNetwork;
    net->m_LogicalName = "en2sp0";
    net->m_Link = "no";
    DeviceManager::instance()->m_ListDeviceNetwork.append(net);

    // Act: 校正链路状态为 yes
    DeviceManager::instance()->correctNetworkLinkStatus("yes", "en2sp0");

    // Assert: 链路状态已更新且非空
    EXPECT_STREQ("yes", net->m_Link.toStdString().c_str());
    EXPECT_FALSE(net->m_Link.isEmpty());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceNetwork.clear();
    delete net;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addOthersDevice)
{
    // Arrange: 构造其他设备
    DeviceOthers *o = new DeviceOthers;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addOthersDevice(o);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceOthers.size());
    EXPECT_EQ(o, DeviceManager::instance()->m_ListDeviceOthers.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceOthers.clear();
    delete o;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setOthersDeviceInfoFromLshw)
{
    // Arrange: 其他设备匹配键与 lshw "bus info" 一致
    DeviceOthers *other = new DeviceOthers;
    other->m_HwinfoToLshw = "usb@1:5";
    DeviceManager::instance()->m_ListDeviceOthers.append(other);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "usb@1:5");
    mapinfo.insert("product", "product");

    // Act: 从 lshw 填充其他设备信息
    DeviceManager::instance()->setOthersDeviceInfoFromLshw(mapinfo);

    // Assert: 设备名被填充, 总线信息同步更新
    EXPECT_STREQ("product", other->m_Name.toStdString().c_str());
    EXPECT_STREQ("usb@1:5", other->m_BusInfo.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceOthers.clear();
    delete other;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addPowerDevice)
{
    // Arrange: 构造电源设备
    DevicePower *p = new DevicePower;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addPowerDevice(p);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDevicePower.size());
    EXPECT_EQ(p, DeviceManager::instance()->m_ListDevicePower.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDevicePower.clear();
    delete p;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addPrintDevice)
{
    // Arrange: 构造打印机设备
    DevicePrint *print = new DevicePrint;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addPrintDevice(print);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDevicePrint.size());
    EXPECT_EQ(print, DeviceManager::instance()->m_ListDevicePrint.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDevicePrint.clear();
    delete print;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addOtherPCIDevice)
{
    // Arrange: 构造其他 PCI 设备
    DeviceOtherPCI *o = new DeviceOtherPCI;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addOtherPCIDevice(o);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceOtherPCI.size());
    EXPECT_EQ(o, DeviceManager::instance()->m_ListDeviceOtherPCI.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceOtherPCI.clear();
    delete o;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addComputerDevice)
{
    // Arrange: 构造计算机设备
    DeviceComputer *c = new DeviceComputer;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addComputerDevice(c);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceComputer.size());
    EXPECT_EQ(c, DeviceManager::instance()->m_ListDeviceComputer.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceComputer.clear();
    delete c;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addCdromDevice)
{
    // Arrange: 构造光驱设备
    DeviceCdrom *c = new DeviceCdrom;

    // Act: 添加到设备管理器
    DeviceManager::instance()->addCdromDevice(c);

    // Assert: 列表长度为 1, 且保存的正是该设备指针
    EXPECT_EQ(1, DeviceManager::instance()->m_ListDeviceCdrom.size());
    EXPECT_EQ(c, DeviceManager::instance()->m_ListDeviceCdrom.at(0));

    // Cleanup
    DeviceManager::instance()->m_ListDeviceCdrom.clear();
    delete c;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addLshwinfoIntoCdromDevice)
{
    // Arrange: 光驱设备匹配键与 lshw "bus info" 一致
    DeviceCdrom *cd = new DeviceCdrom;
    cd->m_HwinfoToLshw = "usb@1:8";
    DeviceManager::instance()->m_ListDeviceCdrom.append(cd);

    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "usb@1:8");
    mapinfo.insert("product", "product");
    mapinfo.insert("vendor", "vendor");

    // Act: 从 lshw 填充光驱设备信息
    DeviceManager::instance()->addLshwinfoIntoCdromDevice(mapinfo);

    // Assert: 设备名与厂商均被填充
    EXPECT_STREQ("product", cd->m_Name.toStdString().c_str());
    EXPECT_STREQ("vendor", cd->m_Vendor.toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_ListDeviceCdrom.clear();
    delete cd;
}


TEST_F(UT_DeviceManager, UT_DeviceManager_addBusId)
{
    // Arrange: 构造总线 ID 列表
    QStringList lst;
    lst << "bus";

    // Act: 添加总线 ID
    DeviceManager::instance()->addBusId(lst);

    // Assert: 总线 ID 列表长度为 1, 内容一致
    EXPECT_EQ(1, DeviceManager::instance()->m_BusIdList.size());
    EXPECT_STREQ("bus", DeviceManager::instance()->m_BusIdList.at(0).toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_BusIdList.clear();
}

TEST_F(UT_DeviceManager, UT_DeviceManager_getBusId)
{
    // Arrange: 添加一条总线 ID
    QStringList lst;
    lst << "bus";
    DeviceManager::instance()->addBusId(lst);

    // Act: 读取总线 ID 列表
    QStringList ret = DeviceManager::instance()->getBusId();

    // Assert: 长度与内容均一致
    EXPECT_EQ(1, ret.size());
    EXPECT_STREQ("bus", ret.at(0).toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_BusIdList.clear();
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addCmdInfo_001)
{
    // Arrange: 构造音频命令输出缓存
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "name");
    QList<QMap<QString, QString>> info;
    info.append(mapinfo);
    QMap<QString, QList<QMap<QString, QString>>> cmdInfo;
    cmdInfo.insert("audio", info);

    // Act: 添加命令输出缓存
    DeviceManager::instance()->addCmdInfo(cmdInfo);

    // Assert: 命令键与条目数均正确
    EXPECT_EQ(1, DeviceManager::instance()->m_cmdInfo.size());
    EXPECT_EQ(1, DeviceManager::instance()->m_cmdInfo["audio"].size());
}

void ut_manager_setcmdinfo()
{
    QString key = "audio";
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Name", "Name");
    QList<QMap<QString, QString>> lst;
    lst.append(mapinfo);
    DeviceManager::instance()->m_cmdInfo.insert(key, lst);
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addCmdInfo_002)
{
    // Arrange: 缓存中已有 "audio" 一条记录
    ut_manager_setcmdinfo();
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "name");
    QList<QMap<QString, QString>> info;
    info.append(mapinfo);
    QMap<QString, QList<QMap<QString, QString>>> cmdInfo;
    cmdInfo.insert("audio", info);

    // Act: 追加同键命令输出缓存
    DeviceManager::instance()->addCmdInfo(cmdInfo);

    // Assert: 不覆盖, 追加为两条记录
    EXPECT_EQ(1, DeviceManager::instance()->m_cmdInfo.size());
    EXPECT_EQ(2, DeviceManager::instance()->m_cmdInfo["audio"].size());
}

TEST_F(UT_DeviceManager, UT_DeviceManager_cmdInfo)
{
    // Arrange: 先注入一条 "audio" 记录, 再通过 addCmdInfo 追加一条
    ut_manager_setcmdinfo();
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "name");
    QList<QMap<QString, QString>> info;
    info.append(mapinfo);
    QMap<QString, QList<QMap<QString, QString>>> cmdInfo;
    cmdInfo.insert("audio", info);
    DeviceManager::instance()->addCmdInfo(cmdInfo);

    // Act: 读取 "audio" 命令输出缓存
    QList<QMap<QString, QString>> lst = DeviceManager::instance()->cmdInfo("audio");

    // Assert: 追加后共两条记录, 内容包含注入字段, 其余键不受影响
    ASSERT_EQ(2, lst.size());
    EXPECT_TRUE(lst.at(0).contains("Name") || lst.at(0).contains("name"));
    EXPECT_EQ(1, DeviceManager::instance()->m_cmdInfo.size());
}

TEST_F(UT_DeviceManager, UT_DeviceManager_getDeviceOverview)
{
    // Arrange: 清空单例, 无任何设备
    DeviceManager::instance()->clear();

    // Act: 获取设备概况信息
    DeviceManager::instance()->getDeviceOverview();

    // Assert: 无设备时概况映射为空
    EXPECT_EQ(0, DeviceManager::instance()->m_OveriewMap.size());
    EXPECT_TRUE(DeviceManager::instance()->m_OveriewMap.isEmpty());
}

void ut_manager_setClassMap()
{
    DeviceCpu *cpu = new DeviceCpu;
    cpu->m_Driver = "";
    QList<DeviceBaseInfo *> lstcpu;
    lstcpu.append(dynamic_cast<DeviceBaseInfo *>(cpu));
    DeviceManager::instance()->m_DeviceClassMap.insert("cpu", lstcpu);

    DeviceAudio *audio = new DeviceAudio;
    audio->m_Driver = "video";
    audio->m_Name = "name";
    QList<DeviceBaseInfo *> lstaudio;
    lstaudio.append(dynamic_cast<DeviceBaseInfo *>(audio));
    DeviceManager::instance()->m_DeviceClassMap.insert("audio", lstaudio);
}

void ut_manager_setdriverpool()
{
    QMap<QString, QStringList> mapaudio;
    QString keyaudio;
    QStringList strLstAudio;
    mapaudio.insert(keyaudio, strLstAudio);
    DeviceManager::instance()->m_DeviceDriverPool.insert("video", mapaudio);
}

TEST_F(UT_DeviceManager, UT_DeviceManager_getDeviceDriverPool)
{
    // Arrange: 类别映射含 CPU/音频设备, 驱动池已有 "video" 记录
    ut_manager_setClassMap();
    ut_manager_setdriverpool();

    // Act: 汇总全部设备驱动池
    DeviceManager::instance()->getDeviceDriverPool();

    // Assert: 空驱动设备不进池, 池大小保持 1
    EXPECT_EQ(1, DeviceManager::instance()->m_DeviceDriverPool.size());
    EXPECT_TRUE(DeviceManager::instance()->m_DeviceDriverPool.contains("video"));

    // Cleanup: 先释放映射内设备再清空, 避免泄漏
    for (const auto &lst : DeviceManager::instance()->m_DeviceClassMap)
        qDeleteAll(lst);
    DeviceManager::instance()->m_DeviceDriverPool.clear();
    DeviceManager::instance()->m_DeviceClassMap.clear();
}

TEST_F(UT_DeviceManager, UT_DeviceManager_addInputInfo)
{
    // Arrange: 清空输入设备信息缓存
    DeviceManager::instance()->m_InputDeviceInfo.clear();
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "name");

    // Act: 添加输入设备信息
    DeviceManager::instance()->addInputInfo("key", mapinfo);

    // Assert: 缓存长度为 1, 内容一致
    EXPECT_EQ(1, DeviceManager::instance()->m_InputDeviceInfo.size());
    EXPECT_EQ(mapinfo, DeviceManager::instance()->m_InputDeviceInfo["key"]);

    // Cleanup
    DeviceManager::instance()->m_InputDeviceInfo.clear();
}

TEST_F(UT_DeviceManager, UT_DeviceManager_inputInfo)
{
    // Arrange: 注入一条输入设备信息
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "name");
    DeviceManager::instance()->addInputInfo("key", mapinfo);

    // Act: 读取该键的输入设备信息
    QMap<QString, QString> map = DeviceManager::instance()->inputInfo("key");

    // Assert: 返回内容与写入内容一致
    EXPECT_EQ(mapinfo, map);
    EXPECT_STREQ("name", map["name"].toStdString().c_str());

    // Cleanup
    DeviceManager::instance()->m_InputDeviceInfo.clear();
}

QList<QMap<QString, QString>> &ut_manager_cmd_btdevice()
{
    static QList<QMap<QString, QString>> lst;
    QMap<QString, QString> map;
    map.insert("blue1", "blue1");
    lst.append(map);
    return lst;
}

TEST_F(UT_DeviceManager, UT_DeviceManager_isDeviceExistInPairedDevice_001)
{
    // Arrange: 打桩 cmdInfo 返回含 blue1 的已配对设备
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_manager_cmd_btdevice);

    // Act: 查询 blue1 是否已配对
    bool ret = DeviceManager::instance()->isDeviceExistInPairedDevice("blue1");

    // Assert: 已配对返回 true, 空名不误报
    EXPECT_TRUE(ret);
    EXPECT_FALSE(DeviceManager::instance()->isDeviceExistInPairedDevice(""));
}

TEST_F(UT_DeviceManager, UT_DeviceManager_isDeviceExistInPairedDevice_002)
{
    // Arrange: 打桩 cmdInfo 返回含 blue1 的已配对设备
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_manager_cmd_btdevice);

    // Act: 查询 blue2 是否已配对
    bool ret = DeviceManager::instance()->isDeviceExistInPairedDevice("blue2");

    // Assert: 未配对返回 false, 已配对名仍命中
    EXPECT_FALSE(ret);
    EXPECT_TRUE(DeviceManager::instance()->isDeviceExistInPairedDevice("blue1"));
}

TEST_F(UT_DeviceManager, UT_DeviceManager_setCpuNum)
{
    // Arrange: 默认 CPU 数量
    // Act: 设置 CPU 数量为 2
    DeviceManager::instance()->setCpuNum(2);

    // Assert: CPU 数量被更新为 2
    EXPECT_EQ(2, DeviceManager::instance()->m_CpuNum);
    EXPECT_NE(0, DeviceManager::instance()->m_CpuNum);

    // Cleanup
    DeviceManager::instance()->m_CpuNum = 0;
}
