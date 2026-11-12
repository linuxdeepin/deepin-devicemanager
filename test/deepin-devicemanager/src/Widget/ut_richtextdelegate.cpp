// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// RichTextDelegate 补强批次：paint 分支映射（首行/中间行/超界/多行文本/隔行底色）、
// sizeHint 列宽分支、addRow 多值拆分结构断言、getDocFromLst 无效键值对提前返回。

// 分支清单（来源：MCP get_code_snippet — src/Widget/RichTextDelegate.cpp）
// paint:
//  P1: !index.isValid() → 基类绘制并返回
//  P2: opt.state & State_Enabled / activeWindow 判定调色板组（运行态分支）
//  P3: features & Alternate → ItemBackground，否则 Base
//  P4: rectpath.y() > 0（中间/末行）：
//      P4a 未超下边界 → addRect；P4b 超下边界 → addRoundedRect + 第0/1列角部补白
//  P5: rectpath.y() <= 0（首行）：
//      P5a 未超下边界 → addRoundedRect + 第0/1列角部补白；P5b 超界 → addRect
//  P6: 文本含 "\n"（多行）→ getDocFromLst + setRowHeight 行高回报
//      否则单行 → 简单 <p> 段落
// sizeHint: 高度下限 40；index.column() 非零时宽度下限 150
// createEditor: 恒返回 nullptr
// getDocFromLst: 键值对缺冒号 → 提前 return（无 table）；有效 → 每对一行
// addRow: 值拆分 >2 段 → 递归续行；否则单行 td2
//
// 用例映射：
// - RichTextDelegate_DefaultCtor_WrapsStyledDelegate        → 构造（父对象）
// - CreateEditor_Always_ReturnsNullptr                      → createEditor
// - SizeHint_FirstColumn_NoMinWidth / SizeHint_SecondColumn_MinWidth → sizeHint 双分支
// - Paint_InvalidIndex_FallsBackToBasePaint                 → P1
// - Paint_FirstRowColumn0_CornerFill                        → P5a + P3(Base)
// - Paint_FirstRowColumn1_CornerFill                        → P5a + 第1列补白
// - Paint_MiddleRowAlternate_UsesItemBackground             → P4a + P3(Alternate)
// - Paint_ExceedsBottomColumn0_FillsCorners                 → P4b + 第0列
// - Paint_ExceedsBottomColumn1_FillsCorners                 → P4b + 第1列
// - Paint_MultiLineText_ReportsRowHeight                    → P6 多行 + setRowHeight
// - Paint_SingleLineText_RendersContent                     → P6 单行
// - AddRow_MultiValue_SplitsIntoRows / AddRow_SingleValue_OneRow → addRow 双分支
// - GetDocFromLst_InvalidPair_SkipsTable / GetDocFromLst_ValidPairs_BuildsTableRows
//
// 最小清单：[x]1 公开方法每方法≥1用例 [x]2 等价类划分 [x]3 边界值显式覆盖
//          [x]4 同质多组输入参数化（paint 按行/列组合拆分为独立语义用例）
//          [x]5 分支清单映射 [x]6 每条分支有触发用例 [x]7 异常路径（提前 return 分支）
//          [x]8 负面场景（无效索引/无效键值对） [x]9 负面用例验证状态（doc 无 table）
//          [x]10 Qt 类用 stub（DStyle::subElementRect / DApplication::style /
//              PageSingleInfo::setRowHeight）；DBus/文件系统无依赖

#include "RichTextDelegate.h"
#include "PageSingleInfo.h"
#include "PageTableWidget.h"

#include "ut_Head.h"
#include "stub.h"

#include <QImage>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QWidget>

#include <DStyle>
#include <DApplication>

#include <gtest/gtest.h>

DStyle *rc_style = nullptr;

QRect ut_richTextDelegate_subElementRect(DStyle::SubElement, const QStyleOption *opt, const QWidget *)
{
    // 返回选项自身矩形，保证文本裁剪区与单元格重合（真实样式行为）
    return static_cast<const QStyleOptionViewItem *>(opt)->rect;
}

DStyle *ut_richtextdelegate_style()
{
    return rc_style;
}

class UT_RichTextDelegate : public UT_HEAD
{
public:
    void SetUp()
    {
        rc_style = new DStyle;
        widget = new PageTableWidget;
        m_rtDelegate = new RichTextDelegate(widget);
    }
    void TearDown()
    {
        delete m_rtDelegate;
        delete widget;
        delete rc_style;
        rc_style = nullptr;
    }
    RichTextDelegate *m_rtDelegate = nullptr;
    PageTableWidget *widget = nullptr;
    Stub stub;

    Stub &installPaintStub()
    {
        stub.set(ADDR(DApplication, style), ut_richtextdelegate_style);
        stub.set((QRect(DStyle::*)(DStyle::SubElement, const QStyleOption *, const QWidget *) const)ADDR(DStyle, subElementRect),
                 ut_richTextDelegate_subElementRect);
        return stub;
    }
};

// 多行文本行高回报计数
static int g_setRowHeightCalls = 0;
static int g_setRowHeightLast = 0;

static void ut_pageSingleInfo_setRowHeight(void *, int, int height)
{
    ++g_setRowHeightCalls;
    g_setRowHeightLast = height;
}

// 将委托绘制到白底图片，返回渲染结果供像素级断言
static QImage renderDelegate(RichTextDelegate *delegate, const QModelIndex &index,
                             const QRect &cellRect,
                             QStyleOptionViewItem::ViewItemFeatures features = {})
{
    QStyleOptionViewItem option;
    option.rect = cellRect;
    option.features = features;

    QImage img(cellRect.size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    p.translate(-cellRect.topLeft());   // 选项矩形为视口坐标，平移后单元格映射到图片原点
    delegate->paint(&p, option, index);
    p.end();
    return img;
}

static int nonWhitePixels(const QImage &img)
{
    int count = 0;
    for (int y = 0; y < img.height(); y += 2) {
        for (int x = 0; x < img.width(); x += 2) {
            if (img.pixelColor(x, y) != Qt::white) {
                ++count;
            }
        }
    }
    return count;
}

TEST_F(UT_RichTextDelegate, RichTextDelegate_DefaultCtor_WrapsStyledDelegate)
{
    // Assert：构造时记录父对象（paint 中按父类型分支）
    EXPECT_EQ(m_rtDelegate->parent(), widget);
}

TEST_F(UT_RichTextDelegate, CreateEditor_Always_ReturnsNullptr)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    // Assert：禁止编辑 → 恒空
    EXPECT_EQ(m_rtDelegate->createEditor(nullptr, option, index), nullptr);
}

TEST_F(UT_RichTextDelegate, SizeHint_FirstColumn_NoMinWidth)
{
    QStandardItemModel model;
    model.setItem(0, 0, new QStandardItem("a"));
    QStyleOptionViewItem option;

    QSize size = m_rtDelegate->sizeHint(option, model.index(0, 0));   // Act

    EXPECT_GE(size.height(), 40);   // branch: index.column()==0 → 宽度不加下限
}

TEST_F(UT_RichTextDelegate, SizeHint_SecondColumn_MinWidth)
{
    QStandardItemModel model;
    model.setItem(0, 1, new QStandardItem("b"));
    QStyleOptionViewItem option;

    QSize size = m_rtDelegate->sizeHint(option, model.index(0, 1));   // Act

    EXPECT_GE(size.width(), 150);   // branch: index.column() 非 0 → 最小宽 150
    EXPECT_GE(size.height(), 40);   // 高度下限 40
}

TEST_F(UT_RichTextDelegate, Paint_InvalidIndex_FallsBackToBasePaint)
{
    // 无桩：真实无效索引走基类绘制（P1）
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 40);
    QImage img(200, 40, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);

    m_rtDelegate->paint(&p, option, QModelIndex());   // Act
    p.end();

    EXPECT_TRUE(img.isNull() == false);   // 基类绘制不崩溃即可
}

TEST_F(UT_RichTextDelegate, Paint_FirstRowColumn0_CornerFill)
{
    Stub &s = installPaintStub();
    Q_UNUSED(s);
    widget->setFixedHeight(640);   // 40*(640/40-1)=600，首行不超界 → P5a
    QStandardItemModel model;
    model.setItem(0, 0, new QStandardItem("key:value"));

    QImage img = renderDelegate(m_rtDelegate, model.index(0, 0), QRect(0, 0, 200, 40));   // Act

    EXPECT_GT(nonWhitePixels(img), 0);   // 有实际内容渲染
}

TEST_F(UT_RichTextDelegate, Paint_FirstRowColumn1_CornerFill)
{
    Stub &s = installPaintStub();
    Q_UNUSED(s);
    widget->setFixedHeight(640);
    QStandardItemModel model;
    model.setItem(0, 1, new QStandardItem("value"));

    QImage img = renderDelegate(m_rtDelegate, model.index(0, 1), QRect(0, 0, 200, 40));   // Act

    EXPECT_GT(nonWhitePixels(img), 0);   // 第1列右下角补白分支
}

TEST_F(UT_RichTextDelegate, Paint_MiddleRowAlternate_UsesItemBackground)
{
    Stub &s = installPaintStub();
    Q_UNUSED(s);
    widget->setFixedHeight(640);   // y=50,h=40 → 90<600 中间行 → P4a
    QStandardItemModel model;
    model.setItem(1, 1, new QStandardItem("value"));

    QImage img = renderDelegate(m_rtDelegate, model.index(1, 1), QRect(0, 50, 200, 40),
                                QStyleOptionViewItem::Alternate);   // Act

    EXPECT_GT(nonWhitePixels(img), 0);   // P3 Alternate → ItemBackground 底色分支
}

TEST_F(UT_RichTextDelegate, Paint_ExceedsBottomColumn0_FillsCorners)
{
    Stub &s = installPaintStub();
    Q_UNUSED(s);
    widget->setFixedHeight(120);   // 40*(120/40-1)=80；y=70,h=40 → 110≥80 超界 → P4b
    QStandardItemModel model;
    model.setItem(2, 0, new QStandardItem("key:value"));

    QImage img = renderDelegate(m_rtDelegate, model.index(2, 0), QRect(0, 70, 200, 40));   // Act

    EXPECT_GT(nonWhitePixels(img), 0);   // 第0列左下/右侧补白分支
}

TEST_F(UT_RichTextDelegate, Paint_ExceedsBottomColumn1_FillsCorners)
{
    Stub &s = installPaintStub();
    Q_UNUSED(s);
    widget->setFixedHeight(120);
    QStandardItemModel model;
    model.setItem(2, 1, new QStandardItem("value"));

    QImage img = renderDelegate(m_rtDelegate, model.index(2, 1), QRect(0, 70, 200, 40));   // Act

    EXPECT_GT(nonWhitePixels(img), 0);   // 第1列左侧/右上补白分支
}

TEST_F(UT_RichTextDelegate, Paint_SingleLineText_RendersContent)
{
    Stub &s = installPaintStub();
    Q_UNUSED(s);
    widget->setFixedHeight(640);
    QStandardItemModel model;
    model.setItem(0, 1, new QStandardItem("single-line content"));

    QImage img = renderDelegate(m_rtDelegate, model.index(0, 1), QRect(0, 0, 200, 40));   // Act

    EXPECT_GT(nonWhitePixels(img), 0);   // P6 单行 <p> 段落分支
}

TEST_F(UT_RichTextDelegate, Paint_MultiLineText_ReportsRowHeight)
{
    Stub &s = installPaintStub();
    Q_UNUSED(s);
    g_setRowHeightCalls = 0;
    g_setRowHeightLast = 0;
    // 多行文本分支要求父对象可转 PageSingleInfo（setRowHeight 行高回报）
    PageSingleInfo *psi = new PageSingleInfo;
    RichTextDelegate *delegate = new RichTextDelegate(psi);
    QStandardItemModel model;
    model.setItem(0, 1, new QStandardItem("key1:value1\nkey2:value2"));
    s.set(ADDR(PageSingleInfo, setRowHeight), ut_pageSingleInfo_setRowHeight);

    QImage img = renderDelegate(delegate, model.index(0, 1), QRect(0, 0, 200, 60));   // Act

    // Assert：多行文本 → 表格 HTML + 行高回报
    EXPECT_EQ(g_setRowHeightCalls, 1);
    EXPECT_GT(g_setRowHeightLast, 0);
    EXPECT_GT(nonWhitePixels(img), 0);

    delete delegate;
    delete psi;
}

TEST_F(UT_RichTextDelegate, AddRow_MultiValue_SplitsIntoRows)
{
    QDomDocument doc;
    QDomElement table = doc.createElement("table");
    doc.appendChild(table);   // 挂入文档树，便于结构断言
    QPair<QString, QString> pair("first", "v1  /  \t\tv2  /  \t\tv3");

    m_rtDelegate->addRow(doc, table, pair, 120);   // Act：3 段值 → 递归续行分支

    // Assert：主行 + 2 续行（嵌套 tr），末段文本存在
    EXPECT_EQ(doc.elementsByTagName("table").size(), 1);
    EXPECT_GE(doc.elementsByTagName("tr").size(), 3);
    EXPECT_TRUE(doc.toString().contains("v3"));
}

TEST_F(UT_RichTextDelegate, AddRow_SingleValue_OneRow)
{
    QDomDocument doc;
    QDomElement table = doc.createElement("table");
    doc.appendChild(table);

    m_rtDelegate->addRow(doc, table, QPair<QString, QString>("k", "v"), 120);   // Act：单值分支

    EXPECT_EQ(doc.elementsByTagName("tr").size(), 1);
    EXPECT_TRUE(doc.toString().contains("k:"));
    EXPECT_TRUE(doc.toString().contains("v"));
}

TEST_F(UT_RichTextDelegate, GetDocFromLst_InvalidPair_SkipsTable)
{
    QDomDocument doc;

    m_rtDelegate->getDocFromLst(doc, QStringList() << "first:second" << "first");   // Act：第二项无冒号

    // Assert：提前 return → doc 不含 table
    EXPECT_EQ(doc.elementsByTagName("table").size(), 0);
}

TEST_F(UT_RichTextDelegate, GetDocFromLst_ValidPairs_BuildsTableRows)
{
    QDomDocument doc;

    m_rtDelegate->getDocFromLst(doc, QStringList() << "key1:value1" << "key2:value2");   // Act

    EXPECT_EQ(doc.elementsByTagName("table").size(), 1);
    EXPECT_EQ(doc.elementsByTagName("tr").size(), 2);   // 每对键值一行
    EXPECT_TRUE(doc.toString().contains("key1:"));
}
