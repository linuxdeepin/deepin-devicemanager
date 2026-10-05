// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "cpu/physicalcpu.h"
#include "cpu/corecpu.h"
#include "cpu/logicalcpu.h"

class PhysicalCpu_UT : public UT_HEAD
{
public:
    void SetUp() {}
    void TearDown() {}
};

// When the requested logical cpu is not found, logicalCpu() must fall back
// to a default-constructed LogicalCpu (logicalID == -1) without inserting a
// sentinel entry into m_MapCoreCpu.
TEST_F(PhysicalCpu_UT, LogicalCpu_NotFound_FallsBackToSentinel)
{
    PhysicalCpu physical(0);

    // No cores have been added, so logicalCpu(0) must take the fallback path.
    int coreCountBefore = physical.coreNums().size();
    LogicalCpu &logical = physical.logicalCpu(0);

    // Fallback returns a default-constructed LogicalCpu whose logicalID is -1.
    EXPECT_EQ(logical.logicalID(), -1);

    // The fallback must NOT have inserted a -1 key into the core map.
    EXPECT_EQ(physical.coreNums().size(), coreCountBefore);
    EXPECT_FALSE(physical.coreNums().contains(-1));

    // Repeated miss queries must not grow the container.
    physical.logicalCpu(1);
    physical.logicalCpu(2);
    EXPECT_EQ(physical.coreNums().size(), coreCountBefore);
    EXPECT_FALSE(physical.coreNums().contains(-1));
}
