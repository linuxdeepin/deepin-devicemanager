// SPDX-FileCopyrightText: 2026 ~ 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "../stub.h"
#include "cpu/logicalcpu.h"
#include "DDLog.h"

using namespace DDLog;

class LogicalCpu_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

// ===== Constructor / Default Values =====

TEST_F(LogicalCpu_UT, DefaultConstructor_InitializesIDsToNegativeOne)
{
    LogicalCpu cpu;
    EXPECT_EQ(cpu.physicalID(), -1);
    EXPECT_EQ(cpu.coreID(), -1);
    EXPECT_EQ(cpu.logicalID(), -1);
}

TEST_F(LogicalCpu_UT, DefaultConstructor_AllStringFieldsEmpty)
{
    LogicalCpu cpu;
    EXPECT_TRUE(cpu.l1dCache().isEmpty());
    EXPECT_TRUE(cpu.l1iCache().isEmpty());
    EXPECT_TRUE(cpu.l2Cache().isEmpty());
    EXPECT_TRUE(cpu.l3Cache().isEmpty());
    EXPECT_TRUE(cpu.l4Cache().isEmpty());
    EXPECT_TRUE(cpu.l1dSharedCpuList().isEmpty());
    EXPECT_TRUE(cpu.l1iSharedCpuList().isEmpty());
    EXPECT_TRUE(cpu.l2SharedCpuList().isEmpty());
    EXPECT_TRUE(cpu.l3SharedCpuList().isEmpty());
    EXPECT_TRUE(cpu.l4SharedCpuList().isEmpty());
    EXPECT_TRUE(cpu.maxFreq().isEmpty());
    EXPECT_TRUE(cpu.minFreq().isEmpty());
    EXPECT_TRUE(cpu.curFreq().isEmpty());
    EXPECT_TRUE(cpu.model().isEmpty());
    EXPECT_TRUE(cpu.modelName().isEmpty());
    EXPECT_TRUE(cpu.stepping().isEmpty());
    EXPECT_TRUE(cpu.vendor().isEmpty());
    EXPECT_TRUE(cpu.cpuFamliy().isEmpty());
    EXPECT_TRUE(cpu.flags().isEmpty());
    EXPECT_TRUE(cpu.bogomips().isEmpty());
    EXPECT_TRUE(cpu.arch().isEmpty());
}

// ===== ID Setters / Getters =====

TEST_F(LogicalCpu_UT, SetPhysicalID_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setPhysicalID(3);
    EXPECT_EQ(cpu.physicalID(), 3);
}

TEST_F(LogicalCpu_UT, SetCoreID_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setCoreID(7);
    EXPECT_EQ(cpu.coreID(), 7);
}

TEST_F(LogicalCpu_UT, SetLogicalID_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setLogicalID(15);
    EXPECT_EQ(cpu.logicalID(), 15);
}

TEST_F(LogicalCpu_UT, SetIDs_OverwritePreviousValues)
{
    LogicalCpu cpu;
    cpu.setPhysicalID(1);
    cpu.setCoreID(2);
    cpu.setLogicalID(3);

    cpu.setPhysicalID(10);
    cpu.setCoreID(20);
    cpu.setLogicalID(30);

    EXPECT_EQ(cpu.physicalID(), 10);
    EXPECT_EQ(cpu.coreID(), 20);
    EXPECT_EQ(cpu.logicalID(), 30);
}

// ===== Cache Setters / Getters =====

TEST_F(LogicalCpu_UT, SetL1dCache_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setL1dCache("32K");
    EXPECT_EQ(cpu.l1dCache(), "32K");
}

TEST_F(LogicalCpu_UT, SetL1iCache_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setL1iCache("32K");
    EXPECT_EQ(cpu.l1iCache(), "32K");
}

TEST_F(LogicalCpu_UT, SetL2Cache_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setL2Cache("256K");
    EXPECT_EQ(cpu.l2Cache(), "256K");
}

TEST_F(LogicalCpu_UT, SetL3Cache_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setL3Cache("12288K");
    EXPECT_EQ(cpu.l3Cache(), "12288K");
}

TEST_F(LogicalCpu_UT, SetL4Cache_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setL4Cache("0K");
    EXPECT_EQ(cpu.l4Cache(), "0K");
}

// ===== Shared CPU List Setters / Getters =====

TEST_F(LogicalCpu_UT, SetL1dSharedCpuList_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setL1dSharedCpuList("0,1");
    EXPECT_EQ(cpu.l1dSharedCpuList(), "0,1");
}

TEST_F(LogicalCpu_UT, SetL1iSharedCpuList_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setL1iSharedCpuList("0,1");
    EXPECT_EQ(cpu.l1iSharedCpuList(), "0,1");
}

TEST_F(LogicalCpu_UT, SetL2SharedCpuList_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setL2SharedCpuList("0,1,2,3");
    EXPECT_EQ(cpu.l2SharedCpuList(), "0,1,2,3");
}

TEST_F(LogicalCpu_UT, SetL3SharedCpuList_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setL3SharedCpuList("0-15");
    EXPECT_EQ(cpu.l3SharedCpuList(), "0-15");
}

TEST_F(LogicalCpu_UT, SetL4SharedCpuList_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setL4SharedCpuList("0-7");
    EXPECT_EQ(cpu.l4SharedCpuList(), "0-7");
}

// ===== Frequency Setters / Getters =====

TEST_F(LogicalCpu_UT, SetMinFreq_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setMinFreq("800MHz");
    EXPECT_EQ(cpu.minFreq(), "800MHz");
}

TEST_F(LogicalCpu_UT, SetCurFreq_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setCurFreq("2900MHz");
    EXPECT_EQ(cpu.curFreq(), "2900MHz");
}

TEST_F(LogicalCpu_UT, SetMaxFreq_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setMaxFreq("4900MHz");
    EXPECT_EQ(cpu.maxFreq(), "4900MHz");
}

// ===== CPU Model Info Setters / Getters =====

TEST_F(LogicalCpu_UT, SetModel_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setModel("165");
    EXPECT_EQ(cpu.model(), "165");
}

TEST_F(LogicalCpu_UT, SetModelName_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setModelName("Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz");
    EXPECT_EQ(cpu.modelName(), "Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz");
}

TEST_F(LogicalCpu_UT, SetStepping_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setStepping("5");
    EXPECT_EQ(cpu.stepping(), "5");
}

TEST_F(LogicalCpu_UT, SetVendor_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setVendor("GenuineIntel");
    EXPECT_EQ(cpu.vendor(), "GenuineIntel");
}

TEST_F(LogicalCpu_UT, SetCpuFamily_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setcpuFamily("6");
    EXPECT_EQ(cpu.cpuFamliy(), "6");
}

TEST_F(LogicalCpu_UT, SetFlags_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    QString flags = "fpu vme de pse tsc msr pae mce cx8 apic sep mtrr";
    cpu.setFlags(flags);
    EXPECT_EQ(cpu.flags(), flags);
}

TEST_F(LogicalCpu_UT, SetBogomips_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setBogomips("5799.99");
    EXPECT_EQ(cpu.bogomips(), "5799.99");
}

TEST_F(LogicalCpu_UT, SetArch_ReturnsCorrectValue)
{
    LogicalCpu cpu;
    cpu.setArch("x86_64");
    EXPECT_EQ(cpu.arch(), "x86_64");
}

// ===== Overwrite Behavior =====

TEST_F(LogicalCpu_UT, Setters_OverwritePreviousValues)
{
    LogicalCpu cpu;
    cpu.setModel("old");
    cpu.setModel("new");
    EXPECT_EQ(cpu.model(), "new");

    cpu.setVendor("vendorA");
    cpu.setVendor("vendorB");
    EXPECT_EQ(cpu.vendor(), "vendorB");
}

// ===== Full Field Population =====

TEST_F(LogicalCpu_UT, SetAllFields_AllGettersReturnCorrectValues)
{
    LogicalCpu cpu;
    cpu.setLogicalID(0);
    cpu.setCoreID(0);
    cpu.setPhysicalID(0);
    cpu.setL1dCache("32K");
    cpu.setL1iCache("32K");
    cpu.setL2Cache("256K");
    cpu.setL3Cache("12288K");
    cpu.setL4Cache("");
    cpu.setL1dSharedCpuList("0");
    cpu.setL1iSharedCpuList("0");
    cpu.setL2SharedCpuList("0,1");
    cpu.setL3SharedCpuList("0-15");
    cpu.setL4SharedCpuList("");
    cpu.setMinFreq("800MHz");
    cpu.setCurFreq("2900MHz");
    cpu.setMaxFreq("4900MHz");
    cpu.setModel("165");
    cpu.setModelName("Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz");
    cpu.setStepping("5");
    cpu.setVendor("GenuineIntel");
    cpu.setcpuFamily("6");
    cpu.setFlags("fpu vme de pse");
    cpu.setBogomips("5799.99");
    cpu.setArch("x86_64");

    EXPECT_EQ(cpu.logicalID(), 0);
    EXPECT_EQ(cpu.coreID(), 0);
    EXPECT_EQ(cpu.physicalID(), 0);
    EXPECT_EQ(cpu.l1dCache(), "32K");
    EXPECT_EQ(cpu.l1iCache(), "32K");
    EXPECT_EQ(cpu.l2Cache(), "256K");
    EXPECT_EQ(cpu.l3Cache(), "12288K");
    EXPECT_EQ(cpu.l4Cache(), "");
    EXPECT_EQ(cpu.l1dSharedCpuList(), "0");
    EXPECT_EQ(cpu.l1iSharedCpuList(), "0");
    EXPECT_EQ(cpu.l2SharedCpuList(), "0,1");
    EXPECT_EQ(cpu.l3SharedCpuList(), "0-15");
    EXPECT_EQ(cpu.l4SharedCpuList(), "");
    EXPECT_EQ(cpu.minFreq(), "800MHz");
    EXPECT_EQ(cpu.curFreq(), "2900MHz");
    EXPECT_EQ(cpu.maxFreq(), "4900MHz");
    EXPECT_EQ(cpu.model(), "165");
    EXPECT_EQ(cpu.modelName(), "Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz");
    EXPECT_EQ(cpu.stepping(), "5");
    EXPECT_EQ(cpu.vendor(), "GenuineIntel");
    EXPECT_EQ(cpu.cpuFamliy(), "6");
    EXPECT_EQ(cpu.flags(), "fpu vme de pse");
    EXPECT_EQ(cpu.bogomips(), "5799.99");
    EXPECT_EQ(cpu.arch(), "x86_64");
}
