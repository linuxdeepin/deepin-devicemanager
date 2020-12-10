#include "DeviceMemory.h"

DeviceMemory::DeviceMemory()
    : DeviceBaseInfo()
{
    initFilterKey();
}

void DeviceMemory::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "description", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "slot", m_Locator);
    setAttribute(mapInfo, "size", m_Size);
    if (m_Size.contains("GiB")) {
        m_Size.replace("GiB", "GB");
    }
    if (m_Size.contains("MiB")) {
        m_Size.replace("MiB", "");
        double size = m_Size.toDouble() / 1024.0;
        m_Size = QString::number(size, 'g', 0) + QString("GB");
    }
    setAttribute(mapInfo, "clock", m_Speed);
    if (m_Speed.contains("MT/s")) {
        m_Speed.replace("MT/s", "MHz");
    }

    setAttribute(mapInfo, "width", m_TotalBandwidth);
    setAttribute(mapInfo, "width", m_DataBandwidth);
    setAttribute(mapInfo, "serial", m_SerialNumber);

    loadOtherDeviceInfo(mapInfo);
}

void DeviceMemory::setKLUInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 设置KLU内存属性
    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "description", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "slot", m_Locator);
    setAttribute(mapInfo, "size", m_Size);
    if (m_Size.contains("GiB")) {
        m_Size.replace("GiB", "GB");
    }
    if (m_Size.contains("MiB")) {
        m_Size.replace("MiB", "");
        double size = m_Size.toDouble() / 1024.0;
        m_Size = QString::number(size, 'g', 0) + QString("GB");
    }

    // 设置速度
    setAttribute(mapInfo, "clock", m_Speed);
    if (m_Speed.contains("MT/s")) {
        m_Speed.replace("MT/s", "MHz");
    }

    // KLU要求 去除2667MHz（0.4ns） 中的 （0.4ns）
    QRegExp reg("([\\s\\S]*)\\([\\s\\S]*\\)");
    if (reg.indexIn(m_Speed) != -1) {
        m_Speed = reg.cap(1);
    }

    setAttribute(mapInfo, "width", m_TotalBandwidth);
    setAttribute(mapInfo, "width", m_DataBandwidth);
    setAttribute(mapInfo, "serial", m_SerialNumber);

    loadOtherDeviceInfo(mapInfo);
}

bool DeviceMemory::setInfoFromDmidecode(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo["Locator"] != m_Locator) {
        return false;
    }
    setAttribute(mapInfo, "Part Number", m_Name);
    setAttribute(mapInfo, "Serial Number", m_SerialNumber);
    setAttribute(mapInfo, "Configured Memory Speed", m_ConfiguredSpeed);
    if (m_ConfiguredSpeed.contains("MT/s")) {
        m_ConfiguredSpeed.replace("MT/s", "MHz");
    }
    setAttribute(mapInfo, "Minimum Voltage", m_MinimumVoltage);
    setAttribute(mapInfo, "Maximum Voltage", m_MaximumVoltage);
    setAttribute(mapInfo, "Configured Voltage", m_ConfiguredVoltage);
    setAttribute(mapInfo, "Total Width", m_TotalBandwidth);
    setAttribute(mapInfo, "Data Width", m_DataBandwidth);
    setAttribute(mapInfo, "Type", m_Type);
    if (m_Type == "<OUT OF SPEC>") {
        m_Type = "";
    }

    loadOtherDeviceInfo(mapInfo);

    return true;
}

void DeviceMemory::initFilterKey()
{
    addFilterKey(QObject::tr("Array Handle"));  // 数组程序
    addFilterKey(QObject::tr("Error Information Handle")); //错误信息程序
    addFilterKey(QObject::tr("Form Factor")); // 尺寸型号
    addFilterKey(QObject::tr("Set"));    // 设置
    addFilterKey(QObject::tr("Bank Locator")); // 内存通道
    addFilterKey(QObject::tr("Type Detail"));   // 类型详细信息
    addFilterKey(QObject::tr("Asset Tag"));    // 资产标签
    addFilterKey(QObject::tr("Part Number"));
    addFilterKey(QObject::tr("Rank"));
    addFilterKey(QObject::tr("Memory Technology"));  // 内存技术
    addFilterKey(QObject::tr("Memory Operating Mode Capability"));  // 内存操作模式
    addFilterKey(QObject::tr("Firmware Version"));   //固件版本
    addFilterKey(QObject::tr("Module Manufacturer ID"));
    addFilterKey(QObject::tr("Module Product ID"));
    addFilterKey(QObject::tr("Memory Subsystem Controller Manufacturer ID"));
    addFilterKey(QObject::tr("Memory Subsystem Controller Product ID"));
    addFilterKey(QObject::tr("Non-Volatile Size"));   // 不易丢失大小
    addFilterKey(QObject::tr("Volatile Size"));       // 易丢失大小
    addFilterKey(QObject::tr("Cache Size"));   // 缓存大小
    addFilterKey(QObject::tr("Logical Size"));  // 逻辑大小
}

const QString &DeviceMemory::name()const
{
    return m_Name;
}

const QString &DeviceMemory::vendor()const
{
    return m_Vendor;
}

const QString &DeviceMemory::size()const
{
    return m_Size;
}

const QString &DeviceMemory::type()const
{
    return m_Type;
}

const QString &DeviceMemory::speed()const
{
    return m_Speed;
}

const QString &DeviceMemory::totalBandwidth()const
{
    return m_TotalBandwidth;
}

const QString &DeviceMemory::dataBandwidth()const
{
    return m_DataBandwidth;
}

const QString &DeviceMemory::locator()const
{
    return m_Locator;
}

const QString &DeviceMemory::serialNumber()const
{
    return m_SerialNumber;
}

const QString &DeviceMemory::configuredSpeed()const
{
    return m_ConfiguredSpeed;
}

const QString &DeviceMemory::minimumVoltage()const
{
    return m_MinimumVoltage;
}

const QString &DeviceMemory::maximumVoltage()const
{
    return m_MaximumVoltage;
}

const QString &DeviceMemory::configuredVoltage()const
{
    return m_ConfiguredVoltage;
}
