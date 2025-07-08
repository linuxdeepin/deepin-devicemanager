// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceInput.h"
#include "DeviceManager.h"
#include "DBusEnableInterface.h"
#include "DBusTouchPad.h"
#include "DBusWakeupInterface.h"
#include "DDLog.h"

// Qt库文件
#include <QLoggingCategory>
#include <QProcess>
#include <QRegularExpression>

using namespace DDLog;

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
    qCDebug(appLog) << "DeviceInput constructor called.";
    initFilterKey();
    m_CanEnable = true;
    m_CanUninstall = true;
}

bool DeviceInput::setInfoFromlshw(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "setInfoFromlshw";
    // 根据bus info属性值与m_KeyToLshw对比,判断是否为同一设备
    if (!matchToLshw(mapInfo)) {
        qCDebug(appLog) << "not match to lshw";
        return false;
    }

    // 设置基础设备信息
    if (m_Vendor.isEmpty() || m_Vendor.contains("0x")) {
        qCDebug(appLog) << "Vendor is empty or contains '0x', setting vendor from mapInfo.";
        setAttribute(mapInfo, "vendor", m_Vendor);
    }
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
        qCDebug(appLog) << "Driver is kernel-in, setting m_CanUninstall to false.";
        m_CanUninstall = false;
    }
    // 当驱动为空，但是又是ps/2鼠键时，驱动不可更新卸载
    if (m_Driver.isEmpty() && "PS/2" == m_Interface) {
        qCDebug(appLog) << "Driver is empty and interface is PS/2, setting m_CanUninstall to false.";
        m_CanUninstall = false;
    }

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);

    qCDebug(appLog) << "Exiting setInfoFromlshw. Returning true.";
    return true;
}

TomlFixMethod DeviceInput::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "setInfoFromTomlOneByOne";

    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Model", m_Model);
    ret = setTomlAttribute(mapInfo, "Interface", m_Interface);
    ret = setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);
    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Speed", m_Speed);
    ret = setTomlAttribute(mapInfo, "Maximum Current", m_MaximumPower);   // 将最大功率改为最大电流
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);

//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "Exiting setInfoFromTomlOneByOne.";
    return ret;
}

void DeviceInput::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "setInfoFromHwinfo";

    //取触摸板的状态
    if (mapInfo.find("Model") != mapInfo.end() && mapInfo["Model"].contains("Touchpad", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "Model contains Touchpad, getting enable status from DBusTouchPad.";
        m_Enable = DBusTouchPad::instance()->getEnable();
    }

    if (mapInfo.find("Enable") != mapInfo.end()) {
        qCDebug(appLog) << "Found 'Enable' in mapInfo, setting m_Enable to false.";
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
    if (m_SerialID.isEmpty()) {
        qCDebug(appLog) << "Serial ID is empty, setting from Unique ID.";
        m_SerialID = m_UniqueID;
    }


    // 获取键盘的接口类型
    if (mapInfo.find("Hotplug") != mapInfo.end()) {
        qCDebug(appLog) << "Found 'Hotplug' in mapInfo, setting interface.";
        setAttribute(mapInfo, "Hotplug", m_Interface);
    } else {
        qCDebug(appLog) << "'Hotplug' not found, setting interface to PS/2.";
        m_Interface = "PS/2";
    }

    // 上面的方法不适合蓝牙键盘的获取方法
    if (isBluetoothDevice(mapInfo["Device Files"])) {
        qCDebug(appLog) << "Device is Bluetooth, setting interface to Bluetooth.";
        m_Interface = "Bluetooth";
    }

    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);
    if (driverIsKernelIn(m_Driver)) {
        qCDebug(appLog) << "Driver is kernel-in, setting m_CanUninstall to false.";
        m_CanUninstall = false;
    }
    // 当驱动为空，但是又是ps/2鼠键时，驱动不可更新卸载
    if (m_Driver.isEmpty() && "PS/2" == m_Interface) {
        qCDebug(appLog) << "Driver is empty and interface is PS/2, setting m_CanUninstall to false.";
        m_CanUninstall = false;
    }

    // ps2键盘的接口 将Device Files作为syspath解析
    if ("PS/2" == m_Interface) {
        qCDebug(appLog) << "Interface is PS/2, getting PS2 Syspath.";
        getPS2Syspath(mapInfo["Device Files"]);
        if (m_Model.contains("Mouse", Qt::CaseInsensitive)) {
            qCDebug(appLog) << "Model contains Mouse, setting m_CanEnable to false.";
            m_CanEnable = false;
        }
    }

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    setHwinfoLshwKey(mapInfo);
    qCDebug(appLog) << "Hwinfo Lshw Key set.";

    // 由bluetoothctl paired-devices设置设备接口
    setInfoFromBluetoothctl();
    qCDebug(appLog) << "Info set from Bluetoothctl.";

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "Exiting setInfoFromHwinfo.";
}

void DeviceInput::setKLUInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "setKLUInfoFromHwinfo";

    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    if (mapInfo.find("Hotplug") != mapInfo.end()) {
        qCDebug(appLog) << "Found 'Hotplug' in mapInfo, setting interface.";
        setAttribute(mapInfo, "Hotplug", m_Interface);
    } else {
        qCDebug(appLog) << "'Hotplug' not found, setting interface to PS/2.";
        m_Interface = "PS/2";
    }

    // 上面的方法不适合蓝牙键盘的获取方法
    if (mapInfo.find("Model")->contains("Bluetooth", Qt::CaseInsensitive) || mapInfo.find("Device")->contains("Bluetooth", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "Model or Device contains Bluetooth, setting interface to Bluetooth.";
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
        qCDebug(appLog) << "SysFS BusID split into 2 words.";
        QStringList chs = words[0].split("-");
        if (chs.size() == 2) {
            qCDebug(appLog) << "First word split into 2 parts, setting m_KeyToLshw.";
            m_KeyToLshw = QString("usb@%1:%2").arg(chs[0]).arg(chs[1]);
        }
    }

    // 由bluetoothctl paired-devices设置设备接口
    setInfoFromBluetoothctl();
    qCDebug(appLog) << "Info set from Bluetoothctl.";

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "Exiting setKLUInfoFromHwinfo.";
}

void DeviceInput::setInfoFromBluetoothctl()
{
    qCDebug(appLog) << "Entering setInfoFromBluetoothctl.";
    // 判断该设备信息是否存在于Bluetoothctl中
    if (isValueValid(m_keysToPairedDevice)) {
        qCDebug(appLog) << "keysToPairedDevice is valid.";
        bool isExist = DeviceManager::instance()->isDeviceExistInPairedDevice(m_keysToPairedDevice.toUpper());

        if (isExist) {
            qCDebug(appLog) << "Device exists in paired device, setting interface to Bluetooth.";
            m_Interface = "Bluetooth";
        }
    }
    qCDebug(appLog) << "Exiting setInfoFromBluetoothctl.";
}

bool DeviceInput::getPS2Syspath(const QString &dfs)
{
    qCDebug(appLog) << "Entering getPS2Syspath with device files: " << dfs;
    // 获取 dfs 中的 event
    QString eventdfs = eventStrFromDeviceFiles(dfs);
    if (eventdfs.isEmpty()) {
        qCDebug(appLog) << "Event string from device files is empty. Returning false.";
        return false;
    }

    QFile file("/proc/bus/input/devices");
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(appLog) << "Failed to open /proc/bus/input/devices. Returning false.";
        return false;
    }

    QString info = file.readAll();
    qCDebug(appLog) << "Successfully read /proc/bus/input/devices.";
    QStringList lstDevices = info.split("\n\n");
    foreach (const QString &item, lstDevices) {
        // qCDebug(appLog) << "Processing device item: " << item.left(50) << "...";
        QStringList lines = item.split("\n");
        QString sysfs = "";
        QString event = "";
        foreach (const QString &line, lines) {
            if (line.startsWith("S:")) {
                sysfs = line;
                // qCDebug(appLog) << "Found sysfs line: " << sysfs;
                continue;
            }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QRegExp reg("H: Handlers=.*(event[0-9]{1,2}).*");
            if (reg.exactMatch(line)) {
                event = reg.cap(1);
                // qCDebug(appLog) << "Matched event using QRegExp: " << event;
            }
#else
            QRegularExpression reg("H: Handlers=.*(event[0-9]{1,2}).*");
            QRegularExpressionMatch match = reg.match(line);
            if (match.hasMatch()) {
                event = match.captured(1);
                // qCDebug(appLog) << "Matched event using QRegularExpression: " << event;
            }
#endif
        }

        if (!event.isEmpty() && !sysfs.isEmpty()) {
            // qCDebug(appLog) << "Event and sysfs are not empty. Checking for match.";
            if (event == eventdfs) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QRegExp regfs;
                if (sysfs.contains("i2c_designware"))
                    regfs = QRegExp("S: Sysfs=(.*)/input/input[0-9]{1,2}");
                else
                    regfs = QRegExp("S: Sysfs=(.*)/input[0-9]{1,2}");
                if (regfs.exactMatch(sysfs)) {
                    m_SysPath = regfs.cap(1);
                }
#else
                QRegularExpression regfs;
                if (sysfs.contains("i2c_designware"))
                    regfs = QRegularExpression("S: Sysfs=(.*)/input/input[0-9]{1,2}");
                else
                    regfs = QRegularExpression("S: Sysfs=(.*)/input[0-9]{1,2}");
                QRegularExpressionMatch match = regfs.match(sysfs);
                if (match.hasMatch()) {
                    m_SysPath = match.captured(1);
                }
#endif
            }
        }
    }

    qCDebug(appLog) << "getPS2Syspath end";
    return true;
}

bool DeviceInput::isBluetoothDevice(const QString &dfs)
{
    qCDebug(appLog) << "isBluetoothDevice";
    // 获取 dfs 中的 event
    QString eventdfs = eventStrFromDeviceFiles(dfs);
    if (eventdfs.isEmpty()) {
        qCDebug(appLog) << "eventdfs is empty";
        return false;
    }

    QFile file("/proc/bus/input/devices");
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(appLog) << "open file failed";
        return false;
    }

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QRegExp reg("H: Handlers=.*(event[0-9]{1,2}).*");
            if (reg.exactMatch(line)) {
                event = reg.cap(1);
            }
#else
            QRegularExpression reg("H: Handlers=.*(event[0-9]{1,2}).*");
            QRegularExpressionMatch match = reg.match(line);
            if (match.hasMatch()) {
                event = match.captured(1);
            }
#endif
        }

        if (event == eventdfs) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QRegExp regUniq(".*([0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}).*");
            if (regUniq.exactMatch(Uniq)) {
                QString id = regUniq.cap(1);
#else
            QRegularExpression regUniq(".*([0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}:[0-9A-Z]{2}).*");
            QRegularExpressionMatch match = regUniq.match(Uniq);
            if (match.hasMatch()) {
                QString id = match.captured(1);
#endif
                QProcess process;
                process.start("hcitool con");
                process.waitForFinished(-1);
                QString hciInfo = process.readAllStandardOutput();
                if (!hciInfo.contains(id))
                    m_BluetoothIsConnected = false;
                qCDebug(appLog) << "id:" << id << "hciInfo:" << hciInfo << "m_BluetoothIsConnected:" << m_BluetoothIsConnected;
                return true;
            }
            break;
        }
    }

    qCDebug(appLog) << "not found in bluetoothctl";
    return false;
}

QString DeviceInput::eventStrFromDeviceFiles(const QString &dfs)
{
    qCDebug(appLog) << "eventStrFromDeviceFiles";
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRegularExpression regdfs(".*(event[0-9]{1,2}).*");
    QRegularExpressionMatch match = regdfs.match(dfs);
    if (match.hasMatch())
        return match.captured(1);
#else
    QRegExp regdfs(".*(event[0-9]{1,2}).*");
    if (regdfs.exactMatch(dfs))
        return regdfs.cap(1);
#endif
    qCDebug(appLog) << "eventStrFromDeviceFiles end";
    return "";
}

const QString &DeviceInput::name() const
{
    // qCDebug(appLog) << "Entering name() const.";
    // qCDebug(appLog) << "Exiting name() const. Returning: " << m_Name;
    return m_Name;
}

const QString &DeviceInput::vendor() const
{
    // qCDebug(appLog) << "vendor";
    return m_Vendor;
}

const QString &DeviceInput::driver() const
{
    // qCDebug(appLog) << "driver";
    return m_Driver;
}

bool DeviceInput::available()
{
    // qCDebug(appLog) << "available";
    if (driver().isEmpty()) {
        // qCDebug(appLog) << "driver is empty";
        m_Available = false;
    }
    if ("PS/2" == m_Interface || "Bluetooth" == m_Interface) {
        // qCDebug(appLog) << "interface is PS/2 or Bluetooth";
        m_Available = true;
    }
    // qCDebug(appLog) << "available end";
    return m_forcedDisplay ? m_forcedDisplay : m_Available;
}

QString DeviceInput::subTitle()
{
    // qCDebug(appLog) << "subTitle";
    // 获取子标题
    if (!m_Name.isEmpty()) {
        // qCDebug(appLog) << "name is not empty";
        return m_Name;
    }
    // qCDebug(appLog) << "name is empty";
    return m_Model;
}

const QString DeviceInput::getOverviewInfo()
{
    // qCDebug(appLog) << "getOverviewInfo";
    // 获取概况信息
    QString ov = QString("%1 (%2)") \
                 .arg(m_Name) \
                 .arg(m_Model);
    // qCDebug(appLog) << "getOverviewInfo end, ov:" << ov;
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
    qCDebug(appLog) << "setEnable";
    if (m_Name.contains("Touchpad", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "setEnable touchpad";
        DBusTouchPad::instance()->setEnable(e);
        m_Enable = e;
        return EDS_Success;
    } else {
        if (m_SerialID.isEmpty()) {
            qCDebug(appLog) << "setEnable no serial";
            return EDS_NoSerial;
        }

        if (m_UniqueID.isEmpty() || m_SysPath.isEmpty()) {
            qCDebug(appLog) << "setEnable no unique id or sys path";
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
        qCDebug(appLog) << "setEnable res:" << res;
        return res ? EDS_Success : EDS_Faild;
    }
}

bool DeviceInput::enable()
{
    // qCDebug(appLog) << "enable";
    // 键盘不可禁用
    if (m_HardwareClass == "keyboard") {
        // qCDebug(appLog) << "enable keyboard";
        m_Enable = true;
    }
    // qCDebug(appLog) << "enable end";
    return m_Enable;
}

bool DeviceInput::canWakeupMachine()
{
    qCDebug(appLog) << "canWakeupMachine";
    if (m_WakeupID.isEmpty() || (m_HardwareClass == "keyboard" && "PS/2" == m_Interface)) {
        qCDebug(appLog) << "canWakeupMachine return false";
        return false;
    }
    QFile file(wakeupPath());
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(appLog) << "canWakeupMachine open file failed";
        return false;
    }
    qCDebug(appLog) << "canWakeupMachine end";
    return true;
}

bool DeviceInput::isWakeupMachine()
{
    qCDebug(appLog) << "isWakeupMachine";
    QFile file(wakeupPath());
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(appLog) << "open wakeup file failed";
        return false;
    }
    QString info = file.readAll();

    if (m_Name.contains("PS/2")) {
//        QStringList lines = info.split("\n");
//        for (QString line : lines) {
//            if (line.startsWith("PS2M" && line.contains("disabled"))) {
//                return false;
//            }
        qCDebug(appLog) << "return ps2 wakeup";
        // /proc/acpi/wakeup文件中状态未刷新，ps2设备通过dbus获取状态
        return DBusWakeupInterface::getInstance()->isInputWakeupMachine(m_SysPath, m_Name);

    } else {
        if (info.contains("disabled")) {
            qCDebug(appLog) << "wakeup is disabled";
            return false;
        }
    }

    qCDebug(appLog) << "wakeup is enabled";
    return true;
}

QString DeviceInput::wakeupPath()
{
    qCDebug(appLog) << "wakeupPath";
    int index = m_SysPath.lastIndexOf('/');
    if (index < 1) {
        qCDebug(appLog) << "wakeupPath index < 1";
        return "";
    }

    if (m_Name.contains("PS/2")) {
        qCDebug(appLog) << "wakeupPath ps2";
        return "/proc/acpi/wakeup";
    } else {
        qCDebug(appLog) << "wakeupPath usb";
        return QString("/sys") + m_SysPath.left(index) + QString("/power/wakeup");
    }
    qCDebug(appLog) << "wakeupPath end";
}

const QString &DeviceInput::wakeupID()
{
    // qCDebug(appLog) << "wakeupID";
    return m_WakeupID;
}

bool DeviceInput::bluetoothIsConnected()
{
    // qCDebug(appLog) << "bluetoothIsConnected";
    return m_BluetoothIsConnected;
}

void DeviceInput::initFilterKey()
{
    // 添加可显示的设备信息
    qCDebug(appLog) << "initFilterKey";
    addFilterKey(QObject::tr("Uniq"));
    addFilterKey(QObject::tr("PROP"));
    addFilterKey(QObject::tr("EV"));
    addFilterKey(QObject::tr("KEY"));
    addFilterKey(QObject::tr("MSC"));
    addFilterKey(QObject::tr("Device File"));
    addFilterKey(QObject::tr("Hardware Class"));

    // addFilterKey(QObject::tr("physical id"));
    qCDebug(appLog) << "initFilterKey end";
}

void DeviceInput::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "loadBaseDeviceInfo";
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Interface"), m_Interface);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
    qCDebug(appLog) << "loadBaseDeviceInfo end";
}

void DeviceInput::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "loadOtherDeviceInfo";
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Module Alias"), m_Modalias);
    addOtherDeviceInfo(tr("Physical ID"), m_PhysID);
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Maximum Current"), m_MaximumPower);   // 1050需求将最大功率改为最大电流
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(tr("Version"), m_Version);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
    qCDebug(appLog) << "loadOtherDeviceInfo end";
}

void DeviceInput::loadTableData()
{
    qCDebug(appLog) << "loadTableData";
    // 加载表格数据
    QString tName = m_Name;

    if (!available()) {
        qCDebug(appLog) << "loadTableData unavailable";
        tName = "(" + tr("Unavailable") + ") " + m_Name;
    }

    if (!enable()) {
        qCDebug(appLog) << "loadTableData disable";
        tName = "(" + tr("Disable") + ") " + m_Name;
    }

    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
    qCDebug(appLog) << "loadTableData end";
}

