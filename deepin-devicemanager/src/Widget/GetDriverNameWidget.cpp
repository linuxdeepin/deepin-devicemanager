#include "GetDriverNameWidget.h"
#include "DriverListView.h"

#include <DFrame>
#include <DLabel>

#include <QVBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QStandardItemModel>
DWIDGET_BEGIN_NAMESPACE
GetDriverNameWidget::GetDriverNameWidget(QWidget *parent)
    :DWidget (parent)
    , mp_ListView(new DriverListView(this))
{
    init();
}

void GetDriverNameWidget::init()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    DFrame *frame = new DFrame;
    frame->setFixedSize(460,180);
    mainLayout->setContentsMargins(0,0,0,0);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    DLabel *titleLable = new DLabel(tr("Select the driver that needs to be updated"));
    hLayout->addStretch();
    hLayout->addWidget(titleLable);
    hLayout->addStretch();

    QHBoxLayout *driverLayout = new QHBoxLayout;
    driverLayout->setContentsMargins(0,10,0,10);
    driverLayout->addStretch();
    driverLayout->addWidget(frame);
    driverLayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(5,10,5,10);
    layout->addStretch();
    layout->addWidget(mp_ListView);
    layout->addStretch();
    frame->setLayout(layout);

    mainLayout->addStretch();
    mainLayout->addLayout(hLayout);
    mainLayout->addLayout(driverLayout);
    mainLayout->addStretch();

    this->setLayout(mainLayout);
}

void GetDriverNameWidget::loadAllDrivers(bool includeSub, const QString& path)
{
    // 获取所有的驱动文件
    QStringList lstDrivers;
    traverseFolders(includeSub,path,lstDrivers);

    // 将信息写到界面上
    QStandardItemModel *model = new QStandardItemModel(this);
    for(int i = 0; i < lstDrivers.size(); i++){
        QStandardItem *icomItem = new QStandardItem;
        icomItem->setData(QIcon::fromTheme("deepin-devicemanager"),Qt::DecorationRole);
        QStandardItem *textItem = new QStandardItem(lstDrivers[i]);
        model->setItem(i,0,icomItem);
        model->setItem(i,1,textItem);
        textItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        textItem->setData(Qt::Checked, Qt::CheckStateRole);
    }
    mp_ListView->setModel(model);
    mp_ListView->setColumnWidth(0,40);
}

QString GetDriverNameWidget::selectName()
{
    return "/lib/modules/4.19.0-amd64-desktop/kernel/drivers/usb/typec/typec.ko";
}

void GetDriverNameWidget::traverseFolders(bool includeSub, const QString& path, QStringList& lstDrivers)
{
    QDir dir(path);
    if(!dir.exists())
        return;
    QStringList nameFiltes;
    nameFiltes << "*.deb" << "*.ko";
    lstDrivers.append(dir.entryList(nameFiltes, QDir::Files | QDir::Readable, QDir::Name));
    if(includeSub){
        QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if(dirs.isEmpty())
            return ;
        for (QString folder : dirs) {
            QString subPath = path + "/" + folder;
            traverseFolders(includeSub,subPath,lstDrivers);
        }
    }
}

DWIDGET_END_NAMESPACE
