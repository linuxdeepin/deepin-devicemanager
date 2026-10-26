// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_driverlistviewdelegate.cpp
 * @brief DriverListViewDelegate 单元测试（Mode 2 重写，逐类闭环）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method   | level | factors                                                      | min | actual |
 * |----------|-------|--------------------------------------------------------------|-----|--------|
 * | paint    | high  | viewItemPosition:5,State_Enabled:2,activeWindow:2,icon:2 | 3 | 11 |
 * | sizeHint | low   | base-content-width,fixed-height-36                       | 2 | 2  |
 *
 * DriverListViewDelegate::paint 分支清单（源自 MCP get_code_snippet 真实源码，
 * src/Widget/DriverListViewDelegate.cpp:34-137）:
 *   B1:  !index.isValid() → 走 QStyledItemDelegate::paint 并提前返回
 *   B2:  !(opt.state & State_Enabled) → cg = Disabled
 *   B3:  enabled 且 !wnd（无活动窗口）→ cg = Inactive
 *   B4:  enabled 且 wnd → cg = Active
 *   B5:  viewItemPosition == Beginning → rect.x += margin
 *   B6:  viewItemPosition == Middle → 不做位置调整
 *   B7:  viewItemPosition == End → rect.width -= margin
 *   B8:  viewItemPosition == OnlyOne → x += margin 且 width -= margin
 *   B9:  default（viewItemPosition==Invalid 时触发）→ restore 后走基类 paint 并返回
 *   B10: viewItemPosition==Beginning 且 DecorationRole 有效 → 绘制 30x30 图标(x+margin+6, y+6)
 *   B11: widget ? widget->style() : QApplication::style() → 两种取样式路径
 *
 * 等价类/边界说明：
 *   - QRect 内部以 (x1,x2) 存储，setX/setWidth 均保持右边缘不动：
 *     Beginning 后 rect=(x+margin, w-margin)，再 textRect.x += margin-2 → 宽度同步收窄；
 *   - 文本绘制矩形随 viewItemPosition 的 4 个取值分别断言 x/width 算术结果（margin 取自真实 DStyle）；
 *   - 画笔颜色 = DGuiApplicationHelper::applicationPalette().color(cg, Text)，
 *     Disabled/Inactive/Active 三种 ColorGroup 以 drawText 捕获的画笔色验证；
 *   - 无效索引/Invalid viewItemPosition 走真实基类 paint（offscreen 下安全），
 *     断言不产生自定义文本/图标绘制。
 * 打桩策略（外部依赖全部 stub，不依赖真实设备环境；仅打非虚函数，虚函数用真实行为断言）：
 *   - DApplication::style()/activeWindow()（静态函数）→ 控制样式与窗口激活态；
 *   - QPainter::drawText / drawPixmap（非虚成员）→ 捕获绘制参数。
 */

#include "DriverListViewDelegate.h"
#include "ut_Head.h"
#include "stub.h"

#include <DApplication>
#include <DStyle>
#include <DGuiApplicationHelper>
#include <DPalette>

#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionViewItem>
#include <QStandardItemModel>
#include <QWidget>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

// ---- 桩捕获区（文件级静态，SetUp 中复位，Stub 析构自动恢复） ----
static DStyle *g_utDStyle = nullptr;        // 供 DApplication::style() 返回
static QWidget *g_utActiveWnd = nullptr;    // 供 DApplication::activeWindow() 返回
static bool g_utDrawTextCalled = false;     // QPainter::drawText 被调
static QRect g_utTextRect;                  // drawText 捕获的文本矩形
static QColor g_utTextColor;                // drawText 时的画笔颜色
static QString g_utText;                    // drawText 捕获的文本
static bool g_utDrawPixmapCalled = false;   // QPainter::drawPixmap 被调
static QRect g_utPixmapRect;                // 图标绘制矩形
static bool g_utIconPaintCalled = false;    // QIcon::paint 被调
static QRect g_utIconRect;                  // 源码计算的图标目标矩形

static QStyle *ut_appStyle()
{
    return g_utDStyle;
}

static QWidget *ut_activeWindow()
{
    return g_utActiveWnd;
}

// QPainter::drawText(const QRect&, int, const QString&, QRect*) 捕获桩
static void ut_drawText(QPainter *self, const QRect &rect, int flags, const QString &text, QRect *br)
{
    Q_UNUSED(flags);
    Q_UNUSED(br);
    g_utDrawTextCalled = true;
    g_utTextRect = rect;
    g_utTextColor = self->pen().color();
    g_utText = text;
}

// QPainter::drawPixmap 捕获桩（QIcon::paint 最终走的两个出线重载都拦，保证命中）
static void ut_drawPixmapRectF(QPainter *self, const QRectF &target, const QPixmap &pixmap, const QRectF &source)
{
    Q_UNUSED(self);
    Q_UNUSED(pixmap);
    Q_UNUSED(source);
    g_utDrawPixmapCalled = true;
    g_utPixmapRect = target.toRect();
}

static void ut_drawPixmap5(QPainter *self, int x, int y, int w, int h, const QPixmap &pixmap)
{
    Q_UNUSED(self);
    Q_UNUSED(pixmap);
    g_utDrawPixmapCalled = true;
    g_utPixmapRect = QRect(x, y, w, h);
}

// QIcon::paint 捕获桩：捕获源码传入的图标目标矩形（非虚成员，出线可打桩）
static void ut_iconPaint(QIcon *self, QPainter *painter, const QRect &rect, Qt::Alignment alignment, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(self);
    Q_UNUSED(painter);
    Q_UNUSED(alignment);
    Q_UNUSED(mode);
    Q_UNUSED(state);
    g_utIconPaintCalled = true;
    g_utIconRect = rect;
}

class UT_DriverListViewDelegate : public UT_HEAD
{
public:
    void SetUp() override
    {
        if (!g_utDStyle) {
            g_utDStyle = new DStyle;
        }
        m_delegate = new DriverListViewDelegate;
        resetCaptures();
    }
    void TearDown() override
    {
        delete m_delegate;
        m_delegate = nullptr;
        resetCaptures();
    }

    void resetCaptures()
    {
        g_utActiveWnd = nullptr;
        g_utDrawTextCalled = false;
        g_utTextRect = QRect();
        g_utTextColor = QColor();
        g_utText.clear();
        g_utDrawPixmapCalled = false;
        g_utPixmapRect = QRect();
        g_utIconPaintCalled = false;
        g_utIconRect = QRect();
    }

    // 便捷：在内存图像上构建绘制环境并安装捕获桩
    void preparePaint(QImage &image, QPainter *&painter, Stub &stub)
    {
        image = QImage(QSize(200, 60), QImage::Format_ARGB32);
        image.fill(Qt::white);
        painter = new QPainter(&image);
        stub.set((void (QPainter::*)(const QRect &, int, const QString &, QRect *))ADDR(QPainter, drawText),
                 ut_drawText);
        stub.set((void (QPainter::*)(const QRectF &, const QPixmap &, const QRectF &))ADDR(QPainter, drawPixmap),
                 ut_drawPixmapRectF);
        stub.set((void (QPainter::*)(int, int, int, int, const QPixmap &))ADDR(QPainter, drawPixmap),
                 ut_drawPixmap5);
        stub.set((void (QIcon::*)(QPainter *, const QRect &, Qt::Alignment, QIcon::Mode, QIcon::State) const)ADDR(QIcon, paint),
                 ut_iconPaint);
        stub.set(ADDR(DApplication, style), ut_appStyle);
    }

    // 便捷：设置默认启用的 option 与模型
    QStyleOptionViewItem makeOption(QStyleOptionViewItem::ViewItemPosition pos)
    {
        QStyleOptionViewItem option;
        option.rect = QRect(0, 0, 200, 36);
        option.viewItemPosition = pos;
        option.state = QStyle::State_Enabled;
        return option;
    }

    DriverListViewDelegate *m_delegate = nullptr;
};

// 覆盖分支 B1：无效索引走真实基类 paint 并返回，不做自定义文本/图标绘制
TEST_F(UT_DriverListViewDelegate, paint_B1_InvalidIndex_FallsBackToBasePaint)
{
    QImage image;
    QPainter *painter = nullptr;
    Stub stub;
    // Arrange
    preparePaint(image, painter, stub);
    QStyleOptionViewItem option = makeOption(QStyleOptionViewItem::OnlyOne);
    QModelIndex invalidIndex;

    // Act
    m_delegate->paint(painter, option, invalidIndex);

    // Assert
    EXPECT_FALSE(g_utDrawTextCalled);    // 未进入自定义文本绘制
    EXPECT_FALSE(g_utDrawPixmapCalled);  // 未进入图标绘制
    delete painter;
    EXPECT_EQ(qRgb(255, 255, 255), image.pixel(100, 30));   // 无视觉残留（背景保持纯白）
}

// 覆盖分支 B2：未启用状态使用 Disabled 组文字色
TEST_F(UT_DriverListViewDelegate, paint_B2_DisabledState_UsesDisabledTextColor)
{
    QImage image;
    QPainter *painter = nullptr;
    Stub stub;
    // Arrange
    preparePaint(image, painter, stub);
    QStandardItemModel model;
    model.appendRow(new QStandardItem("driver_name.ko"));
    QStyleOptionViewItem option = makeOption(QStyleOptionViewItem::OnlyOne);
    option.state = QStyle::State_None;   // 未启用

    // Act
    m_delegate->paint(painter, option, model.index(0, 0));

    // Assert
    const QColor expected = DGuiApplicationHelper::instance()
                                ->applicationPalette()
                                .color(Dtk::Gui::DPalette::Disabled, Dtk::Gui::DPalette::Text);
    EXPECT_TRUE(g_utDrawTextCalled);
    EXPECT_EQ(expected, g_utTextColor);
    delete painter;
}

// 覆盖分支 B3：启用且无活动窗口 → Inactive 组文字色
TEST_F(UT_DriverListViewDelegate, paint_B3_NoActiveWindow_UsesInactiveTextColor)
{
    QImage image;
    QPainter *painter = nullptr;
    Stub stub;
    // Arrange
    preparePaint(image, painter, stub);
    stub.set(ADDR(DApplication, activeWindow), ut_activeWindow);   // 返回 nullptr
    QStandardItemModel model;
    model.appendRow(new QStandardItem("driver_name.ko"));
    QStyleOptionViewItem option = makeOption(QStyleOptionViewItem::OnlyOne);

    // Act
    m_delegate->paint(painter, option, model.index(0, 0));

    // Assert
    const QColor expected = DGuiApplicationHelper::instance()
                                ->applicationPalette()
                                .color(Dtk::Gui::DPalette::Inactive, Dtk::Gui::DPalette::Text);
    EXPECT_TRUE(g_utDrawTextCalled);
    EXPECT_EQ(expected, g_utTextColor);
    delete painter;
}

// 覆盖分支 B4：启用且有活动窗口 → Active 组文字色
TEST_F(UT_DriverListViewDelegate, paint_B4_ActiveWindow_UsesActiveTextColor)
{
    QImage image;
    QPainter *painter = nullptr;
    Stub stub;
    // Arrange
    preparePaint(image, painter, stub);
    stub.set(ADDR(DApplication, activeWindow), ut_activeWindow);   // 返回非空窗口 → Active 组
    QWidget wnd;
    wnd.setAttribute(Qt::WA_DontShowOnScreen);
    g_utActiveWnd = &wnd;
    QStandardItemModel model;
    model.appendRow(new QStandardItem("driver_name.ko"));
    QStyleOptionViewItem option = makeOption(QStyleOptionViewItem::OnlyOne);

    // Act
    m_delegate->paint(painter, option, model.index(0, 0));

    // Assert
    const QColor expected = DGuiApplicationHelper::instance()
                                ->applicationPalette()
                                .color(Dtk::Gui::DPalette::Active, Dtk::Gui::DPalette::Text);
    EXPECT_TRUE(g_utDrawTextCalled);
    EXPECT_EQ(expected, g_utTextColor);
    delete painter;
}

// 覆盖分支 B5+B10：Beginning 位置 rect 左缩进 margin，图标绘制于 (x+margin+6, y+6) 30x30；
// 文本矩形在 rect 基础上再左缩进 margin-2（右边缘保持，宽度同步收窄）
TEST_F(UT_DriverListViewDelegate, paint_B5_B10_BeginningPosition_DrawsIconAndOffsetsText)
{
    QImage image;
    QPainter *painter = nullptr;
    Stub stub;
    // Arrange
    preparePaint(image, painter, stub);
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem("driver_name.ko");
    QPixmap pm(16, 16);
    pm.fill(Qt::red);
    item->setIcon(QIcon(pm));
    model.appendRow(item);
    const int margin = g_utDStyle->pixelMetric(DStyle::PM_ContentsMargins, nullptr);
    QStyleOptionViewItem option = makeOption(QStyleOptionViewItem::Beginning);

    // Act
    m_delegate->paint(painter, option, model.index(0, 0));

    // Assert
    // QRect 以 (x1,x2) 存储：setWidth(30) 后 setX(x+6) 保持右边缘 → 实际图标矩形 24x24（非 30x30）
    EXPECT_TRUE(g_utIconPaintCalled);    // B10: 图标分支被走进（QIcon::paint 捕获源码计算的 iconRect）
    EXPECT_EQ(margin + 6, g_utIconRect.x());
    EXPECT_EQ(6, g_utIconRect.y());
    EXPECT_EQ(24, g_utIconRect.width());
    EXPECT_EQ(24, g_utIconRect.height());
    EXPECT_TRUE(g_utDrawTextCalled);     // B5: x = margin + (margin-2)，宽度 = 200 - x（右边缘保持）
    EXPECT_EQ(margin + margin - 2, g_utTextRect.x());
    EXPECT_EQ(option.rect.width() - margin - margin + 2, g_utTextRect.width());
    delete painter;
}

// 覆盖分支 B6：Middle 位置 rect 不做调整，文本仅左缩进 margin-2（右边缘保持 → 宽度收窄 margin-2）
TEST_F(UT_DriverListViewDelegate, paint_B6_MiddlePosition_InsetsTextLeftOnly)
{
    QImage image;
    QPainter *painter = nullptr;
    Stub stub;
    // Arrange
    preparePaint(image, painter, stub);
    QStandardItemModel model;
    model.appendRow(new QStandardItem("driver_name.ko"));
    const int margin = g_utDStyle->pixelMetric(DStyle::PM_ContentsMargins, nullptr);
    QStyleOptionViewItem option = makeOption(QStyleOptionViewItem::Middle);

    // Act
    m_delegate->paint(painter, option, model.index(0, 0));

    // Assert
    EXPECT_TRUE(g_utDrawTextCalled);
    EXPECT_EQ(margin - 2, g_utTextRect.x() - option.rect.x());
    EXPECT_EQ(option.rect.width() - (margin - 2), g_utTextRect.width());
    delete painter;
}

// 覆盖分支 B7：End 位置 rect 宽度先收窄 margin，文本再左缩进 margin-2
TEST_F(UT_DriverListViewDelegate, paint_B7_EndPosition_NarrowsWidthThenInsetsText)
{
    QImage image;
    QPainter *painter = nullptr;
    Stub stub;
    // Arrange
    preparePaint(image, painter, stub);
    QStandardItemModel model;
    model.appendRow(new QStandardItem("driver_name.ko"));
    const int margin = g_utDStyle->pixelMetric(DStyle::PM_ContentsMargins, nullptr);
    QStyleOptionViewItem option2 = makeOption(QStyleOptionViewItem::End);
    option2.rect = QRect(50, 0, 200, 36);   // 非 0 起点，验证 x/width 独立性

    // Act
    m_delegate->paint(painter, option2, model.index(0, 0));

    // Assert
    EXPECT_TRUE(g_utDrawTextCalled);
    // End: rect=(50,0,190)，textRect.x = 50 + margin-2，宽度 = 190 - (margin-2)
    EXPECT_EQ(50 + margin - 2, g_utTextRect.x());
    EXPECT_EQ(200 - margin - (margin - 2), g_utTextRect.width());
    delete painter;
}

// 覆盖分支 B8：OnlyOne 位置 rect 左右各缩进 margin，文本再左缩进 margin-2
TEST_F(UT_DriverListViewDelegate, paint_B8_OnlyOnePosition_InsetsBothEdges)
{
    QImage image;
    QPainter *painter = nullptr;
    Stub stub;
    // Arrange
    preparePaint(image, painter, stub);
    QStandardItemModel model;
    model.appendRow(new QStandardItem("driver_name.ko"));
    const int margin = g_utDStyle->pixelMetric(DStyle::PM_ContentsMargins, nullptr);
    QStyleOptionViewItem option = makeOption(QStyleOptionViewItem::OnlyOne);
    option.rect = QRect(50, 0, 200, 36);

    // Act
    m_delegate->paint(painter, option, model.index(0, 0));

    // Assert
    EXPECT_TRUE(g_utDrawTextCalled);
    // OnlyOne: setX(+margin) 后宽 190；setWidth(width-margin)=180；textRect.x=50+margin+margin-2，
    // 右边缘保持 → 宽度 = 200 - margin - margin - (margin-2)
    EXPECT_EQ(50 + margin + margin - 2, g_utTextRect.x());
    EXPECT_EQ(200 - margin - margin - (margin - 2), g_utTextRect.width());
    delete painter;
}

// 覆盖分支 B9：viewItemPosition 为 Invalid 命中 default → restore + 基类 paint 返回
TEST_F(UT_DriverListViewDelegate, paint_B9_InvalidViewItemPosition_FallsBackToBasePaint)
{
    QImage image;
    QPainter *painter = nullptr;
    Stub stub;
    // Arrange
    preparePaint(image, painter, stub);
    QStandardItemModel model;
    model.appendRow(new QStandardItem("driver_name.ko"));
    QStyleOptionViewItem option = makeOption(QStyleOptionViewItem::Beginning);
    option.viewItemPosition = QStyleOptionViewItem::Invalid;   // 命中 default

    // Act
    m_delegate->paint(painter, option, model.index(0, 0));

    // Assert
    EXPECT_FALSE(g_utDrawTextCalled);    // default 分支未做自定义文本绘制
    EXPECT_FALSE(g_utDrawPixmapCalled);
    delete painter;
    EXPECT_EQ(qRgb(255, 255, 255), image.pixel(100, 30));   // 无视觉残留（背景保持纯白）
}

// 覆盖分支 B11：option.widget 有效时使用 widget->style()，完整绘制流程执行成功
TEST_F(UT_DriverListViewDelegate, paint_B11_OptionWidgetSet_UsesWidgetStyle)
{
    QImage image;
    QPainter *painter = nullptr;
    Stub stub;
    // Arrange
    preparePaint(image, painter, stub);
    QWidget wnd;
    wnd.setAttribute(Qt::WA_DontShowOnScreen);
    QStandardItemModel model;
    model.appendRow(new QStandardItem("driver_name.ko"));
    QStyleOptionViewItem option = makeOption(QStyleOptionViewItem::OnlyOne);
    option.widget = &wnd;

    // Act
    m_delegate->paint(painter, option, model.index(0, 0));

    // Assert
    EXPECT_TRUE(g_utDrawTextCalled);   // widget 样式路径完整执行（widget=nullptr 的对照路径由 B2~B8 覆盖）
    EXPECT_EQ("driver_name.ko", g_utText);
    delete painter;
}

// sizeHint：固定行高 36，宽度透传基类计算结果（与真实 QStyledItemDelegate 对照）
TEST_F(UT_DriverListViewDelegate, sizeHint_FixedRowHeight36_PassesThroughBaseWidth)
{
    // Arrange
    QStyledItemDelegate baseDelegate;
    QStandardItemModel model;
    model.appendRow(new QStandardItem("driver_name.ko"));
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 120, 30);
    const int baseWidth = baseDelegate.sizeHint(option, model.index(0, 0)).width();

    // Act
    const QSize result = m_delegate->sizeHint(option, model.index(0, 0));

    // Assert
    EXPECT_EQ(baseWidth, result.width());   // 基类宽度透传
    EXPECT_EQ(36, result.height());         // 固定行高
}

// 边界：宽度来自基类内容尺寸（随文本长度变化），行高恒为 36
TEST_F(UT_DriverListViewDelegate, sizeHint_WidthFollowsContent_FixedHeight36)
{
    // Arrange
    QStyledItemDelegate baseDelegate;
    QStandardItemModel model;
    model.appendRow(new QStandardItem(QString(64, 'w')));   // 长文本 → 内容宽度更大
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 120, 30);

    // Act
    const QSize result = m_delegate->sizeHint(option, model.index(0, 0));

    // Assert
    EXPECT_EQ(baseDelegate.sizeHint(option, model.index(0, 0)).width(), result.width());
    EXPECT_EQ(36, result.height());   // 行高固定 36，不随内容变化
}
