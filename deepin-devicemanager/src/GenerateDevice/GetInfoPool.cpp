// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GetInfoPool.h"

#include <QObjectCleanupHandler>
#include <QLoggingCategory>

#include "CmdTool.h"
#include "DeviceManager.h"
#include "DDLog.h"

using namespace DDLog;

static QMutex mutex;

CmdTask::CmdTask(QString key, QString file, QString info, GetInfoPool *parent)
    : m_Key(key)
    , m_File(file)
    , m_Info(info)
    , mp_Parent(parent)
{
    qCDebug(appLog) << "CmdTask constructor, key:" << key;
}

CmdTask::~CmdTask()
{
    qCDebug(appLog) << "CmdTask destructor";

}

void CmdTask::run()
{
    qCDebug(appLog) << "CmdTask::run start";

    CmdTool tool;
    tool.loadCmdInfo(m_Key, m_File);
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = tool.cmdInfo();
    mp_Parent->finishedCmd(m_Info, cmdInfo);
}

GetInfoPool::GetInfoPool()
    : m_Arch("")
    , m_FinishedNum(0)
{
    qCDebug(appLog) << "GetInfoPool constructor";
    initCmd();
}

void GetInfoPool::getAllInfo()
{
    qCDebug(appLog) << "GetInfoPool::getAllInfo start";
    DeviceManager::instance()->clear();

    QList<QStringList>::iterator it = m_CmdList.begin();
    for (; it != m_CmdList.end(); ++it) {
        qCDebug(appLog) << "GetInfoPool::getAllInfo start task for key:" << (*it)[0];
        CmdTask *task = new CmdTask((*it)[0], (*it)[1], (*it)[2], this);
        start(task);
        task->deleteLater();
    }
}

void GetInfoPool::finishedCmd(const QString &info, const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo)
{
    qCDebug(appLog) << "GetInfoPool::finishedCmd, info:" << info << "cmdInfo size:" << cmdInfo.size();
    DeviceManager::instance()->addCmdInfo(cmdInfo);
    QMutexLocker m_lock(&mutex);
    m_FinishedNum++;
    if (m_FinishedNum == m_CmdList.size()) {
        qCDebug(appLog) << "GetInfoPool::finishedCmd all tasks finished";
        emit finishedAll(info);
        m_FinishedNum = 0;
    }
}

void GetInfoPool::setFramework(const QString &arch)
{
    qCDebug(appLog) << "GetInfoPool::setFramework, arch:" << arch;
    // 设置架构
    m_Arch = arch;
}

void GetInfoPool::initCmd()
{
    qCDebug(appLog) << "GetInfoPool::initCmd start";
    m_CmdList.append({ "lshw",                 "lshw.txt",               tr("Loading Audio Device Info...") });
    m_CmdList.append({ "printer",              "printer.txt",            ""});

    m_CmdList.append({ "dmidecode0",           "dmidecode_0.txt",        tr("Loading BIOS Info...")});
    m_CmdList.append({ "dmidecode1",           "dmidecode_1.txt",        ""});
    m_CmdList.append({ "dmidecode2",           "dmidecode_2.txt",        ""});
    m_CmdList.append({ "dmidecode3",           "dmidecode_3.txt",        ""});
    m_CmdList.append({ "dmidecode4",           "dmidecode_4.txt",        ""});
    m_CmdList.append({ "dmidecode13",          "dmidecode_13.txt",       ""});
    m_CmdList.append({ "dmidecode16",          "dmidecode_16.txt",       ""});
    m_CmdList.append({ "dmidecode17",          "dmidecode_17.txt",       ""});
    m_CmdList.append({"dr_config", "dr_config.txt", ""});

    m_CmdList.append({ "hwinfo_monitor",       "hwinfo_monitor.txt",     tr("Loading CD-ROM Info...")});
    m_CmdList.append({ "hwinfo",         "hwinfo.txt",       ""});


    m_CmdList.append({ "upower",               "upower_dump.txt",        ""});
    m_CmdList.append({ "lscpu",                "lscpu.txt",              tr("Loading Operating System Info...")});
    m_CmdList.append({ "lsblk_d",              "lsblk_d.txt",            ""});
    m_CmdList.append({ "ls_sg",                "ls_sg.txt",              ""});//Add a newer way getting device info
    m_CmdList.append({ "xrandr",               "xrandr.txt",             tr("Loading CPU Info...")});
    m_CmdList.append({ "xrandr_verbose",       "xrandr_verbose.txt",     tr("Loading Other Devices Info...")});
    m_CmdList.append({ "dmesg",                "dmesg.txt",              tr("Loading Power Info...")});
    m_CmdList.append({ "hciconfig",            "hciconfig.txt",          tr("Loading Printer Info...")});
    m_CmdList.append({ "nvidia",               "nvidia.txt",             ""});

    m_CmdList.append({ "cat_boardinfo",        "/proc/boardinfo",        tr("Loading Mouse Info...")});
    m_CmdList.append({ "cat_os_release",       "/etc/os-release",        tr("Loading Network Adapter Info...")});
    m_CmdList.append({ "cat_version",          "/proc/version",          ""});
    m_CmdList.append({ "cat_devices",          "/proc/bus/input/devices", ""});
    m_CmdList.append({ "cat_audio",            "/proc/asound/card0/codec#0",     ""});
    m_CmdList.append({ "cat_gpuinfo",          "/proc/gpuinfo_0 ",     ""});
    m_CmdList.append({ "bt_device",            "bt_device.txt",          ""}); // 蓝牙设备配对信息
    qCDebug(appLog) << "GetInfoPool::initCmd end, initialized" << m_CmdList.size() << "commands";
}
