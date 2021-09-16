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
#include "../src/DeviceManager/DeviceManager.h"
#include "../src/LoadInfo/X86Generator.h"
#include "../src/ThreadPool/LoadCpuInfoThread.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class LoadCpuInfoThread_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_loadCpuInfoThread = new LoadCpuInfoThread;
    }
    void TearDown()
    {
        delete m_loadCpuInfoThread;
    }
    LoadCpuInfoThread *m_loadCpuInfoThread;
};

QList<QMap<QString, QString>> ut_LoadCpuInfoThread_cmdInfo()
{
    QList<QMap<QString, QString>> list;
    QMap<QString, QString> map;
    map.insert("/", "/");
    list.append(map);
    return list;
}

TEST_F(LoadCpuInfoThread_UT, LoadCpuInfoThread_UT_getCpuInfoFromLscpu)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_LoadCpuInfoThread_cmdInfo);
    m_loadCpuInfoThread->run();
}
