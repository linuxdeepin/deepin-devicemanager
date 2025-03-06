// SPDX-FileCopyrightText: 2019 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "driverinstallerapt.h"
#include "utils.h"
#include "DDLog.h"

#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <unistd.h>
#include <fstream>

using namespace DDLog;

const int MAX_DPKGRUNING_TEST = 20;
const int TEST_TIME_INTERVAL = 2000;

DriverInstallerApt::DriverInstallerApt(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_iRuningTestCount(0)
    , m_Cancel(false)
{
}

DriverInstallerApt::~DriverInstallerApt()
{
    if (m_process) {
        m_process->kill();
        delete m_process;
        m_process = nullptr;
    }
}

void DriverInstallerApt::installPackage(const QString &package, const QString &version)
{
    // 检查dpkg是否正在运行
    if (Utils::isDpkgLocked()) {
        if (m_iRuningTestCount < MAX_DPKGRUNING_TEST) {
            QTimer::singleShot(TEST_TIME_INTERVAL, this, [this, package, version] {
                m_iRuningTestCount++;
                installPackage(package, version);
            });
            return;
        } else {
            emit errorOccurred(0);
            emit installProgressFinished(false);
            return;
        }
    }

    m_Cancel = false;
    m_iRuningTestCount = 0;
    doOperate(package, version);
}

void DriverInstallerApt::undoInstallDriver()
{
    m_Cancel = true;
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->kill();
        qCInfo(appLog) << "DRIVER_LOG **************************** 取消操作";
    }
}

void DriverInstallerApt::aptClean()
{
    executeCommand("lastore-apt-clean");
}

QString DriverInstallerApt::executeCommand(const QString &cmd)
{
    QProcess process;
    process.start("bash", QStringList() << "-c" << cmd);
    process.waitForFinished();
    return QString::fromUtf8(process.readAllStandardOutput());
}

void DriverInstallerApt::doOperate(const QString &package, const QString &version)
{
    // 清理apt缓存
    aptClean();
    
    // 更新软件源
    executeCommand("apt update");

    // 检查包是否存在
    QString checkCmd = QString("apt-cache show %1=%2").arg(package).arg(version);
    if (executeCommand(checkCmd).isEmpty()) {
        emit errorOccurred(1);  // 包不存在
        qCInfo(appLog) << "DRIVER_LOG : ************************** 安装包不存在";
        return;
    }

    // 创建安装进程
    if (m_process) {
        delete m_process;
    }
    m_process = new QProcess(this);

    // 设置环境变量，避免交互
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("DEBIAN_FRONTEND", "noninteractive");
    m_process->setProcessEnvironment(env);

    // 监控输出获取进度
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        QString output = m_process->readAllStandardOutput();
        if (output.contains("%")) {
            QString progress = output.split("%").first();
            progress = progress.split(" ").last().trimmed();
            bool ok;
            int progressValue = progress.toInt(&ok);
            if (ok) {
                emit installProgressChanged(progressValue);
            }
        }
    });

    // 监控安装结果
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, package](int exitCode, QProcess::ExitStatus exitStatus) {
        if (m_Cancel) {
            emit errorOccurred(2);  // 用户取消
        } else if (exitCode == 0) {
            emit installProgressFinished(true);
        } else {
            if (package.contains("nvidia-driver")) {
                emit installProgressFinished(true);
            } else if (!isNetworkOnline()) {
                emit errorOccurred(3);  // 网络错误
            } else {
                emit errorOccurred(0);  // 其他错误
            }
        }
        qCInfo(appLog) << "DRIVER_LOG : ************************** 安装结束 退出码" << exitCode;
    });

    // 执行安装命令
    QString cmd = QString("apt-get install -y %1=%2").arg(package).arg(version);
    m_process->start("bash", QStringList() << "-c" << cmd);
}

bool DriverInstallerApt::isNetworkOnline(uint sec)
{
    QProcess process;
    process.setStandardOutputFile("netlog.bat", QIODevice::WriteOnly);
    process.start("ping", QStringList() << "www.baidu.com" << "-c" << "2" << "-w" << "2");
    process.waitForFinished(-1);
    
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return false;
    }
    
    if (sec > 0) {
        usleep(sec);
    }

    std::ifstream infile("netlog.bat");
    std::string line;
    std::vector<std::string> lines;
    
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }
    infile.close();

    if (lines.size() > 1) {
        std::string data = lines[lines.size() - 2];
        size_t pos = data.find("received,");
        if (pos != std::string::npos) {
            data = data.substr(pos + 10, 3);
            return atoi(data.c_str()) != 0;
        }
    }
    return false;
} 