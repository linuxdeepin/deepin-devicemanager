// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "../stub.h"
#include "cpu/physicalcpu.h"
#include "cpu/corecpu.h"
#include "cpu/logicalcpu.h"
#include "DDLog.h"

#include <QString>
#include <QList>

/*
 * Branch list for PhysicalCpu:
 * addCoreCpu:
 *   B1: m_MapCoreCpu.find(id) == end -> insert           | AddCoreCpu_NewId_InsertsCore, AddCoreCpu_DuplicateId_NoOverwrite
 * coreIsExisted:
 *   B2: find(id) != end -> true/false                    | CoreIsExisted_UnknownId_ReturnsFalse, CoreIsExisted_ExistingId_ReturnsTrue
 * logicalIsExisted:
 *   B3: i < 0 -> continue (skip sentinel)                | LogicalIsExisted_SentinelCore_SkipsCheck
 *   B4: core.logicalIsExisted(id) -> true/false          | LogicalIsExisted_ExistingId_ReturnsTrue, LogicalIsExisted_UnknownId_ReturnsFalse
 * logicalCpu:
 *   B5: i < 0 -> continue                                | LogicalCpu_NotFound_FallsBackToSentinel
 *   B6: core.logicalIsExisted(id) -> return found        | LogicalCpu_ExistingId_ReturnsReference
 *   B7: fallback -> m_MapCoreCpu[-1].logicalCpu(-1)      | LogicalCpu_NotFound_FallsBackToSentinel
 * getInfo:
 *   B8: coreId() >= 0 -> getInfo (skip sentinel)         | GetInfo_SentinelCoreOnly_ProducesEmptyString, GetInfo_PopulatedCores_ProducesExpectedOutput
 * coreNum:
 *   B9: find(-1) == end -> size                          | CoreNum_NoSentinel_ReturnsSize
 *   B10: find(-1) != end -> size - 1                     | CoreNum_WithSentinel_ExcludesFromCount
 * logicalNum:
 *   B11: id < 0 -> continue                              | LogicalNum_SentinelCore_SkipsInSum
 */

class PhysicalCpu_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

// helper: build a CoreCpu with one logical cpu
static CoreCpu makeCore(int coreId, int logicalId)
{
    CoreCpu core(coreId);
    LogicalCpu lc;
    lc.setLogicalID(logicalId);
    lc.setCoreID(coreId);
    core.addLogicalCpu(logicalId, lc);
    return core;
}

// ===== Constructor =====

TEST_F(PhysicalCpu_UT, DefaultConstructor_NoArgs_ZeroCounts)
{
    // Arrange
    PhysicalCpu cpu;
    // Act
    int coreCount = cpu.coreNum();
    int logicalCount = cpu.logicalNum();
    // Assert
    EXPECT_EQ(coreCount, 0);
    EXPECT_EQ(logicalCount, 0);
    EXPECT_TRUE(cpu.coreNums().isEmpty());
}

TEST_F(PhysicalCpu_UT, IdConstructor_WithId_ZeroCounts)
{
    // Arrange
    PhysicalCpu cpu(2);
    // Act
    int coreCount = cpu.coreNum();
    // Assert
    EXPECT_EQ(coreCount, 0);
    EXPECT_EQ(cpu.coreNums().size(), 0);
}

// ===== addCoreCpu =====

TEST_F(PhysicalCpu_UT, AddCoreCpu_NewId_InsertsCore)
{
    // Arrange
    PhysicalCpu cpu(0);
    CoreCpu core(0);
    // Act
    cpu.addCoreCpu(0, core);
    // Assert
    EXPECT_TRUE(cpu.coreIsExisted(0));
    EXPECT_EQ(cpu.coreNum(), 1);
    EXPECT_EQ(cpu.coreNums().size(), 1);
}

TEST_F(PhysicalCpu_UT, AddCoreCpu_DuplicateId_NoOverwrite)
{
    // Arrange
    PhysicalCpu cpu(0);
    CoreCpu first(0);
    first.addLogicalCpu(0, LogicalCpu());
    cpu.addCoreCpu(0, first);
    CoreCpu second(0);
    second.addLogicalCpu(1, LogicalCpu());
    // Act
    cpu.addCoreCpu(0, second);
    // Assert
    EXPECT_EQ(cpu.coreNum(), 1);
    EXPECT_EQ(cpu.coreCpu(0).logicalNum(), 1);
}

TEST_F(PhysicalCpu_UT, AddCoreCpu_MultipleIds_InsertsAll)
{
    // Arrange
    PhysicalCpu cpu(0);
    // Act
    for (int i = 0; i < 3; ++i) {
        cpu.addCoreCpu(i, CoreCpu(i));
    }
    // Assert
    EXPECT_EQ(cpu.coreNum(), 3);
    EXPECT_EQ(cpu.coreNums().size(), 3);
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(cpu.coreIsExisted(i));
    }
}

// ===== coreIsExisted =====

TEST_F(PhysicalCpu_UT, CoreIsExisted_UnknownId_ReturnsFalse)
{
    // Arrange
    PhysicalCpu cpu(0);
    // Act
    bool result = cpu.coreIsExisted(5);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(cpu.coreNum(), 0);
}

TEST_F(PhysicalCpu_UT, CoreIsExisted_ExistingId_ReturnsTrue)
{
    // Arrange
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(1, CoreCpu(1));
    // Act
    bool result = cpu.coreIsExisted(1);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(cpu.coreNum(), 1);
}

// ===== coreCpu =====

TEST_F(PhysicalCpu_UT, CoreCpu_ExistingId_ReturnsMutableRef)
{
    // Arrange
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(0, CoreCpu(0));
    // Act
    cpu.coreCpu(0).setCoreId(77);
    // Assert
    EXPECT_EQ(cpu.coreCpu(0).coreId(), 77);
    EXPECT_EQ(cpu.coreNum(), 1);
}

// ===== coreNum (sentinel exclusion) =====

TEST_F(PhysicalCpu_UT, CoreNum_NoSentinel_ReturnsSize)
{
    // Arrange
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(0, CoreCpu(0));
    cpu.addCoreCpu(1, CoreCpu(1));
    // Act
    int num = cpu.coreNum();
    // Assert
    EXPECT_EQ(num, 2);
    EXPECT_EQ(cpu.coreNums().size(), 2);
}

TEST_F(PhysicalCpu_UT, CoreNum_WithSentinel_ExcludesFromCount)
{
    // Arrange
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(0, CoreCpu(0));
    cpu.addCoreCpu(-1, CoreCpu(-1));
    // Act
    int num = cpu.coreNum();
    // Assert
    EXPECT_EQ(num, 1);
    EXPECT_EQ(cpu.coreNums().size(), 2);
}

// ===== logicalIsExisted =====

TEST_F(PhysicalCpu_UT, LogicalIsExisted_NoCores_ReturnsFalse)
{
    // Arrange
    PhysicalCpu cpu(0);
    // Act
    bool result = cpu.logicalIsExisted(0);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(cpu.coreNum(), 0);
}

TEST_F(PhysicalCpu_UT, LogicalIsExisted_UnknownId_ReturnsFalse)
{
    // Arrange
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(0, makeCore(0, 0));
    // Act
    bool result = cpu.logicalIsExisted(99);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(cpu.coreNum(), 1);
}

TEST_F(PhysicalCpu_UT, LogicalIsExisted_ExistingId_ReturnsTrue)
{
    // Arrange
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(0, makeCore(0, 0));
    cpu.addCoreCpu(1, makeCore(1, 2));
    // Act
    bool r0 = cpu.logicalIsExisted(0);
    bool r2 = cpu.logicalIsExisted(2);
    // Assert
    EXPECT_TRUE(r0);
    EXPECT_TRUE(r2);
    EXPECT_EQ(cpu.coreNum(), 2);
}

TEST_F(PhysicalCpu_UT, LogicalIsExisted_SentinelCore_SkipsCheck)
{
    // Arrange — add a logical cpu under a sentinel (-1) core; branch B1 skips it
    PhysicalCpu cpu(0);
    CoreCpu sentinel(-1);
    LogicalCpu lc;
    sentinel.addLogicalCpu(5, lc);
    cpu.addCoreCpu(-1, sentinel);
    // Act
    bool result = cpu.logicalIsExisted(5);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(cpu.coreNum(), 0);
}

// ===== logicalCpu =====

TEST_F(PhysicalCpu_UT, LogicalCpu_ExistingId_ReturnsReference)
{
    // Arrange
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(0, makeCore(0, 0));
    // Act
    cpu.logicalCpu(0).setModel("found");
    // Assert
    EXPECT_EQ(cpu.logicalCpu(0).model(), QString("found"));
    EXPECT_EQ(cpu.logicalNum(), 1);
}

TEST_F(PhysicalCpu_UT, LogicalCpu_NotFound_FallsBackToSentinel)
{
    // Arrange — no positive cores; only a -1 sentinel core exists; branch B3 fires
    PhysicalCpu cpu(0);
    CoreCpu sentinel(-1);
    cpu.addCoreCpu(-1, sentinel);
    // Act
    LogicalCpu &lc = cpu.logicalCpu(42);
    // Assert
    EXPECT_EQ(lc.logicalID(), -1);
    EXPECT_EQ(cpu.coreNum(), 0);
}

// ===== logicalNum =====

TEST_F(PhysicalCpu_UT, LogicalNum_MultipleCores_SumsCounts)
{
    // Arrange
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(0, makeCore(0, 0));
    cpu.addCoreCpu(1, makeCore(1, 1));
    cpu.addCoreCpu(2, makeCore(2, 2));
    // Act
    int num = cpu.logicalNum();
    // Assert
    EXPECT_EQ(num, 3);
    EXPECT_EQ(cpu.coreNum(), 3);
}

TEST_F(PhysicalCpu_UT, LogicalNum_SentinelCore_SkipsInSum)
{
    // Arrange — sentinel core skipped in sum; branch B1 fires
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(0, makeCore(0, 0));
    CoreCpu sentinel(-1);
    sentinel.addLogicalCpu(-1, LogicalCpu());
    cpu.addCoreCpu(-1, sentinel);
    // Act
    int num = cpu.logicalNum();
    // Assert
    EXPECT_EQ(num, 1);
    EXPECT_EQ(cpu.coreNum(), 1);
}

// ===== getInfo =====

TEST_F(PhysicalCpu_UT, GetInfo_NoCores_ProducesEmptyString)
{
    // Arrange
    PhysicalCpu cpu(0);
    QString info;
    // Act
    cpu.getInfo(info);
    // Assert
    EXPECT_TRUE(info.isEmpty());
    EXPECT_EQ(info.length(), 0);
}

TEST_F(PhysicalCpu_UT, GetInfo_SentinelCoreOnly_ProducesEmptyString)
{
    // Arrange — sentinel core has coreId=-1, so branch B1 skips it
    PhysicalCpu cpu(0);
    CoreCpu sentinel(-1);
    LogicalCpu lc;
    lc.setLogicalID(0);
    lc.setVendor("Sentinel");
    sentinel.addLogicalCpu(0, lc);
    cpu.addCoreCpu(-1, sentinel);
    QString info;
    // Act
    cpu.getInfo(info);
    // Assert
    EXPECT_TRUE(info.isEmpty());
    EXPECT_EQ(info.indexOf("Sentinel"), -1);
}

TEST_F(PhysicalCpu_UT, GetInfo_PopulatedCores_ProducesExpectedOutput)
{
    // Arrange
    PhysicalCpu cpu(0);
    CoreCpu core0(0);
    LogicalCpu lc0;
    lc0.setLogicalID(0);
    lc0.setCoreID(0);
    lc0.setVendor("GenuineIntel");
    core0.addLogicalCpu(0, lc0);
    cpu.addCoreCpu(0, core0);

    CoreCpu core1(1);
    LogicalCpu lc1;
    lc1.setLogicalID(1);
    lc1.setArch("x86_64");
    core1.addLogicalCpu(1, lc1);
    cpu.addCoreCpu(1, core1);
    QString info;
    // Act
    cpu.getInfo(info);
    // Assert
    EXPECT_NE(info.indexOf("processor : 0"), -1);
    EXPECT_NE(info.indexOf("vendor_id : GenuineIntel"), -1);
    EXPECT_NE(info.indexOf("processor : 1"), -1);
    EXPECT_NE(info.indexOf("Architecture : x86_64"), -1);
}

// ===== coreNums =====

TEST_F(PhysicalCpu_UT, CoreNums_MultipleKeys_ReturnsAllKeys)
{
    // Arrange
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(0, CoreCpu(0));
    cpu.addCoreCpu(2, CoreCpu(2));
    cpu.addCoreCpu(-1, CoreCpu(-1));
    // Act
    QList<int> nums = cpu.coreNums();
    // Assert
    EXPECT_EQ(nums.size(), 3);
    EXPECT_TRUE(nums.contains(0));
    EXPECT_TRUE(nums.contains(2));
    EXPECT_TRUE(nums.contains(-1));
}

// ===== diagPrintInfo =====

TEST_F(PhysicalCpu_UT, DiagPrintInfo_WithCore_DoesNotCrash)
{
    // Arrange
    PhysicalCpu cpu(0);
    cpu.addCoreCpu(0, makeCore(0, 0));
    // Act
    cpu.diagPrintInfo();
    // Assert
    EXPECT_EQ(cpu.coreNum(), 1);
    EXPECT_EQ(cpu.logicalNum(), 1);
}
