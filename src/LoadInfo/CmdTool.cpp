#include "CmdTool.h"
#include <QProcess>

QMap<QString, QList<QMap<QString, QString> > > CmdTool::s_cmdInfo;

CmdTool::CmdTool()
{

}

QMap<QString, QList<QMap<QString, QString> > > &CmdTool::getCmdInfo()
{
    return s_cmdInfo;
}

void CmdTool::loadCmdInfo(const QString &cmd, const QString &key, const QString &paragraphSplit,  KeyValueSplit st, const QString &debugFile)
{
    if (cmd == "sudo lshw") {
        loadLshwInfo(cmd, paragraphSplit, debugFile);
    } else {
        if (s_cmdInfo.find(key) != s_cmdInfo.end()) {
            return;
        }
        QString deviceInfo;
        getDeviceInfo(cmd, deviceInfo, debugFile);
        QStringList items = deviceInfo.split(paragraphSplit);
        foreach (const QString &item, items) {
            QMap<QString, QString> mapInfo;
            getMapInfo(st, item, mapInfo);
            s_cmdInfo[key].append(mapInfo);
        }
    }
}

void CmdTool::loadLshwInfo(const QString &cmd, const QString &paragraphSplit, const QString &debugFile)
{
    QString deviceInfo;
    getDeviceInfo(cmd, deviceInfo, debugFile);
    QStringList items = deviceInfo.split(paragraphSplit);
    foreach (const QString &item, items) {
        QMap<QString, QString> mapInfo;
        if (item.startsWith("cpu")) {
            getMapInfoFromLshw(item, mapInfo);
            s_cmdInfo["lshw_cpu"].append(mapInfo);
        } else if (item.startsWith("disk")) {
            getMapInfoFromLshw(item, mapInfo);
            s_cmdInfo["lshw_disk"].append(mapInfo);
        } else if ((item.startsWith("memory") && !item.startsWith("memory UNCLAIMED")) || item.startsWith("bank:")) {
            getMapInfoFromLshw(item, mapInfo);
            s_cmdInfo["lshw_memory"].append(mapInfo);
        } else if (item.startsWith("display")) {
            getMapInfoFromLshw(item, mapInfo);
            s_cmdInfo["lshw_display"].append(mapInfo);
        } else if (item.startsWith("multimedia")) {
            getMapInfoFromLshw(item, mapInfo);
            s_cmdInfo["lshw_multimedia"].append(mapInfo);
        } else if (item.startsWith("network")) {
            getMapInfoFromLshw(item, mapInfo);
            s_cmdInfo["lshw_network"].append(mapInfo);
        } else if (item.startsWith("usb:")) {
            getMapInfoFromLshw(item, mapInfo);
            s_cmdInfo["lshw_usb"].append(mapInfo);
        }
    }
}

void CmdTool::getMapInfo(KeyValueSplit st, const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    switch (st) {
    case ST_Null:
        break;
    case ST_Common:
        getMapInfoFromCmd(info, mapInfo);
        break;
    case ST_lshw:
        getMapInfoFromLshw(info, mapInfo);
        break;
    case ST_Hwinfo:
        getMapInfoFromHwinfo(info, mapInfo);
        break;
    case ST_dmidecode:
        getMapInfoFromDmidecode(info, mapInfo);
        break;
    }
}
void CmdTool::getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() == 2) {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}
void CmdTool::getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() != 2) {
            continue;
        }
        // && words[0].contains("configuration") == false && words[0].contains("resources") == false
        // 将configuration的内容进行拆分
        if (words[0].contains("configuration") == true) {
            QStringList keyValues = words[1].split(" ");

            for (QStringList::iterator it = keyValues.begin(); it != keyValues.end(); ++it) {
                QStringList attr = (*it).split("=");
                if (attr.size() != 2) {
                    continue;
                }
                mapInfo.insert(attr[0].trimmed(), attr[1].trimmed());
            }
        } else if (words[0].contains("resources") == true) {
            QStringList keyValues = words[1].split(" ");

            for (QStringList::iterator it = keyValues.begin(); it != keyValues.end(); ++it) {
                QStringList attr = (*it).split(":");
                if (attr.size() != 2) {
                    continue;
                }
                if (mapInfo.find(attr[0].trimmed()) != mapInfo.end()) {
                    mapInfo[attr[0].trimmed()] += QString("  ");
                }
                mapInfo[attr[0].trimmed()] += attr[1].trimmed();
            }
        } else {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}
void CmdTool::getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() != 2) {
            continue;
        }
        if (mapInfo.find(words[0].trimmed()) != mapInfo.end()) {
            mapInfo[words[0].trimmed()] += QString(" ");
        }
        QRegExp re(".*\"(.*)\".*");
        if (re.exactMatch(words[1].trimmed())) {
            mapInfo[words[0].trimmed()] += re.cap(1);
        } else {
            if (words[0].trimmed() == "Resolution") {
                mapInfo[words[0].trimmed()] += words[1].trimmed();
            } else {
                mapInfo[words[0].trimmed()] = words[1].trimmed();
            }
        }
    }
}
void CmdTool::getMapInfoFromDmidecode(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList lines = info.split("\n");
    QString lasKey;
    foreach (const QString &line, lines) {
        if (line.isEmpty()) {
            continue;
        }
        QStringList words = line.split(": ");
        if (words.size() == 1 && words[0].endsWith(":")) {
            lasKey = words[0].replace(QRegExp(":$"), "");
            mapInfo.insert(lasKey.trimmed(), " ");
        } else if (words.size() == 1 && !lasKey.isEmpty()) {
            mapInfo[lasKey.trimmed()] += words[0];
            mapInfo[lasKey.trimmed()] += "  /  ";
        } else if (words.size() == 2) {
            lasKey = "";
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}

bool CmdTool::getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile)
{
    if (!deviceInfo.isEmpty()) {
        return true;
    }
//    qint64 begin = QDateTime::currentMSecsSinceEpoch();
    if (false == executeProcess(command, deviceInfo)) {
        return false;
    }
//    qint64 end = QDateTime::currentMSecsSinceEpoch();
//    qDebug() << command << " ******************************* " << (end - begin) / 1000.0;
#ifdef TEST_DATA_FROM_FILE
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();
#endif

    return true;
}
bool CmdTool::executeProcess(const QString &cmd, QString &deviceInfo)
{
#ifdef TEST_DATA_FROM_FILE
    return true;
#endif
    if (false == cmd.startsWith("sudo")) {
        return runCmd(cmd, deviceInfo);
    }

    runCmd("id -un", deviceInfo);
    if (deviceInfo.trimmed() == "root") {
        return runCmd(cmd, deviceInfo);
    }

    QString newCmd = "pkexec deepin-devicemanager-authenticateProxy \"";
    newCmd += cmd;
    newCmd += "\"";
    newCmd.remove("sudo");
    return runCmd(newCmd, deviceInfo);
}
bool CmdTool::runCmd(const QString &proxy, QString &deviceInfo)
{
    QString key = "devicemanager";
    QString cmd = proxy;
    QProcess process_;
    int msecs = 10000;
    if (cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy")) {
        cmd = proxy + QString(" ") + key;
        msecs = -1;
    }

    process_.start(cmd);

    bool res = process_.waitForFinished(msecs);
    deviceInfo = process_.readAllStandardOutput();
    int exitCode = process_.exitCode();
    if (cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy") && (exitCode == 127 || exitCode == 126)) {
        //dError("Run \'" + cmd + "\' failed: Password Error! " + QString::number(exitCode) + "\n");
        return false;
    }

    if (res == false) {
        //dError("Run \'" + cmd + "\' failed\n");
    }

    return res;
}
