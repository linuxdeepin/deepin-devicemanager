// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISABLE_DRIVER

#include "DriverInstaller.h"
#include "Utils.h"
#include "commonfunction.h"

#include <QApt/Backend>
#include <QApt/DebFile>
#include <QApt/Transaction>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QFile>
#include <QFileInfo>

#include <unistd.h>
#include <fstream>
#include <string>
#include <vector>

const QString DriverInstaller::DEVICE_REPO_PATH = "/etc/apt/sources.list.d/devicemanager.list";
const QString DriverInstaller::DRIVER_REPO_PATH = "/etc/apt/sources.list.d/driver.list";
const int MAX_DPKGRUNING_TEST = 20;
const int TEST_TIME_INTERVAL = 2000;

DriverInstaller::DriverInstaller(QObject *parent)
    : QObject(parent)
    , mp_Backend(nullptr)
    , mp_Trans(nullptr)
    , m_iRuningTestCount(0)
    , m_Cancel(false)
{

}

bool DriverInstaller::initBackend()
{
    if (nullptr == mp_Backend)
        mp_Backend = new QApt::Backend;
    aptClean();
    return mp_Backend->init();
}

void DriverInstaller::installPackage(const QString &package, const QString &version)
{
    //检查dpkg是否正在运行，如果正在运行等待2s重试,最多尝试20次
    if (Utils::isDpkgLocked()) {
        if (m_iRuningTestCount < MAX_DPKGRUNING_TEST) {
            QTimer::singleShot(TEST_TIME_INTERVAL, this, [&] {
                installPackage(package, version);
                m_iRuningTestCount++;
            });
            return;
        } else {
            emit errorOccurred(EC_NULL);
            emit installProgressFinished(false);
        }
    }

    m_Cancel = false;
    doOperate(package, version);
    m_iRuningTestCount = 0;
}

void DriverInstaller::undoInstallDriver()
{
    if (nullptr != mp_Trans) {
        mp_Trans->setProperty("isCancellable", true);
        mp_Trans->setProperty("isCancelled", true);
        mp_Trans->cancel();
        m_Cancel = true;
        qInfo() << "DRIVER_LOG **************************** 取消操作";
    }

}

void DriverInstaller::aptClean()
{
    QProcess process;
    process.start("sh", QStringList() << "-c" << QString("/usr/bin/lastore-apt-clean"));//调用商店后端lastore中的接口
    process.waitForFinished();
}

bool DriverInstaller::isNetworkOnline(uint usec)
{
    /*
       -c 2（代表ping次数，ping 2次后结束ping操作） -w 2（代表超时时间，2秒后结束ping操作）
    */
    // example: ping www.baidu.com -c 2 -w 2 >netlog.bat
    QProcess process;
    process.setStandardOutputFile("netlog.bat", QIODevice::WriteOnly);
    process.start("ping", QStringList() << "www.baidu.com" << "-c" << "2" << "-w" << "2");
    process.waitForFinished(-1);
    bool bRet = (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0);
    if (!bRet) {
        return false;
    }
    usleep(usec);

    //把文件一行一行读取放入vector
    std::ifstream infile;
    infile.open("netlog.bat");
    std::string s;
    std::vector<std::string> v;
    while (infile) {
        getline(infile, s);
        if (infile.fail())
            break;
        v.push_back(s);
    }
    infile.close();

    //读取倒数第二行 2 packets transmitted, 2 received, 0% packet loss, time 1001ms
    if (v.size() > 1) {
        std::string data = v[v.size() - 2];
        int iPos = data.find("received,");
        if (iPos != -1) {
            data = data.substr(iPos + 10, 3); //截取字符串返回packet loss
            int  n = atoi(data.c_str());
            if (n == 0)
                return 1;
            else
                return 0 ;
        } else {
            return 0;
        }

    } else {
        return 0;
    }
}

void DriverInstaller::doOperate(const QString &package, const QString &version)
{
    // 按需创建驱动仓库源文件，仅在 driver.list 中不存在 pro-driver-packages 时创建
    ensureDriverRepoSource();

    if (!initBackend()) {
        emit errorOccurred(EC_NULL);
        qInfo() << "DRIVER_LOG : ************************** 初始化backend失败";
        cleanupTempSource();
        return;
    }

    QApt::Package *p = mp_Backend->package(package);
    // 判断包是否存在
    if (nullptr == p) {
        emit errorOccurred(EC_NOTFOUND);
        qInfo() << "DRIVER_LOG : ************************** 安装包不存在";
        cleanupTempSource();
        return;
    }

    // 版本不存在
    if (!p->setVersion(version)) {
        qInfo() << "DRIVER_LOG : ************************** 安装包版本不存在";
        emit errorOccurred(EC_NOTFOUND);
        delete p;
        p = nullptr;
        cleanupTempSource();
        return;
    }

    QApt::PackageList lst;
    lst.append(p);

    mp_Trans = mp_Backend->installPackages(lst);
    if (nullptr == mp_Trans) {
        emit errorOccurred(EC_NULL);
        qInfo() << "DRIVER_LOG : ************************** installPackages";
        cleanupTempSource();
        return;
    }

    qRegisterMetaType<QApt::ExitStatus>("QApt::ExitStatus");
    connect(mp_Trans, &QApt::Transaction::finished, this, [this, package](QApt::ExitStatus status) {
        QApt::ErrorCode code = mp_Trans->error();

        if (QApt::ExitSuccess == status) { // 退出状态成功时
            if (QApt::Success == code) {
                emit installProgressFinished(true);
            }
        } else if (QApt::ExitFailed == status) { // 退出状态失败时
            // 英伟达驱动 + CommitError == 成功
            if (package.contains("nvidia-driver") && QApt::CommitError == code) {
                emit installProgressFinished(true);
            }

            // FetchError == 网络异常
            if (QApt::FetchError == code || (QApt::CommitError == code && !isNetworkOnline())) {
                emit errorOccurred(EC_NETWORK);
            }
        } else if (QApt::ExitCancelled == status) { // 退出状态取消时
            emit errorOccurred(EC_CANCEL);
        }

        qInfo() << "DRIVER_LOG : ************************** 安装结束 结束状态码" << status;
        qInfo() << "DRIVER_LOG : ************************** 安装结束 结束错误码" << code;

        // 清理本次创建的临时源文件
        cleanupTempSource();

        mp_Trans->disconnect();
        mp_Trans->deleteLater();
        mp_Trans = nullptr;
    });

    connect(mp_Trans, &QApt::Transaction::downloadProgressChanged, this, [this](QApt::DownloadProgress dp) {
        qInfo() << "DRIVER_LOG : ************************** 下载进度 " << dp.progress();
        if (m_Cancel) {
            mp_Trans->setProperty("isCancellable", true);
            mp_Trans->setProperty("isCancelled", true);
            mp_Trans->cancel();
            qInfo() << "DRIVER_LOG *************downloadProgressChanged*************** 取消操作";
        }
    });

    connect(mp_Trans, &QApt::Transaction::progressChanged, this, [this](int progress) {
        if (m_Cancel) {
            mp_Trans->setProperty("isCancellable", true);
            mp_Trans->setProperty("isCancelled", true);
            mp_Trans->cancel();
            qInfo() << "DRIVER_LOG *************progressChanged*************** 取消操作";
        }
        if (false == m_Cancel)
            emit installProgressChanged(progress);
        qInfo() << "DRIVER_LOG : ************************** 总进度 " << progress << "  下载状态 " << mp_Trans->downloadProgress().uri();
    });

    mp_Trans->run();
}

void DriverInstaller::ensureDriverRepoSource()
{
    // 重置临时源标记，避免因前一次安装失败导致残留标记
    m_tempSourceCreated = false;

    // 优先检查系统自带的 driver.list 中是否已包含驱动仓库源
    QFile fileDriver(DRIVER_REPO_PATH);
    if (fileDriver.open(QIODevice::ReadOnly)) {
        QString info = fileDriver.readAll();
        QStringList lines = info.split("\n");
        foreach (QString line, lines) {
            if (line.contains("pro-driver-packages")) {
                fileDriver.close();
                return;
            }
        }
        fileDriver.close();
    }

    // 使用 O_EXCL 原子创建文件，避免 TOCTOU 竞争条件（CWE-367）
    // NewOnly 保证文件不存在时才创建，若已存在则 open 失败，无需先检查
    QFile file(DEVICE_REPO_PATH);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QFile::NewOnly)) {
        // 文件已存在属于正常情况（第三方创建），静默返回
        if (!QFile::exists(DEVICE_REPO_PATH)) {
            qInfo() << "DRIVER_LOG : 创建驱动源文件失败:" << file.errorString();
        }
        return;
    }

    file.write("deb https://pro-driver-packages.uniontech.com eagle non-free\n");
    file.close();
    m_tempSourceCreated = true;
    qInfo() << "DRIVER_LOG : 创建临时驱动源文件:" << DEVICE_REPO_PATH;

    // 新源创建后需执行 apt update 更新包索引，
    // 否则 QApt::Backend 无法识别该源的软件包
    // 使用列表传参绕过 shell 解析，避免命令注入风险
    QProcess process;
    process.start("apt", QStringList() << "update");
    process.waitForFinished(50000);
    qInfo() << "DRIVER_LOG : 临时源 apt update 完成";
}

void DriverInstaller::cleanupTempSource()
{
    if (!m_tempSourceCreated) {
        return;
    }

    QFile::remove(DEVICE_REPO_PATH);
    m_tempSourceCreated = false;
    qWarning() << "DRIVER_LOG : 清理临时驱动源文件:" << DEVICE_REPO_PATH;
}

#endif // DISABLE_DRIVER
