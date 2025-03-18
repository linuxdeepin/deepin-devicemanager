// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DevicePrint.h"
#include "DBusEnableInterface.h"

// Qt库文件
#include <QLoggingCategory>

DevicePrint::DevicePrint()
    : DeviceBaseInfo()
    , m_Model("")
    , m_SerialNumber("")
    , m_InterfaceType("")
    , m_URI("")
    , m_Status("")
    , m_Shared("")
    , m_MakeAndModel("")
{
    // 初始化可显示属性
    initFilterKey();

    // 设备可禁用
    m_CanEnable = true;
    m_CanUninstall = false;
}

TomlFixMethod DevicePrint::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Model", m_Model);
    ret = setTomlAttribute(mapInfo, "Serial Number", m_SerialNumber);
    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Shared", m_Shared);
    ret = setTomlAttribute(mapInfo, "URI", m_URI);
    ret = setTomlAttribute(mapInfo, "Status", m_Status);
    ret = setTomlAttribute(mapInfo, "Interface Type", m_InterfaceType);
    ret = setTomlAttribute(mapInfo, "printer-make-and-model", m_MakeAndModel);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

void DevicePrint::setInfo(const QMap<QString, QString> &info)
{
    // 获取打印机类型和型号
    QString vt;
    setAttribute(info, "printer-info", vt);
    QStringList lstInfo = vt.split(" ");
    if (lstInfo.size() > 1) {
        m_Vendor = lstInfo[0];
        m_Model = vt.replace(m_Vendor, "").trimmed();
    }

    // 获取打印机名称
    setAttribute(info, "Name", m_Name);

    // 获取其它信息
    setAttribute(info, "device-uri", m_URI);
    setAttribute(info, "printer-state", m_Status);
    setAttribute(info, "printer-make-and-model", m_MakeAndModel);

    // 获取打印机接口
    QStringList lstUri = m_URI.split(":");
    if (lstUri.size() > 1)
        m_InterfaceType = lstUri[0];

    getOtherMapInfo(info);
}

const QString &DevicePrint::name()const
{
    return m_Name;
}

const QString &DevicePrint::vendor() const
{
    return m_Vendor;
}

const QString DevicePrint::makeAndeModel() const
{
    return m_MakeAndModel;
}

const QString &DevicePrint::driver() const
{
    return m_Driver;
}

bool DevicePrint::available()
{
    return true;
}

QString DevicePrint::subTitle()
{
    return m_Name;
}

const QString DevicePrint::getOverviewInfo()
{
    // 获取概况信息
    return m_Name.isEmpty() ? m_Model : m_Name;
}

EnableDeviceStatus DevicePrint::setEnable(bool e)
{
    bool res  = DBusEnableInterface::getInstance()->enablePrinter("printer", m_Name, m_URI, e);
    if (res) {
        m_Enable = e;
    }
    // 设置设备状态
    return res ? EDS_Success : EDS_Faild;
}

bool DevicePrint::enable()
{
    return m_Status == "5" ? false : true;
}

void DevicePrint::initFilterKey()
{
    // 初始化可显示属性
    addFilterKey("copies");
    addFilterKey("job-cancel-after");
    addFilterKey("job-hold-until");
    addFilterKey("job-priority");
    addFilterKey("marker-change-time");
    addFilterKey("number-up");
    addFilterKey("orientation-requested");
    addFilterKey("print-color-mode");
    addFilterKey("printer-is-accepting-jobs");
    addFilterKey("printer-is-shared");
    addFilterKey("printer-is-temporary");
//    addFilterKey("printer-make-and-model");
    addFilterKey("printer-state-change-time");
    addFilterKey("printer-state-reasons");
    addFilterKey("printer-type");
    addFilterKey("printer-uri-supported");
    addFilterKey("sides");
}

void DevicePrint::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(("Name"), m_Name);
    addBaseDeviceInfo(("Model"), m_Model);
    addBaseDeviceInfo(("Vendor"), m_Vendor);
    addBaseDeviceInfo(("Serial Number"), m_SerialNumber);
}

void DevicePrint::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(("Shared"), m_Shared);
    addOtherDeviceInfo(("URI"), m_URI);
    addOtherDeviceInfo(("Status"), m_Status);
    addOtherDeviceInfo(("Interface Type"), m_InterfaceType);
    addOtherDeviceInfo(("printer-make-and-model"), m_MakeAndModel);
    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DevicePrint::loadTableData()
{
    // 加载表格数据
    QString tName = m_Name;
    if (!enable()) {
        tName = "(" + translateStr("Disable") + ") " + m_Name;
    }

    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
