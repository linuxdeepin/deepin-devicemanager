// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SingleDeviceManager.h"
#include "MainWindow.h"
#include "eventlogutils.h"
#include "DDLog.h"

#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <QJsonObject>

using namespace DDLog;

SingleDeviceManager::SingleDeviceManager(int &argc, char **argv)
    : DApplication(argc, argv)
{
    qCDebug(appLog) << "SingleDeviceManager constructor called";
}

void SingleDeviceManager::activateWindow()
{
    qCInfo(appLog) << "SingleDeviceManager::activateWindow()";
    if (nullptr == m_qspMainWnd.get()) {
        m_qspMainWnd.reset(new MainWindow());
        Dtk::Widget::moveToCenter(m_qspMainWnd.get());
        m_qspMainWnd->show();
        qCDebug(appLog) << "Main window created and shown.";
        QJsonObject obj{
            {"tid", EventLogUtils::Start},
            {"version", QCoreApplication::applicationVersion()},
            {"mode", 1}
        };
        EventLogUtils::get().writeLogs(obj);
    } else {
        m_qspMainWnd->setWindowState(Qt::WindowActive);
        m_qspMainWnd->activateWindow(); // Reactive main window
        m_qspMainWnd->showNormal();     //非特效模式下激活窗口
        qCDebug(appLog) << "Main window already exists, activated and shown.";
    }
    if (!m_PageDescription.isEmpty()) {
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "slotSetPage", Qt::QueuedConnection, Q_ARG(QString, m_PageDescription));
        qCDebug(appLog) << "Setting page to:" << m_PageDescription;
    }
}

bool SingleDeviceManager::parseCmdLine()
{
    qCInfo(appLog) << "SingleDeviceManager::parseCmdLine()";
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Device Manager.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("pageDescription", "Page Description.", "pageDescription");
    parser.process(*this);

    if (!m_PageDescription.isEmpty()) {
        m_PageDescription.clear();
        qCDebug(appLog) << "Cleared existing page description.";
    }
    QStringList paraList = parser.positionalArguments();
    if (!paraList.isEmpty()) {
        m_PageDescription = paraList[0];
        qCDebug(appLog) << "Parsed page description:" << m_PageDescription;
    }

    if (paraList.size() > 0 && m_PageDescription.isEmpty()) {
        qCWarning(appLog) << "Positional arguments exist but page description is empty, returning false.";
        return false;
    }
    qCDebug(appLog) << "SingleDeviceManager::parseCmdLine() result:" << true;
    return true;
}

void SingleDeviceManager::startDeviceManager(QString pageDescription)
{
    qCInfo(appLog) << "SingleDeviceManager::startDeviceManager" << pageDescription;
    if (!pageDescription.isEmpty()) {
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "slotSetPage", Qt::QueuedConnection, Q_ARG(QString, pageDescription));
        qCDebug(appLog) << "SingleDeviceManager::startDeviceManager with page:" << pageDescription;
    } else {
        qCDebug(appLog) << "SingleDeviceManager::startDeviceManager with empty page";
        if (m_qspMainWnd.get()) {                   //先判断当前是否已经存在一个进程。
            qCDebug(appLog) << "Main window exists, activating it.";
            m_qspMainWnd.get()->setWindowState((m_qspMainWnd.get()->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
//            m_qspMainWnd.get()->activateWindow();   //特效模式下激活窗口            m_qspMainWnd.get()->showNormal();       //无特效激活窗口
        }
    }
}
