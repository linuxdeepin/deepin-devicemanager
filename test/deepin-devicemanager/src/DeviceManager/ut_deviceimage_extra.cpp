// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceImage.h"
#include "DBusEnableInterface.h"
#include "DBusInterface.h"

#include "stub.h"

#include <QCoreApplication>
#include <gtest/gtest.h>

/**
 * @brief UT_DeviceImageExtra 补充覆盖 DeviceImage:
 * vendor / setInfoFromTomlOneByOne / setInfoFromHwinfo 的 unique_id 分支 /
 * loadOtherDeviceInfo 的序列号差异分支 / setEnable 成功与失败路径。
 * DBus 调用(DBusEnableInterface::enable 与 DBusInterface::refreshInfo)统一
 * 打桩,不依赖真实设备环境。
 */
class UT_DeviceImageExtra : public ::testing::Test
{
public:
    void SetUp()
    {
        m_deviceImage = new DeviceImage;
        ut_imgextra_refresh_count = 0;
    }
    void TearDown()
    {
        delete m_deviceImage;
    }
    DeviceImage *m_deviceImage;
    static int ut_imgextra_refresh_count;
};

int UT_DeviceImageExtra::ut_imgextra_refresh_count = 0;

/**
 * @brief 打桩:DBusEnableInterface::enable 返回成功
 */
static bool ut_imgextra_enable_true(const QString &, const QString &, const QString &, const QString &, bool, const QString &)
{
    return true;
}

/**
 * @brief 打桩:DBusEnableInterface::enable 返回失败
 */
static bool ut_imgextra_enable_false(const QString &, const QString &, const QString &, const QString &, bool, const QString &)
{
    return false;
}

/**
 * @brief 打桩:DBusInterface::refreshInfo 仅计数,不发起真实 DBus 调用
 */
static void ut_imgextra_refreshinfo()
{
    ++UT_DeviceImageExtra::ut_imgextra_refresh_count;
}

TEST_F(UT_DeviceImageExtra, UT_DeviceImageExtra_vendor_001)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Vendor", "Chicony");
    mapInfo.insert("Device", "Camera Device");
    mapInfo.insert("Serial ID", "SN03");
    mapInfo.insert("Unique ID", "UID03");

    // Act
    m_deviceImage->setInfoFromHwinfo(mapInfo);
    const QString &vendor = m_deviceImage->vendor();

    // Assert
    EXPECT_STREQ("Chicony", vendor.toStdString().c_str());
    EXPECT_STREQ("Chicony", m_deviceImage->m_Vendor.toStdString().c_str());
}

TEST_F(UT_DeviceImageExtra, UT_DeviceImageExtra_setInfoFromTomlOneByOne_001)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", "ModelI");
    mapInfo.insert("Version", "2.0");
    mapInfo.insert("Serial Number", "SN321");
    m_deviceImage->m_SerialID = "OLD";

    // Act
    TomlFixMethod ret = m_deviceImage->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_STREQ("SN321", m_deviceImage->m_SerialID.toStdString().c_str());
    EXPECT_STREQ("ModelI", m_deviceImage->m_Model.toStdString().c_str());
}

TEST_F(UT_DeviceImageExtra, UT_DeviceImageExtra_setInfoFromTomlOneByOne_002)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Serial Number", "nouse");
    m_deviceImage->m_SerialID = "OLD";

    // Act
    TomlFixMethod ret = m_deviceImage->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_nouse, ret);
    EXPECT_STREQ("", m_deviceImage->m_SerialID.toStdString().c_str());
}

TEST_F(UT_DeviceImageExtra, UT_DeviceImageExtra_setInfoFromTomlOneByOne_003)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", "ModelI");
    m_deviceImage->m_SerialID = "OLD";

    // Act
    TomlFixMethod ret = m_deviceImage->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_None, ret);
    EXPECT_STREQ("OLD", m_deviceImage->m_SerialID.toStdString().c_str());
}

TEST_F(UT_DeviceImageExtra, UT_DeviceImageExtra_setInfoFromHwinfo_003)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("unique_id", "UID7");
    mapInfo.insert("name", "Integrated Camera");
    mapInfo.insert("path", "/sys/devices/camera");
    mapInfo.insert("Hardware Class", "camera");

    // Act
    m_deviceImage->setInfoFromHwinfo(mapInfo);

    // Assert
    EXPECT_STREQ("UID7", m_deviceImage->m_UniqueID.toStdString().c_str());
    EXPECT_STREQ("Integrated Camera", m_deviceImage->m_Name.toStdString().c_str());
    EXPECT_STREQ("/sys/devices/camera", m_deviceImage->m_SysPath.toStdString().c_str());
    EXPECT_STREQ("camera", m_deviceImage->m_HardwareClass.toStdString().c_str());
    EXPECT_FALSE(m_deviceImage->enable());
    EXPECT_TRUE(m_deviceImage->canUninstall());
}

TEST_F(UT_DeviceImageExtra, UT_DeviceImageExtra_loadOtherDeviceInfo_002)
{
    // Arrange
    m_deviceImage->m_Modalias = "MOD001";
    m_deviceImage->m_PhysID = "PH001";
    m_deviceImage->m_Speed = "480 Mb/s";
    m_deviceImage->m_MaximumPower = "500mA";
    m_deviceImage->m_Driver = "uvcvideo";
    m_deviceImage->m_Capabilities = "usb";
    m_deviceImage->m_SerialID = "SN1";
    m_deviceImage->m_UniqueID = "UID1";

    // Act
    m_deviceImage->loadOtherDeviceInfo();

    // Assert
    EXPECT_EQ(7, m_deviceImage->m_LstOtherInfo.size());
    QStringList keys;
    for (int i = 0; i < m_deviceImage->m_LstOtherInfo.size(); ++i)
        keys << m_deviceImage->m_LstOtherInfo.at(i).first;
    EXPECT_TRUE(keys.contains("Serial Number"));
    EXPECT_TRUE(keys.contains("Module Alias"));
    EXPECT_TRUE(keys.contains("Driver"));
}

TEST_F(UT_DeviceImageExtra, UT_DeviceImageExtra_setEnable_003)
{
    // Arrange
    m_deviceImage->m_SerialID = "SerialID";
    m_deviceImage->m_UniqueID = "UniqueID";
    m_deviceImage->m_SysPath = "SysPath";
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_imgextra_enable_true);
    stub.set(ADDR(DBusInterface, refreshInfo), ut_imgextra_refreshinfo);

    // Act
    EnableDeviceStatus status = m_deviceImage->setEnable(true);

    // Assert
    EXPECT_EQ(EnableDeviceStatus::EDS_Success, status);
    EXPECT_TRUE(m_deviceImage->enable());
    EXPECT_EQ(1, ut_imgextra_refresh_count);
}

TEST_F(UT_DeviceImageExtra, UT_DeviceImageExtra_setEnable_004)
{
    // Arrange
    m_deviceImage->m_SerialID = "SerialID";
    m_deviceImage->m_UniqueID = "UniqueID";
    m_deviceImage->m_SysPath = "SysPath";
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_imgextra_enable_false);
    stub.set(ADDR(DBusInterface, refreshInfo), ut_imgextra_refreshinfo);

    // Act
    EnableDeviceStatus status = m_deviceImage->setEnable(false);

    // Assert
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, status);
    EXPECT_EQ(0, ut_imgextra_refresh_count);
}

/**
 * @brief 声明表:本文件新增用例与被测方法映射
 * | method | level | factors | min | actual |
 * |---|---|---|---|---|
 * | vendor | low | - | 1 | 1 |
 * | setInfoFromTomlOneByOne | mid | in_degree:1 | 2 | 3 |
 * | setInfoFromHwinfo | mid | lines:51 | 2 | 2 |
 * | loadOtherDeviceInfo | low | - | 1 | 1 |
 * | setEnable | low | - | 1 | 2 |
 */
