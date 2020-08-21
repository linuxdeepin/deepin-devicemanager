#include "PageBoardInfo.h"

#include <DApplicationHelper>
#include <DPalette>

#include "DeviceInfo.h"
#include "DeviceBios.h"
#include "TextBrowser.h"
#include "RichTextDelegate.h"
#include "PageTableWidget.h"

#include <QTableWidgetItem>

PageBoardInfo::PageBoardInfo(QWidget *parent)
    : PageSingleInfo(parent)
    , mp_ItemDelegate(new RichTextDelegate(this))
{

}

void PageBoardInfo::updateInfo(const QList<DeviceBaseInfo *> &lst)
{
    clearContent();

    DeviceBaseInfo *board = nullptr;
    QList<DeviceBaseInfo *> lstOther;
    foreach (DeviceBaseInfo *info, lst) {
        DeviceBios *bios = dynamic_cast<DeviceBios *>(info);
        if (!bios) {continue;}

        if (bios->isBoard()) {
            board = info;
        } else {
            lstOther.append(info);
        }
    }

    if (!board) {return;}

    QList<QPair<QString, QString>> baseInfoMap = board->getBaseAttribs();
    QList<QPair<QString, QString>> otherInfoMap = board->getOtherAttribs();
    baseInfoMap = baseInfoMap + otherInfoMap;
    loadDeviceInfo(lstOther, baseInfoMap);
}

void PageBoardInfo::loadDeviceInfo(const QList<QPair<QString, QString>> &lst, int limiteNum)
{
    if (lst.size() < 1) {
        return;
    }

    int row = lst.size();
    if (mp_Content) {
        mp_Content->setLimitRow(limiteNum);
    }
    mp_Content->setColumnAndRow(row + 1, 2);

    for (int i = 0; i < row; ++i) {
        QTableWidgetItem *itemFirst = new QTableWidgetItem(lst[i].first);
        mp_Content->setItem(i, 0, itemFirst);
        QTableWidgetItem *itemSecond = new QTableWidgetItem(lst[i].second);
        mp_Content->setItem(i, 1, itemSecond);
        if (lst[i].second.contains("\n")) {
            QStringList lstStr = lst[i].second.split("\n");
            mp_Content->setRowHeight(i, 20 * (lstStr.size() + 2));
        }
    }
}

void PageBoardInfo::loadDeviceInfo(const QList<DeviceBaseInfo *> &devices, const QList<QPair<QString, QString>> &lst)
{
    if (lst.size() < 1) {
        return;
    }

    int limitSize = lst.size();
    if (mp_Content) {
        mp_Content->setLimitRow(limitSize);
    }

    int row = limitSize + devices.size();
    mp_Content->setColumnAndRow(row + 1, 2);

    for (int i = 0; i < limitSize; ++i) {
        QTableWidgetItem *itemFirst = new QTableWidgetItem(lst[i].first);
        mp_Content->setItem(i, 0, itemFirst);
        QTableWidgetItem *itemSecond = new QTableWidgetItem(lst[i].second);
        mp_Content->setItem(i, 1, itemSecond);
    }

    QList<QPair<QString, QString>> pairs;
    getOtherInfoPair(devices, pairs);
    for (int i = limitSize; i < row; ++i) {
        mp_Content->setItemDelegateForRow(i, mp_ItemDelegate);
        QTableWidgetItem *itemFirst = new QTableWidgetItem(pairs[i - limitSize].first);
        mp_Content->setItem(i, 0, itemFirst);

        QTableWidgetItem *itemSecond = new QTableWidgetItem(pairs[i - limitSize].second);
        mp_Content->setItem(i, 1, itemSecond);

//        int size = 0;
//        QStringList strList = pairs[i - limitSize].second.split("\n");
//        QStringList::iterator it = strList.begin();
//        for (; it != strList.end(); ++it) {
//            QStringList attris = (*it).split("  /  \t\t");
//            size += attris.size();
//        }
//        mp_Content->setRowHeight(i, size * 30);
    }
}

void PageBoardInfo::getOtherInfoPair(const QList<DeviceBaseInfo *> &lst, QList<QPair<QString, QString>> &lstPair)
{
    foreach (DeviceBaseInfo *dev, lst) {
        DeviceBios *bios = dynamic_cast<DeviceBios *>(dev);
        if (!bios) {continue;}
        QPair<QString, QString> pair;
        pair.first = bios->name();
        getValueInfo(bios, pair);
        lstPair.append(pair);
    }
}

void PageBoardInfo::getValueInfo(DeviceBaseInfo *device, QPair<QString, QString> &pair)
{
    QList<QPair<QString, QString>> baseInfoMap = device->getBaseAttribs();
    QList<QPair<QString, QString>> otherInfoMap = device->getOtherAttribs();
    baseInfoMap = baseInfoMap + otherInfoMap;
    QList<QPair<QString, QString>>::iterator it = baseInfoMap.begin();
    for (; it != baseInfoMap.end(); ++it) {
        pair.second += (*it).first;
        pair.second += ":";
        pair.second += (*it).second;
        pair.second += "\n";
    }
    pair.second.replace(QRegExp("\n$"), "");
}
