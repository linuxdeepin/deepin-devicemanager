// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverBackupThread.h"
#include "DBusDriverInterface.h"
#include "DDLog.h"

#include <QDir>
#include <QProcess>
#include <QLoggingCategory>
#include <QTemporaryDir>

static bool updateFlag = false;

using namespace DDLog;

DriverBackupThread::DriverBackupThread(QObject *parent)
    : QThread(parent)
    , mp_driverInfo(nullptr)
{
    qCDebug(appLog) << "DriverBackupThread constructor";
}

void DriverBackupThread::run()
{
    qCDebug(appLog) << "Driver backup thread started";
    if (!m_isStop && mp_driverInfo) {
        qCDebug(appLog) << "Backup process started for driver:" << mp_driverInfo->name();
        QString debname = mp_driverInfo->packages();
        QString debversion = mp_driverInfo->debVersion();
        if (debname.isEmpty()  && debversion.isEmpty()) {
            qCWarning(appLog) << "Empty package name or version for driver backup";
            emit backupProgressFinished(false);
            return;
        }

        QTemporaryDir tempDir(QDir::tempPath() + "/XXXXXX");
        QString backupPath =  QString("%1/driver/%2").arg(tempDir.path()).arg(debname);
        QDir destdir(backupPath);
        if (!destdir.exists()) {
            qCDebug(appLog) << "Backup destination directory does not exist, creating:" << backupPath;
            if (!destdir.mkpath(destdir.absolutePath()))
                qCInfo(appLog) << "mkpath backupDeb unsucess  :" << backupPath;
        }

        if (m_isStop) {
            qCDebug(appLog) << "Backup stopped before apt update";
            emit backupProgressFinished(false);
            return;
        }

        if(!updateFlag) {
            qCDebug(appLog) << "Updating apt...";
            bool ret = DBusDriverInterface::getInstance()->aptUpdate();
            updateFlag = true;
            if (!ret) {
                emit backupProgressFinished(false);
                qCInfo(appLog) << "apt update failed.";
                return;
            }
            qCDebug(appLog) << "Apt update finished.";
        }

        QStringList options;
        options << "download" << debname + "=" + debversion;
        QProcess process;
        process.setWorkingDirectory(backupPath);
        connect(&process, &QProcess::readyReadStandardOutput, this, [&](){
            qCDebug(appLog) << "QProcess readyReadStandardOutput";
            QByteArray outArry = process.readAllStandardOutput();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QList<QString> lines = QString(outArry).split('\n', QString::SkipEmptyParts);
#else
            QList<QString> lines = QString(outArry).split('\n', Qt::SkipEmptyParts);
#endif
            for (const QString &line : qAsConst(lines)) {
                if (line.contains("无法解析域名")) {
                    qCInfo (appLog) << "network error: " << line;
                    m_isStop = true;
                    emit backupProgressFinished(false);
                }
            }
        });

        if (m_isStop) {
            qCDebug(appLog) << "Backup stopped before process start";
            emit backupProgressFinished(false);
            return;
        }
        qCDebug(appLog) << "Starting process: apt" << options;
        process.start("apt", options);
        process.waitForFinished(-1);
        qCDebug(appLog) << "Process finished";

        if (m_isStop) {
            qCDebug(appLog) << "Backup stopped after process finished";
            emit backupProgressFinished(false);
            return;
        }

        bool flag = 0;
        if (destdir.exists()) {
            qCDebug(appLog) << "Checking for downloaded .deb file in" << destdir.path();
            //获取当前路径下的所有文件名
            QFileInfoList fileInfoList = destdir.entryInfoList();
            foreach (QFileInfo fileInfo, fileInfoList) {
                if (m_isStop) {
                    qCDebug(appLog) << "Backup stopped while iterating files";
                    emit backupProgressFinished(false);
                    return;
                }

                if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                    continue;

                if (fileInfo.isFile() && fileInfo.fileName().contains(".deb") && fileInfo.fileName().contains(debname)) {
                    qCDebug(appLog) << "Found .deb file:" << fileInfo.fileName();
                    DBusDriverInterface::getInstance()->backupDeb(backupPath);

                    while (m_status == Waiting) {
                        qCDebug(appLog) << "Waiting for backup status...";
                        msleep(500);
                    }

                    destdir.remove(fileInfo.fileName());
                    if (m_status == Success) {
                        qCDebug(appLog) << "Backup success";
                        emit backupProgressFinished(true);
                        return;
                    } else if (m_status == Failed) {
                        qCDebug(appLog) << "Backup failed";
                        emit backupProgressFinished(false);
                        return;
                    }

                    flag = 1;
                }
            }
        }
        if (!flag) {
            emit backupProgressFinished(false);
            qCDebug(appLog) << __LINE__ << " backup failed.";
        }
    } else {
        qCDebug(appLog) << "Backup thread run condition not met (m_isStop:" << m_isStop << ", mp_driverInfo:" << mp_driverInfo << ")";
    }
}

void DriverBackupThread::setBackupDriverInfo(DriverInfo *info)
{
    qCDebug(appLog) << "Set backup driver info:" << (info ? info->name() : "null");
    m_isStop = false;
    mp_driverInfo = info;
    m_status = Waiting;
}

void DriverBackupThread::undoBackup()
{
    qCDebug(appLog) << "Backup operation cancelled";
    m_isStop = true;
}

void DriverBackupThread::setStatus(BackupStatus status){
    qCDebug(appLog) << "Backup status changed to:" << status;
    m_status = status;
}
