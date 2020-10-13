#include "DeviceMouse.h"
#include <QDebug>
#include "DeviceManager.h"
//*******************************************************************************************************************************


DeviceMouse::DeviceMouse()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Interface("")
    , m_BusInfo("")
    , m_Version("")
    , m_Capabilities("")
    , m_Driver("")
    , m_MaximumPower("")
    , m_Speed("")
    , m_KeysToCatDevices("")
    , m_KeyToLshw("")
{
    initFilterKey();
}

void DeviceMouse::setInfoFromHwinfo(QMap<QString, QString> mapInfo)
{
    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);

    // 获取鼠标的接口类型
    if (mapInfo.find("Hotplug") != mapInfo.end()) {
        setAttribute(mapInfo, "Hotplug", m_Interface);
    } else {
        m_Interface = "PS/2";
    }

    // 上面的方法不适合蓝牙鼠标的获取方法
    if (mapInfo.find("Model")->contains("Bluetooth", Qt::CaseInsensitive) || mapInfo.find("Device")->contains("Bluetooth", Qt::CaseInsensitive)) {
        m_Interface = "Bluetooth";
    }

    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    QStringList words = mapInfo["SysFS BusID"].split(":");
    if (words.size() == 2) {
        QStringList chs = words[0].split("-");
        if (chs.size() == 2) {
            m_KeyToLshw = QString("usb@%1:%2").arg(chs[0]).arg(chs[1]);
        }
    }

    // 获取映射到  cat /proc/bus/input/devices 里面的关键字
    QRegExp re = QRegExp(".*(event[0-9]{1,2}).*");
    if (re.exactMatch(mapInfo["Device File"])) {
        m_KeysToCatDevices = re.cap(1);
    } else {
        QRegExp rem = QRegExp(".*(mouse[0-9]{1,2}).*");
        if (rem.exactMatch(mapInfo["Device File"])) {
            m_KeysToCatDevices = rem.cap(1);
        }
    }

    // 由cat /proc/bus/devices/input设置设备信息
    setInfoFromInput();

    // 由bluetoothctl paired-devices设置设备接口
    setInfoFromBluetoothctl();

    loadOtherDeviceInfo(mapInfo);
}

bool DeviceMouse::setInfoFromlshw(QMap<QString, QString> mapInfo)
{
    QString key = mapInfo["bus info"];
    key.replace("a", "10");
    if (m_KeyToLshw != key) {
        return false;
    }

    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "", m_Interface);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
    loadOtherDeviceInfo(mapInfo);
    return true;
}

const QString &DeviceMouse::name()const
{
    return m_Name;
}

const QString &DeviceMouse::vendor()const
{
    return m_Vendor;
}

const QString &DeviceMouse::model()const
{
    return m_Model;
}

const QString &DeviceMouse::interface()const
{
    return m_Interface;
}

const QString &DeviceMouse::busInfo()const
{
    return m_BusInfo;
}

const QString &DeviceMouse::version()const
{
    return m_Version;
}

const QString &DeviceMouse::capability()const
{
    return m_Capabilities;
}

const QString &DeviceMouse::driver()const
{
    return m_Driver;
}

const QString &DeviceMouse::maxPower()const
{
    return m_MaximumPower;
}

const QString &DeviceMouse::speed()const
{
    return m_Speed;
}

void DeviceMouse::initFilterKey()
{
    // from cat devices
    addFilterKey(QObject::tr("PROP"));
    addFilterKey(QObject::tr("EV"));
    addFilterKey(QObject::tr("KEY"));
    addFilterKey(QObject::tr("MSC"));
    addFilterKey(QObject::tr("Device File"));
    addFilterKey(QObject::tr("Hardware Class"));

    addFilterKey(QObject::tr("physical id"));

}

void DeviceMouse::setInfoFromBluetoothctl()
{
    // 判断该设备信息是否存在于Bluetoothctl中
    if (!m_keysToPairedDevice.isEmpty()) {
        bool isExist = DeviceManager::instance()->isDeviceExistInPairedDevice(m_keysToPairedDevice.toUpper());

        if (isExist) {
            m_Interface = "Bluetooth";
        }
    }
}

void DeviceMouse::setInfoFromInput()
{
    // 获取对应的由cat /proc/bus/devices/input读取的设备信息
    const QMap<QString, QString> &mapInfo = DeviceManager::instance()->inputInfo(m_KeysToCatDevices);

    // 设置Name属性
    setAttribute(mapInfo, "Name", m_Name, true);

    // Uniq属性标识蓝牙设备Mac地址
    m_keysToPairedDevice = mapInfo["Uniq"].toUpper();
}
