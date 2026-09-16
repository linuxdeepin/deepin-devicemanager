// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "headerinfotableDelegate.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStyle>
#include <DApplication>
#include <DPaletteHelper>

#include <QTableWidget>
#include <QPainter>
#include <QStyleOptionViewItem>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

DStyle *ut_HeaderInfoDelegate_style = nullptr;

static DStyle *ut_headerinfo_style()
{
    return ut_HeaderInfoDelegate_style;
}

class UT_HeaderInfoDelegate : public UT_HEAD
{
public:
    void SetUp() override
    {
        m_tableWidget = new QTableWidget;
        m_delegate = new HeaderInfoDelegate(m_tableWidget);
        m_tableWidget->setItemDelegate(m_delegate);
        m_tableWidget->setColumnCount(1);
        m_tableWidget->insertRow(0);
        m_tableWidget->setItem(0, 0, new QTableWidgetItem("test"));
        ut_HeaderInfoDelegate_style = new DStyle;
    }

    void TearDown() override
    {
        delete m_tableWidget;
        delete ut_HeaderInfoDelegate_style;
        ut_HeaderInfoDelegate_style = nullptr;
    }

    QTableWidget *m_tableWidget = nullptr;
    HeaderInfoDelegate *m_delegate = nullptr;
};

// ---- Constructor -----------------------------------------------------------
TEST_F(UT_HeaderInfoDelegate, UT_HeaderInfoDelegate_ctor)
{
    EXPECT_NE(nullptr, m_delegate);
}

// ---- paint (non-selected state) --------------------------------------------
TEST_F(UT_HeaderInfoDelegate, UT_HeaderInfoDelegate_paint_nonSelected)
{
    QStyleOptionViewItem option;
    QPainter painter(m_tableWidget);
    QModelIndex index = m_tableWidget->model()->index(0, 0);

    Stub stub;
    stub.set(ADDR(DApplication, style), ut_headerinfo_style);

    m_delegate->paint(&painter, option, index);
    EXPECT_FALSE(m_tableWidget->grab().isNull());
}

// ---- paint (selected state) ------------------------------------------------
TEST_F(UT_HeaderInfoDelegate, UT_HeaderInfoDelegate_paint_selected)
{
    QStyleOptionViewItem option;
    option.state |= QStyle::State_Selected;
    QPainter painter(m_tableWidget);
    QModelIndex index = m_tableWidget->model()->index(0, 0);

    Stub stub;
    stub.set(ADDR(DApplication, style), ut_headerinfo_style);

    m_delegate->paint(&painter, option, index);
    EXPECT_FALSE(m_tableWidget->grab().isNull());
}

// ---- paint with no active window (inactive palette path) -------------------
TEST_F(UT_HeaderInfoDelegate, UT_HeaderInfoDelegate_paint_noActiveWindow)
{
    QStyleOptionViewItem option;
    QPainter painter(m_tableWidget);
    QModelIndex index = m_tableWidget->model()->index(0, 0);

    Stub stub;
    stub.set(ADDR(DApplication, style), ut_headerinfo_style);

    // DApplication::activeWindow() returns nullptr when no window is active.
    // This exercises the DPalette::Inactive branch.
    m_delegate->paint(&painter, option, index);
    EXPECT_FALSE(m_tableWidget->grab().isNull());
}
