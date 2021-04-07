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
#include "../src/LoadInfo/DeviceGenerator.h"
#include "../src/LoadInfo/DeviceFactory.h"
#include "../src/ThreadPool/GenerateDevicePool.h"

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

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorComputerDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorComputerDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorCpuDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorCpuDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorBiosDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorBiosDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorDiskDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorDiskDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorGpuDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorGpuDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorMonitorDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorMonitorDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorNetworkDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorNetworkDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorAudioDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorAudioDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorBluetoothDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorBluetoothDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorKeyboardDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorKeyboardDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorMouseDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorMouseDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorPrinterDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorPrinterDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorCameraDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorCameraDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorCdromDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorCdromDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorOthersDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorOthersDevice();
}

TEST_F(DeviceGenerator_UT,DeviceGenerator_UT_generatorPowerDevice)
{
    m_deviceGenerator = DeviceFactory::getDeviceGenerator();
    m_deviceGenerator->generatorPowerDevice();
}
