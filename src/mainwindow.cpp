#include "mainwindow.h"
#include "devicelistwidget.h"
#include <QHBoxLayout>
#include "DStackedWidget"
#include "deviceinfowidgetbase.h"
#include "computeroverviewwidget.h"

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent)
{
    QWidget* mainWidget = new QWidget(this);
    mainWidget->setMaximumHeight(640);
    QHBoxLayout* ly = new QHBoxLayout;
    setFocus(Qt::FocusReason::NoFocusReason);

    leftDeviceList_ = new DeviceListWidget(this);
    leftDeviceList_->setMinimumWidth(160);
    leftDeviceList_->setHeaderHidden(true);
    //leftDeviceList->setFrameStyle(QFrame::NoFrame);

//    leftDeviceList_->addDevice("电脑", ":/cpu.svg");
//    leftDeviceList_->addDevice("主板", ":/cpu.svg");
//    leftDeviceList_->addDevice("处理器", ":/cpu.svg");
//    leftDeviceList_->addDevice("内存", ":/cpu.svg");
//    leftDeviceList_->addDevice("磁盘", ":/cpu.svg");

    ly->addWidget(leftDeviceList_);

    rightDeviceInfoWidget_ = new DStackedWidget(this);
    ComputerOverviewWidget* computerOverviewWidget = new ComputerOverviewWidget(this);

    DeviceInfoWidgetBase* rightWidget = new DeviceInfoWidgetBase(this, "主板");
    addDeviceWidget(computerOverviewWidget);
    addDeviceWidget(rightWidget);

    connect( leftDeviceList_, &DeviceListWidget::currentItemChanged, \
            [this]()
            {
                QString currentDevice = this->leftDeviceList_->currentItem()->text(0);
                rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[currentDevice]);
            }
    );

    ly->addWidget(rightDeviceInfoWidget_);

    rightWidget->setTitle("电脑概况");
    QStringList names;
    names.push_back("电脑型号");
    names.push_back("操作系统");
    names.push_back("处理器");
    names.push_back("处理器");
    names.push_back("处理器");
    names.push_back("处理器");
    names.push_back("处理器");
    names.push_back("电脑型号");
    names.push_back("操作系统");
    names.push_back("处理器");
    names.push_back("处理器");
    names.push_back("处理器");
    names.push_back("处理器");
    names.push_back("处理器");

    QStringList contents;
    contents.push_back("联想F81笔记本电脑");
    contents.push_back("Win 10专业版64位");
    contents.push_back("因特尔Core i5-8300");
    contents.push_back("因特尔Core i5-8300");
    contents.push_back("因特尔Core i5-8300");
    contents.push_back("因特尔Core i5-8300");
    contents.push_back("因特尔Core i5-8300");
    contents.push_back("联想F81笔记本电脑");
    contents.push_back("Win 10专业版64位");
    contents.push_back("因特尔Core i5-8300");
    contents.push_back("因特尔Core i5-8300");
    contents.push_back("因特尔Core i5-8300");
    contents.push_back("因特尔Core i5-8300");
    contents.push_back("因特尔Core i5-8300");

    rightWidget->addSubInfo("PC1", names, contents);
    rightWidget->addSubInfo("PC2", names, contents);
    rightWidget->addSubInfo("PC3", names, contents);
    rightWidget->addSubInfo("PC4", names, contents);

    QStringList headers;
    headers.push_back("内存插槽");
    headers.push_back("大小");
    headers.push_back("类型");
    headers.push_back("速度");
    headers.push_back("状态");

    QList<QStringList> contentsList;
    QStringList list1;
    list1.push_back("Bank 0/DIMM0");
    list1.push_back("4GB");
    list1.push_back("DDR3");
    list1.push_back("1600Mhz");
    list1.push_back("好");

    QStringList list2;
    list2.push_back("Bank 1/DIMM1");
    list2.push_back("");
    list2.push_back("");
    list2.push_back("");
    list2.push_back("-");

    QStringList list3;
    list3.push_back("Bank 2/DIMM2");
    list3.push_back("4GB");
    list3.push_back("DDR3");
    list3.push_back("1600Mhz");
    list3.push_back("好");

    QStringList list4;
    list4.push_back("Bank 3/DIMM3");
    list4.push_back("");
    list4.push_back("");
    list4.push_back("");
    list4.push_back("-");

    contentsList.push_back(list1);
    contentsList.push_back(list2);
    contentsList.push_back(list3);
    contentsList.push_back(list4);

    rightWidget->addTable(headers, contentsList);

    mainWidget->setLayout(ly);

    setCentralWidget(mainWidget);
}

MainWindow::~MainWindow()
{

}

void MainWindow::addDeviceWidget(DeviceInfoWidgetBase* w)
{
    leftDeviceList_->addDevice(w->getDeviceName(), ":/cpu.svg");
    rightDeviceInfoWidget_->addWidget(w);
    deviceInfoWidgetMap_[w->getDeviceName()] = w;
}
