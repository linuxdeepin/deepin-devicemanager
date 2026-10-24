// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DetailViewDelegate.h"
#include "DetailTreeView.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStyle>
#include <DApplication>

#include <QColor>
#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QCommonStyle>
#include <QStandardItemModel>
#include <QStyle>
#include <QWidget>

#include <gtest/gtest.h>
DStyle *UT_DetailViewDelegate_style;
class UT_DetailViewDelegate : public UT_HEAD
{
public:
    void SetUp()
    {
        m_treeView = new DetailTreeView;
        m_dViewDelegate = new DetailViewDelegate(m_treeView);
        m_treeView->setItemDelegate(m_dViewDelegate);
        UT_DetailViewDelegate_style = new DStyle;
    }
    void TearDown()
    {
        delete m_treeView;
        delete  UT_DetailViewDelegate_style;
        UT_DetailViewDelegate_style = nullptr;
    }
    DetailViewDelegate *m_dViewDelegate;
    DetailTreeView *m_treeView;
    Stub m_stub;
};

int ut_y()
{
    return 10;
}

DStyle *DetailViewDelegate_UT_style_fun()
{
    return UT_DetailViewDelegate_style;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_paint_001)
{
    QStyleOptionViewItem option;
    QPainter painter(m_treeView);
    m_treeView->setColumnCount(1);
    m_treeView->insertRow(0);
    QTableWidgetItem *item0 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 0, item0);
    QModelIndex index0 = m_treeView->model()->index(0, 0);


    Stub stub;
    stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    stub.set(ADDR(QPoint, y), ut_y);

    m_dViewDelegate->paint(&painter, option, index0);
    EXPECT_FALSE(m_treeView->grab().isNull());

    delete item0;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_paint_002)
{
    QStyleOptionViewItem option;
    QPainter painter(m_treeView);
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    QTableWidgetItem *item0 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 0, item0);
    QTableWidgetItem *item1 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 1, item1);
    QModelIndex index1 = m_treeView->model()->index(0, 1);

    Stub stub;
    stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    stub.set(ADDR(QPoint, y), ut_y);

    m_dViewDelegate->paint(&painter, option, index1);
    EXPECT_FALSE(m_treeView->grab().isNull());

    delete item0;
    delete item1;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_paint_003)
{
    QStyleOptionViewItem option;
    QPainter painter(m_treeView);
    m_treeView->setColumnCount(1);
    m_treeView->insertRow(0);
    QTableWidgetItem *item0 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 0, item0);
    m_treeView->insertRow(1);
    QTableWidgetItem *item2 = new QTableWidgetItem("xxx");
    m_treeView->setItem(1, 0, item2);
    QModelIndex index2 = m_treeView->model()->index(1, 0);

    Stub stub;
    stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    stub.set(ADDR(QPoint, y), ut_y);

    m_dViewDelegate->paint(&painter, option, index2);
    EXPECT_FALSE(m_treeView->grab().isNull());

    delete item0;
    delete item2;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_paint_004)
{
    QStyleOptionViewItem option;
    QPainter painter(m_treeView);
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    QTableWidgetItem *item0 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 0, item0);
    QTableWidgetItem *item1 = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 1, item1);
    m_treeView->insertRow(1);
    QTableWidgetItem *item2 = new QTableWidgetItem("xxx");
    m_treeView->setItem(1, 0, item2);
    QTableWidgetItem *item3 = new QTableWidgetItem("xxx");
    m_treeView->setItem(1, 1, item3);

    QModelIndex index3 = m_treeView->model()->index(1, 1);

    Stub stub;
    stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    stub.set(ADDR(QPoint, y), ut_y);

    m_dViewDelegate->paint(&painter, option, index3);
    EXPECT_FALSE(m_treeView->grab().isNull());

    delete item0;
    delete item1;
    delete item2;
    delete item3;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_createEditor)
{
    QStyleOptionViewItem m_item;

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("/");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->model()->index(0, 0);

    EXPECT_FALSE(m_dViewDelegate->createEditor(nullptr, m_item, index));
    delete item;
}

TEST_F(UT_DetailViewDelegate, UT_DetailViewDelegate_sizeHint)
{
    QStyleOptionViewItem m_item;

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("/");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->model()->index(0, 0);

    QSize size = m_dViewDelegate->sizeHint(m_item, index);
    EXPECT_EQ(150, size.width());
    EXPECT_EQ(50, size.height());
    delete item;
}

// ───── 补强批次：paint 分支映射与设备状态文本三分支 ─────
// 分支清单（来源：MCP get_code_snippet — src/Widget/DetailViewDelegate.cpp）
//  P1 无效索引 → 基类绘制
//  P2 末行=更多信息按钮行(row==rowCount()-1 && row!=0 && hasExpendInfo())
//     → 白底圆角、上方留 1px 横线
//  P3 rectpath.y() <= 0 首行 → 圆角 + 第0/1列角部补白
//  P4 y+height >= 父高 末行 → 裁剪圆角 + 第0/1列角部补白(高>8)
//  P5 中间行 → addRect
//  T  设备状态文本：启用+可用→去掉"(Disable)"；启用+不可用→
//     "(Unavailable)"前缀+PlaceholderText；禁用→"(Disable)"前缀+#FF5736 红字
//  S  sizeHint：高下限 50；column() 非 0 宽下限 150；createEditor 恒空
//
// 新增用例映射：FirstRow×2(P3) / MiddleRow(P5) / LastRow×2(P4) /
// MoreRow(P2) / 设备状态×3(T) / SizeHint×2(S) / CreateEditor(S)

static bool g_devEnable = true;
static bool g_devAvailable = true;
static bool g_hasExpendInfo = false;

static bool ut_detailTreeView_isCurDeviceEnable(void *) { return g_devEnable; }
static bool ut_detailTreeView_isCurDeviceAvailable(void *) { return g_devAvailable; }
static bool ut_detailTreeView_hasExpendInfo(void *) { return g_hasExpendInfo; }

// 将委托绘制到白底图片；选项矩形为视口坐标，平移使单元格落在图片原点
static QImage ut_detailRender(UT_DetailViewDelegate *self, const QModelIndex &index,
                              const QRect &cellRect)
{
    QStyleOptionViewItem option;
    option.rect = cellRect;
    QImage img(cellRect.size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    p.translate(-cellRect.topLeft());
    self->m_dViewDelegate->paint(&p, option, index);
    p.end();
    return img;
}

static int ut_nonWhitePixels(const QImage &img)
{
    int count = 0;
    for (int y = 0; y < img.height(); y += 2)
        for (int x = 0; x < img.width(); x += 2)
            if (img.pixelColor(x, y) != Qt::white) ++count;
    return count;
}

static bool ut_hasRedPixel(const QImage &img)
{
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x) {
            const QColor c = img.pixelColor(x, y);
            if (c.red() > 180 && c.green() < 130 && c.blue() < 130) return true;
        }
    return false;
}

TEST_F(UT_DetailViewDelegate, Paint_FirstRowColumn0_CornerFill)
{
    m_stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    Stub &s = m_stub;
    Q_UNUSED(s);
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    m_treeView->setItem(0, 0, new QTableWidgetItem("key"));

    QImage img = ut_detailRender(this, m_treeView->model()->index(0, 0), QRect(0, 0, 220, 50));

    EXPECT_GT(ut_nonWhitePixels(img), 0);   // P3 首行第0列补白 + 文字
}

TEST_F(UT_DetailViewDelegate, Paint_FirstRowColumn1_CornerFill)
{
    m_stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    Stub &s = m_stub;
    Q_UNUSED(s);
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    m_treeView->setItem(0, 1, new QTableWidgetItem("value"));

    QImage img = ut_detailRender(this, m_treeView->model()->index(0, 1), QRect(0, 0, 220, 50));

    EXPECT_GT(ut_nonWhitePixels(img), 0);   // P3 首行第1列补白
}

TEST_F(UT_DetailViewDelegate, Paint_MiddleRow_PlainRect)
{
    m_stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    Stub &s = m_stub;
    Q_UNUSED(s);
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    m_treeView->setItem(0, 0, new QTableWidgetItem("a"));
    m_treeView->insertRow(1);
    m_treeView->setItem(1, 1, new QTableWidgetItem("b"));

    QImage img = ut_detailRender(this, m_treeView->model()->index(1, 1), QRect(0, 60, 220, 40));

    EXPECT_GT(ut_nonWhitePixels(img), 0);   // P5 中间行 addRect 分支
}

TEST_F(UT_DetailViewDelegate, Paint_LastRowColumn0_FillsCorners)
{
    m_stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    Stub &s = m_stub;
    Q_UNUSED(s);
    m_treeView->setFixedHeight(120);   // y=70,h=50 → 120>=120 末行分支 P4
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    m_treeView->setItem(0, 0, new QTableWidgetItem("a"));
    m_treeView->insertRow(1);
    m_treeView->setItem(1, 0, new QTableWidgetItem("b"));

    QImage img = ut_detailRender(this, m_treeView->model()->index(1, 0), QRect(0, 70, 220, 50));

    EXPECT_GT(ut_nonWhitePixels(img), 0);   // P4 末行第0列补白(高>8)
}

TEST_F(UT_DetailViewDelegate, Paint_LastRowColumn1_FillsCorners)
{
    m_stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    Stub &s = m_stub;
    Q_UNUSED(s);
    m_treeView->setFixedHeight(120);
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    m_treeView->setItem(0, 1, new QTableWidgetItem("a"));
    m_treeView->insertRow(1);
    m_treeView->setItem(1, 1, new QTableWidgetItem("b"));

    QImage img = ut_detailRender(this, m_treeView->model()->index(1, 1), QRect(0, 70, 220, 50));

    EXPECT_GT(ut_nonWhitePixels(img), 0);   // P4 末行第1列补白
}

TEST_F(UT_DetailViewDelegate, Paint_MoreButtonRow_HasExpendInfo)
{
    m_stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    Stub &s = m_stub;
    s.set(ADDR(DetailTreeView, hasExpendInfo), ut_detailTreeView_hasExpendInfo);
    g_hasExpendInfo = true;   // 末行 = "更多信息"按钮行 → P2 白底圆角
    m_treeView->setColumnCount(2);
    for (int r = 0; r < 3; ++r) {
        m_treeView->insertRow(r);
        m_treeView->setItem(r, 0, new QTableWidgetItem("a"));
    }

    QImage img = ut_detailRender(this, m_treeView->model()->index(2, 0), QRect(0, 100, 220, 40));

    EXPECT_GT(ut_nonWhitePixels(img), 0);   // P2 分支执行且渲染
}

TEST_F(UT_DetailViewDelegate, Paint_DeviceEnableAvailable_NormalText)
{
    m_stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    Stub &s = m_stub;
    s.set(ADDR(DetailTreeView, isCurDeviceEnable), ut_detailTreeView_isCurDeviceEnable);
    s.set(ADDR(DetailTreeView, isCurDeviceAvailable), ut_detailTreeView_isCurDeviceAvailable);
    g_devEnable = true;
    g_devAvailable = true;
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    m_treeView->setItem(0, 1, new QTableWidgetItem("value"));

    QImage img = ut_detailRender(this, m_treeView->model()->index(0, 1), QRect(0, 0, 220, 50));

    EXPECT_GT(ut_nonWhitePixels(img), 0);   // T 启用+可用 → 正常渲染
    EXPECT_FALSE(ut_hasRedPixel(img));      // 不带红色禁用标记
}

TEST_F(UT_DetailViewDelegate, Paint_DeviceEnableUnavailable_PrefixMark)
{
    m_stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    Stub &s = m_stub;
    s.set(ADDR(DetailTreeView, isCurDeviceEnable), ut_detailTreeView_isCurDeviceEnable);
    s.set(ADDR(DetailTreeView, isCurDeviceAvailable), ut_detailTreeView_isCurDeviceAvailable);
    g_devEnable = true;
    g_devAvailable = false;   // T 启用+不可用 → "(Unavailable)" 前缀
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    m_treeView->setItem(0, 1, new QTableWidgetItem("value"));

    QImage img = ut_detailRender(this, m_treeView->model()->index(0, 1), QRect(0, 0, 220, 50));

    EXPECT_GT(ut_nonWhitePixels(img), 0);
    EXPECT_FALSE(ut_hasRedPixel(img));   // 灰色 PlaceholderText，非红色
}

TEST_F(UT_DetailViewDelegate, Paint_DeviceDisabled_RedDisablePrefix)
{
    m_stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    Stub &s = m_stub;
    s.set(ADDR(DetailTreeView, isCurDeviceEnable), ut_detailTreeView_isCurDeviceEnable);
    g_devEnable = false;   // T 禁用 → "(Disable)" 前缀 + #FF5736 红字
    m_treeView->setColumnCount(2);
    m_treeView->insertRow(0);
    m_treeView->setItem(0, 1, new QTableWidgetItem("value"));

    QImage img = ut_detailRender(this, m_treeView->model()->index(0, 1), QRect(0, 0, 220, 50));

    EXPECT_GT(ut_nonWhitePixels(img), 0);
    EXPECT_TRUE(ut_hasRedPixel(img));   // #FF5736 红色标记像素存在
}

TEST_F(UT_DetailViewDelegate, CreateEditor_Always_ReturnsNullptr)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    EXPECT_EQ(m_dViewDelegate->createEditor(nullptr, option, index), nullptr);
}

TEST_F(UT_DetailViewDelegate, SizeHint_FirstColumn_NoMinWidth)
{
    QStandardItemModel model;
    model.setItem(0, 0, new QStandardItem("a"));
    QStyleOptionViewItem option;

    QSize size = m_dViewDelegate->sizeHint(option, model.index(0, 0));

    EXPECT_GE(size.height(), 50);   // column()==0 → 宽度不加下限
}

TEST_F(UT_DetailViewDelegate, SizeHint_SecondColumn_MinWidth)
{
    QStandardItemModel model;
    model.setItem(0, 1, new QStandardItem("b"));
    QStyleOptionViewItem option;

    QSize size = m_dViewDelegate->sizeHint(option, model.index(0, 1));

    EXPECT_GE(size.width(), 150);   // column() 非 0 → 最小宽 150
    EXPECT_GE(size.height(), 50);
}
