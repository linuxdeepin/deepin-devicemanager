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
    m_Stop = false;
    if(includeSub){
        mp_driverPathList.clear();
        mp_driversList.clear();

        if(DBusAnythingInterface::getInstance()->searchDriver(path,mp_driverPathList)){
            for (const QString& filepath : mp_driverPathList) {
                mp_driversList.append(QFileInfo(filepath).fileName());
            }
        }else{
            traverseFolders(path,true);
        }
    }else{
        // 获取所有的驱动文件
        mp_driverPathList.clear();
        mp_driversList.clear();
        traverseFolders(path);
    }

    for (int i = 0; i < mp_driversList.size(); i++) {
        if(m_Stop)
            return;
        QStandardItem *icomItem = new QStandardItem;
        //获取应用文件图标
        QFileInfo info(mp_driverPathList[i]);
        QFileIconProvider icon_provider;
        QIcon icon = icon_provider.icon(info);
        icomItem->setData(icon, Qt::DecorationRole);
        icomItem->setData(QVariant::fromValue(mp_driverPathList[i]), Qt::UserRole);
        QStandardItem *textItem = new QStandardItem(mp_driversList[i]);
        model->setItem(i, 0, icomItem);
        model->setItem(i, 1, textItem);
        textItem->setToolTip(mp_driversList[i]);
    }

    emit finishLoadDrivers();
}

void GetDriverNameModel::traverseFolders(const QString &path, bool traver)
{
    QDir dir(path);
    if (!dir.exists())
        return;

    QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    foreach(const QFileInfo& info , list){
        // 停止遍历
        if(m_Stop){
            break;
        }

        // 递归处理
        if(info.isDir() && traver){
            traverseFolders(info.absoluteFilePath(),traver);
            continue;
        }

        if(info.suffix() == "deb" || info.suffix() == "ko"){
            mp_driversList.append(info.fileName());
            mp_driverPathList.append(info.absoluteFilePath());
        }
    }
}
