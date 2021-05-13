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
#include "../src/LoadInfo/PanguVGenerator.h"
#include "../src/LoadInfo/PanguGenerator.h"
#include "../src/LoadInfo/MipsGenerator.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class PanguVGenerator_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_PanguVGenerator = new PanguVGenerator;
    }
    void TearDown()
    {
        delete m_PanguVGenerator;
    }
    PanguVGenerator *m_PanguVGenerator = nullptr;
};

class PanguGenerator_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_PanguGenerator = new PanguGenerator;
    }
    void TearDown()
    {
        delete m_PanguGenerator;
    }
    PanguGenerator *m_PanguGenerator = nullptr;
};

class MipsGenerator_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_MipsGenerator = new MipsGenerator;
    }
    void TearDown()
    {
        delete m_MipsGenerator;
    }
    MipsGenerator *m_MipsGenerator = nullptr;
};

TEST_F(PanguVGenerator_UT, PanguVGenerator_UT_generatorMonitorDevice)
{
    m_PanguVGenerator->generatorMonitorDevice();
}

TEST_F(PanguGenerator_UT, PanguGenerator_UT_generatorComputerDevice)
{
    m_PanguGenerator->generatorComputerDevice();
}

TEST_F(MipsGenerator_UT, MipsGenerator_UT_generatorComputerDevice)
{
    m_MipsGenerator->generatorComputerDevice();
}
