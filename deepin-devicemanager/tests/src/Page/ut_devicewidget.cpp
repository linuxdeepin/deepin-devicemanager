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
#include "../src/Page/DeviceWidget.h"
#include "../src/DeviceManager/DeviceInfo.h"
#include "../src/DeviceManager/DeviceInput.h"
#include "../src/Page/PageInfoWidget.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class DeviceWidget_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_dWidget = new DeviceWidget;
    }
    void TearDown()
    {
        delete m_dWidget;
    }
    DeviceWidget *m_dWidget;
};

TEST_F(DeviceWidget_UT, ut_updateListView)
{
    QList<QPair<QString, QString>> list;
    list.append(QPair<QString, QString>("/", "/"));
    m_dWidget->updateListView(list);
}

TEST_F(DeviceWidget_UT, ut_updateDevice)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    //    DeviceBaseInfo *info = new DeviceBaseInfo(nullptr);
    //    info->addBaseDeviceInfo("/","/");
    //    bInfo.append(info);
    m_dWidget->mp_PageInfo = new PageInfoWidget(m_dWidget);
    m_dWidget->updateDevice("", bInfo);
    delete device;
}

TEST_F(DeviceWidget_UT, ut_updateOverview)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_dWidget->updateOverview("/", mapinfo);
}

TEST_F(DeviceWidget_UT, ut_slotRefreshInfo)
{
    m_dWidget->initWidgets();
    m_dWidget->slotRefreshInfo();
    m_dWidget->slotExportInfo();
    m_dWidget->slotUpdateUI();
}

TEST_F(DeviceWidget_UT, ut_resizeEvent)
{
    QResizeEvent resizeevent(QSize(10, 10), QSize(10, 10));
    m_dWidget->resizeEvent(&resizeevent);
}
