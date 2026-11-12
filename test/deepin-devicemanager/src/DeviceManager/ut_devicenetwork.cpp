// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * UT_DeviceNetwork — DeviceNetwork 单元测试（qt-autotest-generator Mode 2 重写增强）
 *
 * 打桩决策（依据 skill 依赖矩阵：外部 IO / 子进程 / 设备环境一律 stub）：
 *  - Common::executeClientCmd —— 真实实现会起 modinfo/lsusb/lspci 子进程
 *    （driverIsKernelIn / setVendorNameBylsusbLspci），一律 stub（返回空输出）
 *  - DBusEnableInterface::enable —— setEnable 走 DBus 启停网卡，一律 stub
 *  - QFileInfo::exists(QString)（静态重载，函数指针打桩）—— setIsWireless 读
 *    /sys 目录探测 phy80211/wireless，真实文件系统不可依赖，一律 stub
 * 其余（setAttribute/getOtherMapInfo/mapInfoToList 等纯内存逻辑）使用真实实现。
 *
 * 用例计数声明（min 按技能 §4 level/factors 下限；actual 为本文件实际用例数）：
 *
 * | 方法 | level | factors | min | actual |
 * |--------------------------|-----|------------------------|-----|--------|
 * | DeviceNetwork | mid | in_degree:1 | 2 | 2 |
 * | canDisable | low | - | 1 | 2 |
 * | correctCurrentLinkStatus | mid | in_degree:1 | 2 | 2 |
 * | driver | low | - | 1 | 1 |
 * | enable | mid | in_degree:2 | 3 | 3 |
 * | getOverviewInfo | low | - | 1 | 1 |
 * | hwAddress | low | - | 1 | 1 |
 * | initFilterKey | mid | in_degree:1 | 2 | 2 |
 * | isWireless | low | - | 1 | 2 |
 * | loadBaseDeviceInfo | low | - | 1 | 1 |
 * | loadOtherDeviceInfo | low | - | 1 | 1 |
 * | loadTableData | low | - | 1 | 2 |
 * | loadTableHeader | low | - | 1 | 1 |
 * | logicalName | mid | in_degree:2 | 3 | 3 |
 * | name | low | - | 1 | 1 |
 * | setEnable | low | - | 1 | 4 |
 * | setInfoFromHwinfo | mid | complexity:5;lines:51 | 3 | 4 |
 * | setInfoFromLshw | mid | lines:50 | 2 | 4 |
 * | setInfoFromTomlOneByOne | mid | in_degree:1 | 2 | 2 |
 * | setIsWireless | mid | in_degree:1 | 2 | 2 |
 * | subTitle | low | - | 1 | 1 |
 * | vendor | low | - | 1 | 1 |
 */

// 分支清单（来源：DeviceNetwork::setEnable）
// B1: SysPath 为空 → EDS_Faild（不触发 DBus）
// B2: SysPath 含 usb（&& 左操作数）→ 进入唯一标识校验
// B3: UniqueID 为空（&& 右操作数）→ EDS_Faild
// B4: DBus enable 成功 → m_Enable=e，返回 EDS_Success
// B5: DBus enable 失败 → 仅记录，m_Enable 不变
// B6: res ? EDS_Success : EDS_Faild → 三元选择返回值

// 分支清单（来源：DeviceNetwork::setInfoFromHwinfo）
// B1: map 含 "path" → 取小写键（name/unique_id/driver），m_Enable=false，return true（禁用态）
// B2: 无 "path" → 取大写键（Device/Vendor/Device File/Permanent HW Address 等）
// B3: VID_PID 非空且 Modalias 含 usb → setVendorNameBylsusbLspci 查厂商
// B4: 驱动为核内驱动（driverIsKernelIn）→ m_CanUninstall=false
// B5: SysPath 为空且无 SysFS Device Link 且 SysFS ID 以 /devices 开头 → m_SysPath 取 SysFS ID
// B6: setIsWireless 依据 /sys 路径判定无线
// B7: map 含 "Device" → m_Name 取 Device
// B8: return true

// 分支清单（来源：DeviceNetwork::setInfoFromLshw）
// B1: m_Name 为空 → m_Name 取 description
// B2: 驱动为核内驱动（driverIsKernelIn）→ m_CanUninstall=false
// B3: vendor 为空且 name 含 ARM（忽略大小写）→ m_Vendor="ARM Ltd"
// B4: vendor 为空且 name 不含 ARM → m_Vendor=m_Name
// B5: getOtherMapInfo 汇集其他属性

#include "DeviceNetwork.h"
#include "DeviceBios.h"
#include "DeviceInfo.h"
#include "commonfunction.h"
#include "DBusEnableInterface.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

// Common::executeClientCmd 打桩：阻断 modinfo/lsusb/lspci 真实子进程
static QByteArray ut_net_execCmdRet = QByteArray();
static QByteArray ut_net_execCmd_stub(const QString &, const QStringList &, const QString &, int, bool)
{
    return ut_net_execCmdRet;
}

// DBus enable 打桩返回值（非静态成员函数，stub 首参接收 this）
static bool ut_net_dbusRet = true;
static bool ut_net_dbusEnable(void *, const QString &, const QString &, const QString &,
                              const QString &, bool, const QString &)
{
    return ut_net_dbusRet;
}

// QFileInfo::exists(QString) 打桩返回值（静态重载，无 this）
static bool ut_net_qfiExistsRet = false;
static bool ut_net_qfiExists_stub(const QString &)
{
    return ut_net_qfiExistsRet;
}

class UT_DeviceNetwork : public ::testing::Test
{
public:
    void SetUp()
    {
        m_deviceNetwork = new DeviceNetwork;
    }
    void TearDown()
    {
        delete m_deviceNetwork;
    }
    DeviceNetwork *m_deviceNetwork;
};

void ut_network_setlshwinfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("description", "description");
    mapinfo.insert("product", "product");
    mapinfo.insert("vendor", "vendor");
    mapinfo.insert("version", "version");
    mapinfo.insert("bus info", "bus info");
    mapinfo.insert("logical name", "enp2s0");
    mapinfo.insert("serial", "f4:b5:20:24:5e:4f");
    mapinfo.insert("irq", "irq");
    mapinfo.insert("memory", "memory");
    mapinfo.insert("width", "width");
    mapinfo.insert("clock", "clock");
    mapinfo.insert("capabilities", "capabilities");
    mapinfo.insert("autonegotiation", "autonegotiation");
    mapinfo.insert("broadcast", "broadcast");
    mapinfo.insert("driver", "driver");
    mapinfo.insert("driverversion", "driverversion");
    mapinfo.insert("duplex", "duplex");
    mapinfo.insert("firmware", "firmware");
    mapinfo.insert("port", "port");
    mapinfo.insert("link", "link");
    mapinfo.insert("ip", "ip");
    mapinfo.insert("size", "size");
    mapinfo.insert("capacity", "capacity");
    mapinfo.insert("Latency", "Latency");
    mapinfo.insert("multicast", "multicast");
}

void ut_network_sethwinfomap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Device File", "enp2s0");
    mapinfo.insert("Model", "Model");
    mapinfo.insert("path", "path");
}

// 构造等价类：新建设备为空状态
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_ctor_Defaults)
{
    // Arrange：新建 DeviceNetwork
    ASSERT_NE(nullptr, m_deviceNetwork);

    // Act
    QString name = m_deviceNetwork->name();

    // Assert：名称为空、非无线、SysPath 为空不可禁用
    EXPECT_TRUE(name.isEmpty());
    EXPECT_EQ(m_deviceNetwork->m_Name, name);
    EXPECT_FALSE(m_deviceNetwork->isWireless());
    EXPECT_FALSE(m_deviceNetwork->canDisable());
}

// 构造等价类：默认硬件地址与唯一标识为空
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_ctor_HwAddressEmpty)
{
    // Arrange：新建 DeviceNetwork
    ASSERT_NE(nullptr, m_deviceNetwork);

    // Act
    QString hw = m_deviceNetwork->hwAddress();

    // Assert：MAC 与 UniqueID 均为空
    EXPECT_TRUE(hw.isEmpty());
    EXPECT_EQ(m_deviceNetwork->m_MACAddress, hw);
    EXPECT_TRUE(m_deviceNetwork->m_UniqueID.isEmpty());
}

// 等价类：过滤键初始化（2 个键）
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_initFilterKey)
{
    // Arrange：调用前后键数一致（幂等）
    m_deviceNetwork->initFilterKey();
    const int firstSize = m_deviceNetwork->m_FilterKey.size();

    // Act
    m_deviceNetwork->initFilterKey();

    // Assert
    EXPECT_EQ(2, m_deviceNetwork->m_FilterKey.size());
    EXPECT_EQ(firstSize, m_deviceNetwork->m_FilterKey.size());
}

// 等价类：过滤键内容
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_initFilterKey_KeyContents)
{
    // Arrange：新建设备
    ASSERT_NE(nullptr, m_deviceNetwork);

    // Act
    m_deviceNetwork->initFilterKey();

    // Assert
    EXPECT_EQ(2, m_deviceNetwork->m_FilterKey.size());
    EXPECT_TRUE(m_deviceNetwork->m_FilterKey.contains("ioport"));
    EXPECT_TRUE(m_deviceNetwork->m_FilterKey.contains("network"));
}

// 等价类：概览信息返回设备名
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_getOverviewInfo)
{
    // Arrange：注入 lshw 数据
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Act
    QString overview = m_deviceNetwork->getOverviewInfo();

    // Assert：与 m_Name 一致
    EXPECT_STREQ("product", overview.toStdString().c_str());
    EXPECT_EQ(m_deviceNetwork->m_Name, overview);
}

// 等价类：基本信息 8 项
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_loadBaseDeviceInfo)
{
    // Arrange：注入 lshw 数据
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Act
    m_deviceNetwork->loadBaseDeviceInfo();

    // Assert：8 项，首项为 Name
    ASSERT_EQ(8, m_deviceNetwork->m_LstBaseInfo.size());
    EXPECT_STREQ("Name", m_deviceNetwork->m_LstBaseInfo.at(0).first.toStdString().c_str());
    EXPECT_STREQ("product", m_deviceNetwork->m_LstBaseInfo.at(0).second.toStdString().c_str());
}

// 等价类：其他信息 15 项
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_loadOtherDeviceInfo)
{
    // Arrange：注入 lshw 数据
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Act
    m_deviceNetwork->loadOtherDeviceInfo();

    // Assert：15 项，含 MAC 地址条目（mapInfoToList 按键排序，不假设首项）
    ASSERT_EQ(15, m_deviceNetwork->m_LstOtherInfo.size());
    EXPECT_EQ(1, m_deviceNetwork->m_LstOtherInfo.count(QPair<QString, QString>("MAC Address", "f4:b5:20:24:5e:4f")));
    EXPECT_STREQ("f4:b5:20:24:5e:4f", m_deviceNetwork->m_MACAddress.toStdString().c_str());
}

// 等价类：表头 3 列 Name/Vendor/Type
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_loadTableHeader)
{
    // Arrange：新建设备
    ASSERT_NE(nullptr, m_deviceNetwork);

    // Act
    m_deviceNetwork->loadTableHeader();

    // Assert
    ASSERT_EQ(3, m_deviceNetwork->m_TableHeader.size());
    EXPECT_STREQ("Name", m_deviceNetwork->m_TableHeader.at(0).toStdString().c_str());
    EXPECT_STREQ("Vendor", m_deviceNetwork->m_TableHeader.at(1).toStdString().c_str());
    EXPECT_STREQ("Type", m_deviceNetwork->m_TableHeader.at(2).toStdString().c_str());
}

// 等价类：可用且启用 → 名称无前缀（需预置驱动，否则 available() 因驱动为空强制不可用）
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_loadTableData_AvailableEnabled)
{
    // Arrange：可用 + 启用 + 非空驱动
    m_deviceNetwork->m_Name = "Ethernet Controller";
    m_deviceNetwork->m_Vendor = "Intel Corporation";
    m_deviceNetwork->m_Model = "description";
    m_deviceNetwork->m_Driver = "driver";
    m_deviceNetwork->m_Available = true;
    m_deviceNetwork->m_Enable = true;

    // Act
    m_deviceNetwork->loadTableData();

    // Assert：名称列无前缀
    ASSERT_EQ(3, m_deviceNetwork->m_TableData.size());
    EXPECT_STREQ("Ethernet Controller", m_deviceNetwork->m_TableData.at(0).toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceNetwork->m_TableData.at(1).toStdString().c_str());
}

// 等价类：不可用且禁用 → 名称带 Disable 前缀
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_loadTableData_UnavailableAndDisabled)
{
    // Arrange：禁用态
    m_deviceNetwork->m_Name = "Ethernet Controller";
    m_deviceNetwork->m_Vendor = "Intel Corporation";
    m_deviceNetwork->m_Model = "description";
    m_deviceNetwork->m_Available = false;
    m_deviceNetwork->m_Enable = false;

    // Act
    m_deviceNetwork->loadTableData();

    // Assert：名称列带 Disable 前缀
    ASSERT_EQ(3, m_deviceNetwork->m_TableData.size());
    EXPECT_STREQ("(Disable) Ethernet Controller", m_deviceNetwork->m_TableData.at(0).toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceNetwork->m_TableData.at(1).toStdString().c_str());
}

// 等价类：name() 与 m_Name 一致
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_name)
{
    // Arrange：注入 lshw 数据
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Act
    QString name = m_deviceNetwork->name();

    // Assert
    EXPECT_STREQ("product", name.toStdString().c_str());
    EXPECT_EQ(m_deviceNetwork->m_Name, name);
}

// 等价类：vendor() 与 m_Vendor 一致
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_vendor)
{
    // Arrange：注入 lshw 数据
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Act
    const QString &vendor = m_deviceNetwork->vendor();

    // Assert
    EXPECT_STREQ("vendor", vendor.toStdString().c_str());
    EXPECT_EQ(m_deviceNetwork->m_Vendor, vendor);
}

// 等价类：driver() 与 m_Driver 一致
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_driver)
{
    // Arrange：注入 lshw 数据
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Act
    QString driver = m_deviceNetwork->driver();

    // Assert
    EXPECT_STREQ("driver", driver.toStdString().c_str());
    EXPECT_EQ(m_deviceNetwork->m_Driver, driver);
}

// 等价类：subTitle 与名称一致
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_subTitle)
{
    // Arrange：注入 lshw 数据
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Act
    QString title = m_deviceNetwork->subTitle();

    // Assert
    EXPECT_STREQ("product", title.toStdString().c_str());
    EXPECT_EQ(m_deviceNetwork->m_Name, title);
}

// 等价类：hwAddress() 与 MAC 地址一致
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_hwAddress)
{
    // Arrange：注入 lshw 数据（serial 为 MAC）
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Act
    QString hw = m_deviceNetwork->hwAddress();

    // Assert
    EXPECT_STREQ("f4:b5:20:24:5e:4f", hw.toStdString().c_str());
    EXPECT_EQ(m_deviceNetwork->m_MACAddress, hw);
}

// 等价类：默认非无线网卡
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_isWireless_DefaultFalse)
{
    // Arrange：新建设备
    ASSERT_NE(nullptr, m_deviceNetwork);

    // Act
    bool ret = m_deviceNetwork->isWireless();

    // Assert：与成员一致
    EXPECT_FALSE(ret);
    EXPECT_EQ(m_deviceNetwork->m_IsWireless, ret);
}

// 等价类：/sys 路径探测到无线标识 → 无线（stub QFileInfo::exists）
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_isWireless_AfterSetWireless)
{
    // Arrange：stub /sys 目录探测为命中
    Stub stub;
    stub.set(static_cast<bool (*)(const QString &)>(QFileInfo::exists), ut_net_qfiExists_stub);
    ut_net_qfiExistsRet = true;
    m_deviceNetwork->setIsWireless("/devices/pci0000:00/net/wlan0");

    // Act
    bool ret = m_deviceNetwork->isWireless();

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(m_deviceNetwork->m_IsWireless, ret);
}

// 分支等价类：/sys 路径无无线标识 → 保持非无线（真实文件系统，路径必不存在）
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setIsWireless_Nonexistent_False)
{
    // Arrange：新建设备
    ASSERT_FALSE(m_deviceNetwork->m_IsWireless);

    // Act
    m_deviceNetwork->setIsWireless("/devices/nonexistent-ut-net-xyz");

    // Assert：探测路径不存在 → 非无线
    EXPECT_FALSE(m_deviceNetwork->m_IsWireless);
    EXPECT_EQ(false, m_deviceNetwork->m_IsWireless);
}

// 分支等价类：/sys 路径含 wireless 标识 → 无线（stub QFileInfo::exists）
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setIsWireless_Wireless_True)
{
    // Arrange：stub /sys 目录探测为命中
    Stub stub;
    stub.set(static_cast<bool (*)(const QString &)>(QFileInfo::exists), ut_net_qfiExists_stub);
    ut_net_qfiExistsRet = true;

    // Act
    m_deviceNetwork->setIsWireless("/devices/pci0000:00/net/wlan0");

    // Assert
    EXPECT_TRUE(m_deviceNetwork->m_IsWireless);
    EXPECT_EQ(true, m_deviceNetwork->m_IsWireless);
}

// 分支等价类：DBus 成功且默认态 → 可用
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_enable_DefaultTrue)
{
    // Arrange：新建设备
    ASSERT_NE(nullptr, m_deviceNetwork);

    // Act
    bool ret = m_deviceNetwork->enable();

    // Assert：与成员一致
    EXPECT_TRUE(ret);
    EXPECT_EQ(m_deviceNetwork->m_Enable, ret);
}

// 分支等价类：禁用后 enable() 返回 false
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_enable_AfterSetFalse)
{
    // Arrange：置为禁用
    m_deviceNetwork->m_Enable = false;

    // Act
    bool ret = m_deviceNetwork->enable();

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(m_deviceNetwork->m_Enable, ret);
}

// 分支等价类：启用后 enable() 返回 true
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_enable_AfterSetTrue)
{
    // Arrange：置为启用
    m_deviceNetwork->m_Enable = true;

    // Act
    bool ret = m_deviceNetwork->enable();

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(m_deviceNetwork->m_Enable, ret);
}

// 分支等价类：SysPath 为空 → 不可禁用
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_canDisable_EmptySysPath_False)
{
    // Arrange：SysPath 为空
    m_deviceNetwork->m_SysPath = "";

    // Act
    bool ret = m_deviceNetwork->canDisable();

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(false, ret);
}

// 分支等价类：SysPath 非空 → 可禁用
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_canDisable_WithSysPath_True)
{
    // Arrange：SysPath 非空
    m_deviceNetwork->m_SysPath = "/sys/devices/pci0000:00/net/eth0";

    // Act
    bool ret = m_deviceNetwork->canDisable();

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(true, ret);
}

// 等价类：注入 lshw 后逻辑名一致
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_logicalName_AfterLshw)
{
    // Arrange：注入 lshw 数据
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Act
    QString logicalName = m_deviceNetwork->logicalName();

    // Assert
    EXPECT_STREQ("enp2s0", logicalName.toStdString().c_str());
    EXPECT_EQ(m_deviceNetwork->m_LogicalName, logicalName);
}

// 等价类：默认逻辑名为空
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_logicalName_Empty_Default)
{
    // Arrange：新建设备
    ASSERT_NE(nullptr, m_deviceNetwork);

    // Act
    QString logicalName = m_deviceNetwork->logicalName();

    // Assert
    EXPECT_TRUE(logicalName.isEmpty());
    EXPECT_EQ(m_deviceNetwork->m_LogicalName, logicalName);
    EXPECT_EQ(0, m_deviceNetwork->m_LogicalName.size());
}

// 等价类：预置逻辑名后返回预置值
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_logicalName_Preset)
{
    // Arrange：预置逻辑名
    m_deviceNetwork->m_LogicalName = "wlp3s0";

    // Act
    QString logicalName = m_deviceNetwork->logicalName();

    // Assert
    EXPECT_STREQ("wlp3s0", logicalName.toStdString().c_str());
    EXPECT_EQ(m_deviceNetwork->m_LogicalName, logicalName);
}

// 分支等价类：链路状态不同 → 更新
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_correctCurrentLinkStatus_Different_Updates)
{
    // Arrange：当前链路为 link
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);
    ASSERT_STREQ("link", m_deviceNetwork->m_Link.toStdString().c_str());

    // Act
    m_deviceNetwork->correctCurrentLinkStatus("link1");

    // Assert
    EXPECT_STREQ("link1", m_deviceNetwork->m_Link.toStdString().c_str());
    EXPECT_EQ(QString("link1"), m_deviceNetwork->m_Link);
}

// 分支等价类：链路状态相同 → 保持不变
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_correctCurrentLinkStatus_Same_Keeps)
{
    // Arrange：当前链路为 link
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";
    m_deviceNetwork->setInfoFromLshw(mapinfo);
    ASSERT_STREQ("link", m_deviceNetwork->m_Link.toStdString().c_str());

    // Act
    m_deviceNetwork->correctCurrentLinkStatus("link");

    // Assert
    EXPECT_STREQ("link", m_deviceNetwork->m_Link.toStdString().c_str());
    EXPECT_EQ(QString("link"), m_deviceNetwork->m_Link);
}

// 分支 B1：SysPath 为空 → EDS_Faild（不触发 DBus）
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setEnable_EmptySysPath_Faild)
{
    // Arrange：SysPath 为空
    m_deviceNetwork->m_SysPath = "";

    // Act
    EnableDeviceStatus value = m_deviceNetwork->setEnable(true);

    // Assert：m_HardwareClass 被设置为网络接口，返回失败
    EXPECT_STREQ("network interface", m_deviceNetwork->m_HardwareClass.toStdString().c_str());
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, value);
}

// 分支 B2：SysPath 含 usb 且 UniqueID 为空 → EDS_Faild
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setEnable_UsbNoUniqueID_Faild)
{
    // Arrange：usb 路径但唯一标识为空
    m_deviceNetwork->m_SysPath = "usb";
    m_deviceNetwork->m_UniqueID = "";

    // Act
    EnableDeviceStatus value = m_deviceNetwork->setEnable(true);

    // Assert：返回失败且 m_HardwareClass 已被设置
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, value);
    EXPECT_STREQ("network interface", m_deviceNetwork->m_HardwareClass.toStdString().c_str());
}

// 分支 B3：DBus 成功 → EDS_Success 且 m_Enable 更新
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setEnable_DbusSuccess)
{
    // Arrange：标识齐备，DBus 打桩成功
    m_deviceNetwork->m_SysPath = "usb";
    m_deviceNetwork->m_UniqueID = "unique";
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_net_dbusEnable);
    ut_net_dbusRet = true;

    // Act
    EnableDeviceStatus value = m_deviceNetwork->setEnable(true);

    // Assert
    EXPECT_EQ(EnableDeviceStatus::EDS_Success, value);
    EXPECT_TRUE(m_deviceNetwork->m_Enable);
}

// 分支 B4：DBus 失败 → EDS_Faild 且 m_Enable 不被改写
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setEnable_DbusFail)
{
    // Arrange：标识齐备，DBus 打桩失败，预置启用状态
    m_deviceNetwork->m_SysPath = "usb";
    m_deviceNetwork->m_UniqueID = "unique";
    m_deviceNetwork->m_Enable = true;
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_net_dbusEnable);
    ut_net_dbusRet = false;

    // Act
    EnableDeviceStatus value = m_deviceNetwork->setEnable(false);

    // Assert：返回失败，m_Enable 保持原值 true
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, value);
    EXPECT_TRUE(m_deviceNetwork->m_Enable);
}

// 分支 B1：map 含 path → 禁用态，m_Enable=false，核内驱动不可卸载
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setInfoFromHwinfo_PathBranch_Disabled)
{
    // Arrange：含 path 键的小写键 map，stub executeClientCmd 阻断 modinfo
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_net_execCmd_stub);
    ut_net_execCmdRet = QByteArray();
    QMap<QString, QString> mapinfo;
    ut_network_sethwinfomap(mapinfo);
    mapinfo.insert("name", "network controller");
    mapinfo.insert("driver", "r8169");

    // Act
    bool ret = m_deviceNetwork->setInfoFromHwinfo(mapinfo);

    // Assert：禁用态成员齐备
    EXPECT_TRUE(ret);
    EXPECT_STREQ("network controller", m_deviceNetwork->m_Name.toStdString().c_str());
    EXPECT_STREQ("path", m_deviceNetwork->m_SysPath.toStdString().c_str());
    EXPECT_FALSE(m_deviceNetwork->m_Enable);
    EXPECT_FALSE(m_deviceNetwork->m_CanUninstall);
}

// 分支 B2：无 path → 大写键路径，Permanent HW Address 优先
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setInfoFromHwinfo_Normal_PreferPermanentHwAddress)
{
    // Arrange：大写键 map，stub executeClientCmd 阻断 modinfo
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_net_execCmd_stub);
    ut_net_execCmdRet = QByteArray();
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "network");
    mapinfo.insert("Vendor", "vendor");
    mapinfo.insert("Device File", "enp2s0");
    mapinfo.insert("HW Address", "66:77:88:99:aa:bb");
    mapinfo.insert("Permanent HW Address", "00:11:22:33:44:55");

    // Act
    bool ret = m_deviceNetwork->setInfoFromHwinfo(mapinfo);

    // Assert：MAC 与 UniqueID 均取 Permanent HW Address
    EXPECT_TRUE(ret);
    EXPECT_STREQ("00:11:22:33:44:55", m_deviceNetwork->m_MACAddress.toStdString().c_str());
    EXPECT_STREQ("00:11:22:33:44:55", m_deviceNetwork->m_UniqueID.toStdString().c_str());
}

// 分支 B5：SysPath 为空且 SysFS ID 以 /devices 开头 → m_SysPath 取 SysFS ID
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setInfoFromHwinfo_SysFSID_TakesSysPath)
{
    // Arrange：大写键 map + SysFS ID；驱动为空不触发 modinfo
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "Ethernet Controller");
    mapinfo.insert("SysFS ID", "/devices/pci0000:00/0000:00:1f.6/net/eth0");

    // Act
    bool ret = m_deviceNetwork->setInfoFromHwinfo(mapinfo);

    // Assert：SysPath 取 SysFS ID，名称取 Device
    EXPECT_TRUE(ret);
    EXPECT_STREQ("/devices/pci0000:00/0000:00:1f.6/net/eth0", m_deviceNetwork->m_SysPath.toStdString().c_str());
    EXPECT_STREQ("Ethernet Controller", m_deviceNetwork->m_Name.toStdString().c_str());
}

// 分支 B3：VID_PID 非空且 Modalias 含 usb → 查询 lsusb（stub 子进程，不崩溃）
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setInfoFromHwinfo_UsbModalias_VendorLookup)
{
    // Arrange：usb 设备标识，stub executeClientCmd 阻断 lsusb/lspci
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_net_execCmd_stub);
    ut_net_execCmdRet = QByteArray();
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "USB Ethernet");
    mapinfo.insert("VID_PID", "0b95:7720");
    mapinfo.insert("Module Alias", "usb:v0B95p7720d0010");

    // Act
    bool ret = m_deviceNetwork->setInfoFromHwinfo(mapinfo);

    // Assert：VID_PID 记录到 PhysID，设备名取 Device
    EXPECT_TRUE(ret);
    EXPECT_STREQ("0b95:7720", m_deviceNetwork->m_PhysID.toStdString().c_str());
    EXPECT_STREQ("USB Ethernet", m_deviceNetwork->m_Name.toStdString().c_str());
}

// 分支等价类：标准 lshw 数据全属性赋值
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setInfoFromLshw_Normal)
{
    // Arrange：标准 lshw 数据，stub executeClientCmd 阻断 modinfo
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_net_execCmd_stub);
    ut_net_execCmdRet = QByteArray();
    QMap<QString, QString> mapinfo;
    ut_network_setlshwinfo(mapinfo);
    m_deviceNetwork->m_HwinfoToLshw = "f4:b5:20:24:5e:4f";

    // Act
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Assert：逐项核对
    EXPECT_STREQ("description", m_deviceNetwork->m_Model.toStdString().c_str());
    EXPECT_STREQ("product", m_deviceNetwork->m_Name.toStdString().c_str());
    EXPECT_STREQ("vendor", m_deviceNetwork->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("version", m_deviceNetwork->m_Version.toStdString().c_str());
    EXPECT_STREQ("bus info", m_deviceNetwork->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("enp2s0", m_deviceNetwork->m_LogicalName.toStdString().c_str());
    EXPECT_STREQ("f4:b5:20:24:5e:4f", m_deviceNetwork->m_MACAddress.toStdString().c_str());
    EXPECT_STREQ("irq", m_deviceNetwork->m_Irq.toStdString().c_str());
    EXPECT_STREQ("memory", m_deviceNetwork->m_Memory.toStdString().c_str());
    EXPECT_STREQ("width", m_deviceNetwork->m_Width.toStdString().c_str());
    EXPECT_STREQ("clock", m_deviceNetwork->m_Clock.toStdString().c_str());
    EXPECT_STREQ("capabilities", m_deviceNetwork->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("autonegotiation", m_deviceNetwork->m_Autonegotiation.toStdString().c_str());
    EXPECT_STREQ("broadcast", m_deviceNetwork->m_Broadcast.toStdString().c_str());
    EXPECT_STREQ("driver", m_deviceNetwork->m_Driver.toStdString().c_str());
    EXPECT_STREQ("driverversion", m_deviceNetwork->m_DriverVersion.toStdString().c_str());
    EXPECT_STREQ("duplex", m_deviceNetwork->m_Duplex.toStdString().c_str());
    EXPECT_STREQ("firmware", m_deviceNetwork->m_Firmware.toStdString().c_str());
    EXPECT_STREQ("link", m_deviceNetwork->m_Link.toStdString().c_str());
    EXPECT_STREQ("ip", m_deviceNetwork->m_Ip.toStdString().c_str());
    EXPECT_STREQ("size", m_deviceNetwork->m_Speed.toStdString().c_str());
    EXPECT_STREQ("capacity", m_deviceNetwork->m_Capacity.toStdString().c_str());
    EXPECT_STREQ("Latency", m_deviceNetwork->m_Latency.toStdString().c_str());
    EXPECT_STREQ("multicast", m_deviceNetwork->m_Multicast.toStdString().c_str());
}

// 分支 B1：product 为空 → m_Name 取 description
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setInfoFromLshw_NameEmpty_TakesDescription)
{
    // Arrange：product 缺失，description 存在
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_net_execCmd_stub);
    ut_net_execCmdRet = QByteArray();
    QMap<QString, QString> mapinfo;
    mapinfo.insert("description", "Ethernet interface");
    mapinfo.insert("logical name", "enp3s0");

    // Act
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Assert：名称取 description
    EXPECT_STREQ("Ethernet interface", m_deviceNetwork->m_Name.toStdString().c_str());
    EXPECT_STREQ("Ethernet interface", m_deviceNetwork->m_Model.toStdString().c_str());
}

// 分支 B4：vendor 为空且 name 不含 ARM → m_Vendor 取 name
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setInfoFromLshw_EmptyVendor_TakesName)
{
    // Arrange：无 vendor 键
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_net_execCmd_stub);
    ut_net_execCmdRet = QByteArray();
    QMap<QString, QString> mapinfo;
    mapinfo.insert("product", "Realtek Ethernet");

    // Act
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Assert：厂商回退为名称
    EXPECT_STREQ("Realtek Ethernet", m_deviceNetwork->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Realtek Ethernet", m_deviceNetwork->m_Name.toStdString().c_str());
}

// 分支 B3：vendor 为空且 name 含 ARM → m_Vendor="ARM Ltd"
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setInfoFromLshw_EmptyVendor_ARM)
{
    // Arrange：无 vendor 键，name 含 ARM
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_net_execCmd_stub);
    ut_net_execCmdRet = QByteArray();
    QMap<QString, QString> mapinfo;
    mapinfo.insert("product", "ARM Ethernet Controller");

    // Act
    m_deviceNetwork->setInfoFromLshw(mapinfo);

    // Assert：厂商固定为 ARM Ltd
    EXPECT_STREQ("ARM Ltd", m_deviceNetwork->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("ARM Ethernet Controller", m_deviceNetwork->m_Name.toStdString().c_str());
}

// 分支等价类：空 map → TOML_None，成员不变
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setInfoFromTomlOneByOne_EmptyMap)
{
    // Arrange：空 map
    QMap<QString, QString> mapInfo;

    // Act
    TomlFixMethod ret = m_deviceNetwork->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_None, ret);
    EXPECT_TRUE(m_deviceNetwork->m_LogicalName.isEmpty());
    EXPECT_TRUE(m_deviceNetwork->m_Model.isEmpty());
}

// 分支等价类：多键注入 → 成员逐一赋值，含 Logical Name 键返回 TOML_Cover
TEST_F(UT_DeviceNetwork, UT_DeviceNetwork_setInfoFromTomlOneByOne_WithKeys)
{
    // Arrange：多键 map
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Type", "Ethernet interface");
    mapInfo.insert("Bus Info", "pci@0000:00:1f.6");
    mapInfo.insert("Logical Name", "enp2s0");
    mapInfo.insert("Link", "yes");
    mapInfo.insert("IP", "192.168.1.2");

    // Act
    TomlFixMethod ret = m_deviceNetwork->setInfoFromTomlOneByOne(mapInfo);

    // Assert：ret 取最后一个 setTomlAttribute（Logical Name）
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_STREQ("Ethernet interface", m_deviceNetwork->m_Model.toStdString().c_str());
    EXPECT_STREQ("pci@0000:00:1f.6", m_deviceNetwork->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("enp2s0", m_deviceNetwork->m_LogicalName.toStdString().c_str());
    EXPECT_STREQ("yes", m_deviceNetwork->m_Link.toStdString().c_str());
    EXPECT_STREQ("192.168.1.2", m_deviceNetwork->m_Ip.toStdString().c_str());
}
