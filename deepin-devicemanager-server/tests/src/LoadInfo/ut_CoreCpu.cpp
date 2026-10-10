// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "../stub.h"
#include "cpu/corecpu.h"
#include "cpu/logicalcpu.h"
#include "DDLog.h"

#include <QString>

/*
 * Branch list for CoreCpu:
 * addLogicalCpu:
 *   B1: m_MapLogicalCpu.find(id) == end -> insert        | AddLogicalCpu_NewId_InsertsLogical, AddLogicalCpu_DuplicateId_NoDoubleInsert
 * logicalIsExisted:
 *   B2: m_CoreId < 0 -> return false                      | LogicalIsExisted_NegativeCoreId_ReturnsFalse
 *   B3: find(id) != end -> return true/false              | LogicalIsExisted_ExistingId_ReturnsTrue, LogicalIsExisted_MissingId_ReturnsFalse
 * getInfo:
 *   B4: id < 0 -> continue (skip)                         | GetInfo_NegativeIdEntry_SkipsEntry
 * appendKeyValue(QString,QString):
 *   B5: value.isEmpty() -> return (skip)                  | AppendKeyValueString_EmptyValue_SkipsAppend
 * logicalNum:
 *   B6: find(-1) == end -> size                           | LogicalNum_NoNegativeOneEntry_ReturnsSize
 *   B7: find(-1) != end -> size - 1                       | LogicalNum_HasNegativeOneEntry_ReturnsSizeMinusOne
 */

class CoreCpu_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

// ===== Constructor / coreId =====

TEST_F(CoreCpu_UT, DefaultConstructor_NoArgs_CoreIdIsNegativeOne)
{
    // Arrange
    CoreCpu core;
    // Act
    int id = core.coreId();
    // Assert
    EXPECT_EQ(id, -1);
    EXPECT_EQ(core.logicalNum(), 0);
}

TEST_F(CoreCpu_UT, IdConstructor_WithId_CoreIdMatchesArg)
{
    // Arrange
    CoreCpu core(5);
    // Act
    int id = core.coreId();
    // Assert
    EXPECT_EQ(id, 5);
    EXPECT_EQ(core.logicalNum(), 0);
}

TEST_F(CoreCpu_UT, SetCoreId_ValidId_UpdatesCoreId)
{
    // Arrange
    CoreCpu core;
    // Act
    core.setCoreId(42);
    // Assert
    EXPECT_EQ(core.coreId(), 42);
    EXPECT_EQ(core.coreId(), 42);
}

TEST_F(CoreCpu_UT, SetCoreId_WithExistingLogical_CascadesCoreId)
{
    // Arrange
    CoreCpu core(0);
    LogicalCpu lc;
    lc.setLogicalID(1);
    core.addLogicalCpu(1, lc);
    // Act
    core.setCoreId(7);
    // Assert
    EXPECT_EQ(core.coreId(), 7);
    EXPECT_EQ(core.logicalCpu(1).coreID(), 7);
    EXPECT_EQ(core.logicalCpu(1).logicalID(), 1);
}

// ===== addLogicalCpu =====

TEST_F(CoreCpu_UT, AddLogicalCpu_NewId_InsertsLogical)
{
    // Arrange
    CoreCpu core(0);
    LogicalCpu lc;
    lc.setLogicalID(3);
    // Act
    core.addLogicalCpu(3, lc);
    // Assert
    EXPECT_TRUE(core.logicalIsExisted(3));
    EXPECT_EQ(core.logicalNum(), 1);
}

TEST_F(CoreCpu_UT, AddLogicalCpu_DuplicateId_NoDoubleInsert)
{
    // Arrange
    CoreCpu core(0);
    LogicalCpu lc1;
    lc1.setLogicalID(2);
    core.addLogicalCpu(2, lc1);
    LogicalCpu lc2;
    lc2.setLogicalID(99);
    // Act
    core.addLogicalCpu(2, lc2);
    // Assert
    EXPECT_EQ(core.logicalNum(), 1);
    EXPECT_EQ(core.logicalCpu(2).logicalID(), 2);
}

// ===== logicalIsExisted =====

TEST_F(CoreCpu_UT, LogicalIsExisted_NegativeCoreId_ReturnsFalse)
{
    // Arrange — default ctor sets m_CoreId = -1, so branch B1 fires
    CoreCpu core;
    LogicalCpu lc;
    core.addLogicalCpu(0, lc);
    // Act
    bool result = core.logicalIsExisted(0);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(core.coreId(), -1);
}

TEST_F(CoreCpu_UT, LogicalIsExisted_ExistingId_ReturnsTrue)
{
    // Arrange
    CoreCpu core(0);
    LogicalCpu lc;
    core.addLogicalCpu(1, lc);
    // Act
    bool result = core.logicalIsExisted(1);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(core.logicalNum(), 1);
}

TEST_F(CoreCpu_UT, LogicalIsExisted_MissingId_ReturnsFalse)
{
    // Arrange
    CoreCpu core(0);
    LogicalCpu lc;
    core.addLogicalCpu(1, lc);
    // Act
    bool result = core.logicalIsExisted(999);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(core.logicalNum(), 1);
}

// ===== logicalCpu =====

TEST_F(CoreCpu_UT, LogicalCpu_ExistingId_ReturnsInsertedCpu)
{
    // Arrange
    CoreCpu core(0);
    LogicalCpu lc;
    lc.setLogicalID(10);
    lc.setVendor("GenuineIntel");
    core.addLogicalCpu(10, lc);
    // Act
    LogicalCpu &retrieved = core.logicalCpu(10);
    // Assert
    EXPECT_EQ(retrieved.logicalID(), 10);
    EXPECT_EQ(retrieved.vendor(), "GenuineIntel");
}

// ===== getInfo =====

TEST_F(CoreCpu_UT, GetInfo_ValidLogicalCpu_AppendsKeys)
{
    // Arrange
    CoreCpu core(0);
    LogicalCpu lc;
    lc.setLogicalID(0);
    lc.setVendor("GenuineIntel");
    lc.setModelName("Intel Core i7");
    core.addLogicalCpu(0, lc);
    QString info;
    // Act
    core.getInfo(info);
    // Assert
    EXPECT_NE(info.indexOf("vendor_id : GenuineIntel"), -1);
    EXPECT_NE(info.indexOf("model name : Intel Core i7"), -1);
    EXPECT_NE(info.indexOf("processor : 0"), -1);
}

TEST_F(CoreCpu_UT, GetInfo_NegativeIdEntry_SkipsEntry)
{
    // Arrange — insert a logical CPU with id=-1 and a valid one with id=0
    CoreCpu core(0);
    LogicalCpu negLc;
    negLc.setLogicalID(-1);
    negLc.setVendor("ShouldNotAppear");
    core.addLogicalCpu(-1, negLc);

    LogicalCpu validLc;
    validLc.setLogicalID(0);
    validLc.setVendor("GenuineIntel");
    core.addLogicalCpu(0, validLc);
    QString info;
    // Act
    core.getInfo(info);
    // Assert
    EXPECT_EQ(info.indexOf("ShouldNotAppear"), -1);
    EXPECT_NE(info.indexOf("GenuineIntel"), -1);
}

TEST_F(CoreCpu_UT, GetInfo_EmptyMap_ProducesEmptyString)
{
    // Arrange
    CoreCpu core(0);
    QString info;
    // Act
    core.getInfo(info);
    // Assert
    EXPECT_TRUE(info.isEmpty());
    EXPECT_EQ(info.length(), 0);
}

// ===== appendKeyValue =====

TEST_F(CoreCpu_UT, AppendKeyValueString_NonEmptyValue_AppendsTrimmed)
{
    // Arrange
    CoreCpu core(0);
    QString info;
    // Act
    core.appendKeyValue(info, "key1", "  value1  ");
    // Assert
    EXPECT_EQ(info, QString("key1 : value1\n"));
    EXPECT_GT(info.length(), 0);
}

TEST_F(CoreCpu_UT, AppendKeyValueString_EmptyValue_SkipsAppend)
{
    // Arrange
    CoreCpu core(0);
    QString info;
    // Act
    core.appendKeyValue(info, "key1", "");
    // Assert
    EXPECT_TRUE(info.isEmpty());
    EXPECT_EQ(info.length(), 0);
}

TEST_F(CoreCpu_UT, AppendKeyValueInt_ValidValue_AlwaysAppended)
{
    // Arrange
    CoreCpu core(0);
    QString info;
    // Act
    core.appendKeyValue(info, "count", 42);
    core.appendKeyValue(info, "zero", 0);
    // Assert
    EXPECT_NE(info.indexOf("count : 42"), -1);
    EXPECT_NE(info.indexOf("zero : 0"), -1);
    EXPECT_GT(info.length(), 0);
}

// ===== logicalNum =====

TEST_F(CoreCpu_UT, LogicalNum_NoNegativeOneEntry_ReturnsSize)
{
    // Arrange
    CoreCpu core(0);
    LogicalCpu lc1, lc2;
    core.addLogicalCpu(0, lc1);
    core.addLogicalCpu(1, lc2);
    // Act
    int num = core.logicalNum();
    // Assert
    EXPECT_EQ(num, 2);
    EXPECT_FALSE(core.logicalIsExisted(999));
}

TEST_F(CoreCpu_UT, LogicalNum_HasNegativeOneEntry_ReturnsSizeMinusOne)
{
    // Arrange — insert id=-1 plus two valid ids; branch B2 fires
    CoreCpu core(0);
    LogicalCpu lcNeg, lc1, lc2;
    core.addLogicalCpu(-1, lcNeg);
    core.addLogicalCpu(0, lc1);
    core.addLogicalCpu(1, lc2);
    // Act
    int num = core.logicalNum();
    // Assert
    EXPECT_EQ(num, 2);
    EXPECT_EQ(core.coreId(), 0);
}

TEST_F(CoreCpu_UT, LogicalNum_EmptyMap_ReturnsZero)
{
    // Arrange
    CoreCpu core(0);
    // Act
    int num = core.logicalNum();
    // Assert
    EXPECT_EQ(num, 0);
    EXPECT_EQ(core.coreId(), 0);
}

// ===== diagPrintInfo =====

TEST_F(CoreCpu_UT, DiagPrintInfo_WithLogicalCpu_DoesNotCrash)
{
    // Arrange
    CoreCpu core(0);
    LogicalCpu lc;
    lc.setLogicalID(0);
    core.addLogicalCpu(0, lc);
    // Act
    core.diagPrintInfo();
    // Assert
    EXPECT_EQ(core.coreId(), 0);
    EXPECT_EQ(core.logicalNum(), 1);
}
