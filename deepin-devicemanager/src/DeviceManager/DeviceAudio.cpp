// 项目自身文件
#include "DeviceAudio.h"

// 其它头文件
#include "EnableManager.h"

DeviceAudio::DeviceAudio()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Version("")
    , m_BusInfo("")
    , m_Irq("")
    , m_Memory("")
    , m_Width("")
    , m_Clock("")
    , m_Capabilities("")
    , m_Description("")
    , m_Chip("")
    , m_Driver("snd_hda_intel")
    , m_DriverModules("")
    , m_UniqueKey("")
{
    // 初始化可显示属性
    initFilterKey();

    // 设置可禁用
    m_CanEnable = true;
    m_IsCatDevice = false;
}

void DeviceAudio::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    //1. 获取设备的基本信息
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "", m_Version);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "IRQ", m_Irq);
    setAttribute(mapInfo, "Memory Range", m_Memory);
    setAttribute(mapInfo, "", m_Width);
    setAttribute(mapInfo, "", m_Clock);
    setAttribute(mapInfo, "", m_Capabilities);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Driver Modules", m_Driver);   // 驱动模块
    //2. 获取设备的唯一标识
    /*
     * 在这里将设备的总线信息作为一个设备的唯一标识
     * 我们会通过总线信息判断从两个不同的命令获取的设备信息是不是同一个设备的信息
     * 如果从两个命令中获取的设备信息的总线信息一样，我们就认为是同一个设备
     * 比如从hwinfo里面获取到的  SysFS BusID: 1-3:1.0   和
     *    从lshw里面获取到的    bus info: usb@1:3       是同一个设备
     */
    m_UniqueKey = mapInfo["SysFS BusID"];

    //3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
}

bool DeviceAudio::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    //1. 先判断传入的设备信息是否是该设备信息，根据总线信息来判断
    QStringList words = mapInfo["bus info"].split("@");
    if (words.size() != 2) {
        return false;
    }

    if (words[1] != m_UniqueKey) {
        return false;
    }

    //2. 确定了是该设备信息，则获取设备的基本信息
    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    if (m_Vendor == "0000") {
        m_Vendor = "";
    }

    setAttribute(mapInfo, "", m_Model);

    /*
     * 再过去设备的版本号的时候，会出现版本号为00的情况，
     * 这不符合常理，所以当版本号为00时，默认版本号获取不到
    */
    setAttribute(mapInfo, "version", m_Version);
    if (m_Version == "00") {
        m_Version = "";
    }

    // 获取设备的基本信息
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "", m_Irq);
    setAttribute(mapInfo, "", m_Memory);
    setAttribute(mapInfo, "width", m_Width);
    setAttribute(mapInfo, "clock", m_Clock);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "description", m_Description);

    //3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);

    return true;
}

bool DeviceAudio::setInfoFromCatDevices(const QMap<QString, QString> &mapInfo)
{
    //1. 获取设备的基本信息
    setAttribute(mapInfo, "Name", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "Version", m_Version);
    setAttribute(mapInfo, "Bus", m_BusInfo);
    setAttribute(mapInfo, "", m_Irq);
    setAttribute(mapInfo, "", m_Memory);
    setAttribute(mapInfo, "", m_Width);
    setAttribute(mapInfo, "", m_Clock);
    setAttribute(mapInfo, "", m_Capabilities);
    setAttribute(mapInfo, "", m_Description);

    //2. 获取设备的其它信息
    getOtherMapInfo(mapInfo);

    //3. get from cat /input/devices
    m_IsCatDevice = true;

    return true;
}

void DeviceAudio::setInfoFromCatAudio(const QMap<QString, QString> &mapInfo)
{
    //1. 获取设备的基本信息
    setAttribute(mapInfo, "Name", m_Name);

    //2. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
}

bool DeviceAudio::setAudioChipFromDmesg(const QString &info)
{
    // 设置声卡芯片型号
    m_Chip = info;
    return true;
}

const QString &DeviceAudio::name()const
{
    return m_Name;
}

const QString &DeviceAudio::driver() const
{
    return m_Driver;
}

EnableDeviceStatus DeviceAudio::setEnable(bool e)
{
    // 设置设备状态
    EnableDeviceStatus res = EnableManager::instance()->enableDeviceByDriver(e, m_Driver);
    if (e != enable()) {
        res = EDS_Faild;
    }

    return res;
}

bool DeviceAudio::enable()
{
    // 获取设备状态
    bool eDriver = EnableManager::instance()->isDeviceEnableByDriver(m_Driver);
    bool eDriverM = EnableManager::instance()->isDeviceEnableByDriver(m_DriverModules);
    m_Enable = eDriver || eDriverM;


    // 如果是从cat /input/devices里面获取的则返回true
    if (m_IsCatDevice) {
        return true;
    }

    return m_Enable;
}

QString DeviceAudio::subTitle()
{
    // 设备信息子标题
    return m_Name;
}

const QString DeviceAudio::getOverviewInfo()
{
    // 获取概况信息
    return m_Name.isEmpty() ? m_Model : m_Name;
}

void DeviceAudio::initFilterKey()
{
    // 添加可显示的属性
    addFilterKey(tr("Device Name"));
    addFilterKey(QObject::tr("SubVendor"));
    addFilterKey(QObject::tr("SubDevice"));
    addFilterKey(QObject::tr("Driver"));
//    addFilterKey(QObject::tr("Driver Modules"));
    addFilterKey(QObject::tr("Driver Status"));
    addFilterKey(QObject::tr("Driver Activation Cmd"));
    addFilterKey(QObject::tr("Config Status"));

    //addFilterKey(QObject::tr("irq"));
    addFilterKey(QObject::tr("physical id"));
    addFilterKey(QObject::tr("latency"));

    addFilterKey(QObject::tr("Phys"));
    addFilterKey(QObject::tr("Sysfs"));
    addFilterKey(QObject::tr("Handlers"));
    addFilterKey(QObject::tr("PROP"));
    addFilterKey(QObject::tr("EV"));
    addFilterKey(QObject::tr("KEY"));

    addFilterKey(QObject::tr("Model"));
    addFilterKey(QObject::tr("Vendor"));
    addFilterKey(QObject::tr("Version"));
    addFilterKey(QObject::tr("Bus"));
}

void DeviceAudio::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
}

void DeviceAudio::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Chip"), m_Chip);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(tr("Clock"), m_Clock);
    addOtherDeviceInfo(tr("Width"), m_Width);
    addOtherDeviceInfo(tr("Memory"), m_Memory);
    addOtherDeviceInfo(tr("IRQ"), m_Irq);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceAudio::loadTableHeader()
{
    // 表头信息
    m_TableHeader.append(tr("Name"));
    m_TableHeader.append(tr("Vendor"));
}

void DeviceAudio::loadTableData()
{
    // 记载表格内容
    QString name;
    if (!enable()) {
        name = "(" + tr("Disable") + ") " + m_Name;
    } else {
        name = m_Name;
    }

    m_TableData.append(name);
    m_TableData.append(m_Vendor);
}
