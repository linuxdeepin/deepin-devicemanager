// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISABLE_DRIVER

#include "debinstaller.h"
#include "utils.h"
#include "DDLog.h"

#include <QLoggingCategory>
#include <QTimer>

#include <QApt/Backend>
#include <QApt/DebFile>
#include <QApt/Transaction>

using namespace DDLog;

const int MAX_DPKGRUNING_TEST = 20;
const int TEST_TIME_INTERVAL = 2000;

DebInstaller::DebInstaller(QObject *parent): QObject(parent)
{
    qCDebug(appLog) << "DebInstaller initialized";

    initBackend();
}

bool DebInstaller::isValid()
{
    return  m_bValid;
}

bool DebInstaller::isArchMatched(const QString &path)
{
    qCDebug(appLog) << "Checking architecture match for:" << path;

    QApt::DebFile deb(path);
    if (!deb.isValid()) {
        return false;
    }
    const QString arch = deb.architecture();

    if ("all" == arch || "any" == arch)
        return false;

    bool architectures = m_backend->architectures().contains(deb.architecture());
    if (!architectures)
        return false;
    return true;
}

bool DebInstaller::isDebValid(const QString &path)
{
    qCDebug(appLog) << "Validating deb package:" << path;

    QApt::DebFile deb(path);
    return deb.isValid();
}

void DebInstaller::initBackend()
{
    qCDebug(appLog) << "Initializing QApt backend";

    m_backend = new QApt::Backend;
    m_bValid = m_backend->init();
    if (!m_bValid) {
        qCInfo(appLog) << m_backend->initErrorMessage();
    }
}

void DebInstaller::reset()
{
    qCDebug(appLog) << "Resetting DebInstaller";

    m_bValid = m_backend->reloadCache();
    if (!m_bValid) {
        qCInfo(appLog) << m_backend->initErrorMessage();
    }
    m_iRuningTestCount = 0;
    m_pTrans = nullptr;
}

void DebInstaller::doOperate(const QString &package, bool binstall)
{
    qCDebug(appLog) << "Operating on package:" << package << "install:" << binstall;

    if (binstall) {
        QApt::DebFile deb(package);
        if (!deb.isValid()) {
            emit this->errorOccurred("Deb is invalid");
            return ;
        }
        m_pTrans = m_backend->installFile(deb);
    } else {
        m_backend->markPackageForRemoval(package);
        m_pTrans = m_backend->commitChanges();
    }

    if (nullptr == m_pTrans) {
        emit this->errorOccurred("Get transaction failed!");
        return ;
    }
    connect(m_pTrans, &QApt::Transaction::finished, this, [ = ](QApt::ExitStatus) {
        if (m_pTrans) {
            emit this->installFinished(QApt::Success == m_pTrans->error());
            m_pTrans->disconnect(this);
            m_pTrans->deleteLater();
        }
        reset();//结束重置
    });
    connect(m_pTrans, &QApt::Transaction::progressChanged, this, &DebInstaller::progressChanged);
    connect(m_pTrans, &QApt::Transaction::errorOccurred, this, [ = ](QApt::ErrorCode error) {
        emit this->errorOccurred(QString("%1").arg(error));
    });

    m_pTrans->run();
}

/**
 * @brief DebInstaller::installPackage deb包安装
 * @param filepath 包文件路径
 */
void DebInstaller::installPackage(const QString &filepath)
{
    qCDebug(appLog) << "Installing package from:" << filepath;

    //检查dpkg是否正在运行，如果正在运行等待2s重试,最多尝试20次
    if (Utils::isDpkgLocked()) {
        if (m_iRuningTestCount < MAX_DPKGRUNING_TEST) {
            QTimer::singleShot(TEST_TIME_INTERVAL, this, [&] {
                installPackage(filepath);
                m_iRuningTestCount++;
            });
            return;
        } else {
            emit errorOccurred("Dpkg locked ,time out");
            emit installFinished(false);
        }
    }
    if (!m_bValid) {
        return;
    }
    doOperate(filepath, true);
}

void DebInstaller::uninstallPackage(const QString &packagename)
{
    qCDebug(appLog) << "Uninstalling package:" << packagename;

    //检查dpkg是否正在运行，如果正在运行等待2s重试,最多尝试20次
    if (Utils::isFileLocked("/var/lib/dpkg/lock")) {
        if (m_iRuningTestCount < MAX_DPKGRUNING_TEST) {
            QTimer::singleShot(TEST_TIME_INTERVAL, this, [&] {
                uninstallPackage(packagename);
                m_iRuningTestCount++;
            });
            return;
        } else {
            emit errorOccurred("Dpkg locked ,time out");
            emit installFinished(false);
        }
    }
    doOperate(packagename, false);
}
#endif // DISABLE_DRIVER
