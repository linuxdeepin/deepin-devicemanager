// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "WakeupUtils.h"
#include "EnableSqlManager.h"

#include <QStringList>
#include <QMap>
#include <QFile>
#include <QDebug>

#define LEAST_NUM 10

WakeupUtils::WakeupUtils()
{

}

void WakeupUtils::updateWakeupDeviceInfo(const QString& info)
{
    QStringList items = info.split("\n\n");
    foreach(const QString& item,items){
        QMap<QString,QString> mapItem;
        if(!getMapInfo(item,mapItem))
            continue;

        // Unique ID
        if(mapItem.find("Unique ID") == mapItem.end())
            continue;

        // 查找数据库，判断是否存在相同 Unique ID
        if(!EnableSqlManager::getInstance()->isWakeupUniqueIdExisted(mapItem["Unique ID"]))
            continue;

        // 判断数据库里面的记录状态更新信息
        QString wp;
        if(wakeupPath(mapItem["SysFS ID"].isEmpty()?getPS2Syspath(mapItem["Device Files"]):mapItem["SysFS ID"],wp)){
            bool wakeup = EnableSqlManager::getInstance()->isWakeup(mapItem["Unique ID"]);
            writeWakeupFile(wp,wakeup);
        }
    }
}

bool WakeupUtils::wakeupPath(const QString& syspath, QString& wakeuppath)
{
    int index = syspath.lastIndexOf('/');
    if(index < 1){
        return false;
    }

    wakeuppath = QString("/sys") + syspath.left(index) + QString("/power/wakeup");
    if(!QFile::exists(wakeuppath))
        return false;
    return true;
}

bool WakeupUtils::writeWakeupFile(const QString& path, bool wakeup)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadWrite)){
        return false;
    }

    qint64 size = -1;
    if(wakeup)
        size = file.write("enabled");
    else
        size = file.write("disabled");
    file.close();

    if(size < 1)
        return false;
    return true;
}

WakeupUtils::EthStatus WakeupUtils::wakeOnLanIsOpen(const QString& logicalName)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0){
        return ES_SOCKET_FAILED;
    }

    struct ifreq ifr;
    struct ethtool_wolinfo wolinfo;
    memset(&ifr,0,sizeof (ifr));
    strcpy(ifr.ifr_name,logicalName.toStdString().c_str());
    wolinfo.cmd = ETHTOOL_GWOL;
    ifr.ifr_data = reinterpret_cast<char*>(&wolinfo);
    if(0 != ioctl(fd, SIOCETHTOOL, &ifr)){
        return ES_IOCTL_ERROR;
    }

    if(47 != wolinfo.supported){
        return ES_NOT_SUPPORT_WAKE_ON;
    }

    if(0 == wolinfo.wolopts){
        return ES_WAKE_ON_CLOSE;
    }else if(32 == wolinfo.wolopts){
        return ES_WAKE_ON_OPEN;
    }else{
        return ES_WAKE_ON_UNKNOW;
    }
}

bool WakeupUtils::setWakeOnLan(const QString& logicalName, bool open)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
        return false;

    struct ifreq ifr;
    struct ethtool_wolinfo wolinfo;
    memset(&ifr,0,sizeof (ifr));
    strcpy(ifr.ifr_name,logicalName.toStdString().c_str());
    wolinfo.cmd = ETHTOOL_SWOL;
    if(open)
        wolinfo.wolopts = 0 | WAKE_MAGIC;
    else
        wolinfo.wolopts = 0;
    ifr.ifr_data = reinterpret_cast<char*>(&wolinfo);

    if(0 == ioctl(fd, SIOCETHTOOL, &ifr))
        return true;
    return false;
}

bool WakeupUtils::getMapInfo(const QString& item,QMap<QString,QString>& mapInfo)
{
    QStringList lines = item.split("\n");
    // 行数太少则为无用信息
    if(lines.size() <= LEAST_NUM){
        return false;
    }

    foreach(const QString& line,lines){
        QStringList words = line.split(": ");
        if(words.size() != 2)
            continue;
        mapInfo.insert(words[0].trimmed(),words[1].replace("\"","").trimmed());
    }

    if(mapInfo["Hardware Class"] != "keyboard" && mapInfo["Hardware Class"] != "mouse")
        return false;

    return true;
}

QString WakeupUtils::getPS2Syspath(const QString& dfs)
{
    // 获取 dfs 中的 event
    QRegExp regdfs = QRegExp(".*(event[0-9]{1,2}).*");
    if(!regdfs.exactMatch(dfs)){
        return "";
    }
    QString eventdfs = regdfs.cap(1);

    QFile file("/proc/bus/input/devices");
    if(!file.open(QIODevice::ReadOnly))
        return "";

    QString info = file.readAll();
    QStringList lstDevices = info.split("\n\n");
    foreach(const QString& item,lstDevices){
        QStringList lines = item.split("\n");
        QString sysfs = "";
        QString event = "";
        foreach(const QString& line,lines){
            if(line.startsWith("S:")){
                sysfs = line;
                continue;
            }
            QRegExp reg = QRegExp("H: Handlers=.*(event[0-9]{1,2}).*");
            if(reg.exactMatch(line)){
                event = reg.cap(1);
            }
        }

        if(!event.isEmpty() && !sysfs.isEmpty()){
            if(event == eventdfs){
                QRegExp regfs;
                if(sysfs.contains("i2c_designware"))
                    regfs = QRegExp("S: Sysfs=(.*)/input/input[0-9]{1,2}");
                else
                    regfs = QRegExp("S: Sysfs=(.*)/input[0-9]{1,2}");
                if(regfs.exactMatch(sysfs)){
                    return regfs.cap(1);
                }
            }
        }
    }

    return "";
}
