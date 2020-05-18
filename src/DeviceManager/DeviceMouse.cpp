#include "DeviceMouse.h"

//*******************************************************************************************************************************


DeviceMouse::DeviceMouse()
    : DeviceBaseInfo(), m_Name(""), m_Vendor(""), m_Model(""), m_Interface(""), m_BusInfo("")
    , m_Version(""), m_Capabilities(""), m_Driver(""), m_MaximumPower(""), m_Speed("")
    , m_KeysToCatDevices(""), m_KeyToLshw("")
{
    initFilterKey();
}

bool DeviceMouse::setInfoFromCatInputDevices(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromCatDevice(mapInfo, info);
    if (mapInfo["HwinfoKey"] != m_KeysToCatDevices) {
        return false;
    }

    setAttribute(mapInfo, "Name", m_Name);
    loadOtherDeviceInfo(mapInfo);
    return true;
}

void DeviceMouse::setInfoFromHwinfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromHwinfo(mapInfo, info);

    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    if (mapInfo.find("Hotplug") != mapInfo.end()) {
        setAttribute(mapInfo, "Hotplug", m_Interface);
    } else {
        m_Interface = "PS/2";
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

    // 获取映射到cat devices 的 关键字
    QRegExp re = QRegExp(".*(mouse[0-9]{1,2}).*");
    if (re.exactMatch(mapInfo["Device File"])) {
        m_KeysToCatDevices = re.cap(1);
    }

    loadOtherDeviceInfo(mapInfo);
}

bool DeviceMouse::setInfoFromlshw(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfo(mapInfo, info);

    if (m_KeyToLshw != mapInfo["bus info"]) {
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
}

void DeviceMouse::getMapInfoFromCatDevice(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch)
{
    QMap<QString, QString> mapTemp;
    QStringList lines = info.split("\n");
    foreach (const QString &line, lines) {
        QStringList words = line.split(": ");
        if (words.size() != 2) {
            continue;
        }

        if (words[0] == QString("N") || words[0] == QString("B")) { // Name="AT Translated Set 2 keyboard"
            QStringList keys = words[1].split("=");
            if (keys.size() == 2) {
                mapInfo.insert(keys[0].trimmed(), keys[1].trimmed().replace(QRegExp("^\""), "").replace(QRegExp("\"$"), ""));
            }
        } else if (words[0] == QString("H")) { // Handlers=sysrq kbd leds event14
            QRegExp re = QRegExp(".*(mouse[0-9]{1,2}).*");
            if (re.exactMatch(words[1])) {
                mapInfo.insert("HwinfoKey", re.cap(1));
            }
        }
    }
}
