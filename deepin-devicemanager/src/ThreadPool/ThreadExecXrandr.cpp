#include "ThreadExecXrandr.h"

#include <QProcess>

#include <DeviceManager.h>

ThreadExecXrandr::ThreadExecXrandr(bool gpu)
    : m_Gpu(gpu)
{

}

void ThreadExecXrandr::run()
{
    if (m_Gpu) {
        getGpuInfoFromXrandr();
    } else {
        getMonitorInfoFromXrandrVerbose();
        getMonitorRefreshRateFromXrandr();
    }
}

void ThreadExecXrandr::runCmd(QString &info, const QString &cmd)
{
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    info = process.readAllStandardOutput();
}

void ThreadExecXrandr::loadXrandrInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd)
{
    QString deviceInfo;
    runCmd(deviceInfo, cmd);

    QMap<QString, QString> mapInfo;
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        // 刷新率
        QRegExp reResolution("^[\\s]{3}([0-9]{3,5}x[0-9]{3,5}).*([0-9]{2,3}.[0-9]{2,3}\\*).*");
        if (reResolution.exactMatch(line)) {
            QString rate = reResolution.cap(2).replace("*", "");
            mapInfo.insert("rate", rate);
        }

        // 最大,最小,当前分辨率
        if (line.startsWith("Screen")) {
            QRegExp re(".*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*");
            if (re.exactMatch(line)) {
                mapInfo["minResolution"] = re.cap(1);
                mapInfo["curResolution"] = re.cap(2);
                mapInfo["maxResolution"] = re.cap(3);
            }
        } else if (line.startsWith("HDMI")) {
            mapInfo["HDMI"] = "Enable";
        } else if (line.startsWith("VGA")) {
            mapInfo["VGA"] = "Enable";
        } else if (line.startsWith("DP") || line.startsWith("DisplayPort"))  {
            mapInfo["DP"] = "Enable";
        } else if (line.startsWith("eDP")) {
            mapInfo["eDP"] = "Enable";
        } else if (line.startsWith("DVI")) {
            mapInfo["DVI"] = "Enable";
        }
    }
    lstMap.append(mapInfo);
}

void ThreadExecXrandr::loadXrandrVerboseInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd)
{
    QString deviceInfo;
    runCmd(deviceInfo, cmd);

    QStringList lines = deviceInfo.split(QRegExp("\n"));
    QString mainInfo("");
    QString edid("");
    foreach (QString line, lines) {
        if (line.startsWith("Screen")) {
            continue;
        }
        QRegExp reResolution("^[\\s]{2}([0-9]{3,4}x[0-9]{3,4}).*");
        if (reResolution.exactMatch(line)) {
            continue;
        }

        // 主屏幕信息
        QRegExp reMain("^[a-zA-Z].*");
        if (reMain.exactMatch(line)) {
            if (!mainInfo.isEmpty()) {
                QMap<QString, QString> mapInfo;
                mapInfo.insert("mainInfo", mainInfo.trimmed());
                mapInfo.insert("edid", edid.trimmed());
                lstMap.append(mapInfo);
            }
            mainInfo = line;
            edid = "";
            continue;
        }

        // edid信息
        QRegExp reEdid("^[\\t]{2}[0-9]{1}.*");
        if (reEdid.exactMatch(line)) {
            edid.append(line.trimmed());
            edid.append("\n");
            continue;
        }
    }
    QMap<QString, QString> mapInfo;
    mapInfo.insert("mainInfo", mainInfo.trimmed());
    mapInfo.insert("edid", edid.trimmed());
    lstMap.append(mapInfo);
}

void ThreadExecXrandr::getMonitorInfoFromXrandrVerbose()
{
    QList<QMap<QString, QString>> lstMap;
    loadXrandrVerboseInfo(lstMap, "xrandr --verbose");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setMonitorInfoFromXrandr((*it)["mainInfo"], (*it)["edid"]);
    }
}

void ThreadExecXrandr::getMonitorRefreshRateFromXrandr()
{
    QList<QMap<QString, QString>> lstMap;
    loadXrandrInfo(lstMap, "xrandr");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setCurrentResolution((*it)["curResolution"], (*it)["rate"]);
    }
}

void ThreadExecXrandr::getGpuInfoFromXrandr()
{
    QList<QMap<QString, QString>> lstMap;
    loadXrandrInfo(lstMap, "xrandr");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setGpuInfoFromXrandr(*it);
    }
}
