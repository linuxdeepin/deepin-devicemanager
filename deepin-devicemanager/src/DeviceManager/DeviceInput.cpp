// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceInput.h"
#include "DeviceManager.h"
#include "DBusEnableInterface.h"
#include "DBusTouchPad.h"
#include "DBusWakeupInterface.h"

// Qt库文件
#include <QLoggingCategory>
#include <QProcess>

QStringList DeviceInput::m_supportInterfaces= {"PS/2", "Bluetooth", "I2C", "USB"};

DeviceInput::DeviceInput()
    : DeviceBaseInfo()
    , m_Model("")
    , m_Interface("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_MaximumPower("")
    , m_Speed("")
    , m_KeyToLshw("")
    , m_WakeupID("")
    , m_BluetoothIsConnected(true)
{
    initFilterKey();
    m_CanEnable = true;
    m_CanUninstall = true;
}

bool DeviceInput::setInfoFromlshw(const QMap<QString, QString> &mapInfo)
{
    // 根据bus info属性值与m_KeyToLshw对比,判断是否为同一设备
    if (!matchToLshw(mapInfo)) {
        return false;
    }

    // 设置基础设备信息
    if (m_Vendor.isEmpty() || m_Vendor.contains("0x"))
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
    if (driverIsKernelIn(m_Driver)) {
        m_CanUninstall = false;
    }
    // 当驱动为空，但是又是ps/2鼠键时，驱动不可更新卸载
    if (m_Driver.isEmpty() && "PS/2" == m_Interface) {
        m_CanUninstall = false;
    }
    getMouseInfoFromBusDevice();

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);

    return true;
}

TomlFixMethod DeviceInput::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    setTomlAttribute(mapInfo, "Model", m_Model);
    setTomlAttribute(mapInfo, "Interface", m_Interface);
    setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);
    // 添加其他信息,成员变量
    setTomlAttribute(mapInfo, "Speed", m_Speed);
    setTomlAttribute(mapInfo, "Maximum Current", m_MaximumPower);   // 将最大功率改为最大电流
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);

//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

void DeviceInput::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    //取触摸板的状态
    if (mapInfo.find("Model") != mapInfo.end() && mapInfo["Model"].contains("Touchpad", Qt::CaseInsensitive)) {
        m_Enable = DBusTouchPad::instance()->getEnable();
    }

    if (mapInfo.find("Enable") != mapInfo.end()) {
        m_Enable = false;
    }
    // 设置设备基本属性
    setAttribute(mapInfo, "Serial ID", m_SerialID);
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "name", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "SysFS ID", m_SysPath);
    setAttribute(mapInfo, "Unique ID", m_WakeupID);
    setAttribute(mapInfo, "Unique ID", m_UniqueID);
    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);
    m_PhysID = m_VID_PID;
    // 防止Serial ID为空
    if (m_SerialID.isEmpty())
        m_SerialID = m_UniqueID;


    // 获取键盘的接口类型
    if (mapInfo.find("Hotplug") != mapInfo.end())
        setAttribute(mapInfo, "Hotplug", m_Interface);
    else
        m_Interface = "PS/2";

    // 上面的方法不适合蓝牙键盘的获取方法
    if (isBluetoothDevice(mapInfo["Device Files"])) {
        m_Interface = "Bluetooth";
    }

    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);
    if (driverIsKernelIn(m_Driver)) {
        m_CanUninstall = false;
    }
    // 当驱动为空，但是又是ps/2鼠键时，驱动不可更新卸载
    if (m_Driver.isEmpty() && "PS/2" == m_Interface) {
        m_CanUninstall = false;
    }

    // ps2键盘的接口 将Device Files作为syspath解析
    if ("PS/2" == m_Interface) {
        getPS2Syspath(mapInfo["Device Files"]);
        if (m_Model.contains("Mouse", Qt::CaseInsensitive)) {
            m_CanEnable = false;
        }
    }

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    setHwinfoLshwKey(mapInfo);

    // 由bluetoothctl paired-devices设置设备接口
    setInfoFromBluetoothctl();

    getMouseInfoFromBusDevice();
    // 获取其他设备信息
    getOtherMapInfo(mapInfo);
}

void DeviceInput::setInfoFromBluetoothctl()
{
    // 判断该设备信息是否存在于Bluetoothctl中
    if (isValueValid(m_keysToPairedDevice)) {
        bool isExist = DeviceManager::instance()->isDeviceExistInPairedDevice(m_keysToPairedDevice.toUpper());

        if (isExist)
            m_Interface = "Bluetooth";
    }
}

bool DeviceInput::getPS2Syspath(const QString &dfs)
{
    // 获取 dfs 中的 event
    QString eventdfs = eventStrFromDeviceFiles(dfs);
    if (eventdfs.isEmpty())
        return false;

    QFile file("/proc/bus/input/devices");
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QString info = file.readAll();
    QStringList lstDevices = info.split("\n\n");
    foreach (const QString &item, lstDevices) {
        QStringList lines = item.split("\n");
        QString sysfs = "";
        QString event = "";
        foreach (const QString &line, lines) {
            if (line.startsWith("S:")) {
                sysfs = line;
                continue;
            }
            QRegExp reg = QRegExp("H: Handlers=.*(event[0-9]{1,2}).*");
            if (reg.exactMatch(line)) {
                event = reg.cap(1);
            }
        }

        if (!event.isEmpty() && !sysfs.isEmpty()) {
            if (event == eventdfs) {
                QRegExp regfs;
                if (sysfs.contains("i2c_designware"))
                    regfs = QRegExp("S: Sysfs=(.*)/input/input[0-9]{1,2}");
                else
                    regfs = QRegExp("S: Sysfs=(.*)/input[0-9]{1,2}");
                if (regfs.exactMatch(sysfs)) {
                    m_SysPath = regfs.cap(1);
                }
            }
        }
    }

    return true;
}

bool DeviceInput::isBluetoothDevice(const QString &dfs)
{
    // 获取 dfs 中的 event
    QString eventdfs = eventStrFromDeviceFiles(dfs);
    if (eventdfs.isEmpty())
        return false;

    QFile file("/proc/bus/input/devices");
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QString info = file.readAll();
    QStringList lstDevices = info.split("\n\n");
    foreach (const QString &item, lstDevices) {
        QStringList lines = item.split("\n");
        QString Uniq = "";
        QString event = "";
        foreach (const QString &line, lines) {
            if (line.startsWith("U:")) {
                Uniq = line;
                continue;
            }
            QRegExp reg = QRegExp("H: Handlers=.*(event[0-9]{1,2}).*");
            if (reg.exactMatch(line)) {
                event = reg.cap(1);
            }
        }

        if (event == eventdfs) {
            QRegExp regUniq(".*([0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}).*");
            if (regUniq.exactMatch(Uniq)) {
                QString id = regUniq.cap(1);
                QProcess process;
                process.start("hcitool con");
                process.waitForFinished(-1);
                QString hciInfo = process.readAllStandardOutput();
                if (!hciInfo.contains(id))
                    m_BluetoothIsConnected = false;
                return true;
            }
            break;
        }
    }

    return false;
}

QString DeviceInput::eventStrFromDeviceFiles(const QString &dfs)
{
    QRegExp regdfs = QRegExp(".*(event[0-9]{1,2}).*");
    if (regdfs.exactMatch(dfs))
        return regdfs.cap(1);
    return "";
}

InputDeviceBusInfo DeviceInput::getDetailBusInfo(const QString &busId)
{
    InputDeviceBusInfo info;
    info.busId = busId;

    // 根据 Bus ID 查找对应的接口类型信息-只会使用 interfaceType，其他内容预留维护参考
    static QMap<QString, InputDeviceBusInfo> busInfoMap = {
        {"0000", {"0000", "UINPUT", "虚拟设备", "用于软件模拟输入"}},
        {"0001", {"0001", "PCI", "特殊输入卡", "通过 PCI/PCIe 总线连接的设备"}},
        {"0002", {"0002", "ISA", "古老 ISA 设备", "已淘汰的 ISA 接口设备"}},
        {"0003", {"0003", "USB", "USB鼠标/键盘/手柄", "最常见的外设接口，设备路径含 usb"}},
        {"0004", {"0004", "HIL", "HP-HIL终端设备", "历史遗留系统 (HP-HIL)"}},
        {"0005", {"0005", "BLUETOOTH", "蓝牙鼠标/键盘", "设备名称含 bluetooth，需 rfkill 管理"}},
        {"0006", {"0006", "VIRTUAL", "VMware/QEMU虚拟输入", "虚拟机中的输入设备"}},
        {"0007", {"0007", "SERIAL)", "串口鼠标", "设备节点为 /dev/ttyS*"}},
        {"0008", {"0008", "HOST", "内置键盘/触摸板", "通过主板直接连接的设备"}},
        {"0009", {"0009", "Game Port", "游戏手柄", "15针 D-Sub 接口"}},
        {"0010", {"0010", "PARALLEL", "老式输入设备", "/dev/parport*"}},
        {"0011", {"0011", "PS/2", "PS/2鼠标/键盘", "圆形 6-pin 接口，设备节点为 /dev/psaux"}},
        {"0012", {"0012", "RADIO", "无线接收器", "专用 2.4G 设备 (如罗技 Unifying)"}},
        {"0013", {"0013", "J1939", "车载工业设备", "CAN 总线扩展"}},
        {"0018", {"0018", "I2C", "高端触摸板/触控笔", "设备名称含 i2c，需 i2c-tools 调试"}},
        {"0019", {"0019", "SPI", "嵌入式触控屏", "通过 SPI 总线通信"}},
        {"001a", {"001A", "ILLUMINANCE", "环境光传感器", "部分笔记本的自动亮度调节"}},
        {"001b", {"001B", "GDIX", "Surface Dial", "微软 Surface Dial 特殊旋转输入设备"}},
        {"001c", {"001C", "WACOM", "数位板/绘图屏", "设备名含 wacom"}},
        {"001d", {"001D", "UCSI", "USB Type-C输入", "新式笔记本的 USB-C 扩展设备"}}
    };

    // 查找匹配的 Bus ID (不区分大小写)
    QString lowerBusId = busId.toLower();
    if (busInfoMap.contains(lowerBusId)) {
        return busInfoMap.value(lowerBusId);
    }

    // 如果没有找到匹配的，返回未知类型
    info.interfaceType = "Unknown";
    info.typicalDevices = "UnKnown Device";
    info.description = QString("Unrecognized Bus ID: %1").arg(busId);

    return info;
}

void DeviceInput::getMouseInfoFromBusDevice()
{
    QProcess process;
    process.start("cat", QStringList() << "/proc/bus/input/devices");
    process.waitForFinished(10000);
    QString rawContent = process.readAllStandardOutput();

    QMap<QString, QString> nameToBusMap;

    if (rawContent.isEmpty()) {
        return ;
    }

    QStringList deviceBlocks = rawContent.split("\n\n", QString::SkipEmptyParts);

    for (const QString &block : deviceBlocks) {
        QString bus, name;
        QStringList lines = block.split("\n", QString::SkipEmptyParts);

        for (const QString &line : lines) {
            QString trimmedLine = line.trimmed();

            // 提取 Bus 信息 (I: Bus=0019 ...)
            if (trimmedLine.startsWith("I:") && trimmedLine.contains("Bus=")) {
                QRegExp busRegex("Bus=([0-9a-fA-F]+)");
                if (busRegex.indexIn(trimmedLine) != -1) {
                    bus = busRegex.cap(1);
                }
            }

            // 提取 Name 信息 (N: Name="Sleep Button")
            if (trimmedLine.startsWith("N:") && trimmedLine.contains("Name=")) {
                QRegExp nameRegex("Name=\"([^\"]+)\"");
                if (nameRegex.indexIn(trimmedLine) != -1) {
                    name = nameRegex.cap(1);
                }
            }
        }

        // 如果同时找到了 name 和 bus，则添加到映射中
        if (!name.isEmpty() && !bus.isEmpty()) {
            nameToBusMap.insert(name, bus);
        }
    }

    if (nameToBusMap.contains(m_Name)) {
        QString busID = nameToBusMap.value(m_Name);
        m_Interface = getDetailBusInfo(busID).interfaceType;
    }
}

QString DeviceInput::getBusInfo() const
{
    return m_SysPath;
}

QString DeviceInput::getInterface() const
{
    return m_Interface;
}

const QString &DeviceInput::name() const
{
    return m_Name;
}

const QString &DeviceInput::vendor() const
{
    return m_Vendor;
}

const QString &DeviceInput::driver() const
{
    return m_Driver;
}

bool DeviceInput::available()
{
    if (driver().isEmpty()) {
        m_Available = false;
    }

    m_Available = m_supportInterfaces.contains(m_Interface, Qt::CaseInsensitive);

    return m_forcedDisplay ? m_forcedDisplay : m_Available;
}

QString DeviceInput::subTitle()
{
    // 获取子标题
    if (!m_Name.isEmpty())
        return m_Name;
    return m_Model;
}

const QString DeviceInput::getOverviewInfo()
{
    // 获取概况信息
    QString ov = QString("%1 (%2)") \
                 .arg(m_Name) \
                 .arg(m_Model);

    return ov;
}

/**
 * @brief setEnable : 设置禁用
 * @param enable : 启用禁用
 * @return 返回操作是否成功
 *
 * 修改：修改触摸板禁用方法，改为调用daemon提供的接口
 */

EnableDeviceStatus DeviceInput::setEnable(bool e)
{
    if (m_Name.contains("Touchpad", Qt::CaseInsensitive)) {
        DBusTouchPad::instance()->setEnable(e);
        m_Enable = e;
        return EDS_Success;
    } else {
        if (m_SerialID.isEmpty()) {
            return EDS_NoSerial;
        }

        if (m_UniqueID.isEmpty() || m_SysPath.isEmpty()) {
            return EDS_Faild;
        }
        if(e){
            //鼠标启用时，唤醒能力打开
            DBusWakeupInterface::getInstance()->setWakeupMachine(wakeupID(), sysPath(), true, name());
            m_wakeupChanged = true;

        } else if (m_wakeupChanged) { //鼠标禁用时，唤醒能力关闭
            m_wakeupChanged = false;
            DBusWakeupInterface::getInstance()->setWakeupMachine(wakeupID(), sysPath(), false, name());
        }
        bool res  = DBusEnableInterface::getInstance()->enable(m_HardwareClass, m_Name, m_SysPath, m_UniqueID, e, m_Driver);
        if (res) {
            m_Enable = e;
        }
        // 设置设备状态
        return res ? EDS_Success : EDS_Faild;
    }
}

bool DeviceInput::enable()
{
    // 键盘不可禁用
    if (m_HardwareClass == "keyboard") {
        m_Enable = true;
    }
    return m_Enable;
}

bool DeviceInput::canWakeupMachine()
{
    if (m_WakeupID.isEmpty() || (m_HardwareClass == "keyboard" && "PS/2" == m_Interface))
        return false;
    QFile file(wakeupPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    return true;
}

bool DeviceInput::isWakeupMachine()
{
    QFile file(wakeupPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QString info = file.readAll();

    if (m_Name.contains("PS/2") || m_Interface.contains("PS/2")) {
//        QStringList lines = info.split("\n");
//        for (QString line : lines) {
//            if (line.startsWith("PS2M" && line.contains("disabled"))) {
//                return false;
//            }
        // /proc/acpi/wakeup文件中状态未刷新，ps2设备通过dbus获取状态
        return DBusWakeupInterface::getInstance()->isInputWakeupMachine(m_SysPath, m_Name);

    } else {
        if (info.contains("disabled"))
            return false;
    }

    return true;
}

QString DeviceInput::wakeupPath()
{
    int index = m_SysPath.lastIndexOf('/');
    if (index < 1) {
        return "";
    }

    if (m_Name.contains("PS/2")) {
        return "/proc/acpi/wakeup";
    } else {
        return QString("/sys") + m_SysPath.left(index) + QString("/power/wakeup");
    }
}

const QString &DeviceInput::wakeupID()
{
    return m_WakeupID;
}

bool DeviceInput::bluetoothIsConnected()
{
    return m_BluetoothIsConnected;
}

void DeviceInput::initFilterKey()
{
    // 添加可显示的设备信息
    addFilterKey("Uniq");
    addFilterKey("PROP");
    addFilterKey("EV");
    addFilterKey("KEY");
    addFilterKey("MSC");
    addFilterKey("Device File");
    addFilterKey("Hardware Class");

    // addFilterKey("physical id");

}

void DeviceInput::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(("Name"), m_Name);
    addBaseDeviceInfo(("Vendor"), m_Vendor);
    addBaseDeviceInfo(("Model"), m_Model);
    addBaseDeviceInfo(("Interface"), m_Interface);
    addBaseDeviceInfo(("Bus Info"), m_BusInfo);
}

void DeviceInput::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(("Module Alias"), m_Modalias);
    addOtherDeviceInfo(("Physical ID"), m_PhysID);
    addOtherDeviceInfo(("Speed"), m_Speed);
    addOtherDeviceInfo(("Maximum Current"), m_MaximumPower);   // 1050需求将最大功率改为最大电流
    addOtherDeviceInfo(("Driver"), m_Driver);
    addOtherDeviceInfo(("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(("Version"), m_Version);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceInput::loadTableData()
{
    // 加载表格数据
    QString tName = m_Name;

    if (!available()) {
        tName = "(" + translateStr("Unavailable") + ") " + m_Name;
    }

    if (!enable()) {
        tName = "(" + translateStr("Disable") + ") " + m_Name;
    }

    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}

