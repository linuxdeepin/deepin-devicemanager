#include "PageBoardInfo.h"

#include <DApplicationHelper>
#include <DPalette>
#include <DFontSizeManager>

#include <QTableWidgetItem>
#include <QDebug>

#include "DeviceInfo.h"
#include "DeviceBios.h"
#include "TextBrowser.h"
#include "RichTextDelegate.h"
#include "PageTableWidget.h"


PageBoardInfo::PageBoardInfo(QWidget *parent)
    : PageSingleInfo(parent)
    , mp_ItemDelegate(new RichTextDelegate(this))
{

}

void PageBoardInfo::updateInfo(const QList<DeviceBaseInfo *> &lst)
{
    mp_Device = lst[0];
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


        QFont font = DFontSizeManager::instance()->t8();
        QFontMetrics fm(font);
        int height = 0;
        QStringList strList = pairs[i - limitSize].second.split("\n");
        int fontHeight = fm.boundingRect(pairs[i - limitSize].second).height() + 6;

        foreach (const QString &str, strList) {
            QStringList lst = str.split(":");
            if (lst.size() == 2) {
                int width = fm.boundingRect(lst[0]).width();
                int num = width / 100;
                int num0 = width % 100;
                if (num0 == 0) {
                    num = num - 1;
                }
                height += num * fontHeight;
            }
            QStringList attris = str.split("  /  \t\t");
            height += attris.size() * fontHeight;
        }
        height += 20;
        mp_Content->setRowHeight(i, height);
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
