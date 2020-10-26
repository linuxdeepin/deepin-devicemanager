// 项目自身文件
#include "DeviceMemory.h"

DeviceMemory::DeviceMemory()
    : DeviceBaseInfo()
{
    // 初始化可显示属性
    initFilterKey();
}

void DeviceMemory::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 由lshw设置基本信息
    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "description", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "slot", m_Locator);
    setAttribute(mapInfo, "size", m_Size);

    // 替换内存大小单位
    if (m_Size.contains("GiB")) {
        m_Size.replace("GiB", "GB");
    }

    if (m_Size.contains("MiB")) {
        m_Size.replace("MiB", "");
        // MiB换算MB
        double size = m_Size.toDouble() / 1024.0;
        m_Size = QString::number(size, 'g', 0) + QString("GB");
    }

    // 设置内存速率
    setAttribute(mapInfo, "clock", m_Speed);
    if (m_Speed.contains("MT/s")) {
        m_Speed.replace("MT/s", "MHz");
    }

    // 由lshw设置基本信息
    setAttribute(mapInfo, "width", m_TotalBandwidth);
    setAttribute(mapInfo, "width", m_DataBandwidth);
    setAttribute(mapInfo, "serial", m_SerialNumber);

    getOtherMapInfo(mapInfo);
}

bool DeviceMemory::setInfoFromDmidecode(const QMap<QString, QString> &mapInfo)
{
    // 由 locator属性判断是否为同一内存条
    if (mapInfo["Locator"] != m_Locator) {
        return false;
    }

    // 由dmidecode设置基本属性
    setAttribute(mapInfo, "Part Number", m_Name);
    setAttribute(mapInfo, "Serial Number", m_SerialNumber);

    // 设置配置频率
    setAttribute(mapInfo, "Configured Memory Speed", m_ConfiguredSpeed);
    if (m_ConfiguredSpeed.contains("MT/s")) {
        m_ConfiguredSpeed.replace("MT/s", "MHz");
    }

    // 由dmidecode设置基本属性
    setAttribute(mapInfo, "Minimum Voltage", m_MinimumVoltage);
    setAttribute(mapInfo, "Maximum Voltage", m_MaximumVoltage);
    setAttribute(mapInfo, "Configured Voltage", m_ConfiguredVoltage);
    setAttribute(mapInfo, "Total Width", m_TotalBandwidth);
    setAttribute(mapInfo, "Data Width", m_DataBandwidth);

    // 设置类型
    setAttribute(mapInfo, "Type", m_Type);
    if (m_Type == "<OUT OF SPEC>") {
        m_Type = "";
    }

    // 获取其他属性
    getOtherMapInfo(mapInfo);

    return true;
}

void DeviceMemory::initFilterKey()
{
    // 初始化可显示属性
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

void DeviceMemory::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Size"), m_Size);
    addBaseDeviceInfo(tr("Type"), m_Type);
    addBaseDeviceInfo(tr("Speed"), m_Speed);
    addBaseDeviceInfo(tr("Total Width"), m_TotalBandwidth);
    addBaseDeviceInfo(tr("Locator"), m_Locator);
    addBaseDeviceInfo(tr("Serial Number"), m_SerialNumber);
}

void DeviceMemory::loadOtherDeviceInfo()
{
    // 倒序，头插，保证原来的顺序
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Configured Voltage"), m_ConfiguredVoltage);
    addOtherDeviceInfo(tr("Maximum Voltage"), m_MaximumVoltage);
    addOtherDeviceInfo(tr("Minimum Voltage"), m_MinimumVoltage);
    addOtherDeviceInfo(tr("Configured Speed"), m_ConfiguredSpeed);
    addOtherDeviceInfo(tr("Data Width"), m_DataBandwidth);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceMemory::loadTableHeader()
{
    // 加载表头
    m_TableHeader.append(tr("Name"));
    m_TableHeader.append(tr("Vendor"));
    m_TableHeader.append(tr("Type"));
    m_TableHeader.append(tr("Speed"));
    m_TableHeader.append(tr("Size"));
}

void DeviceMemory::loadTableData()
{
    // 加载表格内容
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Type);
    m_TableData.append(m_Speed);
    m_TableData.append(m_Size);
}

const QString &DeviceMemory::name()const
{
    return m_Name;
}

const QString &DeviceMemory::driver() const
{
    return m_Driver;
}

QString DeviceMemory::subTitle()
{
    // 获取子标题
    return m_Vendor + " " + m_Name;
}

const QString DeviceMemory::getOverviewInfo()
{
    // 获取概况信息
    QString ov;
    ov += QString("%1(%2 %3 %4)") \
          .arg(m_Size) \
          .arg(m_Name != "" ? m_Name : m_Vendor) \
          .arg(m_Type) \
          .arg(m_Speed);

    return ov;
}
