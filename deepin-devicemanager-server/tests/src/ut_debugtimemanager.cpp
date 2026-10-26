// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * 用例计数声明（method | level | factors | min | actual）
 * | DebugTimeManager::DebugTimeManager | mid | in_degree:3 | 2 | 2 |
 * | DebugTimeManager::beginPointLinux | mid | in_degree:2 | 2 | 4 |
 * | DebugTimeManager::beginPointQt | low | - | 1 | 2 |
 * | DebugTimeManager::clear | mid | in_degree:11,name_pattern:clear | 2 | 2 |
 * | DebugTimeManager::endPointLinux | mid | in_degree:2 | 2 | 2 |
 * | DebugTimeManager::endPointQt | low | - | 1 | 3 |
 */

#include <gtest/gtest.h>

#define private public // hack complier
#define protected public

#include "debugtimemanager.h"

#undef private
#undef protected
#include "stub.h"
#include "stubext.h"

#include <QDateTime>

using namespace stub_ext;

// ---------------------------------------------------------------------------
// 测试夹具：DebugTimeManager_Test
// ---------------------------------------------------------------------------
class DebugTimeManager_Test : public ::testing::Test
{
protected:
    void SetUp() override { }
    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(DebugTimeManager_Test, DebugTimeManager_UT_001)
{
    // Arrange
    DebugTimeManager *dtm = new DebugTimeManager();
    // Act
    dtm->beginPointLinux("001", "");
    // Assert
    EXPECT_FALSE(dtm->m_MapPoint.isEmpty());
    EXPECT_EQ(1, dtm->m_MapPoint.size());
    delete dtm;
}

TEST_F(DebugTimeManager_Test, DebugTimeManager_UT_002)
{
    // Arrange
    DebugTimeManager *dtm = new DebugTimeManager();
    // Act
    dtm->beginPointLinux("001", "");
    dtm->endPointLinux("001");
    // Assert
    EXPECT_GE(dtm->m_MapPoint["001"].time, static_cast<qint64>(0));
    EXPECT_EQ(1, dtm->m_MapPoint.size());
    delete dtm;
}

TEST_F(DebugTimeManager_Test, DebugTimeManager_UT_003)
{
    // Arrange
    DebugTimeManager *dtm = new DebugTimeManager();
    // Act
    dtm->clear();

    // Assert
    EXPECT_TRUE(dtm->m_MapPoint.isEmpty());
    EXPECT_EQ(size_t(0), dtm->m_MapPoint.size());
    delete dtm;
}

/* ---------------- UT-T30 追加用例：beginPointQt / endPointQt / clear 补充分支 ---------------- */

// beginPointQt：写入点位描述与毫秒时间戳
TEST_F(DebugTimeManager_Test, BeginPointQt_NewPoint_StoresStatusAndTimestamp)
{
    // Arrange
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<qint64 (*)()>(&QDateTime::currentMSecsSinceEpoch),
                   []() -> qint64 { return 1234567890; });

    DebugTimeManager *dtm = new DebugTimeManager();
    // Act
    dtm->beginPointQt("POINT-T30", "status-x");

    // Assert
    ASSERT_TRUE(dtm->m_MapPoint.contains("POINT-T30"));
    EXPECT_EQ("status-x", dtm->m_MapPoint["POINT-T30"].desc);
    EXPECT_EQ(1234567890, dtm->m_MapPoint["POINT-T30"].time);

    stub.clear();
    delete dtm;
}

// beginPointQt：同名点位重复打点时覆盖旧记录而非新增
TEST_F(DebugTimeManager_Test, BeginPointQt_OverwriteSamePoint_ReplacesEntry)
{
    // Arrange
    DebugTimeManager *dtm = new DebugTimeManager();
    // Act
    dtm->beginPointQt("POINT-T30", "first");
    dtm->beginPointQt("POINT-T30", "second");

    // Assert
    EXPECT_EQ(1, dtm->m_MapPoint.size());
    EXPECT_EQ("second", dtm->m_MapPoint["POINT-T30"].desc);

    delete dtm;
}

// endPointQt：以调用时刻毫秒差更新点位耗时（QDateTime 打桩控制时钟）
TEST_F(DebugTimeManager_Test, EndPointQt_AfterInterval_ComputesElapsedMs)
{
    // Arrange
    qint64 fakeTick = 1000;
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<qint64 (*)()>(&QDateTime::currentMSecsSinceEpoch),
                   [&fakeTick]() -> qint64 {
                       qint64 value = fakeTick;
                       fakeTick += 1234;   // 第二次调用即结束时刻
                       return value;
                   });

    DebugTimeManager *dtm = new DebugTimeManager();
    // Act
    dtm->beginPointQt("POINT-T30", "");
    dtm->endPointQt("POINT-T30");

    // Assert
    EXPECT_EQ(1234, dtm->m_MapPoint["POINT-T30"].time);
    EXPECT_EQ(1, dtm->m_MapPoint.size());   // 只更新既有点位

    stub.clear();
    delete dtm;
}

// endPointQt：起止同一时刻时耗时为 0（边界）
TEST_F(DebugTimeManager_Test, EndPointQt_ZeroElapsed_BoundaryCase)
{
    // Arrange
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<qint64 (*)()>(&QDateTime::currentMSecsSinceEpoch),
                   []() -> qint64 { return 5000; });

    DebugTimeManager *dtm = new DebugTimeManager();
    // Act
    dtm->beginPointQt("POINT-T30", "");
    dtm->endPointQt("POINT-T30");

    // Assert
    EXPECT_EQ(0, dtm->m_MapPoint["POINT-T30"].time);
    EXPECT_FALSE(dtm->m_MapPoint.isEmpty());   // 点位仍保留

    stub.clear();
    delete dtm;
}

// endPointQt：结束不存在的点位时无副作用（不新增记录）
TEST_F(DebugTimeManager_Test, EndPointQt_MissingPoint_NoOp)
{
    // Arrange
    DebugTimeManager *dtm = new DebugTimeManager();
    // Act
    dtm->beginPointQt("POINT-T30", "");

    dtm->endPointQt("POINT-NOT-EXIST");

    // Assert
    EXPECT_EQ(1, dtm->m_MapPoint.size());
    EXPECT_FALSE(dtm->m_MapPoint.contains("POINT-NOT-EXIST"));

    delete dtm;
}

// beginPointLinux：不同点位各自独立记录
TEST_F(DebugTimeManager_Test, BeginPointLinux_DistinctPoints_KeepsEntries)
{
    // Arrange
    DebugTimeManager *dtm = new DebugTimeManager();
    // Act
    dtm->beginPointLinux("POINT-P1", "s1");
    dtm->beginPointLinux("POINT-P2", "s2");

    // Assert
    EXPECT_EQ(2, dtm->m_MapPoint.size());
    EXPECT_EQ("s1", dtm->m_MapPoint["POINT-P1"].desc);
    EXPECT_EQ("s2", dtm->m_MapPoint["POINT-P2"].desc);

    delete dtm;
}

// clear：清空非空缓存后所有点位不可见
TEST_F(DebugTimeManager_Test, Clear_NonEmptyCache_RemovesAllPoints)
{
    // Arrange
    DebugTimeManager *dtm = new DebugTimeManager();
    // Act
    dtm->beginPointLinux("POINT-P1", "s1");
    dtm->beginPointQt("POINT-P2", "s2");
    // Assert
    ASSERT_FALSE(dtm->m_MapPoint.isEmpty());

    dtm->clear();

    EXPECT_TRUE(dtm->m_MapPoint.isEmpty());
    EXPECT_EQ(size_t(0), dtm->m_MapPoint.size());

    delete dtm;
}

TEST_F(DebugTimeManager_Test, DebugTimeManager_DefaultCtor_MapStartsEmpty)
{
    // Arrange
    // （无外部依赖，直接构造）
    // Act
    DebugTimeManager dtm;
    // Assert
    EXPECT_TRUE(dtm.m_MapPoint.isEmpty());   // 默认构造后无任何计时点
    EXPECT_EQ(size_t(0), dtm.m_MapPoint.size());
}
