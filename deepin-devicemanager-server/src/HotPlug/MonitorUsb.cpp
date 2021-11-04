#include "MonitorUsb.h"
#include "EnableSqlManager.h"

#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QDateTime>

#define LEAST_NUM 10

MonitorUsb::MonitorUsb()
    : m_Udev(nullptr)
    , mp_Timer(new QTimer)
    , m_UsbChangeTime()
    , m_UsbChanged(false)
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

    // 定时器发送消息
    connect(mp_Timer, &QTimer::timeout, this, &MonitorUsb::slotTimeout);
    mp_Timer->start(1000);
}

void MonitorUsb::monitor()
{
    char buf[10];
    fd_set fds;
    struct timeval tv;
    while (true) {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        int ret = select(fd + 1, &fds, nullptr, nullptr, &tv);

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
        if (curNum == 0) {
            udev_device_unref(dev);
            continue;
        }

        // 只有add和remove事件才会更新缓存信息
        strcpy(buf, udev_device_get_action(dev));
        if (0 == strcmp("add", buf) || 0 == strcmp("remove", buf)) {
            if(0 == strcmp("add", buf)){
                EnableSqlManager::getInstance()->clearEnableFromAuthorizedTable();
                disableDevice();
            }
            m_UsbChanged = true;
            m_UsbChangeTime = QDateTime::currentMSecsSinceEpoch();
        }

        udev_device_unref(dev);
    }
}

void MonitorUsb::disableDevice()
{
    QProcess process;
    process.start("hwinfo --usb");
    process.waitForFinished(-1);
    QString info = process.readAllStandardOutput();

    QStringList items = info.split("\n\n");
    foreach(const QString& item,items){
        QMap<QString,QString> mapItem;
        if(!getMapInfo(item,mapItem))
            continue;
        // 防止禁用的设备被启用
        QString uniqueID = mapItem["Module Alias"];
        uniqueID.replace(QRegExp("[0-9a-zA-Z]{10}$"),"");
        if(uniqueID.isEmpty()){
            return;
        }

        QString path = mapItem["SysFS ID"];
        path.replace(QRegExp("[1-9]$"),"0");
        if(EnableSqlManager::getInstance()->uniqueIDExisted(uniqueID)){
            QFile file("/sys" + path + QString("/authorized"));
            if(!file.open(QIODevice::ReadWrite)){
                return;
            }
            file.write("0");
            file.close();
            EnableSqlManager::getInstance()->updateDataToAuthorizedTable(uniqueID,path);
        }
    }
}

bool MonitorUsb::getMapInfo(const QString& item,QMap<QString,QString>& mapInfo)
{
    QStringList lines = item.split("\n");
    // 行数太少则为无用信息
    if(lines.size() <= LEAST_NUM){
        return false;
    }

    foreach(const QString& line,lines){
        QStringList words = line.split(": ");
        if(words.size() != 2)
            continue;
        mapInfo.insert(words[0].trimmed(),words[1].replace("\"","").trimmed());
    }

    // hub为usb接口，可以直接过滤
    if(mapInfo["Hardware Class"] == "hub"){
        return false;
    }

    // 没有总线信息的设备可以过滤
    if(mapInfo.find("SysFS BusID") == mapInfo.end()){
        return false;
    }

    return true;

}

void MonitorUsb::slotTimeout()
{
    if(!m_UsbChanged)
        return;
    if(QDateTime::currentMSecsSinceEpoch() - m_UsbChangeTime < 1000)
        return;
    m_UsbChanged = false;
    emit usbChanged();
}
