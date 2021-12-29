/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      jixiaomei <jixiaomei@uniontech.com>
* Maintainer:  jixiaomei <jixiaomei@uniontech.com>
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

#include "GetDriverNameModel.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStackedWidget>
#include <DLabel>

#include <QDir>
#include <QFileIconProvider>

#include <gtest/gtest.h>


class UT_GetDriverNameModel : public UT_HEAD
{
public:
    void SetUp()
    {
        m_GetDriverNameModel = new GetDriverNameModel;
    }
    void TearDown()
    {
        delete m_GetDriverNameModel;
    }

    GetDriverNameModel *m_GetDriverNameModel;
};

TEST_F(UT_GetDriverNameModel, UT_GetDriverNameModel_stopLoadingDrivers)
{
    m_GetDriverNameModel->stopLoadingDrivers();
    EXPECT_TRUE(m_GetDriverNameModel->m_Stop);
}

TEST_F(UT_GetDriverNameModel, UT_GetDriverNameModel_startLoadDrivers_001)
{
    QStandardItemModel *model = new QStandardItemModel();
    m_GetDriverNameModel->startLoadDrivers(model, true, "/home");

    EXPECT_LE(0, m_GetDriverNameModel->mp_driverPathList.size());
    EXPECT_LE(0, m_GetDriverNameModel->mp_driversList.size());

    delete model;
}

TEST_F(UT_GetDriverNameModel, UT_GetDriverNameModel_startLoadDrivers_002)
{
    QStandardItemModel *model = new QStandardItemModel();
    m_GetDriverNameModel->startLoadDrivers(model, false, "/home");

    EXPECT_LE(0, m_GetDriverNameModel->mp_driverPathList.size());
    EXPECT_LE(0, m_GetDriverNameModel->mp_driversList.size());
    delete model;
}

TEST_F(UT_GetDriverNameModel, UT_GetDriverNameModel_traverseFolders_001)
{
    m_GetDriverNameModel->traverseFolders("/home");

    m_GetDriverNameModel->mp_driversList << "a.ko" << "b.ko";

    EXPECT_LE(2, m_GetDriverNameModel->mp_driversList.size());
}

TEST_F(UT_GetDriverNameModel, UT_GetDriverNameModel_traverseFolders_002)
{
    m_GetDriverNameModel->traverseFolders("/home/tets");

    EXPECT_LE(0, m_GetDriverNameModel->mp_driversList.size());
}
