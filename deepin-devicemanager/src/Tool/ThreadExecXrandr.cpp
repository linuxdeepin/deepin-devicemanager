// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ThreadExecXrandr.h"
#include "commonfunction.h"
#include "DDLog.h"
#include <DSysInfo>

#include <QProcess>
#include <QLoggingCategory>
#include <QDBusInterface>
#include <QDBusReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <DApplication>
#include <QRegularExpression>
#include <DeviceManager.h>
#include<QDateTime>

const QString DISPLAY_SERVICE_NAME_V23 = "org.deepin.dde.Display1";
const QString DISPLAY_SERVICE_PATH_V23 = "/org/deepin/dde/Display1";
const QString DISPLAY_INTERFACE_V23 = "org.deepin.dde.Display1";

const QString DISPLAY_DAEMON_SERVICE_NAME_V23 = "org.deepin.dde.Display1";
const QString DISPLAY_DAEMON_SERVICE_PATH_V23 = "/org/deepin/dde/Display1";
const QString DISPLAY_DAEMON_INTERFACE_V23 = "org.deepin.dde.Display1";
const QString DISPLAY_MONITOR_INTERFACE_V23 = "org.deepin.dde.Display1.Monitor";

const QString DISPLAY_SERVICE_NAME_V20 = "com.deepin.system.Display";
const QString DISPLAY_SERVICE_PATH_V20 = "/com/deepin/system/Display";
const QString DISPLAY_INTERFACE_V20 = "com.deepin.system.Display";

const QString DISPLAY_DAEMON_SERVICE_NAME_V20 = "com.deepin.daemon.Display";
const QString DISPLAY_DAEMON_SERVICE_PATH_V20 = "/com/deepin/daemon/Display";
const QString DISPLAY_DAEMON_INTERFACE_V20 = "com.deepin.daemon.Display";
const QString DISPLAY_MONITOR_INTERFACE_V20 = "com.deepin.daemon.Display.Monitor";

inline bool isV20() { return Dtk::Core::DSysInfo::majorVersion() == "20"; }

const QString DISPLAY_SERVICE_NAME = isV20() ? DISPLAY_SERVICE_NAME_V20 : DISPLAY_SERVICE_NAME_V23;
const QString DISPLAY_SERVICE_PATH = isV20() ? DISPLAY_SERVICE_PATH_V20 : DISPLAY_SERVICE_PATH_V23;
const QString DISPLAY_INTERFACE = isV20() ? DISPLAY_INTERFACE_V20 : DISPLAY_INTERFACE_V23;

const QString DISPLAY_DAEMON_SERVICE_NAME = isV20() ? DISPLAY_DAEMON_SERVICE_NAME_V20 : DISPLAY_DAEMON_SERVICE_NAME_V23;
const QString DISPLAY_DAEMON_SERVICE_PATH = isV20() ? DISPLAY_DAEMON_SERVICE_PATH_V20 : DISPLAY_DAEMON_SERVICE_PATH_V23;
const QString DISPLAY_DAEMON_INTERFACE = isV20() ? DISPLAY_DAEMON_INTERFACE_V20 : DISPLAY_DAEMON_INTERFACE_V23;
const QString DISPLAY_MONITOR_INTERFACE = isV20() ? DISPLAY_MONITOR_INTERFACE_V20 : DISPLAY_MONITOR_INTERFACE_V23;

const QString DISPLAY_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";

using namespace DDLog;
ThreadExecXrandr::ThreadExecXrandr(bool gpu, bool isDXcbPlatform)
    : m_Gpu(gpu), m_isDXcbPlatform(isDXcbPlatform)
{
    qCDebug(appLog) << "ThreadExecXrandr created. GPU mode:" << gpu << "DXcbPlatform:" << isDXcbPlatform;
}

void ThreadExecXrandr::run()
{
    qCDebug(appLog) << "Thread started execution";

    if (m_Gpu) {
        qCDebug(appLog) << "Getting GPU info from xrandr";
        getGpuInfoFromXrandr();
    } else {           
       if(Common::boardVendorType() == "PGUV") {
           qCDebug(appLog) << "PGUV platform detected, getting resolution from DBus";
           QList<QMap<QString, QString>> lstMap;
           getResolutionRateFromDBus(lstMap);
       } else {
            qCDebug(appLog) << "Getting monitor info from xrandr --verbose";
            getMonitorInfoFromXrandrVerbose();
       }
    }
}

void ThreadExecXrandr::runCmd(QString &info, const QString &cmd)
{
    qCDebug(appLog) << "Executing command:" << cmd;

    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    info = process.readAllStandardOutput();
}

void ThreadExecXrandr::loadXrandrInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd)
{
    qCDebug(appLog) << "Loading xrandr info with command:" << cmd;

    QString deviceInfo;
    runCmd(deviceInfo, cmd);
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        if (line.startsWith("Screen")) {
            qCDebug(appLog) << "Found Screen line:" << line;
            lstMap.append(QMap<QString, QString>());
            QRegularExpression re(".*\\s([0-9]{1,5}\\sx\\s[0-9]{1,5}).*\\s([0-9]{1,5}\\sx\\s[0-9]{1,5}).*\\s([0-9]{1,5}\\sx\\s[0-9]{1,5}).*");
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                lstMap[lstMap.count() - 1].insert("minResolution", match.captured(1));
                lstMap[lstMap.count() - 1].insert("curResolution", match.captured(2));
                lstMap[lstMap.count() - 1].insert("maxResolution", match.captured(3));
                qCDebug(appLog) << "Parsed resolutions: min" << match.captured(1) << "cur" << match.captured(2) << "max" << match.captured(3);
            }
            continue;
        }

        if (line.startsWith("HDMI")) {
            qCDebug(appLog) << "Found HDMI interface.";
            lstMap[lstMap.count() - 1].insert("HDMI", "Enable");
        } else if (line.startsWith("VGA")) {
            qCDebug(appLog) << "Found VGA interface.";
            lstMap[lstMap.count() - 1].insert("VGA", "Enable");
        } else if (line.startsWith("DP") || line.startsWith("DisplayPort")) {
            qCDebug(appLog) << "Found DP interface.";
            lstMap[lstMap.count() - 1].insert("DP", "Enable");
        } else if (line.startsWith("eDP")) {
            qCDebug(appLog) << "Found eDP interface.";
            lstMap[lstMap.count() - 1].insert("eDP", "Enable");
        } else if (line.startsWith("DVI")) {
            qCDebug(appLog) << "Found DVI interface.";
            lstMap[lstMap.count() - 1].insert("DVI", "Enable");
        } else if (line.startsWith("DigitalOutput")) {
            qCDebug(appLog) << "Found DigitalOutput interface.";
            lstMap[lstMap.count() - 1].insert("DigitalOutput", "Enable");
        }
    }
}

void ThreadExecXrandr::loadXrandrVerboseInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd)
{
    qCDebug(appLog) << "Loading verbose xrandr info with command:" << cmd;

    QString deviceInfo;
    runCmd(deviceInfo, cmd);

    QStringList lines = deviceInfo.split("\n");
    QStringList::iterator it = lines.begin();
    for (; it != lines.end(); ++it) {
        if ((*it).startsWith("Screen"))
            continue;

        // 获取 HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm
        QRegularExpression reg("^[A-Za-z].*");
        if (reg.match(*it).hasMatch() && !(*it).contains("disconnected")) {
            qCDebug(appLog) << "Found new display screen:" << (*it).trimmed();
            // 新的显示屏
            QMap<QString, QString> newMap;
            newMap.insert("mainInfo", (*it).trimmed());
            lstMap.append(newMap);
            continue;
        }

        if (lstMap.size() < 1)
            continue;
        QMap<QString, QString> &last = lstMap.last();
        // 获取edid信息
        QString edid;
        QRegularExpression reEdid("^[\\t]{2}([0-9a-f]{32}).*");
        QRegularExpressionMatch edidMatch = reEdid.match(*it);
        if (edidMatch.hasMatch()) {
            while (true) {
                edid.append(edidMatch.captured(1));
                edid.append("\n");
                if (++it == lines.end())
                    return;
                edidMatch = reEdid.match(*it);
                if (!edidMatch.hasMatch()) {
                    qCDebug(appLog) << "Parsed EDID info:" << edid;
                    last.insert("edid", edid);
                    break;
                }
            }
            continue;
        }

        // 获取当前频率
        if ((*it).contains("*current")) {
            if ((it += 2) >= lines.end())
                return;
            QRegularExpression regRate(".*([0-9]{1,5}\\.[0-9]{1,5}Hz).*");
            QRegularExpressionMatch rateMatch = regRate.match(*it);
            if (rateMatch.hasMatch()) {
                last.insert("rate", rateMatch.captured(1));
                qCDebug(appLog) << "Parsed refresh rate:" << rateMatch.captured(1);
            }
        }
    }
}

void ThreadExecXrandr::getRefreshRateFromDBus(QList<QMap<QString, QString> > &lstMap)
{
    qCDebug(appLog) << "Creating display interface";
    QDBusInterface displayInterface(DISPLAY_SERVICE_NAME, DISPLAY_SERVICE_PATH, DISPLAY_INTERFACE, QDBusConnection::systemBus());
    if (!displayInterface.isValid())
        return;
    QDBusReply<QString> reply = displayInterface.call("GetConfig");
    if (!reply.isValid())
        return;
    QString info = reply.value();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(info.toLatin1());
    if (!jsonDoc.isObject())
        return;
    QJsonObject object = jsonDoc.object();
    QStringList keys = object.keys();

    //读取Config
    if (!keys.contains("Config"))
        return;
    QJsonValue configValue = object.value("Config");
    if (!configValue.isObject())
        return;
    QJsonObject configObject = configValue.toObject();
    QStringList configKeys = configObject.keys();

    //读取Screens
    if (!configKeys.contains("Screens"))
        return;
    QJsonValue screensValue = configObject.value("Screens");
    if (!screensValue.isObject())
        return;
    QJsonObject screensObject = screensValue.toObject();
    QStringList screensKeys = screensObject.keys();

    for (auto screenKey : screensKeys) {
        QJsonValue screenValue = screensObject.value(screenKey);
        if (!screenValue.isObject())
            continue;
        QJsonObject screenObject = screenValue.toObject();
        QStringList screenKeys = screenObject.keys();
        qCDebug(appLog) << "Processing screen:" << screenKey;

        if (!screenKeys.contains("Extend"))
            continue;
        QJsonValue extendValue = screenObject.value("Extend");
        if (!extendValue.isObject())
            continue;
        QJsonObject extendObject = extendValue.toObject();
        QStringList extendKeys = extendObject.keys();

        if (!extendKeys.contains("Monitors"))
            continue;
        QJsonValue monitorsValue = extendObject.value("Monitors");
        if (!monitorsValue.isArray())
            continue;
        QJsonArray monitorsObject = monitorsValue.toArray();

        for (auto monitor : monitorsObject) {
            if (!monitor.isObject())
                continue;

            QJsonObject monitorObject = monitor.toObject();
            QStringList monitorKeys = monitorObject.keys();
            if (!monitorKeys.contains("Name") || !monitorKeys.contains("RefreshRate"))
                continue;
            qCDebug(appLog) << "Processing monitor:" << monitorObject.value("Name").toString();

            QJsonValue ss = monitorObject.value("RefreshRate");
            QMap<QString, QString> infoMap;
            infoMap.insert("Name", monitorObject.value("Name").toString());
            QString refreshRate = QString::number(monitorObject.value("RefreshRate").toDouble(), 'f', 2);
            infoMap.insert("RefreshRate", refreshRate);
            lstMap.append(infoMap);
        }
    }
}

void ThreadExecXrandr::getMonitorInfoFromXrandrVerbose()
{
    qCDebug(appLog) << "Getting monitor info from xrandr verbose";

    QList<QMap<QString, QString>> lstMap;
    loadXrandrVerboseInfo(lstMap, "xrandr --verbose");
    if (Common::specialComType == Common::SpecialComputerType::NormalCom) {
        std::reverse(lstMap.begin(), lstMap.end());
    }
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setMonitorInfoFromXrandr((*it)["mainInfo"], (*it)["edid"], (*it)["rate"]);
    }
}

struct MonitorResolution {
    uint32_t index;
    uint16_t width;
    uint16_t height;
    double refreshRate;
};

const QDBusArgument &operator>>(const QDBusArgument &argument, MonitorResolution &resolution)
{
    argument.beginStructure();
    argument >> resolution.index;
    argument >> resolution.width;
    argument >> resolution.height;
    argument >> resolution.refreshRate;
    argument.endStructure();
    return argument;
}
void ThreadExecXrandr::getResolutionFromDBus(QMap<QString, QString> &lstMap)
{
    qCDebug(appLog) << "Getting resolution from DBus";

    QDBusInterface displayInterface(DISPLAY_DAEMON_SERVICE_NAME, DISPLAY_DAEMON_SERVICE_PATH, DISPLAY_DAEMON_INTERFACE, QDBusConnection::sessionBus());
    if (!displayInterface.isValid())
        return;

    QVariant monitors = displayInterface.property("Monitors");

    if (!monitors.isValid())
        return;
    m_monitorLst.clear();
    QList<QDBusObjectPath> monitorList = monitors.value<QList<QDBusObjectPath> >();
    int maxResolutionWidth = -1, maxResolutionHeight = -1, minResolutionWidth = -1, minResolutionHeight = -1;
    for (auto monitor : monitorList) {
        if (monitor.path().isEmpty())
            continue;
        qCDebug(appLog) << "Processing monitor:" << monitor.path();

        m_monitorLst << monitor.path();
        QDBusInterface monitorEnabledInterface(DISPLAY_DAEMON_SERVICE_NAME, monitor.path(), DISPLAY_MONITOR_INTERFACE, QDBusConnection::sessionBus());
        if (!monitorEnabledInterface.isValid()) {
            qCWarning(appLog) << "Monitor interface is not valid for" << monitor.path();
            continue;
        }

        QVariant enbaled = monitorEnabledInterface.property("Enabled");
        if (!enbaled.isValid() || !enbaled.toBool()) {
            qCDebug(appLog) << "Monitor is not enabled:" << monitor.path();
            continue;
        }

        QDBusInterface monitorInterface(DISPLAY_DAEMON_SERVICE_NAME, monitor.path(), DISPLAY_PROPERTIES_INTERFACE, QDBusConnection::sessionBus());
        if (!monitorInterface.isValid()) {
            qCWarning(appLog) << "Monitor properties interface is not valid for" << monitor.path();
            continue;
        }

        QDBusMessage replay = monitorInterface.call("Get", DISPLAY_MONITOR_INTERFACE, "Modes");
        QVariant v =  replay.arguments().first();
        qCDebug(appLog) << v.value<QDBusVariant>().variant();
        QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();
        arg.beginArray();
        int curMaxResolutionWidth = -1, curMaxResolutionHeight = -1;
        while (!arg.atEnd()) {
            MonitorResolution resolution;
            arg >> resolution;
            qCDebug(appLog) << "Found resolution:" << resolution.width << "x" << resolution.height << "@" << resolution.refreshRate;
            if (curMaxResolutionWidth == -1) {
                curMaxResolutionWidth = resolution.width;
                curMaxResolutionHeight = resolution.height;
            } else {
                int curResolution = static_cast<int>(resolution.width) * static_cast<int>(resolution.height);
                if (curResolution > (curMaxResolutionWidth * curMaxResolutionHeight)) {
                    curMaxResolutionWidth = resolution.width;
                    curMaxResolutionHeight = resolution.height;
                }
            }
            if (minResolutionWidth == -1) {
                minResolutionWidth = resolution.width;
                minResolutionHeight = resolution.height;
            } else {
                int curResolution = static_cast<int>(resolution.width) * static_cast<int>(resolution.height);
                if (curResolution < (minResolutionWidth * minResolutionHeight)) {
                    minResolutionWidth = resolution.width;
                    minResolutionHeight = resolution.height;
                }
            }
        }
        if (maxResolutionWidth == -1) {
            maxResolutionWidth = curMaxResolutionWidth;
            maxResolutionHeight = curMaxResolutionHeight;
        } else {
            maxResolutionWidth += curMaxResolutionWidth;
            maxResolutionHeight += curMaxResolutionHeight;
        }
    }

    if (maxResolutionWidth != -1) {
        lstMap.insert("maxResolution", QString("%1×%2").arg(maxResolutionWidth).arg(maxResolutionHeight));
        lstMap.insert("minResolution", QString("%1×%2").arg(minResolutionWidth).arg(minResolutionHeight));
    }
}

void ThreadExecXrandr::getGpuInfoFromXrandr()
{
    qCDebug(appLog) << "Getting GPU info from xrandr";

    QList<QMap<QString, QString>> lstMap;
    loadXrandrInfo(lstMap, "xrandr");

    // 通过dbus获取最大最小分辨率
    QMap<QString, QString> dbusMap;
    getResolutionFromDBus(dbusMap);
    for (auto &lstInfo : lstMap) {
        if (dbusMap.contains("minResolution")) {
            lstInfo["minResolution"] = dbusMap["minResolution"];
            qCDebug(appLog) << "Set min resolution from DBus:" << dbusMap["minResolution"];
        }
        if (dbusMap.contains("maxResolution")) {
            lstInfo["maxResolution"] = dbusMap["maxResolution"];
            qCDebug(appLog) << "Set max resolution from DBus:" << dbusMap["maxResolution"];
        }
    }

    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setGpuInfoFromXrandr(*it);
    }
}

void ThreadExecXrandr::getResolutionRateFromDBus(QList<QMap<QString, QString> > &lstMap)
{
    qCDebug(appLog) << "Getting resolution rate from DBus";

    QDBusInterface displayInterface(DISPLAY_DAEMON_SERVICE_NAME, DISPLAY_DAEMON_SERVICE_PATH, DISPLAY_DAEMON_INTERFACE, QDBusConnection::sessionBus());
    if (!displayInterface.isValid())
        return;

    QVariant monitors = displayInterface.property("Monitors");

    if (!monitors.isValid())
        return;

    QList<QDBusObjectPath> monitorList = monitors.value<QList<QDBusObjectPath> >();
    for (auto monitor : monitorList) {
        if (monitor.path().isEmpty())
            continue;
        qCDebug(appLog) << "Processing monitor:" << monitor.path();

        QDBusInterface monitorEnabledInterface(DISPLAY_DAEMON_SERVICE_NAME, monitor.path(), DISPLAY_MONITOR_INTERFACE, QDBusConnection::sessionBus());
        if (!monitorEnabledInterface.isValid()) {
            qCWarning(appLog) << "Monitor interface is not valid for" << monitor.path();
            continue;
        }

        QVariant enbaled = monitorEnabledInterface.property("Enabled");
        if (!enbaled.isValid() || !enbaled.toBool()) {
            qCDebug(appLog) << "Monitor is not enabled:" << monitor.path();
            continue;
        }
        QVariant tconnected = monitorEnabledInterface.property("Connected");
        QVariant tmanufacture = monitorEnabledInterface.property("Manufacturer");
        QVariant tname = monitorEnabledInterface.property("Name");

        // QVariant rep_currentRes = monitorEnabledInterface.property("CurrentMode");

        QDBusInterface monitorInterface(DISPLAY_DAEMON_SERVICE_NAME, monitor.path(), DISPLAY_PROPERTIES_INTERFACE, QDBusConnection::sessionBus());
        if (!monitorInterface.isValid())
            continue;

        QDBusMessage replay = monitorInterface.call("Get", DISPLAY_MONITOR_INTERFACE, "CurrentMode");   // "com.deepin.daemon.Display.Monitor","CurrentMode"
        QVariant v =  replay.arguments().first();
        qCDebug(appLog) << v.value<QDBusVariant>().variant();
        QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();

        int curResolutionWidth = -1, curResolutionHeight = -1;
        double resRate = 0;
        {
           MonitorResolution resolution;
           arg >> resolution;

            curResolutionWidth = resolution.width;
            curResolutionHeight = resolution.height;
            resRate = resolution.refreshRate;
            QMap<QString,QString>infoMap;
            QString tmpS = QString("%1×%2@").arg(curResolutionWidth).arg(curResolutionHeight)  + QString::number(resRate, 'f', 2);
            infoMap.insert("CurResolution", tmpS + "Hz");
            infoMap.insert("Name", tname.toString());
            infoMap.insert("Display Input", tname.toString());
            infoMap.insert("Manufacture", tmanufacture.toString());
            lstMap.append(infoMap);
        }  //end of while
    }  //end of for

    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;
        // qCDebug(appLog) << "Setting monitor info from DBus for:" << (*it)["Name"];

        DeviceManager::instance()->setMonitorInfoFromDbus(*it);
    }
}
