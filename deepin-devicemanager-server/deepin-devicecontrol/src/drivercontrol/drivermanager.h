// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISABLE_DRIVER

#ifndef DRIVERMANAGER_H
#define DRIVERMANAGER_H

#include "commonfunction.h"

#include <QObject>
#include <QDBusUnixFileDescriptor>

#include <cups.h>

class ModCore;
class AptInstaller;
class DriverInstallerApt;
class QThread;

class DriverManager : public QObject
{
    Q_OBJECT

public:
    typedef struct TPrinterInfo {
        int intSid;         //sid
        QString strDescribe;    //描述
        QString strPpd;         //ppd
        bool isExcat;       //打印机厂商和型号
        QString strDriver;      //驱动
    } TDriverInfo;


    explicit DriverManager(QObject *parent = nullptr);
    ~ DriverManager();

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
    //判断架构是否匹配（fd 方式：内容桥接到守护进程私有目录后校验）
    bool isArchMatchedFd(const QDBusUnixFileDescriptor &fileFd);
    //判断包是否有效（fd 方式）
    bool isDebValidFd(const QDBusUnixFileDescriptor &fileFd);
    //备份驱动（fd 方式：dirFd 为前端打开的暂存目录描述符，debname 仅作备份目录名）
    bool backupDebFd(const QDBusUnixFileDescriptor &dirFd, const QString &debname);
    //安装用户选择的驱动文件（fd 方式：内容桥接到私有目录后复用 installDriver 流程）
    bool installDriverFd(const QDBusUnixFileDescriptor &fileFd, const QString &filename);
    bool delDeb(const QString &debname);
    bool aptUpdate();

private:
    void initConnections();
    bool unInstallModule(const QString &moduleName, QString &msg);

    bool isNetworkOnline();

signals:
    void sigProgressDetail(int progress, const QString &strDeatils);
    void sigFinished(bool bsuccess, QString msg);
    void sigDebInstall(const QString &package);
    void sigDebUnstall(const QString &package);

    void sigDownloadProgressChanged(QStringList msg);//驱动下载进度、速度、已下载大小
    void sigDownloadFinished();//下载完成
    void sigInstallProgressChanged(int progress);//安装进度
    void sigInstallProgressFinished(bool bsuccess, int err);
    void sigBackupProgressFinished(bool bsuccess);

private:
    ModCore *mp_modcore = nullptr;
    AptInstaller *mp_debinstaller = nullptr;
    QThread *mp_deboperatethread = nullptr;
    DriverInstallerApt *mp_driverInstaller = nullptr;
    QThread *mp_driverOperateThread = nullptr;

    int m_installprocess = 0;
    QString errmsg;
    QString m_fdBridgePath;   // fd 安装时桥接到守护进程私有目录的临时文件
    bool m_IsNetworkOnline = true;
    bool m_StopQueryNetwork = false;

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
#endif // DISABLE_DRIVER
