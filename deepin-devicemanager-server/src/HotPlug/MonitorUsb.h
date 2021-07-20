#ifndef MONITORUSB_H
#define MONITORUSB_H
#include <libudev.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <QObject>

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

private:
    struct udev                       *m_Udev;              //<! udev Environment
    struct udev_monitor               *mon;                 //<! object of mon
    int                               fd;                   //<! fd

};

#endif // MONITORUSB_H
