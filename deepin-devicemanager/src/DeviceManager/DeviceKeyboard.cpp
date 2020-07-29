#include "DeviceKeyboard.h"
#include <QDebug>

DeviceKeyboard::DeviceKeyboard()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Version("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_Description("")
    , m_Driver("")
    , m_MaximumPower("")
    , m_Speed("")
    , m_KeyToLshw("")
{
    initFilterKey();
}

bool DeviceKeyboard::setInfoFromlshw(const QMap<QString, QString> &mapInfo)
{
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
    setAttribute(mapInfo, "description", m_Description);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
    return true;
}

void DeviceKeyboard::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);

    // 获取键盘的接口类型
    if (mapInfo.find("Hotplug") != mapInfo.end()) {
        setAttribute(mapInfo, "Hotplug", m_Interface);
    } else {
        m_Interface = "PS/2";
    }

    // 上面的方法不适合蓝牙键盘的获取方法
    if (mapInfo.find("Model")->contains("Bluetooth", Qt::CaseInsensitive) || mapInfo.find("Device")->contains("Bluetooth", Qt::CaseInsensitive)) {
        m_Interface = "Bluetooth";
    }

    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Hardware Class", m_Description);
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
    getOtherMapInfo(mapInfo);
}

void DeviceKeyboard::setKLUInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
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
    if (m_Name.contains("")) {
        m_Interface = "";
    }
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Hardware Class", m_Description);
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
    getOtherMapInfo(mapInfo);
}

const QString &DeviceKeyboard::name()const
{
    return m_Name;
}

const QString &DeviceKeyboard::vendor()const
{
    return m_Vendor;
}

const QString &DeviceKeyboard::model()const
{
    return m_Model;
}

const QString &DeviceKeyboard::version()const
{
    return m_Version;
}

const QString &DeviceKeyboard::interface()const
{
    return m_Interface;
}

const QString &DeviceKeyboard::busInfo()const
{
    return m_BusInfo;
}

const QString &DeviceKeyboard::capabilities()const
{
    return m_Capabilities;
}

const QString &DeviceKeyboard::driver()const
{
    return m_Driver;
}

const QString &DeviceKeyboard::maxinumPower()const
{
    return m_MaximumPower;
}

const QString &DeviceKeyboard::speed()const
{
    return m_Speed;
}

const QString &DeviceKeyboard::description()const
{
    return m_Description;
}

QString DeviceKeyboard::subTitle()
{
    return m_Model;
}

const QString DeviceKeyboard::getOverviewInfo()
{
    QString ov = QString("%1 (%2)") \
                 .arg(m_Name) \
                 .arg(m_Model);

    return ov;
}

void DeviceKeyboard::initFilterKey()
{
    addFilterKey(QObject::tr("PROP"));
    addFilterKey(QObject::tr("EV"));
    addFilterKey(QObject::tr("KEY"));
    addFilterKey(QObject::tr("MSC"));
    addFilterKey(QObject::tr("Device File"));
    addFilterKey(QObject::tr("Hardware Class"));

    addFilterKey(QObject::tr("physical id"));
}

void DeviceKeyboard::loadBaseDeviceInfo()
{
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Interface"), m_Interface);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);

//    m_SubTitle = m_Model;
}

void DeviceKeyboard::getKeyboardMapInfoFromInputDevice(QMap<QString, QString> &mapInfo, const QString &info)
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
            QRegExp re = QRegExp(".*(event[0-9]{1,2}).*");
            if (re.exactMatch(words[1])) {
                mapInfo.insert("HwinfoKey", re.cap(1));
            }
        }
    }
}

void DeviceKeyboard::loadOtherDeviceInfo()
{
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(tr("Version"), m_Version);

    mapInfoToList();
}

void DeviceKeyboard::loadTableData()
{
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
