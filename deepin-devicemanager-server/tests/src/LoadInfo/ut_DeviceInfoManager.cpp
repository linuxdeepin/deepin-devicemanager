// SPDX-FileCopyrightText: 2019 ~ 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "../stub.h"
#include "deviceinfomanager.h"
#include "DDLog.h"

using namespace DDLog;

class DeviceInfoManager_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_dm = DeviceInfoManager::getInstance();
    }
    void TearDown()
    {
    }
    DeviceInfoManager *m_dm;
};

TEST_F(DeviceInfoManager_UT, IsPathExisted_StripsSysPrefix_MatchesSubstring)
{
    QString hwinfo =
        "P: /devices/pci0000:00/0000:00:01.0\n"
        "E: ID_VENDOR=Linux\n";
    m_dm->addInfo("hwinfo", hwinfo);

    EXPECT_TRUE(m_dm->isPathExisted("/sys/devices/pci0000:00/0000:00:01.0"));
}

TEST_F(DeviceInfoManager_UT, IsPathExisted_PathNotInHwinfo_ReturnsFalse)
{
    QString hwinfo =
        "P: /devices/pci0000:00/0000:00:01.0\n"
        "E: ID_VENDOR=Linux\n";
    m_dm->addInfo("hwinfo", hwinfo);

    EXPECT_FALSE(m_dm->isPathExisted("/sys/devices/nonexistent"));
}

TEST_F(DeviceInfoManager_UT, IsPathExisted_MultipleSysInPath_OnlyPrefixStripped)
{
    QString hwinfo =
        "P: /sys/devices/usb1\n"
        "E: ID_VENDOR=Linux\n";
    m_dm->addInfo("hwinfo", hwinfo);

    EXPECT_TRUE(m_dm->isPathExisted("/sys/sys/devices/usb1"));
}
