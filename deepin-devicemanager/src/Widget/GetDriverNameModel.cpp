// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GetDriverNameModel.h"
#include "DBusAnythingInterface.h"
#include "DDLog.h"

#include <QDir>
#include <QFileIconProvider>

#include <unistd.h>

using namespace DDLog;

GetDriverNameModel::GetDriverNameModel(QObject *parent)
    : QObject(parent)
{
    qCDebug(appLog) << "Initializing get driver name model";
}

void GetDriverNameModel::stopLoadingDrivers()
{
    qCDebug(appLog) << "Stopping driver loading";
    m_Stop = true;
}

void GetDriverNameModel::startLoadDrivers(QStandardItemModel* model, bool includeSub, const QString &path)
{
    qCDebug(appLog) << "Starting to load drivers, includeSub:" << includeSub << "path:" << path;

    m_Stop = false;
    if(includeSub){
        qCDebug(appLog) << "Including sub folders";
        mp_driverPathList.clear();
        mp_driversList.clear();

        if(DBusAnythingInterface::getInstance()->searchDriver(path,mp_driverPathList)){
            for (const QString& filepath : mp_driverPathList) {
                mp_driversList.append(QFileInfo(filepath).fileName());
            }
        }else{
            qCWarning(appLog) << "Failed to search driver via DBus, traversing folders instead";
            traverseFolders(path,true);
        }
    }else{
        // 获取所有的驱动文件
        qCDebug(appLog) << "Not including sub folders";
        mp_driverPathList.clear();
        mp_driversList.clear();
        traverseFolders(path);
    }

    for (int i = 0; i < mp_driversList.size(); i++) {
        if(m_Stop) {
            qCDebug(appLog) << "Driver loading stopped, return";
            return;
        }
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

void GetDriverNameModel::traverseFolders(const QString &path, bool recursion)
{
    qCDebug(appLog) << "Traversing folder:" << path << "recursion:" << recursion;

    if(m_Stop) {
        qCDebug(appLog) << "Stop traversing folders";
        return;
    }
    QDir dir(path);
    if (!dir.exists()) {
        qCWarning(appLog) << "Directory does not exist:" << path;
        return;
    }

    QStringList nameFiltes;
    nameFiltes << "*.deb" << "*.ko";
    QFileInfoList list = dir.entryInfoList(nameFiltes, QDir::Files | QDir::NoSymLinks);
    foreach(const QFileInfo& info , list){
        qCDebug(appLog) << "Found driver file:" << info.fileName();
        mp_driversList.append(info.fileName());
        mp_driverPathList.append(info.absoluteFilePath());
    }

    // 递归处理子文件夹
    if(recursion){
        qCDebug(appLog) << "Recursively traversing sub folders";
        list = dir.entryInfoList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        foreach(const QFileInfo& info , list){
            traverseFolders(info.absoluteFilePath(),recursion);
        }
    }
}
