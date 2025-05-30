// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detectthread.h"
#include "monitorusb.h"
#include "DDLog.h"

#include <QLoggingCategory>
#include <QProcess>
#include <QCryptographicHash>

#define LEAST_NUM 10

using namespace DDLog;

DetectThread::DetectThread(QObject *parent)
    : QThread(parent)
    , mp_MonitorUsb(new MonitorUsb())
{
    qCDebug(appLog) << "Initializing DetectThread";
    // 连接槽函数
    connect(mp_MonitorUsb, SIGNAL(usbChanged()), this, SLOT(slotUsbChanged()), Qt::QueuedConnection);

    QMap<QString, QMap<QString, QString>> usbInfo;
    qCDebug(appLog) << "Getting initial USB info";
    curHwinfoUsbInfo(usbInfo);
    updateMemUsbInfo(usbInfo);
    qCDebug(appLog) << "Initial USB devices count:" << usbInfo.size();
}

void DetectThread::run()
{
    qCDebug(appLog) << "Starting USB detection thread";
    if (mp_MonitorUsb) {
        qCDebug(appLog) << "Starting USB monitor";
        mp_MonitorUsb->monitor();
    }
    qCDebug(appLog) << "USB detection thread finished";
}

void DetectThread::setWorkingFlag(bool flag)
{
    qCDebug(appLog) << "Setting working flag to:" << flag;
    mp_MonitorUsb->setWorkingFlag(flag);
    if (flag && !isRunning()) {
        qCDebug(appLog) << "Starting thread due to flag change";
        start();
    }
}

void DetectThread::slotUsbChanged()
{
    // 当监听到新的usb时，内核需要加载usb信息，而上层应用需要在内核处理之后获取信息
    // 为了确保缓存信息之前，内核已经处理完毕，先判断内核是否处理完信息，且判断时间不能多于10s
    qint64 begin = QDateTime::currentMSecsSinceEpoch();
    qint64 end = begin;
    while ((end - begin) <= 10000) {
        if (isUsbDevicesChanged())
            break;
        sleep(1);
        end = QDateTime::currentMSecsSinceEpoch();
    }
    qCInfo(appLog) << " 此次判断插拔是否完成的时间为 ************ " << QDateTime::currentMSecsSinceEpoch() - begin;
    emit usbChanged();
}

bool DetectThread::isUsbDevicesChanged()
{
    qCDebug(appLog) << "Checking for USB device changes";
    QMap<QString, QMap<QString, QString>> curUsbInfo;
    curHwinfoUsbInfo(curUsbInfo);

    // 拔出的时候，如果当前的usb设备个数小于m_MapUsbInfo的个数则返回true
    if (curUsbInfo.size() < m_MapUsbInfo.size()) {
        qCDebug(appLog) << "USB device removed, count changed from" << m_MapUsbInfo.size() << "to" << curUsbInfo.size();
        updateMemUsbInfo(curUsbInfo);
        return true;
    }

    // 数量一样或curUsbInfo的大小大于m_MapUsbInfo的大小，则一个一个的比较
    // 如果curUsbInfo里面的在m_MapUsbInfo里面找不到则说明内核信息还没有处理完
    foreach (const QString &key, curUsbInfo.keys()) {
        if (m_MapUsbInfo.find(key) != m_MapUsbInfo.end())
            continue;
        if ("disk" == curUsbInfo[key]["Hardware Class"]
                && curUsbInfo[key].find("Capacity") == curUsbInfo[key].end())
            continue;
        qCDebug(appLog) << "New USB device detected:" << key;
        updateMemUsbInfo(curUsbInfo);
        return true;
    }
    qCDebug(appLog) << "No USB device changes detected";
    return false;
}

void DetectThread::updateMemUsbInfo(const QMap<QString, QMap<QString, QString>> &usbInfo)
{
    m_MapUsbInfo.clear();
    m_MapUsbInfo = usbInfo;
}

void DetectThread::curHwinfoUsbInfo(QMap<QString, QMap<QString, QString>> &usbInfo)
{
    qCDebug(appLog) << "Getting current USB info from hwinfo";
    QProcess process;
    process.start("hwinfo --usb --keyboard --mouse");
    process.waitForFinished(-1);
    QString info = process.readAllStandardOutput();
    qCDebug(appLog) << "hwinfo output length:" << info.length();

    QStringList items = info.split("\n\n");
    qCDebug(appLog) << "Found" << items.size() << "USB devices in hwinfo output";
    
    foreach (const QString &item, items) {
        QMap<QString, QString> mapItem;
        if (!getMapInfo(item, mapItem))
            continue;

        QString uniqueID;
        if (mapItem.contains("Unique ID"))
            uniqueID = mapItem["Unique ID"];
        if (mapItem.contains("Vendor") && mapItem.contains("Device") && (mapItem.contains("SysFS ID") || mapItem.contains("SysFS Device Link"))
                && mapItem["Vendor"].contains("0x") && mapItem["Device"].contains("0x")) {
            QStringList vendorlist = mapItem["Vendor"].split(" ");
            QStringList devicelist = mapItem["Device"].split(" ");
            if (vendorlist.size() > 1 && devicelist.size() > 1  && ((mapItem.contains("SysFS ID") && !mapItem["SysFS ID"].isEmpty())
                                                                    || (mapItem.contains("SysFS Device Link") && !mapItem["SysFS Device Link"].isEmpty()))) {
                QString valueStr = vendorlist[1].trimmed() + devicelist[1].remove("0x", Qt::CaseSensitive).trimmed();
                QCryptographicHash Hash(QCryptographicHash::Md5);
                QByteArray buf;
                buf.append(valueStr.toUtf8());
                if (mapItem.contains("SysFS Device Link") && !mapItem["SysFS Device Link"].isEmpty()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                    buf.append(mapItem["SysFS Device Link"].trimmed());
#else
                    buf.append(mapItem["SysFS Device Link"].trimmed().toUtf8());
#endif
                } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                    buf.append(mapItem["SysFS ID"].trimmed());
#else
                    buf.append(mapItem["SysFS ID"].trimmed().toUtf8());
#endif
                }
                Hash.addData(buf);
                uniqueID = QString::fromStdString(Hash.result().toBase64().toStdString());
                qCDebug(appLog) << "Generated Unique ID:" << uniqueID;
            }
        }

        // 使用 Unique ID 作为唯一标识
        usbInfo.insert(uniqueID, mapItem);
    }
}

bool DetectThread::getMapInfo(const QString &item, QMap<QString, QString> &mapInfo)
{
    QStringList lines = item.split("\n");
    // 行数太少则为无用信息
    if (lines.size() <= LEAST_NUM) {
        return false;
    }

    foreach (const QString &line, lines) {
        QStringList words = line.split(": ");
        if (words.size() != 2)
            continue;
        mapInfo.insert(words[0].trimmed(), words[1].trimmed());
    }

    // hub为usb接口，可以直接过滤
    if ("hub" == mapInfo["Hardware Class"]) {
        return false;
    }

    return true;
}
