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
#include "../src/LoadInfo/DeviceFactory.h"
#include "../src/LoadInfo/X86Generator.h"
#include "../src/ThreadPool/GenerateDevicePool.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class GenerateTask_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_generateTask = new GenerateTask(type);
    }
    void TearDown()
    {
        delete m_generateTask;
    }
    GenerateTask *m_generateTask;
    DeviceType type = DT_Cpu;
};

class GenerateDevicePool_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_generateDevicePool = new GenerateDevicePool;
    }
    void TearDown()
    {
        delete m_generateDevicePool;
    }
    GenerateDevicePool *m_generateDevicePool;
};

DeviceGenerator *ut_getDeviceGenerator(){
    DeviceGenerator *generator = nullptr;
    generator = new X86Generator();
    return generator;
}

TEST_F(GenerateTask_UT,GenerateTask_UT_run)
{
   Stub stub;
   stub.set(ADDR(DeviceFactory,getDeviceGenerator),ut_getDeviceGenerator);
   m_generateTask->run();
}

TEST_F(GenerateDevicePool_UT,GenerateDevicePool_UT_generateDevice)
{
   m_generateDevicePool->generateDevice();
}
