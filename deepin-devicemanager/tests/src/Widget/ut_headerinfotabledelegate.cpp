// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 HeaderInfoDelegate::paint 的活动/非活动调色板与选中/普通四种组合。
//
// 用例计数声明表：
//
// | method | level | factors | min | actual |
// | --- | --- | --- | --- | --- |
// | HeaderInfoDelegate::paint | low | B1(活动窗口),B2(State_Selected) | 4 | 4 |
//
// 通过 stub DPaletteHelper::palette 注入可控调色板、stub DApplication::activeWindow
// 控制活动窗口、stub QStyledItemDelegate::paint 捕获委托最终使用的调色板颜色，
// 不依赖真实窗口焦点与桌面主题。QApplication 由 tests/src/test_main.cpp
// 提供（offscreen 全局实例），此处不再重建。

#include "headerinfotableDelegate.h"

#include <stubext.h>

#include <DApplication>
#include <DPalette>
#include <DPaletteHelper>

#include <QStandardItemModel>
#include <QPainter>
#include <QImage>
#include <QStyleOptionViewItem>

#include <gtest/gtest.h>

#include <dlfcn.h>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

// ---- 方法分支清单（源自 MCP get_code_snippet HeaderInfoDelegate::paint 18-38 行）----
// B1: wnd（DApplication::activeWindow() 非空）→ cg=Active；否则 Inactive
//     用例1/3 走 true；用例2/4 走 false
// B2: opt.state & QStyle::State_Selected → 使用 HighlightedText 色；否则 Text 色
//     用例1/2 走 true；用例3/4 走 false
// 四种组合（Active/Inactive × Selected/Normal）在用例1-4 全覆盖。

// 基类绘制桩（静态函数版，捕获委托最终使用的调色板颜色）。
// 注意：QStyledItemDelegate::paint 为 Qt 虚函数，成员函数指针在 Itanium ABI 下
// 携带 vtable 偏移编码而非代码地址，stub 的 addrof 无法直接使用；因此这里用
// dlsym 取动态符号表中的真实实现地址打桩。
static int g_base_paint_count = 0;
static QColor g_captured_text;
static QColor g_captured_window_text;

static void *ut_base_paint_impl_addr()
{
    return dlsym(RTLD_DEFAULT, "_ZNK19QStyledItemDelegate5paintEP8QPainterRK20QStyleOptionViewItemRK11QModelIndex");
}

static void ut_base_paint_stub(const QStyledItemDelegate *self, QPainter *painter,
                               const QStyleOptionViewItem &option, const QModelIndex &index)
{
    ++g_base_paint_count;
    g_captured_text = option.palette.color(QPalette::Text);
    g_captured_window_text = option.palette.color(QPalette::WindowText);
}

namespace {

class HeaderInfoDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        active_wnd = new QWidget;
        crafted_palette = DPalette();
        crafted_palette.setColor(DPalette::Active, DPalette::Text, QColor(Qt::blue));
        crafted_palette.setColor(DPalette::Inactive, DPalette::Text, QColor(Qt::darkBlue));
        crafted_palette.setColor(DPalette::Active, DPalette::HighlightedText, QColor(Qt::red));
        crafted_palette.setColor(DPalette::Inactive, DPalette::HighlightedText, QColor(Qt::darkRed));
        // stub 调色板获取：返回固定可预期的 DPalette
        stub.set_lamda(static_cast<DPalette (DPaletteHelper::*)(const QWidget *, const QPalette &) const>(&DPaletteHelper::palette),
                       [this](const DPaletteHelper *self, const QWidget *widget, const QPalette &base) -> DPalette {
                           __DBG_STUB_INVOKE__
                           return crafted_palette;
                       });
        activateWindow(true);
        // stub 基类绘制：捕获委托传入的最终调色板颜色，不执行真实绘制
        stub.set(ut_base_paint_impl_addr(), ut_base_paint_stub);
        g_base_paint_count = 0;
        g_captured_text = QColor();
        g_captured_window_text = QColor();
        model = new QStandardItemModel;
        model->setItem(0, 0, new QStandardItem(QStringLiteral("test")));
        // Act: 构造被测对象
        delegate = new HeaderInfoDelegate(active_wnd);
        image = QImage(80, 24, QImage::Format_ARGB32);
        image.fill(Qt::white);
        painter = new QPainter(&image);
    }

    void TearDown() override
    {
        delete painter;
        delete delegate;
        delete model;
        delete active_wnd;
        active_wnd = nullptr;
        stub.clear();
    }

    // 注入/恢复活动窗口桩
    void activateWindow(bool active)
    {
        if (active) {
            QWidget *wnd = active_wnd;
            stub.set_lamda(static_cast<QWidget *(*)()>(&DApplication::activeWindow),
                           [wnd]() -> QWidget * {
                               __DBG_STUB_INVOKE__
                               return wnd;
                           });
        } else {
            stub.set_lamda(static_cast<QWidget *(*)()>(&DApplication::activeWindow),
                           []() -> QWidget * {
                               __DBG_STUB_INVOKE__
                               return nullptr;
                           });
        }
    }

    // 以给定状态触发一次 paint
    void paintOnce(QStyle::StateFlag extraState)
    {
        QStyleOptionViewItem option;
        option.state |= extraState;
        option.widget = active_wnd;
        const QModelIndex index = model->index(0, 0);
        delegate->paint(painter, option, index);
    }

    stub_ext::StubExt stub;
    QStandardItemModel *model = nullptr;
    HeaderInfoDelegate *delegate = nullptr;
    QWidget *active_wnd = nullptr;
    QPainter *painter = nullptr;
    QImage image;
    DPalette crafted_palette;
};

}  // namespace

// ---- paint: 选中 + 有活动窗口（B1=true, B2=true） ---------------------------
TEST_F(HeaderInfoDelegateTest, Paint_SelectedWithActiveWindow_UsesActiveHighlightColor)
{
    // Arrange: activeWindow 桩已在 SetUp 注入，paint 桩计数清零
    const QStyle::StateFlag state = QStyle::State_Selected;

    // Act
    paintOnce(state);

    // Assert: 使用活动组 HighlightedText 色，且委托基类被调用一次
    EXPECT_EQ(g_captured_text, QColor(Qt::red));
    EXPECT_EQ(g_captured_window_text, QColor(Qt::red));
    EXPECT_EQ(g_base_paint_count, 1);
}

// ---- paint: 选中 + 无活动窗口（B1=false, B2=true） --------------------------
TEST_F(HeaderInfoDelegateTest, Paint_SelectedWithoutActiveWindow_UsesInactiveHighlightColor)
{
    // Arrange
    activateWindow(false);

    // Act
    paintOnce(QStyle::State_Selected);

    // Assert: 使用非活动组 HighlightedText 色
    EXPECT_EQ(g_captured_text, QColor(Qt::darkRed));
    EXPECT_EQ(g_captured_window_text, QColor(Qt::darkRed));
    EXPECT_EQ(g_base_paint_count, 1);
}

// ---- paint: 普通 + 有活动窗口（B1=true, B2=false） --------------------------
TEST_F(HeaderInfoDelegateTest, Paint_NormalWithActiveWindow_UsesActiveTextColor)
{
    // Arrange
    activateWindow(true);

    // Act
    paintOnce(QStyle::State_None);

    // Assert: 使用活动组 Text 色
    EXPECT_EQ(g_captured_text, QColor(Qt::blue));
    EXPECT_EQ(g_captured_window_text, QColor(Qt::blue));
    EXPECT_EQ(g_base_paint_count, 1);
}

// ---- paint: 普通 + 无活动窗口（B1=false, B2=false） -------------------------
TEST_F(HeaderInfoDelegateTest, Paint_NormalWithoutActiveWindow_UsesInactiveTextColor)
{
    // Arrange
    activateWindow(false);

    // Act
    paintOnce(QStyle::State_None);

    // Assert: 使用非活动组 Text 色
    EXPECT_EQ(g_captured_text, QColor(Qt::darkBlue));
    EXPECT_EQ(g_captured_window_text, QColor(Qt::darkBlue));
    EXPECT_EQ(g_base_paint_count, 1);
}
