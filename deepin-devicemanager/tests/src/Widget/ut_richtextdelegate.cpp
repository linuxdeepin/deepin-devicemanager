/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "RichTextDelegate.h"
#include "PageTableWidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QWidget>

#include <DStyle>

#include <gtest/gtest.h>

class UT_RichTextDelegate : public UT_HEAD
{
public:
    void SetUp()
    {
        widget = new PageTableWidget;
        m_rtDelegate = new RichTextDelegate(widget);
    }
    void TearDown()
    {
        delete m_rtDelegate;
        delete widget;
    }
    RichTextDelegate *m_rtDelegate;
    PageTableWidget *widget;
};

bool ut_richTextDelegate_isValid()
{
    return true;
}

QRect ut_richTextDelegate_subElementRect()
{
    return QRect(10, 10, 10, 10);
}

TEST_F(UT_RichTextDelegate, UT_RichTextDelegate_RichTextDelegate_paint)
{
//    QStyleOptionViewItem option;
//    QPainter painter(widget);
//    QModelIndex index;
//    Stub stub;
//    stub.set(ADDR(QModelIndex, isValid), ut_richTextDelegate_isValid);
//    stub.set((QRect(DStyle::*)(DStyle::SubElement, const QStyleOption *, const QWidget *widget) const)ADDR(DStyle, subElementRect), ut_richTextDelegate_subElementRect);
//    m_rtDelegate->paint(&painter, option, index);
//    EXPECT_FALSE(widget->grab().isNull());
}

TEST_F(UT_RichTextDelegate, UT_RichTextDelegate_RichTextDelegate_createEditor)
{
    QStyleOptionViewItem option;
    QPainter painter(widget);
    QModelIndex index;
    EXPECT_FALSE(m_rtDelegate->createEditor(nullptr, option, index));
}

TEST_F(UT_RichTextDelegate, UT_RichTextDelegate_RichTextDelegate_sizeHint)
{
    QStyleOptionViewItem option;
    QPainter painter(widget);
    QModelIndex index;
    QSize size = m_rtDelegate->sizeHint(option, index);
    EXPECT_EQ(150,size.width());
    EXPECT_EQ(40,size.height());
}

TEST_F(UT_RichTextDelegate, UT_RichTextDelegate_RichTextDelegate_addRow)
{
    QDomDocument doc;
    QDomElement table = doc.createElement("table");
    table.setAttribute("style", "border-collapse: collapse;border-spacing: 0;");
    QPair<QString, QString> pair = QPair<QString, QString>("first", "second1  /  \t\tsecond2  /  \t\tsecond3");
    m_rtDelegate->addRow(doc, table, pair);
    EXPECT_FALSE(doc.isNull());
}

TEST_F(UT_RichTextDelegate, UT_RichTextDelegate_RichTextDelegate_getDocFromLst)
{
    QDomDocument doc;
    doc.setNodeValue("/");
    m_rtDelegate->getDocFromLst(doc, QStringList() << "first:second"
                                                   << "first");
    EXPECT_FALSE(doc.isNull());
}
