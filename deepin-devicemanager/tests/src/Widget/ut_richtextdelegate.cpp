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
#include "../src/Widget/RichTextDelegate.h"
#include "../src/Page/PageTableWidget.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QWidget>

#include <DStyle>

#include <gtest/gtest.h>
#include "../stub.h"

class RichTextDelegate_UT : public UT_HEAD
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

TEST_F(RichTextDelegate_UT, ut_paint)
{
    QStyleOptionViewItem option;
    QPainter painter(widget);
    QModelIndex index;
    Stub stub;
    //    stub.set(ADDR(QModelIndex,isValid),ut_richTextDelegate_isValid);
    stub.set((QRect(DStyle::*)(DStyle::SubElement, const QStyleOption *, const QWidget *widget) const)ADDR(DStyle, subElementRect), ut_richTextDelegate_subElementRect);
    m_rtDelegate->paint(&painter, option, index);
}

TEST_F(RichTextDelegate_UT, ut_createEditor)
{
    QStyleOptionViewItem option;
    QPainter painter(widget);
    QModelIndex index;
    m_rtDelegate->createEditor(nullptr, option, index);
}

TEST_F(RichTextDelegate_UT, ut_sizeHint)
{
    QStyleOptionViewItem option;
    QPainter painter(widget);
    QModelIndex index;
    m_rtDelegate->sizeHint(option, index);
}

TEST_F(RichTextDelegate_UT, ut_addRow)
{
    QDomDocument doc;
    QDomElement table;
    doc.setNodeValue("/");
    QPair<QString, QString> pair = QPair<QString, QString>("/", "/");
    m_rtDelegate->addRow(doc, table, pair);
}
