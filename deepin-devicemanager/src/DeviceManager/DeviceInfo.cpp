#include "DeviceInfo.h"
#include "../commondefine.h"
#include<QDebug>
#include"DeviceManager.h"
#include <DApplication>
#include "../deviceinfoparser.h"

DWIDGET_USE_NAMESPACE

DeviceBaseInfo::DeviceBaseInfo()
{

}

DeviceBaseInfo::~DeviceBaseInfo()
{

}

const QMap<QString, QString> &DeviceBaseInfo::getOtherAttribs()const
{
    return m_MapOtherInfo;
}

void DeviceBaseInfo::addFilterKey(const QString &key)
{

    m_FilterKey.insert(key);
}

void DeviceBaseInfo::loadOtherDeviceInfo(const QMap<QString, QString> &mapInfo)
{
    QMap<QString, QString>::const_iterator it = mapInfo.begin();
    for (; it != mapInfo.end(); ++it) {
        QString k = DApplication::translate("QObject", it.key().trimmed().toStdString().data());
        if (m_FilterKey.find(k) != m_FilterKey.end()) {
            m_MapOtherInfo.insert(k, it.value().trimmed());
        }
    }
}

void DeviceBaseInfo::setAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite)
{
    if (mapInfo.find(key) == mapInfo.end()) {
        return;
    }
    if (mapInfo[key] == "" || mapInfo[key] == "0000") {
        return;
    }
    if (overwrite) {
        variable = mapInfo[key].trimmed();
    } else {
        if (variable.isEmpty()) {
            variable = mapInfo[key].trimmed();
        }

        if (variable.contains("Unknown", Qt::CaseInsensitive)) {
            variable = mapInfo[key].trimmed();
        }
    }
}
