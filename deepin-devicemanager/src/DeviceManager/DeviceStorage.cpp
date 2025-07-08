// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceStorage.h"
#include "commonfunction.h"
#include <cmath>

// Qt库文件
#include <QDir>
#include "DDLog.h"
// Qt库文件
#include<QLoggingCategory>
#include <QRegularExpression>
using namespace DDLog;

#define DISK_SCALE_1024 1024
#define DISK_SCALE_1000 1000

DeviceStorage::DeviceStorage()
    : DeviceBaseInfo()
    , m_Model("")
//    , m_Vendor("")
    , m_MediaType("")
    , m_Size("")
    , m_SizeBytes(0)
    , m_RotationRate("")
    , m_Interface("")
    , m_SerialNumber("")
    , m_Capabilities("")
    , m_KeyToLshw("")
    , m_KeyFromStorage("")
{
    qCDebug(appLog) << "DeviceStorage constructor";
    // 初始化可显示属性
    initFilterKey();
}

TomlFixMethod DeviceStorage::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceStorage::setInfoFromTomlOneByOne";
    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Media Type", m_MediaType);
    ret = setTomlAttribute(mapInfo, "Size", m_Size);
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);
    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Firmware Version", m_FirmwareVersion);
    ret = setTomlAttribute(mapInfo, "Speed", m_Speed);
    ret = setTomlAttribute(mapInfo, "Serial Number", m_SerialNumber);
    ret = setTomlAttribute(mapInfo, "Interface", m_Interface);
    ret = setTomlAttribute(mapInfo, "Rotation Rate", m_RotationRate);
    //3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

static QString decimalkilos(quint64 value)
{
    qCDebug(appLog) << "decimalkilos";
    const QString prefixes("KMGTPEZY");
    int i = 0;
    quint64 curValue = value;
    QString valueStr = QString();
    while ((i < prefixes.size()) && ((curValue > 1000) || (curValue % 1000 == 0)))
    {
        value = curValue;
        curValue = curValue / 1000;
        i++;
    }

    if (i < 4) {
        qCDebug(appLog) << "decimalkilos, i < 4";
        quint64 diffValue = value - curValue * 1000;
        double calValue = diffValue / 1000.0 + 0.1;
        curValue += static_cast<quint64>(calValue);
        valueStr = QString::number(curValue) + " ";
        if (i > 0)
            valueStr += prefixes[i - 1];
    } else if (i <= prefixes.size()) { // 单位T以上处理
        qCDebug(appLog) << "decimalkilos, i <= prefixes.size()";
        if (value % 1000 >= 1)
            valueStr = QString::number(value) + " " + prefixes[i - 2]; //保留小数部分 如1920GB 10001GB
        else
            valueStr = QString::number(curValue) + " " + prefixes[i - 1]; //无小数部分入整 如1TB
    }
    valueStr += "B";
    return valueStr;
}

static quint64 convertToBytes(const QString& size, double scale)
{
    qCDebug(appLog) << "convertToBytes";
    /*convert "KB MB GB TB PB EB ZB YB " to bytes */
    const QString prefixes("KMGTPEZY");
    quint64 diskBytesSize = 0;
    double multiplier = 1;
    double diskSizeFloat = 0;

    QRegularExpression reg("([0-9]*\\.?[0-9]*)([A-Za-z]*)");
    QRegularExpressionMatch match = reg.match(size);
    if(!match.hasMatch()) {
        qCDebug(appLog) << "convertToBytes, no match";
        return diskBytesSize;
    }
    QString sizeValue1 = match.captured(1);
    if(sizeValue1.isEmpty()) {
        qCDebug(appLog) << "convertToBytes, sizeValue1 is empty";
        return diskBytesSize;
    }
    QString diskUnits = size.right(size.length() - sizeValue1.size()).trimmed().toUpper();
    if(diskUnits.isEmpty()) {
        qCDebug(appLog) << "convertToBytes, diskUnits is empty";
        return diskBytesSize;
    }
    diskUnits.replace("B","").replace("I","");
    QChar lastChar = diskUnits.at(diskUnits.length() - 1);

    if (prefixes.contains(diskUnits)) {
        qCDebug(appLog) << "convertToBytes, prefixes contains diskUnits";
        diskSizeFloat = sizeValue1.toDouble();

        int i = 0;
        while (i < prefixes.size()) {
            QChar iChar = prefixes.at(i++);
            if(iChar == lastChar) {
                qCDebug(appLog) << "convertToBytes, iChar == lastChar";
                multiplier = std::pow(scale,i);
                break;
            }
        }
    }
    diskBytesSize = static_cast<quint64>(diskSizeFloat * multiplier);
    return diskBytesSize;
}

void DeviceStorage::unitConvertByDecimal()
{
    qCDebug(appLog) << "DeviceStorage::unitConvertByDecimal";
    if(m_SizeBytes > 0)
        m_Size = decimalkilos(m_SizeBytes);

    quint64 gbyte =  1000000000;
    if (m_Interface.contains("UFS", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "DeviceStorage::unitConvertByDecimal, interface contains UFS";
        if(m_SizeBytes > 255*gbyte && m_SizeBytes < 257*gbyte) {
            m_Size = "256 GB";
        } else if(m_SizeBytes > 511*gbyte && m_SizeBytes < 513*gbyte) {
                m_Size = "512 GB";
        } else if(m_SizeBytes > 999*gbyte && m_SizeBytes < 1001*gbyte) {
            m_Size = "1 TB";
        }
    }
}

bool DeviceStorage::setHwinfoInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Set hwinfo info for storage device";
    // 龙芯机器中 hwinfo --disk会列出所有的分区信息
    // 存储设备不应包含分区，根据SysFS BusID 来确定是否是分区信息
    if (mapInfo.find("SysFS BusID") == mapInfo.end()) {
        qCDebug(appLog) << "Invalid hwinfo info - missing SysFS BusID";
        return false;
    }

    setAttribute(mapInfo, "Model", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);

    // 希捷硬盘为ATA硬盘，无法直接获取厂商信息,只能特殊处理
    if (m_Name.startsWith("ST") && m_Vendor.isEmpty()) {
        qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, vendor is Seagate";
        m_Vendor = "ST";
    }
    //根据产品PN中的固定前两位 RS来匹配厂商 为Longsys 如产品PN ：RSYE3836N-480G    RSYE3836N-960G    RSYE3836N-1920     RSYE3836N-3840
    if (m_Name.startsWith("RS") && m_Vendor.isEmpty()) {
        qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, vendor is Longsys";
        m_Vendor = "Longsys";
    }

    setAttribute(mapInfo, "Driver", m_Driver); // 驱动
    QRegularExpression exp("pci 0x[0-9a-zA-Z]*");
    QRegularExpressionMatch match = exp.match(m_Vendor);
    if (match.hasMatch()) {
        qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, vendor is pci";
        m_Vendor = "";
    }

    setAttribute(mapInfo, "Attached to", m_Interface);
    QRegularExpression re(".*\\((.*)\\).*");
    match = re.match(m_Interface);
    if (match.hasMatch()) {
        qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, interface has match";
        m_Interface = match.captured(1);
        m_Interface.replace("Controller", "");
        m_Interface.replace("controller", "");
    }

    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Capacity", m_Size);

    // hwinfo里面显示的内容是  14 GB (15376000000 bytes) 需要处理
    QRegularExpression reSize(".*\\((.*)bytes\\).*");
    match = reSize.match(m_Size);
    if (match.hasMatch()) {
        qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, size has match";
        bool ok = false;
        quint64 bytesSize = match.captured(1).trimmed().toULongLong(&ok);
        if (ok) {
            qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, size convert to ullong success";
            m_Size = decimalkilos(bytesSize);
            m_SizeBytes = bytesSize;
        } else {
            qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, size convert to ullong failed";
            m_Size.replace(QRegularExpression("\\(.*\\)"), "").replace(" ", "");
            m_SizeBytes = convertToBytes(m_Size,DISK_SCALE_1024);
        }
    } else {
        qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, size has no match";
        m_Size.replace(QRegularExpression("\\(.*\\)"), "").replace(" ", "");
        m_SizeBytes = convertToBytes(m_Size,DISK_SCALE_1024);
    }

    // 如果既没有capacity也没有序列号则认为该磁盘无效,否则都属于有效磁盘
    if ((m_Size.startsWith("0") || m_Size == "") && m_SerialNumber == "") {
        qCDebug(appLog) << "Invalid storage device - empty size and serial number";
        return false;
    }

    // get serial num
    m_SerialNumber = "";
    QString strLink = mapInfo["SysFS Device Link"];
    m_SerialNumber = getSerialID(strLink);
    if (m_SerialNumber.isEmpty()) {
        qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, serial number is empty";
        setAttribute(mapInfo, "Serial ID", m_SerialNumber);
    }
    if (m_SerialNumber.compare("0",Qt::CaseInsensitive) == 0) {
        qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, serial number is 0";
        m_SerialNumber = "";
    }

    setAttribute(mapInfo, "SysFS BusID", m_KeyToLshw);
    setAttribute(mapInfo, "Device File", m_DeviceFile);

    // 专有文件
    QString logicalName = ((QString)mapInfo["SysFS ID"]).replace("/class/block", "");

    QDir blockDir("/sys/block/");
    QStringList blockfs = blockDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);
    foreach (QString fsname, blockfs) {
        if (m_DeviceFile.contains(fsname, Qt::CaseInsensitive)) {
            qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, device file contains fsname";
            logicalName = fsname;
            break;
        }
    }

    QString Path = "/sys/block/" + logicalName + "/device/spec_version";
    QFile file(Path);
    if (file.open(QIODevice::ReadOnly)) {
        qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, open spec_version success";
        QString output2 = file.readAll();
        if (output2.contains("310", Qt::CaseInsensitive)) {
            m_Interface = "UFS 3.1";
        } else if (output2.contains("300", Qt::CaseInsensitive)) {
            m_Interface = "UFS 3.0";
        }
        file.close();
    }

    if (m_KeyToLshw.contains("nvme", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "DeviceStorage::setHwinfoInfo, key contains nvme";
        setAttribute(mapInfo, "SysFS Device Link", m_NvmeKey);
    }

    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);
    m_PhysID = m_VID_PID;

    getOtherMapInfo(mapInfo);
    return true;
}

QString DeviceStorage::getSerialID(QString &strDeviceLink)
{
    qCDebug(appLog) << "DeviceStorage::getSerialID";
    //取SerialID优先级：1.取cid 2.取unique_number 3.取hwinfo中的SerialID
    QString strSerialNumber = "";
    if (!strDeviceLink.isEmpty() && strDeviceLink.contains("platform/")) {
        qCDebug(appLog) << "DeviceStorage::getSerialID, device link contains platform";
        // /devices/platform/f8300000.ufs/host0/target0:0:0/0:0:0:3
        // /proc/bootdevice/name:f8300000.ufs
        QRegularExpression reg(".*platform/([^/]+)/.*");
        QString strName = "";
        QString strBootdeviceName = "";
        if (reg.match(strDeviceLink).hasMatch()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            strName = reg.cap(1); // Qt 5
#else
            strName = reg.match(strDeviceLink).captured(1); // Qt 6
#endif
        }
        if (!strName.isEmpty()) { //取到设备名称再去读文件，因为读文件开销大。
            qCDebug(appLog) << "DeviceStorage::getSerialID, strName is not empty";
            QString Path = "/proc/bootdevice/name";
            QFile file(Path);
            if (file.open(QIODevice::ReadOnly)) {
                qCDebug(appLog) << "DeviceStorage::getSerialID, open /proc/bootdevice/name success";
                strBootdeviceName = file.readAll();
                file.close();
            }
            if (strName == strBootdeviceName) {
                qCDebug(appLog) << "DeviceStorage::getSerialID, strName == strBootdeviceName";
                Path = "/proc/bootdevice/cid";
                QFile filecid(Path);
                if (filecid.open(QIODevice::ReadOnly)) {
                    qCDebug(appLog) << "DeviceStorage::getSerialID, open /proc/bootdevice/cid success";
                    strSerialNumber = filecid.readAll().trimmed();
                    filecid.close();
                }
            }
        }
    }

    if (strSerialNumber.isEmpty()) {
        qCDebug(appLog) << "DeviceStorage::getSerialID, serial number is empty";
        QString Path = "/sys" + strDeviceLink + "/unique_number";
        QFile file(Path);
        if (file.open(QIODevice::ReadOnly)) {
            qCDebug(appLog) << "DeviceStorage::getSerialID, open unique_number success";
            QString value = file.readAll();
            strSerialNumber = value;
            file.close();
        }
    }
    return strSerialNumber;
}

bool DeviceStorage::setKLUHwinfoInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceStorage::setKLUHwinfoInfo";
    // 龙芯机器中 hwinfo --disk会列出所有的分区信息
    // 存储设备不应包含分区，根据SysFS BusID 来确定是否是分区信息
    if (mapInfo.find("SysFS BusID") == mapInfo.end())
        return false;

    setAttribute(mapInfo, "Model", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Driver", m_Driver); // 驱动

    setAttribute(mapInfo, "Attached to", m_Interface);
    QRegularExpression re(".*\\((.*)\\).*");
    QRegularExpressionMatch match = re.match(m_Interface);
    if (match.hasMatch()) {
        qCDebug(appLog) << "DeviceStorage::setKLUHwinfoInfo, interface has match";
        m_Interface = match.captured(1);
        m_Interface.replace("Controller", "");
        m_Interface.replace("controller", "");
    }

    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Capacity", m_Size);

    // hwinfo里面显示的内容是  14 GB (15376000000 bytes) 需要处理
    QRegularExpression reSize(".*\\((.*)bytes\\).*");
    match = reSize.match(m_Size);
    if (match.hasMatch()) {
        qCDebug(appLog) << "DeviceStorage::setKLUHwinfoInfo, size has match";
        bool ok = false;
        quint64 bytesSize = match.captured(1).trimmed().toULongLong(&ok);
        if (ok) {
            qCDebug(appLog) << "DeviceStorage::setKLUHwinfoInfo, size convert to ullong success";
            m_Size = decimalkilos(bytesSize);
        } else {
            qCDebug(appLog) << "DeviceStorage::setKLUHwinfoInfo, size convert to ullong failed";
            m_Size.replace(QRegularExpression("\\(.*\\)"), "").replace(" ", "");
        }
    } else {
        qCDebug(appLog) << "DeviceStorage::setKLUHwinfoInfo, size has no match";
        m_Size.replace(QRegularExpression("\\(.*\\)"), "").replace(" ", "");
    }
    if (m_Size.startsWith("0") || m_Size == "")
        return false;

    setAttribute(mapInfo, "Serial ID", m_SerialNumber);
    // setDiskSerialID(mapInfo["Device Files"]);
    setAttribute(mapInfo, "SysFS BusID", m_KeyToLshw);
    setAttribute(mapInfo, "Device File", m_DeviceFile);

    // KLU里面的介质类型的处理方式比较特殊
    if (mapInfo["Driver"].contains("usb-storage")) {
        qCDebug(appLog) << "DeviceStorage::setKLUHwinfoInfo, driver contains usb-storage";
        m_MediaType = "USB";
    }

    getOtherMapInfo(mapInfo);
    return true;
}

bool DeviceStorage::addInfoFromlshw(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Add lshw info for storage device";
    // 先获取需要进行匹配的关键字
    QStringList keys = mapInfo["bus info"].split("@");
    if (keys.size() != 2) {
        qCDebug(appLog) << "Invalid lshw info - malformed bus info";
        return false;
    }

    QString key = keys[1].trimmed();
    key.replace(".", ":");

    if (key != m_KeyToLshw) {
        qCDebug(appLog) << "Key mismatch - expected:" << m_KeyToLshw << "got:" << key;
        return false;
    }

    // 获取唯一key
    QStringList words = mapInfo["bus info"].split(":");
    if (words.size() == 2) {
        qCDebug(appLog) << "DeviceStorage::addInfoFromlshw, words size is 2";
        m_KeyFromStorage = words[0];
        m_KeyFromStorage.replace("@", "");
    }

    // 更新接口
    setAttribute(mapInfo, "interface", m_Interface, false);

    // 获取基本信息
    getInfoFromLshw(mapInfo);

    // 获取其它设备信息
    getOtherMapInfo(mapInfo);

    return true;
}

bool DeviceStorage::addNVMEInfoFromlshw(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceStorage::addNVMEInfoFromlshw";
    QStringList keys = mapInfo["bus info"].split("@");
    if (keys.size() != 2)
        return false;

    QString key = keys[1].trimmed();

    // SysFS Device Link: /devices/pci0000:00/0000:00:05.0/0000:0d:00.0/nvme/nvme0
    //bus info: pci@0000:0d:00.0
    // 确认为同一设备
    if (m_NvmeKey.contains(key, Qt::CaseInsensitive)) {
        qCDebug(appLog) << "DeviceStorage::addNVMEInfoFromlshw, key contains nvme";
        setAttribute(mapInfo, "vendor", m_Vendor);
    } else {
        qCDebug(appLog) << "DeviceStorage::addNVMEInfoFromlshw, key not contains nvme";
        return false;
    }

    return true;
}

bool DeviceStorage::addInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceStorage::addInfoFromSmartctl";
    // 查看传入的设备信息与当前的设备信息是不是同一个设备信息
    if (!m_DeviceFile.contains(name, Qt::CaseInsensitive))
        return false;

    // 获取基本信息
    getInfoFromsmartctl(mapInfo);
    return true;
}

bool DeviceStorage::setMediaType(const QString &name, const QString &value)
{
    qCDebug(appLog) << "DeviceStorage::setMediaType";
    if (!m_DeviceFile.contains(name)) {
        qCDebug(appLog) << "DeviceStorage::setMediaType, device file does not contain name";
        return false;
    }

    if (QString("0") == value) {
        qCDebug(appLog) << "DeviceStorage::setMediaType, media type is SSD";
        m_MediaType = QObject::tr("SSD");
    } else if (QString("1") == value) {
        qCDebug(appLog) << "DeviceStorage::setMediaType, media type is HDD";
        m_MediaType = QObject::tr("HDD");
    } else {
        qCDebug(appLog) << "DeviceStorage::setMediaType, media type is Unknown";
        m_MediaType = QObject::tr("Unknown");
    }

    return true;
}

bool DeviceStorage::setKLUMediaType(const QString &name, const QString &value)
{
    qCDebug(appLog) << "DeviceStorage::setKLUMediaType";
    if (!m_DeviceFile.contains(name)) {
        qCDebug(appLog) << "DeviceStorage::setKLUMediaType, device file does not contain name";
        return false;
    }

    if (m_MediaType == "USB") {
        qCDebug(appLog) << "DeviceStorage::setKLUMediaType, media type is USB";
        return true;
    }

    if (QString("0") == value) {
        qCDebug(appLog) << "DeviceStorage::setKLUMediaType, media type is SSD";
        m_MediaType = QObject::tr("SSD");
    } else if (QString("1") == value) {
        qCDebug(appLog) << "DeviceStorage::setKLUMediaType, media type is HDD";
        m_MediaType = QObject::tr("HDD");
    } else {
        qCDebug(appLog) << "DeviceStorage::setKLUMediaType, media type is Unknown";
        m_MediaType = QObject::tr("Unknown");
    }

    return true;
}

bool DeviceStorage::isValid()
{
    qCDebug(appLog) << "DeviceStorage::isValid";
    // 若是m_Size为空则 该设备无效
    if (m_Size.isEmpty() && m_SizeBytes == 0) {
        qCDebug(appLog) << "Storage device invalid - empty size";
        return false;
    }

    return true;
}

void DeviceStorage::setDiskSerialID(const QString &deviceFiles)
{
    qCDebug(appLog) << "DeviceStorage::setDiskSerialID";
    // Serial ID 与 device Files 中信息一致
    if (!m_SerialNumber.isEmpty() && deviceFiles.contains(m_SerialNumber))
        return;

    QStringList itemList = deviceFiles.split(",");

    QString modelName = m_Name;
    modelName.replace(" ", "_");

    foreach (auto item, itemList) {
        if (item.contains("by-id", Qt::CaseInsensitive) &&
                item.contains(modelName, Qt::CaseInsensitive)) {
            qCDebug(appLog) << "DeviceStorage::setDiskSerialID, item contains by-id and modelName";

            int index;
            index = item.lastIndexOf("_");
            item = item.mid(index + 1);
            item.replace(QRegularExpression("-[\\s\\S]*$"), "");
            m_SerialNumber = item;
            break;
        }
    }
}

QString DeviceStorage::getDiskSerialID()
{
    qCDebug(appLog) << "DeviceStorage::getDiskSerialID";
    if (m_Interface.contains("USB", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "DeviceStorage::getDiskSerialID, interface contains USB";
        return m_SerialNumber + m_KeyToLshw;
    }
    return m_SerialNumber;
}

void DeviceStorage::appendDisk(DeviceStorage *device)
{
    qCDebug(appLog) << "DeviceStorage::appendDisk";
    QList<QPair<QString, QString> > allAttribs = device->getBaseAttribs();
    QMap<QString, QString> allAttribMaps;
    for (int i = 0; i < allAttribs.size(); ++i) {
        allAttribMaps.insert(allAttribs[i].first, allAttribs[i].second);
    }

    quint64 size2 = device->getDiskSizeByte();
    if(m_SizeBytes == 0) {
        qCDebug(appLog) << "DeviceStorage::appendDisk, m_SizeBytes is 0";
        m_SizeBytes = size2;
    } else if (size2 > 0) {
        qCDebug(appLog) << "DeviceStorage::appendDisk, size2 > 0";
        m_SizeBytes += size2;

        QList<QPair<QString, QString> > allOtherAttribs = device->getOtherAttribs();
        QMap<QString, QString> allOtherAttribMaps;
        for (int i = 0; i < allOtherAttribs.size(); ++i) {
            allAttribMaps.insert(allOtherAttribs[i].first, allOtherAttribs[i].second);
        }

        loadOtherDeviceInfo();
        //合并
        QMap<QString, QString> curAllOtherAttribMaps;
        for (int i = 0; i < m_LstOtherInfo.size(); ++i) {
            curAllOtherAttribMaps.insert(m_LstOtherInfo[i].first, m_LstOtherInfo[i].second);
        }

        QStringList keyList;
        keyList.append(QObject::tr("bus info"));
        keyList.append(QObject::tr("Device File"));
        // keyList.append(QObject::tr("physical id"));
        keyList.append(QObject::tr("Device Number"));
        keyList.append(QObject::tr("logical name"));
        for (QString keyStr : keyList) {
            QString curBusInfo = curAllOtherAttribMaps[keyStr];
            QString busInfo = allAttribMaps[keyStr];
            if (!curBusInfo.isEmpty() && !busInfo.isEmpty() && curBusInfo != busInfo) {
                // qCDebug(appLog) << "DeviceStorage::appendDisk, bus info not empty and not equal";
                setOtherDeviceInfo(keyStr, curBusInfo + "," + busInfo);
            }
        }
        loadOtherDeviceInfo();
    }
}

void DeviceStorage::checkDiskSize()
{
    qCDebug(appLog) << "DeviceStorage::checkDiskSize";
    QRegularExpression reg("[0-9]*.?[0-9]*");
    int index = reg.match(m_Size).capturedStart();
    // index>0时，对于"32GB"（数字开头的字符串,index=0）无法获取正确的数据32
    // 所以要改为index >= 0
    if (index >= 0) {
        qCDebug(appLog) << "DeviceStorage::checkDiskSize, index >= 0";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        double num = reg.cap(0).toDouble(); // Qt 5 使用 cap
#else
        double num = reg.match(m_Size).captured(0).toDouble(); // Qt 6 使用 captured
#endif
        double num1 = num - int(num);
        QString type = m_Size.right(m_Size.length() - reg.match(m_Size).captured(0).size()).trimmed();
        if (!qFuzzyCompare(num1, 0.0) && type == "GB") {
            qCDebug(appLog) << "DeviceStorage::checkDiskSize, num1 is not 0 and type is GB";
            m_Size = QString::number(int(num) + 1) + " " + type;
        }
    }
}

QString DeviceStorage::compareSize(const QString &size1, const QString &size2)
{
    qCDebug(appLog) << "DeviceStorage::compareSize";
    // 比较smartctl中可能提供的两个大小，取大值作为存储设备的大小
    if (size1.isEmpty() || size2.isEmpty())
        return size1 + size2;

    // 将字符串转为数字大小进行比较
    int num1 = 0;
    int num2 = 0;
    QRegularExpression reg(".*\\[(\\d*).*\\]$");
    if (reg.match(size1).hasMatch())
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        num1 = reg.cap(1).toInt(); // Qt 5 使用 cap
#else
        num1 = reg.match(size1).captured(1).toInt(); // Qt 6 使用 captured
#endif
    if (reg.match(size2).hasMatch())
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        num2 = reg.cap(1).toInt(); // Qt 5 使用 cap
#else
        num2 = reg.match(size2).captured(1).toInt(); // Qt 6 使用 captured
#endif

    // 返回较大值
    if (num1 > num2) {
        qCDebug(appLog) << "DeviceStorage::compareSize, num1 > num2";
        return size1;
    } else {
        qCDebug(appLog) << "DeviceStorage::compareSize, num1 <= num2";
        return size2;
    }
}

const QString &DeviceStorage::name() const
{
    // qCDebug(appLog) << "DeviceStorage::name";
    return m_Name;
}

const QString &DeviceStorage::vendor() const
{
    // qCDebug(appLog) << "DeviceStorage::vendor";
    return m_Vendor;
}

const QString &DeviceStorage::driver() const
{
    // qCDebug(appLog) << "DeviceStorage::driver";
    return m_Driver;
}

const QString &DeviceStorage::keyFromStorage()const
{
    // qCDebug(appLog) << "DeviceStorage::keyFromStorage";
    return m_KeyFromStorage;
}

QString DeviceStorage::subTitle()
{
    // qCDebug(appLog) << "DeviceStorage::subTitle";
    return m_Name;
}

const QString DeviceStorage::getOverviewInfo()
{
    // qCDebug(appLog) << "DeviceStorage::getOverviewInfo";
    return QString("%1 (%2)").arg(m_Name).arg(m_Size);;
}

void DeviceStorage::initFilterKey()
{
    qCDebug(appLog) << "DeviceStorage::initFilterKey";
    // hwinfo --disk
    addFilterKey(QObject::tr("Hardware Class"));
    addFilterKey(QObject::tr("Device File"));
    addFilterKey(QObject::tr("ansiversion"));
    addFilterKey(QObject::tr("bus info"));
    addFilterKey(QObject::tr("logical name"));
    addFilterKey(QObject::tr("logicalsectorsize"));
    // addFilterKey(QObject::tr("physical id"));
    addFilterKey(QObject::tr("sectorsize"));
    addFilterKey(QObject::tr("guid"));
    addFilterKey(QObject::tr("Config Status"));
    addFilterKey(QObject::tr("Device Number"));
    addFilterKey(QObject::tr("Geometry (Logical)"));

}

void DeviceStorage::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "DeviceStorage::loadBaseDeviceInfo";
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Media Type"), m_MediaType);
    addBaseDeviceInfo(tr("Size"), m_Size);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Capabilities"), m_Capabilities);
}

void DeviceStorage::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "DeviceStorage::loadOtherDeviceInfo";
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Firmware Version"), m_FirmwareVersion);
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Description"), m_Description);

    for(int i = 0 ; i < m_SerialNumber.count(); i++) {
        QChar cha = m_SerialNumber.at(i);
        ushort uni = cha.unicode();
        if(uni < 33 || uni > 126) {
            qCWarning(appLog)<<"smartctl Serial number is not LetterOrNumber "<< m_SerialNumber;
            m_SerialNumber.clear();
            break;
        }
    }

    addOtherDeviceInfo(tr("Serial Number"), m_SerialNumber);
    addOtherDeviceInfo(tr("Interface"), m_Interface);
    addOtherDeviceInfo(tr("Rotation Rate"), m_RotationRate);
    addOtherDeviceInfo(tr("Module Alias"), m_Modalias);
    addOtherDeviceInfo(tr("Physical ID"), m_PhysID);

    if (m_RotationRate == QString("Solid State Device")) {
        qCDebug(appLog) << "DeviceStorage::loadOtherDeviceInfo, rotation rate is Solid State Device";
        m_MediaType = QObject::tr("SSD");
    }

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceStorage::loadTableHeader()
{
    qCDebug(appLog) << "DeviceStorage::loadTableHeader";
    // 加载表头信息
    m_TableHeader.append(tr("Name"));
    m_TableHeader.append(tr("Vendor"));
    m_TableHeader.append(tr("Media Type"));
    m_TableHeader.append(tr("Size"));
}

void DeviceStorage::loadTableData()
{
    qCDebug(appLog) << "DeviceStorage::loadTableData";
    // 加载表格数据
    QString model = m_Name;
    if (!available()) {
        qCDebug(appLog) << "DeviceStorage::loadTableData, not available";
        model = "(" + tr("Unavailable") + ") " + m_Name;
    }
    m_TableData.append(model);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_MediaType);
    m_TableData.append(m_Size);
}

void DeviceStorage::getInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceStorage::getInfoFromLshw";
    // lshw信息获取
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "serial", m_SerialNumber, false);
    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "description", m_Description);
    QString sizeFromlshw = QString();
    quint64 sizeByte = 0;
    setAttribute(mapInfo, "size", sizeFromlshw, true);
    // 样式数据 223GiB (240GB) size: 57GiB (61GB)  size: 931GiB (1TB)
    QRegularExpression re(".*\\((.*)\\)$");
    QRegularExpressionMatch match = re.match(sizeFromlshw);
    if (match.hasMatch()) {
        qCDebug(appLog) << "DeviceStorage::getInfoFromLshw, size has match";
        sizeFromlshw = match.captured(1);
        sizeByte = convertToBytes(sizeFromlshw, DISK_SCALE_1000);
    }
    if (m_SizeBytes == 0)
        m_SizeBytes = sizeByte;

    if (m_Size.isEmpty())
        m_Size = sizeFromlshw;

    if (m_SerialNumber.compare("0", Qt::CaseInsensitive) == 0)
        m_SerialNumber = "";
}

void DeviceStorage::getInfoFromsmartctl(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl";
    if (mapInfo.size() < 5) {
        if (!mapInfo.isEmpty() && m_Interface.contains("USB", Qt::CaseInsensitive)) {
            qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, mapInfo is not empty and interface contains USB";
            m_MediaType = QObject::tr("SSD");
        }
        return;
    }
    // 固件版本
    m_FirmwareVersion = mapInfo["Firmware Version"];

    // 速度
    QString sataVersion = mapInfo["SATA Version is"];
    QStringList strList = sataVersion.split(",");
    if (strList.size() == 2) {
        qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, strList size is 2";
        m_Speed = strList[1];
    }

    setAttribute(mapInfo, "Rotation Rate", m_RotationRate);
    // 解决Bug45428,INTEL SSDSA2BW160G3L 这个型号的硬盘通过lsblk获取的rota是１，所以这里需要特殊处理
    if (m_RotationRate == QString("Solid State Device")) {
        qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, rotation rate is Solid State Device";
        m_MediaType = QObject::tr("SSD");
    }

    // 按照HW的需求，如果是固态硬盘就不显示转速
    if (m_RotationRate == QString("HW_SSD")) {
        qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, rotation rate is HW_SSD";
        m_MediaType = QObject::tr("SSD");
        m_RotationRate = "";
    }

    // 安装大小
    QString capacity = mapInfo["User Capacity"];
    if (capacity == "")
        capacity = compareSize(mapInfo["Total NVM Capacity"], mapInfo["Namespace 1 Size/Capacity"]);

    if (capacity != "") {
        qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, capacity is not empty";
        QRegularExpression reg(".*\\[(.*)\\]$");
        if (reg.match(capacity).hasMatch())
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            m_Size = reg.cap(1); // Qt 5 使用 cap
#else
            m_Size = reg.match(capacity).captured(1); // Qt 6 使用 captured
#endif

        capacity.replace(",","").replace(" ","");
        QRegularExpression re("(\\d+)bytes*");     //取值格式如： User Capacity:    1,000,204,886,016 bytes [1.00 TB]     Total NVM Capacity:   256,060,514,304 [256 GB]
        int pos = re.match(capacity).capturedStart();
        if (pos != -1) {
            qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, capacity has match";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QString byteSize = re.cap(1); // Qt 5 使用 cap
#else
            QString byteSize = re.match(capacity).captured(1); // Qt 6 使用 captured
#endif
            bool isValue = false;
            quint64 value = byteSize.trimmed().toULongLong(&isValue);
            if (value > 0 && isValue)
                m_SizeBytes = value;
        }
    }

    // 通过不断适配，当厂商有在固件中提供时，硬盘型号从smartctl中获取更加合理
    // 因为hwinfo获取的是主控的型号，而硬盘厂商由于还不能自己生产主控，只能采购别人的主控
    //SATA
    if (mapInfo.find("Device Model") != mapInfo.end()) {
        qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, mapInfo contains Device Model";
        m_Name = mapInfo["Device Model"];
    }
    //NVME
    if (mapInfo.find("Model Number") != mapInfo.end()) {
        qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, mapInfo contains Model Number";
        m_Name = mapInfo["Model Number"];
    }

    if (mapInfo.find("Serial Number") != mapInfo.end()) {
        qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, mapInfo contains Serial Number";
        setAttribute(mapInfo, "Serial Number", m_SerialNumber, true);
    } else if (mapInfo.find("Serial number") != mapInfo.end()) {
        qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, mapInfo contains Serial number";
        setAttribute(mapInfo, "Serial number", m_SerialNumber, true);
    }

    // 修正数值
    if(Common::boardVendorType() != "KLVV" && Common::boardVendorType() != "KLVU" \
        && Common::boardVendorType() != "PGUW" && Common::boardVendorType() != "PGUV")
            m_Size.replace(QRegularExpression("\\.0[1-9]"), ".00");
    //根据产品PN中的固定前两位 RS来匹配厂商 为Longsys 如产品PN ：RSYE3836N-480G    RSYE3836N-960G    RSYE3836N-1920     RSYE3836N-3840
    if (m_Name.startsWith("RS") && m_Vendor.isEmpty()) {
        qCDebug(appLog) << "DeviceStorage::getInfoFromsmartctl, name starts with RS and vendor is empty";
        m_Vendor = "Longsys";
    }
}
