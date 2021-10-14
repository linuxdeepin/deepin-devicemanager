#include "ThreadExecLshwNetwork.h"
#include "DeviceManager.h"

#include <QProcess>
#include <QMap>

ThreadExecLshwNetwork::ThreadExecLshwNetwork()
{

}

void ThreadExecLshwNetwork::run() {
    QString info;
    runCmd(info,"lshw -C network");

}

void ThreadExecLshwNetwork::runCmd(QString &info, const QString &cmd)
{
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    info = process.readAllStandardOutput();

//    QList<QMap<QString, QString>> lstMap;
    QStringList items = info.split("*-");
    foreach(const QString& item,items){
        QMap<QString, QString> mapInfo;
        getMapInfoFromCmd(item,mapInfo);
        DeviceManager::instance()->updateNetworkInfo(mapInfo);
    }
}

void ThreadExecLshwNetwork::getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() != 2)
            continue;

        // && words[0].contains("configuration") == false && words[0].contains("resources") == false
        // 将configuration的内容进行拆分
        if (true == words[0].contains("configuration")) {
            QStringList keyValues = words[1].split(" ");

            for (QStringList::iterator itKV = keyValues.begin(); itKV != keyValues.end(); ++itKV) {
                QStringList attr = (*itKV).split("=");
                if (attr.size() != 2)
                    continue;

                mapInfo.insert(attr[0].trimmed(), attr[1].trimmed());
            }
        } else if (true == words[0].contains("resources")) {
            QStringList keyValues = words[1].split(" ");

            for (QStringList::iterator itKV = keyValues.begin(); itKV != keyValues.end(); ++itKV) {
                QStringList attr = (*itKV).split(":");
                if (attr.size() != 2)
                    continue;

                if (mapInfo.find(attr[0].trimmed()) != mapInfo.end())
                    mapInfo[attr[0].trimmed()] += QString("  ");

                mapInfo[attr[0].trimmed()] += attr[1].trimmed();
            }
        } else {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}
