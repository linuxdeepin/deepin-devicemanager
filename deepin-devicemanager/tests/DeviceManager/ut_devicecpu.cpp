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
#include "src/DeviceManager/DeviceCpu.h"

#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class DeviceCpu_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_deviceCpu;
    }
    DeviceCpu *m_deviceCpu;
};

TEST_F(DeviceCpu_UT, DeviceCpu_UT_setCpuInfo)
{
    m_deviceCpu = new DeviceCpu;
    QMap<QString, QString> mapLscpu;
    QMap<QString, QString> mapLshw;
    QMap<QString, QString> mapDmidecode;
    QMap<QString, QString> catInfo;
    mapLscpu.insert("Vendor", "1@n");
    mapLshw.insert("Version", "1.0");
    mapDmidecode.insert("Vendor", "test");
    catInfo.insert("Version", "2.0");
    m_deviceCpu->setCpuInfo(mapLscpu, mapLshw, mapDmidecode, catInfo, 0, 0);
}

TEST_F(DeviceCpu_UT, DeviceCpu_UT_initFilterKey)
{
    m_deviceCpu = new DeviceCpu;
    m_deviceCpu->initFilterKey();
    m_deviceCpu->getOverviewInfo();
    m_deviceCpu->loadBaseDeviceInfo();
    m_deviceCpu->loadOtherDeviceInfo();
    m_deviceCpu->loadTableHeader();
    m_deviceCpu->loadTableData();
}

TEST_F(DeviceCpu_UT, DeviceCpu_UT_name)
{
    m_deviceCpu = new DeviceCpu;
    m_deviceCpu->name();
    m_deviceCpu->driver();
    m_deviceCpu->subTitle();
    ASSERT_FALSE(m_deviceCpu->frequencyIsRange());
}

TEST_F(DeviceCpu_UT, DeviceCpu_UT_setInfoFromLscpu)
{
    m_deviceCpu = new DeviceCpu;
    QMap<QString, QString> mapLscpu;
    QMap<QString, QString> mapLshw;
    QMap<QString, QString> mapDmidecode;
    QMap<QString, QString> catInfo;
    mapLscpu.insert("Vendor", "1@n");
    mapLshw.insert("Version", "1.0");
    mapDmidecode.insert("Vendor", "test");
    catInfo.insert("Version", "2.0");
    m_deviceCpu->setInfoFromLscpu(mapLscpu);
}

TEST_F(DeviceCpu_UT, DeviceCpu_UT_setInfoFromLshw)
{
    m_deviceCpu = new DeviceCpu;
    QMap<QString, QString> mapLscpu;
    QMap<QString, QString> mapLshw;
    QMap<QString, QString> mapDmidecode;
    QMap<QString, QString> catInfo;
    mapLscpu.insert("Vendor", "1@n");
    mapLshw.insert("Version", "1.0");
    mapDmidecode.insert("Vendor", "test");
    catInfo.insert("Version", "2.0");
    m_deviceCpu->setInfoFromLshw(mapLshw);
}

TEST_F(DeviceCpu_UT, DeviceCpu_UT_setInfoFromDmidecode)
{
    m_deviceCpu = new DeviceCpu;
    QMap<QString, QString> mapLscpu;
    QMap<QString, QString> mapLshw;
    QMap<QString, QString> mapDmidecode;
    QMap<QString, QString> catInfo;
    mapLscpu.insert("Vendor", "1@n");
    mapLshw.insert("Version", "1.0");
    mapDmidecode.insert("Name", "Loongson");
    catInfo.insert("Version", "2.0");
    m_deviceCpu->setInfoFromDmidecode(mapDmidecode);
}

TEST_F(DeviceCpu_UT, DeviceCpu_UT_setInfoFromCatCpuinfo)
{
    m_deviceCpu = new DeviceCpu;
    QMap<QString, QString> mapLscpu;
    QMap<QString, QString> mapLshw;
    QMap<QString, QString> mapDmidecode;
    QMap<QString, QString> catInfo;
    mapLscpu.insert("Vendor", "1@n");
    mapLshw.insert("Version", "1.0");
    mapDmidecode.insert("Name", "Loongson");
    catInfo.insert("Version", "2.0");
    m_deviceCpu->setInfoFromCatCpuinfo(catInfo);
}

TEST_F(DeviceCpu_UT, DeviceCpu_UT_getTrNumber)
{
    m_deviceCpu = new DeviceCpu;
    m_deviceCpu->getTrNumber();
}
