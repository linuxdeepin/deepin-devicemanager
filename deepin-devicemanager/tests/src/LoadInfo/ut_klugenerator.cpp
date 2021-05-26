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
#include "../src/LoadInfo/KLUGenerator.h"
#include "../src/DeviceManager/DeviceManager.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class KLUGenerator_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_KLUGenerator = new KLUGenerator;
    }
    void TearDown()
    {
        delete m_KLUGenerator;
    }
    KLUGenerator *m_KLUGenerator = nullptr;
};

QList<QMap<QString, QString>> ut_cmdInfo()
{
    QList<QMap<QString, QString>> list;
    QMap<QString, QString> map;
    map.insert("/", "/");
    list.append(map);
    return list;
}

TEST_F(KLUGenerator_UT, KLUGenerator_UT_generatorComputerDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_cmdInfo);
    m_KLUGenerator->generatorComputerDevice();
}

TEST_F(KLUGenerator_UT, KLUGenerator_UT_generatorGpuDevice)
{
    m_KLUGenerator->generatorGpuDevice();
}

TEST_F(KLUGenerator_UT, KLUGenerator_UT_generatorMonitorDevice)
{
    m_KLUGenerator->generatorMonitorDevice();
}

TEST_F(KLUGenerator_UT, KLUGenerator_UT_generatorAudioDevice)
{
    m_KLUGenerator->generatorAudioDevice();
}

TEST_F(KLUGenerator_UT, KLUGenerator_UT_generatorPowerDevice)
{
    m_KLUGenerator->generatorPowerDevice();
}

TEST_F(KLUGenerator_UT, KLUGenerator_UT_getKeyboardInfoFromHwinfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_cmdInfo);
    m_KLUGenerator->getKeyboardInfoFromHwinfo();
    m_KLUGenerator->getOthersInfoFromHwinfo();
    m_KLUGenerator->getDiskInfoFromHwinfo();
    m_KLUGenerator->getDiskInfoFromLshw();
    m_KLUGenerator->getDiskInfoFromLsblk();
    m_KLUGenerator->getDiskInfoFromSmartCtl();
}
