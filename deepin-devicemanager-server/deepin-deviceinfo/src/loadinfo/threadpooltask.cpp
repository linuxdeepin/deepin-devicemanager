// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "threadpooltask.h"

#include <QTime>
#include <QProcess>
#include <QFile>
#include <QLoggingCategory>
#include <QDir>
#include <unistd.h>

#include "deviceinfomanager.h"
#include "cpu/cpuinfo.h"

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
    if (m_Cmd == "lscpu") {
        loadCpuInfo();
        return;
    }
    runCmdToCache(m_Cmd);
}

void ThreadPoolTask::runCmd(const QString &cmd)
{
    QProcess process;
    QStringList options;
    options << "-c" << cmd;
    process.start("/bin/bash", options);
    process.waitForFinished(m_Waiting);
}

void ThreadPoolTask::runCmd(const QString &cmd, QString &info)
{
    QProcess process;
    QString cmdT = cmd;
//    process.start(cmdT.replace(QString(" >  ") + PATH + m_File, ""));

    QStringList options;
    options << "-c" << cmdT.replace(QString(" >  ") + PATH + m_File, "");
    process.start("/bin/bash", options);

    process.waitForFinished(m_Waiting);
    info = process.readAllStandardOutput();
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

    // 2. 执行命令获取设备信息
    QString info;
    runCmd(cmd, info);
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

void ThreadPoolTask::runCmdToFile(const QString &cmd)
{
    // 1. 先判断通过该命令获取的信息是不是需要刷新的,如果是cpu，内存条，主板等信息则只需要开机获取即可
    QString path = PATH + m_File;
    QFile file(path);
    if (m_CanNotReplace && file.exists()) {
        return;
    }

    // 2. 执行命令获取设备信息
    runCmd(cmd);

    if (m_File == "lsblk_d.txt") {
        // 如果命令是 lsblk  , 则需要执行 smartctl --all /dev/***命令
        loadSmartctlInfoToFile(file);
    } else if (m_File == "lspci.txt") {
        // 如果命令是 lspci  , 则需要执行 lspci -v -s %1 > lspci_vs.txt 命令
        loadLspciVSInfoToFile(file);
    }
}


void ThreadPoolTask::loadSmartctlInfoToFile(QFile &file)
{
    if (file.open(QIODevice::ReadOnly)) {
        QString info = file.readAll();
        QStringList lines = info.split("\n");
        foreach (QString line, lines) {
            QStringList words = line.replace(QRegExp("[\\s]+"), " ").split(" ");
            // NAME ROTA
            if (words.size() != 2 || words[0] == "NAME") {
                continue;
            }

            QString smartCmd = QString("smartctl --all /dev/%1 > /tmp/device-info/smartctl_%2.txt").arg(words[0].trimmed()).arg(words[0].trimmed());
            runCmd(smartCmd);
        }
        file.close();
    }
}

void ThreadPoolTask::loadLspciVSInfoToFile(QFile &file)
{
    if (file.open(QIODevice::ReadOnly)) {
        QString info = file.readAll();
        QStringList lines = info.split("\n");
        foreach (const QString &line, lines) {
            QStringList words = line.split(" ");
            if (words.size() < 2) {
                continue;
            }
            if (words[1] == QString("ISA")) {
                QString cmd = QString("lspci -v -s %1 > /tmp/device-info/lspci_vs.txt").arg(words[0].trimmed());
                runCmd(cmd);
                break;
            }
        }
        file.close();
    }
}
