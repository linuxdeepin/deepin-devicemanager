// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PageBoardInfo.h"
#include "DeviceInfo.h"
#include "DeviceBios.h"
#include "TextBrowser.h"
#include "RichTextDelegate.h"
#include "PageTableWidget.h"
#include "DeviceManager.h"
#include "MacroDefinition.h"
#include "DDLog.h"

// Dtk头文件
#include <DGuiApplicationHelper>
#include <DPalette>
#include <DFontSizeManager>

// Qt库文件
#include <QTableWidgetItem>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <unistd.h>

using namespace DDLog;

PageBoardInfo::PageBoardInfo(QWidget *parent)
    : PageSingleInfo(parent)
    , m_FontChangeFlag(false)
{
    qCDebug(appLog) << "PageBoardInfo constructor";
}

void PageBoardInfo::updateInfo(const QList<DeviceBaseInfo *> &lst)
{
    qCDebug(appLog) << "Updating board info with" << lst.size() << "devices";
    if (lst.size() < 1) {
        qCWarning(appLog) << "Empty device list provided";
        return;
    }
    mp_Device = lst[0];

    // 获取主板信息
    DeviceBaseInfo *board = nullptr;
    QList<DeviceBaseInfo *> lstOther;
    foreach (DeviceBaseInfo *info, lst) {
        // qCDebug(appLog) << "PageBoardInfo::updateInfo processing device:" << info->name();
        DeviceBios *bios = dynamic_cast<DeviceBios *>(info);
        if (!bios) {
            // qCDebug(appLog) << "PageBoardInfo::updateInfo not a bios device, continue";
            continue;
        }

        // 判断是否是主板
        if (bios->isBoard()) {
            // qCDebug(appLog) << "PageBoardInfo::updateInfo is a board device";
            board = info;
        } else {
            // qCDebug(appLog) << "PageBoardInfo::updateInfo is not a board device, add to other list";
            lstOther.append(info);
        }
    }

    if (!board) {
        qCWarning(appLog) << "PageBoardInfo::updateInfo no board device found";
        return;
    }

    // 获取主板信息并加载
    QList<QPair<QString, QString>> baseInfoMap = board->getBaseAttribs();
    QList<QPair<QString, QString>> otherInfoMap = board->getOtherAttribs();
    baseInfoMap = baseInfoMap + otherInfoMap;
    loadDeviceInfo(lstOther, baseInfoMap);
}

void PageBoardInfo::loadDeviceInfo(const QList<DeviceBaseInfo *> &devices, const QList<QPair<QString, QString>> &lst)
{
    qCDebug(appLog) << "Loading device info, lst size:" << lst.size() << "devices size:" << devices.size();
    if (lst.size() < 1) {
        qCWarning(appLog) << "Empty info list provided";
        return;
    }

    // 比较页面可显示的最大行数与主板信息,取小值
    int maxRow = this->height() / ROW_HEIGHT - 3;
    // 需要确保两个参数的类型一致 qsizetype 是在 Qt 6 中存在的无符号整数类型
    int limitSize = std::min(static_cast<qsizetype>(lst.size()), static_cast<qsizetype>(maxRow));
    if (mp_Content) {
        qCDebug(appLog) << "PageBoardInfo::loadDeviceInfo set limit row:" << limitSize;
        mp_Content->setLimitRow(limitSize);
    }

    // 字体无变化如果是展开状态则不更新
    if (!m_FontChangeFlag) {
        qCDebug(appLog) << "PageBoardInfo::loadDeviceInfo font not changed";
        if (isExpanded()) {
            qCDebug(appLog) << "PageBoardInfo::loadDeviceInfo is expanded, return";
            return;
        }
    }
    m_FontChangeFlag = false;
    // clear info
    clearContent();

    // 表格所有行数应等于主板信息行+其他信息行
    int row = lst.size() + devices.size();

    // 设置主板信息行数,此接口目前仅在主板界面中使用,用来配合更多/收起按钮的使用
    setDeviceInfoNum(lst.size());
    mp_Content->setColumnAndRow(row + 1, 2);

    // 主板信息正常显示
    for (int i = 0; i < lst.size(); ++i) {
        // qCDebug(appLog) << "PageBoardInfo::loadDeviceInfo add board info item:" << lst[i].first;
        QTableWidgetItem *itemFirst = new QTableWidgetItem(lst[i].first);
        mp_Content->setItem(i, 0, itemFirst);
        QTableWidgetItem *itemSecond = new QTableWidgetItem(lst[i].second);
        mp_Content->setItem(i, 1, itemSecond);
    }

    QList<QPair<QString, QString>> pairs;
    getOtherInfoPair(devices, pairs);

    // 其他信息使用富文本代理
    // 其他信息的Id是出去所有BIOS信息以外的信息,使用Richtext进行显示
    for (int i = lst.size(); i < row; ++i) {
        // qCDebug(appLog) << "PageBoardInfo::loadDeviceInfo add other info item:" << pairs[i - lst.size()].first;
        mp_Content->setItemDelegateForRow(i, m_ItemDelegate);
        QTableWidgetItem *itemFirst = new QTableWidgetItem(pairs[i - lst.size()].first);
        mp_Content->setItem(i, 0, itemFirst);

        QTableWidgetItem *itemSecond = new QTableWidgetItem(pairs[i - lst.size()].second);
        mp_Content->setItem(i, 1, itemSecond);

//        // 行高已在RichTextDelegate中设置，该段代码先保留
//        // 计算行高
//        QFont font = DFontSizeManager::instance()->t8();
//        QFontMetrics fm(font);
//        int height = 0;
//        QStringList strList = pairs[i - lst.size()].second.split("\n");
//        int fontHeight = fm.boundingRect(pairs[i - lst.size()].second).height()/* + 2*/;
//        //qCInfo(appLog) << strList;
//        // 根据行数增加行高
//        foreach (const QString &str, strList) {
//            QStringList lst = str.split(":");
//            if (lst.size() == 2) {
//                // 属性名称
//                int width = fm.boundingRect(lst[0]).width() + fm.boundingRect(":").width();//未计算“:”，导致width为110时，实际上换行了，而高度又未算入
//                int num = width / 110;
//                int num0 = width % 110;
//                if (num0 == 0)
//                    num = num - 1;

//                int line = 0;
//                // 属性值
//                if (!lst[1].contains("  /  \t\t")) {
//                    width = fm.boundingRect(lst[1]).width();
//                    line = width / 480;
//                    int line0 = width % 480;
//                    if (line0 == 0)
//                        line = line - 1;
//                }

//                if (num > 0 || line > 0)
//                    height += std::max(num, line) * fontHeight;
//            }
//            QStringList attris = str.split("  /  \t\t");
//            height += attris.size() * fontHeight;
//        }
//        height += 20;
//        mp_Content->setRowHeight(i, height);
    }
}

void PageBoardInfo::getOtherInfoPair(const QList<DeviceBaseInfo *> &lst, QList<QPair<QString, QString>> &lstPair)
{
    qCDebug(appLog) << "PageBoardInfo::getOtherInfoPair start, lst size:" << lst.size();
    // 获取其他信息键值对
    foreach (DeviceBaseInfo *dev, lst) {
        // qCDebug(appLog) << "PageBoardInfo::getOtherInfoPair process device:" << dev->name();
        DeviceBios *bios = dynamic_cast<DeviceBios *>(dev);
        if (!bios) {
            // qCDebug(appLog) << "PageBoardInfo::getOtherInfoPair not a bios device, continue";
            continue;
        }
        QPair<QString, QString> pair;
        pair.first = bios->name();
        getValueInfo(bios, pair);
        lstPair.append(pair);
    }
    qCDebug(appLog) << "PageBoardInfo::getOtherInfoPair end";
}

void PageBoardInfo::getValueInfo(DeviceBaseInfo *device, QPair<QString, QString> &pair)
{
    qCDebug(appLog) << "PageBoardInfo::getValueInfo start, device:" << device->name();
    // 获取信息并保存为pair
    QList<QPair<QString, QString>> baseInfoMap = device->getBaseAttribs();
    QList<QPair<QString, QString>> otherInfoMap = device->getOtherAttribs();
    baseInfoMap = baseInfoMap + otherInfoMap;
    QList<QPair<QString, QString>>::iterator it = baseInfoMap.begin();
    for (; it != baseInfoMap.end(); ++it) {
        // qCDebug(appLog) << "PageBoardInfo::getValueInfo process attribute:" << (*it).first;
        QString first = (*it).first;
        QString second = (*it).second;
        // 防止字符串本的 : 带来影响
        pair.second += first.replace(":", " ");
        pair.second += ":";
        pair.second += second.replace(":", " ");
        pair.second += "\n";
    }
    pair.second.replace(QRegularExpression(QStringLiteral("\\n$")), QString());
    qCDebug(appLog) << "PageBoardInfo::getValueInfo end";
}

void PageBoardInfo::setFontChangeFlag()
{
    qCDebug(appLog) << "Setting font change flag";
    // 设置字体变化标志
    m_FontChangeFlag = true;
}
