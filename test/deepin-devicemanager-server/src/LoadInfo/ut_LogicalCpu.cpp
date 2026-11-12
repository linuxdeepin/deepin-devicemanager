// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "cpu/logicalcpu.h"

#include <QString>

/*
 * Branch list for LogicalCpu:
 * 逻辑核为纯数据类：ctor 默认值 + setter/getter 成对读写 + 覆写语义
 *   ctor 默认 ID=-1、字符串字段为空   → DefaultConstructor_Ids_AllNegativeOne,
 *                                        DefaultConstructor_StringFields_AllEmpty
 *   setter/getter 往返一致            → Set*_Value_RoundTrip 系列
 *   重复 setter 覆写                  → SetIDs_Overwrite_NewValues,
 *                                        Setters_Overwrite_ModelAndVendor
 *   全字段灌入后逐项校验              → SetAllFields_AllGetters_RoundTrip
 */

class LogicalCpu_UT : public ::testing::Test
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

// ===== Constructor / Default Values =====

TEST_F(LogicalCpu_UT, DefaultConstructor_Ids_AllNegativeOne)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    const int pid = cpu.physicalID();
    const int cid = cpu.coreID();
    const int lid = cpu.logicalID();
    // Assert
    EXPECT_EQ(pid, -1);
    EXPECT_EQ(cid, -1);
    EXPECT_EQ(lid, -1);
}

TEST_F(LogicalCpu_UT, DefaultConstructor_StringFields_AllEmpty)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    // Assert — 字符串字段默认为空 QString
    EXPECT_EQ(cpu.l1dCache(), QString());
    EXPECT_EQ(cpu.l1iCache(), QString());
    EXPECT_EQ(cpu.l2Cache(), QString());
    EXPECT_EQ(cpu.l3Cache(), QString());
    EXPECT_EQ(cpu.l4Cache(), QString());
    EXPECT_EQ(cpu.l1dSharedCpuList(), QString());
    EXPECT_EQ(cpu.l1iSharedCpuList(), QString());
    EXPECT_EQ(cpu.l2SharedCpuList(), QString());
    EXPECT_EQ(cpu.l3SharedCpuList(), QString());
    EXPECT_EQ(cpu.l4SharedCpuList(), QString());
    EXPECT_EQ(cpu.maxFreq(), QString());
    EXPECT_EQ(cpu.minFreq(), QString());
    EXPECT_EQ(cpu.curFreq(), QString());
    EXPECT_EQ(cpu.model(), QString());
    EXPECT_EQ(cpu.modelName(), QString());
    EXPECT_EQ(cpu.stepping(), QString());
    EXPECT_EQ(cpu.vendor(), QString());
    EXPECT_EQ(cpu.cpuFamliy(), QString());
    EXPECT_EQ(cpu.flags(), QString());
    EXPECT_EQ(cpu.bogomips(), QString());
    EXPECT_EQ(cpu.arch(), QString());
}

// ===== ID Setters / Getters =====

TEST_F(LogicalCpu_UT, SetPhysicalID_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setPhysicalID(3);
    // Assert
    EXPECT_EQ(cpu.physicalID(), 3);
    EXPECT_NE(cpu.physicalID(), -1);
}

TEST_F(LogicalCpu_UT, SetCoreID_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setCoreID(7);
    // Assert
    EXPECT_EQ(cpu.coreID(), 7);
    EXPECT_NE(cpu.coreID(), -1);
}

TEST_F(LogicalCpu_UT, SetLogicalID_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setLogicalID(15);
    // Assert
    EXPECT_EQ(cpu.logicalID(), 15);
    EXPECT_NE(cpu.logicalID(), -1);
}

TEST_F(LogicalCpu_UT, SetIDs_Overwrite_NewValues)
{
    // Arrange
    LogicalCpu cpu;
    cpu.setPhysicalID(1);
    cpu.setCoreID(2);
    cpu.setLogicalID(3);
    // Act
    cpu.setPhysicalID(10);
    cpu.setCoreID(20);
    cpu.setLogicalID(30);
    // Assert — 后写覆盖先写
    EXPECT_EQ(cpu.physicalID(), 10);
    EXPECT_EQ(cpu.coreID(), 20);
    EXPECT_EQ(cpu.logicalID(), 30);
}

// ===== Cache Setters / Getters =====

TEST_F(LogicalCpu_UT, SetL1dCache_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setL1dCache("32K");
    // Assert
    EXPECT_EQ(cpu.l1dCache(), QString("32K"));
    EXPECT_NE(cpu.l1dCache(), QString());
}

TEST_F(LogicalCpu_UT, SetL1iCache_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setL1iCache("32K");
    // Assert
    EXPECT_EQ(cpu.l1iCache(), QString("32K"));
    EXPECT_NE(cpu.l1iCache(), QString());
}

TEST_F(LogicalCpu_UT, SetL2Cache_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setL2Cache("256K");
    // Assert
    EXPECT_EQ(cpu.l2Cache(), QString("256K"));
    EXPECT_NE(cpu.l2Cache(), QString());
}

TEST_F(LogicalCpu_UT, SetL3Cache_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setL3Cache("12288K");
    // Assert
    EXPECT_EQ(cpu.l3Cache(), QString("12288K"));
    EXPECT_NE(cpu.l3Cache(), QString());
}

TEST_F(LogicalCpu_UT, SetL4Cache_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setL4Cache("0K");
    // Assert
    EXPECT_EQ(cpu.l4Cache(), QString("0K"));
    EXPECT_NE(cpu.l4Cache(), QString());
}

// ===== Shared CPU List Setters / Getters =====

TEST_F(LogicalCpu_UT, SetL1dSharedCpuList_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setL1dSharedCpuList("0,1");
    // Assert
    EXPECT_EQ(cpu.l1dSharedCpuList(), QString("0,1"));
    EXPECT_NE(cpu.l1dSharedCpuList(), QString());
}

TEST_F(LogicalCpu_UT, SetL1iSharedCpuList_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setL1iSharedCpuList("0,1");
    // Assert
    EXPECT_EQ(cpu.l1iSharedCpuList(), QString("0,1"));
    EXPECT_NE(cpu.l1iSharedCpuList(), QString());
}

TEST_F(LogicalCpu_UT, SetL2SharedCpuList_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setL2SharedCpuList("0,1,2,3");
    // Assert
    EXPECT_EQ(cpu.l2SharedCpuList(), QString("0,1,2,3"));
    EXPECT_NE(cpu.l2SharedCpuList(), QString());
}

TEST_F(LogicalCpu_UT, SetL3SharedCpuList_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setL3SharedCpuList("0-15");
    // Assert
    EXPECT_EQ(cpu.l3SharedCpuList(), QString("0-15"));
    EXPECT_NE(cpu.l3SharedCpuList(), QString());
}

TEST_F(LogicalCpu_UT, SetL4SharedCpuList_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setL4SharedCpuList("0-7");
    // Assert
    EXPECT_EQ(cpu.l4SharedCpuList(), QString("0-7"));
    EXPECT_NE(cpu.l4SharedCpuList(), QString());
}

// ===== Frequency Setters / Getters =====

TEST_F(LogicalCpu_UT, SetMinFreq_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setMinFreq("800MHz");
    // Assert
    EXPECT_EQ(cpu.minFreq(), QString("800MHz"));
    EXPECT_NE(cpu.minFreq(), QString());
}

TEST_F(LogicalCpu_UT, SetCurFreq_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setCurFreq("2900MHz");
    // Assert
    EXPECT_EQ(cpu.curFreq(), QString("2900MHz"));
    EXPECT_NE(cpu.curFreq(), QString());
}

TEST_F(LogicalCpu_UT, SetMaxFreq_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setMaxFreq("4900MHz");
    // Assert
    EXPECT_EQ(cpu.maxFreq(), QString("4900MHz"));
    EXPECT_NE(cpu.maxFreq(), QString());
}

// ===== CPU Model Info Setters / Getters =====

TEST_F(LogicalCpu_UT, SetModel_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setModel("165");
    // Assert
    EXPECT_EQ(cpu.model(), QString("165"));
    EXPECT_NE(cpu.model(), QString());
}

TEST_F(LogicalCpu_UT, SetModelName_Value_RoundTrip)
{
    // Arrange
    const QString name = "Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz";
    LogicalCpu cpu;
    // Act
    cpu.setModelName(name);
    // Assert
    EXPECT_EQ(cpu.modelName(), name);
    EXPECT_EQ(cpu.modelName().contains("i7-10700"), true);
}

TEST_F(LogicalCpu_UT, SetStepping_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setStepping("5");
    // Assert
    EXPECT_EQ(cpu.stepping(), QString("5"));
    EXPECT_NE(cpu.stepping(), QString());
}

TEST_F(LogicalCpu_UT, SetVendor_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setVendor("GenuineIntel");
    // Assert
    EXPECT_EQ(cpu.vendor(), QString("GenuineIntel"));
    EXPECT_NE(cpu.vendor(), QString());
}

TEST_F(LogicalCpu_UT, SetCpuFamily_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setcpuFamily("6");
    // Assert — getter 拼写为 cpuFamliy（与源码一致）
    EXPECT_EQ(cpu.cpuFamliy(), QString("6"));
    EXPECT_NE(cpu.cpuFamliy(), QString());
}

TEST_F(LogicalCpu_UT, SetFlags_Value_RoundTrip)
{
    // Arrange
    const QString flags = "fpu vme de pse tsc msr pae mce cx8 apic sep mtrr";
    LogicalCpu cpu;
    // Act
    cpu.setFlags(flags);
    // Assert
    EXPECT_EQ(cpu.flags(), flags);
    EXPECT_TRUE(cpu.flags().startsWith("fpu"));
}

TEST_F(LogicalCpu_UT, SetBogomips_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setBogomips("5799.99");
    // Assert
    EXPECT_EQ(cpu.bogomips(), QString("5799.99"));
    EXPECT_NE(cpu.bogomips(), QString());
}

TEST_F(LogicalCpu_UT, SetArch_Value_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act
    cpu.setArch("x86_64");
    // Assert
    EXPECT_EQ(cpu.arch(), QString("x86_64"));
    EXPECT_NE(cpu.arch(), QString());
}

// ===== Overwrite Behavior =====

TEST_F(LogicalCpu_UT, Setters_Overwrite_ModelAndVendor)
{
    // Arrange
    LogicalCpu cpu;
    cpu.setModel("old");
    cpu.setVendor("vendorA");
    // Act
    cpu.setModel("new");
    cpu.setVendor("vendorB");
    // Assert
    EXPECT_EQ(cpu.model(), QString("new"));
    EXPECT_EQ(cpu.vendor(), QString("vendorB"));
}

// ===== Full Field Population =====

TEST_F(LogicalCpu_UT, SetAllFields_AllGetters_RoundTrip)
{
    // Arrange
    LogicalCpu cpu;
    // Act — 全字段灌入
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
    // Assert — 逐项往返校验
    EXPECT_EQ(cpu.logicalID(), 0);
    EXPECT_EQ(cpu.coreID(), 0);
    EXPECT_EQ(cpu.physicalID(), 0);
    EXPECT_EQ(cpu.l1dCache(), QString("32K"));
    EXPECT_EQ(cpu.l1iCache(), QString("32K"));
    EXPECT_EQ(cpu.l2Cache(), QString("256K"));
    EXPECT_EQ(cpu.l3Cache(), QString("12288K"));
    EXPECT_EQ(cpu.l4Cache(), QString(""));
    EXPECT_EQ(cpu.l1dSharedCpuList(), QString("0"));
    EXPECT_EQ(cpu.l1iSharedCpuList(), QString("0"));
    EXPECT_EQ(cpu.l2SharedCpuList(), QString("0,1"));
    EXPECT_EQ(cpu.l3SharedCpuList(), QString("0-15"));
    EXPECT_EQ(cpu.l4SharedCpuList(), QString(""));
    EXPECT_EQ(cpu.minFreq(), QString("800MHz"));
    EXPECT_EQ(cpu.curFreq(), QString("2900MHz"));
    EXPECT_EQ(cpu.maxFreq(), QString("4900MHz"));
    EXPECT_EQ(cpu.model(), QString("165"));
    EXPECT_EQ(cpu.modelName(), QString("Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz"));
    EXPECT_EQ(cpu.stepping(), QString("5"));
    EXPECT_EQ(cpu.vendor(), QString("GenuineIntel"));
    EXPECT_EQ(cpu.cpuFamliy(), QString("6"));
    EXPECT_EQ(cpu.flags(), QString("fpu vme de pse"));
    EXPECT_EQ(cpu.bogomips(), QString("5799.99"));
    EXPECT_EQ(cpu.arch(), QString("x86_64"));
}
