// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceOtherPCI.h"
#include "DDLog.h"

using namespace DDLog;

DeviceOtherPCI::DeviceOtherPCI()
    : DeviceBaseInfo()
    , m_Model("")
    , m_BusInfo("")
    , m_Width("")
    , m_Clock("")
    , m_Capabilities("")
    , m_Irq("")
    , m_Memory("")
    , m_Latency("")
    , m_InputOutput("")
{
    qCDebug(appLog) << "DeviceOtherPCI::DeviceOtherPCI()";
}

TomlFixMethod DeviceOtherPCI::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceOtherPCI::setInfoFromTomlOneByOne";
    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    setTomlAttribute(mapInfo, "Model", m_Model);
    setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);

    // 添加其他信息,成员变量
    setTomlAttribute(mapInfo, "Input/Output", m_InputOutput);
    setTomlAttribute(mapInfo, "Memory", m_Memory);
    setTomlAttribute(mapInfo, "IRQ", m_Irq);
    setTomlAttribute(mapInfo, "Latency", m_Latency);
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Version);
    //3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

const QString &DeviceOtherPCI::name()const
{
    // qCDebug(appLog) << "DeviceOtherPCI::name";
    return m_Name;
}

const QString &DeviceOtherPCI::vendor() const
{
    // qCDebug(appLog) << "DeviceOtherPCI::vendor";
    return m_Vendor;
}

const QString &DeviceOtherPCI::driver()const
{
    // qCDebug(appLog) << "DeviceOtherPCI::driver";
    return m_Driver;
}

QString DeviceOtherPCI::subTitle()
{
    // qCDebug(appLog) << "DeviceOtherPCI::subTitle";
    return m_Model;
}

const QString DeviceOtherPCI::getOverviewInfo()
{
    // qCDebug(appLog) << "DeviceOtherPCI::getOverviewInfo";
    return m_Name.isEmpty() ? m_Model : m_Name;
}

void DeviceOtherPCI::initFilterKey()
{
    // qCDebug(appLog) << "DeviceOtherPCI::initFilterKey";
}

void DeviceOtherPCI::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "DeviceOtherPCI::loadBaseDeviceInfo";
    // 添加基本信息
    addBaseDeviceInfo("Name", m_Name.isEmpty() ? m_Vendor + m_Model : m_Name);
    addBaseDeviceInfo("Vendor", m_Vendor);
    addBaseDeviceInfo("Model", m_Model);
    addBaseDeviceInfo("Bus Info", m_BusInfo);
    addBaseDeviceInfo("Version", m_Version);
}

void DeviceOtherPCI::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "DeviceOtherPCI::loadOtherDeviceInfo";
    // 添加其他信息,成员变量
    addOtherDeviceInfo("Input/Output", m_InputOutput);
    addOtherDeviceInfo("Memory", m_Memory);
    addOtherDeviceInfo("IRQ", m_Irq);
    addOtherDeviceInfo("Latency", m_Latency);
//    addOtherDeviceInfo("Clock", m_Clock);
//    addOtherDeviceInfo("Width", m_Width);
    addOtherDeviceInfo("Driver", m_Driver);
    addOtherDeviceInfo("Capabilities", m_Version);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceOtherPCI::loadTableData()
{
    qCDebug(appLog) << "DeviceOtherPCI::loadTableData";
    // 加载表格数据
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
