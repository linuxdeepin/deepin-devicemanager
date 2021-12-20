#include "EnableUtils.h"
#include "EnableSqlManager.h"

#include <QStringList>
#include <QMap>
#include <QFile>
#include <QProcess>

#define LEAST_NUM 10

EnableUtils::EnableUtils()
{

}

void EnableUtils::disableOutDevice(const QString& info)
{
    QStringList items = info.split("\n\n");
    foreach(const QString& item,items){
        QMap<QString,QString> mapItem;
        if(!getMapInfo(item,mapItem))
            continue;

        // 获取设备的唯一标识
        // 网卡的唯一标识为网卡的物理地址
        // 有序列号id的设备使用序列号id作为唯一标识
        QString uniqueID;
        if(mapItem.find("Permanent HW Address") != mapItem.end()){
            uniqueID = mapItem["Permanent HW Address"];
        }else if(mapItem.find("Serial ID") != mapItem.end()){
            uniqueID = mapItem["Serial ID"];
        }else{
            uniqueID = mapItem["Module Alias"];
            uniqueID.replace(QRegExp("[0-9a-zA-Z]{10}$"), "");
        }
        if (uniqueID.isEmpty()) {
            continue;
        }

        // 获取设备信息路径
        // 有 SysFS Device Link 的使用 SysFS Device Link
        // 没有 SysFS Device Link 的使用 SysFS ID
        QString path;
        if(mapItem.find("SysFS Device Link") != mapItem.end()){
            path = mapItem["SysFS Device Link"];
        }else{
            path = mapItem["SysFS ID"];
        }
        path.replace(QRegExp("[1-9]$"), "0");

        // 先判断设备是否被记录在数据库，如果在则禁用
        if(EnableSqlManager::getInstance()->uniqueIDExisted(uniqueID)){
            QFile file("/sys" + path + QString("/authorized"));
            if(!file.open(QIODevice::ReadWrite)){
                return;
            }
            file.write("0");
            file.close();
            // 更数据库信息，方式更换usb接口
            EnableSqlManager::getInstance()->updateDataToAuthorizedTable(uniqueID,path);
        }
    }
}

void EnableUtils::disableOutDevice()
{
    QProcess process;
    process.start("hwinfo --usb");
    process.waitForFinished(-1);
    QString info = process.readAllStandardOutput();
    EnableUtils::disableOutDevice(info);
}

void EnableUtils::disableInDevice()
{
    // 由于rescan会将所有的remove数据都回复，因此需要重新禁用其它设备
    QStringList rpList;
    EnableSqlManager::getInstance()->removePathList(rpList);

    foreach (const QString &path, rpList) {
        QFile file("/sys" + path + QString("/remove"));
        if (file.open(QIODevice::WriteOnly)) {
            file.write("1");
            file.close();
        }else{
            // platform disable with reset
            if(path.contains("platform")){
                QFile filep("/sys" + path + QString("/reset"));
                if(filep.open(QIODevice::WriteOnly)){
                    filep.write("1");
                    filep.close();
                }
            }
        }
    }
}

bool EnableUtils::getMapInfo(const QString& item,QMap<QString,QString>& mapInfo)
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

    // hub为usb接口，可以直接过滤
    if(mapInfo["Hardware Class"] == "hub"){
        return false;
    }

    // 没有总线信息的设备可以过滤
    if(mapInfo.find("SysFS BusID") == mapInfo.end()){
        return false;
    }

    return true;
}
