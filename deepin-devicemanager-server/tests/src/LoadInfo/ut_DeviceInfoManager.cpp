// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "../stub.h"
#include "deviceinfomanager.h"
#include "DDLog.h"

#include <QString>

/*
 * Branch list for DeviceInfoManager:
 * addInfo:
 *   B1: key not in map -> insert                           | AddInfo_NewKey_InsertsValue
 *   B2: key already in map -> overwrite                    | AddInfo_ExistingKey_OverwritesValue
 * getInfo:
 *   B3: key in map -> return value                         | AddInfo_NewKey_InsertsValue
 *   B4: key not in map -> return empty string              | GetInfo_MissingKey_ReturnsEmpty
 * isInfoExisted:
 *   B5: key in map -> true                                 | IsInfoExisted_ExistingKey_ReturnsTrue
 *   B6: key not in map -> false                            | IsInfoExisted_MissingKey_ReturnsFalse
 * isPathExisted:
 *   B7: no "hwinfo" key -> return false                    | IsPathExisted_NoHwinfo_ReturnsFalse
 *   B8: replace "/sys" then check contains -> true/false   | IsPathExisted_PathInHwinfo_ReturnsTrue, IsPathExisted_PathNotInHwinfo_ReturnsFalse
 */

class DeviceInfoManager_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
        // DeviceInfoManager is a process-wide singleton; tests use a private
        // key namespace to avoid interference with other test suites.
    }
    void TearDown() override
    {
    }
};

// ===== Singleton =====

TEST_F(DeviceInfoManager_UT, GetInstance_FirstCall_ReturnsNonNull)
{
    // Arrange
    QString testKey = "ut_dim_uninit_key";
    // Act
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    // Assert
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->isInfoExisted(testKey), false);
}

TEST_F(DeviceInfoManager_UT, GetInstance_SecondCall_ReturnsSamePointer)
{
    // Arrange
    DeviceInfoManager *first = DeviceInfoManager::getInstance();
    // Act
    DeviceInfoManager *second = DeviceInfoManager::getInstance();
    // Assert
    EXPECT_EQ(first, second);
    EXPECT_EQ(first, DeviceInfoManager::getInstance());
}

// ===== addInfo / getInfo / isInfoExisted =====

TEST_F(DeviceInfoManager_UT, AddInfo_NewKey_InsertsValue)
{
    // Arrange
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    // Act
    inst->addInfo("ut_dim_key_basic", "value123");
    // Assert
    EXPECT_EQ(inst->getInfo("ut_dim_key_basic"), QString("value123"));
    EXPECT_TRUE(inst->isInfoExisted("ut_dim_key_basic"));
}

TEST_F(DeviceInfoManager_UT, AddInfo_ExistingKey_OverwritesValue)
{
    // Arrange
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    inst->addInfo("ut_dim_key_overwrite", "first");
    // Act
    inst->addInfo("ut_dim_key_overwrite", "second");
    // Assert
    EXPECT_EQ(inst->getInfo("ut_dim_key_overwrite"), QString("second"));
    EXPECT_NE(inst->getInfo("ut_dim_key_overwrite"), QString("first"));
}

TEST_F(DeviceInfoManager_UT, AddInfo_EmptyValue_StoresEmptyString)
{
    // Arrange
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    // Act
    inst->addInfo("ut_dim_key_empty", "");
    // Assert
    EXPECT_TRUE(inst->isInfoExisted("ut_dim_key_empty"));
    EXPECT_EQ(inst->getInfo("ut_dim_key_empty").length(), 0);
}

TEST_F(DeviceInfoManager_UT, GetInfo_MissingKey_ReturnsEmpty)
{
    // Arrange
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    // Act
    const QString &val = inst->getInfo("ut_dim_key_nonexistent");
    // Assert
    EXPECT_TRUE(val.isEmpty());
    EXPECT_EQ(val.length(), 0);
}

TEST_F(DeviceInfoManager_UT, IsInfoExisted_ExistingKey_ReturnsTrue)
{
    // Arrange
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    inst->addInfo("ut_dim_key_exists", "yes");
    // Act
    bool result = inst->isInfoExisted("ut_dim_key_exists");
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(inst->getInfo("ut_dim_key_exists"), QString("yes"));
}

TEST_F(DeviceInfoManager_UT, IsInfoExisted_MissingKey_ReturnsFalse)
{
    // Arrange
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    // Act
    bool result = inst->isInfoExisted("ut_dim_key_missing");
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(inst->getInfo("ut_dim_key_missing").length(), 0);
}

TEST_F(DeviceInfoManager_UT, AddInfo_MultipleKeys_AllAccessible)
{
    // Arrange
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    // Act
    inst->addInfo("ut_dim_k1", "1");
    inst->addInfo("ut_dim_k2", "2");
    inst->addInfo("ut_dim_k3", "3");
    // Assert
    EXPECT_EQ(inst->getInfo("ut_dim_k1"), QString("1"));
    EXPECT_EQ(inst->getInfo("ut_dim_k2"), QString("2"));
    EXPECT_EQ(inst->getInfo("ut_dim_k3"), QString("3"));
}

// ===== isPathExisted =====

TEST_F(DeviceInfoManager_UT, IsPathExisted_NoHwinfo_ReturnsFalse)
{
    // Arrange — no hwinfo populated for this key namespace
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    // Act
    bool result = inst->isPathExisted("/sys/devices/ut_dim_nonexistent_path");
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(inst->getInfo("hwinfo").length(), 0);
}

TEST_F(DeviceInfoManager_UT, IsPathExisted_PathInHwinfo_ReturnsTrue)
{
    // Arrange — isPathExisted strips the leading "/sys" and checks substring
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    inst->addInfo("hwinfo", "P: /devices/pci0000:00/0000:00:1f.2\n"
                            "E: ID_VENDOR=Generic\n");
    // Act
    bool result = inst->isPathExisted("/sys/devices/pci0000:00/0000:00:1f.2");
    // Assert
    EXPECT_TRUE(result);
    EXPECT_GT(inst->getInfo("hwinfo").length(), 0);
}

TEST_F(DeviceInfoManager_UT, IsPathExisted_StripsSysPrefix_MatchesSubstring)
{
    // Arrange — verify the /sys stripping behaviour explicitly
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    inst->addInfo("hwinfo", "some /devices/usb1 data here");
    // Act
    bool matched = inst->isPathExisted("/sys/devices/usb1");
    bool unmatched = inst->isPathExisted("/sys/devices/usb2");
    // Assert
    EXPECT_TRUE(matched);
    EXPECT_GT(inst->getInfo("hwinfo").length(), 0);
}

TEST_F(DeviceInfoManager_UT, IsPathExisted_PathNotInHwinfo_ReturnsFalse)
{
    // Arrange
    DeviceInfoManager *inst = DeviceInfoManager::getInstance();
    inst->addInfo("hwinfo", "only /devices/pci0000:00 content");
    // Act
    bool result = inst->isPathExisted("/sys/devices/usb/ffff");
    // Assert
    EXPECT_FALSE(result);
    EXPECT_GT(inst->getInfo("hwinfo").length(), 0);
}
