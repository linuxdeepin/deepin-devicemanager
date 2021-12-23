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
#include "drivericonwidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <DLabel>

#include <QIcon>

#include <gtest/gtest.h>


class UT_DriverIconWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        QIcon icon(QIcon::fromTheme("cautious"));
        QPixmap pic = icon.pixmap(80, 80);
        m_DriverIconWidgetPixMap = new DriverIconWidget(pic, "Warning", "The device will be unavailable after the driver uninstallation");
    }
    void TearDown()
    {
        delete m_DriverIconWidgetPixMap;
    }

    DriverIconWidget *m_DriverIconWidgetPixMap;
};
