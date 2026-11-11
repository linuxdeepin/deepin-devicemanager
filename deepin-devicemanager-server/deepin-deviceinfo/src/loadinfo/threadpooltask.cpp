// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "threadpooltask.h"
#include "deviceinfomanager.h"
#include "cpu/cpuinfo.h"
#include "DDLog.h"
using namespace DDLog;

#include <QTime>
#include <QProcess>
#include <QLoggingCategory>
#include <QDir>
#include <unistd.h>

ThreadPoolTask::ThreadPoolTask(QString cmd, QString file, bool replace, int waiting, QObject *parent)
    : QObject(parent),
      m_Cmd(cmd),
      m_File(file),
      m_CanNotReplace(replace),
      m_Waiting(waiting)
{

}

ThreadPoolTask::~ThreadPoolTask()
{

}

void ThreadPoolTask::run()
{
    runCmdToCache(m_Cmd);
}

void ThreadPoolTask::runCmd(const QString &cmd, QString &info)
{
    QString cmdExec = cmd.left(cmd.indexOf('>')).trimmed();
    QString cmdStr = cmdExec.split(' ').first().trimmed();
    QString cmdArg = cmdExec.mid(cmdStr.count() + 1).trimmed();
    QStringList args;
    if (!cmdArg.isEmpty())
        args = cmdArg.split(' ');
    if (cmdStr.isEmpty())
        return;

    // 处理包含*的命令参数
    if (cmd.startsWith("ls /dev/sg*")) {
        info = runAsteriskCmd(cmdStr, args.first().trimmed());
        return;
    } else if (cmdExec.startsWith("cat /boot/config*")) {
        QString filter = cmdExec.split('|').last().split(' ').last().replace('\'', "");
        if (filter.isEmpty())
            return;
        QString outPut = runAsteriskCmd("ls", "/boot/config*");
        if (outPut.isEmpty())
            return;
        QStringList paths = outPut.split('\n');
        QStringList results;
        for (auto path : paths) {
            if (path.isEmpty())
                continue;
            QProcess proc;
            proc.start("cat", QStringList() << path);
            proc.waitForFinished();
            QString info1;
            if (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0)
                info1 = proc.readAllStandardOutput();
            if (info1.isEmpty())
                continue;
            QStringList lines = info1.split('\n');
            for (auto line : lines) {
                if (line.contains(filter))
                    results.push_back(line);
            }
        }

        if (!results.isEmpty())
            info = results.join('\n');
        //qCInfo(deviceInfoLog) << "runcmdExec:" << cmdExec << "args:" << args << "outPut:" << info;
        return;
    }

    QProcess process;
    process.start(cmdStr, args);
    process.waitForFinished(m_Waiting);
    info = process.readAllStandardOutput();

    //qCInfo(deviceInfoLog) << "runcmdExec:" << cmdExec << "args:" << args << "outPut:" << info;
}

QString ThreadPoolTask::runAsteriskCmd(const QString &cmd, const QString &arg)
{
    QString info = "";
    if (cmd.isEmpty())
        return info;

    QStringList args;
    QString path;
    QString startWord;
    if (arg == "/dev/sg*" || arg == "/boot/config*") {
        path = arg.left(arg.lastIndexOf('/'));
        args << path;
        startWord = arg.split('/').last().replace('*', "");
    } else {
        return info;
    }

    QProcess process;
    process.start(cmd, args);
    process.waitForFinished(-1);

    QString outPut = process.readAllStandardOutput();
    QStringList outPutLines = outPut.trimmed().split('\n');
    QStringList filterLines;
    for (auto line : outPutLines) {
        if (line.startsWith(startWord))
            filterLines.push_back(path + "/" +line +"\n");
    }

    if (!filterLines.isEmpty())
        info = filterLines.join(' ');

    //qCInfo(deviceInfoLog) << "runAsteriskCmd:" << cmd << "arg:" << arg << "info:" << info;

    return info;
}

void ThreadPoolTask::runCmdToCache(const QString &cmd)
{
    QString key = m_File;
    key.replace(".txt", "");
    bool existed = DeviceInfoManager::getInstance()->isInfoExisted(key);

    // 1. 先判断通过该命令获取的信息是不是需要刷新的,如果是cpu，内存条，主板等信息则只需要开机获取即可
    if (m_CanNotReplace && existed) {
        return;
    }

    // lscpu 通过 CpuInfo 直接读取 /sys 与 /proc，不依赖 lscpu 命令的输出，
    // 因此单独处理。放在这里而不是 run() 里，是为了同样受 canNotReplace 约束，
    // 避免每次刷新都重复采集 CPU 信息。
    if (m_Cmd == "lscpu") {
        loadCpuInfo();
        return;
    }

    // 2. 执行命令获取设备信息
    QString info;
    runCmd(cmd, info);

    // 命令执行失败（如权限不足、工具缺失、硬件不支持）时输出为空。
    // 此时不写入缓存，否则 isInfoExisted() 会认为该信息已经采集过，
    // 对于 canNotReplace 的命令来说就永远不会再重试了。
    if (info.isEmpty()) {
        qCWarning(appLog) << "command produced empty output, skip caching:" << cmd;
        return;
    }

    // 3. 管理设备信息
    // 如果命令是 lsblk  , 则需要执行 smartctl --all /dev/***命令
    if (m_File == "lsblk_d.txt") {
        loadSmartCtlInfoToCache(info);
    }

    // 如果命令是 ls /dev/sg* ,则需要执行 smartctl --all /dev/*** 命令
    if (m_File == "ls_sg.txt") {
        loadSgSmartCtlInfoToCache(info);
    }

    // 如果命令是 lspci  , 则需要执行 lspci -v -s %1 > lspci_vs.txt 命令
    if (m_File == "lspci.txt") {
        loadLspciVSInfoToCache(info);
    }

    if ("hwinfo_display.txt" == m_File) {
        loadDisplayWidth(info);
    }

    DeviceInfoManager::getInstance()->addInfo(key, info);
}

void ThreadPoolTask::loadSmartCtlInfoToCache(const QString &info)
{
    QStringList lines = info.split("\n");
    foreach (QString line, lines) {
        QStringList words = line.replace(QRegExp("[\\s]+"), " ").split(" ");
        // NAME ROTA
        if (words.size() != 2 || words[0] == "NAME") {
            continue;
        }

        QString smartCmd = QString("smartctl --all /dev/%1").arg(words[0].trimmed());
        QString sInfo;
        runCmd(smartCmd, sInfo);
        // 在使用smartctl的时候会出现对 /dev/sda 出现判断错误的情况，此时可以对/dev/sda1进行处理
        if (sInfo.contains("Read Device Identity failed:")) {
            smartCmd = smartCmd + "1";
            runCmd(smartCmd, sInfo);
        }
        DeviceInfoManager::getInstance()->addInfo(QString("smartctl_%1").arg(words[0].trimmed()), sInfo);
    }
}

void ThreadPoolTask::loadCpuInfo()
{
    CpuInfo cpu;
    if (cpu.loadCpuInfo()) {
        QString info;
        cpu.logicalCpus(info);
        DeviceInfoManager::getInstance()->addInfo("lscpu", info);

        QString numInfo;
        numInfo += QString("%1 : %2\n").arg("physical").arg(cpu.physicalNum());
        numInfo += QString("%1 : %2\n").arg("core").arg(cpu.coreNum());
        numInfo += QString("%1 : %2\n").arg("logical").arg(cpu.logicalNum());
        DeviceInfoManager::getInstance()->addInfo("lscpu_num", numInfo);
    }
}

void ThreadPoolTask::loadSgSmartCtlInfoToCache(const QString &info)
{
    QStringList lines = info.split("\n");

    foreach (QString line, lines) {
        if (line.isEmpty()) {
            continue;
        }

        QStringList words = line.split("/");

        QString smartCmd = QString("smartctl --all /dev/%1").arg(words[2].trimmed());
        QString sInfo;
        runCmd(smartCmd, sInfo);
        DeviceInfoManager::getInstance()->addInfo(QString("smartctl_%1").arg(words[2]), sInfo);
    }
}

void ThreadPoolTask::loadLspciVSInfoToCache(const QString &info)
{
    QStringList lines = info.split("\n");
    foreach (const QString &line, lines) {
        QStringList words = line.split(" ");
        if (words.size() < 2) {
            continue;
        }
        if (words[1] == QString("ISA")) {
            QString cmd = QString("lspci -v -s %1").arg(words[0].trimmed()); //  > /tmp/device-info/lspci_vs.txt
            QString sInfo;
            runCmd(cmd, sInfo);
            DeviceInfoManager::getInstance()->addInfo("lspci_vs", sInfo);
            break;
        }
    }
}

void ThreadPoolTask::loadDisplayWidth(const QString &info)
{
    QString widthS;
    QStringList params = info.split("\n\n");
    foreach (const QString &param, params) {

        QStringList lines = param.split("\n");
        if (lines.size() < 5)
            continue;
        foreach (const QString &line, lines) {
            if (line.contains("SysFS ID")) {
                QString pci = line.right(7);
                int width = getDisplayWidthFromLspci(pci);
                widthS += pci;
                widthS += "-";
                widthS += QString::number(width);
                widthS += "\n";
                break;
            }
        }

        DeviceInfoManager::getInstance()->addInfo("width", widthS);
    }
}

int ThreadPoolTask::getDisplayWidthFromLspci(const QString &info)
{
    QString cmd = QString("lspci -v -s %1").arg(info);
    QString sInfo;
    runCmd(cmd, sInfo);
    QStringList lines = sInfo.split("\n");
    foreach (const QString &line, lines) {
        if (!line.contains("Memory at")) {
            continue;
        }
        if (line.contains("32-bit"))
            return 32;
        else
            return 64;
    }
    return 64;
}
