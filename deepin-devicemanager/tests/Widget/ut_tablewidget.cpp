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
#include "../src/Widget/TableWidget.h"
#include "../src/DeviceManager/DeviceInfo.h"
#include "../src/DeviceManager/DeviceInput.h"

#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class TableWidget_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_tableWidget;
    }
    TableWidget *m_tableWidget;
};

TEST_F(TableWidget_UT, ut_setHeaderLabels)
{
    m_tableWidget = new TableWidget;
    m_tableWidget->setHeaderLabels(QStringList() << "no");
}

TEST_F(TableWidget_UT, ut_setItem)
{
    m_tableWidget = new TableWidget;
    DStandardItem *item = new DStandardItem;
    m_tableWidget->setItem(0, 0, item);
    delete item;
    m_tableWidget->setColumnAverage();
    m_tableWidget->updateCurItemEnable(0, true);
}
