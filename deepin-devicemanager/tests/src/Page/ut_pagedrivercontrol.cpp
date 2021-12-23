/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      jixiaomei <jixiaomei@uniontech.com>
* Maintainer:  jixiaomei <jixiaomei@uniontech.com>
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
#include "PageDriverControl.h"
#include "DBusDriverInterface.h"
#include "DriverWaitingWidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <DWaterProgress>

#include <QKeyEvent>
#include <QEvent>

#include <gtest/gtest.h>

class UT_PageDriverControl : public UT_HEAD
{
public:
    void SetUp()
    {
        m_PageDriverControlInstall = new PageDriverControl(nullptr, "Update Drivers", true, "camera", "");
        m_PageDriverControlUninstall = new PageDriverControl(nullptr, "Update Drivers", false, "camera", "");
    }
    void TearDown()
    {
        delete m_PageDriverControlInstall;
        delete m_PageDriverControlUninstall;
    }

    PageDriverControl *m_PageDriverControlInstall;
    PageDriverControl *m_PageDriverControlUninstall;
};

TEST_F(UT_PageDriverControl, UT_PageDriverControl_isRunning)
{
    EXPECT_FALSE(m_PageDriverControlInstall->isRunning());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotBtnCancel)
{
    m_PageDriverControlInstall->slotBtnCancel();
    EXPECT_TRUE(m_PageDriverControlInstall->isHidden());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotBtnNext_002)
{
    m_PageDriverControlUninstall->slotBtnNext();
    EXPECT_EQ(1, m_PageDriverControlUninstall->mp_stackWidget->currentIndex());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotProcessChange)
{
    m_PageDriverControlInstall->slotProcessChange(40, "ee");
    EXPECT_EQ(40, m_PageDriverControlInstall->mp_WaitDialog->mp_Progress->value());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotProcessEnd)
{
    m_PageDriverControlInstall->slotProcessEnd(false, "Unknown error");
    EXPECT_EQ(1, m_PageDriverControlInstall->mp_stackWidget->currentIndex());

}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotClose)
{
    m_PageDriverControlInstall->slotClose();
    EXPECT_TRUE(m_PageDriverControlInstall->isHidden());

}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotBackPathPage)
{
    m_PageDriverControlInstall->slotBackPathPage();
    EXPECT_EQ(0, m_PageDriverControlInstall->mp_stackWidget->currentIndex());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_removeBtn)
{
    m_PageDriverControlInstall->removeBtn();
    EXPECT_EQ(0, m_PageDriverControlInstall->buttonCount());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_installErrorTips)
{
    bool ret = m_PageDriverControlInstall->installErrorTips("devicemanager.deb");
    EXPECT_FALSE(ret);
}

bool ut_pagedrivercontrol_isDebValid()
{
    return true;
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_installErrorTips_002)
{
    Stub stub;
    stub.set(ADDR(DBusDriverInterface, isDebValid), ut_pagedrivercontrol_isDebValid);

    bool ret = m_PageDriverControlInstall->installErrorTips("devicemanager.deb");
    EXPECT_FALSE(ret);
}

bool ut_pagedrivercontrol_isArchMatched()
{
    return true;
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_installErrorTips_003)
{
    Stub stub;
    stub.set(ADDR(DBusDriverInterface, isDebValid), ut_pagedrivercontrol_isDebValid);
    stub.set(ADDR(DBusDriverInterface, isArchMatched), ut_pagedrivercontrol_isArchMatched);

    bool ret = m_PageDriverControlInstall->installErrorTips("devicemanager.deb");
    EXPECT_FALSE(ret);
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_F4, Qt::AltModifier);
    EXPECT_TRUE(m_PageDriverControlInstall->event(&keyPressEvent));
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_closeEvent)
{

}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_initErrMsg)
{

}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_errMsg)
{

}

