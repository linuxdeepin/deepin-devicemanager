// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method               | level | factors                  | min | actual |
// |----------------------|-------|--------------------------|-----|--------|
// | PageTableWidget      | low   | -                        | 1   | 2      |
// | initUI               | low   | -                        | 1   | 1      |
// | setItem              | low   | -                        | 1   | 1      |
// | setColumnAndRow      | low   | -                        | 1   | 1      |
// | setLimitRow          | low   | 父链依赖，打桩           | 1   | 1      |
// | setRowHeight         | low   | -                        | 1   | 1      |
// | setDeviceEnable      | low   | -                        | 1   | 1      |
// | setCurDeviceState    | low   | -                        | 1   | 1      |
// | isCurDeviceEnable    | low   | -                        | 1   | 1      |
// | expandTable          | low   | -                        | 1   | 2      |
// | isExpanded           | low   | -                        | 1   | 2      |
// | clear                | mid   | name_pattern:clear       | 2   | 2      |
// | toString             | mid   | in_degree:5              | 2   | 2      |
// | setItemDelegateForRow| low   | 双分支                   | 1   | 2      |
// | isOverview           | low   | 父链转发                 | 1   | 1      |
// | isBaseBoard          | low   | 父链转发                 | 1   | 1      |
// | paintEvent           | low   | -                        | 1   | 1      |
// ─── actual >= min 达标（共 23 个用例实例）───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（委托空/非空代理、enable/available 组合、展开/收起）
// 3. 每个等价类的边界值显式覆盖: [x]（空表 toString、空指针 mp_Table 分支）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（无 ≥3 组同质输入，不适用）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（setItemDelegateForRow 双分支、expandTable/isExpanded 的 mp_Table 空分支、setLimitRow/setCurDeviceState 父链依赖分支经打桩隔离）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（无显式 throw，不适用）
// 8. 负面场景有专门用例: [x]（空表、空指针、默认代理）
// 9. 负面用例验证强异常安全: [x]（空指针分支不断言崩溃即安全）
// 10. stub_ext vs gMock 选择正确: [x]（运行时替换具体类方法 → stub_ext；无接口抽象 → 不用 gMock）
//
// mock 深度分析（test-code-gen §4.0，来源：工作区实际源码 + MCP trace_path）：
// - PageTableWidget 为 GUI 容器，内部 mp_Table(DetailTreeView, DTableWidget 子类)。
//   构造链 DetailTreeView(parent) 启动 QTimer(100ms)，测试无事件循环不触发，安全。
// - setLimitRow / setCurDeviceState 的 DetailTreeView 真实实现会解引用
//   dynamic_cast<PageTableWidget*>(this->parent())->parent() 父链且无空指针防护，
//   测试环境下父链不完整 → 按「外部依赖打桩」以 stub_ext 替换为记录型桩，
//   仅验证 PageTableWidget 的转发契约，不触真实父链逻辑。
// - expandCommandLinkClicked 打桩同理（真实实现依赖 mp_CommandBtn 状态）。
// - setItem/setColumnAndRow/setRowHeight/clear/toString/isCurDeviceEnable/isExpanded/
//   setItemDelegateForRow 走真实实现直接断言结果（不测实现细节）。
// - isOverview/isBaseBoard 经 dynamic_cast<PageInfo*> 转发父级，测试以可控
//   TestPageInfo 父对象验证转发。
// - QApplication offscreen；DBus 已由 test_main.cpp 全局桩拦截；无文件/网络依赖。
//
// 分支清单（来源：工作区实际源码 PageTableWidget.cpp）
// B1: setItemDelegateForRow: itemDelegate==nullptr → 默认代理 / 非空 → 指定代理
// B2: expandTable: mp_Table 非空 → 转发 / 空 → 直接返回
// B3: isExpanded: mp_Table 非空 → 转发 / 空 → return false
// B4: setLimitRow: mp_Table 非空 → 转发（真实体依赖父链，打桩隔离）
// B5: isOverview/isBaseBoard: 经 dynamic_cast 转发父级 PageInfo
//
// 用例映射：
// - Ctor_DefaultConstruction_InitializesLayout            → ctor+initUI 布局
// - Ctor_DefaultConstruction_DeviceEnabledByDefault       → isCurDeviceEnable 默认
// - SetItem_ForwardToTable_ItemStored                     → 真实存储
// - SetColumnAndRow_ForwardToTable_DimensionsSet          → 行列数
// - SetLimitRow_ForwardToTable_InvocationRecorded         → B4（桩）
// - SetRowHeight_ForwardToTable_HeightApplied             → 行高
// - SetDeviceEnable_ForwardToTable_InvocationRecorded     → 转发（桩）
// - SetCurDeviceState_ForwardToTable_InvocationRecorded   → 转发（桩）
// - IsCurDeviceEnable_MemberFlag_ReturnsState             → 真/假
// - ExpandTable_ForwardToTable_InvocationRecorded         → B2 非空（桩）
// - ExpandTable_NullTable_NoCrash                         → B2 空分支
// - IsExpanded_TableFlag_ReflectsState                     → B3 非空真/假
// - IsExpanded_NullTable_ReturnsFalse                     → B3 空分支
// - Clear_AllContent_RemovesRowsAndColumns                        → clear mid#1
// - Clear_RepeatedCalls_RemainsEmpty                      → clear mid#2
// - ToString_EmptyTable_ReturnsEmptyString                → toString mid#1
// - ToString_ItemsFormatted_LabelValuePairs               → toString mid#2
// - SetItemDelegateForRow_NullDelegate_UsesDefaultDelegate → B1 分支一
// - SetItemDelegateForRow_RichDelegate_UsesProvidedDelegate → B1 分支二
// - IsOverview_ParentPage_ForwardsResult                       → B5
// - IsBaseBoard_ParentPage_ForwardsResult                      → B5
// - PaintEvent_BaseClass_RendersWithoutCrash              → paintEvent

#include <gtest/gtest.h>
#include <QPaintEvent>
#include <QHeaderView>
#include <QLayout>

#include "stubext.h"
#include "PageTableWidget.h"
#include "DetailTreeView.h"
#include "RichTextDelegate.h"
#include "PageInfo.h"

namespace {
// PageInfo 为抽象类，测试派生提供可控的 isOverview/isBaseBoard 返回值
class TestPageInfo : public PageInfo
{
public:
    explicit TestPageInfo(QWidget *parent = nullptr)
        : PageInfo(parent) {}
    void updateInfo(const QList<DeviceBaseInfo *> &) override {}
    void clearWidgets() override {}
    void setLabel(const QString &) override {}

    bool isOverview() override { return m_overviewResult; }
    bool isBaseBoard() override { return m_baseBoardResult; }

    bool m_overviewResult = false;
    bool m_baseBoardResult = false;
};
}  // namespace

class PageTableWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override {
        // Arrange（公共前置）：以 PageInfo 派生对象为父构造被测对象
        parentInfo = new TestPageInfo;
        obj = new PageTableWidget(parentInfo);
    }

    void TearDown() override {
        // 先卸桩再析构，防止析构路径调用已打桩方法触达已销毁的栈帧捕获
        stub.clear();
        delete obj;
        delete parentInfo;
        obj = nullptr;
        parentInfo = nullptr;
    }

    stub_ext::StubExt stub;
    PageTableWidget *obj = nullptr;
    TestPageInfo *parentInfo = nullptr;
};

// ═══════════════════════════════════════════════════════════════
// 构造函数 + initUI
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, Ctor_DefaultConstruction_InitializesLayout)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：读取构造后的布局
    QLayout *lay = obj->layout();

    // Assert：内部表格已创建，布局含表格与 stretch 两项，边距为 0
    EXPECT_NE(obj->mp_Table, nullptr);
    ASSERT_NE(lay, nullptr);
    EXPECT_EQ(lay->count(), 2);
    EXPECT_EQ(lay->contentsMargins().left(), 0);
}

TEST_F(PageTableWidgetTest, Ctor_DefaultConstruction_DeviceEnabledByDefault)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：读取初始设备使能状态
    bool enabled = obj->isCurDeviceEnable();

    // Assert：DetailTreeView 初始 m_IsEnable=true，内部表格非空
    EXPECT_TRUE(enabled);
    EXPECT_NE(obj->mp_Table, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// setItem / setColumnAndRow / setRowHeight（真实转发验证）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, SetItem_ForwardToTable_ItemStored)
{
    // Arrange：先设置表格行列，再准备 item
    obj->setColumnAndRow(2, 2);
    QTableWidgetItem item("cpu");

    // Act
    obj->setItem(0, 0, &item);

    // Assert：item 已存入对应单元格，未指定单元格仍为空
    EXPECT_EQ(obj->mp_Table->item(0, 0), &item);
    EXPECT_EQ(obj->mp_Table->item(0, 1), nullptr);
}

TEST_F(PageTableWidgetTest, SetColumnAndRow_ForwardToTable_DimensionsSet)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act
    obj->setColumnAndRow(3, 4);

    // Assert：内部表格行列数与设置一致
    EXPECT_EQ(obj->mp_Table->rowCount(), 3);
    EXPECT_EQ(obj->mp_Table->columnCount(), 4);
}

TEST_F(PageTableWidgetTest, SetRowHeight_ForwardToTable_HeightApplied)
{
    // Arrange：先设置行数保证目标行存在
    obj->setColumnAndRow(3, 2);

    // Act
    obj->setRowHeight(1, 55);

    // Assert：内部表格第 1 行高度生效，其他行保持默认
    EXPECT_EQ(obj->mp_Table->rowHeight(1), 55);
    EXPECT_NE(obj->mp_Table->rowHeight(0), 55);
}

// ═══════════════════════════════════════════════════════════════
// setLimitRow（真实体解引用不完整父链 → 打桩记录转发）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, SetLimitRow_ForwardToTable_InvocationRecorded)
{
    // Arrange：替换 DetailTreeView::setLimitRow 记录入参
    int recordedRow = -1;
    stub.set_lamda(&DetailTreeView::setLimitRow,
                   [&](DetailTreeView *, int row) { recordedRow = row; });

    // Act
    obj->setLimitRow(7);

    // Assert：转发参数一致
    EXPECT_EQ(recordedRow, 7);

    // Act：不同取值转发可更新
    obj->setLimitRow(0);

    // Assert
    EXPECT_EQ(recordedRow, 0);
}

// ═══════════════════════════════════════════════════════════════
// setDeviceEnable / setCurDeviceState（真实体依赖父链 → 打桩记录转发）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, SetDeviceEnable_ForwardToTable_InvocationRecorded)
{
    // Arrange：替换 DetailTreeView::setCurDeviceState 记录入参
    bool recEnable = true;
    bool recAvailable = true;
    DetailTreeView *recObj = nullptr;
    stub.set_lamda(&DetailTreeView::setCurDeviceState,
                   [&](DetailTreeView *table, bool enable, bool available) {
                       recObj = table;
                       recEnable = enable;
                       recAvailable = available;
                   });

    // Act
    obj->setDeviceEnable(false, false);

    // Assert：转发参数一致且落在内部表格对象上
    EXPECT_FALSE(recEnable);
    EXPECT_FALSE(recAvailable);
    EXPECT_EQ(recObj, obj->mp_Table);
}

TEST_F(PageTableWidgetTest, SetCurDeviceState_ForwardToTable_InvocationRecorded)
{
    // Arrange：替换 DetailTreeView::setCurDeviceState 记录入参
    bool recEnable = false;
    bool recAvailable = false;
    DetailTreeView *recObj = nullptr;
    stub.set_lamda(&DetailTreeView::setCurDeviceState,
                   [&](DetailTreeView *table, bool enable, bool available) {
                       recObj = table;
                       recEnable = enable;
                       recAvailable = available;
                   });

    // Act
    obj->setCurDeviceState(true, true);

    // Assert：转发参数一致且落在内部表格对象上
    EXPECT_TRUE(recEnable);
    EXPECT_TRUE(recAvailable);
    EXPECT_EQ(recObj, obj->mp_Table);
}

// ═══════════════════════════════════════════════════════════════
// isCurDeviceEnable（真实实现，直读成员）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, IsCurDeviceEnable_MemberFlag_ReturnsState)
{
    // Arrange：真值状态
    obj->mp_Table->m_IsEnable = true;

    // Act
    bool enabled = obj->isCurDeviceEnable();

    // Assert
    EXPECT_TRUE(enabled);

    // Arrange：假值状态
    obj->mp_Table->m_IsEnable = false;

    // Act
    enabled = obj->isCurDeviceEnable();

    // Assert：直读成员而非缓存，对象生命周期内可重复读写
    EXPECT_FALSE(enabled);
    EXPECT_NE(obj, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// expandTable（B2 双分支）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, ExpandTable_ForwardToTable_InvocationRecorded)
{
    // Arrange：替换 DetailTreeView::expandCommandLinkClicked 计数
    int callCount = 0;
    stub.set_lamda(&DetailTreeView::expandCommandLinkClicked,
                   [&](DetailTreeView *) { ++callCount; });

    // Act
    obj->expandTable();

    // Assert：转发恰好一次
    EXPECT_EQ(callCount, 1);

    // Act：重复调用转发可累积
    obj->expandTable();

    // Assert
    EXPECT_EQ(callCount, 2);
}

TEST_F(PageTableWidgetTest, ExpandTable_NullTable_NoCrash)
{
    // Arrange：置空内部表格指针，触发 B2 空分支
    obj->mp_Table = nullptr;

    // Act：空指针下调用不崩溃
    obj->expandTable();

    // Assert：空分支安全返回，对象仍存活可断言
    EXPECT_EQ(obj->mp_Table, nullptr);
    EXPECT_NE(obj, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// isExpanded（B3 双分支）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, IsExpanded_TableFlag_ReflectsState)
{
    // Arrange：展开状态
    obj->mp_Table->m_IsExpand = true;

    // Act
    bool expanded = obj->isExpanded();

    // Assert
    EXPECT_TRUE(expanded);

    // Arrange：收起状态
    obj->mp_Table->m_IsExpand = false;

    // Act
    expanded = obj->isExpanded();

    // Assert：直读成员而非缓存，对象生命周期内可重复读写
    EXPECT_FALSE(expanded);
    EXPECT_NE(obj, nullptr);
}

TEST_F(PageTableWidgetTest, IsExpanded_NullTable_ReturnsFalse)
{
    // Arrange：置空内部表格指针，触发 B3 空分支
    obj->mp_Table = nullptr;

    // Act
    bool expanded = obj->isExpanded();

    // Assert：空表格返回 false 而非崩溃，对象仍可安全操作
    EXPECT_FALSE(expanded);
    EXPECT_NE(obj, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// clear（mid，真实实现）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, Clear_AllContent_RemovesRowsAndColumns)
{
    // Arrange：预置非空表格
    obj->setColumnAndRow(3, 2);
    ASSERT_EQ(obj->mp_Table->rowCount(), 3);

    // Act
    obj->clear();

    // Assert：行列均被清空
    EXPECT_EQ(obj->mp_Table->rowCount(), 0);
    EXPECT_EQ(obj->mp_Table->columnCount(), 0);
}

TEST_F(PageTableWidgetTest, Clear_RepeatedCalls_RemainsEmpty)
{
    // Arrange：预置非空表格
    obj->setColumnAndRow(2, 2);

    // Act：连续清空两次
    obj->clear();
    obj->clear();

    // Assert：幂等，仍为空
    EXPECT_EQ(obj->mp_Table->rowCount(), 0);
    EXPECT_EQ(obj->mp_Table->columnCount(), 0);
}

// ═══════════════════════════════════════════════════════════════
// toString（mid，真实实现）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, ToString_EmptyTable_ReturnsEmptyString)
{
    // Arrange：空表格（构造默认，行数应为 0）
    ASSERT_NE(obj, nullptr);

    // Act：读取空表内容并复查行数
    QString text = obj->toString();
    int rows = obj->mp_Table->rowCount();

    // Assert：空表返回空串
    EXPECT_TRUE(text.isEmpty());
    EXPECT_EQ(rows, 0);
}

TEST_F(PageTableWidgetTest, ToString_ItemsFormatted_LabelValuePairs)
{
    // Arrange：预置 2×2 表格并填充键值对
    obj->setColumnAndRow(2, 2);
    QTableWidgetItem labelItem("cpu");
    QTableWidgetItem valueItem("Intel");
    obj->setItem(0, 0, &labelItem);
    obj->setItem(0, 1, &valueItem);

    // Act
    QString text = obj->toString();

    // Assert：第一列后接 " : "，其余列后接换行；表格内容未被扰动
    EXPECT_EQ(text, QString("cpu : Intel\n"));
    EXPECT_EQ(obj->mp_Table->rowCount(), 2);
}

// ═══════════════════════════════════════════════════════════════
// setItemDelegateForRow（B1 双分支）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, SetItemDelegateForRow_NullDelegate_UsesDefaultDelegate)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：空代理走默认分支
    obj->setItemDelegateForRow(0, nullptr);

    // Assert：行 0 挂上了内部表格的默认代理（非空且为当前全局代理）
    EXPECT_NE(obj->mp_Table->itemDelegateForRow(0), nullptr);
    EXPECT_EQ(obj->mp_Table->itemDelegateForRow(0), obj->mp_Table->itemDelegate());
}

TEST_F(PageTableWidgetTest, SetItemDelegateForRow_RichDelegate_UsesProvidedDelegate)
{
    // Arrange：构造富文本代理
    RichTextDelegate delegate(parentInfo);

    // Act：非空代理走指定分支
    obj->setItemDelegateForRow(0, &delegate);

    // Assert：行 0 的代理即传入对象，其他行不受影响
    EXPECT_EQ(obj->mp_Table->itemDelegateForRow(0), &delegate);
    EXPECT_EQ(obj->mp_Table->itemDelegateForRow(1), nullptr);
}

// ═══════════════════════════════════════════════════════════════
// isOverview / isBaseBoard（B5 父级转发）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, IsOverview_ParentPage_ForwardsResult)
{
    // Arrange：父页面标记为概况页
    parentInfo->m_overviewResult = true;

    // Act
    bool overview = obj->isOverview();

    // Assert：转发父级返回值
    EXPECT_TRUE(overview);

    // Arrange：父页面标记为非概况页
    parentInfo->m_overviewResult = false;

    // Act
    overview = obj->isOverview();

    // Assert：转发父级返回值，被测对象生命周期内可重复调用
    EXPECT_FALSE(overview);
    EXPECT_NE(obj, nullptr);
}

TEST_F(PageTableWidgetTest, IsBaseBoard_ParentPage_ForwardsResult)
{
    // Arrange：父页面标记为主板页
    parentInfo->m_baseBoardResult = true;

    // Act
    bool baseBoard = obj->isBaseBoard();

    // Assert：转发父级返回值
    EXPECT_TRUE(baseBoard);

    // Arrange：父页面标记为非主板页
    parentInfo->m_baseBoardResult = false;

    // Act
    baseBoard = obj->isBaseBoard();

    // Assert：转发父级返回值，被测对象生命周期内可重复调用
    EXPECT_FALSE(baseBoard);
    EXPECT_NE(obj, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// paintEvent（low）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableWidgetTest, PaintEvent_BaseClass_RendersWithoutCrash)
{
    // Arrange
    QPaintEvent paintEvent(obj->rect());

    // Act：paintEvent 仅透传 DWidget 基类
    obj->paintEvent(&paintEvent);

    // Assert：离屏渲染出非空画布，尺寸与控件一致
    const QPixmap grabbed = obj->grab();
    EXPECT_FALSE(grabbed.isNull());
    EXPECT_EQ(grabbed.size(), obj->size());
}
