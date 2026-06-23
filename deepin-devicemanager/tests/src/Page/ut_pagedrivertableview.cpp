// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 PageDriverTableView 的转发方法(转给内部 DriverTableView)。构造后建表头即可调用。

#include "PageDriverTableView.h"
#include "MacroDefinition.h"
#include "ut_Head.h"

#include <QAbstractItemModel>
#include <QStringList>
#include <QList>

#include <gtest/gtest.h>

class UT_PageDriverTableView : public UT_HEAD
{
public:
    void SetUp() override
    {
        v = new PageDriverTableView;
        v->initHeaderView(QStringList() << "Cb"
                                        << "Name"
                                        << "Status"
                                        << "Op",
                          false);
    }
    void TearDown() override
    {
        delete v;
    }
    PageDriverTableView *v = nullptr;
};

TEST_F(UT_PageDriverTableView, initHeaderView)
{
    EXPECT_NE(v->model(), nullptr);
    EXPECT_GE(v->model()->columnCount(), 1);
}

TEST_F(UT_PageDriverTableView, appendRowItems_clear)
{
    v->appendRowItems(4);
    v->appendRowItems(4);
    EXPECT_NO_FATAL_FAILURE(v->clear());
}

TEST_F(UT_PageDriverTableView, setColumnWidth)
{
    EXPECT_NO_FATAL_FAILURE(v->setColumnWidth(0, 100));
}

TEST_F(UT_PageDriverTableView, setHeaderCbStatus)
{
    EXPECT_NO_FATAL_FAILURE(v->setHeaderCbStatus(true));
    EXPECT_NO_FATAL_FAILURE(v->setHeaderCbStatus(false));
}

TEST_F(UT_PageDriverTableView, getCheckedDriverIndex)
{
    QList<int> idx;
    EXPECT_NO_FATAL_FAILURE(v->getCheckedDriverIndex(idx));
}

TEST_F(UT_PageDriverTableView, setItemStatus)
{
    EXPECT_NO_FATAL_FAILURE(v->setItemStatus(0, ST_NOT_INSTALL));
}

TEST_F(UT_PageDriverTableView, setErrorMsg)
{
    EXPECT_NO_FATAL_FAILURE(v->setErrorMsg(0, "error msg"));
}

TEST_F(UT_PageDriverTableView, hasItemDisabled)
{
    EXPECT_NO_FATAL_FAILURE(v->hasItemDisabled());
}

TEST_F(UT_PageDriverTableView, setItemOperationEnable)
{
    EXPECT_NO_FATAL_FAILURE(v->setItemOperationEnable(0, true));
}

TEST_F(UT_PageDriverTableView, removeItemAndWidget)
{
    EXPECT_NO_FATAL_FAILURE(v->removeItemAndWidget(0, 0));
}

TEST_F(UT_PageDriverTableView, model)
{
    EXPECT_NE(v->model(), nullptr);
}
