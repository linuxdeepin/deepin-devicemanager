/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GETDRIVERNAMEMODEL_H
#define GETDRIVERNAMEMODEL_H

#include <QObject>
#include <QStandardItemModel>

class GetDriverNameModel : public QObject
{
    Q_OBJECT
public:
    explicit GetDriverNameModel(QObject *parent = nullptr);

    /**
     * @brief stopLoadingDrivers 停止加载设备
     */
    void stopLoadingDrivers();

public slots:
    /**
     * @brief startLoadDrivers 通过给的路径，查找路径下的所有驱动文件
     * @param includeSub 是否查找目录下的子目录
     * @param path 给定的目录
     */
    void startLoadDrivers(QStandardItemModel* model, bool includeSub, const QString &path);

signals:
    void finishLoadDrivers();

private:
    /**
     * @brief traverseFolders 遍历目录下的文件
     * @param path 给定的目录
     */
    void traverseFolders(const QString &path, bool traver = false);

private:
    QStringList             mp_driverPathList;   //驱动路径列表
    QStringList             mp_driversList;      //驱动名列表
    bool                    m_Stop = false;      //停止加载
};

#endif // GETDRIVERNAMEMODEL_H
