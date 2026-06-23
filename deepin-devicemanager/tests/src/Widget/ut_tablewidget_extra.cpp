// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 TableWidget 的数据方法 + 未测 slot(slotActionUpdateDriver/RemoveDriver/Wakeup)。

#include "TableWidget.h"
#include "ut_Head.h"

#include <QStringList>
#include <DStandardItem>

#include <gtest/gtest.h>

class UT_TableWidgetExtra : public UT_HEAD
{
public:
    void SetUp() override
    {
        t = new TableWidget;
        t->setHeaderLabels(QStringList() << "A"
                                         << "B"
                                         << "C");
    }
    void TearDown() override
    {
        delete t;
    }
    TableWidget *t = nullptr;
};

TEST_F(UT_TableWidgetExtra, setRowNum_setItem_clear)
{
    t->setRowNum(3);
    DStandardItem *item = new DStandardItem("cell");
    EXPECT_NO_FATAL_FAILURE(t->setItem(0, 0, item));
    EXPECT_NO_FATAL_FAILURE(t->clear());
}

TEST_F(UT_TableWidgetExtra, setColumnAverage)
{
    EXPECT_NO_FATAL_FAILURE(t->setColumnAverage());
}

TEST_F(UT_TableWidgetExtra, updateCurItemEnable)
{
    t->setRowNum(2);
    EXPECT_NO_FATAL_FAILURE(t->updateCurItemEnable(0, true));
    EXPECT_NO_FATAL_FAILURE(t->updateCurItemEnable(0, false));
}

TEST_F(UT_TableWidgetExtra, slotActionRefresh)
{
    EXPECT_NO_FATAL_FAILURE(t->slotActionRefresh());
}

TEST_F(UT_TableWidgetExtra, slotActionExport)
{
    EXPECT_NO_FATAL_FAILURE(t->slotActionExport());
}

TEST_F(UT_TableWidgetExtra, slotActionEnable)
{
    EXPECT_NO_FATAL_FAILURE(t->slotActionEnable());
}

TEST_F(UT_TableWidgetExtra, slotActionUpdateDriver)
{
    EXPECT_NO_FATAL_FAILURE(t->slotActionUpdateDriver());
}

TEST_F(UT_TableWidgetExtra, slotActionRemoveDriver)
{
    EXPECT_NO_FATAL_FAILURE(t->slotActionRemoveDriver());
}

TEST_F(UT_TableWidgetExtra, slotWakeupMachine)
{
    EXPECT_NO_FATAL_FAILURE(t->slotWakeupMachine());
}
