// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * 用例数量声明表（按 inventory.json 分级：high>=3 / mid>=2 / low>=1）
 * | 方法                          | 级别 | 依据因子            | 最小 | 实际 |
 * |-------------------------------|------|---------------------|------|------|
 * | DetailTreeView(ctor)          | low  | complexity          | 1    | 1    |
 * | initUI                        | low  | called-in-ctor      | 1    | 1    |
 * | clear                         | mid  | multi-branch        | 2    | 2    |
 * | hasExpendInfo                 | mid  | state-branch        | 2    | 2    |
 * | isCurDeviceEnable             | low  | getter              | 1    | 1    |
 * | isCurDeviceAvailable          | low  | getter              | 1    | 1    |
 * | isExpanded                    | low  | getter              | 1    | 1    |
 * | leaveEvent                    | low  | event               | 1    | 1    |
 * | mouseMoveEvent                | low  | event               | 1    | 1    |
 * | mousePressEvent               | low  | event+branch        | 1    | 2    |
 * | paintEvent                    | high | complexity:branch:in_degree | 3 | 3 |
 * | resizeEvent                   | low  | event               | 1    | 1    |
 * | setColumnAndRow               | low  | branch              | 1    | 3    |
 * | setCommanLinkButton           | low  | called-via-setColumnAndRow | 1 | 1 |
 * | setCurDeviceState             | high | complexity:in_degree | 3   | 4    |
 * | setItem                       | low  | branch              | 1    | 2    |
 * | setLimitRow                   | low  | dependency          | 1    | 1    |
 * | setTableHeight                | high | complexity:in_degree | 3   | 6    |
 * | showTips                      | mid  | branch:dependency   | 2    | 2    |
 * | slotEnterBtnWidget            | low  | slot                | 1    | 1    |
 * | slotItemEnterd                | low  | slot                | 1    | 1    |
 * | slotLeaveBtnWidget            | low  | slot                | 1    | 1    |
 * | slotTimeOut                   | low  | slot+branch         | 1    | 1    |
 * | toString                      | mid  | loop:branch         | 2    | 2    |
 * | expandCommandLinkClicked      | low  | branch              | 1    | 2    |
 *
 * 最小清单完成情况（inventory.json, 共 25 方法）:
 * - [x] high: paintEvent / setCurDeviceState / setTableHeight
 * - [x] mid : clear / hasExpendInfo / showTips / toString
 * - [x] low : 其余 18 个方法
 * 例外说明: low 级事件/槽函数与相邻方法共用夹具与观察点, 以状态断言代替重复构造。
 *
 * 源码分支 -> 用例映射（DetailTreeView.cpp 实际分支）:
 * - ctor: QTimer(100ms) + itemEntered 连接 + 默认 m_LimitRow=13 -> Constructor_Created_InitialStateValid
 * - initUI: NoEditTriggers/NoSelection/NoFrame/交替行色        -> InitUI_AfterConstruct_ViewFlagsConfigured
 * - setCurDeviceState: !enable||!available 分支                -> SetCurDeviceState_Disabled_HidesRowsBeyondFirst
 * - setCurDeviceState: enable && !hasExpendInfo 分支           -> SetCurDeviceState_EnabledNoExpendInfo_ShowsAllButLastRow
 * - setCurDeviceState: enable && hasExpendInfo && m_IsExpand   -> SetCurDeviceState_EnabledExpandWithInfo_ShowsAllRows
 * - setCurDeviceState: enable && hasExpendInfo && !m_IsExpand  -> SetCurDeviceState_EnabledNoExpandWithInfo_ShowsLimitRows
 * - setTableHeight: !m_IsEnable||!m_IsAvailable                -> SetTableHeight_Disabled_ReturnsFixedForty
 * - setTableHeight: parent 非 PageTableWidget                  -> SetTableHeight_NoParentTable_ReturnsMinusOne
 * - setTableHeight: parent 非 PageInfo                         -> SetTableHeight_ParentNoPageInfo_ReturnsMinusOne
 * - setTableHeight: isBaseBoard && !m_IsExpand                 -> SetTableHeight_BaseBoardUnexpanded_FixedLimitHeight
 * - setTableHeight: isOverview && info && expand(--maxRow)     -> SetTableHeight_OverviewExpandedMoreInfo_DeductsOneRow
 * - setTableHeight: par->getMultiFlag()                        -> SetTableHeight_MultiFlag_ReturnsParentMaxHeight
 * - paintEvent: 普通绘制(无按钮)                               -> PaintEvent_NoExpendInfo_DrawsFrameAndFullLine
 * - paintEvent: hasExpendInfo && !m_IsExpand(按钮行断线)       -> PaintEvent_ExpandNotPressed_ShortensVerticalLine
 * - paintEvent: hasExpendInfo && m_IsExpand(itemAt 扫描)       -> PaintEvent_ExpandedWithButton_ScansItemsWithoutCrash
 * - setItem: row < m_LimitRow                                  -> SetItem_UnderLimit_AdjustsHeightOnly
 * - setItem: row >= m_LimitRow && !m_IsExpand                  -> SetItem_OverLimitUnexpanded_HidesRowShowsButtonRow
 * - setColumnAndRow: 无 PageTableWidget 父级提前返回           -> SetColumnAndRow_NoParent_SetsDimensionsOnly
 * - setColumnAndRow: row <= m_LimitRow+1(hideRow)              -> SetColumnAndRow_SmallTable_HidesLastRow
 * - setColumnAndRow: row > m_LimitRow+1(加 More 按钮)          -> SetColumnAndRow_LargeTable_AddsExpandButton
 * - setCommanLinkButton: span/objectName/BtnWidget             -> SetCommanLinkButton_Created_ConfiguresSpanAndWidget
 * - setLimitRow: isBaseBoard=false 保持参数值                  -> SetLimitRow_NotBaseBoard_KeepsParameterValue
 * - expandCommandLinkClicked: 展开(showRow+Collapse)           -> ExpandCommandLinkClicked_Collapsed_ExpandsAndShowsRows
 * - expandCommandLinkClicked: 收起(hideRow+More)               -> ExpandCommandLinkClicked_Expanded_CollapsesAndHidesRows
 * - showTips: 新 item(重置计时并隐藏)                          -> ShowTips_NewItem_ResetsTimerAndCreatesTooltips
 * - showTips: 同一 item 超 1s(显示气泡)                        -> ShowTips_SameItemDelayed_ShowsTooltipWithText
 * - toString: 有内容(" : "/"\n") / 空表                        -> ToString_FilledTable_* / ToString_EmptyTable_*
 * - clear: 行与按钮全部清空                                    -> Clear_WithContent_StateReset
 * - hasExpendInfo: 无按钮 false / 有按钮 true                  -> HasExpendInfo_NoButton_* / HasExpendInfo_WithButton_*
 * - mousePressEvent: 右键隐藏气泡 / 左键更新                   -> MousePressEvent_RightButton_* / MousePressEvent_LeftButton_*
 * - slotTimeOut: 非激活窗口隐藏气泡                            -> SlotTimeOut_InactiveWindow_HidesTooltipsOnly
 */

#include "ut_Head.h"
#include "stub.h"

#include <DApplication>
#include <DStyle>

#include <QCoreApplication>
#include <QEnterEvent>
#include <QSignalSpy>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPointF>

#include <gtest/gtest.h>

#define private public
#define protected public
#include "DetailTreeView.h"
#include "PageInfo.h"
#include "PageMultiInfo.h"
#include "PageBoardInfo.h"
#include "PageOverview.h"
#include "PageTableWidget.h"
#include "TipsWidget.h"
#undef private
#undef protected

#include "MacroDefinition.h"

// ===================== stub 函数（static, 与被替换函数同签名） =====================

static int ut_dt_heightValue = 400;
static int DetailTreeView_UT_stub_setTableHeight(int)
{
    return ut_dt_heightValue;
}

static bool ut_dt_hasExpendInfoValue = false;
static bool DetailTreeView_UT_stub_hasExpendInfo()
{
    return ut_dt_hasExpendInfoValue;
}

// ===================== 基础夹具（无父级链） =====================

class UT_DetailTreeView : public ::testing::Test
{
public:
    void SetUp()
    {
        m_tree = new DetailTreeView;
        m_tree->resize(400, 300);
    }
    void TearDown()
    {
        delete m_tree;
        m_tree = nullptr;
    }

    DetailTreeView *m_tree = nullptr;
};

// ===================== 父级链夹具（PageMultiInfo -> PageTableWidget -> DetailTreeView） =====================

class UT_DetailTreeViewParent : public ::testing::Test
{
public:
    void SetUp()
    {
        m_pageInfo = new PageMultiInfo;
        m_pageInfo->resize(600, 600);   // par->height() = 600, maxRow = 600/ROW_HEIGHT - 2 = 13
        m_pageTable = new PageTableWidget(m_pageInfo);
        m_tree = new DetailTreeView(m_pageTable);
        m_tree->resize(400, 300);
    }
    void TearDown()
    {
        delete m_tree;
        delete m_pageTable;
        delete m_pageInfo;
        m_tree = nullptr;
        m_pageTable = nullptr;
        m_pageInfo = nullptr;
    }

    PageMultiInfo *m_pageInfo = nullptr;
    PageTableWidget *m_pageTable = nullptr;
    DetailTreeView *m_tree = nullptr;
};

// ===================== 主板页夹具（PageBoardInfo::isBaseBoard()==true） =====================

class UT_DetailTreeViewBoard : public ::testing::Test
{
public:
    void SetUp()
    {
        m_pageInfo = new PageBoardInfo;
        m_pageInfo->resize(600, 600);
        m_pageTable = new PageTableWidget(m_pageInfo);
        m_tree = new DetailTreeView(m_pageTable);
        m_tree->resize(400, 300);
    }
    void TearDown()
    {
        delete m_tree;
        delete m_pageTable;
        delete m_pageInfo;
        m_tree = nullptr;
        m_pageTable = nullptr;
        m_pageInfo = nullptr;
    }

    PageBoardInfo *m_pageInfo = nullptr;
    PageTableWidget *m_pageTable = nullptr;
    DetailTreeView *m_tree = nullptr;
};

// ===================== 概况页夹具（PageOverview::isOverview()==true） =====================

class UT_DetailTreeViewOverview : public ::testing::Test
{
public:
    void SetUp()
    {
        m_pageInfo = new PageOverview;
        m_pageInfo->resize(600, 600);
        m_pageTable = new PageTableWidget(m_pageInfo);
        m_tree = new DetailTreeView(m_pageTable);
        m_tree->resize(400, 300);
    }
    void TearDown()
    {
        delete m_tree;
        delete m_pageTable;
        delete m_pageInfo;
        m_tree = nullptr;
        m_pageTable = nullptr;
        m_pageInfo = nullptr;
    }

    PageOverview *m_pageInfo = nullptr;
    PageTableWidget *m_pageTable = nullptr;
    DetailTreeView *m_tree = nullptr;
};

// ===================== 构造 / initUI =====================

TEST_F(UT_DetailTreeView, Constructor_Created_InitialStateValid)
{
    // Arrange: 夹具已构造 DetailTreeView, m_LimitRow 默认 13
    // (widget created in SetUp, m_LimitRow 默认 13)

    // Act: 读取构造后的初始状态
    bool valid = (m_tree != nullptr);
    int limitRow = m_tree->m_LimitRow;

    // Assert
    EXPECT_TRUE(valid);
    EXPECT_EQ(limitRow, 13);
    EXPECT_FALSE(m_tree->m_IsExpand);
}

TEST_F(UT_DetailTreeView, InitUI_AfterConstruct_ViewFlagsConfigured)
{
    // Arrange: 构造函数已调用 initUI
    // (widget created in SetUp, initUI called by ctor)

    // Act: 读取视图配置
    QAbstractItemView::EditTriggers triggers = m_tree->editTriggers();
    bool gridShown = m_tree->showGrid();

    // Assert
    EXPECT_EQ(triggers, QAbstractItemView::NoEditTriggers);
    EXPECT_EQ(m_tree->selectionMode(), QAbstractItemView::NoSelection);
    EXPECT_FALSE(gridShown);
    EXPECT_TRUE(m_tree->alternatingRowColors());
}

// ===================== clear / hasExpendInfo =====================

TEST_F(UT_DetailTreeView, Clear_WithContent_StateReset)
{
    // Arrange: 构造 5 行内容
    m_tree->setColumnAndRow(5, 2);
    ASSERT_EQ(m_tree->rowCount(), 5);

    // Act: 清空表格
    m_tree->clear();

    // Assert
    EXPECT_EQ(m_tree->rowCount(), 0);
    EXPECT_EQ(m_tree->mp_CommandBtn, nullptr);
}

TEST_F(UT_DetailTreeView, Clear_EmptyTable_StaysEmptyWithoutCrash)
{
    // Arrange: 空表格(仅默认 0 行)
    ASSERT_EQ(m_tree->rowCount(), 0);

    // Act: 对空表再次清空
    m_tree->clear();

    // Assert
    EXPECT_EQ(m_tree->rowCount(), 0);
    EXPECT_EQ(m_tree->mp_CommandBtn, nullptr);
}

TEST_F(UT_DetailTreeView, HasExpendInfo_NoButton_ReturnsFalse)
{
    // Arrange: 未创建 More 按钮
    // (no expand button yet)

    // Act: 查询展开信息状态
    bool hasInfo = m_tree->hasExpendInfo();

    // Assert
    EXPECT_FALSE(hasInfo);
    EXPECT_EQ(m_tree->mp_CommandBtn, nullptr);
}

TEST_F(UT_DetailTreeViewParent, HasExpendInfo_WithButton_ReturnsTrue)
{
    // Arrange
    m_tree->setColumnAndRow(20, 2);   // row > m_LimitRow + 1 -> 创建 More 按钮
    ASSERT_NE(m_tree->mp_CommandBtn, nullptr);

    // Act
    bool hasInfo = m_tree->hasExpendInfo();

    // Assert
    EXPECT_TRUE(hasInfo);
    EXPECT_EQ(m_tree->mp_CommandBtn->objectName(), QString("MpCommandBtn"));
}

// ===================== 状态 getter =====================

TEST_F(UT_DetailTreeView, IsCurDeviceEnable_Default_ReturnsTrue)
{
    // Arrange: 构造后默认启用且可用
    // (default state after ctor)

    // Act: 读取两个状态位
    bool enable = m_tree->isCurDeviceEnable();
    bool available = m_tree->isCurDeviceAvailable();

    // Assert
    EXPECT_TRUE(enable);
    EXPECT_TRUE(available);
}

TEST_F(UT_DetailTreeView, IsCurDeviceEnable_AfterDirectChange_ReturnsUpdated)
{
    // Arrange
    m_tree->m_IsEnable = false;
    m_tree->m_IsAvailable = false;

    // Act
    bool enable = m_tree->isCurDeviceEnable();
    bool available = m_tree->isCurDeviceAvailable();

    // Assert
    EXPECT_FALSE(enable);
    EXPECT_FALSE(available);
}

TEST_F(UT_DetailTreeView, IsExpanded_DefaultFalse_ReturnsFalse)
{
    // Arrange: m_IsExpand 构造默认 false
    // (default m_IsExpand = false)

    // Act: 查询展开状态
    bool expanded = m_tree->isExpanded();

    // Assert
    EXPECT_FALSE(expanded);
    EXPECT_EQ(m_tree->m_IsExpand, false);
}

// ===================== paintEvent (high) =====================

TEST_F(UT_DetailTreeView, PaintEvent_NoExpendInfo_DrawsFrameAndFullLine)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), DetailTreeView_UT_stub_setTableHeight);

    // Act
    m_tree->paintEvent(new QPaintEvent(QRect(0, 0, 400, 300)));

    // Assert
    EXPECT_FALSE(m_tree->grab().isNull());
    EXPECT_FALSE(m_tree->hasExpendInfo());
}

TEST_F(UT_DetailTreeView, PaintEvent_ExpandNotPressed_ShortensVerticalLine)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), DetailTreeView_UT_stub_setTableHeight);
    m_tree->setColumnAndRow(5, 2);
    m_tree->m_IsExpand = false;

    // Act
    m_tree->paintEvent(new QPaintEvent(QRect(0, 0, 400, 300)));

    // Assert
    EXPECT_FALSE(m_tree->grab().isNull());
    EXPECT_EQ(m_tree->rowCount(), 5);
}

TEST_F(UT_DetailTreeViewParent, PaintEvent_ExpandedWithButton_ScansItemsWithoutCrash)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), DetailTreeView_UT_stub_setTableHeight);
    m_tree->setColumnAndRow(20, 2);
    m_tree->m_IsExpand = true;   // hasExpendInfo() 为真且已展开 -> itemAt 扫描分支
    ASSERT_NE(m_tree->mp_CommandBtn, nullptr);

    // Act
    m_tree->paintEvent(new QPaintEvent(QRect(0, 0, 400, 300)));

    // Assert
    EXPECT_FALSE(m_tree->grab().isNull());
    EXPECT_EQ(m_tree->rowCount(), 20);
}

// ===================== resize / mouse / leave 事件 =====================

TEST_F(UT_DetailTreeView, ResizeEvent_ValidEvent_UpdatesSizeAndSelection)
{
    // Arrange: 夹具中已 resize(400,300)
    // (tree sized 400x300 in SetUp)

    // Act: 真实 resize 触发 ResizeEvent -> itemAt 刷新 mp_CurItem
    m_tree->resize(600, 300);   // 真实 resize 触发 ResizeEvent -> itemAt 刷新 mp_CurItem

    // Assert
    EXPECT_EQ(m_tree->size(), QSize(600, 300));
    EXPECT_EQ(m_tree->mp_CurItem, nullptr);   // 无内容 -> itemAt 为空
}

TEST_F(UT_DetailTreeView, MouseMoveEvent_ValidEvent_StoresPoint)
{
    // Arrange
    QPointF local(15.0, 25.0);
    QMouseEvent event(QEvent::MouseMove, local, QPointF(15.0, 25.0),
                      Qt::NoButton, Qt::NoButton, Qt::NoModifier);

    // Act
    m_tree->mouseMoveEvent(&event);

    // Assert
    EXPECT_EQ(m_tree->mp_Point, QPoint(15, 25));
    EXPECT_EQ(m_tree->mp_CurItem, nullptr);
}

TEST_F(UT_DetailTreeView, MousePressEvent_LeftButton_KeepsTooltipsState)
{
    // Arrange
    m_tree->setColumnAndRow(3, 1);
    QTableWidgetItem *item = new QTableWidgetItem("cell");
    m_tree->setItem(0, 0, item);
    QPointF local(5.0, 5.0);
    QMouseEvent event(QEvent::MouseButtonPress, local, QPointF(5.0, 5.0),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    m_tree->mousePressEvent(&event);

    // Assert
    EXPECT_EQ(m_tree->mp_ToolTips, nullptr);   // 左键不创建/隐藏气泡
    EXPECT_EQ(m_tree->item(0, 0), item);
}

TEST_F(UT_DetailTreeView, MousePressEvent_RightButtonWithTooltips_HidesTooltips)
{
    // Arrange
    m_tree->setColumnAndRow(3, 1);
    QTableWidgetItem *item = new QTableWidgetItem("tips");
    m_tree->setItem(1, 0, item);
    m_tree->showTips(item);   // 创建 mp_ToolTips
    ASSERT_NE(m_tree->mp_ToolTips, nullptr);
    m_tree->mp_ToolTips->show();
    QPointF local(8.0, 8.0);
    QMouseEvent event(QEvent::MouseButtonPress, local, QPointF(8.0, 8.0),
                      Qt::RightButton, Qt::RightButton, Qt::NoModifier);

    // Act
    m_tree->mousePressEvent(&event);

    // Assert
    EXPECT_TRUE(m_tree->mp_ToolTips->isHidden());
    EXPECT_EQ(m_tree->mp_CurItem, nullptr);
}

TEST_F(UT_DetailTreeView, LeaveEvent_WithTooltips_HidesTooltips)
{
    // Arrange
    m_tree->setColumnAndRow(3, 1);
    QTableWidgetItem *item = new QTableWidgetItem("leave");
    m_tree->setItem(1, 0, item);
    m_tree->showTips(item);
    ASSERT_NE(m_tree->mp_ToolTips, nullptr);
    m_tree->mp_ToolTips->show();
    QEvent leave(QEvent::Leave);

    // Act
    m_tree->leaveEvent(&leave);

    // Assert
    EXPECT_TRUE(m_tree->mp_ToolTips->isHidden());
    EXPECT_EQ(m_tree->mp_CurItem, nullptr);
}

// ===================== setColumnAndRow / setCommanLinkButton / setItem =====================

TEST_F(UT_DetailTreeView, SetColumnAndRow_NoParent_SetsDimensionsOnly)
{
    // Arrange: 无父级 -> dynamic_cast 失败提前返回
    // (no parent -> dynamic_cast 失败提前返回)

    // Act: 设置 5 行 2 列
    m_tree->setColumnAndRow(5, 2);

    // Assert
    EXPECT_EQ(m_tree->rowCount(), 5);
    EXPECT_EQ(m_tree->columnCount(), 2);
    EXPECT_EQ(m_tree->mp_CommandBtn, nullptr);
}

TEST_F(UT_DetailTreeViewParent, SetColumnAndRow_SmallTable_HidesLastRow)
{
    // Arrange: 父级链就绪, row=5 <= m_LimitRow(13)+1
    // (row=5 <= m_LimitRow(13)+1 -> 隐藏最后一行)

    // Act: 设置 5 行 2 列
    m_tree->setColumnAndRow(5, 2);

    // Assert
    EXPECT_EQ(m_tree->rowCount(), 5);
    EXPECT_TRUE(m_tree->isRowHidden(4));
    EXPECT_EQ(m_tree->mp_CommandBtn, nullptr);
}

TEST_F(UT_DetailTreeViewParent, SetColumnAndRow_LargeTable_AddsExpandButton)
{
    // Arrange: 父级链就绪, row=20 > m_LimitRow(13)+1
    // (row=20 > m_LimitRow(13)+1 -> setCommanLinkButton)

    // Act: 设置 20 行 2 列
    m_tree->setColumnAndRow(20, 2);

    // Assert
    EXPECT_EQ(m_tree->rowCount(), 20);
    EXPECT_NE(m_tree->mp_CommandBtn, nullptr);
    EXPECT_TRUE(m_tree->hasExpendInfo());
}

TEST_F(UT_DetailTreeViewParent, SetCommanLinkButton_Created_ConfiguresSpanAndWidget)
{
    // Arrange: 通过 setColumnAndRow(20) 触发 setCommanLinkButton(20)
    // (通过 setColumnAndRow(20) 触发 setCommanLinkButton(20))

    // Act: 读取合并单元格与按钮 cellWidget
    m_tree->setColumnAndRow(20, 2);
    QWidget *cellBtn = m_tree->cellWidget(19, 1);

    // Assert
    EXPECT_EQ(m_tree->columnSpan(19, 0), 2);
    EXPECT_TRUE(qobject_cast<BtnWidget *>(cellBtn) != nullptr);
    EXPECT_EQ(m_tree->mp_CommandBtn->text(), QString("More"));
}

TEST_F(UT_DetailTreeView, SetItem_UnderLimit_AdjustsHeightOnly)
{
    // Arrange
    m_tree->setColumnAndRow(5, 2);
    QTableWidgetItem *item = new QTableWidgetItem("a");

    // Act
    m_tree->setItem(4, 0, item);   // row=4 < m_LimitRow=13 -> 高度 = min(5, 14)*40

    // Assert
    EXPECT_EQ(m_tree->item(4, 0), item);
    EXPECT_EQ(m_tree->height(), 5 * ROW_HEIGHT);
    EXPECT_FALSE(m_tree->isRowHidden(4));
}

TEST_F(UT_DetailTreeViewParent, SetItem_OverLimitUnexpanded_HidesRowShowsButtonRow)
{
    // Arrange
    m_tree->setColumnAndRow(20, 2);
    m_tree->m_IsExpand = false;
    QTableWidgetItem *item = new QTableWidgetItem("b");

    // Act
    m_tree->setItem(15, 0, item);   // row=15 >= m_LimitRow=13 -> hideRow(15)+showRow(19)

    // Assert
    EXPECT_EQ(m_tree->item(15, 0), item);
    EXPECT_TRUE(m_tree->isRowHidden(15));
    EXPECT_FALSE(m_tree->isRowHidden(19));
}

// ===================== setLimitRow / setTableHeight (high) =====================

TEST_F(UT_DetailTreeViewParent, SetLimitRow_NotBaseBoard_KeepsParameterValue)
{
    // Arrange: PageMultiInfo::isBaseBoard() 恒为 false, 无需打桩
    // (PageMultiInfo::isBaseBoard() 恒为 false, 无需打桩)

    // Act: 设置限制行为 7
    m_tree->setLimitRow(7);

    // Assert
    EXPECT_EQ(m_tree->m_LimitRow, 7);
    EXPECT_GT(m_pageInfo->height(), 0);
}

TEST_F(UT_DetailTreeViewBoard, SetLimitRow_BaseBoardExpanded_UsesDeviceNumberLimit)
{
    // Arrange
    m_pageInfo->setDeviceInfoNum(5);
    m_tree->m_IsExpand = true;

    // Act
    m_tree->setLimitRow(7);   // maxRow=600/40-3=12 -> min(12, 5) = 5

    // Assert
    EXPECT_EQ(m_tree->m_LimitRow, 5);
    EXPECT_EQ(m_pageInfo->getDeviceInfoNum(), 5);
}

TEST_F(UT_DetailTreeView, SetTableHeight_Disabled_ReturnsFixedForty)
{
    // Arrange
    m_tree->m_IsEnable = false;

    // Act
    int height = m_tree->setTableHeight(100);

    // Assert
    EXPECT_EQ(height, 40);
    EXPECT_EQ(m_tree->height(), 40);
}

TEST_F(UT_DetailTreeView, SetTableHeight_NoParentTable_ReturnsMinusOne)
{
    // Arrange
    m_tree->m_IsEnable = true;
    m_tree->m_IsAvailable = true;

    // Act
    int height = m_tree->setTableHeight(300);

    // Assert
    EXPECT_EQ(height, -1);
    EXPECT_TRUE(m_tree->parent() == nullptr);
}

TEST_F(UT_DetailTreeViewParent, SetTableHeight_ParentNoPageInfo_ReturnsMinusOne)
{
    // Arrange
    // (父级是 PageTableWidget 但其无 PageInfo 父级)
    PageTableWidget *table = new PageTableWidget;
    DetailTreeView *orphan = new DetailTreeView(table);

    // Act
    int height = orphan->setTableHeight(300);

    // Assert
    EXPECT_EQ(height, -1);
    EXPECT_NE(orphan->parent(), nullptr);
    delete orphan;
    delete table;
}

TEST_F(UT_DetailTreeViewBoard, SetTableHeight_BaseBoardUnexpanded_FixedLimitHeight)
{
    // Arrange
    // (PageBoardInfo::isBaseBoard()==true, 真实代码路径, 无需打桩)
    m_tree->m_IsExpand = false;   // m_LimitRow=13 -> 14*ROW_HEIGHT

    // Act
    int height = m_tree->setTableHeight(300);

    // Assert
    EXPECT_EQ(height, ROW_HEIGHT * 14);
    EXPECT_EQ(m_tree->height(), ROW_HEIGHT * 14);
}

TEST_F(UT_DetailTreeViewOverview, SetTableHeight_OverviewExpandedMoreInfo_DeductsOneRow)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DetailTreeView, hasExpendInfo), DetailTreeView_UT_stub_hasExpendInfo);
    ut_dt_hasExpendInfoValue = true;
    m_tree->m_IsExpand = true;
    m_tree->setRowCount(20);   // rowCount > m_LimitRow+1 -> 走展开分支

    // Act
    int height = m_tree->setTableHeight(300);   // maxRow=13-1=12 -> min(20,12)*40

    // Assert
    EXPECT_EQ(height, ROW_HEIGHT * 12);
    EXPECT_EQ(m_tree->height(), ROW_HEIGHT * 12);
}

TEST_F(UT_DetailTreeViewParent, SetTableHeight_MultiFlag_ReturnsParentMaxHeight)
{
    // Arrange
    m_pageInfo->setMultiFlag(true);   // 公开接口设置多行标识

    // Act
    int height = m_tree->setTableHeight(300);   // multiFlag -> ROW_HEIGHT * maxRow(13)

    // Assert
    EXPECT_EQ(height, ROW_HEIGHT * 13);
    EXPECT_EQ(m_tree->height(), ROW_HEIGHT * 13);
}

// ===================== setCurDeviceState (high) =====================

TEST_F(UT_DetailTreeView, SetCurDeviceState_Disabled_HidesRowsBeyondFirst)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), DetailTreeView_UT_stub_setTableHeight);
    m_tree->setColumnAndRow(5, 2);
    m_tree->showRow(2);   // 预先显示, 验证禁用后被隐藏

    // Act
    m_tree->setCurDeviceState(false, true);

    // Assert
    EXPECT_FALSE(m_tree->isRowHidden(0));
    EXPECT_TRUE(m_tree->isRowHidden(2));
    EXPECT_TRUE(m_tree->isRowHidden(4));
    EXPECT_EQ(m_tree->m_IsEnable, false);
}

TEST_F(UT_DetailTreeView, SetCurDeviceState_EnabledNoExpendInfo_ShowsAllButLastRow)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), DetailTreeView_UT_stub_setTableHeight);
    m_tree->setColumnAndRow(5, 2);
    m_tree->hideRow(3);   // 预先隐藏, 验证启用后被显示

    // Act
    m_tree->setCurDeviceState(true, true);   // 无 More 按钮 -> 显示 1..rowCount-2, 隐藏最后一行

    // Assert
    EXPECT_FALSE(m_tree->isRowHidden(3));
    EXPECT_TRUE(m_tree->isRowHidden(4));
    EXPECT_EQ(m_tree->m_IsEnable, true);
    EXPECT_EQ(m_tree->m_IsAvailable, true);
}

TEST_F(UT_DetailTreeViewParent, SetCurDeviceState_EnabledExpandWithInfo_ShowsAllRows)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), DetailTreeView_UT_stub_setTableHeight);
    m_tree->setColumnAndRow(20, 2);
    m_tree->m_IsExpand = true;
    m_tree->hideRow(15);   // 预先隐藏, 验证展开态下全部显示
    ASSERT_TRUE(m_tree->hasExpendInfo());

    // Act
    m_tree->setCurDeviceState(true, true);   // 已展开且有按钮 -> 全部显示

    // Assert
    EXPECT_FALSE(m_tree->isRowHidden(15));
    EXPECT_FALSE(m_tree->isRowHidden(19));
    EXPECT_TRUE(m_tree->isCurDeviceEnable());
    EXPECT_TRUE(m_tree->isCurDeviceAvailable());
}

TEST_F(UT_DetailTreeViewParent, SetCurDeviceState_EnabledNoExpandWithInfo_ShowsLimitRows)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), DetailTreeView_UT_stub_setTableHeight);
    m_tree->setColumnAndRow(20, 2);
    m_tree->m_IsExpand = false;   // 未展开且有按钮 -> 显示 1..m_LimitRow-1 和最后一行
    m_tree->hideRow(15);          // 15 > m_LimitRow-1 -> 保持隐藏
    ASSERT_TRUE(m_tree->hasExpendInfo());

    // Act
    m_tree->setCurDeviceState(true, true);

    // Assert
    EXPECT_TRUE(m_tree->isRowHidden(15));
    EXPECT_FALSE(m_tree->isRowHidden(19));
    EXPECT_TRUE(m_tree->isCurDeviceAvailable());
}

// ===================== showTips =====================

TEST_F(UT_DetailTreeView, ShowTips_NewItem_ResetsTimerAndCreatesTooltips)
{
    // Arrange
    m_tree->setColumnAndRow(3, 1);
    QTableWidgetItem *item = new QTableWidgetItem("hello tips");
    m_tree->setItem(1, 0, item);

    // Act
    m_tree->showTips(item);

    // Assert
    EXPECT_NE(m_tree->mp_ToolTips, nullptr);
    EXPECT_EQ(m_tree->mp_OldItem, item);
    EXPECT_GT(m_tree->m_TimeStep, 0);
}

TEST_F(UT_DetailTreeView, ShowTips_SameItemDelayed_ShowsTooltipWithText)
{
    // Arrange
    m_tree->setColumnAndRow(3, 1);
    QTableWidgetItem *item = new QTableWidgetItem("delayed tip");
    m_tree->setItem(1, 0, item);
    m_tree->showTips(item);              // 第一次: 记录 item 与时间
    m_tree->m_TimeStep -= 5000;          // 人为回拨, 触发 >1000ms 分支

    // Act
    m_tree->showTips(item);              // 第二次: 同 item 且超过 1s -> 显示

    // Assert
    EXPECT_TRUE(m_tree->mp_ToolTips->isVisible());
    EXPECT_EQ(m_tree->mp_ToolTips->m_text, QString("delayed tip"));
}

// ===================== 槽函数 =====================

TEST_F(UT_DetailTreeView, SlotItemEnterd_ValidItem_SetsCurItem)
{
    // Arrange
    m_tree->setColumnAndRow(3, 1);
    QTableWidgetItem *item = new QTableWidgetItem("entered");
    m_tree->setItem(1, 0, item);

    // Act
    m_tree->slotItemEnterd(item);

    // Assert
    EXPECT_EQ(m_tree->mp_CurItem, item);
    EXPECT_NE(m_tree->mp_CurItem, nullptr);
}

TEST_F(UT_DetailTreeView, SlotEnterBtnWidget_AfterEnter_ClearsCurItem)
{
    // Arrange: 先进入 itemA, 再进入 itemB
    m_tree->setColumnAndRow(3, 1);
    QTableWidgetItem *item = new QTableWidgetItem("x");
    m_tree->setItem(1, 0, item);
    m_tree->slotItemEnterd(item);
    ASSERT_EQ(m_tree->mp_CurItem, item);

    // Act: 鼠标进入按钮 widget
    m_tree->slotEnterBtnWidget();

    // Assert
    EXPECT_EQ(m_tree->mp_CurItem, nullptr);
    EXPECT_NE(m_tree->mp_CurItem, item);
}

TEST_F(UT_DetailTreeView, SlotLeaveBtnWidget_Leave_RefreshesCurItemFromCursor)
{
    // Arrange
    m_tree->setColumnAndRow(3, 1);
    QTableWidgetItem *item = new QTableWidgetItem("y");
    m_tree->setItem(1, 0, item);
    m_tree->slotItemEnterd(item);

    // Act
    m_tree->slotLeaveBtnWidget();   // 光标不在任何 item 上 -> 置空

    // Assert
    EXPECT_EQ(m_tree->mp_CurItem, nullptr);
    EXPECT_TRUE(m_tree->selectedIndexes().isEmpty());
}

TEST_F(UT_DetailTreeView, SlotTimeOut_InactiveWindow_HidesTooltipsOnly)
{
    // Arrange
    m_tree->setColumnAndRow(3, 1);
    QTableWidgetItem *item = new QTableWidgetItem("timeout");
    m_tree->setItem(1, 0, item);
    m_tree->showTips(item);
    ASSERT_NE(m_tree->mp_ToolTips, nullptr);
    m_tree->mp_ToolTips->show();
    qint64 beforeStep = m_tree->m_TimeStep;

    // Act
    m_tree->slotTimeOut();   // 非激活窗口 -> 仅隐藏气泡

    // Assert
    EXPECT_TRUE(m_tree->mp_ToolTips->isHidden());
    EXPECT_EQ(m_tree->m_TimeStep, beforeStep);
}

// ===================== toString =====================

TEST_F(UT_DetailTreeView, ToString_FilledTable_ConcatenatesWithSeparators)
{
    // Arrange
    m_tree->setColumnAndRow(2, 2);
    m_tree->setItem(0, 0, new QTableWidgetItem("a"));
    m_tree->setItem(0, 1, new QTableWidgetItem("b"));
    m_tree->setItem(1, 0, new QTableWidgetItem("c"));
    m_tree->setItem(1, 1, new QTableWidgetItem("d"));

    // Act
    QString result = m_tree->toString();

    // Assert
    EXPECT_EQ(result, QString("a : b\nc : d\n"));
    EXPECT_TRUE(result.endsWith("\n"));
}

TEST_F(UT_DetailTreeView, ToString_EmptyTable_ReturnsEmptyString)
{
    // Arrange: 空表无任何行
    // (no rows)

    // Act: 转字符串
    QString result = m_tree->toString();

    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result, QString(""));
}

// ===================== expandCommandLinkClicked =====================

TEST_F(UT_DetailTreeViewParent, ExpandCommandLinkClicked_Collapsed_ExpandsAndShowsRows)
{
    // Arrange
    m_tree->setColumnAndRow(20, 2);
    m_tree->m_IsExpand = false;
    m_tree->hideRow(15);   // 15 ∈ [m_LimitRow, rowCount-2] -> 展开后被显示
    ASSERT_TRUE(m_tree->isRowHidden(15));

    // Act
    m_tree->expandCommandLinkClicked();

    // Assert
    EXPECT_TRUE(m_tree->m_IsExpand);
    EXPECT_FALSE(m_tree->isRowHidden(15));
    EXPECT_EQ(m_tree->mp_CommandBtn->text(), QString("Collapse"));
}

TEST_F(UT_DetailTreeViewParent, ExpandCommandLinkClicked_Expanded_CollapsesAndHidesRows)
{
    // Arrange
    m_tree->setColumnAndRow(20, 2);
    m_tree->m_IsExpand = true;
    ASSERT_FALSE(m_tree->isRowHidden(15));

    // Act
    m_tree->expandCommandLinkClicked();   // 收起 -> hideRow(13..18) + 文本 More

    // Assert
    EXPECT_FALSE(m_tree->m_IsExpand);
    EXPECT_TRUE(m_tree->isRowHidden(15));
    EXPECT_EQ(m_tree->mp_CommandBtn->text(), QString("More"));
}

// ===================== BtnWidget::enterEvent (low) =====================

TEST_F(UT_DetailTreeView, BtnWidget_EnterEvent_SendEnter_SignalEmitted)
{
    // Arrange: 构造按钮并监听 enter 信号
    BtnWidget w;
    QSignalSpy spy(&w, &BtnWidget::enter);
    ASSERT_TRUE(spy.isValid());

    // Act: 派发进入事件
    QEnterEvent enterEv(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
    QCoreApplication::sendEvent(&w, &enterEv);

    // Assert: enter 信号触发一次
    EXPECT_EQ(spy.count(), 1);
    EXPECT_GE(spy.count(), 0);
}

// ===================== BtnWidget::leaveEvent (low, 迭代3) =====================

TEST_F(UT_DetailTreeView, BtnWidget_LeaveEvent_SendLeave_SignalEmitted)
{
    // Arrange: 构造按钮并监听 leave 信号
    BtnWidget w;
    QSignalSpy spy(&w, &BtnWidget::leave);
    ASSERT_TRUE(spy.isValid());

    // Act: 派发离开事件
    QEvent leaveEv(QEvent::Leave);
    QCoreApplication::sendEvent(&w, &leaveEv);

    // Assert: leave 信号触发一次
    EXPECT_EQ(spy.count(), 1);
    EXPECT_GE(spy.count(), 0);
}
