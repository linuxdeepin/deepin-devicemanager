// Copyright (C) 2019 ~ 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QTableWidget>

#define private public
#define protected public
#include "headerinfotablewidget.h"
#include "MacroDefinition.h"

class UT_HeaderInfoTableWidget : public UT_HEAD
{
public:
    void SetUp() override
    {
        m_widget = new HeaderInfoTableWidget;
    }
    void TearDown() override
    {
        delete m_widget;
    }
    HeaderInfoTableWidget *m_widget = nullptr;
};

// 构造函数/initUI
TEST_F(UT_HeaderInfoTableWidget, UT_HeaderInfoTableWidget_ctor)
{
    EXPECT_EQ(2, m_widget->columnCount());
    EXPECT_EQ(QAbstractItemView::NoEditTriggers, m_widget->editTriggers());
    EXPECT_EQ(QAbstractItemView::NoSelection, m_widget->selectionMode());
}

// updateData：有数据
TEST_F(UT_HeaderInfoTableWidget, UT_HeaderInfoTableWidget_updateData)
{
    QList<QPair<QString, QString>> data;
    data << QPair<QString, QString>("Name", "Intel CPU")
         << QPair<QString, QString>("Vendor", "GenuineIntel");
    m_widget->updateData(data);

    EXPECT_EQ(2, m_widget->rowCount());
    EXPECT_STREQ("Name", m_widget->item(0, 0)->text().toStdString().c_str());
    EXPECT_STREQ("Intel CPU", m_widget->item(0, 1)->text().toStdString().c_str());
    EXPECT_STREQ("Vendor", m_widget->item(1, 0)->text().toStdString().c_str());
    EXPECT_STREQ("GenuineIntel", m_widget->item(1, 1)->text().toStdString().c_str());
    // 高度 = ROW_HEIGHT(40) * 行数 + 2
    EXPECT_EQ(ROW_HEIGHT * 2 + 2, m_widget->height());
}

// updateData：空数据 => 高度为 0
TEST_F(UT_HeaderInfoTableWidget, UT_HeaderInfoTableWidget_updateData_empty)
{
    QList<QPair<QString, QString>> data;
    m_widget->updateData(data);
    EXPECT_EQ(0, m_widget->rowCount());
    EXPECT_EQ(0, m_widget->height());
}
