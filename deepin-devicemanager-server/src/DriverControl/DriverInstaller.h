/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Libang.Liu <liulibang@uniontech.com>
*
* Maintainer: Libang.Liu <liulibang@uniontech.com>
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

#ifndef DRIVERINSTALLER_H
#define DRIVERINSTALLER_H
#include "commonfunction.h"

#include <QObject>
#include <QApt/Globals>
#include <QApt/Backend>
#include <QApt/Transaction>

class DriverInstaller: public QObject
{
    Q_OBJECT
public:
    explicit DriverInstaller(QObject *parent = nullptr);

private:
    void initBackend();
    void reset();
    void doOperate(const QString &package, const QString &version, bool binstall = true);
    void doAptClean();//清理apt缓存
    bool downloadInfo(QStringList &lstInfo, int total_progress);
public slots:
    void installPackage(const QString &filename, const QString &version);
    void undoInstallDriver();

    void slotStatusChanged(QApt::TransactionStatus status);
    void slotDownloadStatusChanged(QApt::ExitStatus status);
    void slotProgressChanged(int progress);
signals:
    void installFinished(bool bsuccess);
    void errorOccurred(int error);
    void progressChanged(int progress);

    void downloadProgressChanged(QStringList msg);//驱动下载进度、速度、已下载大小
    void downloadFinished();//下载完成
    void installProgressChanged(int progress);//安装进度
    void installProgressFinished(bool bsuccess);

private:
    QApt::Backend *m_backend = nullptr;
    QApt::Transaction *m_pTrans = nullptr;
    bool m_bValid = false;
    int m_iRuningTestCount = 0;

};

#endif // DRIVERINSTALLER_H
