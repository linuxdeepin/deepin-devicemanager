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
#include "../src/DeviceManager/EnableManager.h"
#include "../src/ZeroMQ/ZmqOrder.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QProcess>
#include <QIODevice>

#include <gtest/gtest.h>
#include "../stub.h"

class EnableManager_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

void ut_start()
{
    return;
}

QByteArray ut_readAllStandardOutput()
{
    return "Device Enabled:1";
}

TEST_F(EnableManager_UT, EnableManager_UT_isDeviceEnable)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput);
    ASSERT_TRUE(EnableManager::instance()->isDeviceEnable("/"));
}

TEST_F(EnableManager_UT, EnableManager_UT_isDeviceEnable2)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput);
    ASSERT_TRUE(EnableManager::instance()->isDeviceEnable(1));
}

bool ut_execDriverOrder()
{
    return false;
}

TEST_F(EnableManager_UT, EnableManager_UT_enableDeviceByDriver)
{
    Stub stub;
    stub.set(ADDR(ZmqOrder, execDriverOrder), ut_execDriverOrder);
    ASSERT_EQ(EnableManager::instance()->enableDeviceByDriver(false, "/"), 1);
}

QByteArray ut_readAllStandardOutput_1()
{
    return "filename://123 \n /abc";
}

TEST_F(EnableManager_UT, EnableManager_UT_enablePrinter)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_1);
    EnableManager::instance()->enablePrinter("/", false);
}

bool ut_execIfconfigOrder()
{
    return true;
}

TEST_F(EnableManager_UT, EnableManager_UT_enableNetworkByIfconfig)
{
    Stub stub;
    stub.set(ADDR(ZmqOrder, execIfconfigOrder), ut_execIfconfigOrder);
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_1);
    ASSERT_EQ(EnableManager::instance()->enableNetworkByIfconfig("/", false), 2);
}

TEST_F(EnableManager_UT, EnableManager_UT_isDeviceId)
{
    Stub stub;
    stub.set(ADDR(ZmqOrder, execIfconfigOrder), ut_execIfconfigOrder);
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_1);
    EnableManager::instance()->isDeviceId(10, "/");
}

TEST_F(EnableManager_UT, EnableManager_UT_getDriverPath)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_1);
    EnableManager::instance()->getDriverPath("/");
}
