#include "DriverInstaller.h"
#include "Utils.h"

#include <QDebug>
#include <QTimer>
#include <QProcess>

#include <QApt/Backend>
#include <QApt/DebFile>
#include <QApt/Package>
#include <QApt/Transaction>

const int MAX_DPKGRUNING_TEST = 20;
const int TEST_TIME_INTERVAL = 2000;


DriverInstaller::DriverInstaller(QObject *parent): QObject(parent)
{

    qRegisterMetaType<QApt::ExitStatus>("QApt::ExitStatus");
    initBackend();
}

void DriverInstaller::doOperate(const QString &package, const QString &version, bool binstall)
{
    if (binstall) {
        QApt::PackageList packages;
        QApt::Package *p = m_backend->package(package);//"htsy-prn-pclcustom-drv"
        if (!p) {
            //包名不存在
            qInfo() << "package : \"" << package << "\"" << "was not founded !";
            emit this->errorOccurred(EC_NOTFOUND);//查看源码，只有授权错误和apt自身错误
            return;
        }

        if (!p->setVersion(version)){
            qInfo() << "version : \"" << version << "\"" << "was not founded !";
            emit this->errorOccurred(EC_NOTFOUND);
            return;
        }

        packages.append(p);
        m_pTrans = m_backend->installPackages(packages);//QApt::Transaction *installPackages(QApt::PackageList packages);
    }

    if (nullptr == m_pTrans) {
        emit errorOccurred(EC_NULL);
        return ;
    }

    connect(m_pTrans, &QApt::Transaction::finished, this, [ = ](QApt::ExitStatus status) {
        Q_UNUSED(status)
        if (m_pTrans) {
            QApt::DownloadProgress dp = this->m_pTrans->downloadProgress();

            // 通过判断文件本身大小和已经下载的大小是否一直判断是否取消
            quint64 file_size = dp.fileSize();
            quint64 fetched_size = dp.fetchedSize();
            if (m_pTrans->property("isCancelled").toBool() && file_size > fetched_size) {
                emit this->errorOccurred(EC_CANCEL);
            } else {
                emit this->installProgressFinished(QApt::Success == m_pTrans->error());
            }

            m_pTrans->disconnect(this);
            m_pTrans->deleteLater();
        }
        reset();//结束重置
        doAptClean();
    });

    qRegisterMetaType<QApt::TransactionStatus>("QApt::TransactionStatus");
    connect(m_pTrans, &QApt::Transaction::statusChanged, this, &DriverInstaller::slotStatusChanged);
    connect(m_pTrans, &QApt::Transaction::progressChanged, this, &DriverInstaller::slotProgressChanged);
    qRegisterMetaType<QApt::ErrorCode>("QApt::ErrorCode");
    connect(m_pTrans, &QApt::Transaction::errorOccurred, this, [ = ](QApt::ErrorCode error) {
        m_pTrans->disconnect(this);
        m_pTrans->deleteLater();
        if(5 == error)
            emit this->errorOccurred(EC_NETWORK);
        else if(6 == error)
            emit this->errorOccurred(EC_NOTFOUND); //没有指定版本的驱动包
        else
            emit this->errorOccurred(EC_NULL);
    });

    m_pTrans->run();
}

void DriverInstaller::doAptClean()
{
    QProcess process;
    process.start("sh", QStringList() << "-c" << QString("/usr/bin/lastore-apt-clean"));//调用商店后端lastore中的接口
    process.waitForFinished();
}

bool DriverInstaller::downloadInfo(QStringList &lstInfo, int total_progress)
{
    QApt::DownloadProgress dp = this->m_pTrans->downloadProgress();
    quint64 fileSize = dp.fileSize();
    int progress = dp.progress();
    double fetchedSize  = dp.fetchedSize();
    double downloadSpeed = m_pTrans->downloadSpeed();

    if (0 == fileSize && 100 == progress) {
        return false;
    }

    static qint64 time_ms = QDateTime::currentSecsSinceEpoch();
    if (total_progress <= 2) {
        time_ms = QDateTime::currentSecsSinceEpoch();
    }

    // 进度百分比
    lstInfo.append(QString::number(progress));

    // 已经下载的文件大小
    if (fetchedSize < 1024 * 1024) {
        lstInfo.append(QString::number(fetchedSize / 1024, 'f', 2) + "KB");
    } else if (fetchedSize < 1024 * 1024 * 1024) {
        lstInfo.append(QString::number(fetchedSize / 1024 / 1024, 'f', 2) + "MB");
    } else {
        lstInfo.append(QString::number(fetchedSize / 1024 / 1024 / 1024, 'f', 2) + "GB");
    }


    // 如果下载速度为0，则根据下载的大小计算
    if (downloadSpeed < 0.1) {
        qint64 time_cur = QDateTime::currentSecsSinceEpoch();
        qint64 detal = (time_cur - time_ms < 1) ? 1 : time_cur - time_ms;
        downloadSpeed = fetchedSize / detal;
    }

    // 当前下载速度
    if (downloadSpeed < 1024 * 1024) {
        lstInfo.append(QString::number(downloadSpeed / 1024, 'f', 2) + "KB");
    } else if (downloadSpeed < 1024 * 1024 * 1024) {
        lstInfo.append(QString::number(downloadSpeed / 1024 / 1024, 'f', 2) + "MB");
    } else {
        lstInfo.append(QString::number(downloadSpeed / 1024 / 1024 / 1024, 'f', 2) + "GB");
    }

    return true;
}

void DriverInstaller::reset()
{
    m_bValid = m_backend->reloadCache();
    if (!m_bValid) {
        qInfo() << "Error in \"DriverInstaller::reset()\" : " << m_backend->initErrorMessage();
    }
    m_iRuningTestCount = 0;
    m_pTrans = nullptr;
}

void DriverInstaller::initBackend()
{
    m_backend = new QApt::Backend;
    m_bValid = m_backend->init();
    if (!m_bValid) {
        qInfo() << "Error in \"DriverInstaller::initBackend()\" : " << m_backend->initErrorMessage();
        return;
    }
    m_backend->updateCache();
}
/**
 * @brief DriverInstaller::installPackage deb包安装
 * @param filepath 包文件路径
 */
void DriverInstaller::installPackage(const QString &filename, const QString &version)
{
    //检查dpkg是否正在运行，如果正在运行等待2s重试,最多尝试20次
    if (Utils::isDpkgLocked()) {
        if (m_iRuningTestCount < MAX_DPKGRUNING_TEST) {
            QTimer::singleShot(TEST_TIME_INTERVAL, this, [&] {
                installPackage(filename, version);
                m_iRuningTestCount++;
            });
            return;
        } else {
            emit errorOccurred(EC_NULL);
            emit installProgressFinished(false);
        }
    }
    if (!m_bValid) {
        return;
    }
    doOperate(filename, version);
}

void DriverInstaller::undoInstallDriver()
{
    if (m_pTrans) {
        m_pTrans->setProperty("isCancellable", true);
        m_pTrans->setProperty("isCancelled", true);
        m_pTrans->cancel();
        qInfo() << "Successfully canceled the current driver installation : " << m_pTrans->packages();
    }
}

void DriverInstaller::slotStatusChanged(QApt::TransactionStatus status)
{
    if (status == QApt::TransactionStatus::CommittingStatus) {
        emit downloadFinished();
    }
}

void DriverInstaller::slotDownloadStatusChanged(QApt::ExitStatus status)
{
    Q_UNUSED(status)
}

void DriverInstaller::slotProgressChanged(int progress)
{
    //这里的progress包含下载、安装2个过程，下载是0-50，安装是51-100。
    if (progress <= 50) {
        QStringList strStatus;
        if (downloadInfo(strStatus, progress))
            emit this->downloadProgressChanged(strStatus);
    } else {
        int iProcess = (progress - 50) * 2;
        if(iProcess < 100){
            emit installProgressChanged(iProcess);
        }
    }
}
