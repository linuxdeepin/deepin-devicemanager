// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceManager 的遍历查找方法 getMouseDevice/getBluetoothDevice/getAudioDevice/
// getNetworkDevice/getImageDevice/getOthersDevice/getBluetoothAtIndex/deleteDisableDuplicate_AudioDevice。
// 注入设备到各 list 后查找,纯逻辑遍历。

#include "DeviceManager.h"
#include "DeviceInput.h"
#include "DeviceBluetooth.h"
#include "DeviceAudio.h"
#include "DeviceNetwork.h"
#include "DeviceImage.h"
#include "DeviceOthers.h"
#include "ut_Head.h"

#include <QList>
#include <QPair>
#include <QString>

#include <gtest/gtest.h>

class UT_DeviceManagerQuery : public UT_HEAD
{
public:
    void SetUp() override
    {
        dm = DeviceManager::instance();
        dm->clear();
        dm->addMouseDevice(new DeviceInput);
        dm->addKeyboardDevice(new DeviceInput);
        dm->addBluetoothDevice(new DeviceBluetooth);
        dm->addAudioDevice(new DeviceAudio);
        dm->addNetworkDevice(new DeviceNetwork);
        dm->addImageDevice(new DeviceImage);
        dm->addOthersDevice(new DeviceOthers);
    }
    void TearDown() override
    {
        dm->clear();
    }
    DeviceManager *dm = nullptr;
};

TEST_F(UT_DeviceManagerQuery, getMouseDevice_notFound)
{
    EXPECT_EQ(dm->getMouseDevice("nonexist_uid"), nullptr);
}

TEST_F(UT_DeviceManagerQuery, getBluetoothDevice_notFound)
{
    EXPECT_EQ(dm->getBluetoothDevice("nonexist_uid"), nullptr);
}

TEST_F(UT_DeviceManagerQuery, getBluetoothAtIndex)
{
    EXPECT_NE(dm->getBluetoothAtIndex(0), nullptr);
    EXPECT_EQ(dm->getBluetoothAtIndex(99), nullptr);
}

TEST_F(UT_DeviceManagerQuery, getAudioDevice_notFound)
{
    EXPECT_EQ(dm->getAudioDevice("/nonexist/path"), nullptr);
}

TEST_F(UT_DeviceManagerQuery, getNetworkDevice_notFound)
{
    EXPECT_EQ(dm->getNetworkDevice("nonexist_uid"), nullptr);
}

TEST_F(UT_DeviceManagerQuery, getImageDevice_notFound)
{
    EXPECT_EQ(dm->getImageDevice("nonexist_uid"), nullptr);
}

TEST_F(UT_DeviceManagerQuery, getOthersDevice_notFound)
{
    EXPECT_EQ(dm->getOthersDevice("nonexist_uid"), nullptr);
}

TEST_F(UT_DeviceManagerQuery, deleteDisableDuplicate_AudioDevice)
{
    EXPECT_NO_FATAL_FAILURE(dm->deleteDisableDuplicate_AudioDevice());
}

TEST_F(UT_DeviceManagerQuery, getCpuHeaderInfo)
{
    QList<QList<QPair<QString, QString> > > info;
    EXPECT_NO_FATAL_FAILURE(dm->getCpuHeaderInfo(info));
}
