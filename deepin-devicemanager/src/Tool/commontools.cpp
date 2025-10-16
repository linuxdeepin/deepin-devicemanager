// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commontools.h"
#include "commondefine.h"
#include "DDLog.h"
#include "EDIDParser.h"
#include "DeviceMonitor.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <DConfig>

DWIDGET_USE_NAMESPACE
using namespace DDLog;
#define ICON_SIZE_WIDTH 36
#define ICON_SIZE_HEIGHT 36

//  名称("Name") 厂商("Vendor") 型号("Model") 版本(Version) 显存("Graphics Memory")

constexpr char kName[] { "Name" };
constexpr char kVendor[] { "Vendor" };
constexpr char kModel[] { "Model" };
constexpr char kVersion[] { "Version" };
constexpr char kGraphicsMemory[] { "Graphics Memory" };

QMap<DriverType, QString> CommonTools::m_MapDriverIcon = {
    {DR_Bluetooth, QString(":/icons/deepin/builtin/icons/bluetooth.svg")}
    , {DR_Camera, QString(":/icons/deepin/builtin/icons/image.svg")}
    , {DR_Gpu, QString(":/icons/deepin/builtin/icons/GPU.svg")}
    , {DR_Keyboard, QString(":/icons/deepin/builtin/icons/keyboard.svg")}
    , {DR_Sound, QString(":/icons/deepin/builtin/icons/voice.svg")}
    , {DR_Mouse, QString(":/icons/deepin/builtin/icons/mouse.svg")}
    , {DR_Network, QString(":/icons/deepin/builtin/icons/network.svg")}
    , {DR_Printer, QString(":/icons/deepin/builtin/icons/printer.svg")}
    , {DR_Scaner, QString(":/icons/deepin/builtin/icons/scaner.svg")}
    , {DR_Tablet, QString(":/icons/deepin/builtin/icons/tablet.svg")}
    , {DR_WiFi, QString(":/icons/deepin/builtin/icons/Wi-Fi.svg")}
    , {DR_OtherDevice, QString(":/icons/deepin/builtin/icons/scaner.svg")}
};

QMap<DriverType, QString> CommonTools::m_MapDriverType = {
    {DR_Bluetooth, QObject::tr("Bluetooth adapter")}
    , {DR_Camera, QObject::tr("Imaging device")}
    , {DR_Gpu, QObject::tr("Display adapter")}
    , {DR_Keyboard, QObject::tr("Keyboard")}
    , {DR_Sound, QObject::tr("Sound card")}
    , {DR_Mouse, QObject::tr("Mouse")}
    , {DR_Network, QObject::tr("Network adapter")}
    , {DR_Printer, QObject::tr("Printer")}
    , {DR_Scaner, QObject::tr("Imaging device")}
    , {DR_Tablet, QObject::tr("Other Devices")}
    , {DR_WiFi, QObject::tr("Wireless network adapter")}
    , {DR_OtherDevice, QObject::tr("Other Devices")}
};

QMap<Status, QString> CommonTools::m_MapStatusIcon = {
    {ST_SUCESS, QString(":/icons/deepin/builtin/icons/ok.svg")}
    , {ST_FAILED, QString(":/icons/deepin/builtin/icons/warning.svg")}
    , {ST_DOWNLOADING, QString("")}
    , {ST_INSTALL, QString("")}
    , {ST_NOT_INSTALL, QString(":/icons/deepin/builtin/icons/alert.svg")}
    , {ST_CAN_UPDATE, QString(":/icons/deepin/builtin/icons/alert.svg")}
    , {ST_WAITING, QString(":/icons/deepin/builtin/icons/waiting.svg")}
    , {ST_DRIVER_NOT_BACKUP, QString(":/icons/deepin/builtin/icons/alert.svg")}
    , {ST_DRIVER_BACKING_UP, QString("")}
    , {ST_DRIVER_BACKUP_FAILED, QString(":/icons/deepin/builtin/icons/warning.svg")}
    , {ST_DRIVER_BACKUP_SUCCESS, QString(":/icons/deepin/builtin/icons/ok.svg")}
    , {ST_DRIVER_RESTORING, QString("")}
};

QMap<Status, QString> CommonTools::m_MapStatusType = {
    {ST_SUCESS, QObject::tr("Installation successful")}
    , {ST_FAILED, QObject::tr("Installation failed")}
    , {ST_DOWNLOADING, QObject::tr("Downloading")}
    , {ST_INSTALL, QObject::tr("Installing")}
    , {ST_NOT_INSTALL, QObject::tr("Not installed")}
    , {ST_CAN_UPDATE, QObject::tr("Out-of-date")}
    , {ST_WAITING, QObject::tr("Waiting")}
    , {ST_DRIVER_NOT_BACKUP, QObject::tr("Not backed up")}
    , {ST_DRIVER_BACKING_UP, QObject::tr("Backing up")}
    , {ST_DRIVER_BACKUP_FAILED, QObject::tr("Backup failed")}
    , {ST_DRIVER_BACKUP_SUCCESS, QObject::tr("Backup successful")}
    , {ST_DRIVER_RESTORING, QObject::tr("Restoring")}
};

QMap<int, QString> CommonTools::m_MapErrorString = {
    {EC_NULL, QObject::tr("Unknown error")}
    , {EC_NETWORK, QObject::tr("Network error")}
    , {EC_CANCEL, QObject::tr("Canceled")}
    , {EC_NOTFOUND, QObject::tr("Failed to get driver files")}
    , {EC_NOTIFY_NETWORK, tr("EC_NOTIFY_NETWORK")}
    , {EC_REINSTALL, tr("EC_REINSTALL")}
    , {EC_6, tr("EC_6")}
};

CommonTools::CommonTools(QObject *parent)
    : QObject(parent)
{
    qCDebug(appLog) << "Initializing common tools";
}

QString CommonTools::getDriverType(DriverType type)
{
    // qCDebug(appLog) << "Getting driver type";
    return m_MapDriverType[type];
}

QString CommonTools::getDriverPixmap(DriverType type)
{
    // qCDebug(appLog) << "Getting driver pixmap";
    return m_MapDriverIcon[type];
}

QString CommonTools::getStausType(Status type)
{
    // qCDebug(appLog) << "Getting status type";
    return m_MapStatusType[type];
}

QString CommonTools::getStatusPixmap(Status type)
{
    // qCDebug(appLog) << "Getting status pixmap";
    return m_MapStatusIcon[type];
}

QString CommonTools::getSystemTime()
{
    qCDebug(appLog) << "Getting current system time";

    QDateTime time = QDateTime::currentDateTime();
    QString date = time.toString("yyyy-M-d hh:mm");
    return date;
}

QString CommonTools::getErrorString(int err)
{
    qCDebug(appLog) << "Getting error string for code:" << err;

    return m_MapErrorString[err];
}

void CommonTools::feedback()
{
    qCDebug(appLog) << "Initiating feedback session";

    QDBusInterface interface("com.deepin.dde.ServiceAndSupport",
                              "/com/deepin/dde/ServiceAndSupport",
                              "com.deepin.dde.ServiceAndSupport");

     //    selfSupport         = 0, //自助支持
     //    messageConsultation = 1, //留言咨询
     //    customerChat        = 2, //在线客服
     //    contentUs           = 3  //联系我们
     uint8_t supporttype = 1;
     QDBusReply<void> reply = interface.call("ServiceSession", supporttype);
     if (reply.isValid()) {
         qCDebug(appLog) << "call com.deepin.dde.ServiceAndSupport success";
     } else {
         qCDebug(appLog) << "call com.deepin.dde.ServiceAndSupport failed";
     }
}

QString CommonTools::getUrl()
{
    qCDebug(appLog) << "Getting driver search URL";

    QFile file(QDir::homePath() + "/url");
    if(!file.open(QIODevice::ReadOnly)){
        qCDebug(appLog) << "Using default driver search URL";
        return "https://driver.uniontech.com/api/v1/drive/search";
    }
    QString info = file.readAll();
    if("true" == info){
        qCDebug(appLog) << "Using production driver search URL";
        return "https://driver.uniontech.com/api/v1/drive/search";
    }else{
        qCDebug(appLog) << "Using pre-production driver search URL";
        return "https://drive-pre.uniontech.com/api/v1/drive/search";
    }
}

QString CommonTools::getBackupPath()
{
    qCDebug(appLog) << "Getting backup path";
    return "/var/lib/deepin-devicemanager/";
}

void CommonTools::parseEDID(const QStringList &allEDIDS, const QString &input, bool isHW)
{
    for (auto edid:allEDIDS) {
        QString edidStr;
        if (isHW) {
            QProcess process;
            process.start(QString("hexdump %1").arg(edid));
            process.waitForFinished(-1);

            QString deviceInfo = process.readAllStandardOutput();
            if (deviceInfo.isEmpty())
                continue;

            QStringList lines = deviceInfo.split("\n");
            for (auto line:lines) {
                QStringList words = line.trimmed().split(" ");
                if (words.size() != 9)
                    continue;

                words.removeAt(0);
                QString l = words.join("");
                l.append("\n");
                edidStr.append(l);
            }
        } else {
            QProcess process;
            process.start(QString("hexdump -C %1").arg(edid));
            process.waitForFinished(-1);
            QString deviceInfo = process.readAllStandardOutput();
            if (deviceInfo.isEmpty())
                continue;

            QStringList lines = deviceInfo.split("\n");
            for (auto line: lines) {
                if (line.trimmed().isEmpty())
                    continue;
                int firstSpace = line.indexOf(" ");
                int lastPipe = line.indexOf("|");

                if (firstSpace == -1 || lastPipe == -1 || firstSpace >= lastPipe)
                    continue;

                QString hexPart = line.mid(firstSpace + 2, lastPipe - firstSpace -3).trimmed();

                QStringList hexBytes = hexPart.split(QRegularExpression("\\s+"));

                QString lineData;
                for (const QString &hexByte : hexBytes) {
                    if (hexByte.length() == 2 && hexByte != "")
                        lineData.append(hexByte);
                }

                if (lineData.length() == 32)
                    edidStr.append(lineData + "\n");
            }
        }

        QStringList lines = edidStr.split("\n");
        if (lines.size() > 3){
            EDIDParser edidParser;
            QString errorMsg;
            edidParser.setEdid(edidStr,errorMsg,"\n", isHW ? false : true);

            QMap<QString, QString> mapInfo;
            mapInfo.insert("Vendor",edidParser.vendor());
            if (isHW)
                mapInfo.insert("Model",edidParser.model());
            else
                mapInfo.insert("Model",edidParser.monitorName());
            mapInfo.insert("Size",edidParser.screenSize());
            mapInfo.insert("Display Input",input);

            DeviceMonitor *device = new DeviceMonitor();
            if (isHW)
                device->setInfoFromEdid(mapInfo);
            else
                device->setInfoFromEdidForCustom(mapInfo);
            DeviceManager::instance()->addMonitor(device);
        }
    }
}

QString CommonTools::getGpuInfoCommandFromDConfig()
{
    QString cmd;
    DConfig *dconfig = DConfig::create("org.deepin.devicemanager","org.deepin.devicemanager");
    if (dconfig && dconfig->isValid() && dconfig->keyList().contains("CommandToGetGPUInfo"))
        cmd = dconfig->value("CommandToGetGPUInfo").toString();
    return cmd;
}

QString CommonTools::preGenerateGpuInfo()
{
    static QString gpuBaseInfo { "" };
    static QString gpuMemInfo { "" };

    if (gpuBaseInfo.isEmpty()) {
        QMap<QString, QString> mapInfo;
        if (getGpuBaseInfo(mapInfo)) {
            for (auto it = mapInfo.begin(); it != mapInfo.end(); ++it) {
                QString tmpInfo = it.key() + ": " + it.value() + "\n";
                gpuBaseInfo.append(tmpInfo);
            }
        }
    }

    if (gpuBaseInfo.isEmpty()) {
        qCritical() << "Error: failed to get gpu base info!";
        return "";
    }

    if (gpuMemInfo.isEmpty()) {
        QDBusInterface iface("org.deepin.DeviceInfo",
                             "/org/deepin/DeviceInfo",
                             "org.deepin.DeviceInfo",
                             QDBusConnection::systemBus());
        if (iface.isValid()) {
            QDBusReply<QString> replyList = iface.call("getGpuInfoForFTDTM");
            if (replyList.isValid()) {
                gpuMemInfo = replyList.value();
            } else {
                qCritical() << "Error: failed to call dbus to get gpu memery info! ";
            }
        }
    }

    return (gpuBaseInfo + gpuMemInfo);
}

bool CommonTools::getGpuBaseInfo(QMap<QString, QString> &mapInfo)
{
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    process.setProcessEnvironment(env);
    process.start("/usr/bin/glxinfo", QStringList() << "-B");
    if (!process.waitForFinished()) {
        qCritical() << "Error executing glxinfo:" << process.errorString();
        return false;
    }

    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QStringList lines = output.split('\n');
    QRegularExpression regex("^([^:]+):\\s*(.+)$");
    for (const QString &line : lines) {
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            QString key = match.captured(1).trimmed();
            QString value = match.captured(2).trimmed();
            if (key == "OpenGL renderer string") {
                mapInfo.insert(kName, value);
                mapInfo.insert(kModel, value);
            } else if (key == "OpenGL vendor string") {
                mapInfo.insert(kVendor, value);
            }
        }
    }

    return true;
}
