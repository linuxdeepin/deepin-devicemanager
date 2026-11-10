// SPDX-FileCopyrightText: 2019 ~ 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitorusb.h"
#include "controlinterface.h"
#include "mainjob.h"

#include <QLoggingCategory>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QStringList>

// ---------------------------------------------------------------------------
// Sysfs-based USB device enumeration
//
// Reading /sys/bus/usb/devices/ replaces the external "hwinfo --usb" process.
// hwinfo opens every /dev/bus/usb/*/* node and issues ioctl(USBDEVFS_CONTROL)
// control transfers to read descriptors; on some xHCI controllers these
// control transfers stall isochronous audio URBs for ~970 ms, exceeding the
// ~93 ms PulseAudio buffer and causing audible stuttering (PMS 381051).
// Reading sysfs attribute files performs no USB I/O, eliminating the
// control-transfer interference.
//
// The generated string keeps the key:value line format that
// EnableUtils::disableOutDevice() and WakeupUtils::updateWakeupDeviceInfo()
// already parse (lines split on ": ", device blocks separated by blank lines,
// >10 lines per block).
// ---------------------------------------------------------------------------

// Read a sysfs attribute file and return its trimmed content.
static QString readSysfsAttr(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return QString::fromLocal8Bit(file.readAll()).trimmed();
}

// Determine the hwinfo-style "Hardware Class" for a USB device.
// Only the values downstream consumers act on are distinguished:
//   "hub"      -- bDeviceClass 0x09 (filtered out by getMapInfo)
//   "keyboard" -- HID boot interface, protocol 1
//   "mouse"    -- HID boot interface, protocol 2
// Everything else is reported as "usb".
static QString usbHardwareClass(const QString &deviceDir, const QString &deviceName)
{
    const QString bDeviceClass = readSysfsAttr(deviceDir + QStringLiteral("/bDeviceClass"));
    if (bDeviceClass == QLatin1String("09"))
        return QStringLiteral("hub");

    QDir dir(deviceDir);
    const QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QString &entry : entries) {
        if (!entry.startsWith(deviceName + QLatin1Char(':')))
            continue;
        const QString ifaceDir = deviceDir + QLatin1Char('/') + entry;
        const QString bInterfaceClass = readSysfsAttr(ifaceDir + QStringLiteral("/bInterfaceClass"));
        if (bInterfaceClass != QLatin1String("03"))   // HID
            continue;
        const QString bInterfaceSubClass = readSysfsAttr(ifaceDir + QStringLiteral("/bInterfaceSubClass"));
        const QString bInterfaceProtocol = readSysfsAttr(ifaceDir + QStringLiteral("/bInterfaceProtocol"));
        if (bInterfaceSubClass == QLatin1String("01")) {  // Boot interface
            if (bInterfaceProtocol == QLatin1String("01"))
                return QStringLiteral("keyboard");
            if (bInterfaceProtocol == QLatin1String("02"))
                return QStringLiteral("mouse");
        }
    }
    return QStringLiteral("usb");
}

// Build a hwinfo --usb compatible information string by reading
// /sys/bus/usb/devices/ directly (no ioctl, no USB control transfers).
static QString getUsbInfoFromSysfs()
{
    QDir baseDir(QStringLiteral("/sys/bus/usb/devices"));
    const QStringList deviceEntries = baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    QStringList deviceBlocks;
    int index = 0;

    for (const QString &entry : deviceEntries) {
        // Skip interface entries such as "1-3:1.0" -- only enumerate devices.
        if (entry.contains(QLatin1Char(':')))
            continue;

        const QString deviceDir = QStringLiteral("/sys/bus/usb/devices/") + entry;
        const QString idVendor = readSysfsAttr(deviceDir + QStringLiteral("/idVendor"));
        const QString idProduct = readSysfsAttr(deviceDir + QStringLiteral("/idProduct"));
        if (idVendor.isEmpty() || idProduct.isEmpty())
            continue;

        const QString manufacturer = readSysfsAttr(deviceDir + QStringLiteral("/manufacturer"));
        const QString product = readSysfsAttr(deviceDir + QStringLiteral("/product"));
        const QString serial = readSysfsAttr(deviceDir + QStringLiteral("/serial"));
        const QString bcdDevice = readSysfsAttr(deviceDir + QStringLiteral("/bcdDevice"));
        const QString bDeviceClass = readSysfsAttr(deviceDir + QStringLiteral("/bDeviceClass"));
        const QString bDeviceSubClass = readSysfsAttr(deviceDir + QStringLiteral("/bDeviceSubClass"));
        const QString bDeviceProtocol = readSysfsAttr(deviceDir + QStringLiteral("/bDeviceProtocol"));
        const QString busnum = readSysfsAttr(deviceDir + QStringLiteral("/busnum"));
        const QString devnum = readSysfsAttr(deviceDir + QStringLiteral("/devnum"));
        const QString speed = readSysfsAttr(deviceDir + QStringLiteral("/speed"));

        // Canonical sysfs path, stripped of the /sys prefix so it matches
        // hwinfo's "SysFS Device Link" (e.g. /devices/pci.../usb1/1-3).
        const QString canonicalDevice = QDir(deviceDir).canonicalPath();
        if (canonicalDevice.isEmpty())
            continue;
        QString sysfsDeviceLink = canonicalDevice;
        if (sysfsDeviceLink.startsWith(QStringLiteral("/sys")))
            sysfsDeviceLink = sysfsDeviceLink.mid(4);

        // SysFS ID uses the first interface path, falling back to the device
        // path when no interface exists.
        QString sysfsId = sysfsDeviceLink;
        QDir devDir(deviceDir);
        const QStringList subEntries = devDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString &sub : subEntries) {
            if (!sub.startsWith(entry + QLatin1Char(':')))
                continue;
            const QString ifaceCanonical = QDir(deviceDir + QLatin1Char('/') + sub).canonicalPath();
            if (!ifaceCanonical.isEmpty()) {
                sysfsId = ifaceCanonical;
                if (sysfsId.startsWith(QStringLiteral("/sys")))
                    sysfsId = sysfsId.mid(4);
                break;
            }
        }

        const QString hwClass = usbHardwareClass(deviceDir, entry);

        // Detect USB network adapters so disableOutDevice() can obtain the
        // interface name ("Device File") and MAC ("Permanent HW Address").
        QString permHwAddr;
        QString deviceFile;
        for (const QString &sub : subEntries) {
            if (!sub.startsWith(entry + QLatin1Char(':')))
                continue;
            QDir netDir(deviceDir + QLatin1Char('/') + sub + QStringLiteral("/net"));
            if (netDir.exists()) {
                const QStringList netEntries = netDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                if (!netEntries.isEmpty()) {
                    deviceFile = netEntries.first();
                    permHwAddr = readSysfsAttr(netDir.absoluteFilePath(deviceFile) + QStringLiteral("/address"));
                    break;
                }
            }
        }

        const QString moduleAlias = QStringLiteral("usb:v%1p%2d%3dc%4dsc%5dp%6")
                .arg(idVendor).arg(idProduct).arg(bcdDevice)
                .arg(bDeviceClass).arg(bDeviceSubClass).arg(bDeviceProtocol);

        QStringList lines;
        lines << QStringLiteral("%1: USB 00.0: 0000 USB Device").arg(++index);
        lines << QStringLiteral("  [Created at sysfs]");
        lines << QStringLiteral("  Unique ID: %1").arg(entry);
        lines << QStringLiteral("  SysFS ID: %1").arg(sysfsId);
        lines << QStringLiteral("  SysFS BusID: usb%1:%2").arg(busnum).arg(devnum);
        lines << QStringLiteral("  SysFS Device Link: %1").arg(sysfsDeviceLink);
        lines << QStringLiteral("  Hardware Class: %1").arg(hwClass);
        lines << QStringLiteral("  Model: \"%1\"").arg(product);
        lines << QStringLiteral("  Vendor: usb 0x%1 \"%2\"").arg(idVendor).arg(manufacturer);
        lines << QStringLiteral("  Device: usb 0x%1 \"%2\"").arg(idProduct).arg(product);
        lines << QStringLiteral("  Revision: \"%1\"").arg(bcdDevice);
        if (!serial.isEmpty())
            lines << QStringLiteral("  Serial ID: \"%1\"").arg(serial);
        if (!permHwAddr.isEmpty())
            lines << QStringLiteral("  Permanent HW Address: %1").arg(permHwAddr);
        if (!deviceFile.isEmpty())
            lines << QStringLiteral("  Device File: %1").arg(deviceFile);
        lines << QStringLiteral("  Speed: %1 Mb/s").arg(speed);
        lines << QStringLiteral("  Module Alias: \"%1\"").arg(moduleAlias);

        deviceBlocks << lines.join(QLatin1Char('\n'));
    }

    return deviceBlocks.join(QStringLiteral("\n\n"));
}

MonitorUsb::MonitorUsb()
    : m_Udev(nullptr)
    , mp_Timer(new QTimer(this))
    , m_UsbChangeTime()
    , m_UsbChanged(false)
    , m_workingFlag(true)
{
    m_Udev = udev_new();
    if (!m_Udev) {
        printf("error!!!\n");
    }
    // 创建一个新的monitor
    mon = udev_monitor_new_from_netlink(m_Udev, "udev");
    // 增加一个udev事件过滤器
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", nullptr);
    udev_monitor_filter_add_match_subsystem_devtype(mon, "bluetooth", nullptr);
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
        if (!m_workingFlag)
            break;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
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

        // 监测蓝牙设备
        if (0 == strcmp(udev_device_get_devtype(dev), "link") && m_workingFlag) {
            emit usbChanged();
            continue;
        }

        // 获取事件并判断是否是插拔
        unsigned long long curNum = udev_device_get_devnum(dev);
        if (0 == curNum) {
            udev_device_unref(dev);
            continue;
        }

        // 只有add和remove事件才会更新缓存信息
        strcpy(buf, udev_device_get_action(dev));
        if ((0 == strcmp("add", buf) || 0 == strcmp("remove", buf)) && m_workingFlag) {
            // Read USB device info from sysfs instead of spawning "hwinfo --usb".
            // hwinfo issues ioctl(USBDEVFS_CONTROL) control transfers to every
            // USB device; on affected xHCI controllers these stall audio
            // isochronous URBs and cause playback stuttering (PMS 381051).
            // Sysfs reads perform no USB I/O, eliminating the interference.
            QString info = getUsbInfoFromSysfs();
            if (0 == strcmp("add", buf)) {
                ControlInterface::getInstance()->disableOutDevice(info);
            }
            ControlInterface::getInstance()->updateWakeup(info);
            m_UsbChanged = true;
            m_UsbChangeTime = QDateTime::currentMSecsSinceEpoch();
        }

        udev_device_unref(dev);
    }
}

void MonitorUsb::setWorkingFlag(bool flag)
{
    m_workingFlag = flag;
}

void MonitorUsb::slotTimeout()
{
    if (!m_UsbChanged || !m_workingFlag)
        return;
    if (QDateTime::currentMSecsSinceEpoch() - m_UsbChangeTime < 1000)
        return;
    m_UsbChanged = false;
    emit usbChanged();
}
