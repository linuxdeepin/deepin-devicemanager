// SPDX-FileCopyrightText: 2019 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aptinstaller.h"
#include "utils.h"
#include "DDLog.h"

#include <QTimer>
#include <QRegularExpression>
#include <QFileInfo>

using namespace DDLog;

AptInstaller::AptInstaller(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_bValid(true)
    , m_iRuningTestCount(0)
{
    qCDebug(appLog) << "AptInstaller initialized";

    // 连接进程信号
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
        parseAptOutput(output);
    });
    
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        bool success = (exitCode == 0 && exitStatus == QProcess::NormalExit);
        emit installFinished(success);
        if (!success) {
            emit errorOccurred(QString("Process failed with exit code: %1").arg(exitCode));
        }
    });
}

bool AptInstaller::isValid()
{
    return m_bValid;
}

bool AptInstaller::isArchMatched(const QString &path)
{
    qCDebug(appLog) << "Checking architecture match for:" << path;

    QProcess process;
    process.start("dpkg", {"-I", path});
    process.waitForFinished();
    
    QString output = process.readAllStandardOutput();
    QString systemArch = QSysInfo::currentCpuArchitecture();
    
    // 检查包的架构信息
    QRegularExpression re("Architecture:\\s*(\\w+)");
    auto match = re.match(output);
    if (match.hasMatch()) {
        QString pkgArch = match.captured(1);
        return (pkgArch == "all" || pkgArch == systemArch);
    }
    return false;
}

bool AptInstaller::isDebValid(const QString &path)
{
    qCDebug(appLog) << "Validating deb package:" << path;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !path.endsWith(".deb")) {
        return false;
    }
    
    QProcess process;
    process.start("dpkg", {"-I", path});
    process.waitForFinished();
    
    return process.exitCode() == 0;
}

void AptInstaller::doOperate(const QString &package, bool isInstall)
{
    qCDebug(appLog) << "Operating on package:" << package << "install:" << isInstall;

    QStringList args;
    if (isInstall) {
        args << "install" << "-y" << package;
    } else {
        args << "remove" << "-y" << package;
    }
    
    m_process->start("apt", args);
}

void AptInstaller::installPackage(const QString &filepath)
{
    qCDebug(appLog) << "Installing package from:" << filepath;

    if (Utils::isDpkgLocked()) {
        if (m_iRuningTestCount < MAX_DPKGRUNING_TEST) {
            QTimer::singleShot(TEST_TIME_INTERVAL, this, [this, filepath]() {
                installPackage(filepath);
                m_iRuningTestCount++;
            });
            return;
        } else {
            emit errorOccurred("Dpkg locked, time out");
            emit installFinished(false);
            return;
        }
    }
    
    if (!m_bValid) {
        return;
    }
    
    doOperate(filepath, true);
}

void AptInstaller::uninstallPackage(const QString &packagename)
{
    qCDebug(appLog) << "Uninstalling package:" << packagename;

    if (Utils::isFileLocked("/var/lib/dpkg/lock")) {
        if (m_iRuningTestCount < MAX_DPKGRUNING_TEST) {
            QTimer::singleShot(TEST_TIME_INTERVAL, this, [this, packagename]() {
                uninstallPackage(packagename);
                m_iRuningTestCount++;
            });
            return;
        } else {
            emit errorOccurred("Dpkg locked, time out");
            emit installFinished(false);
            return;
        }
    }
    
    doOperate(packagename, false);
}

void AptInstaller::parseAptOutput(const QString &output)
{
    qCDebug(appLog) << "Parsing apt output";

    // 解析apt输出以更新进度
    static QRegularExpression progressRe("\\b(\\d+)%");
    auto match = progressRe.match(output);
    if (match.hasMatch()) {
        int progress = match.captured(1).toInt();
        emit progressChanged(progress);
    }
} 