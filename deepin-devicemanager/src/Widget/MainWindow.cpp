#include "MainWindow.h"

#include <QResizeEvent>
#include <QDateTime>
#include <QDebug>

#include <DApplication>

#include "WaitingWidget.h"
#include "DetailWidget.h"
#include "MacroDefinition.h"
#include "ThreadPool.h"
#include "deviceinfoparser.h"

DWIDGET_USE_NAMESPACE

// 主界面需要的一些宏定义
#define INIT_WIDTH  1070    // 窗口的初始化宽度
#define INIT_HEIGHT 790     // 窗口的初始化高度
#define MIN_WIDTH  840      // 窗口的最小宽度
#define MIN_HEIGHT 360      // 窗口的最小高度

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
    , mp_MainStackWidget(new DStackedWidget(this))
    , mp_WaitingWidget(new WaitingWidget(this))
    , mp_DetailWidget(new DetailWidget(this))
    , mp_ThreadPool(new ThreadPool(this))
{
    // 加载授权框
    if (false == DeviceInfoParser::Instance().getRootPassword()) {
        exit(-1);
    }

    // 初始化窗口相关的嗯内容，比如界面布局，控件大小
    initWindow();

    // 加载设备信息
    refreshDataBase();

    // 关联信号槽
    connect(mp_ThreadPool, &ThreadPool::finished, this, &MainWindow::loadingFinishSlot);
}

MainWindow::~MainWindow()
{
    DELETE_PTR(mp_WaitingWidget);
    DELETE_PTR(mp_DetailWidget);
    DELETE_PTR(mp_MainStackWidget);
    DELETE_PTR(mp_ThreadPool);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    DMainWindow::resizeEvent(event);
}

void MainWindow::initWindow()
{
    //1. 第一步初始化窗口大小
    initWindowSize();

    //2. 初始化界面布局
    initWidgets();
}

void MainWindow::initWindowSize()
{
    // 设置窗口的最小尺寸
    QSize minSize(MIN_WIDTH, MIN_HEIGHT);
    setMinimumSize(minSize);

    // 设置窗口的大小
    QSize initSize(INIT_WIDTH, INIT_HEIGHT);
    resize(initSize);
}

void MainWindow::initWidgets()
{
    // 设置窗口的主控件
    setCentralWidget(mp_MainStackWidget);
    setContentsMargins(0, 0, 0, 0);

    // 添加加载等待界面
    mp_MainStackWidget->addWidget(mp_WaitingWidget);
    mp_WaitingWidget->start();


    // 添加信息显示界面
    mp_MainStackWidget->addWidget(mp_DetailWidget);

}

void MainWindow::refreshDataBase()
{
    // 设置应用程序强制光标为cursor
    DApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // 启动线程加载设备信息
    if (mp_ThreadPool) {
        mp_ThreadPool->loadCmdInfo();
    }
}

void MainWindow::loadingFinishSlot(const QString &message)
{
    static bool begin = true;
    static qint64 b = 0;
    static qint64 c = 0;

    if (begin) {
        b = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
    if (message == "finish") {
        c = QDateTime::currentDateTime().toMSecsSinceEpoch();
        begin = true;
        qDebug() << "************************&&*************************" << (c - b) / 1000.0;
        DApplication::restoreOverrideCursor();
        mp_MainStackWidget->setCurrentWidget(mp_DetailWidget);
    }

}
