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
#include "DeviceAudio.h"
#include "EnableManager.h"

#include "stub.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>


class UT_DeviceAudio : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceAudio = new DeviceAudio;
    }
    void TearDown()
    {
        delete m_deviceAudio;
    }
    DeviceAudio *m_deviceAudio;
};

TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromHwinfo)
{
    QMap<QString, QString> map;
    map.insert("Device", "Cannon Lake PCH cAVS");
    map.insert("Vendor", "Intel Corporation");
    map.insert("Model", "Intel Cannon Lake PCH cAVS");
    map.insert("Driver", "snd_hda_intel");
    m_deviceAudio->setInfoFromHwinfo(map);
    EXPECT_STREQ("Cannon Lake PCH cAVS", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceAudio->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Intel Cannon Lake PCH cAVS", m_deviceAudio->m_Model.toStdString().c_str());
    EXPECT_STREQ("snd_hda_intel", m_deviceAudio->m_Driver.toStdString().c_str());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromLshw)
{
    QMap<QString, QString> map;
    map.insert("product", "Cannon Lake PCH cAVS");
    map.insert("vendor", "Intel Corporation");
    map.insert("version", "10");
    map.insert("width", "64 bits");
    map.insert("clock", "33MHz");
    map.insert("description", "Audio device");
    map.insert("bus info", "pci@0000:00:1f.3");
    m_deviceAudio->m_UniqueKey = "0000:00:1f.3";
    m_deviceAudio->setInfoFromLshw(map);
    EXPECT_STREQ("Cannon Lake PCH cAVS", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceAudio->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("10", m_deviceAudio->m_Version.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceAudio->m_Width.toStdString().c_str());
    EXPECT_STREQ("33MHz", m_deviceAudio->m_Clock.toStdString().c_str());
    EXPECT_STREQ("Audio device", m_deviceAudio->m_Description.toStdString().c_str());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromCatDevices)
{
    QMap<QString, QString> map;
    map.insert("Name", "Cannon Lake PCH cAVS");
    map.insert("Vendor", "Intel Corporation");
    map.insert("Version", "10");
    m_deviceAudio->setInfoFromCatDevices(map);
    EXPECT_STREQ("Cannon Lake PCH cAVS", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceAudio->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("10", m_deviceAudio->m_Version.toStdString().c_str());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromCatAudio)
{
    QMap<QString, QString> map;
    map.insert("Name", "Cannon Lake PCH cAVS");
    m_deviceAudio->setInfoFromCatAudio(map);
    EXPECT_STREQ("Cannon Lake PCH cAVS", m_deviceAudio->m_Name.toStdString().c_str());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_setAudioChipFromDmesg)
{
    m_deviceAudio->setAudioChipFromDmesg("x86");
    EXPECT_STREQ("x86", m_deviceAudio->m_Chip.toStdString().c_str());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_name)
{
    QMap<QString, QString> map;
    map.insert("Name", "Cannon Lake PCH cAVS");
    m_deviceAudio->setInfoFromCatAudio(map);
    QString name = m_deviceAudio->name();
    EXPECT_STREQ("Cannon Lake PCH cAVS", name.toStdString().c_str());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_driver)
{
    QMap<QString, QString> map;
    map.insert("Driver", "snd_hda_intel");
    m_deviceAudio->setInfoFromHwinfo(map);
    QString value = m_deviceAudio->driver();
    EXPECT_STREQ("snd_hda_intel", value.toStdString().c_str());
}

EnableDeviceStatus ut_audio_enableDeviceByDriver()
{
    return EnableDeviceStatus::EDS_Faild;
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_setEnable)
{
    Stub stub;
    stub.set(ADDR(EnableManager, enableDeviceByDriver), ut_audio_enableDeviceByDriver);
    EnableDeviceStatus value = m_deviceAudio->setEnable(false);
    EXPECT_EQ(1, value);
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_subTitle)
{
    QMap<QString, QString> map;
    map.insert("Name", "Cannon Lake PCH cAVS");
    m_deviceAudio->setInfoFromCatAudio(map);
    QString value = m_deviceAudio->subTitle();
    EXPECT_STREQ("Cannon Lake PCH cAVS", value.toStdString().c_str());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_getOverviewInfo)
{
    QMap<QString, QString> map;
    map.insert("Name", "Cannon Lake PCH cAVS");
    m_deviceAudio->setInfoFromCatAudio(map);
    QString value = m_deviceAudio->getOverviewInfo();
    EXPECT_STREQ("Cannon Lake PCH cAVS", value.toStdString().c_str());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_initFilterKey)
{
    m_deviceAudio->initFilterKey();
    EXPECT_EQ(19, m_deviceAudio->m_FilterKey.size());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_loadBaseDeviceInfo)
{
    QMap<QString, QString> map;
    map.insert("Device", "Cannon Lake PCH cAVS");
    map.insert("Vendor", "Intel Corporation");
    map.insert("Model", "Intel Cannon Lake PCH cAVS");
    map.insert("Driver", "snd_hda_intel");
    m_deviceAudio->setInfoFromHwinfo(map);

    m_deviceAudio->loadBaseDeviceInfo();

    QPair<QString, QString> value0 = m_deviceAudio->m_LstBaseInfo.at(0);
    EXPECT_STREQ("Cannon Lake PCH cAVS", value0.second.toStdString().c_str());
    QPair<QString, QString> value1 = m_deviceAudio->m_LstBaseInfo.at(1);
    EXPECT_STREQ("Intel Corporation", value1.second.toStdString().c_str());
    QPair<QString, QString> value2 = m_deviceAudio->m_LstBaseInfo.at(2);
    EXPECT_STREQ("Intel Cannon Lake PCH cAVS", value2.second.toStdString().c_str());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_loadOtherDeviceInfo)
{
    m_deviceAudio->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_deviceAudio->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_loadTableHeader)
{
    m_deviceAudio->loadTableHeader();
    EXPECT_EQ(2, m_deviceAudio->m_TableHeader.size());
}

TEST_F(UT_DeviceAudio, UT_DeviceAudio_loadTableData)
{
    m_deviceAudio->loadTableData();
    EXPECT_EQ(2, m_deviceAudio->m_TableData.size());
}


