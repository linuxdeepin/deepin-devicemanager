#include "SingleDeviceManager.h"
#include "MainWindow.h"
#include "eventlogutils.h"

#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <QJsonObject>

SingleDeviceManager::SingleDeviceManager(int &argc, char **argv)
    : DApplication(argc, argv)
{

}

void SingleDeviceManager::activateWindow()
{
    qInfo() << "SingleDeviceManager::activateWindow()";
    if (nullptr == m_qspMainWnd.get()) {
        m_qspMainWnd.reset(new MainWindow());
        Dtk::Widget::moveToCenter(m_qspMainWnd.get());
        m_qspMainWnd->show();
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
    }
    if (!m_PageDescription.isEmpty()) {
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "slotSetPage", Qt::QueuedConnection, Q_ARG(QString, m_PageDescription));
    }
}

bool SingleDeviceManager::parseCmdLine()
{
    qInfo() << "SingleDeviceManager::parseCmdLine()";
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Device Manager.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("pageDescription", "Page Description.", "pageDescription");
    parser.process(*this);

    if (!m_PageDescription.isEmpty()) {
        m_PageDescription.clear();
    }
    QStringList paraList = parser.positionalArguments();
    if (!paraList.isEmpty()) {
        m_PageDescription = paraList[0];
    }

    if (paraList.size() > 0 && m_PageDescription.isEmpty()) {
        return false;
    }
    return true;
}

void SingleDeviceManager::startDeviceManager(QString pageDescription)
{
    qInfo() << "SingleDeviceManager::startDeviceManager" << pageDescription;
    if (!pageDescription.isEmpty()) {
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "slotSetPage", Qt::QueuedConnection, Q_ARG(QString, pageDescription));
    } else {
        if (m_qspMainWnd.get()) {                   //先判断当前是否已经存在一个进程。
            m_qspMainWnd.get()->activateWindow();   //特效模式下激活窗口            m_qspMainWnd.get()->showNormal();       //无特效激活窗口
        }
    }
}
