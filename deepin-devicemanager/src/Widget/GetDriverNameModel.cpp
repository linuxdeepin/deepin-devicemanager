#include "GetDriverNameModel.h"
#include "DBusAnythingInterface.h"

#include <QDir>
#include <QFileIconProvider>

#include <unistd.h>

GetDriverNameModel::GetDriverNameModel(QObject *parent)
    : QObject(parent)
{

}

void GetDriverNameModel::stopLoadingDrivers()
{
    m_Stop = true;
}

void GetDriverNameModel::startLoadDrivers(QStandardItemModel* model, bool includeSub, const QString &path)
{
    if(includeSub){
        mp_driverPathList.clear();
        mp_driversList.clear();

        QString pathT = path;
        DBusAnythingInterface::getInstance()->searchDriver(pathT,mp_driverPathList);
        for (const QString& filepath : mp_driverPathList) {
            mp_driversList.append(QFileInfo(filepath).fileName());
        }
    }else{
        // 获取所有的驱动文件
        mp_driverPathList.clear();
        mp_driversList.clear();
        traverseFolders(path);
    }

    m_Stop = false;
    for (int i = 0; i < mp_driversList.size(); i++) {
        if(m_Stop)
            return;
        QStandardItem *icomItem = new QStandardItem;
        //获取应用文件图标
        QFileInfo info(mp_driverPathList[i]);
        QFileIconProvider icon_provider;
        QIcon icon = icon_provider.icon(info);
        icomItem->setData(icon, Qt::DecorationRole);
        QStandardItem *textItem = new QStandardItem(mp_driversList[i]);
        model->setItem(i, 0, icomItem);
        model->setItem(i, 1, textItem);
        textItem->setToolTip(mp_driversList[i]);
    }

    emit finishLoadDrivers();
}

void GetDriverNameModel::traverseFolders(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return;
    QStringList nameFiltes;
    nameFiltes << "*.deb" << "*.ko";
    mp_driversList.append(dir.entryList(nameFiltes, QDir::Files | QDir::Readable, QDir::Name));
    mp_driversList.append(mp_driversList);
    for (const QString& filename : mp_driversList) {
        mp_driverPathList.append(path + "/" + filename);
    }
}
