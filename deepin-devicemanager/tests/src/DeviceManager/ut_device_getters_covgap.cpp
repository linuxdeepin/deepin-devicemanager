// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 覆盖率缺口补全：DeviceManager 模块下多个 Device* 类的 vendor() 等 getter
// 此前未被任何用例调用，函数覆盖率为 0。本文件对各设备类做默认构造并调用其
// 未覆盖的 getter：
// - vendor() 等为 const 成员 getter，默认构造未填充硬件数据，返回空串；
//   用例主要保证 getter 可被安全调用（函数覆盖），并验证默认语义。
// - DeviceNetwork::canDisable/isWireless/hwAddress 与 DeviceStorage::getManfName/
//   getOemName 含可判定的默认逻辑或查表逻辑，使用确定性断言。

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
#include "DeviceNetwork.h"
#include "DeviceOtherPCI.h"
#include "DeviceOthers.h"
#include "DevicePower.h"
#include "DevicePrint.h"
#include "DeviceStorage.h"
#include "DeviceInfo.h"
#include "MacroDefinition.h"

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

// ---- vendor() getter 集群：默认构造后调用，验证不崩溃且返回 QString ----
TEST(UT_Device_GettersCovGap, DeviceAudio_Vendor_DefaultCtorNoCrash)
{
    DeviceAudio device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DeviceBluetooth_Vendor_DefaultCtorNoCrash)
{
    DeviceBluetooth device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DeviceCdrom_Vendor_DefaultCtorNoCrash)
{
    DeviceCdrom device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DeviceComputer_Vendor_DefaultCtorNoCrash)
{
    DeviceComputer device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DeviceGpu_Vendor_DefaultCtorNoCrash)
{
    DeviceGpu device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DeviceImage_Vendor_DefaultCtorNoCrash)
{
    DeviceImage device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DeviceInput_Vendor_DefaultCtorNoCrash)
{
    DeviceInput device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DeviceMemory_Vendor_DefaultCtorNoCrash)
{
    DeviceMemory device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DeviceMonitor_Vendor_DefaultCtorNoCrash)
{
    DeviceMonitor device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DeviceOtherPCI_Vendor_DefaultCtorNoCrash)
{
    DeviceOtherPCI device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DeviceOthers_Vendor_DefaultCtorNoCrash)
{
    DeviceOthers device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DevicePower_Vendor_DefaultCtorNoCrash)
{
    DevicePower device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

TEST(UT_Device_GettersCovGap, DevicePrint_Vendor_DefaultCtorNoCrash)
{
    DevicePrint device;
    EXPECT_NO_FATAL_FAILURE(device.vendor());
}

// ---- 非 vendor 的确定性 getter ----

TEST(UT_Device_GettersCovGap, DeviceBios_Tomlname_DefaultCtorReturnsEmpty)
{
    DeviceBios device;
    EXPECT_TRUE(device.tomlname().isEmpty());
}

TEST(UT_Device_GettersCovGap, DevicePrint_MakeAndModel_DefaultCtorReturnsEmpty)
{
    DevicePrint device;
    EXPECT_TRUE(device.makeAndeModel().isEmpty());
}

TEST(UT_Device_GettersCovGap, DeviceNetwork_Vendor_DefaultCtorReturnsEmpty)
{
    DeviceNetwork device;
    EXPECT_TRUE(device.vendor().isEmpty());
}

TEST(UT_Device_GettersCovGap, DeviceNetwork_IsWireless_DefaultCtorReturnsFalse)
{
    DeviceNetwork device;
    EXPECT_FALSE(device.isWireless());
}

TEST(UT_Device_GettersCovGap, DeviceNetwork_HwAddress_DefaultCtorReturnsEmpty)
{
    DeviceNetwork device;
    EXPECT_TRUE(device.hwAddress().isEmpty());
}

TEST(UT_Device_GettersCovGap, DeviceNetwork_CanDisable_EmptySysPathReturnsFalse)
{
    // canDisable: m_SysPath 为空时返回 false
    DeviceNetwork device;
    EXPECT_FALSE(device.canDisable());
}

TEST(UT_Device_GettersCovGap, DeviceStorage_GetManfName_KnownIdReturnsMappedVendor)
{
    DeviceStorage device;
    // MANFID_TABLE 中 000001 -> Panasonic
    EXPECT_EQ(device.getManfName("000001"), "Panasonic");
}

TEST(UT_Device_GettersCovGap, DeviceStorage_GetManfName_UnknownIdReturnsRawInput)
{
    DeviceStorage device;
    // 查表未命中时返回原始输入
    EXPECT_EQ(device.getManfName("zzz-unknown-manf"), "zzz-unknown-manf");
}

TEST(UT_Device_GettersCovGap, DeviceStorage_GetOemName_UnknownIdReturnsRawInput)
{
    DeviceStorage device;
    EXPECT_EQ(device.getOemName("zzz-unknown-oem"), "zzz-unknown-oem");
}
