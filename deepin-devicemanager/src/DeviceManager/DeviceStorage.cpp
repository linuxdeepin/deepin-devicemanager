// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceStorage.h"
#include "commonfunction.h"
#include "DDLog.h"
// Qt库文件
#include<QLoggingCategory>

using namespace DDLog;

DeviceStorage::DeviceStorage()
    : DeviceBaseInfo()
    , m_Model("")
//    , m_Vendor("")
    , m_MediaType("")
    , m_Size("")
    , m_RotationRate("")
    , m_Interface("")
    , m_SerialNumber("")
    , m_Capabilities("")
    , m_KeyToLshw("")
    , m_KeyFromStorage("")
{
    // 初始化可显示属性
    initFilterKey();
}

TomlFixMethod DeviceStorage::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
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

bool DeviceStorage::setHwinfoInfo(const QMap<QString, QString> &mapInfo)
{
    // 龙芯机器中 hwinfo --disk会列出所有的分区信息
    // 存储设备不应包含分区，根据SysFS BusID 来确定是否是分区信息
    if (mapInfo.find("SysFS BusID") == mapInfo.end())
        return false;

    setAttribute(mapInfo, "Model", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);

    // 希捷硬盘为ATA硬盘，无法直接获取厂商信息,只能特殊处理
    if (m_Name.startsWith("ST") && m_Vendor.isEmpty())
        m_Vendor = "ST";

    setAttribute(mapInfo, "Driver", m_Driver); // 驱动
    QRegExp exp("pci 0x[0-9a-zA-Z]*");
    if (exp.indexIn(m_Vendor) != -1)
        m_Vendor = "";

    setAttribute(mapInfo, "Attached to", m_Interface);
    QRegExp re(".*\\((.*)\\).*");
    if (re.exactMatch(m_Interface)) {
        m_Interface = re.cap(1);
        m_Interface.replace("Controller", "");
        m_Interface.replace("controller", "");
    }

    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Capacity", m_Size);

    // hwinfo里面显示的内容是  14 GB (15376000000 bytes) 需要处理
    m_Size.replace(QRegExp("\\(.*\\)"), "").replace(" ", "");

    // 如果既没有capacity也没有序列号则认为该磁盘无效,否则都属于有效磁盘
    if ((m_Size.startsWith("0") || m_Size == "") && m_SerialNumber == "")
        return false;

    // get serial num
    m_SerialNumber = "";
    QString strLink = mapInfo["SysFS Device Link"];
    m_SerialNumber = getSerialID(strLink);
    if (m_SerialNumber.isEmpty()) {
        setAttribute(mapInfo, "Serial ID", m_SerialNumber);
    }
    if (m_SerialNumber.compare("0",Qt::CaseInsensitive) == 0)
        m_SerialNumber = "";

    setAttribute(mapInfo, "SysFS BusID", m_KeyToLshw);
    setAttribute(mapInfo, "Device File", m_DeviceFile);
    if (m_KeyToLshw.contains("nvme", Qt::CaseInsensitive))
        setAttribute(mapInfo, "SysFS Device Link", m_NvmeKey);

    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);
    m_PhysID = m_VID_PID;

    getOtherMapInfo(mapInfo);
    return true;
}

QString DeviceStorage::getSerialID(QString &strDeviceLink)
{
    //取SerialID优先级：1.取cid 2.取unique_number 3.取hwinfo中的SerialID
    QString strSerialNumber = "";
    if (!strDeviceLink.isEmpty() && strDeviceLink.contains("platform/")) {
        // /devices/platform/f8300000.ufs/host0/target0:0:0/0:0:0:3
        // /proc/bootdevice/name:f8300000.ufs
        QRegExp reg(".*platform/([^/]+)/.*");
        QString strName = "";
        QString strBootdeviceName = "";
        if (reg.exactMatch(strDeviceLink)) {
            strName = reg.cap(1);
        }
        if (!strName.isEmpty()) { //取到设备名称再去读文件，因为读文件开销大。
            QString Path = "/proc/bootdevice/name";
            QFile file(Path);
            if (file.open(QIODevice::ReadOnly)) {
                strBootdeviceName = file.readAll();
                file.close();
            }
            if (strName == strBootdeviceName) {
                Path = "/proc/bootdevice/cid";
                QFile filecid(Path);
                if (filecid.open(QIODevice::ReadOnly)) {
                    strSerialNumber = filecid.readAll().trimmed();
                    filecid.close();
                }
            }
        }
    }

    if (strSerialNumber.isEmpty()) {
        QString Path = "/sys" + strDeviceLink + "/unique_number";
        QFile file(Path);
        if (file.open(QIODevice::ReadOnly)) {
            QString value = file.readAll();
            strSerialNumber = value;
            file.close();
        }
    }
    return strSerialNumber;
}

bool DeviceStorage::setKLUHwinfoInfo(const QMap<QString, QString> &mapInfo)
{
    // 龙芯机器中 hwinfo --disk会列出所有的分区信息
    // 存储设备不应包含分区，根据SysFS BusID 来确定是否是分区信息
    if (mapInfo.find("SysFS BusID") == mapInfo.end())
        return false;

    setAttribute(mapInfo, "Model", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Driver", m_Driver); // 驱动

    setAttribute(mapInfo, "Attached to", m_Interface);
    QRegExp re(".*\\((.*)\\).*");
    if (re.exactMatch(m_Interface)) {
        m_Interface = re.cap(1);
        m_Interface.replace("Controller", "");
        m_Interface.replace("controller", "");
    }

    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Capacity", m_Size);

    // hwinfo里面显示的内容是  14 GB (15376000000 bytes) 需要处理
    m_Size.replace(QRegExp("\\(.*\\)"), "").replace(" ", "");
    if (m_Size.startsWith("0") || m_Size == "")
        return false;

    setAttribute(mapInfo, "Serial ID", m_SerialNumber);
//    setDiskSerialID(mapInfo["Device Files"]);
    setAttribute(mapInfo, "SysFS BusID", m_KeyToLshw);
    setAttribute(mapInfo, "Device File", m_DeviceFile);

    // KLU里面的介质类型的处理方式比较特殊
    if (mapInfo["Driver"].contains("usb-storage"))
        m_MediaType = "USB";

    getOtherMapInfo(mapInfo);
    return true;
}

bool DeviceStorage::addInfoFromlshw(const QMap<QString, QString> &mapInfo)
{

    // 先获取需要进行匹配的关键字
    QStringList keys = mapInfo["bus info"].split("@");
    if (keys.size() != 2)
        return false;

    QString key = keys[1].trimmed();
    key.replace(".", ":");

    if (key != m_KeyToLshw)
        return false;

    // 获取唯一key
    QStringList words = mapInfo["bus info"].split(":");
    if (words.size() == 2) {
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
    QStringList keys = mapInfo["bus info"].split("@");
    if (keys.size() != 2)
        return false;

    QString key = keys[1].trimmed();

    // SysFS Device Link: /devices/pci0000:00/0000:00:05.0/0000:0d:00.0/nvme/nvme0
    //bus info: pci@0000:0d:00.0
    // 确认为同一设备
    if (m_NvmeKey.contains(key, Qt::CaseInsensitive)) {
        setAttribute(mapInfo, "vendor", m_Vendor);
    } else {
        return false;
    }

    return true;
}

bool DeviceStorage::addInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo)
{
    // 查看传入的设备信息与当前的设备信息是不是同一个设备信息
    if (!m_DeviceFile.contains(name, Qt::CaseInsensitive))
        return false;

    // 获取基本信息
    getInfoFromsmartctl(mapInfo);
    return true;
}

bool DeviceStorage::setMediaType(const QString &name, const QString &value)
{
    if (!m_DeviceFile.contains(name))
        return false;

    if (QString("0") == value)
        m_MediaType = QObject::tr("SSD");
    else if (QString("1") == value)
        m_MediaType = QObject::tr("HDD");
    else
        m_MediaType = QObject::tr("Unknown");

    return true;
}

bool DeviceStorage::setKLUMediaType(const QString &name, const QString &value)
{
    if (!m_DeviceFile.contains(name))
        return false;

    if (m_MediaType == "USB")
        return true;

    if (QString("0") == value)
        m_MediaType = QObject::tr("SSD");
    else if (QString("1") == value)
        m_MediaType = QObject::tr("HDD");
    else
        m_MediaType = QObject::tr("Unknown");

    return true;
}

bool DeviceStorage::isValid()
{
    // 若是m_Size为空则 该设备无效
    if (m_Size.isEmpty())
        return false;

    return true;
}

void DeviceStorage::setDiskSerialID(const QString &deviceFiles)
{
    // Serial ID 与 device Files 中信息一致
    if (!m_SerialNumber.isEmpty() && deviceFiles.contains(m_SerialNumber))
        return;

    QStringList itemList = deviceFiles.split(",");

    QString modelName = m_Name;
    modelName.replace(" ", "_");

    foreach (auto item, itemList) {
        if (item.contains("by-id", Qt::CaseInsensitive) &&
                item.contains(modelName, Qt::CaseInsensitive)) {

            int index;/* = item.indexOf(QRegExp("_[\\S\\d]*[\\s\\S]*$"));
            item = item.mid(index);*/
            index = item.lastIndexOf("_");
            item = item.mid(index + 1);
            item.replace(QRegExp("-[\\s\\S]*$"), "");
            m_SerialNumber = item;
            break;
        }
    }
}

QString DeviceStorage::getDiskSerialID()
{
    if (m_Interface.contains("USB", Qt::CaseInsensitive)) {
        return m_SerialNumber + m_KeyToLshw;
    }
    return m_SerialNumber;
}

void DeviceStorage::appendDisk(DeviceStorage *device)
{
    QList<QPair<QString, QString> > allAttribs = device->getBaseAttribs();
    QMap<QString, QString> allAttribMaps;
    for (int i = 0; i < allAttribs.size(); ++i) {
        allAttribMaps.insert(allAttribs[i].first, allAttribs[i].second);
    }
    QString size = allAttribMaps[tr("Size")];

    if (size.isEmpty()) return;
    // 直接设置大小
    if (m_Size.isEmpty())
        m_Size = size;
    else {
        QRegExp reg("[0-9]*.?[0-9]*");
        int index = reg.indexIn(m_Size);
        double num1 = 0;
        QString type1;
        // index>0时，对于"32GB"（数字开头的字符串,index=0）无法获取正确的数据32
        // 所以要改为index >= 0
        if (index >= 0) {
            num1 = reg.cap(0).toDouble();
            type1 = m_Size.right(m_Size.length() - reg.cap(0).size()).trimmed();
        }

        index = reg.indexIn(size);
        double num2 = 0;
        QString type2;
        if (index >= 0) {
            num2 = reg.cap(0).toDouble();
            type2 = size.right(size.length() - reg.cap(0).size()).trimmed();
        }
        // 匹配大小
        if (type1 != type2) {
            if (type1 == "TB" && type2 != "TB") {
                num2 /= 1000.0;
                type2 = "TB";
            } else if (type1 != "TB" && type2 == "TB") {
                num1 /= 1000.0;
                type1 = "TB";
            }
        }

        if (type1 == type2) {
            num1 = num1 + num2;
            if (num1 > int(num1))
                m_Size = QString("%1 %2").arg(QString::number(num1, 'f', 2)).arg(type1);
            else {
                m_Size = QString("%1 %2").arg(num1).arg(type1);
            }

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
                    setOtherDeviceInfo(keyStr, curBusInfo + "," + busInfo);
                }
            }

            loadOtherDeviceInfo();
        }
    }
}

void DeviceStorage::checkDiskSize()
{
    QRegExp reg("[0-9]*.?[0-9]*");
    int index = reg.indexIn(m_Size);
    // index>0时，对于"32GB"（数字开头的字符串,index=0）无法获取正确的数据32
    // 所以要改为index >= 0
    if (index >= 0) {
        double num = reg.cap(0).toDouble();
        double num1 = num - int(num);
        QString type = m_Size.right(m_Size.length() - reg.cap(0).size()).trimmed();
        if (!qFuzzyCompare(num1, 0.0) && type == "GB") {
            m_Size = QString::number(int(num) + 1) + type;
        }
    }
}

QString DeviceStorage::compareSize(const QString &size1, const QString &size2)
{
    // 比较smartctl中可能提供的两个大小，取大值作为存储设备的大小
    if (size1.isEmpty() || size2.isEmpty())
        return size1 + size2;

    // 将字符串转为数字大小进行比较
    int num1 = 0;
    int num2 = 0;
    QRegExp reg(".*\\[(\\d*).*\\]$");
    if (reg.exactMatch(size1))
        num1 = reg.cap(1).toInt();
    if (reg.exactMatch(size2))
        num2 = reg.cap(1).toInt();

    // 返回较大值
    if (num1 > num2)
        return size1;
    else
        return size2;
}

const QString &DeviceStorage::name() const
{
    return m_Name;
}

const QString &DeviceStorage::vendor() const
{
    return m_Vendor;
}

const QString &DeviceStorage::driver() const
{
    return m_Driver;
}

const QString &DeviceStorage::keyFromStorage()const
{
    return m_KeyFromStorage;
}

QString DeviceStorage::subTitle()
{
    return m_Name;
}

const QString DeviceStorage::getOverviewInfo()
{
    return QString("%1 (%2)").arg(m_Name).arg(m_Size);;
}

void DeviceStorage::initFilterKey()
{
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
        m_MediaType = QObject::tr("SSD");
    }

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceStorage::loadTableHeader()
{
    // 加载表头信息
    m_TableHeader.append(tr("Name"));
    m_TableHeader.append(tr("Vendor"));
    m_TableHeader.append(tr("Media Type"));
    m_TableHeader.append(tr("Size"));
}

void DeviceStorage::loadTableData()
{
    // 加载表格数据
    QString model = m_Name;
    if (!available()) {
        model = "(" + tr("Unavailable") + ") " + m_Name;
    }
    m_TableData.append(model);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_MediaType);
    m_TableData.append(m_Size);
}

void DeviceStorage::getInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // lshw信息获取
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "serial", m_SerialNumber, false);
    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "description", m_Description);
    setAttribute(mapInfo, "size", m_Size);
    // 223GiB (240GB)
    QRegExp re(".*\\((.*)\\)$");
    if (re.exactMatch(m_Size))
        m_Size = re.cap(1);

    if (m_SerialNumber.compare("0",Qt::CaseInsensitive) == 0)
        m_SerialNumber = "";
}

void DeviceStorage::getInfoFromsmartctl(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() < 5) {
        if (!mapInfo.isEmpty() && m_Interface.contains("USB", Qt::CaseInsensitive)) {
            m_MediaType = QObject::tr("SSD");
        }
        return;
    }
    // 固件版本
    m_FirmwareVersion = mapInfo["Firmware Version"];

    // 速度
    QString sataVersion = mapInfo["SATA Version is"];
    QStringList strList = sataVersion.split(",");
    if (strList.size() == 2)
        m_Speed = strList[1];

    setAttribute(mapInfo, "Rotation Rate", m_RotationRate);
    // 解决Bug45428,INTEL SSDSA2BW160G3L 这个型号的硬盘通过lsblk获取的rota是１，所以这里需要特殊处理
    if (m_RotationRate == QString("Solid State Device"))
        m_MediaType = QObject::tr("SSD");

    // 按照HW的需求，如果是固态硬盘就不显示转速
    if (m_RotationRate == QString("HW_SSD")) {
        m_MediaType = QObject::tr("SSD");
        m_RotationRate = "";
    }

    // 安装大小
    QString capacity = mapInfo["User Capacity"];
    if (capacity == "")
        capacity = compareSize(mapInfo["Total NVM Capacity"], mapInfo["Namespace 1 Size/Capacity"]);

    if (capacity != "") {
        QRegExp reg(".*\\[(.*)\\]$");
        if (reg.exactMatch(capacity))
            m_Size = reg.cap(1);
    }

    // 通过不断适配，当厂商有在固件中提供时，硬盘型号从smartctl中获取更加合理
    // 因为hwinfo获取的是主控的型号，而硬盘厂商由于还不能自己生产主控，只能采购别人的主控
    //SATA
    if (mapInfo.find("Device Model") != mapInfo.end())
        m_Name = mapInfo["Device Model"];
    //NVME
    if (mapInfo.find("Model Number") != mapInfo.end())
        m_Name = mapInfo["Model Number"];

    if (mapInfo.find("Serial Number") != mapInfo.end())
        setAttribute(mapInfo, "Serial Number", m_SerialNumber, true);
    else if (mapInfo.find("Serial number") != mapInfo.end()) {
        setAttribute(mapInfo, "Serial number", m_SerialNumber, true);
    }

    // 修正数值
    if(Common::boardVendorType() != "KLVV" && Common::boardVendorType() != "KLVU" \
        && Common::boardVendorType() != "PGUW" && Common::boardVendorType() != "PGUV")
            m_Size.replace(QRegExp("\\.0[1-9]"), ".00");
}
