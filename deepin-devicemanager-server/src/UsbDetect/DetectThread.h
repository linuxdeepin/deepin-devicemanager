/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DETECTTHREAD_H
#define DETECTTHREAD_H

#include <QThread>
#include <QMap>
#include <QDateTime>

class MonitorUsb;

/**
 * @brief The DetectThread class
 */
class DetectThread : public QThread
{
    Q_OBJECT
public:
    DetectThread(QObject *parent);

    /**
     * @brief run
     */
    void run() override;

signals:
    /**
     * @brief usbChanged
     */
    void usbChanged();

private slots:
    /**
     * @brief slotUsbChanged usb发生变化时的曹函数处理
     */
    void slotUsbChanged();

private:
    /**
     * @brief isUsbDevicesChanged 判断hwinfo --usb信息是否发生变化
     * @return
     */
    bool isUsbDevicesChanged();

    /**
     * @brief updateMemUsbInfo 更新
     * @param usbInfo
     */
    void updateMemUsbInfo(const QMap<QString,QMap<QString,QString>>& usbInfo);

    /**
     * @brief curHwinfoUsbInfo 获取当前的usb信息
     * @param usbInfo
     */
    void curHwinfoUsbInfo(QMap<QString,QMap<QString,QString>>& usbInfo);

    /**
     * @brief getMapInfo 解析usb信息
     * @param item
     * @param mapInfo
     * @return
     */
    bool getMapInfo(const QString& item,QMap<QString,QString>& mapInfo);

private:
    MonitorUsb *mp_MonitorUsb; //<! udev检测任务
    QMap<QString,QMap<QString,QString>> m_MapUsbInfo; //<! usb信息
};

#endif // DETECTTHREAD_H
