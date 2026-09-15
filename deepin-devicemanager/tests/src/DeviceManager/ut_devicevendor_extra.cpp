// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 增量补全：集中覆盖各 DeviceXxx 子类的 vendor()/chip_name()/tomlname() 访问器。
// 这些均为返回成员变量的简单 getter，利用 -fno-access-control 直接设置基类/子类
// 受保护成员后调用 getter 校验返回值。

#include "DeviceInfo.h"
#include "DeviceAudio.h"
#include "DeviceBios.h"
#include "DeviceBluetooth.h"
#include "DeviceCdrom.h"
#include "DeviceComputer.h"
#include "DeviceGpu.h"
#include "DeviceImage.h"
#include "DeviceInput.h"
#include "DeviceMemory.h"
#include "DeviceMonitor.h"
#include "DeviceOtherPCI.h"
#include "DeviceOthers.h"
#include "DevicePower.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>

#include <gtest/gtest.h>

// DeviceAudio::chip_name() -> m_Chip ; vendor() -> m_Vendor
TEST(UT_DeviceVendorExtra, DeviceAudio_ChipNameAndVendor_ReturnMembers)
{
    DeviceAudio *d = new DeviceAudio;
    d->m_Chip = "ALC892";
    d->m_Vendor = "Realtek";
    EXPECT_STREQ("ALC892", d->chip_name().toStdString().c_str());
    EXPECT_STREQ("Realtek", d->vendor().toStdString().c_str());
    delete d;
}

// DeviceBios::tomlname() -> m_tomlName
TEST(UT_DeviceVendorExtra, DeviceBios_Tomlname_ReturnsMember)
{
    DeviceBios *d = new DeviceBios;
    d->m_tomlName = "bios";
    EXPECT_STREQ("bios", d->tomlname().toStdString().c_str());
    delete d;
}

// DeviceBluetooth::vendor()
TEST(UT_DeviceVendorExtra, DeviceBluetooth_Vendor_ReturnsMember)
{
    DeviceBluetooth *d = new DeviceBluetooth;
    d->m_Vendor = "Intel";
    EXPECT_STREQ("Intel", d->vendor().toStdString().c_str());
    delete d;
}

// DeviceCdrom::vendor()
TEST(UT_DeviceVendorExtra, DeviceCdrom_Vendor_ReturnsMember)
{
    DeviceCdrom *d = new DeviceCdrom;
    d->m_Vendor = "ASUS";
    EXPECT_STREQ("ASUS", d->vendor().toStdString().c_str());
    delete d;
}

// DeviceComputer::vendor()
TEST(UT_DeviceVendorExtra, DeviceComputer_Vendor_ReturnsMember)
{
    DeviceComputer *d = new DeviceComputer;
    d->m_Vendor = "Dell";
    EXPECT_STREQ("Dell", d->vendor().toStdString().c_str());
    delete d;
}

// DeviceGpu::vendor()
TEST(UT_DeviceVendorExtra, DeviceGpu_Vendor_ReturnsMember)
{
    DeviceGpu *d = new DeviceGpu;
    d->m_Vendor = "NVIDIA";
    EXPECT_STREQ("NVIDIA", d->vendor().toStdString().c_str());
    delete d;
}

// DeviceImage::vendor()
TEST(UT_DeviceVendorExtra, DeviceImage_Vendor_ReturnsMember)
{
    DeviceImage *d = new DeviceImage;
    d->m_Vendor = "Canon";
    EXPECT_STREQ("Canon", d->vendor().toStdString().c_str());
    delete d;
}

// DeviceInput::vendor()
TEST(UT_DeviceVendorExtra, DeviceInput_Vendor_ReturnsMember)
{
    DeviceInput *d = new DeviceInput;
    d->m_Vendor = "Logitech";
    EXPECT_STREQ("Logitech", d->vendor().toStdString().c_str());
    delete d;
}

// DeviceMemory::vendor()
TEST(UT_DeviceVendorExtra, DeviceMemory_Vendor_ReturnsMember)
{
    DeviceMemory *d = new DeviceMemory;
    d->m_Vendor = "Kingston";
    EXPECT_STREQ("Kingston", d->vendor().toStdString().c_str());
    delete d;
}

// DeviceMonitor::vendor()
TEST(UT_DeviceVendorExtra, DeviceMonitor_Vendor_ReturnsMember)
{
    DeviceMonitor *d = new DeviceMonitor;
    d->m_Vendor = "Dell";
    EXPECT_STREQ("Dell", d->vendor().toStdString().c_str());
    delete d;
}

// DeviceOtherPCI::vendor()
TEST(UT_DeviceVendorExtra, DeviceOtherPCI_Vendor_ReturnsMember)
{
    DeviceOtherPCI *d = new DeviceOtherPCI;
    d->m_Vendor = "Broadcom";
    EXPECT_STREQ("Broadcom", d->vendor().toStdString().c_str());
    delete d;
}

// DeviceOthers::vendor()
TEST(UT_DeviceVendorExtra, DeviceOthers_Vendor_ReturnsMember)
{
    DeviceOthers *d = new DeviceOthers;
    d->m_Vendor = "Other";
    EXPECT_STREQ("Other", d->vendor().toStdString().c_str());
    delete d;
}

// DevicePower::vendor()
TEST(UT_DeviceVendorExtra, DevicePower_Vendor_ReturnsMember)
{
    DevicePower *d = new DevicePower;
    d->m_Vendor = "APC";
    EXPECT_STREQ("APC", d->vendor().toStdString().c_str());
    delete d;
}
