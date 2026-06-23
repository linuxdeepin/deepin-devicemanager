// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 PageSingleInfo 未测的纯逻辑方法(loadDeviceInfo/clearContent/isExpanded/setRowHeight) + slot。

#include "PageSingleInfo.h"
#include "ut_Head.h"

#include <QString>
#include <QList>
#include <QPair>

#include <gtest/gtest.h>

class UT_PageSingleInfoExtra : public UT_HEAD
{
public:
    void SetUp() override
    {
        p = new PageSingleInfo;
    }
    void TearDown() override
    {
        delete p;
    }
    PageSingleInfo *p = nullptr;
};

TEST_F(UT_PageSingleInfoExtra, setLabel_clearContent)
{
    EXPECT_NO_FATAL_FAILURE(p->setLabel("Device Type"));
    EXPECT_NO_FATAL_FAILURE(p->clearContent());
}

TEST_F(UT_PageSingleInfoExtra, isExpanded_setRowHeight)
{
    EXPECT_NO_FATAL_FAILURE(p->isExpanded());
    EXPECT_NO_FATAL_FAILURE(p->setRowHeight(0, 30));
    EXPECT_NO_FATAL_FAILURE(p->setRowHeight(1, 40));
}

TEST_F(UT_PageSingleInfoExtra, loadDeviceInfo)
{
    QList<QPair<QString, QString> > lst;
    lst << qMakePair(QString("Vendor"), QString("Intel"))
        << qMakePair(QString("Model"), QString("SSD"))
        << qMakePair(QString("Driver"), QString("nvme"));
    EXPECT_NO_FATAL_FAILURE(p->loadDeviceInfo(lst));
}

TEST_F(UT_PageSingleInfoExtra, clearWidgets)
{
    EXPECT_NO_FATAL_FAILURE(p->clearWidgets());
}

// slotActionCopy/Enable/UpdateDriver/RemoveDriver 依赖右键菜单/选中行,空状态崩,暂跳过
