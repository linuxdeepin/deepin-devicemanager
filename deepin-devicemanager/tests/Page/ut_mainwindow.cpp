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
#include "../src/Page/PageInfo.h"
#include "../src/Page/MainWindow.h"
#include "../src/DeviceManager/DeviceInput.h"
#include "../src/DeviceManager/DeviceInfo.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QFileDialog>
#include <QProcess>
#include <QJsonArray>
#include <QJsonDocument>
#include <QIODevice>

#include <gtest/gtest.h>
#include <tests/stub.h>

void ut_refreshDataBase()
{
    return;
}

class MainWindow_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
        delete m_mainWindow;
    }
    MainWindow *m_mainWindow = nullptr;
};

TEST_F(MainWindow_UT, MainWindow_UT_refresh)
{
    Stub stub;
    stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
    m_mainWindow = new MainWindow;
    m_mainWindow->refresh();
    m_mainWindow->slotRefreshInfo();
}

QString UT_getSaveFileName()
{
    return "1.txt";
}

TEST_F(MainWindow_UT, MainWindow_UT_exportTo)
{
    Stub stub;
    stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
    stub.set(ADDR(QFileDialog, getSaveFileName), UT_getSaveFileName);
    m_mainWindow = new MainWindow;
    m_mainWindow->exportTo();
    m_mainWindow->slotExportInfo();
}

bool ut_process_startDetached()
{
    return false;
}

TEST_F(MainWindow_UT, MainWindow_UT_showDisplayShortcutsHelpDialog)
{
    //
    //        Stub stub;
    //        stub.set((bool (QProcess::*)(const QString &, const QStringList &))ADDR(QProcess,startDetached), ut_process_startDetached);
    //        m_mainWindow->showDisplayShortcutsHelpDialog();
}

TEST_F(MainWindow_UT, MainWindow_UT_addJsonArrayItem)
{
    Stub stub;
    stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
    m_mainWindow = new MainWindow;
    QJsonArray array;
    array.insert(0, QJsonValue("/"));
    m_mainWindow->addJsonArrayItem(array, "test", "/");
}

TEST_F(MainWindow_UT, MainWindow_UT_getJsonDoc)
{
    Stub stub;
    stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
    m_mainWindow = new MainWindow;
    QJsonDocument doc;
    QJsonArray array;
    array.insert(0, QJsonValue("/"));
    doc.setArray(array);
    m_mainWindow->getJsonDoc(doc);
    m_mainWindow->windowMaximizing();
}

TEST_F(MainWindow_UT, MainWindow_UT_resizeEvent)
{
    Stub stub;
    stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
    m_mainWindow = new MainWindow;
    QResizeEvent resizeevent(QSize(10, 10), QSize(10, 10));
    m_mainWindow->resizeEvent(&resizeevent);
}

TEST_F(MainWindow_UT, MainWindow_UT_slotListItemClicked)
{
    Stub stub;
    stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
    m_mainWindow = new MainWindow;
    m_mainWindow->slotListItemClicked("Monitor");
    m_mainWindow->changeUI();
}

TEST_F(MainWindow_UT, MainWindow_UT_keyPressEvent)
{
    Stub stub;
    stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
    m_mainWindow = new MainWindow;
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(m_mainWindow, &keyPressEvent);
}

TEST_F(MainWindow_UT, MainWindow_UT_event)
{
    Stub stub;
    stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
    m_mainWindow = new MainWindow;
    QEvent event(QEvent::ApplicationFontChange);
    QCoreApplication::sendEvent(m_mainWindow, &event);
}

TEST_F(MainWindow_UT, MainWindow_UT_initWindow)
{
    Stub stub;
    stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
    m_mainWindow = new MainWindow;
    m_mainWindow->initWindow();
}

TEST_F(MainWindow_UT, MainWindow_UT_loadingFinishSlot)
{
    Stub stub;
    stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
    m_mainWindow = new MainWindow;
    m_mainWindow->loadingFinishSlot("finish");
}
