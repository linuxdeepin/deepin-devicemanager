// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ===================== 用例计数声明（总用例数: 65） =====================
// 设备: DeviceStorage（存储设备）
//
// | method | level | factors | min | actual |
// |---|---|---|---|---|
// | setHwinfoInfo | high | 等价类/边界/分支 | 10 | 14 |
// | checkDiskSize | high | 边界/分支 | 4 | 6 |
// | getSerialID | high | 等价类/边界 | 2 | 2 |
// | addInfoFromSmartctl | high | 等价类 | 2 | 2 |
// | getInfoFromsmartctl | high | 等价类/分支 | 3 | 4 |
// | appendDisk | high | 边界/分支 | 3 | 3 |
// | addInfoFromlshw | mid | 等价类/分支 | 2 | 3 |
// | addNVMEInfoFromlshw | mid | 等价类 | 2 | 3 |
// | setMediaType | mid | 等价类/边界 | 3 | 4 |
// | setDiskSerialID | mid | 分支 | 2 | 3 |
// | compareSize | mid | 等价类/边界 | 3 | 4 |
// | getManfName | low | 等价类 | 1 | 2 |
// | getOemName | low | 等价类 | 1 | 2 |
// | unitConvertByDecimal | low | 边界 | 1 | 2 |
// | getOverviewInfo | low | 分支 | 2 | 3 |
// | isValid | low | 等价类 | 1 | 2 |
// | 加载接口与getter | low | 冒烟 | 5 | 6 |
//
// 复杂方法分支清单:
// - setHwinfoInfo: B1 缺少SysFS BusID返回false | B2 非hw平台设置Model | B3 ST前缀厂商=ST |
//   B4 RS前缀厂商=Longsys | B5 Vendor匹配pci正则清空 | B6 Driver Modules含usb_storage接口=USB |
//   B7 Attached to正则剥离Controller | B8 Capacity含bytes十进制换算 | B8b Capacity无bytes回退换算 |
//   B9 空容量且无序列号返回false | B10 SerialID为空取map值 | B11 SerialID为"0"清空 |
//   B12 nvme关键字设置NvmeKey | B13 mmcblk驱动读sysfs(需桩) | B14 返回true
//   环境限制: B6中spec_version文件仅真实UFS设备存在,本机不可覆盖
// - getSerialID: B1 空链接返回空 | B2 含platform/读/proc/bootdevice(本机不存在) |
//   B3 cid读取 | B4 回退unique_number | B5 返回空
//   环境限制: B3依赖/proc/bootdevice/cid仅ARM UFS机器存在,本机不可覆盖
// - appendDisk: B1 当前容量为0采用对方容量 | B2 双方容量>0求和并合并属性 | B3 对方容量0无操作
// - checkDiskSize: B1 非hw平台直接返回 | B2 256GB区间 | B3 512GB区间 | B4 1TB区间 |
//   B5 2TB区间 | B6 USB 16GB区间 | B7 USB 32GB区间 | B8 USB 64GB区间 | B9 USB 128GB区间
// - addInfoFromlshw: B1 bus info格式非法返回false | B2 关键字不匹配返回false |
//   B3 匹配后提取唯一key | B4 更新接口 | B5 解析基本信息
// - addNVMEInfoFromlshw: B1 bus info格式非法返回false | B2 关键字不匹配返回false |
//   B3 匹配后设置厂商
// - setDiskSerialID: B1 序列号已在列表中直接返回 | B2 by-id且含型号提取序列号 |
//   B3 无匹配项序列号不变
// - compareSize: B1 任一为空返回拼接 | B2 提取bytes1>bytes2返回size1 |
//   B3 bytes1<=bytes2返回size2 | B4 无方括号格式按0比较
// - setMediaType: B1 设备名不匹配返回false | B2 value=0→SSD | B3 value=1→HDD |
//   B4 其他值→Unknown
// - getOverviewInfo: B1 UFS接口 | B2 USB接口 | B3 默认媒体类型

#include "DeviceStorage.h"
#include "commonfunction.h"
#include "stub.h"

#include <QMap>
#include <QString>
#include <QStringList>

#include <gtest/gtest.h>

// ===================== 数据构造辅助 =====================
static void ut_storage_setHwinfoInfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("SysFS BusID", "nvme0n12:0:0:0");
    mapinfo.insert("Model", "ST240BX500SSD1");
    mapinfo.insert("Vendor", "");
    mapinfo.insert("Driver", "ahci, sd");
    mapinfo.insert("Attached to", "#2 (SATA controller)");
    mapinfo.insert("Revision", "R013");
    mapinfo.insert("Hardware Class", "disk");
    mapinfo.insert("Capacity", "223 GB (240057409536 bytes)");
    mapinfo.insert("Serial ID", "2002E3E0B393");
    mapinfo.insert("Device File", "/dev/nvme0n1");
    mapinfo.insert("SysFS Device Link", "/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0:0:0");
}

static void ut_storage_setLshwInfo(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("bus info", "scsi@nvme0n12:0.0.0");
    mapinfo.insert("interface", "sata");
    mapinfo.insert("capabilities", "gpt-1.00 partitioned partitioned:gpt");
    mapinfo.insert("version", "R013");
    mapinfo.insert("serial", "2002E3E0B393");
    mapinfo.insert("product", "CT240BX500SSD1");
    mapinfo.insert("description", "ATA Disk");
    mapinfo.insert("size", "223GiB (240GB)");
}

static void ut_storage_setSmartCtlInfo_001(QMap<QString, QString> &mapinfo)
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

static void ut_storage_setSmartCtlInfo_002(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Firmware Version", "M6CR013");
    mapinfo.insert("SATA Version is", "SATA 3.2, 6.0 Gb/s (current: 6.0 Gb/s)");
    mapinfo.insert("Rotation Rate", "Solid State Device");
    mapinfo.insert("Power On Hours", "3717");
    mapinfo.insert("Power Cycles", "306");
    mapinfo.insert("Total NVM Capacity", "240,057,409,536 bytes [240 GB]");
    mapinfo.insert("Namespace 1 Size/Capacity", "240,057,409,536 bytes [240 GB]");
    mapinfo.insert("Model Number", "CT240BX500SSD1");
    mapinfo.insert("Serial Number", "2002E3E0B393");
}

// safeReadSysFsFile 桩: 覆盖 setHwinfoInfo 中 mmcblk 驱动的 sysfs 读取分支,
// 避免测试依赖真实 /sys 设备文件
static QString ut_storage_safeReadSysFsFile(const QString &baseDir, const QString &fileName, QString &errOut)
{
    Q_UNUSED(baseDir);
    errOut.clear();
    if (fileName == "name")
        return "SD64G_CARD";
    if (fileName == "manfid")
        return "0x000003";
    if (fileName == "oemid")
        return "0x5054";
    return QString();
}

// ===================== 测试夹具 =====================
class UT_DeviceStorage : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 保存静态平台类型, 用例内可切换 hw 平台, 结束时恢复, 避免污染其他用例
        m_savedSpecialComType = Common::specialComType;
        m_deviceStorage = new DeviceStorage;
    }
    void TearDown() override
    {
        Common::specialComType = m_savedSpecialComType;
        delete m_deviceStorage;
        m_deviceStorage = nullptr;
    }
    DeviceStorage *m_deviceStorage = nullptr;
    int m_savedSpecialComType = Common::Unknow;
    Stub m_stub;
};

// ===================== setMediaType（B1-B4） =====================
// 用例: setMediaType 设备文件不含指定名称返回 false
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetMediaType_NameNotInDeviceFile_ReturnsFalse)
{
    // Arrange
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    // Act
    bool result = m_deviceStorage->setMediaType("nvme0n1", "0");
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceStorage->mediaType(), QString(""));
}

// 用例: setMediaType value=0 识别为 SSD（B2）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetMediaType_Rotational0_SetsSSD)
{
    // Arrange
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    // Act
    bool result = m_deviceStorage->setMediaType("sda", "0");
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("SSD", m_deviceStorage->mediaType().toStdString().c_str());
}

// 用例: setMediaType value=1 识别为 HDD（B3）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetMediaType_Rotational1_SetsHDD)
{
    // Arrange
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    // Act
    bool result = m_deviceStorage->setMediaType("sda", "1");
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("HDD", m_deviceStorage->mediaType().toStdString().c_str());
}

// 用例: setMediaType 其他 value 识别为 Unknown（B4 边界等价类）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetMediaType_OtherValue_SetsUnknown)
{
    // Arrange
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    // Act
    bool result = m_deviceStorage->setMediaType("sda", "2");
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("Unknown", m_deviceStorage->mediaType().toStdString().c_str());
}

// ===================== isValid =====================
// 用例: 有效磁盘（容量非空）isValid 返回 true
TEST_F(UT_DeviceStorage, UT_DeviceStorage_IsValid_SizeNonEmpty_ReturnsTrue)
{
    // Arrange
    m_deviceStorage->m_Size = "32GB";
    // Act
    bool valid = m_deviceStorage->isValid();
    // Assert
    EXPECT_TRUE(valid);
    EXPECT_EQ(m_deviceStorage->m_Size, QString("32GB"));
}

// 用例: 默认构造（容量空）isValid 返回 false
TEST_F(UT_DeviceStorage, UT_DeviceStorage_IsValid_DefaultEmpty_ReturnsFalse)
{
    // Arrange
    bool valid = false;
    // Act
    valid = m_deviceStorage->isValid();
    // Assert
    EXPECT_FALSE(valid);
    EXPECT_EQ(m_deviceStorage->m_Size, QString(""));
}

// ===================== setDiskSerialID（B1-B3） =====================
// 用例: 序列号已在 device files 列表中, 保持不变（B1）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetDiskSerialID_SerialInList_KeepsSerial)
{
    // Arrange
    QString deviceFiles = "/dev/sda, /dev/disk/by-path/pci-0000:00:17.0-ata-3, /dev/disk/by-id/ata-CT240BX500SSD1_2002E3E0B393";
    m_deviceStorage->m_SerialNumber = "2002E3E0B393";
    // Act
    m_deviceStorage->setDiskSerialID(deviceFiles);
    // Assert
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
    EXPECT_TRUE(deviceFiles.contains(m_deviceStorage->m_SerialNumber));
}

// 用例: 带前缀序列号从 by-id 路径按型号提取（B2）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetDiskSerialID_ByIdWithModel_ExtractsSerial)
{
    // Arrange
    QString deviceFiles = "/dev/sda, /dev/disk/by-path/pci-0000:00:17.0-ata-3, /dev/disk/by-id/ata-CT240BX500SSD1_2002E3E0B393";
    m_deviceStorage->m_Name = "CT240BX500SSD1";
    m_deviceStorage->m_SerialNumber = "XXX2002E3E0B393";
    // Act
    m_deviceStorage->setDiskSerialID(deviceFiles);
    // Assert
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
    EXPECT_NE(m_deviceStorage->m_SerialNumber, QString("XXX2002E3E0B393"));
}

// 用例: 无 by-id 匹配项时序列号保持原值（B3）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetDiskSerialID_NoMatchKeepsOriginal)
{
    // Arrange
    QString deviceFiles = "/dev/sda, /dev/disk/by-path/pci-0000:00:17.0-ata-3";
    m_deviceStorage->m_Name = "CT240BX500SSD1";
    m_deviceStorage->m_SerialNumber = "ORIGINAL123";
    // Act
    m_deviceStorage->setDiskSerialID(deviceFiles);
    // Assert
    EXPECT_STREQ("ORIGINAL123", m_deviceStorage->m_SerialNumber.toStdString().c_str());
    EXPECT_FALSE(deviceFiles.contains(m_deviceStorage->m_SerialNumber));
}

// ===================== compareSize（B1-B4） =====================
// 用例: size1 为空时返回拼接结果（B1）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_CompareSize_EmptyFirstReturnsConcat)
{
    // Arrange
    QString size1;
    QString size2 = "32GB";
    // Act
    QString larger = m_deviceStorage->compareSize(size1, size2);
    // Assert
    EXPECT_EQ(larger, QString("32GB"));
    EXPECT_TRUE(larger == size1 + size2);
}

// 用例: 无方括号格式按 0 字节比较返回 size2（B4）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_CompareSize_PlainStringsReturnSecond)
{
    // Arrange
    QString size1 = "16GB";
    QString size2 = "32GB";
    // Act
    QString larger = m_deviceStorage->compareSize(size1, size2);
    // Assert
    EXPECT_EQ(larger, QString("32GB"));
    EXPECT_EQ(larger, size2);
}

// 用例: 方括号格式 bytes1 <= bytes2 返回 size2（B3）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_CompareSize_BracketFormat_LargerSecond)
{
    // Arrange
    QString size1 = "240,057,409,536 bytes [240 GB]";
    QString size2 = "500,000,000,000 bytes [500 GB]";
    // Act
    QString larger = m_deviceStorage->compareSize(size1, size2);
    // Assert
    EXPECT_EQ(larger, QString("500,000,000,000 bytes [500 GB]"));
    EXPECT_EQ(larger, size2);
}

// 用例: 方括号格式 bytes1 > bytes2 返回 size1（B2）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_CompareSize_BracketFormat_LargerFirst)
{
    // Arrange
    QString size1 = "1,024,209,543,168 bytes [1.02 TB]";
    QString size2 = "500,000,000,000 bytes [500 GB]";
    // Act
    QString larger = m_deviceStorage->compareSize(size1, size2);
    // Assert
    EXPECT_EQ(larger, QString("1,024,209,543,168 bytes [1.02 TB]"));
    EXPECT_EQ(larger, size1);
}

// ===================== setHwinfoInfo（B1-B14） =====================
// 用例: 缺少 SysFS BusID 直接返回 false（B1 无效输入等价类）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_MissingBusID_ReturnsFalse)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Model", "ST240BX500SSD1");
    // Act
    bool result = m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceStorage->m_KeyToLshw, QString(""));
}

// 用例: 完整 hwinfo 信息解析, 名称/驱动/序列号均设置（B2/B7/B10, 返回 true）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_ValidDisk_ParsesFields)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    // Act
    bool result = m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("ST240BX500SSD1", m_deviceStorage->name().toStdString().c_str());
    EXPECT_STREQ("ahci, sd", m_deviceStorage->driver().toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
}

// 用例: ST 前缀且厂商为空时识别为希捷 ST（B3 边界值）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_SeagatePrefix_VendorST)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    // Act
    m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_STREQ("ST", m_deviceStorage->vendor().toStdString().c_str());
    EXPECT_TRUE(m_deviceStorage->name().startsWith("ST"));
}

// 用例: RS 前缀且厂商为空时识别为 Longsys（B4 边界值）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_LongsysPrefix_VendorLongsys)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "sda1:0:0:0");
    mapinfo.insert("Model", "RSYE3836N-480G");
    mapinfo.insert("Vendor", "");
    mapinfo.insert("Capacity", "480 GB (480103981056 bytes)");
    mapinfo.insert("Serial ID", "TESTSERIAL01");
    // Act
    bool result = m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("Longsys", m_deviceStorage->vendor().toStdString().c_str());
    EXPECT_STREQ("RSYE3836N-480G", m_deviceStorage->name().toStdString().c_str());
}

// 用例: Vendor 匹配 pci 正则时清空厂商（B5）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_PciVendorRegex_ClearsVendor)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "sda:0:0:0");
    mapinfo.insert("Model", "SAMSUNG870");
    mapinfo.insert("Vendor", "pci 0x1b4b");
    mapinfo.insert("Capacity", "256 GB (256060514304 bytes)");
    mapinfo.insert("Serial ID", "S3Z8NB0K123456");
    // Act
    bool result = m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(m_deviceStorage->vendor(), QString(""));
    EXPECT_FALSE(m_deviceStorage->m_Vendor.contains("pci"));
}

// 用例: Driver Modules 含 usb_storage 时接口为 USB（B6, 无 Attached to 避免覆盖）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_UsbStorage_SetsUSB)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "sdb:0:0:0");
    mapinfo.insert("Model", "USB Flash Drive");
    mapinfo.insert("Vendor", "Generic");
    mapinfo.insert("Driver Modules", "usb_storage");
    mapinfo.insert("Capacity", "16 GB (16008678400 bytes)");
    mapinfo.insert("Serial ID", "0700B4E3E0B393");
    // Act
    bool result = m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("USB", m_deviceStorage->interface().toStdString().c_str());
    EXPECT_TRUE(m_deviceStorage->interface().contains("USB"));
}

// 用例: Attached to 正则剥离 Controller 后缀（B7）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_AttachedToRegex_StripsController)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    // Act
    m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_FALSE(m_deviceStorage->interface().contains("Controller", Qt::CaseInsensitive));
    EXPECT_STREQ("SATA ", m_deviceStorage->interface().toStdString().c_str());
}

// 用例: Capacity 含 bytes 时按十进制换算为 240 GB（B8）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_SizeWithBytes_UsesDecimalKilos)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    // Act
    m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_STREQ("240 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_EQ(240057409536ULL, m_deviceStorage->getDiskSizeByte());
}

// 用例: Capacity 无 bytes 时回退到 convertToBytes 换算（B8b 分支覆盖）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_SizeWithoutBytes_FallsBackConvert)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "sda:0:0:0");
    mapinfo.insert("Model", "SAMSUNG870");
    mapinfo.insert("Capacity", "223 GB");
    mapinfo.insert("Serial ID", "S3Z8NB0K123456");
    // Act
    bool result = m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("223GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_EQ(239444426752ULL, m_deviceStorage->getDiskSizeByte());
}

// 用例: 容量为空且无序列号判定为无效磁盘（B9 边界值, 返回 false）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_EmptySizeNoSerial_ReturnsFalse)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "sda:0:0:0");
    mapinfo.insert("Model", "UNKNOWN_DISK");
    mapinfo.insert("Vendor", "Generic");
    // Act
    bool result = m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceStorage->m_Size, QString(""));
}

// 用例: getSerialID 结果为空时回填 map 中 Serial ID（B10）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_SerialEmpty_FallsBackToMap)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    // Act
    m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
    EXPECT_FALSE(m_deviceStorage->m_SerialNumber.isEmpty());
}

// 用例: Serial ID 为 "0" 时清空序列号（B11 边界值）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_SerialZero_Cleared)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "sda:0:0:0");
    mapinfo.insert("Model", "SAMSUNG870");
    mapinfo.insert("Capacity", "256 GB (256060514304 bytes)");
    mapinfo.insert("Serial ID", "0");
    // Act
    bool result = m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(m_deviceStorage->m_SerialNumber.isEmpty());
    EXPECT_EQ(m_deviceStorage->m_SerialNumber, QString(""));
}

// 用例: BusID 含 nvme 时设置 NvmeKey 链接属性（B12）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_NvmeKey_SetsLinkAttribute)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    // Act
    m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_TRUE(m_deviceStorage->m_KeyToLshw.contains("nvme"));
    EXPECT_STREQ("/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0:0:0",
                 m_deviceStorage->m_NvmeKey.toStdString().c_str());
}

// 用例: mmcblk 驱动时通过 sysfs 桩读取名称与厂商（B13, safeReadSysFsFile 打桩）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SetHwinfoInfo_MmcBlkDriver_SysFsOverrides)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "mmcblk0:0:0:0");
    mapinfo.insert("Model", "MMC_CARD");
    mapinfo.insert("Vendor", "");
    mapinfo.insert("Driver", "mmcblk");
    mapinfo.insert("Capacity", "64 GB (64021856256 bytes)");
    mapinfo.insert("Serial ID", "0x00070101");
    mapinfo.insert("SysFS Device Link", "/devices/platform/sdhci/ mmcblk0");
    m_stub.set(ADDR(Common, safeReadSysFsFile), ut_storage_safeReadSysFsFile);
    // Act
    bool result = m_deviceStorage->setHwinfoInfo(mapinfo);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("SD64G_CARD", m_deviceStorage->name().toStdString().c_str());
    EXPECT_STREQ("SanDisk", m_deviceStorage->vendor().toStdString().c_str());
}

// ===================== addInfoFromlshw（B1-B5） =====================
// 用例: bus info 格式非法返回 false（B1 无效输入等价类）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AddInfoFromlshw_MalformedBusInfo_ReturnsFalse)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "no-at-sign");
    // Act
    bool result = m_deviceStorage->addInfoFromlshw(mapinfo);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceStorage->keyFromStorage(), QString(""));
}

// 用例: 关键字与 m_KeyToLshw 不匹配返回 false（B2）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AddInfoFromlshw_KeyMismatch_ReturnsFalse)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setLshwInfo(mapinfo);
    m_deviceStorage->m_KeyToLshw = "9:9:9:9";
    // Act
    bool result = m_deviceStorage->addInfoFromlshw(mapinfo);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceStorage->m_KeyToLshw, QString("9:9:9:9"));
}

// 用例: 匹配后解析基本信息并提取唯一 key（B3-B5）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AddInfoFromlshw_Match_ParsesFields)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setLshwInfo(mapinfo);
    mapinfo.insert("bus info", "scsi@2:0.0.0");
    m_deviceStorage->m_KeyToLshw = "2:0:0:0";
    // Act
    bool result = m_deviceStorage->addInfoFromlshw(mapinfo);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("scsi2", m_deviceStorage->keyFromStorage().toStdString().c_str());
    EXPECT_STREQ("gpt-1.00 partitioned partitioned:gpt", m_deviceStorage->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("R013", m_deviceStorage->m_Version.toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
}

// ===================== addNVMEInfoFromlshw（B1-B3） =====================
// 用例: bus info 格式非法返回 false（B1）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AddNVMEInfoFromlshw_Malformed_ReturnsFalse)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "malformed");
    // Act
    bool result = m_deviceStorage->addNVMEInfoFromlshw(mapinfo);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceStorage->vendor(), QString(""));
}

// 用例: NvmeKey 不含 bus info 关键字返回 false（B2）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AddNVMEInfoFromlshw_KeyMismatch_ReturnsFalse)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "pci@0000:0d:00.0");
    mapinfo.insert("vendor", "Samsung");
    m_deviceStorage->m_NvmeKey = "/devices/nvme/nvme1";
    // Act
    bool result = m_deviceStorage->addNVMEInfoFromlshw(mapinfo);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceStorage->vendor(), QString(""));
}

// 用例: 匹配后设置厂商属性（B3）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AddNVMEInfoFromlshw_Match_SetsVendor)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "pci@0000:0d:00.0");
    mapinfo.insert("vendor", "Samsung");
    m_deviceStorage->m_NvmeKey = "/devices/pci0000:00/0000:0d:00.0/nvme/nvme0";
    // Act
    bool result = m_deviceStorage->addNVMEInfoFromlshw(mapinfo);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("Samsung", m_deviceStorage->vendor().toStdString().c_str());
}

// ===================== addInfoFromSmartctl =====================
// 用例: 设备文件匹配时解析 smartctl 信息
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AddInfoFromSmartctl_MatchingDevice_Parses)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setSmartCtlInfo_002(mapinfo);
    m_deviceStorage->m_DeviceFile = "/dev/nvme0n1";
    // Act
    bool result = m_deviceStorage->addInfoFromSmartctl("nvme0n1", mapinfo);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
    EXPECT_STREQ("M6CR013", m_deviceStorage->m_FirmwareVersion.toStdString().c_str());
}

// 用例: 设备文件不匹配返回 false
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AddInfoFromSmartctl_DifferentDevice_ReturnsFalse)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setSmartCtlInfo_001(mapinfo);
    m_deviceStorage->m_DeviceFile = "/dev/sda";
    // Act
    bool result = m_deviceStorage->addInfoFromSmartctl("nvme0n1", mapinfo);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceStorage->m_FirmwareVersion, QString(""));
}

// ===================== getInfoFromsmartctl =====================
// 用例: 传统 SATA 键值解析（固件/速率/媒体类型/容量/序列号）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetInfoFromsmartctl_LegacyKeys_ParsesAll)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setSmartCtlInfo_001(mapinfo);
    // Act
    m_deviceStorage->getInfoFromsmartctl(mapinfo);
    // Assert
    EXPECT_STREQ("M6CR013", m_deviceStorage->m_FirmwareVersion.toStdString().c_str());
    EXPECT_STREQ(" 6.0 Gb/s (current: 6.0 Gb/s)", m_deviceStorage->m_Speed.toStdString().c_str());
    EXPECT_STREQ("SSD", m_deviceStorage->m_MediaType.toStdString().c_str());
    EXPECT_STREQ("240 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
}

// 用例: NVME 键值解析（Total NVM Capacity + Namespace 容量取大值）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetInfoFromsmartctl_NvmeKeys_ParsesAll)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setSmartCtlInfo_002(mapinfo);
    // Act
    m_deviceStorage->getInfoFromsmartctl(mapinfo);
    // Assert
    EXPECT_STREQ("M6CR013", m_deviceStorage->m_FirmwareVersion.toStdString().c_str());
    EXPECT_STREQ("SSD", m_deviceStorage->m_MediaType.toStdString().c_str());
    EXPECT_STREQ("240 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
}

// 用例: 无任何型号键时序列号/固件仍然解析, 型号为空
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetInfoFromsmartctl_NoModelKeys_SerialParsed)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Firmware Version", "M6CR013");
    mapinfo.insert("SATA Version is", "SATA 3.2, 6.0 Gb/s");
    mapinfo.insert("Rotation Rate", "Solid State Device");
    mapinfo.insert("Serial Number", "2002E3E0B393");
    mapinfo.insert("Power_On_Hours", "3717");
    // Act
    m_deviceStorage->getInfoFromsmartctl(mapinfo);
    // Assert
    EXPECT_STREQ("M6CR013", m_deviceStorage->m_FirmwareVersion.toStdString().c_str());
    EXPECT_STREQ("2002E3E0B393", m_deviceStorage->m_SerialNumber.toStdString().c_str());
    EXPECT_STREQ("SSD", m_deviceStorage->m_MediaType.toStdString().c_str());
    EXPECT_TRUE(m_deviceStorage->name().isEmpty());
}

// 用例: 键值不足 5 个且非 USB 接口时直接返回不解析（B 守卫分支）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetInfoFromsmartctl_SmallMap_EarlyReturn)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Firmware Version", "M6CR013");
    mapinfo.insert("Serial Number", "2002E3E0B393");
    // Act
    m_deviceStorage->getInfoFromsmartctl(mapinfo);
    // Assert
    EXPECT_EQ(m_deviceStorage->m_FirmwareVersion, QString(""));
    EXPECT_EQ(m_deviceStorage->m_SerialNumber, QString(""));
}

// ===================== appendDisk（B1-B3） =====================
// 用例: 当前容量为 0 时采用对方磁盘容量（B1）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AppendDisk_EmptyCurrent_AdoptsOtherSize)
{
    // Arrange
    DeviceStorage other;
    other.m_SizeBytes = 240057409536ULL;
    // Act
    m_deviceStorage->appendDisk(&other);
    // Assert
    EXPECT_EQ(240057409536ULL, m_deviceStorage->getDiskSizeByte());
    EXPECT_NE(0ULL, m_deviceStorage->getDiskSizeByte());
}

// 用例: 双方容量均大于 0 时求和（B2）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AppendDisk_SecondDisk_SumsSizes)
{
    // Arrange
    m_deviceStorage->m_SizeBytes = 100000000000ULL;
    DeviceStorage other;
    other.m_SizeBytes = 200000000000ULL;
    // Act
    m_deviceStorage->appendDisk(&other);
    // Assert
    EXPECT_EQ(300000000000ULL, m_deviceStorage->getDiskSizeByte());
    EXPECT_EQ(300000000000ULL, m_deviceStorage->m_SizeBytes);
}

// 用例: 对方容量为 0 时不改变当前容量（B3 边界值）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_AppendDisk_OtherZeroSize_NoChange)
{
    // Arrange
    m_deviceStorage->m_SizeBytes = 100000000000ULL;
    DeviceStorage other;
    other.m_SizeBytes = 0;
    // Act
    m_deviceStorage->appendDisk(&other);
    // Assert
    EXPECT_EQ(100000000000ULL, m_deviceStorage->getDiskSizeByte());
    EXPECT_NE(0ULL, m_deviceStorage->getDiskSizeByte());
}

// ===================== checkDiskSize（B1-B9, hw 平台桩） =====================
// 用例: 非 hw 平台直接返回, 容量不变（B1）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_CheckDiskSize_NotHwPlatform_NoChange)
{
    // Arrange
    Common::specialComType = Common::Unknow;
    m_deviceStorage->m_Size = "245.7 GB";
    m_deviceStorage->m_SizeBytes = 245700000000ULL;
    m_deviceStorage->m_Interface = "SATA ";
    // Act
    m_deviceStorage->checkDiskSize();
    // Assert
    EXPECT_STREQ("245.7 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_EQ(245700000000ULL, m_deviceStorage->getDiskSizeByte());
}

// 用例: 256GB 区间规范为 "256 GB"（B2 边界值）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_CheckDiskSize_256GBRange_Sets256GB)
{
    // Arrange
    Common::specialComType = Common::PGUW;
    m_deviceStorage->m_Size = "256.5 GB";
    m_deviceStorage->m_SizeBytes = 256500000000ULL;
    m_deviceStorage->m_Interface = "SATA ";
    // Act
    m_deviceStorage->checkDiskSize();
    // Assert
    EXPECT_STREQ("256 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_EQ(256500000000ULL, m_deviceStorage->getDiskSizeByte());
}

// 用例: 1TB 区间规范为 "1 TB"（B4 边界值）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_CheckDiskSize_1TBRange_Sets1TB)
{
    // Arrange
    Common::specialComType = Common::PGUW;
    m_deviceStorage->m_Size = "1002.4 GB";
    m_deviceStorage->m_SizeBytes = 1002400000000ULL;
    m_deviceStorage->m_Interface = "SATA ";
    // Act
    m_deviceStorage->checkDiskSize();
    // Assert
    EXPECT_STREQ("1 TB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_EQ(1002400000000ULL, m_deviceStorage->getDiskSizeByte());
}

// 用例: 不在任何规范区间时保持原值（B2/B4 边界外等价类）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_CheckDiskSize_OutOfRanges_KeepsSize)
{
    // Arrange
    Common::specialComType = Common::PGUW;
    m_deviceStorage->m_Size = "300 GB";
    m_deviceStorage->m_SizeBytes = 300000000000ULL;
    m_deviceStorage->m_Interface = "SATA ";
    // Act
    m_deviceStorage->checkDiskSize();
    // Assert
    EXPECT_STREQ("300 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_FALSE(m_deviceStorage->m_Size.contains("256"));
}

// 用例: USB 接口 16GB 区间规范为 "16 GB"（B6）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_CheckDiskSize_UsbInterface_16GBRange)
{
    // Arrange
    Common::specialComType = Common::PGUW;
    m_deviceStorage->m_Size = "16.2 GB";
    m_deviceStorage->m_SizeBytes = 16200000000ULL;
    m_deviceStorage->m_Interface = "USB";
    // Act
    m_deviceStorage->checkDiskSize();
    // Assert
    EXPECT_STREQ("16 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_TRUE(m_deviceStorage->m_Interface.contains("USB"));
}

// 用例: USB 接口 128GB 区间规范为 "128 GB"（B9 边界值）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_CheckDiskSize_UsbInterface_128GBRange)
{
    // Arrange
    Common::specialComType = Common::PGUW;
    m_deviceStorage->m_Size = "128.2 GB";
    m_deviceStorage->m_SizeBytes = 128200000000ULL;
    m_deviceStorage->m_Interface = "USB";
    // Act
    m_deviceStorage->checkDiskSize();
    // Assert
    EXPECT_STREQ("128 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_NE(QString("128.2 GB"), m_deviceStorage->m_Size);
}

// ===================== getSerialID（B1/B2/B4/B5） =====================
// 用例: 空链接直接返回空（B1 边界值）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetSerialID_EmptyLink_ReturnsEmpty)
{
    // Arrange
    QString link;
    // Act
    QString serial = m_deviceStorage->getSerialID(link);
    // Assert
    EXPECT_TRUE(serial.isEmpty());
    EXPECT_EQ(serial, QString(""));
}

// 用例: platform 链接在本机无 /proc/bootdevice 时回退 unique_number 仍为空（B2/B4）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetSerialID_PlatformLink_NoFiles_ReturnsEmpty)
{
    // Arrange
    QString link = "/devices/platform/f8300000.ufs/host0/target0:0:0/0:0:0:3";
    // Act
    QString serial = m_deviceStorage->getSerialID(link);
    // Assert
    EXPECT_EQ(serial, QString(""));
    EXPECT_FALSE(link.isEmpty());
}

// ===================== getManfName / getOemName =====================
// 用例: MANFID 表已知 ID 映射厂商名
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetManfName_KnownId_ReturnsVendor)
{
    // Arrange
    const QString rawManfId = "0x000003";
    // Act
    QString vendor = m_deviceStorage->getManfName(rawManfId);
    // Assert
    EXPECT_STREQ("SanDisk", vendor.toStdString().c_str());
    EXPECT_FALSE(vendor.isEmpty());
}

// 用例: MANFID 表未知 ID 返回原始值（无效输入等价类）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetManfName_UnknownId_ReturnsRaw)
{
    // Arrange
    const QString rawManfId = "0xdead";
    // Act
    QString vendor = m_deviceStorage->getManfName(rawManfId);
    // Assert
    EXPECT_STREQ("0xdead", vendor.toStdString().c_str());
    EXPECT_EQ(vendor, QString("0xdead"));
}

// 用例: OEMID 表已知 ID 映射厂商名
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetOemName_KnownId_ReturnsName)
{
    // Arrange
    const QString rawOemId = "0x5054";
    // Act
    QString vendor = m_deviceStorage->getOemName(rawOemId);
    // Assert
    EXPECT_STREQ("SanDisk", vendor.toStdString().c_str());
    EXPECT_FALSE(vendor.isEmpty());
}

// 用例: OEMID 表未知 ID 返回原始值（无效输入等价类）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetOemName_UnknownId_ReturnsRaw)
{
    // Arrange
    const QString rawOemId = "0xzzzz";
    // Act
    QString vendor = m_deviceStorage->getOemName(rawOemId);
    // Assert
    EXPECT_STREQ("0xzzzz", vendor.toStdString().c_str());
    EXPECT_EQ(vendor, QString("0xzzzz"));
}

// ===================== unitConvertByDecimal =====================
// 用例: 容量为 0 时不换算（边界值）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_UnitConvertByDecimal_ZeroSize_KeepsSize)
{
    // Arrange
    m_deviceStorage->m_Size = "";
    m_deviceStorage->m_SizeBytes = 0;
    // Act
    m_deviceStorage->unitConvertByDecimal();
    // Assert
    EXPECT_TRUE(m_deviceStorage->m_Size.isEmpty());
    EXPECT_EQ(0ULL, m_deviceStorage->getDiskSizeByte());
}

// 用例: 容量大于 0 时按十进制换算为 "240 GB"
TEST_F(UT_DeviceStorage, UT_DeviceStorage_UnitConvertByDecimal_Positive_Converts)
{
    // Arrange
    m_deviceStorage->m_Size = "";
    m_deviceStorage->m_SizeBytes = 240057409536ULL;
    // Act
    m_deviceStorage->unitConvertByDecimal();
    // Assert
    EXPECT_STREQ("240 GB", m_deviceStorage->m_Size.toStdString().c_str());
    EXPECT_FALSE(m_deviceStorage->m_Size.isEmpty());
}

// ===================== getOverviewInfo（B1-B3） =====================
// 用例: UFS 接口概况附加 UFS 标识（B1）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetOverviewInfo_UfsInterface_AppendsUfs)
{
    // Arrange
    m_deviceStorage->m_Size = "240 GB";
    m_deviceStorage->m_Interface = "UFS";
    // Act
    QString overview = m_deviceStorage->getOverviewInfo();
    // Assert
    EXPECT_STREQ("240 GB UFS", overview.toStdString().c_str());
    EXPECT_TRUE(overview.contains("UFS"));
}

// 用例: USB 接口概况附加 USB 标识（B2）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetOverviewInfo_UsbInterface_AppendsUsb)
{
    // Arrange
    m_deviceStorage->m_Size = "16 GB";
    m_deviceStorage->m_Interface = "USB";
    // Act
    QString overview = m_deviceStorage->getOverviewInfo();
    // Assert
    EXPECT_STREQ("16 GB USB", overview.toStdString().c_str());
    EXPECT_TRUE(overview.contains("USB"));
}

// 用例: 默认接口概况附加媒体类型（B3）
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetOverviewInfo_Default_AppendsMediaType)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    m_deviceStorage->setHwinfoInfo(mapinfo);
    // Act
    QString overview = m_deviceStorage->getOverviewInfo();
    // Assert
    EXPECT_STREQ("240 GB ", overview.toStdString().c_str());
    EXPECT_TRUE(overview.startsWith("240 GB"));
}

// ===================== getter 与加载接口 =====================
// 用例: 初始化过滤键后包含 11 个键
TEST_F(UT_DeviceStorage, UT_DeviceStorage_InitFilterKey_FillsEleven)
{
    // Arrange
    int keyCount = 0;
    // Act
    m_deviceStorage->initFilterKey();
    keyCount = m_deviceStorage->m_FilterKey.size();
    // Assert
    EXPECT_EQ(11, keyCount);
    EXPECT_FALSE(m_deviceStorage->m_FilterKey.isEmpty());
}

// 用例: 子标题返回设备名称
TEST_F(UT_DeviceStorage, UT_DeviceStorage_SubTitle_ReturnsName)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_storage_setHwinfoInfo(mapinfo);
    m_deviceStorage->setHwinfoInfo(mapinfo);
    // Act
    QString title = m_deviceStorage->subTitle();
    // Assert
    EXPECT_STREQ("ST240BX500SSD1", title.toStdString().c_str());
    EXPECT_EQ(title, m_deviceStorage->name());
}

// 用例: 表头与表数据各 4 项
TEST_F(UT_DeviceStorage, UT_DeviceStorage_LoadTableHeaderAndData_FourRows)
{
    // Arrange
    int headerCount = 0;
    int dataCount = 0;
    // Act
    m_deviceStorage->loadTableHeader();
    m_deviceStorage->loadTableData();
    headerCount = m_deviceStorage->m_TableHeader.size();
    dataCount = m_deviceStorage->m_TableData.size();
    // Assert
    EXPECT_EQ(4, headerCount);
    EXPECT_EQ(4, dataCount);
}

// 用例: 无附加信息时基础信息列表为空
TEST_F(UT_DeviceStorage, UT_DeviceStorage_LoadBaseDeviceInfo_EmptyList)
{
    // Arrange
    int baseCount = -1;
    // Act
    m_deviceStorage->loadBaseDeviceInfo();
    baseCount = m_deviceStorage->m_LstBaseInfo.size();
    // Assert
    EXPECT_EQ(0, baseCount);
    EXPECT_TRUE(m_deviceStorage->m_LstBaseInfo.isEmpty());
}

// 用例: 转速率设置后加载其他信息列表为 1 项
TEST_F(UT_DeviceStorage, UT_DeviceStorage_LoadOtherDeviceInfo_RotationRate_OneRow)
{
    // Arrange
    m_deviceStorage->m_RotationRate = "Solid State Device";
    // Act
    m_deviceStorage->loadOtherDeviceInfo();
    // Assert
    EXPECT_EQ(1, m_deviceStorage->m_LstOtherInfo.size());
    EXPECT_FALSE(m_deviceStorage->m_LstOtherInfo.isEmpty());
}

// 用例: getDiskSizeByte 返回内部字节数
TEST_F(UT_DeviceStorage, UT_DeviceStorage_GetDiskSizeByte_ReturnsSizeBytes)
{
    // Arrange
    m_deviceStorage->m_SizeBytes = 123456789ULL;
    // Act
    quint64 bytes = m_deviceStorage->getDiskSizeByte();
    // Assert
    EXPECT_EQ(123456789ULL, bytes);
    EXPECT_EQ(m_deviceStorage->m_SizeBytes, bytes);
}
