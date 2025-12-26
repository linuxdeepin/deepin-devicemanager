// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceCpu.h"
#include "DDLog.h"
#include <QRegularExpression>
#include <math.h>

const QMap<int, const char*> DeviceCpu::m_trNumber = {
    {1, QT_TR_NOOP("One")},
    {2, QT_TR_NOOP("Two")},
    {4, QT_TR_NOOP("Four")},
    {6, QT_TR_NOOP("Six")},
    {8, QT_TR_NOOP("Eight")},
    {10, QT_TR_NOOP("Ten")},
    {12, QT_TR_NOOP("Twelve")},
    {14, QT_TR_NOOP("Fourteen")},
    {16, QT_TR_NOOP("Sixteen")},
    {18, QT_TR_NOOP("Eighteen")},
    {20, QT_TR_NOOP("Twenty")},
    {22, QT_TR_NOOP("Twenty-two")},
    {24, QT_TR_NOOP("Twenty-four")},
    {26, QT_TR_NOOP("Twenty-six")},
    {28, QT_TR_NOOP("Twenty-eight")},
    {30, QT_TR_NOOP("Thirty")},
    {32, QT_TR_NOOP("Thirty-two")},
    {34, QT_TR_NOOP("Thirty-four")},
    {36, QT_TR_NOOP("Thirty-six")},
    {38, QT_TR_NOOP("Thirty-eight")},
    {40, QT_TR_NOOP("Forty")},
    {42, QT_TR_NOOP("Forty-two")},
    {44, QT_TR_NOOP("Forty-four")},
    {46, QT_TR_NOOP("Forty-six")},
    {48, QT_TR_NOOP("Forty-eight")},
    {50, QT_TR_NOOP("Fifty")},
    {52, QT_TR_NOOP("Fifty-two")},
    {54, QT_TR_NOOP("Fifty-four")},
    {56, QT_TR_NOOP("Fifty-six")},
    {58, QT_TR_NOOP("Fifty-eight")},
    {60, QT_TR_NOOP("Sixty")},
    {62, QT_TR_NOOP("Sixty-two")},
    {64, QT_TR_NOOP("Sixty-four")},
    {66, QT_TR_NOOP("Sixty-six")},
    {68, QT_TR_NOOP("Sixty-eight")},
    {70, QT_TR_NOOP("Seventy")},
    {72, QT_TR_NOOP("Seventy-two")},
    {74, QT_TR_NOOP("Seventy-four")},
    {76, QT_TR_NOOP("Seventy-six")},
    {78, QT_TR_NOOP("Seventy-eight")},
    {80, QT_TR_NOOP("Eighty")},
    {82, QT_TR_NOOP("Eighty-two")},
    {84, QT_TR_NOOP("Eighty-four")},
    {86, QT_TR_NOOP("Eighty-six")},
    {88, QT_TR_NOOP("Eighty-eight")},
    {90, QT_TR_NOOP("Ninety")},
    {92, QT_TR_NOOP("Ninety-two")},
    {94, QT_TR_NOOP("Ninety-four")},
    {96, QT_TR_NOOP("Ninety-six")},
    {98, QT_TR_NOOP("Ninety-eight")},
    {100, QT_TR_NOOP("One hundred")},
    {102, QT_TR_NOOP("One hundred and Two")},
    {104, QT_TR_NOOP("One hundred and four")},
    {106, QT_TR_NOOP("One hundred and Six")},
    {108, QT_TR_NOOP("One hundred and Eight")},
    {110, QT_TR_NOOP("One hundred and Ten")},
    {112, QT_TR_NOOP("One hundred and Twelve")},
    {114, QT_TR_NOOP("One hundred and Fourteen")},
    {116, QT_TR_NOOP("One hundred and Sixteen")},
    {118, QT_TR_NOOP("One hundred and Eighteen")},
    {120, QT_TR_NOOP("One hundred and Twenty")},
    {122, QT_TR_NOOP("One hundred and Twenty-two")},
    {124, QT_TR_NOOP("One hundred and Twenty-four")},
    {126, QT_TR_NOOP("One hundred and Twenty-six")},
    {128, QT_TR_NOOP("One hundred and Twenty-eight")},
    {192, QT_TR_NOOP("One hundred and Ninety-two")},
    {256, QT_TR_NOOP("Two hundred and fifty-six")}
};

using namespace DDLog;

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
    qCDebug(appLog) << "DeviceCpu constructor initialized";
    initFilterKey();
}

void DeviceCpu::setCpuInfo(const QMap<QString, QString> &mapLscpu, const QMap<QString, QString> &mapLshw, const QMap<QString, QString> &mapDmidecode, int coreNum, int logicalNum)
{
    qCDebug(appLog) << "Start setting CPU info";

    // 设置CPU信息
    setInfoFromLscpu(mapLscpu);
    setInfoFromLshw(mapLshw);
    setInfoFromDmidecode(mapDmidecode);

    // CPU 名称后面不需要加个数
    m_Name.replace(QRegularExpression("/[0-9]*$"), "");
    m_Name.replace(QRegularExpression("x [0-9]*$"), "");
    qCDebug(appLog) << "CPU name after regex replacement: " << m_Name;

    if (Common::specialComType == Common::kSpecialType5 ||
        Common::specialComType == Common::kSpecialType6 ||
        Common::specialComType == Common::kSpecialType7) {
        m_Frequency = m_Frequency.replace("2.189", "2.188");
        m_MaxFrequency = m_MaxFrequency.replace("2189", "2188");
    }

    //  获取逻辑数和core数
    m_LogicalCPUNum = logicalNum;
    m_CPUCoreNum = coreNum;
    qCDebug(appLog) << "Logical CPU Num: " << m_LogicalCPUNum << ", CPU Core Num: " << m_CPUCoreNum;
    qCDebug(appLog) << "DeviceCpu::setCpuInfo finished.";
}

void DeviceCpu::initFilterKey()
{
    qCDebug(appLog) << "DeviceCpu::initFilterKey called.";
    // 添加可显示的属性
    addFilterKey("CPU implementer");
    addFilterKey("CPU architecture");
    addFilterKey("CPU variant");
    addFilterKey("CPU part");
    addFilterKey("CPU revision");
    qCDebug(appLog) << "Filter keys initialized.";
}

void DeviceCpu::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "Start loading base CPU info";

    // 添加基本信息
    addBaseDeviceInfo("Name", m_Name);
    addBaseDeviceInfo("Vendor", m_Vendor);
    //    addBaseDeviceInfo(("CPU ID"), m_PhysicalID);
    //    addBaseDeviceInfo(("Core ID"), m_CoreID);
    addBaseDeviceInfo("Threads", m_ThreadNum);
    if (!m_FrequencyIsCur) {
        qCDebug(appLog) << "Frequency is not current, adding Max Speed.";
        addBaseDeviceInfo("Max Speed", m_MaxFrequency);
    }
//    addBaseDeviceInfo(("BogoMIPS"), m_BogoMIPS);
    addBaseDeviceInfo("Architecture", m_Architecture);
    addBaseDeviceInfo("CPU Family", m_Familly);
    addBaseDeviceInfo("Model", m_Model);
}

const QString &DeviceCpu::vendor() const
{
    // qCDebug(appLog) << "DeviceCpu::vendor called, returning: " << m_Vendor;
    return m_Vendor;
}

const QString &DeviceCpu::name() const
{
    // qCDebug(appLog) << "DeviceCpu::name called, returning: " << m_Name;
    return m_Name ;
}

const QString &DeviceCpu::driver() const
{
    // qCDebug(appLog) << "DeviceCpu::driver called, returning: " << m_Driver;
    return m_Driver;
}

bool DeviceCpu::available()
{
    // qCDebug(appLog) << "DeviceCpu::available called, returning true.";
    return true;
}

bool DeviceCpu::frequencyIsRange()const
{
    // qCDebug(appLog) << "DeviceCpu::frequencyIsRange called, returning: " << m_FrequencyIsRange;
    return m_FrequencyIsRange;
}

QString DeviceCpu::subTitle()
{
    // qCDebug(appLog) << "DeviceCpu::subTitle called, returning: " << QString("%1 %2").arg(translateStr("Processor")).arg(m_PhysicalID);
    return QString("%1 %2").arg(translateStr("Processor")).arg(m_PhysicalID);
}

const QString DeviceCpu::getOverviewInfo()
{
    qCDebug(appLog) << "Getting CPU overview info";

    // 获取阿拉伯数字的英文翻译
    QString ov = QString("%1 (%2%3 / %4%5)") \
                 .arg(m_Name) \
                 .arg(tr(m_trNumber.value(m_CPUCoreNum, ""))) \
                 .arg(translateStr("Core(s)")) \
                 .arg(tr(m_trNumber.value(m_LogicalCPUNum, ""))) \
                 .arg(translateStr("Processor"));
    qCDebug(appLog) << "Overview info: " << ov;
    qCDebug(appLog) << "DeviceCpu::getOverviewInfo finished.";
    return ov;
}

void DeviceCpu::setInfoFromLscpu(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Start setting CPU info from lscpu";

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
    qCDebug(appLog) << "Attributes set from lscpu.";

    // 计算频率范围
    bool min = mapInfo.find("CPU min MHz") != mapInfo.end();
    bool max = mapInfo.find("CPU max MHz") != mapInfo.end();
    if (min && max) {
        qCDebug(appLog) << "Min and max CPU MHz found, calculating frequency range.";
        QString minS = mapInfo["CPU min MHz"];
        QString maxS = mapInfo["CPU max MHz"];
        m_MaxFrequency = maxS;
        double minHz = minS.replace("MHz", "").toDouble() / 1000;
        double maxHz = maxS.replace("MHz", "").toDouble() / 1000;
        m_FrequencyIsRange = true;

        // 如果最大最小频率相等则不显示范围
        if (fabs(minHz - maxHz) < 0.001) {
            qCDebug(appLog) << "Min and max frequency are close, setting as single value.";
            m_FrequencyIsRange = false;
            m_Frequency = maxHz > 1 ? QString("%1 GHz").arg(maxHz) : QString("%1 MHz").arg(maxHz * 1000);
        } else {
            qCDebug(appLog) << "Min and max frequency differ, setting as range.";
            m_Frequency = QString("%1-%2 GHz").arg(minHz).arg(maxHz);
        }
    } else if (mapInfo.find("CPU MHz") != mapInfo.end()) {
        qCDebug(appLog) << "Only CPU MHz found, setting as single value.";
        QString maxS = mapInfo["CPU MHz"];
        m_Frequency = maxS.indexOf("MHz") > -1 ? maxS : maxS + " MHz";
    }
    //获取扩展指令集
    QStringList orders = {"MMX", "SSE", "SSE2", "SSE3", "3D Now", "SSE4", "SSSE3", "SSE4_1", "SSE4_2", "AMD64", "EM64T"};
    foreach (const QString &order, orders) {
        if (mapInfo["Flags"].contains(order, Qt::CaseInsensitive)) {
            m_Extensions += QString("%1 ").arg(order);
            qCDebug(appLog) << "Found extension:" << order;
        }
    }
    qCDebug(appLog) << "Extensions found: " << m_Extensions.trimmed();

    // 获取其他属性
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceCpu::setInfoFromLscpu finished.";
}

void DeviceCpu::setCurFreq(const QString &curFreq)
{
    qCDebug(appLog) << "DeviceCpu::setCurFreq called with curFreq: " << curFreq;
    if (!curFreq.isEmpty()) {
        m_CurFrequency = curFreq;
        qCDebug(appLog) << "Current frequency set to: " << m_CurFrequency;
    }
}

void DeviceCpu::setFrequencyIsCur(const bool &flag)
{
    qCDebug(appLog) << "DeviceCpu::setFrequencyIsCur called with flag: " << flag;
    m_FrequencyIsCur = flag;
}

void DeviceCpu::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceCpu::setInfoFromLshw started.";
    // longxin CPU型号不从lshw中获取
    // bug39874
    if (m_Name.contains("Loongson", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "CPU name contains 'Loongson', skipping product attribute setting.";
        setAttribute(mapInfo, "version", m_Name, false);
    } else {
        setAttribute(mapInfo, "product", m_Name, false);//这里覆盖了lscpu中的数据。主要版本中lshw中的version是CPU信息，而1050A中读取到的是版本信息。
        qCDebug(appLog) << "Product attribute set from lshw: " << m_Name;
        // bug-108166 lshw 中 product 包含NULL信息，version 信息正确
        // bug-112403 lshw 中 product 包含ARMv信息，version 信息正确
        if (m_Name.contains("null", Qt::CaseInsensitive) || m_Name.contains("ARMv", Qt::CaseInsensitive)) {
            qCDebug(appLog) << "Product name contains 'null' or 'ARMv', using version for name.";
            setAttribute(mapInfo, "version", m_Name);
        }
    }

    // 获取设备基本信息
    setAttribute(mapInfo, "vendor", m_Vendor);
    qCDebug(appLog) << "Vendor attribute set from lshw: " << m_Vendor;

    // 获取设备其他信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceCpu::setInfoFromLshw finished.";
}

TomlFixMethod DeviceCpu::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceCpu::setInfoFromTomlOneByOne started.";
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
    qCDebug(appLog) << "Basic toml attributes set.";
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
    qCDebug(appLog) << "Setting CPU info from dmidecode";
    // longxin CPU型号不从dmidecode中获取
    // bug39874
    if (m_Name.contains("Loongson", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "Loongson CPU detected, not setting product from dmidecode";
        setAttribute(mapInfo, "product", m_Name, false);
    } else {
        qCDebug(appLog) << "Setting product from dmidecode";
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

    // 获取其他cpu信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "Finished setting CPU info from dmidecode";
}

void DeviceCpu::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "Loading other device info";
    // 倒序，头插，保证原来的顺序
    // 添加其他信息,成员变量
    addOtherDeviceInfo("Virtualization", m_HardwareVirtual);
    addOtherDeviceInfo("Flags", m_Flags);
    addOtherDeviceInfo("Extensions", m_Extensions);
    addOtherDeviceInfo("L4 Cache", m_CacheL4);
    addOtherDeviceInfo("L3 Cache", m_CacheL3);
    addOtherDeviceInfo("L2 Cache", m_CacheL2);
    addOtherDeviceInfo("L1i Cache", m_CacheL1Order);
    addOtherDeviceInfo("L1d Cache", m_CacheL1Data);
    addOtherDeviceInfo("Stepping", m_Step);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
    qCDebug(appLog) << "Other device info loaded.";
}

void DeviceCpu::loadTableHeader()
{
    qCDebug(appLog) << "Loading table header";
    // 加载表头
    m_TableHeader.append("Name");
    m_TableHeader.append("Vendor");
    m_TableHeader.append(frequencyIsRange() ? tr("Frequency") : tr("Max Frequency"));
    m_TableHeader.append("Architecture");
}

void DeviceCpu::loadTableData()
{
    qCDebug(appLog) << "Loading table data";
    // 加载表格信息
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Frequency);
    m_TableData.append(m_Architecture);
}
