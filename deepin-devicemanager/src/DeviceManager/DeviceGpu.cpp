// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceGpu.h"
#include "commonfunction.h"
#include "DDLog.h"

// Qt库文件
#include<QLoggingCategory>
#include <QRegularExpression>

using namespace DDLog;

DeviceGpu::DeviceGpu()
    : DeviceBaseInfo()
    , m_Model("")
    , m_GraphicsMemory("")
    , m_Width("")
    , m_DisplayPort("")
    , m_Clock("")
    , m_IRQ("")
    , m_Capabilities("")
    , m_DisplayOutput("")
    , m_VGA("")
    , m_HDMI("")
    , m_eDP("")
    , m_DVI("")
    , m_Digital("")
    , m_CurrentResolution("")
    , m_MinimumResolution("")
    , m_MaximumResolution("")
{
    qCDebug(appLog) << "Device GPU created";
    // 初始化可显示属性
    initFilterKey();
    m_CanUninstall = true;
}

void DeviceGpu::initFilterKey()
{
    qCDebug(appLog) << "DeviceGpu::initFilterKey called.";
    // 添加可显示属性
    addFilterKey("Device");
    addFilterKey("SubVendor");
    addFilterKey("SubDevice");
    addFilterKey("Driver Modules");
    addFilterKey("Config Status");
    addFilterKey("Latency");

    // gpuinfo
    addFilterKey("GDDR capacity");
    addFilterKey("GPU vendor");
    addFilterKey("GPU type");
    addFilterKey("EGL version");
    addFilterKey("EGL client APIs");
    addFilterKey("GL version");
    addFilterKey("GLSL version");
    qCDebug(appLog) << "Filter keys initialized.";
}

void DeviceGpu::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "Loading base device info";

    // 添加基本信息
    addBaseDeviceInfo("Name", m_Name);
    addBaseDeviceInfo("Vendor", m_Vendor);
    addBaseDeviceInfo("Model", m_Model);
    addBaseDeviceInfo("Version", m_Version);
    addBaseDeviceInfo("Graphics Memory", m_GraphicsMemory);
    qCDebug(appLog) << "Base device info loaded.";
}

void DeviceGpu::setLshwInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Setting LSHW info";

    // 判断是否是同一个gpu
    if (!matchToLshw(mapInfo)) {
        qCDebug(appLog) << "Not the same GPU";
        return;
    }

    // 设置属性
    if ((m_Name.isEmpty() || m_Name.startsWith("pci")) && mapInfo.contains("product") && !mapInfo["product"].startsWith("pci")) {
        qCDebug(appLog) << "Setting model from Lshw product.";
        m_Model = mapInfo["product"];
    }
    setAttribute(mapInfo, "product", m_Name, m_Name.isEmpty() || (m_Name.startsWith("pci") && mapInfo.contains("product") && !mapInfo["product"].startsWith("pci")));
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "width", m_Width, false);
    setAttribute(mapInfo, "clock", m_Clock);
    setAttribute(mapInfo, "irq", m_IRQ);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "description", m_Description);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "ioport", m_IOPort);
    setAttribute(mapInfo, "memory", m_MemAddress);
    // setAttribute(mapInfo, "physical id", m_PhysID);
    qCDebug(appLog) << "Lshw attributes set.";

    if (driverIsKernelIn(m_Driver) || m_Driver.isEmpty()) {
        qCDebug(appLog) << "Driver is kernel module or empty";
        m_CanUninstall = false;
    }

    // 获取其他属性
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceGpu::setLshwInfo finished.";
}

TomlFixMethod DeviceGpu::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceGpu::setInfoFromTomlOneByOne started.";
    TomlFixMethod ret = TOML_None;
    setTomlAttribute(mapInfo, "Model", m_Model);
    setTomlAttribute(mapInfo, "Graphics Memory", m_GraphicsMemory);
    setTomlAttribute(mapInfo, "Memory Address", m_MemAddress);
    setTomlAttribute(mapInfo, "IO Port", m_IOPort);
    setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);
    setTomlAttribute(mapInfo, "Maximum Resolution", m_MaximumResolution);
    setTomlAttribute(mapInfo, "Minimum Resolution", m_MinimumResolution);
    setTomlAttribute(mapInfo, "Current Resolution", m_CurrentResolution);
    setTomlAttribute(mapInfo, "Driver", m_Driver);
    setTomlAttribute(mapInfo, "Clock", m_Clock);
    setTomlAttribute(mapInfo, "DP", m_DisplayPort);
    setTomlAttribute(mapInfo, "eDP", m_eDP);
    setTomlAttribute(mapInfo, "HDMI", m_HDMI);
    setTomlAttribute(mapInfo, "VGA", m_VGA);
    setTomlAttribute(mapInfo, "DVI", m_DVI);
    setTomlAttribute(mapInfo, "DigitalOutput", m_Digital);
    setTomlAttribute(mapInfo, "Display Output", m_DisplayOutput);
    setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);
    setTomlAttribute(mapInfo, "IRQ", m_IRQ);
    setTomlAttribute(mapInfo, "Width", m_Width);
    setAttribute(mapInfo, "Version", m_Version);
    qCDebug(appLog) << "Toml attributes set.";
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceGpu::setInfoFromTomlOneByOne finished, return: " << ret;
    return ret;
}

bool DeviceGpu::setHwinfoInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Setting HWINFO info";

    // 设置属性
    setAttribute(mapInfo, "Vendor", m_Vendor, false);
    setAttribute(mapInfo, "Device", m_Name, true);
    // 如果subdevice有值则使用subdevice
    if (m_Name.isEmpty() || (mapInfo.contains("SubDevice") && mapInfo["Device"].startsWith("pci") && !mapInfo["SubDevice"].startsWith("pci"))) {
        qCDebug(appLog) << "Setting name from SubDevice.";
        setAttribute(mapInfo, "SubDevice", m_Name, true);
    }
    setAttribute(mapInfo, "Model", m_Model);
    if (m_Model.isEmpty() && !m_Name.isEmpty() && !m_Name.startsWith("pci")) {
        qCDebug(appLog) << "Model is empty, setting model to name.";
        m_Model = m_Name;
    }
    setAttribute(mapInfo, "Revision", m_Version, false);
    setAttribute(mapInfo, "IRQ", m_IRQ, false);
    setAttribute(mapInfo, "Driver", m_Driver, false);
    setAttribute(mapInfo, "Width", m_Width);

    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);
    m_PhysID = m_VID_PID;
    qCDebug(appLog) << "Hwinfo attributes set.";

    if (driverIsKernelIn(m_Driver) || m_Driver.isEmpty()) {
        qCDebug(appLog) << "Driver is kernel module or empty";
        m_CanUninstall = false;
    }

    m_SysPath = mapInfo["SysFS ID"];
    QRegularExpression reUniqueId("[a-zA-Z0-9_+-]{4}\\.(.*)");
    if (reUniqueId.match(mapInfo["Unique ID"]).hasMatch()) {
        qCDebug(appLog) << "Unique ID matched regex, setting m_UniqueID.";
        m_UniqueID = reUniqueId.match(mapInfo["Unique ID"]).captured(1);
    }
    // read gpu-info file
    if (!m_SysPath.isEmpty()) {
        qCDebug(appLog) << "SysPath is not empty, attempting to read gpu-info file.";
        QString pathStr = "/sys" + m_SysPath + "/gpu-info";
        QFile file(pathStr);
        if (file.open(QIODevice::ReadOnly)) {
            qCDebug(appLog) << "gpu-info file opened successfully.";
            QString allStr(file.readAll());
            QStringList items = allStr.split("\n\n");
            file.close();
            foreach (const QString &item, items) {
                if (item.isEmpty()) {
                    // qCDebug(appLog) << "Skipping empty item in gpu-info.";
                    continue;
                }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QStringList items = allStr.split(":", QString::SkipEmptyParts);
#else
                QStringList items = allStr.split(":", Qt::SkipEmptyParts);
#endif
                if (items.size() != 2) {
                    // qCDebug(appLog) << "Skipping item with incorrect size in gpu-info: " << item;
                    continue;
                }
                if (items.first().trimmed() == "VRAM total size") {
                    bool ok;
                    quint64 vramSize = items.last().trimmed().toULong(&ok, 16);
                    if (ok && vramSize >= 1048576) {
                        // qCDebug(appLog) << "VRAM total size found and valid, converting.";
                        vramSize /= 1048576;
                        auto curSize = vramSize / 1024.0;
                        if (curSize > 1) {
                            m_GraphicsMemory = QString::number(curSize) + "GB";
                        } else {
                            m_GraphicsMemory = QString::number(vramSize) + "MB";
                        }
                        // qCDebug(appLog) << "Graphics Memory set to: " << m_GraphicsMemory;
                    }
                }
            }
        } else {
            qCDebug(appLog) << "Failed to open gpu-info file: " << pathStr;
        }
    }
    QString jjwFile = "/proc/gpuinfo_0";
    if (m_VID_PID.contains("0731") && QFile::exists(jjwFile)) {
        qCDebug(appLog) << "VID_PID contains '0731' and jjwFile exists, attempting to read jjwFile.";
        QFile file(jjwFile);
        if (file.open(QIODevice::ReadOnly)) {
            qCDebug(appLog) << "jjwFile opened successfully.";
            QString allStr(file.readAll());
            QStringList infos = allStr.split("\n");
            file.close();
            foreach (const QString &item, infos) {
                if (item.isEmpty()) {
                    // qCDebug(appLog) << "Skipping empty item in jjwFile.";
                    continue;
                }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QStringList items = item.split(":", QString::SkipEmptyParts);
#else
                QStringList items = item.split(":", Qt::SkipEmptyParts);
#endif
                if (items.size() != 2) {
                    // qCDebug(appLog) << "Skipping item with incorrect size in jjwFile: " << item;
                    continue;
                }
                if (items.first().trimmed() == "Memory Size") {
                    m_GraphicsMemory = items.last().trimmed();
                    // qCDebug(appLog) << "Graphics Memory set from jjwFile: " << m_GraphicsMemory;
                    break;
                }
            }
        } else {
            qCDebug(appLog) << "Failed to open jjwFile: " << jjwFile;
        }
    }

    // // 获取 匹配到lshw的Key
    setHwinfoLshwKey(mapInfo);
    qCDebug(appLog) << "DeviceGpu::setHwinfoInfo finished.";

    // getOtherMapInfo(mapInfo);
    return true;
}

void DeviceGpu::setXrandrInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceGpu::setXrandrInfo started.";
    // 设置分辨率属性
    m_MinimumResolution = mapInfo["minResolution"];
    m_CurrentResolution = mapInfo["curResolution"];
    m_MaximumResolution = mapInfo["maxResolution"];
    qCDebug(appLog) << "Resolutions set: Min=" << m_MinimumResolution << ", Cur=" << m_CurrentResolution << ", Max=" << m_MaximumResolution;

    // 设置显卡支持的接口
    if (mapInfo.find("HDMI") != mapInfo.end()) {
        qCDebug(appLog) << "HDMI found, setting m_HDMI.";
        m_HDMI = mapInfo["HDMI"];
    }

    if (mapInfo.find("VGA") != mapInfo.end()) {
        qCDebug(appLog) << "VGA found, setting m_VGA.";
        m_VGA = mapInfo["VGA"];
    }

    if (mapInfo.find("DP") != mapInfo.end()) {
        qCDebug(appLog) << "DP found, setting m_DisplayPort.";
        m_DisplayPort = mapInfo["DP"];
    }

    if (mapInfo.find("eDP") != mapInfo.end()) {
        qCDebug(appLog) << "eDP found, setting m_eDP.";
        m_eDP = mapInfo["eDP"];
    }

    if (mapInfo.find("DVI") != mapInfo.end()) {
        qCDebug(appLog) << "DVI found, setting m_DVI.";
        m_DVI = mapInfo["DVI"];
    }

    if (mapInfo.find("DigitalOutput") != mapInfo.end())
        m_Digital = mapInfo["DigitalOutput"];   // bug-105482添加新接口类型
}

void DeviceGpu::setDmesgInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Setting DMESG info";

    if (mapInfo.contains("BusID") && mapInfo["BusID"].size() >= m_HwinfoToLshw.size()
            && mapInfo["BusID"].right(m_HwinfoToLshw.size()) != m_HwinfoToLshw)
        return;
    // Bug-85049 JJW 显存特殊处理
    if (mapInfo["Size"].contains("null")) {
        QString size = mapInfo["Size"];
        m_GraphicsMemory = size.replace("null=", "");
    }

    // 设置设备名称
    if (mapInfo.contains("Device") && !mapInfo["Device"].isEmpty() && m_Name.startsWith("pci")) {
        setAttribute(mapInfo, "Device", m_Name, true);
        m_Model = mapInfo["Device"];
        QMap<QString, QString> devInfo;
        devInfo.insert("Device", mapInfo["Device"]);
        getOtherMapInfo(devInfo);
    }

    // 设置显存大小
    if (mapInfo["Size"].contains(m_HwinfoToLshw)) {
        QString size = mapInfo["Size"];
        m_GraphicsMemory = size.replace(m_HwinfoToLshw + "=", "");
    }
}

void DeviceGpu::setGpuInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Setting GPU info";

    m_HDMI = "";
    m_VGA = "";
    m_DisplayPort = "";
    m_eDP = "";
    m_DVI = "";

    setAttribute(mapInfo, "Name", m_Name);

    // 获取其他属性
    getOtherMapInfo(mapInfo);
}

//  名称(Name) 厂商(Vendor) 型号(Model) 显存(Graphics Memory)
void DeviceGpu::setGpuInfoByCustom(const QMap<QString, QString> &mapInfo)
{
    setAttribute(mapInfo, "Name", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Graphics Memory", m_GraphicsMemory);
}

const QString &DeviceGpu::name() const
{
    // qCDebug(appLog) << "DeviceGpu::name called, returning: " << m_Name;
    return m_Name;
}

const QString &DeviceGpu::vendor() const
{
    // qCDebug(appLog) << "DeviceGpu::vendor called, returning: " << m_Vendor;
    return m_Vendor;
}

const QString &DeviceGpu::driver() const
{
    // qCDebug(appLog) << "DeviceGpu::driver called, returning: " << m_Driver;
    return m_Driver;
}

QString DeviceGpu::subTitle()
{
    // qCDebug(appLog) << "DeviceGpu::subTitle called, returning: " << m_Model;
    return m_Model;
}

const QString DeviceGpu::getOverviewInfo()
{
    // qCDebug(appLog) << "Getting GPU overview info";
    // 获取概况信息
    return m_Name.isEmpty() ? m_Model : m_Name;
}

void DeviceGpu::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "Loading other device info";

    QString type = Common::boardVendorType();
    // 添加其他信息,成员变量

    addOtherDeviceInfo("Module Alias", m_Modalias);
    addOtherDeviceInfo("Physical ID", m_PhysID);
    addOtherDeviceInfo("Memory Address", m_MemAddress);
    addOtherDeviceInfo("IO Port", m_IOPort);
    addOtherDeviceInfo("Bus Info", m_BusInfo);
    if (type != Common::specialHString()) {
        addOtherDeviceInfo("Maximum Resolution", m_MaximumResolution);
        addOtherDeviceInfo("Minimum Resolution", m_MinimumResolution);
    }
    addOtherDeviceInfo("Current Resolution", m_CurrentResolution);
    addOtherDeviceInfo("Driver", m_Driver);
    addOtherDeviceInfo("Description", m_Description);
//    addOtherDeviceInfo(("Clock", m_Clock);
    addOtherDeviceInfo("DP", m_DisplayPort);
    addOtherDeviceInfo("eDP", m_eDP);
    addOtherDeviceInfo("HDMI", m_HDMI);
    addOtherDeviceInfo("VGA", m_VGA);
    addOtherDeviceInfo("DVI", m_DVI);
    addOtherDeviceInfo("DigitalOutput", m_Digital);   // bug-105482添加新接口类型
    addOtherDeviceInfo("Display Output", m_DisplayOutput);
    addOtherDeviceInfo("Capabilities", m_Capabilities);
    addOtherDeviceInfo("IRQ", m_IRQ);
//    addOtherDeviceInfo(("Width"), m_Width);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceGpu::loadTableData()
{
    qCDebug(appLog) << "Loading table data";
    // 加载表格内容
    QString tName = m_Name;
    if (!available()) {
        qCDebug(appLog) << "Device not available, updating tName.";
        tName = "(" + translateStr("Unavailable") + ") " + m_Name;
    }
    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
    qCDebug(appLog) << "Table data loaded.";
}
