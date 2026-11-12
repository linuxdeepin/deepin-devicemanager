// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logviewitemdelegate.h"
#include "logtreeview.h"
#include "ut_Head.h"
#include "stub.h"

#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

DStyle *logViewItemDelegateSytle = nullptr;
class UT_LogViewItemDelegate : public UT_HEAD
{
public:
    void SetUp()
    {
        m_Model = new QStandardItemModel;
        m_view = new LogTreeView;
        m_view->setModel(m_Model);
        m_Model->appendRow(new QStandardItem());
        m_logDelegate = new LogViewItemDelegate(m_view);
        logViewItemDelegateSytle = new DStyle;
    }
    void TearDown()
    {
        delete m_logDelegate;
        delete m_view;
        delete m_Model;
        delete logViewItemDelegateSytle;
    }
    LogViewItemDelegate *m_logDelegate;
    LogTreeView *m_view;
    QStandardItemModel *m_Model;
};

int ut_itemdelegate_pixelMetric()
{
    return 10;
}

DStyle *ut_style()
{
    return logViewItemDelegateSytle;
}

TEST_F(UT_LogViewItemDelegate, UT_LogViewItemDelegate_paint)
{
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);

    Stub stub;
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget * widget) const)ADDR(DStyle, pixelMetric), ut_itemdelegate_pixelMetric);
    stub.set(ADDR(DApplication, style), ut_style); // DApplication::style()
    m_logDelegate->paint(&painter, option, index);
    EXPECT_FALSE(m_view->grab().isNull());
}

TEST_F(UT_LogViewItemDelegate, UT_LogViewItemDelegate_createEditor)
{
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);
    EXPECT_FALSE(m_logDelegate->createEditor(nullptr, option, index));
}

TEST_F(UT_LogViewItemDelegate, UT_LogViewItemDelegate_sizeHint)
{
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);
    QSize size = m_logDelegate->sizeHint(option, index);
    // 产品契约(logviewitemdelegate.cpp): 高度 = max(36, 基类高度), 宽度非负。
    // 不应断言精确 6×36: 基类高度/边距取决于应用级字体与样式, 而
    // ut_drivertableview 等套件会 QApplication::setStyle(DStyle), 其 polish
    // 修改全局字体度量, 使精确值随套件顺序漂移(顺序污染)。
    EXPECT_GE(size.height(), 36);
    EXPECT_GE(size.width(), 0);
}
