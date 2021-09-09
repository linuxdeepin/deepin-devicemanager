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
#include "logviewheaderview.h"
#include "ut_Head.h"
#include "stub.h"

#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QStandardItemModel>

#include <gtest/gtest.h>

DStyle *style = nullptr;
class UT_LogviewHeaderView : public UT_HEAD
{
public:
    void SetUp()
    {
        m_Model = new QStandardItemModel;
        m_logViewHeaderView = new LogViewHeaderView(Qt::Horizontal);
        m_logViewHeaderView->setModel(m_Model);
        style = new DStyle;
    }
    void TearDown()
    {
        delete m_logViewHeaderView;
        delete m_Model;
        delete style;
    }
    LogViewHeaderView *m_logViewHeaderView;
    QStandardItemModel *m_Model;
};

int ut_headerview_pixelMetric()
{
    return 10;
}

QVariant ut_model_headerData()
{
    return "header";
}

DStyle *ut_header_style()
{
    return style;
}

TEST_F(UT_LogviewHeaderView, UT_LogviewHeaderView_paintSection)
{
    QPainter painter(m_logViewHeaderView);
    Stub stub;
    stub.set(ADDR(DApplication, style), ut_header_style); // DApplication::style()
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget * widget) const)ADDR(DStyle, pixelMetric), ut_headerview_pixelMetric);
    m_logViewHeaderView->paintSection(&painter, QRect(10, 10, 10, 10), 0);
    EXPECT_FALSE(m_logViewHeaderView->grab().isNull());
}

TEST_F(UT_LogviewHeaderView, UT_LogviewHeaderView_paintEvent)
{
    QPaintEvent paint(QRect(m_logViewHeaderView->rect()));
    Stub stub;
    stub.set(ADDR(DApplication, style), ut_header_style);
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget * widget) const)ADDR(DStyle, pixelMetric), ut_headerview_pixelMetric);
    m_logViewHeaderView->paintEvent(&paint);
    EXPECT_FALSE(m_logViewHeaderView->grab().isNull());
}

TEST_F(UT_LogviewHeaderView, UT_LogviewHeaderView_sizeHint)
{
    QSize size = m_logViewHeaderView->sizeHint();
    EXPECT_EQ(640,size.width());
    EXPECT_EQ(37,size.height());
}
