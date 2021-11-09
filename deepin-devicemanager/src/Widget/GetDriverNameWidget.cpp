#include "GetDriverNameWidget.h"

#include <DFrame>
#include <DLabel>

#include <QVBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QFileIconProvider>

DWIDGET_BEGIN_NAMESPACE

GetDriverNameWidget::GetDriverNameWidget(QWidget *parent)
    :DWidget (parent)
    ,mp_ListView(new DriverListView(this))
{
    init();
    initConnections();
}

void GetDriverNameWidget::init()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    DFrame *frame = new DFrame;
    frame->setFixedSize(460,160);
    mainLayout->setContentsMargins(0,0,0,0);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    DLabel *titleLable = new DLabel(tr("Select a driver for update"));
    hLayout->addStretch();
    hLayout->addWidget(titleLable);
    hLayout->addStretch();

    QHBoxLayout *driverLayout = new QHBoxLayout;
    driverLayout->setContentsMargins(0,0,0,10);
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

void GetDriverNameWidget::initConnections()
{
    connect(mp_ListView, &DriverListView::clicked, this, &GetDriverNameWidget::slotSelectedDriver);
}

void GetDriverNameWidget::loadAllDrivers(bool includeSub, const QString& path)
{
    // 获取所有的驱动文件
    QStringList lstDrivers;
    traverseFolders(includeSub, path, lstDrivers);
    if(lstDrivers.isEmpty()){
        mp_selectedRow = -1;
        return;
    }
    mp_model = new QStandardItemModel(this);
    for (int i = 0; i < lstDrivers.size(); i++) {
        QStandardItem *icomItem = new QStandardItem;
        //获取应用文件图标
        QFileInfo info(mp_driverPathList[i]);
        QFileIconProvider icon_provider;
        QIcon icon = icon_provider.icon(info);
        icomItem->setData(icon, Qt::DecorationRole);
        QStandardItem *textItem = new QStandardItem(lstDrivers[i]);
        mp_model->setItem(i,0,icomItem);
        mp_model->setItem(i,1,textItem);
        textItem->setCheckable(true);
        textItem->setToolTip(lstDrivers[i]);
    }

    mp_ListView->setModel(mp_model);
    mp_ListView->setColumnWidth(0,40);
}

QString GetDriverNameWidget::selectName()
{
    if (-1 == mp_selectedRow || !(mp_selectedRow < mp_driverPathList.size() && mp_selectedRow >-1))
        return "";
    return mp_driverPathList[mp_selectedRow];
}

void GetDriverNameWidget::traverseFolders(bool includeSub, const QString& path, QStringList& lstDrivers)
{
    QDir dir(path);
    if(!dir.exists())
        return;
    QStringList nameFiltes;
    nameFiltes << "*.deb" << "*.ko";
    QStringList drivers;
    drivers.append(dir.entryList(nameFiltes, QDir::Files | QDir::Readable, QDir::Name));
    mp_driversList.append(drivers);
    for(QString filename : drivers){
        mp_driverPathList.append(path+ "/" + filename);
    }

    if (includeSub) {
        QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if(!dirs.isEmpty()){
            for (QString folder : dirs) {
                if(folder.isEmpty())
                    continue;
                QString subPath = path + "/" + folder;
                traverseFolders(includeSub, subPath, lstDrivers);
            }
        }
    }
    lstDrivers = mp_driversList;
}

void GetDriverNameWidget::slotSelectedDriver(const QModelIndex &index)
{
    emit signalItemClicked();
    int row = index.row();
    QStandardItem *item =  mp_model->item(row, 1);
    if (!item)
        return;
    if (Qt::Unchecked == item->checkState()){
        item->setCheckState(Qt::Checked);
        QStandardItem *lastItem = mp_model->item(mp_selectedRow, 1);
        if (lastItem)
            lastItem->setCheckState(Qt::Unchecked);
         mp_selectedRow = row;
    }else{
        item->setCheckState(Qt::Unchecked);
        mp_selectedRow = -1;
    }
}

DWIDGET_END_NAMESPACE
