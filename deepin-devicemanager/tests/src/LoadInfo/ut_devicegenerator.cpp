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
#include "DeviceGenerator.h"
#include "DeviceFactory.h"
#include "GenerateDevicePool.h"
#include "DeviceManager.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class DeviceGenerator_UT : public UT_HEAD
{
public:
    void SetUp()
    {
//        m_generateDevicePool = new GenerateDevicePool;
    }
    void TearDown()
    {
        delete m_deviceGenerator;
    }
    DeviceGenerator *m_deviceGenerator = nullptr;
};

QList<QMap<QString, QString>> ut_generator_cmdInfo()
{
    QList<QMap<QString, QString>> info;
    QMap<QString, QString> map;
    map.insert("name", "/");
    map.insert("type", "/");
    map.insert("name", "/");
    map.insert("type", "/");
    map.insert("name", "/");
    map.insert("type", "/");
    info.append(map);
    return info;
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorComputerDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorComputerDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorCpuDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorCpuDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_getMemoryInfoFromDmidecode)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->getMemoryInfoFromDmidecode();
    m_deviceGenerator->getGpuInfoFromLshw();
    m_deviceGenerator->getMonitorInfoFromXrandrVerbose();
    m_deviceGenerator->getAudioInfoFromHwinfo();
    m_deviceGenerator->getAudioInfoFromLshw();
    m_deviceGenerator->getAudioInfoFromCatInput();
    m_deviceGenerator->getAudioChipInfoFromDmesg();
    m_deviceGenerator->getBluetoothInfoFromLshw();
    m_deviceGenerator->getOthersInfoFromLshw();
    m_deviceGenerator->getCdromInfoFromLshw();
    m_deviceGenerator->getImageInfoFromLshw();
    m_deviceGenerator->getGpuInfoFromXrandr();
    m_deviceGenerator->getImageInfoFromHwinfo();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorBiosDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorBiosDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorDiskDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorDiskDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorGpuDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorGpuDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorMonitorDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorMonitorDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorNetworkDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorNetworkDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorAudioDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorAudioDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorBluetoothDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorBluetoothDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorKeyboardDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorKeyboardDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorMouseDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorMouseDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorPrinterDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorPrinterDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorCameraDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorCameraDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorCdromDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorCdromDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorOthersDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorOthersDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_generatorPowerDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorPowerDevice();
}

TEST_F(DeviceGenerator_UT, DeviceGenerator_UT_getOthersInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_generator_cmdInfo);
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->getOthersInfoFromLshw();
    m_deviceGenerator->getOthersInfoFromHwinfo();
}
