// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceMemory.h"
#include "commonfunction.h"
#include "DeviceCpu.h"
#include "DDLog.h"

using namespace DDLog;

DeviceMemory::DeviceMemory()
    : DeviceBaseInfo()
    , m_MatchedFromDmi(false)
{
    qCDebug(appLog) << "DeviceMemory constructor initialized";
    // 初始化可显示属性
    initFilterKey();
}

void DeviceMemory::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Setting memory info from lshw data";
    // 由lshw设置基本信息
    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "description", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "slot", m_Locator);
    setAttribute(mapInfo, "size", m_Size);
    qCInfo(appLog) << "Basic memory attributes set - Name:" << m_Name << "Vendor:" << m_Vendor << "Size:" << m_Size;

    // 替换内存大小单位
    qCInfo(appLog) << "Processing memory size units conversion";
    if (m_Size.contains("GiB")) {
        qCInfo(appLog) << "Converting GiB to GB";
        m_Size.replace("GiB", "GB");
    }

    if (m_Size.contains("MiB")) {
        qCInfo(appLog) << "Converting MiB to GB, original value:" << m_Size;
        m_Size.replace("MiB", "");
        // MiB换算MB
        double size = m_Size.toDouble() / 1024.0;
#ifdef __sw_64__ //lutianyu 2021-2-4 num is not even
        if (int(size) % 2 != 0)
            size = size + 1;
#endif
        m_Size = QString::number(size, 'g', 0) + QString("GB");
    }

    // 设置内存速率
    setAttribute(mapInfo, "clock", m_Speed);

    // 由lshw设置基本信息
    setAttribute(mapInfo, "width", m_TotalBandwidth);
    setAttribute(mapInfo, "width", m_DataBandwidth);
    setAttribute(mapInfo, "serial", m_SerialNumber);

    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "Finished setting memory info from lshw";
}

TomlFixMethod DeviceMemory::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Setting memory info from TOML one by one";
    TomlFixMethod ret = TOML_None;
        // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Size", m_Size);
    ret = setTomlAttribute(mapInfo, "Type", m_Type);
    ret = setTomlAttribute(mapInfo, "Speed", m_Speed);
    ret = setTomlAttribute(mapInfo, "Total Width", m_TotalBandwidth);
    ret = setTomlAttribute(mapInfo, "Locator", m_Locator);
    ret = setTomlAttribute(mapInfo, "Serial Number", m_SerialNumber);
    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Configured Voltage", m_ConfiguredVoltage);
    ret = setTomlAttribute(mapInfo, "Maximum Voltage", m_MaximumVoltage);
    ret = setTomlAttribute(mapInfo, "Minimum Voltage", m_MinimumVoltage);
    ret = setTomlAttribute(mapInfo, "Configured Speed", m_ConfiguredSpeed);
    ret = setTomlAttribute(mapInfo, "Data Width", m_DataBandwidth);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "Finished setting memory info from TOML";
    return ret;
}

bool DeviceMemory::setInfoFromDmidecode(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Setting memory info from dmidecode data";
    // 由 locator属性判断是否为同一内存条
    if (mapInfo["Locator"] != m_Locator || m_MatchedFromDmi == true) {
        qCDebug(appLog) << "Memory locator does not match or already matched from DMI, skipping";
        return false;
    }

    // 由dmidecode设置基本属性
    setAttribute(mapInfo, "Part Number", m_Name);
    setAttribute(mapInfo, "Serial Number", m_SerialNumber);
    qCInfo(appLog) << "Basic attributes set from dmidecode - Name:" << m_Name << "Serial:" << m_SerialNumber;

    setAttribute(mapInfo, "Speed", m_Speed);
    // 设置配置频率
    setAttribute(mapInfo, "Configured Memory Speed", m_ConfiguredSpeed);

    // 由dmidecode设置基本属性
    setAttribute(mapInfo, "Minimum Voltage", m_MinimumVoltage);
    setAttribute(mapInfo, "Maximum Voltage", m_MaximumVoltage);
    setAttribute(mapInfo, "Configured Voltage", m_ConfiguredVoltage);
    setAttribute(mapInfo, "Total Width", m_TotalBandwidth);
    setAttribute(mapInfo, "Data Width", m_DataBandwidth);

    // 设置类型
    setAttribute(mapInfo, "Type", m_Type);
    if (m_Type == "<OUT OF SPEC>") {
        qCDebug(appLog) << "Memory type is '<OUT OF SPEC>', clearing it";
        m_Type = "";
    }

    // 获取其他属性
    getOtherMapInfo(mapInfo);
    m_MatchedFromDmi = true;
    qCDebug(appLog) << "Successfully set memory info from dmidecode";
    return true;
}

void DeviceMemory::initFilterKey()
{
    qCDebug(appLog) << "Initializing filter keys for memory device";
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
    qCDebug(appLog) << "Filter keys initialized";
}

void DeviceMemory::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "Loading base device info for memory";
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Size"), m_Size);
    addBaseDeviceInfo(tr("Type"), m_Type);
    addBaseDeviceInfo(tr("Speed"), m_Speed);
    addBaseDeviceInfo(tr("Total Width"), m_TotalBandwidth);
    addBaseDeviceInfo(tr("Locator"), m_Locator);
    addBaseDeviceInfo(tr("Serial Number"), m_SerialNumber);
    qCDebug(appLog) << "Base device info loaded";
}

void DeviceMemory::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "Loading other device info for memory";
    // 倒序，头插，保证原来的顺序
    // 添加其他信息,成员变量
    if (Common::boardVendorType().isEmpty()) {
        qCDebug(appLog) << "Board vendor type is empty, loading full voltage info";
        addOtherDeviceInfo(tr("Configured Voltage"), m_ConfiguredVoltage);
        addOtherDeviceInfo(tr("Maximum Voltage"), m_MaximumVoltage);
        addOtherDeviceInfo(tr("Minimum Voltage"), m_MinimumVoltage);
        addOtherDeviceInfo(tr("Configured Speed"), m_ConfiguredSpeed);
    } else {
        qCDebug(appLog) << "Board vendor type is not empty, loading configured speed only";
        addOtherDeviceInfo(tr("Configured Speed"), m_ConfiguredSpeed);
    }
    addOtherDeviceInfo(tr("Data Width"), m_DataBandwidth);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
    qCDebug(appLog) << "Other device info loaded";
}

void DeviceMemory::loadTableHeader()
{
    qCDebug(appLog) << "Loading table header for memory";
    // 加载表头
    m_TableHeader.append(tr("Name"));
    m_TableHeader.append(tr("Vendor"));
    m_TableHeader.append(tr("Type"));
    m_TableHeader.append(tr("Speed"));
    m_TableHeader.append(tr("Size"));
    qCDebug(appLog) << "Table header loaded";
}

void DeviceMemory::loadTableData()
{
    qCDebug(appLog) << "Loading table data for memory";
    // 加载表格内容
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Type);
    m_TableData.append(m_Speed);
    m_TableData.append(m_Size);
    qCDebug(appLog) << "Table data loaded";
}

const QString &DeviceMemory::name()const
{
    // qCDebug(appLog) << "Getting memory name:" << m_Name;
    return m_Name;
}

const QString &DeviceMemory::vendor() const
{
    // qCDebug(appLog) << "Getting memory vendor:" << m_Vendor;
    return m_Vendor;
}

const QString &DeviceMemory::driver() const
{
    // qCDebug(appLog) << "Getting memory driver:" << m_Driver;
    return m_Driver;
}

bool DeviceMemory::available()
{
    // qCDebug(appLog) << "Checking memory availability, returning true";
    return true;
}

QString DeviceMemory::subTitle()
{
    // qCDebug(appLog) << "Getting memory subtitle";
    // 获取子标题
    return m_Vendor + " " + m_Name;
}

const QString DeviceMemory::getOverviewInfo()
{
    qCDebug(appLog) << "Getting memory overview information";
    // 获取概况信息
    QString ov;
    QString nameStr = m_Name != "" ? m_Name : m_Vendor;
    if (nameStr == "--") {
        qCDebug(appLog) << "Name string is '--', clearing it";
        nameStr.clear();
    }
    ov += QString("%1(%2 %3 %4)") \
          .arg(m_Size) \
          .arg(nameStr) \
          .arg(m_Type) \
          .arg(m_Speed);

    qCDebug(appLog) << "Memory overview info:" << ov;
    return ov;
}
