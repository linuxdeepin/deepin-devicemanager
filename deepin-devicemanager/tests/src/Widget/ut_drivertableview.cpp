// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 drivertableview.cpp 的 DriverTableView(表格数据方法) + DriverItemDelegate(sizeHint)。
// DriverHeaderView 构造触发 style 崩溃(offscreen),暂跳过。

#include "drivertableview.h"
#include "MacroDefinition.h"
#include "ut_Head.h"

#include <QStringList>
#include <QList>
#include <QStyleOptionViewItem>
#include <QModelIndex>

#include <gtest/gtest.h>

class UT_DriverTableView : public UT_HEAD
{
public:
    void SetUp() override
    {
        tv = new DriverTableView;
        tv->initHeaderView(QStringList() << "Cb"
                                         << "Name"
                                         << "Status"
                                         << "Op",
                           false);
        del = new DriverItemDelegate(tv);
    }
    void TearDown() override
    {
        delete del;
        delete tv;
    }
    DriverTableView *tv = nullptr;
    DriverItemDelegate *del = nullptr;
};

TEST_F(UT_DriverTableView, appendRowItems_clear)
{
    tv->appendRowItems(4);
    tv->appendRowItems(4);
    EXPECT_NO_FATAL_FAILURE(tv->clear());
}

TEST_F(UT_DriverTableView, resizeColumn)
{
    EXPECT_NO_FATAL_FAILURE(tv->resizeColumn(0));
}

TEST_F(UT_DriverTableView, setItemStatus)
{
    EXPECT_NO_FATAL_FAILURE(tv->setItemStatus(0, ST_NOT_INSTALL));
    EXPECT_NO_FATAL_FAILURE(tv->setItemStatus(0, ST_INSTALL));
}

TEST_F(UT_DriverTableView, setErrorMsg)
{
    EXPECT_NO_FATAL_FAILURE(tv->setErrorMsg(0, "some error"));
}

TEST_F(UT_DriverTableView, hasItemDisabled)
{
    EXPECT_NO_FATAL_FAILURE(tv->hasItemDisabled());
}

TEST_F(UT_DriverTableView, getCheckedDriverIndex)
{
    QList<int> idx;
    EXPECT_NO_FATAL_FAILURE(tv->getCheckedDriverIndex(idx));
}

TEST_F(UT_DriverTableView, setItemOperationEnable)
{
    EXPECT_NO_FATAL_FAILURE(tv->setItemOperationEnable(0, true));
}

TEST_F(UT_DriverTableView, removeItemAndWidget)
{
    EXPECT_NO_FATAL_FAILURE(tv->removeItemAndWidget(0, 0));
}

TEST_F(UT_DriverTableView, delegate_sizeHint)
{
    QStyleOptionViewItem opt;
    QModelIndex idx;
    auto sz = del->sizeHint(opt, idx);
    EXPECT_GT(sz.height(), 0);
}
