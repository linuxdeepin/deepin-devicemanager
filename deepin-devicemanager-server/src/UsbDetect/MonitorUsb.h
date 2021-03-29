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
    struct udev *udev;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_monitor *mon;
    int fd;

};

#endif // MONITORUSB_H
