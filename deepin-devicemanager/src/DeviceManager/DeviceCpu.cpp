// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceCpu.h"

#include <math.h>

DeviceCpu::DeviceCpu()
    : DeviceBaseInfo()
    , m_PhysicalID("")
    , m_CoreID("")
    , m_ThreadNum("")
    , m_Frequency("")
    , m_CurFrequency("")
    , m_BogoMIPS("")
    , m_Architecture(""),
      m_Familly("")
    , m_Model("")
    , m_Step("")
    , m_CacheL1Data("")
    , m_CacheL1Order("")
    , m_CacheL2("")
    , m_CacheL3("")
    , m_CacheL4("")
    , m_Extensions("")
    , m_Flags("")
    , m_HardwareVirtual("")
    , m_LogicalCPUNum(0)
    , m_CPUCoreNum(0)
    , m_FrequencyIsRange(false)
    , m_FrequencyIsCur(true)
{
    initFilterKey();
}

void DeviceCpu::setCpuInfo(const QMap<QString, QString> &mapLscpu, const QMap<QString, QString> &mapLshw, const QMap<QString, QString> &mapDmidecode, int coreNum, int logicalNum)
{
    // 设置CPU信息
    setInfoFromLscpu(mapLscpu);
    setInfoFromLshw(mapLshw);
    setInfoFromDmidecode(mapDmidecode);

    // CPU 名称后面不需要加个数
    m_Name.replace(QRegExp("/[0-9]*$"), "");
    m_Name.replace(QRegExp("x [0-9]*$"), "");

    if (Common::specialComType == Common::kSpecialType5 ||
        Common::specialComType == Common::kSpecialType6 ||
        Common::specialComType == Common::kSpecialType7) {
        m_Frequency = m_Frequency.replace("2.189", "2.188");
        m_MaxFrequency = m_MaxFrequency.replace("2189", "2188");
    }

    //  获取逻辑数和core数
    m_LogicalCPUNum = logicalNum;
    m_CPUCoreNum = coreNum;
}

void DeviceCpu::initFilterKey()
{
    // 添加可显示的属性
    addFilterKey("CPU implementer");
    addFilterKey("CPU architecture");
    addFilterKey("CPU variant");
    addFilterKey("CPU part");
    addFilterKey("CPU revision");
}

void DeviceCpu::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(("Name"), m_Name);
    addBaseDeviceInfo(("Vendor"), m_Vendor);
//    addBaseDeviceInfo(("CPU ID"), m_PhysicalID);
//    addBaseDeviceInfo(("Core ID"), m_CoreID);
    addBaseDeviceInfo(("Threads"), m_ThreadNum);
    if (!m_FrequencyIsCur)
        addBaseDeviceInfo(("Max Frequency"), m_MaxFrequency);
//    addBaseDeviceInfo(("BogoMIPS"), m_BogoMIPS);
    addBaseDeviceInfo(("Architecture"), m_Architecture);
    addBaseDeviceInfo(("CPU Family"), m_Familly);
    addBaseDeviceInfo(("Model"), m_Model);
}

const QString &DeviceCpu::vendor() const
{
    return m_Vendor;
}

const QString &DeviceCpu::name() const
{
    return m_Name ;
}

const QString &DeviceCpu::driver() const
{
    return m_Driver;
}

bool DeviceCpu::available()
{
    return true;
}

bool DeviceCpu::frequencyIsRange()const
{
    return m_FrequencyIsRange;
}

QString DeviceCpu::subTitle()
{
    return QString("%1 %2").arg(translateStr("Processor")).arg(m_PhysicalID);
}

const QString DeviceCpu::getOverviewInfo()
{
    // 获取阿拉伯数字的英文翻译
    getTrNumber();

    QString ov = QString("%1 (%2%3 / %4%5)") \
                 .arg(m_Name) \
                 .arg(m_trNumber[m_CPUCoreNum]) \
                 .arg(translateStr("Core(s)")) \
                 .arg(m_trNumber[m_LogicalCPUNum]) \
                 .arg(translateStr("Processor"));

    return ov;
}

void DeviceCpu::setInfoFromLscpu(const QMap<QString, QString> &mapInfo)
{
    // 设置CPU属性
    setAttribute(mapInfo, "model name", m_Name);
    setAttribute(mapInfo, "vendor_id", m_Vendor, false);
    setAttribute(mapInfo, "Thread(s) per core", m_ThreadNum);
    setAttribute(mapInfo, "bogomips", m_BogoMIPS);
    setAttribute(mapInfo, "Architecture", m_Architecture);
    setAttribute(mapInfo, "cpu family", m_Familly);
    setAttribute(mapInfo, "CPU MHz", m_CurFrequency);
    setAttribute(mapInfo, "model", m_Model);
    setAttribute(mapInfo, "stepping", m_Step);
    setAttribute(mapInfo, "L1d cache", m_CacheL1Data);
    setAttribute(mapInfo, "L1i cache", m_CacheL1Order);
    setAttribute(mapInfo, "L2 cache", m_CacheL2);
    setAttribute(mapInfo, "L3 cache", m_CacheL3);
    setAttribute(mapInfo, "L4 cache", m_CacheL4);
    setAttribute(mapInfo, "flags", m_Flags);
    setAttribute(mapInfo, "Virtualization", m_HardwareVirtual);

    setAttribute(mapInfo, "processor", m_PhysicalID);
    setAttribute(mapInfo, "core id", m_CoreID);

    // 计算频率范围
    bool min = mapInfo.find("CPU min MHz") != mapInfo.end();
    bool max = mapInfo.find("CPU max MHz") != mapInfo.end();
    if (min && max) {
        QString minS = mapInfo["CPU min MHz"];
        QString maxS = mapInfo["CPU max MHz"];
        m_MaxFrequency = maxS;
        double minHz = minS.replace("MHz", "").toDouble() / 1000;
        double maxHz = maxS.replace("MHz", "").toDouble() / 1000;
        m_FrequencyIsRange = true;

        // 如果最大最小频率相等则不显示范围
        if (fabs(minHz - maxHz) < 0.001) {
            m_FrequencyIsRange = false;
            m_Frequency = maxHz > 1 ? QString("%1 GHz").arg(maxHz) : QString("%1 MHz").arg(maxHz * 1000);
        } else {
            m_Frequency = QString("%1-%2 GHz").arg(minHz).arg(maxHz);
        }
    } else if (mapInfo.find("CPU MHz") != mapInfo.end()) {
        QString maxS = mapInfo["CPU MHz"];
        m_Frequency = maxS.indexOf("MHz") > -1 ? maxS : maxS + " MHz";
    }
    //获取扩展指令集
    QStringList orders = {"MMX", "SSE", "SSE2", "SSE3", "3D Now", "SSE4", "SSSE3", "SSE4_1", "SSE4_2", "AMD64", "EM64T"};
    foreach (const QString &order, orders) {
        if (mapInfo["Flags"].contains(order, Qt::CaseInsensitive))
            m_Extensions += QString("%1 ").arg(order);
    }

    // 获取其他属性
    getOtherMapInfo(mapInfo);
}

void DeviceCpu::setCurFreq(const QString &curFreq)
{
    if (!curFreq.isEmpty())
        m_CurFrequency = curFreq;
}

void DeviceCpu::setFrequencyIsCur(const bool &flag)
{
    m_FrequencyIsCur = flag;
}

void DeviceCpu::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // longxin CPU型号不从lshw中获取
    // bug39874
    if (m_Name.contains("Loongson", Qt::CaseInsensitive))
        setAttribute(mapInfo, "version", m_Name, false);
    else {
        setAttribute(mapInfo, "product", m_Name, false);//这里覆盖了lscpu中的数据。主要版本中lshw中的version是CPU信息，而1050A中读取到的是版本信息。

        // bug-108166 lshw 中 product 包含NULL信息，version 信息正确
        // bug-112403 lshw 中 product 包含ARMv信息，version 信息正确
        if (m_Name.contains("null", Qt::CaseInsensitive) || m_Name.contains("ARMv", Qt::CaseInsensitive)) {
            setAttribute(mapInfo, "version", m_Name);
        }
    }

    // 获取设备基本信息
    setAttribute(mapInfo, "vendor", m_Vendor);

    // 获取设备其他信息
    getOtherMapInfo(mapInfo);
}

TomlFixMethod DeviceCpu::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    setTomlAttribute(mapInfo, "CPU ID", m_PhysicalID);
    setTomlAttribute(mapInfo, "Core ID", m_CoreID);
    setTomlAttribute(mapInfo, "Threads", m_ThreadNum);
    setTomlAttribute(mapInfo, "Frequency", m_Frequency);
    setTomlAttribute(mapInfo, "Current Frequency", m_CurFrequency);
    setTomlAttribute(mapInfo, "Max Frequency", m_MaxFrequency);
    setTomlAttribute(mapInfo, "BogoMIPS", m_BogoMIPS);
    setTomlAttribute(mapInfo, "Architecture", m_Architecture);
    setTomlAttribute(mapInfo, "CPU Family", m_Familly);
    setTomlAttribute(mapInfo, "Model", m_Model);
    // 添加其他信息,成员变量
    setTomlAttribute(mapInfo, "Virtualization", m_HardwareVirtual);
    setTomlAttribute(mapInfo, "Flags", m_Flags);
    setTomlAttribute(mapInfo, "Extensions", m_Extensions);
    setTomlAttribute(mapInfo, "L4 Cache", m_CacheL4);
    setTomlAttribute(mapInfo, "L3 Cache", m_CacheL3);
    setTomlAttribute(mapInfo, "L2 Cache", m_CacheL2);
    setTomlAttribute(mapInfo, "L1i Cache", m_CacheL1Order);
    setTomlAttribute(mapInfo, "L1d Cache", m_CacheL1Data);
    ret = setTomlAttribute(mapInfo, "Stepping", m_Step);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

void DeviceCpu::setInfoFromDmidecode(const QMap<QString, QString> &mapInfo)
{
    // longxin CPU型号不从dmidecode中获取
    // bug39874
    if (m_Name.contains("Loongson", Qt::CaseInsensitive)) {
        setAttribute(mapInfo, "product", m_Name, false);
    } else {
        setAttribute(mapInfo, "product", m_Name);
    }

    if (Common::isHwPlatform()) {
        if (mapInfo.contains("Version")) {
            setAttribute(mapInfo, "Version", m_Name);
        }
    }

    // 获取设备基本信息
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Max Speed", m_Frequency, false);
    setAttribute(mapInfo, "Max Speed", m_MaxFrequency, false);
    // 飞腾架构由于无法通过lscpu获取当前频率，因此需要通过dmidecode获取
    setAttribute(mapInfo, "Current Speed", m_CurFrequency, false);
    setAttribute(mapInfo, "Family", m_Familly, false);
    // 特殊机型，通过 dmidecode 获取最大加速频率
    if (Common::curCpuType == Common::kSpecialCpuType1)
        setAttribute(mapInfo, "Max Speed", m_MaxBoostClock);

    // 获取其他cpu信息
    getOtherMapInfo(mapInfo);
}

void DeviceCpu::loadOtherDeviceInfo()
{
    // 倒序，头插，保证原来的顺序
    // 添加其他信息,成员变量
    addOtherDeviceInfo(("Virtualization"), m_HardwareVirtual);
    addOtherDeviceInfo(("Flags"), m_Flags);
    addOtherDeviceInfo(("Extensions"), m_Extensions);
    addOtherDeviceInfo(("L4 Cache"), m_CacheL4);
    addOtherDeviceInfo(("L3 Cache"), m_CacheL3);
    addOtherDeviceInfo(("L2 Cache"), m_CacheL2);
    addOtherDeviceInfo(("L1i Cache"), m_CacheL1Order);
    addOtherDeviceInfo(("L1d Cache"), m_CacheL1Data);
    addOtherDeviceInfo(("Stepping"), m_Step);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceCpu::loadTableHeader()
{
    // 加载表头
    m_TableHeader.append("Name");
    m_TableHeader.append("Vendor");
    m_TableHeader.append(frequencyIsRange() ? ("Frequency") : ("Max Frequency"));
    if (Common::curCpuType == Common::kSpecialCpuType1)
        m_TableHeader.append(tr("Max Boost Clock"));
    m_TableHeader.append("Architecture");
}

void DeviceCpu::loadTableData()
{
    // 加载表格信息
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Frequency);
    if (Common::curCpuType == Common::kSpecialCpuType1)
        m_TableData.append(m_MaxBoostClock);
    m_TableData.append(m_Architecture);
}

void DeviceCpu::getTrNumber()
{
    // 将数字转换为英文翻译
    m_trNumber.insert(1, translateStr("One"));
    m_trNumber.insert(2, translateStr("Two"));
    m_trNumber.insert(4, translateStr("Four"));
    m_trNumber.insert(6, translateStr("Six"));
    m_trNumber.insert(8, translateStr("Eight"));
    m_trNumber.insert(9, translateStr("Nine"));
    m_trNumber.insert(10, translateStr("Ten"));
    m_trNumber.insert(12, translateStr("Twelve"));
    m_trNumber.insert(14, translateStr("Fourteen"));
    m_trNumber.insert(16, translateStr("Sixteen"));
    m_trNumber.insert(18, translateStr("Eighteen"));

    m_trNumber.insert(20, translateStr("Twenty"));
    m_trNumber.insert(22, translateStr("Twenty-two"));
    m_trNumber.insert(24, translateStr("Twenty-four"));
    m_trNumber.insert(26, translateStr("Twenty-six"));
    m_trNumber.insert(28, translateStr("Twenty-eight"));

    m_trNumber.insert(30, translateStr("Thirty"));
    m_trNumber.insert(32, translateStr("Thirty-two"));
    m_trNumber.insert(34, translateStr("Thirty-four"));
    m_trNumber.insert(36, translateStr("Thirty-six"));
    m_trNumber.insert(38, translateStr("Thirty-eight"));

    m_trNumber.insert(40, translateStr("Forty"));
    m_trNumber.insert(42, translateStr("Forty-two"));
    m_trNumber.insert(44, translateStr("Forty-four"));
    m_trNumber.insert(46, translateStr("Forty-six"));
    m_trNumber.insert(48, translateStr("Forty-eight"));

    m_trNumber.insert(50, translateStr("Fifty"));
    m_trNumber.insert(52, translateStr("Fifty-two"));
    m_trNumber.insert(54, translateStr("Fifty-four"));
    m_trNumber.insert(56, translateStr("Fifty-six"));
    m_trNumber.insert(58, translateStr("Fifty-eight"));

    m_trNumber.insert(60, translateStr("Sixty"));
    m_trNumber.insert(62, translateStr("Sixty-two"));
    m_trNumber.insert(64, translateStr("Sixty-four"));
    m_trNumber.insert(66, translateStr("Sixty-six"));
    m_trNumber.insert(68, translateStr("Sixty-eight"));

    m_trNumber.insert(70, translateStr("Seventy"));
    m_trNumber.insert(72, translateStr("Seventy-two"));
    m_trNumber.insert(74, translateStr("Seventy-four"));
    m_trNumber.insert(76, translateStr("Seventy-six"));
    m_trNumber.insert(78, translateStr("Seventy-eight"));

    m_trNumber.insert(80, translateStr("Eighty"));
    m_trNumber.insert(82, translateStr("Eighty-two"));
    m_trNumber.insert(84, translateStr("Eighty-four"));
    m_trNumber.insert(86, translateStr("Eighty-six"));
    m_trNumber.insert(88, translateStr("Eighty-eight"));

    m_trNumber.insert(90, translateStr("Ninety"));
    m_trNumber.insert(92, translateStr("Ninety-two"));
    m_trNumber.insert(94, translateStr("Ninety-four"));
    m_trNumber.insert(96, translateStr("Ninety-six"));
    m_trNumber.insert(98, translateStr("Ninety-eight"));

    m_trNumber.insert(100, translateStr("One hundred"));
    m_trNumber.insert(102, translateStr("One hundred and Two"));
    m_trNumber.insert(104, translateStr("One hundred and four"));
    m_trNumber.insert(106, translateStr("One hundred and Six"));
    m_trNumber.insert(108, translateStr("One hundred and Eight"));

    m_trNumber.insert(110, translateStr("One hundred and Ten"));
    m_trNumber.insert(112, translateStr("One hundred and Twelve"));
    m_trNumber.insert(114, translateStr("One hundred and Fourteen"));
    m_trNumber.insert(116, translateStr("One hundred and Sixteen"));
    m_trNumber.insert(118, translateStr("One hundred and Eighteen"));

    m_trNumber.insert(120, translateStr("One hundred and Twenty"));
    m_trNumber.insert(122, translateStr("One hundred and Twenty-two"));
    m_trNumber.insert(124, translateStr("One hundred and Twenty-four"));
    m_trNumber.insert(126, translateStr("One hundred and Twenty-six"));
    m_trNumber.insert(128, translateStr("One hundred and Twenty-eight"));
    m_trNumber.insert(192, translateStr("One hundred and Ninety-two"));
    m_trNumber.insert(256, translateStr("Two hundred and fifty-six"));
}


