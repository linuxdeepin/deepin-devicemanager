#include "MonitorUsb.h"
#include "QDebug"
MonitorUsb::MonitorUsb()
    : m_Udev(nullptr)
{
    m_Udev = udev_new();
    if (!m_Udev) {
        printf("error!!!\n");
    }
    // 创建一个新的monitor
    mon = udev_monitor_new_from_netlink(m_Udev, "udev");
    // 增加一个udev事件过滤器
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", nullptr);
    // 启动监控
    udev_monitor_enable_receiving(mon);
    // 获取该监控的文件描述符，fd就代表了这个监控
    fd = udev_monitor_get_fd(mon);
}

void MonitorUsb::monitor()
{
    char buf[10];
    fd_set fds;
    struct timeval tv;
    int ret;
    while (true) {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        ret = select(fd + 1, &fds, nullptr, nullptr, &tv);

        // 判断是否有事件产生
        if (!ret)
            continue;
        if (! FD_ISSET(fd, &fds))
            continue;

        // 获取产生事件的设备映射
        struct udev_device *dev = udev_monitor_receive_device(mon);
        if (!dev)
            continue;

        // 获取事件并判断是否是插拔
        unsigned long long curNum = udev_device_get_devnum(dev);
        if (curNum <= 0) {
            udev_device_unref(dev);
            continue;
        }

        // 只有add和remove事件才会更新缓存信息
        strcpy(buf, udev_device_get_action(dev));
        if (0 == strcmp("add", buf) || 0 == strcmp("remove", buf)) {
            qInfo() << QString::fromLocal8Bit(buf) << " ***************************** DETECT  " << curNum ;
            emit usbChanged();
        }

        udev_device_unref(dev);
    }
}
