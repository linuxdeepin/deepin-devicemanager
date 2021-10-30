#ifndef MONITORUSB_H
#define MONITORUSB_H
#include <libudev.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <QObject>
#include <QTimer>

class MonitorUsb : public QObject
{
    Q_OBJECT
public:
    MonitorUsb();

    /**
     * @brief monitor
     */
    void monitor();

signals:
    /**
     * @brief usbChanged
     */
    void usbChanged();

private slots:
    /**
     * @brief slotTimeout
     */
    void slotTimeout();

private:
    /**
     * @brief disableDevice 禁用设备
     */
    void disableDevice();

    /**
     * @brief getMapInfo 解析usb信息
     * @param item
     * @param mapInfo
     * @return
     */
    bool getMapInfo(const QString& item,QMap<QString,QString>& mapInfo);

private:
    struct udev                       *m_Udev;              //<! udev Environment
    struct udev_monitor               *mon;                 //<! object of mon
    int                               fd;                   //<! fd
    QTimer                            *mp_Timer;            //<! 定时器
    qint64                            m_UsbChangeTime;      //<! 记录当前时间
    bool                              m_UsbChanged;         //<! 记录是否有usb插拔

};

#endif // MONITORUSB_H
