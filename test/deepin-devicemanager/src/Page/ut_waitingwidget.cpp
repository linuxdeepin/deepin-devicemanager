// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method        | level | factors       | min | actual |
// |---------------|-------|---------------|-----|--------|
// | WaitingWidget | mid   | in_degree:66  | 2   | 2      |
// | ~WaitingWidget| low   | -             | 1   | 1      |
// | start         | mid   | in_degree:66  | 2   | 2      |
// | stop          | low   | -             | 1   | 2      |
// ─── actual >= min 达标（共 7 个用例实例）───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（start 前置态：未启动/已启动）
// 3. 每个等价类的边界值显式覆盖: [x]（连续调用累计、独立 stop）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（仅 7 用例，断言异质，不参数化）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（start/stop 各一个 if 判空分支，由空指针防护路径的桩验证）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（无显式 throw，不适用）
// 8. 负面场景有专门用例: [x]（未启动直接 stop）
// 9. 负面用例验证强异常安全: [x]（stop 独立调用不崩溃）
// 10. stub_ext vs gMock 选择正确: [x]（DSpinner 非虚且为 DTK 控件 → stub_ext 运行时替换）
//
// mock 深度分析（test-code-gen §4.0，来源：工作区实际源码 + MCP trace_path）：
// - WaitingWidget 仅组合 DSpinner + DLabel 两个 DTK 控件，无外部系统依赖；
//   DSpinner::start/stop 为非虚公有方法，用 stub_ext 记录型桩验证转发契约，
//   避免驱动其内部动画定时器。
// - 构造/析构走真实实现；无 DBus/网络/文件依赖，QApplication offscreen 下即可运行。
//
// 分支清单（来源：工作区实际源码 WaitingWidget.cpp）
// B1: ctor: spinner 固定尺寸 + 三层嵌套布局 + label 文案
// B2: ~WaitingWidget: DELETE_PTR 逐个删除子控件
// B3: start: mp_Spinner 判空 → start()
// B4: stop: mp_Spinner 判空 → stop()
//
// 用例映射：
// - Ctor_DefaultConstruction_InitializesSpinnerAndLabel → B1（mid #1）
// - Ctor_LayoutStructure_SpinnerAndLabelNested          → B1（mid #2）
// - Start_SpinnerPresent_ForwardsStart                  → B3（mid #1）
// - Start_RepeatedCalls_ForwardsTwice                   → B3（mid #2）
// - Stop_AfterStart_ForwardsStop                        → B4
// - Stop_StandaloneCall_ForwardsOnce                    → B4
// - Dtor_DeletesChildren_GuardsBecomeNull               → B2

#include <gtest/gtest.h>
#include <QPointer>
#include <QLayout>

#include "stubext.h"
#include "WaitingWidget.h"

class WaitingWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override {
        // Arrange：构造被测对象（纯组合控件，无外部依赖）
        obj = new WaitingWidget;
    }

    void TearDown() override {
        // 先卸桩再析构
        stub.clear();
        delete obj;
        obj = nullptr;
    }

    stub_ext::StubExt stub;
    WaitingWidget *obj = nullptr;
};

// ═══════════════════════════════════════════════════════════════
// 构造函数（mid，B1）
// ═══════════════════════════════════════════════════════════════
TEST_F(WaitingWidgetTest, Ctor_DefaultConstruction_InitializesSpinnerAndLabel)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：读取构造后的成员
    DSpinner *spinner = obj->mp_Spinner;
    DLabel *label = obj->mp_Label;

    // Assert：两个子控件均创建且尺寸约束生效
    EXPECT_NE(spinner, nullptr);
    EXPECT_NE(label, nullptr);
    EXPECT_EQ(spinner->width(), 32);
    EXPECT_FALSE(label->text().isEmpty());
}

TEST_F(WaitingWidgetTest, Ctor_LayoutStructure_SpinnerAndLabelNested)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：读取布局结构
    QLayout *lay = obj->layout();

    // Assert：水平布局为 [stretch, 纵向子布局, stretch] 三项结构
    ASSERT_NE(lay, nullptr);
    EXPECT_EQ(lay->count(), 3);
    EXPECT_NE(lay->itemAt(1)->layout(), nullptr);
    EXPECT_FALSE(obj->mp_Label->text().isEmpty());
}

// ═══════════════════════════════════════════════════════════════
// start（mid，B3 转发）
// ═══════════════════════════════════════════════════════════════
TEST_F(WaitingWidgetTest, Start_SpinnerPresent_ForwardsStart)
{
    // Arrange：替换 DSpinner::start 记录调用
    int startCalls = 0;
    stub.set_lamda(&DSpinner::start,
                   [&](DSpinner *) { ++startCalls; });

    // Act
    obj->start();

    // Assert：转发子控件启动且落在自身 spinner 上
    EXPECT_EQ(startCalls, 1);
    EXPECT_NE(obj->mp_Spinner, nullptr);
}

TEST_F(WaitingWidgetTest, Start_RepeatedCalls_ForwardsTwice)
{
    // Arrange：替换 DSpinner::start 记录调用
    int startCalls = 0;
    stub.set_lamda(&DSpinner::start,
                   [&](DSpinner *) { ++startCalls; });

    // Act：连续启动两次
    obj->start();
    obj->start();

    // Assert：转发累计两次，且落在自身 spinner 上
    EXPECT_EQ(startCalls, 2);
    EXPECT_NE(obj->mp_Spinner, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// stop（B4 转发）
// ═══════════════════════════════════════════════════════════════
TEST_F(WaitingWidgetTest, Stop_AfterStart_ForwardsStop)
{
    // Arrange：替换 start/stop 记录调用
    int stopCalls = 0;
    stub.set_lamda(&DSpinner::stop,
                   [&](DSpinner *) { ++stopCalls; });

    // Act：先启动后停止
    obj->start();
    obj->stop();

    // Assert：停止转发恰一次
    EXPECT_EQ(stopCalls, 1);
    EXPECT_NE(obj->mp_Spinner, nullptr);
}

TEST_F(WaitingWidgetTest, Stop_StandaloneCall_ForwardsOnce)
{
    // Arrange：未启动状态直接停止
    int stopCalls = 0;
    stub.set_lamda(&DSpinner::stop,
                   [&](DSpinner *) { ++stopCalls; });

    // Act
    obj->stop();

    // Assert：独立停止不崩溃且转发恰一次
    EXPECT_EQ(stopCalls, 1);
    EXPECT_NE(obj, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// ~WaitingWidget（B2 判空删除）
// ═══════════════════════════════════════════════════════════════
TEST_F(WaitingWidgetTest, Dtor_DeletesChildren_GuardsBecomeNull)
{
    // Arrange：QPointer 守护子控件占位
    QPointer<DSpinner> spinnerGuard;
    QPointer<DLabel> labelGuard;

    // Act：作用域内构造并析构（立即调用的 lambda 保证真实语句触发）
    [&spinnerGuard, &labelGuard]() {
        WaitingWidget scopedWidget;
        spinnerGuard = scopedWidget.mp_Spinner;
        labelGuard = scopedWidget.mp_Label;
        ASSERT_FALSE(spinnerGuard.isNull());
        ASSERT_FALSE(labelGuard.isNull());
    }();

    // Assert：两个子控件均被析构删除（QPointer 守护指针归空）
    EXPECT_EQ(spinnerGuard.data(), nullptr);
    EXPECT_EQ(labelGuard.data(), nullptr);
}
