/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     kongyunzhen <kongyunzhen@uniontech.com>
*
* Maintainer: kongyunzhen <kongyunzhen@uniontech.com>
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
#ifndef DRIVERMANAGER_H
#define DRIVERMANAGER_H

#include "commonfunction.h"

#include <QObject>

#include <cups.h>

class ModCore;
class DebInstaller;
class DriverInstaller;
class QThread;

class DriverManager : public QObject
{
    Q_OBJECT
public:
    explicit DriverManager(QObject *parent = nullptr);
    ~ DriverManager();

    bool checkDriverInfo(); //检测驱动是否需要更新
    bool unInstallDriver(const QString &moduleName); //驱动卸载
    bool installDriver(const QString &filepath);     // 驱动安装
    void installDriver(const QString &pkgName, const QString &version);// 驱动安装
    void undoInstallDriver(); // 取消当前的驱动安装
    //获取依赖当前模块在使用的模块
    QStringList checkModuleInUsed(const QString &modName);
    //检查当前模块是否在黑名单
    bool isBlackListed(const QString &modName);
    //判断文件是否驱动包
    bool isDriverPackage(const QString &filepath);
    //判断文件是否数字签名
    bool isSigned(const QString &filepath);
    //判断架构是否匹配
    bool isArchMatched(const QString &path);
    //判断包是否有效
    bool isDebValid(const QString &filePath);

private:
    void initConnections();
    bool unInstallModule(const QString &moduleName, QString &msg);
    bool checkPrinterInfo(); //检测打印机驱动是否需要更新 返回值true：是 false：否
    void loadPrinterInfo(QMap<QString, QString> &mapInfo);
    bool checkBoardCardInfo();//检测板载设备驱动是否需要更新 返回值true：是 false：否
    bool checkBoardCardInfo(const DriverType type, QMap<QString, QString> &mapInfo);
    bool checkCameraInfo(QMap<QString, QString> &mapInfo);

    /**
     * @brief getMapInfo:解析打印机cups第三方库获取的信息
     * @param mapInfo:解析得到的map信息
     * @param src:cups获取的原始信息
     */
    void getMapInfo(QMap<QString, QString> &mapInfo, cups_dest_t *src);
    void getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));


signals:
    void sigProgressDetail(int progress, const QString &strDeatils);
    void sigFinished(bool bsuccess, QString msg);
    void sigDebInstall(const QString &package);
    void sigDebUnstall(const QString &package);

    void sigDownloadProgressChanged(QStringList msg);//驱动下载进度、速度、已下载大小
    void sigDownloadFinished();//下载完成
    void sigInstallProgressChanged(int progress);//安装进度
    void sigInstallProgressFinished(bool bsuccess, int err);

public slots:

private:
    ModCore *mp_modcore = nullptr;
    DebInstaller *mp_debinstaller = nullptr;
    QThread *mp_deboperatethread = nullptr;
    DriverInstaller *mp_driverInstaller = nullptr;
    QThread *mp_driverOperateThread = nullptr;

    int m_installprocess = 0;
    QString errmsg;

    //打印机相关===begin
public:
    typedef struct TPrinterInfo {
        int intSid;         //sid
        QString strDescribe;    //描述
        QString strPpd;         //ppd
        bool isExcat;       //打印机厂商和型号
        QString strDriver;      //驱动
    } TDriverInfo;
public slots:
    //卸载打印机
    Q_SCRIPTABLE bool uninstallPrinter(const QString &vendor, const QString &model);

private:
    //解析服务器回读的数据
    QList<TPrinterInfo> parsePrinterInfo(const QByteArray &byteArray);

    //安装包已经安装
    bool printerHasInstalled(const QString &packageName);

    //安装
    bool installPrinter(const QString &packageName);

    //卸载
    bool unInstallPrinter(const QString &packageName);
    //打印机相关===end
};

#endif // DRIVERMANAGER_H
