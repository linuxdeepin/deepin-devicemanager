// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverBackupThread.h"
#include "DBusDriverInterface.h"

#include <QDir>
#include <QProcess>
#include <QDebug>

// 备份临时路径
#define DB_PATH_TMP "/tmp/deepin-devicemanager"

DriverBackupThread::DriverBackupThread(QObject *parent)
    : QThread(parent)
    , mp_driverInfo(nullptr)
{

}

void DriverBackupThread::run()
{
    if (!m_isStop && mp_driverInfo) {
        QString debname = mp_driverInfo->packages();
        QString debversion = mp_driverInfo->debVersion();
        if (debname.isEmpty()  && debversion.isEmpty()) {
            emit backupProgressFinished(false);
            return;
        }

        QString backupPath =  QString("%1/driver/%2").arg(DB_PATH_TMP).arg(debname);
        QDir destdir(backupPath);
        if (!destdir.exists()) {
            if (!destdir.mkpath(destdir.absolutePath()))
                qInfo() << "mkpath backupDeb unsucess  :" << backupPath;
        }

        QStringList options;
        options << "-c" << "apt download " + debname + "=" + debversion;
        QProcess process;
        process.setWorkingDirectory(backupPath);
        connect(&process, &QProcess::readyReadStandardOutput, this, [&](){
            QByteArray outArry = process.readAllStandardOutput();
            QList<QString> lines = QString(outArry).split('\n', QString::SkipEmptyParts);
            for (const QString &line : qAsConst(lines)) {
                if (line.contains("无法解析域名")) {
                    qInfo () << "network error: " << line;
                    m_isStop = true;
                    emit backupProgressFinished(false);
                }
            }
        });
        process.start("/bin/bash", options);
        process.waitForFinished(-1);

        if (m_isStop) {
            emit backupProgressFinished(false);
            return;
        }

        //emit backupProgressChanged(50);

        if (destdir.exists()) {
            //获取当前路径下的所有文件名
            QFileInfoList fileInfoList = destdir.entryInfoList();
            foreach (QFileInfo fileInfo, fileInfoList) {
                if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                    continue;
                if (fileInfo.isFile() && fileInfo.fileName().contains(".deb") && fileInfo.fileName().contains(debname)) {
                    bool ret = DBusDriverInterface::getInstance()->backupDeb(backupPath);
                    qInfo() << "backupDeb: "  << ret << backupPath << fileInfo.fileName();
                    destdir.remove(fileInfo.fileName());
                    emit backupProgressFinished(ret);
                }
            }
        }
    }
}

void DriverBackupThread::setBackupDriverInfo(DriverInfo *info)
{
    m_isStop = false;
    mp_driverInfo = info;
}

void DriverBackupThread::undoBackup()
{
    m_isStop = true;
}
