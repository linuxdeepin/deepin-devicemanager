#include "ThreadExecXrandr.h"

#include <QProcess>
#include <QDebug>

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
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        if (line.startsWith("Screen")) {
            lstMap.append(QMap<QString, QString>());
            QRegExp re(".*([0-9]{1,5}\\sx\\s[0-9]{1,5}).*([0-9]{1,5}\\sx\\s[0-9]{1,5}).*([0-9]{1,5}\\sx\\s[0-9]{1,5}).*");
            if (re.exactMatch(line)) {
                lstMap[lstMap.count() - 1].insert("minResolution", re.cap(1));
                lstMap[lstMap.count() - 1].insert("curResolution", re.cap(2));
                lstMap[lstMap.count() - 1].insert("maxResolution", re.cap(3));
            }
            continue;
        }

        if (line.startsWith("HDMI")) {
            lstMap[lstMap.count() - 1].insert("HDMI", "Enable");
        } else if (line.startsWith("VGA")) {
            lstMap[lstMap.count() - 1].insert("VGA", "Enable");
        } else if (line.startsWith("DP") || line.startsWith("DisplayPort")) {
            lstMap[lstMap.count() - 1].insert("DP", "Enable");
        } else if (line.startsWith("eDP")) {
            lstMap[lstMap.count() - 1].insert("eDP", "Enable");
        } else if (line.startsWith("DVI")) {
            lstMap[lstMap.count() - 1].insert("DVI", "Enable");
        } else if (line.startsWith("DigitalOutput")) {
            lstMap[lstMap.count() - 1].insert("DigitalOutput", "Enable");
        }
    }
}

void ThreadExecXrandr::loadXrandrVerboseInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd)
{
    QString deviceInfo;
    runCmd(deviceInfo, cmd);

    QStringList lines = deviceInfo.split("\n");
    QStringList::iterator it = lines.begin();
    for (; it != lines.end(); ++it) {
        if ((*it).startsWith("Screen"))
            continue;

        //获取 HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm
        QRegExp reg("^[A-Za-z].*");
        if (reg.exactMatch(*it) && !(*it).contains("disconnected")) {
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
        QRegExp reEdid("^[\\t]{2}([0-9a-f]{32}).*");
        if (reEdid.exactMatch(*it)) {
            while (true) {
                edid.append(reEdid.cap(1));
                edid.append("\n");
                if (++it == lines.end())
                    return;
                if (!reEdid.exactMatch(*it)) {
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
            QRegExp regRate(".*([0-9]{2}\\.[0-9]{2}Hz).*");
            if (regRate.exactMatch(*it))
                last.insert("rate", regRate.cap(1));
        }
    }
}

void ThreadExecXrandr::getMonitorInfoFromXrandrVerbose()
{
    QList<QMap<QString, QString>> lstMap;
    loadXrandrVerboseInfo(lstMap, "xrandr --verbose");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setMonitorInfoFromXrandr((*it)["mainInfo"], (*it)["edid"], (*it)["rate"]);
    }
}

void ThreadExecXrandr::getGpuInfoFromXrandr()
{
    QList<QMap<QString, QString>> lstMap;
    loadXrandrInfo(lstMap, "xrandr");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setGpuInfoFromXrandr(*it);
    }
}
