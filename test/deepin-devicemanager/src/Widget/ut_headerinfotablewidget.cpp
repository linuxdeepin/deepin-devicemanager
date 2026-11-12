// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_headerinfotablewidget.cpp
 * @brief HeaderInfoTableWidget 单元测试（Mode 2 重写，逐类闭环）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method              | level | factors                | min | actual |
 * |---------------------|-------|------------------------|-----|--------|
 * | ctor(initUI)        | low   | -                      | 1   | 1      |
 * | updateData          | high  | transitive_loop_depth:3| 4   | 5      |
 * | resetTableContents  | mid   | name_pattern           | 2   | 1+     |
 * | paintEvent          | low   | -                      | 1   | 2      |
 *
 * 说明：ctor→initUI 为私有链路，经构造函数用例覆盖（-fno-access-control 允许
 * 直读私有成员校验 initUI 结果）；resetTableContents 的清空语义经
 * updateData 空数据/重复调用/直接调用三类用例交叉覆盖，方法级分支已全部触达。
 *
 * 分支清单（来源：MCP get_code_snippet，headerinfotablewidget.cpp 26-125 行）：
 *   updateData:
 *   B1: wnd = DApplication::activeWindow() 为空 → cg = Inactive
 *   B2: for 循环逐行建 item（0 次 / 1 次 / N 次）
 *   B3: i % 2 == 0 → colorEven，否则 colorOdd（背景交替）
 *   B4: nRow > 0 → setFixedHeight(ROW_HEIGHT*nRow+2)，否则 setFixedHeight(0)
 *   paintEvent:
 *   B5: wnd 非空 → cg = Active
 *   用例映射：
 *   - updateData_EmptyList_ClearsOldRowsAndZeroHeight      → B2(0次)+B4(else)
 *   - updateData_SingleRow_ItemsFlagsAndHeightCorrect      → B2(1次)+B3(偶)+B4(then)
 *   - updateData_FourRows_TextsInOrderAndHeightScales      → B2(N次)
 *   - updateData_OddRowCount_BackgroundAlternatesEvenOdd   → B3(偶+奇)
 *   - updateData_RepeatedCall_StaleRowsCleared             → B2+B4
 *   - paintEvent_ActiveWindowStubbed_StatePreserved        → B5
 *   - paintEvent_NoActiveWindow_StatePreserved             → B1
 * 打桩策略（外部依赖全部 stub，不依赖真实窗口环境）：
 *   - QApplication::activeWindow（静态继承成员）→ static_cast 消歧后 set_lamda，
 *     控制 Active/Inactive 分支，不依赖窗口管理器状态；
 *   - DPaletteHelper/DFontSizeManager 为 DTK 进程内服务，offscreen 下可正常工作，
 *     背景色断言用同一 DPaletteHelper 取色镜像源码逻辑，主题无关。
 */

#include "headerinfotablewidget.h"
#include "MacroDefinition.h"
#include "ut_Head.h"
#include "stubext.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QPaintEvent>
#include <QHeaderView>
#include <QTableWidget>
#include <DPalette>
#include <DPaletteHelper>

class UT_HeaderInfoTableWidget : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new HeaderInfoTableWidget;
    }
    void TearDown() override
    {
        stub.clear();
        delete m_widget;
    }

    static QList<QPair<QString, QString>> makeData(int rows)
    {
        QList<QPair<QString, QString>> data;
        for (int i = 0; i < rows; ++i)
            data << qMakePair(QStringLiteral("Key%1").arg(i),
                              QStringLiteral("Value%1").arg(i));
        return data;
    }

    HeaderInfoTableWidget *m_widget = nullptr;
    stub_ext::StubExt stub;
};

// 构造函数 → initUI：表格按设计完成初始化（2 列、不可编辑、无网格、圆角透明等）
TEST_F(UT_HeaderInfoTableWidget, ctor_InitUIState_TableConfiguredAsDesigned)
{
    // Arrange：fixture 已构造控件，initUI 在构造函数内完成
    ASSERT_NE(nullptr, m_widget);

    // Act：读取 initUI 后的各项配置
    const int columnCount = m_widget->columnCount();
    const auto editTriggers = m_widget->editTriggers();
    const auto selectionMode = m_widget->selectionMode();
    const auto focusPolicy = m_widget->focusPolicy();
    const void *delegate = m_widget->itemDelegate();

    // Assert：逐项校验设计配置与委托挂接
    EXPECT_EQ(2, columnCount);
    EXPECT_EQ(QAbstractItemView::NoEditTriggers, editTriggers);
    EXPECT_EQ(QAbstractItemView::NoSelection, selectionMode);
    EXPECT_EQ(Qt::NoFocus, focusPolicy);
    EXPECT_EQ(QFrame::NoFrame, m_widget->frameStyle());
    EXPECT_EQ(false, m_widget->showGrid());
    EXPECT_EQ(true, m_widget->alternatingRowColors());
    EXPECT_EQ(false, m_widget->isSortingEnabled());
    EXPECT_EQ(false, m_widget->verticalHeader()->isVisible());
    EXPECT_EQ(false, m_widget->horizontalHeader()->isVisible());
    EXPECT_EQ(true, m_widget->horizontalHeader()->stretchLastSection());
    EXPECT_EQ(ROW_HEIGHT, m_widget->verticalHeader()->defaultSectionSize());
    EXPECT_EQ(180, m_widget->horizontalHeader()->defaultSectionSize());
    EXPECT_EQ(true, m_widget->testAttribute(Qt::WA_TranslucentBackground));
    EXPECT_NE(nullptr, delegate);
    EXPECT_EQ(delegate, reinterpret_cast<const void *>(m_widget->m_delegate));
}

// updateData 空数据：清空旧行（resetTableContents），高度归零（B2 0 次 + B4 else）
TEST_F(UT_HeaderInfoTableWidget, updateData_EmptyList_ClearsOldRowsAndZeroHeight)
{
    // Arrange：先填入 2 行，保证"清空"语义可观察
    m_widget->updateData(makeData(2));
    ASSERT_EQ(2, m_widget->rowCount());
    ASSERT_EQ(ROW_HEIGHT * 2 + 2, m_widget->height());

    // Act：传入空列表
    m_widget->updateData({});

    // Assert：行与单元格全部清空，高度为 0，列配置保持
    EXPECT_EQ(0, m_widget->rowCount());
    EXPECT_EQ(nullptr, m_widget->item(0, 0));
    EXPECT_EQ(0, m_widget->height());
    EXPECT_EQ(2, m_widget->columnCount());
}

// updateData 单行：文本按列摆放、item 不可选中、高度 = ROW_HEIGHT*1+2（B2 1 次 + B4 then）
TEST_F(UT_HeaderInfoTableWidget, updateData_SingleRow_ItemsFlagsAndHeightCorrect)
{
    // Arrange：一行两列数据
    QList<QPair<QString, QString>> data { qMakePair(QStringLiteral("Name"),
                                                   QStringLiteral("Intel CPU")) };

    // Act
    m_widget->updateData(data);

    // Assert：文本、不可选中标志、行高与高度
    EXPECT_EQ(1, m_widget->rowCount());
    ASSERT_NE(nullptr, m_widget->item(0, 0));
    ASSERT_NE(nullptr, m_widget->item(0, 1));
    EXPECT_EQ(QStringLiteral("Name"), m_widget->item(0, 0)->text());
    EXPECT_EQ(QStringLiteral("Intel CPU"), m_widget->item(0, 1)->text());
    EXPECT_EQ(0, m_widget->item(0, 0)->flags() & Qt::ItemIsSelectable);
    EXPECT_EQ(0, m_widget->item(0, 1)->flags() & Qt::ItemIsSelectable);
    EXPECT_EQ(ROW_HEIGHT * 1 + 2, m_widget->height());
}

// updateData 多行：4 行文本按序摆放、全部不可选中、高度随行数缩放（B2 N 次）
TEST_F(UT_HeaderInfoTableWidget, updateData_FourRows_TextsInOrderAndHeightScales)
{
    // Arrange：4 行数据
    const auto data = makeData(4);

    // Act
    m_widget->updateData(data);

    // Assert：逐格校验文本顺序与标志，高度 = ROW_HEIGHT*4+2
    EXPECT_EQ(4, m_widget->rowCount());
    for (int i = 0; i < 4; ++i) {
        ASSERT_NE(nullptr, m_widget->item(i, 0));
        ASSERT_NE(nullptr, m_widget->item(i, 1));
        EXPECT_EQ(QStringLiteral("Key%1").arg(i), m_widget->item(i, 0)->text());
        EXPECT_EQ(QStringLiteral("Value%1").arg(i), m_widget->item(i, 1)->text());
        EXPECT_EQ(0, m_widget->item(i, 0)->flags() & Qt::ItemIsSelectable);
    }
    EXPECT_EQ(ROW_HEIGHT * 4 + 2, m_widget->height());
}

// updateData 奇数行数：偶/奇行背景在 colorEven/colorOdd 间交替（B3）
TEST_F(UT_HeaderInfoTableWidget, updateData_OddRowCount_BackgroundAlternatesEvenOdd)
{
    // Arrange：3 行数据；窗口打桩为空 → 走 Inactive 分支（B1），取色可镜像
    stub.set_lamda(static_cast<QWidget *(*)()>(&QApplication::activeWindow),
                   []() -> QWidget * { return nullptr; });
    const auto palette = Dtk::Widget::DPaletteHelper::instance()->palette(m_widget);
    const QColor colorEven = palette.color(Dtk::Gui::DPalette::Inactive, Dtk::Gui::DPalette::Base);
    const QColor colorOdd = palette.color(Dtk::Gui::DPalette::Inactive, Dtk::Gui::DPalette::ItemBackground);
    ASSERT_NE(colorEven, colorOdd);

    // Act
    m_widget->updateData(makeData(3));

    // Assert：第 0/2 行（偶）背景同色，第 1 行（奇）为另一色
    const QBrush bg0 = m_widget->item(0, 0)->background();
    const QBrush bg1 = m_widget->item(1, 0)->background();
    const QBrush bg2 = m_widget->item(2, 0)->background();
    EXPECT_EQ(QBrush(colorEven), bg0);
    EXPECT_EQ(QBrush(colorOdd), bg1);
    EXPECT_EQ(QBrush(colorEven), bg2);
    EXPECT_NE(bg0, bg1);
}

// updateData 重复调用：3 行 → 1 行，旧行被清空不残留（B2 + B4 回归）
TEST_F(UT_HeaderInfoTableWidget, updateData_RepeatedCall_StaleRowsCleared)
{
    // Arrange：先填 3 行
    m_widget->updateData(makeData(3));
    ASSERT_EQ(3, m_widget->rowCount());

    // Act：再填 1 行
    m_widget->updateData(makeData(1));

    // Assert：行数收缩、越界旧单元格为空、高度按新行数计算
    EXPECT_EQ(1, m_widget->rowCount());
    EXPECT_EQ(nullptr, m_widget->item(1, 0));
    EXPECT_EQ(nullptr, m_widget->item(2, 1));
    EXPECT_EQ(ROW_HEIGHT * 1 + 2, m_widget->height());
}

// resetTableContents 直接调用：清空行与单元格，但保留列数与已设高度
TEST_F(UT_HeaderInfoTableWidget, resetTableContents_DirectCall_ClearsRowsKeepsColumnsAndHeight)
{
    // Arrange：填 2 行并记录当前高度（resetTableContents 不调整高度）
    m_widget->updateData(makeData(2));
    ASSERT_EQ(2, m_widget->rowCount());
    const int heightBefore = m_widget->height();

    // Act：直接调用私有方法（编译期 -fno-access-control）
    m_widget->resetTableContents();

    // Assert：行清空、单元格为空、列数与高度保持
    EXPECT_EQ(0, m_widget->rowCount());
    EXPECT_EQ(nullptr, m_widget->item(0, 0));
    EXPECT_EQ(2, m_widget->columnCount());
    EXPECT_EQ(heightBefore, m_widget->height());
}

// paintEvent 无激活窗口：Inactive 分支（B1）绘制边框，控件状态不受影响
TEST_F(UT_HeaderInfoTableWidget, paintEvent_NoActiveWindow_StatePreserved)
{
    // Arrange：填 2 行并快照状态；窗口打桩为空
    m_widget->updateData(makeData(2));
    const int heightBefore = m_widget->height();
    const int rowCountBefore = m_widget->rowCount();
    const QString cellBefore = m_widget->item(0, 0)->text();
    stub.set_lamda(static_cast<QWidget *(*)()>(&QApplication::activeWindow),
                   []() -> QWidget * { return nullptr; });
    QPaintEvent event(m_widget->viewport()->rect());

    // Act：直接触发 paintEvent（protected，-fno-access-control 下可调用）
    m_widget->paintEvent(&event);

    // Assert：绘制不改变数据与布局状态
    EXPECT_EQ(heightBefore, m_widget->height());
    EXPECT_EQ(rowCountBefore, m_widget->rowCount());
    EXPECT_EQ(cellBefore, m_widget->item(0, 0)->text());
    EXPECT_EQ(2, m_widget->columnCount());
}

// paintEvent 有激活窗口：Active 分支（B5）绘制边框，控件状态不受影响
TEST_F(UT_HeaderInfoTableWidget, paintEvent_ActiveWindowStubbed_StatePreserved)
{
    // Arrange：填 2 行并快照状态；窗口打桩为非空哑窗口
    m_widget->updateData(makeData(2));
    const int heightBefore = m_widget->height();
    const QString cellBefore = m_widget->item(1, 1)->text();
    stub.set_lamda(static_cast<QWidget *(*)()>(&QApplication::activeWindow),
                   []() -> QWidget * {
                       static QWidget s_dummy;
                       return &s_dummy;
                   });
    QPaintEvent event(m_widget->viewport()->rect());

    // Act
    m_widget->paintEvent(&event);

    // Assert：Active 分支绘制不改变数据与布局状态
    EXPECT_EQ(heightBefore, m_widget->height());
    EXPECT_EQ(cellBefore, m_widget->item(1, 1)->text());
    EXPECT_EQ(2, m_widget->rowCount());
    EXPECT_EQ(2, m_widget->columnCount());
}
