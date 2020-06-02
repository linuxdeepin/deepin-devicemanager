#include "DeviceAudio.h"

DeviceAudio::DeviceAudio()
    : DeviceBaseInfo(), m_Name(""), m_Vendor(""), m_Model(""), m_Version(""), m_BusInfo("")
    , m_Irq(""), m_Memory(""), m_Width(""), m_Clock(""), m_Capabilities(""), m_Description("")
    , m_UniqueKey("")
{
    initFilterKey();
}

void DeviceAudio::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "", m_Version);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "IRQ", m_Irq);
    setAttribute(mapInfo, "Memory Range", m_Memory);
    setAttribute(mapInfo, "", m_Width);
    setAttribute(mapInfo, "", m_Clock);
    setAttribute(mapInfo, "", m_Capabilities);
    setAttribute(mapInfo, "Hardware Class", m_Description);

    m_UniqueKey = mapInfo["SysFS BusID"];

    loadOtherDeviceInfo(mapInfo);
}

bool DeviceAudio::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 先判断传入的设备信息是否是同一个
    QStringList words = mapInfo["bus info"].split("@");
    if (words.size() != 2) {
        return false;
    }
    if (words[1] != m_UniqueKey) {
        return false;
    }

    // 设置设备属性
    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "version", m_Version);
    if (m_Version == "00") {
        m_Version = "";
    }
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "", m_Irq);
    setAttribute(mapInfo, "", m_Memory);
    setAttribute(mapInfo, "width", m_Width);
    setAttribute(mapInfo, "clock", m_Clock);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "description", m_Description);

    loadOtherDeviceInfo(mapInfo);
    return true;
}

bool DeviceAudio::setInfoFromCatDevices(const QMap<QString, QString> &mapInfo)
{
    setAttribute(mapInfo, "Name", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "Version", m_Version);
    setAttribute(mapInfo, "Bus", m_BusInfo);
    setAttribute(mapInfo, "", m_Irq);
    setAttribute(mapInfo, "", m_Memory);
    setAttribute(mapInfo, "", m_Width);
    setAttribute(mapInfo, "", m_Clock);
    setAttribute(mapInfo, "", m_Capabilities);
    setAttribute(mapInfo, "", m_Description);


    loadOtherDeviceInfo(mapInfo);
    return true;
}

bool DeviceAudio::setAudioChipFromDmesg(const QString &info)
{
    m_Chip = info;
    return true;
}

const QString &DeviceAudio::name()const
{
    return m_Name;
}
const QString &DeviceAudio::vendor()const
{
    return m_Vendor;
}
const QString &DeviceAudio::model()const
{
    return m_Model;
}
const QString &DeviceAudio::version()const
{
    return m_Version;
}
const QString &DeviceAudio::busInfo()const
{
    return m_BusInfo;
}
const QString &DeviceAudio::irq()const
{
    return m_Irq;
}
const QString &DeviceAudio::memory()const
{
    return m_Memory;
}
const QString &DeviceAudio::width()const
{
    return m_Width;
}
const QString &DeviceAudio::clock()const
{
    return m_Clock;
}
const QString &DeviceAudio::capabilities()const
{
    return m_Capabilities;
}
const QString &DeviceAudio::description()const
{
    return m_Description;
}

const QString &DeviceAudio::chip() const
{
    return m_Chip;
}
void DeviceAudio::initFilterKey()
{
    addFilterKey(QObject::tr("Device Name"));
    addFilterKey(QObject::tr("SubVendor"));
    addFilterKey(QObject::tr("SubDevice"));
    addFilterKey(QObject::tr("Driver"));
    addFilterKey(QObject::tr("Driver Modules"));
    addFilterKey(QObject::tr("Driver Status"));
    addFilterKey(QObject::tr("Driver Activation Cmd"));
    addFilterKey(QObject::tr("Config Status"));

    //addFilterKey(QObject::tr("irq"));
    addFilterKey(QObject::tr("physical id"));
    addFilterKey(QObject::tr("latency"));

    addFilterKey(QObject::tr("Phys"));
    addFilterKey(QObject::tr("Sysfs"));
    addFilterKey(QObject::tr("Handlers"));
    addFilterKey(QObject::tr("PROP"));
    addFilterKey(QObject::tr("EV"));
    addFilterKey(QObject::tr("KEY"));
}
