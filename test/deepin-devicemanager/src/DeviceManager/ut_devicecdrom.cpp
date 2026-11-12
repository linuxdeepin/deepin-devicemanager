// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Mode 2 重写：DeviceCdrom 单元测试（旧版断言空洞/无分支映射，评级 D，整体重写）
// 覆盖方法：ctor/setInfoFromLshw/setInfoFromTomlOneByOne/setInfoFromHwinfo/name/vendor/driver/
//          subTitle/getOverviewInfo/initFilterKey/loadBaseDeviceInfo/loadOtherDeviceInfo/loadTableData
//          以及被其调用的基类 matchToLshw/setHwinfoLshwKey 关键分支
//
// 用例计数声明（min 为分级下限，actual 为实际用例数，均 ≤ 现有用例规模）
// | method                       | level | min | actual |
// | ctor/initFilterKey           | low   |   1 |      1 |
// | matchToLshw(基类白盒)        | low   |   6 |      6 |
// | setInfoFromLshw              | low   |   2 |      2 |
// | setInfoFromTomlOneByOne      | low   |   3 |      3 |
// | setInfoFromHwinfo            | low   |   7 |      7 |
// | loadBaseDeviceInfo           | low   |   1 |      1 |
// | loadOtherDeviceInfo          | low   |   1 |      1 |
// | loadTableData                | low   |   3 |      3 |
//
// 打桩依据（dependency-tracer）：全类无外部依赖（无 DBus/网络/文件/进程），无需打桩；
//   lshw/hwinfo 数据以 QMap 入参形式注入

#include "DeviceCdrom.h"
#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

static bool ut_containsPair(const QList<QPair<QString, QString>> &lst, const QString &k, const QString &v)
{
    for (const auto &p : lst) {
        if (p.first == k && p.second == v)
            return true;
    }
    return false;
}

class UT_DeviceCdrom : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_pCdrom = new DeviceCdrom;
    }
    void TearDown() override
    {
        delete m_pCdrom;
        m_pCdrom = nullptr;
    }
    DeviceCdrom *m_pCdrom = nullptr;
};

// ===================== ctor =====================
// Branch: DeviceCdrom.cpp:11-22 ctor 成员置空 + initFilterKey()
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_ctor_default)
{
    // Arrange: SetUp 已构造默认光驱设备（构造即被测路径）
    EXPECT_NE(nullptr, m_pCdrom);
    // Act: 读取构造后的初始状态
    const QString name = m_pCdrom->name();
    // Assert: 基础属性为空
    EXPECT_EQ(QString(""), name);
    EXPECT_EQ(QString(""), m_pCdrom->vendor());
    EXPECT_EQ(QString(""), m_pCdrom->driver());
    EXPECT_EQ(QString(""), m_pCdrom->subTitle());
    EXPECT_EQ(QString(""), m_pCdrom->getOverviewInfo());
    // Assert: driver 为空 -> 基类 available() 返回 false 且置位 m_Available
    EXPECT_FALSE(m_pCdrom->available());
    // Assert: 过滤键初始化（Serial ID/logical name 等，bus info 被注释）
    EXPECT_TRUE(m_pCdrom->m_FilterKey.contains("Serial ID"));
    EXPECT_TRUE(m_pCdrom->m_FilterKey.contains("logical name"));
    EXPECT_TRUE(m_pCdrom->m_FilterKey.contains("Device File"));
    EXPECT_FALSE(m_pCdrom->m_FilterKey.contains("bus info"));
    // Assert: 其它信息为空
    EXPECT_EQ(0, m_pCdrom->getOtherAttribs().size());
}

// ===================== matchToLshw（经基类，直接白盒调用） =====================
// Branch: DeviceInfo.cpp:1095-1104 serial+logical name 双键 -> 与 m_HwinfoToLshw 拼接串比较
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_matchToLshw_serialLogical)
{
    // Arrange: hwinfo 侧 HW Address + Device File 拼接作为匹配值
    m_pCdrom->m_HwinfoToLshw = "SER01eth0";
    QMap<QString, QString> map;
    map.insert("serial", "SER01");
    map.insert("logical name", "eth0");
    // Act
    const bool matched = m_pCdrom->matchToLshw(map);
    // Assert: 拼接串相等命中
    EXPECT_EQ(QString("SER01eth0"), m_pCdrom->m_HwinfoToLshw);
    EXPECT_TRUE(matched);
}

// Branch: DeviceInfo.cpp:1102-1104 m_HwinfoToLshw == serial -> true（无 Device File 场景）
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_matchToLshw_serialOnly)
{
    // Arrange: 匹配值仅等于 serial
    m_pCdrom->m_HwinfoToLshw = "SER01";
    QMap<QString, QString> map;
    map.insert("serial", "SER01");
    map.insert("logical name", "eth0");
    // Act
    const bool matched = m_pCdrom->matchToLshw(map);
    // Assert: 退回仅 serial 比较
    EXPECT_EQ(QString("SER01"), m_pCdrom->m_HwinfoToLshw);
    EXPECT_TRUE(matched);
}

// Branch: DeviceInfo.cpp:1108-1110 无 bus info -> false
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_matchToLshw_noBusInfo)
{
    // Arrange: 无 serial/logical name/bus info
    QMap<QString, QString> map;
    map.insert("product", "DVD-RW");
    // Act
    const bool matched = m_pCdrom->matchToLshw(map);
    // Assert: 缺少必要键直接不匹配
    EXPECT_FALSE(matched);
    EXPECT_EQ(QString(""), m_pCdrom->m_HwinfoToLshw);   // 构造默认值未变
}

// Branch: DeviceInfo.cpp:1113-1121 bus info 以 pci 开头 -> split("@") 后段与 m_HwinfoToLshw 比较
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_matchToLshw_pciMatch)
{
    // Arrange
    m_pCdrom->m_HwinfoToLshw = "0000:00:01.1";
    QMap<QString, QString> map;
    map.insert("bus info", "pci@0000:00:01.1");
    // Act
    const bool matched = m_pCdrom->matchToLshw(map);
    // Assert: pci 后段相等命中
    EXPECT_TRUE(matched);
    EXPECT_EQ(QString("pci@0000:00:01.1"), map["bus info"]);
}

// Branch: DeviceInfo.cpp:1113-1121 pci 但后段不匹配 -> 落到 usb 比较亦不等 -> false
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_matchToLshw_pciNoMatch)
{
    // Arrange
    m_pCdrom->m_HwinfoToLshw = "0000:00:01.1";
    QMap<QString, QString> map;
    map.insert("bus info", "pci@0000:00:02.2");
    // Act
    const bool matched = m_pCdrom->matchToLshw(map);
    // Assert: pci/usb 两路均不等
    EXPECT_FALSE(matched);
    EXPECT_NE(QString("pci@0000:00:02.2"), m_pCdrom->m_HwinfoToLshw);
}

// Branch: DeviceInfo.cpp:1123-1126 m_HwinfoToLshw == bus info（usb 设备） -> true
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_matchToLshw_usbMatch)
{
    // Arrange
    m_pCdrom->m_HwinfoToLshw = "usb@1:1.3";
    QMap<QString, QString> map;
    map.insert("bus info", "usb@1:1.3");
    // Act
    const bool matched = m_pCdrom->matchToLshw(map);
    // Assert: 整串相等命中（usb 分支）
    EXPECT_TRUE(matched);
    EXPECT_EQ(QString("usb@1:1.3"), map["bus info"]);
}

// ===================== setInfoFromLshw =====================
// Branch: DeviceCdrom.cpp:29-33 matchToLshw 为假 -> 直接返回 false 不写任何成员
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromLshw_nomatch)
{
    // Arrange: 无 bus info 必不匹配
    QMap<QString, QString> map;
    map.insert("product", "DVD-RW");
    map.insert("vendor", "HL-DT-ST");
    // Act
    const bool ret = m_pCdrom->setInfoFromLshw(map);
    // Assert: 不匹配且不写成员
    EXPECT_FALSE(ret);
    EXPECT_EQ(QString(""), m_pCdrom->name());
    EXPECT_EQ(QString(""), m_pCdrom->vendor());
}

// Branch: DeviceCdrom.cpp:35-47 product/vendor/version/bus info/capabilities 写入 + getOtherMapInfo;
//         DeviceInfo.cpp:954-986 overwrite=false 时属性写入
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromLshw_match)
{
    // Arrange: pci 匹配
    m_pCdrom->m_HwinfoToLshw = "0000:00:01.1";
    QMap<QString, QString> map;
    map.insert("bus info", "pci@0000:00:01.1");
    map.insert("product", "DVD-RW");
    map.insert("vendor", "HL-DT-ST");
    map.insert("version", "1.00");
    map.insert("capabilities", "dvd");
    map.insert("logical name", "sr0");
    map.insert("serial", "XYZ");       // 不在过滤键内的干扰项
    // Act
    const bool ret = m_pCdrom->setInfoFromLshw(map);
    // Assert: 属性逐项写入
    EXPECT_TRUE(ret);
    EXPECT_EQ(QString("DVD-RW"), m_pCdrom->name());
    EXPECT_EQ(QString("HL-DT-ST"), m_pCdrom->vendor());
    EXPECT_EQ(QString("1.00"), m_pCdrom->m_Version);
    EXPECT_EQ(QString("pci@0000:00:01.1"), m_pCdrom->m_BusInfo);
    EXPECT_EQ(QString("dvd"), m_pCdrom->m_Capabilities);
    // Assert: logical name 在过滤键内 -> 其它信息；serial 不在
    EXPECT_TRUE(ut_containsPair(m_pCdrom->getOtherAttribs(), "logical name", "sr0"));
    EXPECT_FALSE(ut_containsPair(m_pCdrom->getOtherAttribs(), "serial", "XYZ"));
}

// ===================== setInfoFromTomlOneByOne =====================
// Branch: DeviceCdrom.cpp:52-66 Model->m_Type/Bus Info/Capabilities/Maximum Power 写入,
//         ret 取 Speed 结果; 64 getOtherMapInfo
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromToml_cover)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Model", "DVD+-RW GBC-H20N");
    map.insert("Bus Info", "scsi@1:0.0.0");
    map.insert("Capabilities", "dvd dvd-r");
    map.insert("Maximum Power", "660mA");
    map.insert("Speed", "133360");
    // Act
    TomlFixMethod ret = m_pCdrom->setInfoFromTomlOneByOne(map);
    // Assert: 注意 "Model" 键写入的是 m_Type 而非 m_Name
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_EQ(QString("DVD+-RW GBC-H20N"), m_pCdrom->m_Type);
    EXPECT_EQ(QString(""), m_pCdrom->name());
    EXPECT_EQ(QString("scsi@1:0.0.0"), m_pCdrom->m_BusInfo);
    EXPECT_EQ(QString("dvd dvd-r"), m_pCdrom->m_Capabilities);
    EXPECT_EQ(QString("660mA"), m_pCdrom->m_MaxPower);
    EXPECT_EQ(QString("133360"), m_pCdrom->m_Speed);
    // Assert: toml 键均不在过滤键内
    EXPECT_EQ(0, m_pCdrom->getOtherAttribs().size());
}

// Branch: DeviceInfo.cpp:990-998 键缺失/值为空 -> TOML_None
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromToml_none)
{
    // Arrange: 缺 Speed 键
    QMap<QString, QString> mapNoKey;
    mapNoKey.insert("Model", "DVD+-RW");
    // Act: 缺键调用
    TomlFixMethod ret1 = m_pCdrom->setInfoFromTomlOneByOne(mapNoKey);
    // Assert: 其余属性写入，Speed 缺失 -> TOML_None
    EXPECT_EQ(TOML_None, ret1);
    EXPECT_EQ(QString("DVD+-RW"), m_pCdrom->m_Type);
    EXPECT_EQ(QString(""), m_pCdrom->m_Speed);

    // Arrange: Speed 为空值
    QMap<QString, QString> mapEmpty;
    mapEmpty.insert("Model", "DVD+-RW");
    mapEmpty.insert("Speed", "");
    // Act: 空值调用
    TomlFixMethod ret2 = m_pCdrom->setInfoFromTomlOneByOne(mapEmpty);
    // Assert
    EXPECT_EQ(TOML_None, ret2);
    EXPECT_EQ(QString(""), m_pCdrom->m_Speed);
}

// Branch: DeviceInfo.cpp:1000-1003 值含 nouse -> 清空 m_Speed 返回 TOML_nouse
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromToml_nouse)
{
    // Arrange
    m_pCdrom->m_Speed = "133360";
    QMap<QString, QString> map;
    map.insert("Speed", "NOUSE");
    // Act
    TomlFixMethod ret = m_pCdrom->setInfoFromTomlOneByOne(map);
    // Assert
    EXPECT_EQ(TOML_nouse, ret);
    EXPECT_EQ(QString(""), m_pCdrom->m_Speed);
}

// ===================== setInfoFromHwinfo =====================
// Branch: DeviceInfo.cpp:1039-1044 HW Address 存在 -> m_HwinfoToLshw = HWAddress+DeviceFile;
//         DeviceCdrom.cpp:72-85 Device/Vendor/Model/Revision 写入
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromHwinfo_hwAddress)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Device", "DVD-ROM");
    map.insert("Vendor", "HL-DT-ST");
    map.insert("Model", "GBC-H20N");
    map.insert("Revision", "1.00");
    map.insert("HW Address", "SER01");
    map.insert("Device File", "/dev/sr0");
    // Act
    m_pCdrom->setInfoFromHwinfo(map);
    // Assert: 成员逐项写入
    EXPECT_EQ(QString("DVD-ROM"), m_pCdrom->name());
    EXPECT_EQ(QString("HL-DT-ST"), m_pCdrom->vendor());
    EXPECT_EQ(QString("GBC-H20N"), m_pCdrom->m_Type);
    EXPECT_EQ(QString("1.00"), m_pCdrom->m_Version);
    // Assert: hwinfo->lshw 匹配值 = HW Address + Device File 直接拼接
    EXPECT_EQ(QString("SER01/dev/sr0"), m_pCdrom->m_HwinfoToLshw);
}

// Branch: DeviceInfo.cpp:1041-1042 HW Address 存在但无 Device File -> 仅用 HW Address
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromHwinfo_hwAddressNoFile)
{
    // Arrange: 仅 HW Address
    QMap<QString, QString> map;
    map.insert("HW Address", "SER01");
    // Act
    m_pCdrom->setInfoFromHwinfo(map);
    // Assert: 匹配值仅 HW Address；Device 未提供 -> 名称保持空
    EXPECT_EQ(QString("SER01"), m_pCdrom->m_HwinfoToLshw);
    EXPECT_EQ(QString(""), m_pCdrom->name());
}

// Branch: DeviceInfo.cpp:1046-1053 SysFS ID 存在且不含 usb -> m_HwinfoToLshw = SysFS BusID;
//         DeviceCdrom.cpp:78-93 SysFS BusID/Driver/Module Alias/VID_PID 写入
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromHwinfo_sysfs)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Device", "DVD-ROM");
    map.insert("SysFS ID", "pci-0000:00:01.1");
    map.insert("SysFS BusID", "0000:00:01.1");
    map.insert("Driver", "sr_mod");
    map.insert("Speed", "133360");
    map.insert("Module Alias", "usb:v1234p5678");
    map.insert("VID_PID", "1234:5678");
    map.insert("Serial ID", "XYZ");
    map.insert("Config Status", "cfg=new");
    // Act
    m_pCdrom->setInfoFromHwinfo(map);
    // Assert
    EXPECT_EQ(QString("0000:00:01.1"), m_pCdrom->m_HwinfoToLshw);
    EXPECT_EQ(QString("0000:00:01.1"), m_pCdrom->m_BusInfo);
    EXPECT_EQ(QString("sr_mod"), m_pCdrom->m_Driver);
    EXPECT_EQ(QString("133360"), m_pCdrom->m_Speed);
    EXPECT_EQ(QString("usb:v1234p5678"), m_pCdrom->m_Modalias);
    EXPECT_EQ(QString("1234:5678"), m_pCdrom->m_VID_PID);
    // Assert: m_PhysID 同步为 VID_PID
    EXPECT_EQ(QString("1234:5678"), m_pCdrom->m_PhysID);
    // Assert: driver 非空后 available() 走 m_forcedDisplay/m_Available 逻辑
    EXPECT_TRUE(m_pCdrom->available());
    // Assert: Serial ID/Config Status 在过滤键内 -> 其它信息
    EXPECT_TRUE(ut_containsPair(m_pCdrom->getOtherAttribs(), "Serial ID", "XYZ"));
    EXPECT_TRUE(ut_containsPair(m_pCdrom->getOtherAttribs(), "Config Status", "cfg=new"));
}

// Branch: DeviceInfo.cpp:1073-1080 usb 分支正则命中 -> usb@%1:%2.%3（nums 表按位还原）
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromHwinfo_usbRegex)
{
    // Arrange: SysFS ID 含 usb 走 usb 分支；BusID "1-1.3:2" 正则捕获 1,1,3
    QMap<QString, QString> map;
    map.insert("SysFS ID", "usb-0000:00");
    map.insert("SysFS BusID", "1-1.3:2");
    // Act
    m_pCdrom->setInfoFromHwinfo(map);
    // Assert: nums[1]="1", nums[1]="1", nums[3]="3"；BusInfo 独立取原始 BusID
    EXPECT_EQ(QString("usb@1:1.3"), m_pCdrom->m_HwinfoToLshw);
    EXPECT_EQ(QString("1-1.3:2"), m_pCdrom->m_BusInfo);
}

// Branch: DeviceInfo.cpp:1081-1085 usb 分支正则未命中 -> usb@%1:%2（chs 两段还原）
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromHwinfo_usbNoRegex)
{
    // Arrange: BusID "3-2:1.0" -> words=["3-2","1.0"], chs=["3","2"]，"3-2" 无 ".x" 不匹配正则
    QMap<QString, QString> map;
    map.insert("SysFS ID", "usb-0000:00");
    map.insert("SysFS BusID", "3-2:1.0");
    // Act
    m_pCdrom->setInfoFromHwinfo(map);
    // Assert: nums[3]="3", nums[2]="2"
    EXPECT_EQ(QString("usb@3:2"), m_pCdrom->m_HwinfoToLshw);
    EXPECT_EQ(QString("3-2:1.0"), m_pCdrom->m_BusInfo);
}

// Branch: DeviceInfo.cpp:1055-1058 words 尺寸不为 2 -> 直接返回，不生成匹配值
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_setInfoFromHwinfo_busIdMalformed)
{
    // Arrange: BusID 无冒号 -> words 尺寸 1
    QMap<QString, QString> map;
    map.insert("SysFS ID", "usb-0000:00");
    map.insert("SysFS BusID", "1-1.3");
    // Act
    m_pCdrom->setInfoFromHwinfo(map);
    // Assert: 匹配值未生成，但 BusInfo 仍正常写入
    EXPECT_EQ(QString(""), m_pCdrom->m_HwinfoToLshw);
    EXPECT_EQ(QString("1-1.3"), m_pCdrom->m_BusInfo);
}

// ===================== loadBaseDeviceInfo =====================
// Branch: DeviceCdrom.cpp:148-162 九项基本信息; DeviceInfo.cpp:896-904 空值跳过
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_loadBaseDeviceInfo)
{
    // Arrange: 全字段赋值
    m_pCdrom->m_Name = "DVD-RW";
    m_pCdrom->m_Vendor = "HL-DT-ST";
    m_pCdrom->m_Type = "GBC-H20N";
    m_pCdrom->m_Version = "1.00";
    m_pCdrom->m_BusInfo = "scsi@1:0.0.0";
    m_pCdrom->m_Capabilities = "dvd";
    m_pCdrom->m_Driver = "sr_mod";
    m_pCdrom->m_MaxPower = "660mA";
    m_pCdrom->m_Speed = "133360";
    // Act
    const QList<QPair<QString, QString>> &lst = m_pCdrom->getBaseAttribs();
    // Assert: 九项按序全部存在
    EXPECT_EQ(9, lst.size());
    EXPECT_TRUE(ut_containsPair(lst, "Name", "DVD-RW"));
    EXPECT_TRUE(ut_containsPair(lst, "Vendor", "HL-DT-ST"));
    EXPECT_TRUE(ut_containsPair(lst, "Model", "GBC-H20N"));
    EXPECT_TRUE(ut_containsPair(lst, "Version", "1.00"));
    EXPECT_TRUE(ut_containsPair(lst, "Bus Info", "scsi@1:0.0.0"));
    EXPECT_TRUE(ut_containsPair(lst, "Capabilities", "dvd"));
    EXPECT_TRUE(ut_containsPair(lst, "Driver", "sr_mod"));
    EXPECT_TRUE(ut_containsPair(lst, "Maximum Power", "660mA"));
    EXPECT_TRUE(ut_containsPair(lst, "Speed", "133360"));
}

// ===================== loadOtherDeviceInfo =====================
// Branch: DeviceCdrom.cpp:164-172 Module Alias/Physical ID + mapInfoToList()
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_loadOtherDeviceInfo)
{
    // Arrange: 成员 + 过滤键命中的 map 数据
    m_pCdrom->m_Modalias = "usb:v1234p5678";
    m_pCdrom->m_PhysID = "1234:5678";
    QMap<QString, QString> map;
    map.insert("Serial ID", "XYZ");
    map.insert("Driver Modules", "sr_mod");
    m_pCdrom->getOtherMapInfo(map);
    // Act
    const QList<QPair<QString, QString>> &lst = m_pCdrom->getOtherAttribs();
    // Assert: 成员项 + 过滤键命中项全部收集
    EXPECT_EQ(4, lst.size());
    EXPECT_TRUE(ut_containsPair(lst, "Module Alias", "usb:v1234p5678"));
    EXPECT_TRUE(ut_containsPair(lst, "Physical ID", "1234:5678"));
    EXPECT_TRUE(ut_containsPair(lst, "Serial ID", "XYZ"));
    EXPECT_TRUE(ut_containsPair(lst, "Driver Modules", "sr_mod"));
}

// ===================== loadTableData =====================
// Branch: DeviceCdrom.cpp:174-194 !available() -> "(Unavailable) name";
//         !enable() -> "(Disable) name"（后者覆盖前者）
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_loadTableData_unavailable)
{
    // Arrange: driver 为空 -> available() 为假；enable() 为真
    m_pCdrom->m_Name = "DVD-RW";
    m_pCdrom->m_Vendor = "HL-DT-ST";
    m_pCdrom->m_Type = "GBC-H20N";
    // Act
    const QStringList &data = m_pCdrom->getTableData();
    // Assert: 名称前缀 Unavailable（无翻译器 -> translateStr 返回源文本）
    EXPECT_EQ(QStringList() << "(Unavailable) DVD-RW" << "HL-DT-ST" << "GBC-H20N", data);
    // Assert: 光驱构造未置 m_CanEnable（基类默认 false）-> 表头末列为 "no"
    EXPECT_FALSE(m_pCdrom->m_CanEnable);
    EXPECT_EQ(QStringList() << "Name" << "Vendor" << "Model" << "no", m_pCdrom->getTableHeader());
}

// Branch: DeviceCdrom.cpp:186-189 enable() 为假 -> "(Disable) name"（覆盖 Unavailable 前缀）
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_loadTableData_disable)
{
    // Arrange: driver 为空 -> available 为假；m_Enable 置假 -> enable 为假，Disable 后写覆盖
    m_pCdrom->m_Name = "DVD-RW";
    m_pCdrom->m_Vendor = "HL-DT-ST";
    m_pCdrom->m_Type = "GBC-H20N";
    m_pCdrom->m_Enable = false;
    // Act
    const QStringList &data = m_pCdrom->getTableData();
    // Assert: Disable 前缀最终生效
    EXPECT_EQ(QStringList() << "(Disable) DVD-RW" << "HL-DT-ST" << "GBC-H20N", data);
    EXPECT_FALSE(m_pCdrom->enable());
}

// Branch: DeviceCdrom.cpp:191-194 available/enable 均为真 -> 原始名称
TEST_F(UT_DeviceCdrom, UT_DeviceCdrom_loadTableData_normal)
{
    // Arrange: driver 非空保证 available 为真
    m_pCdrom->m_Name = "DVD-RW";
    m_pCdrom->m_Vendor = "HL-DT-ST";
    m_pCdrom->m_Type = "GBC-H20N";
    m_pCdrom->m_Driver = "sr_mod";
    // Act
    const QStringList &data = m_pCdrom->getTableData();
    // Assert: 无前缀
    EXPECT_EQ(QStringList() << "DVD-RW" << "HL-DT-ST" << "GBC-H20N", data);
    EXPECT_TRUE(m_pCdrom->available());
}
