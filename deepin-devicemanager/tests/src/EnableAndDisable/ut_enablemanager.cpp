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
#include "EnableManager.h"
#include "DBusInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QProcess>
#include <QIODevice>

#include <gtest/gtest.h>

class UT_EnableManager : public UT_HEAD
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

TEST_F(UT_EnableManager, UT_EnableManager_isDeviceEnable)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput);
    ASSERT_TRUE(EnableManager::instance()->isDeviceEnable("/"));
}

TEST_F(UT_EnableManager, UT_EnableManager_isDeviceEnable2)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput);
    ASSERT_TRUE(EnableManager::instance()->isDeviceEnable(1));
}

bool ut_execDriverOrder_001()
{
    return false;
}
bool ut_execDriverOrder_002()
{
    return true;
}

TEST_F(UT_EnableManager, UT_EnableManager_enableDeviceByDriver_001)
{
    Stub stub;
    stub.set(ADDR(DBusInterface, execDriverOrder), ut_execDriverOrder_001);
    ASSERT_EQ(EnableManager::instance()->enableDeviceByDriver(false, "driver"), 1);
    ASSERT_EQ(EnableManager::instance()->enableDeviceByDriver(true, "driver"), 1);
}

TEST_F(UT_EnableManager, UT_EnableManager_enableDeviceByDriver_011)
{
    Stub stub;
    stub.set(ADDR(DBusInterface, execDriverOrder), ut_execDriverOrder_002);
    ASSERT_EQ(EnableManager::instance()->enableDeviceByDriver(false, "driver"), 2);
    ASSERT_EQ(EnableManager::instance()->enableDeviceByDriver(true, "driver"), 2);
}

TEST_F(UT_EnableManager, UT_EnableManager_enableDeviceByDriver_002)
{
    Stub stub;
    stub.set(ADDR(DBusInterface, execDriverOrder), ut_execDriverOrder_002);
    ASSERT_EQ(EnableManager::instance()->enableDeviceByDriver(false, "driver"), 2);
    ASSERT_EQ(EnableManager::instance()->enableDeviceByDriver(true, "driver"), 2);
}

QByteArray ut_readAllStandardOutput_1()
{
    return "";
}
QByteArray ut_readAllStandardOutput_2()
{
    return "filename:abc";
}

TEST_F(UT_EnableManager, UT_EnableManager_enablePrinter_001)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_1);
    ASSERT_EQ(EnableManager::instance()->enablePrinter("printer", true), 2);
    ASSERT_EQ(EnableManager::instance()->enablePrinter("printer", false), 2);
}

TEST_F(UT_EnableManager, UT_EnableManager_enablePrinter_002)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_2);
    ASSERT_EQ(EnableManager::instance()->enablePrinter("printer", true), 1);
    ASSERT_EQ(EnableManager::instance()->enablePrinter("printer", false), 1);
}

bool ut_execIfconfigOrder_1()
{
    return true;
}
bool ut_execIfconfigOrder_2()
{
    return false;
}

TEST_F(UT_EnableManager, UT_EnableManager_enableNetworkByIfconfig_001)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_1);
    stub.set(ADDR(DBusInterface, execIfconfigOrder), ut_execIfconfigOrder_1);
    ASSERT_EQ(EnableManager::instance()->enableNetworkByIfconfig("ifconfig", true), 2);
    ASSERT_EQ(EnableManager::instance()->enableNetworkByIfconfig("ifconfig", false), 2);
}

TEST_F(UT_EnableManager, UT_EnableManager_enableNetworkByIfconfig_002)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_1);
    stub.set(ADDR(DBusInterface, execIfconfigOrder), ut_execIfconfigOrder_2);
    ASSERT_EQ(EnableManager::instance()->enableNetworkByIfconfig("ifconfig", true), 1);
    ASSERT_EQ(EnableManager::instance()->enableNetworkByIfconfig("ifconfig", false), 1);
}

TEST_F(UT_EnableManager, UT_EnableManager_isDeviceId)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_2);
    EXPECT_TRUE(EnableManager::instance()->isDeviceId(10, "abc"));
}

TEST_F(UT_EnableManager, UT_EnableManager_getDriverPath)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_start);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_readAllStandardOutput_2);
    EXPECT_STREQ("abc", EnableManager::instance()->getDriverPath("path").toStdString().c_str());
}
