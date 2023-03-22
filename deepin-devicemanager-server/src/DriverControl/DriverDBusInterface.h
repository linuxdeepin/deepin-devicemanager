// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISABLE_DRIVER

#ifndef DRIVERDBUSINTERFACE_H
#define DRIVERDBUSINTERFACE_H

#include "commonfunction.h"

#include <QObject>
#include <QDBusContext>

class DriverManager;
class ModCore;
class DriverDBusInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.drivermanager")
public:
    explicit DriverDBusInterface(QObject *parent = nullptr);

private:
    void initConnects();
    bool getUserAuthorPasswd(void);

signals:
    Q_SCRIPTABLE void sigProgressDetail(int progress, const QString &strDeatils);
    Q_SCRIPTABLE void sigFinished(bool bsuccess, QString msg);

    Q_SCRIPTABLE void sigDownloadProgressChanged(QStringList msg);//驱动下载进度、速度、已下载大小
    Q_SCRIPTABLE void sigDownloadFinished();//下载完成
    Q_SCRIPTABLE void sigInstallProgressChanged(int progress);//安装进度
    Q_SCRIPTABLE void sigInstallProgressFinished(bool bsuccess, int err);

public slots:
    Q_SCRIPTABLE bool unInstallDriver(const QString &modulename);
    Q_SCRIPTABLE bool installDriver(const QString &filepath);
    Q_SCRIPTABLE void installDriver(const QString &modulename, const QString &version);
    Q_SCRIPTABLE void undoInstallDriver();
    Q_SCRIPTABLE QStringList checkModuleInUsed(const QString &modulename);
    Q_SCRIPTABLE bool isDriverPackage(const QString &filepath);
    Q_SCRIPTABLE bool isBlackListed(const QString &modName);
    Q_SCRIPTABLE bool isArchMatched(const QString &filePath);
    Q_SCRIPTABLE bool isDebValid(const QString &filePath);
    Q_SCRIPTABLE bool unInstallPrinter(const QString &vendor, const QString &model);

private:
    DriverManager *mp_drivermanager;
    ModCore *pcore = nullptr; //临时测试使用
};

#endif // DRIVERDBUSINTERFACE_H
#endif // DISABLE_DRIVER
