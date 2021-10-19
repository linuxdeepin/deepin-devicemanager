#include "CpuInfo.h"

#include <QFile>
#include <QDir>
#include <QDebug>

#include <sys/utsname.h>

CpuInfo::CpuInfo()
    : m_Arch("unknow")
{
}
CpuInfo::~CpuInfo()
{
    // clear physicalcpu
    m_MapPhysicalCpu.clear();
}

bool CpuInfo::loadCpuInfo()
{
    // get arch
    readCpuArchitecture();

    // read /sys/devices/system/cpu
    readSysCpu();

    // read the file /proc/cpuinfo
    if (!readProcCpuinfo())
        return false;
    return true;
}

const QString& CpuInfo::arch() const
{
    return m_Arch;
}

void CpuInfo::logicalCpus(QString &info)
{
    foreach (int id, m_MapPhysicalCpu.keys()) {
        PhysicalCpu& physical = m_MapPhysicalCpu[id];
        physical.getInfo(info);
    }
}

int CpuInfo::physicalNum()
{
    if(m_MapPhysicalCpu.find(-1) == m_MapPhysicalCpu.end()){
        return m_MapPhysicalCpu.size();
    }
    else{
        return m_MapPhysicalCpu.size() - 1;
    }
}

int CpuInfo::coreNum()
{
    return 0;
}

int CpuInfo::logicalNum()
{
    return 0;
}

void CpuInfo::readCpuArchitecture()
{
    struct utsname utsbuf;
    if (uname(&utsbuf) == -1)
        return;
    m_Arch = QString::fromLocal8Bit(utsbuf.machine);
}

bool CpuInfo::readProcCpuinfo()
{
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QString cpuInfo = file.readAll();
    QStringList infos = cpuInfo.split("\n\n");
    foreach (const QString &info, infos) {
        if (info.isEmpty())
            continue;
        parseInfo(info);
    }

    file.close();
    return true;
}

bool CpuInfo::parseInfo(const QString &info)
{
    if (info.isEmpty())
        return false;
    // 解析段落信息，获取逻辑id号
    QMap<QString, QString> mapInfo;
    int logical_id = -1;
    QStringList lines = info.split("\n");
    foreach (const QString &line, lines) {
        if (line.isEmpty())
            continue;
        QStringList words = line.split(QRegExp("[\\s]*:[\\s]*"));
        if (words.size() != 2)
            continue;
        if(words[0] == "core"){
            mapInfo.insert("core id", words[1]);
        }else if(words[0] == "package"){
            mapInfo.insert("physical id", words[1]);
        }else{
            mapInfo.insert(words[0].toLower(), words[1]);
        }
        if (words[0].contains("processor"))
            logical_id = words[1].toInt();
    }

    if(logical_id < 0)
        return false;

    // 找到逻辑cpu
    if(mapInfo.find("physical id") != mapInfo.end()
            && mapInfo.find("core id") != mapInfo.end()){
        int physical_id = mapInfo["physical id"].toInt();
        if(m_MapPhysicalCpu.find(physical_id) == m_MapPhysicalCpu.end())
            return false;
        PhysicalCpu& physical = m_MapPhysicalCpu[physical_id];
        int core_id = mapInfo["core id"].toInt();
        if(!physical.coreIsExisted(core_id))
            return false;
        CoreCpu& core = physical.coreCpu(core_id);
        if(!core.logicalIsExisted(logical_id))
            return false;
        LogicalCpu& logical = core.logicalCpu(logical_id);
        if(logical.logicalID() >= 0)
            setProcCpuinfo(logical,mapInfo);
    }else{
        LogicalCpu& logical = logicalCpu(logical_id);
        if(logical.logicalID() >= 0)
            setProcCpuinfo(logical,mapInfo);
    }

    return true;
}

LogicalCpu& CpuInfo::logicalCpu(int logical_id)
{
    foreach(int physical_id,m_MapPhysicalCpu.keys()){
        PhysicalCpu physical = m_MapPhysicalCpu[physical_id];
        if(physical.logicalIsExisted(logical_id)){
            return physical.logicalCpu(logical_id);
        }
    }
    return m_MapPhysicalCpu[-1].logicalCpu(-1);
}

void CpuInfo::setProcCpuinfo(LogicalCpu& logical,const QMap<QString, QString>& mapInfo)
{
    logical.setFlags(mapInfo["flags"]);
    logical.setModel(mapInfo["model"]);
    logical.setModelName(mapInfo["model name"]);
    logical.setVendor(mapInfo["vendor_id"]);
    logical.setStepping(mapInfo["stepping"]);
    logical.setcpuFamily(mapInfo["cpu family"]);
    logical.setBogomips(mapInfo["bogomips"]);

    // diff in loognsoon and loongarch
    if (m_Arch == "mips64" || m_Arch == "loongarch64") {
        logical.setCurFreq(mapInfo["cpu mhz"]);
        logical.setModel(mapInfo["cpu model"]);
    }
}

void CpuInfo::readSysCpu()
{
    // /sys/devices/system/cpu/cpu*
    QDir dir("/sys/devices/system/cpu");
    dir.setFilter(QDir::Dirs);
    QRegExp reg("cpu([0-9]{1,4})");
    foreach (const QFileInfo &info, dir.entryInfoList()) {
        const QString &name = info.fileName();
        if (! reg.exactMatch(name))
            continue;
        readSysCpuN(reg.cap(1).toInt(), info.filePath());
    }
}

void CpuInfo::readSysCpuN(int N, const QString &path)
{
    // 第一步先读取物理cpu
    // /sys/devices/system/cpu/cpu0/topology/physical_package_id
    QString physicalPath = path+"/topology/physical_package_id";
    int physical_id = readPhysicalID(physicalPath);
    if(physical_id < 0){
        return;
    }
    if(m_MapPhysicalCpu.find(physical_id)==m_MapPhysicalCpu.end()){
        PhysicalCpu physical = PhysicalCpu(physical_id);
        m_MapPhysicalCpu.insert(physical_id, physical);
    }

    // 第二步读取core id
    // /sys/devices/system/cpu/cpu0/topology/core_id
    QString corePath = path+"/topology/core_id";
    int core_id = readCoreID(corePath);
    if(core_id < 0){
        return;
    }
    PhysicalCpu& cpu = m_MapPhysicalCpu[physical_id];
    if(!cpu.coreIsExisted(core_id)){
        CoreCpu core = CoreCpu(core_id);
        cpu.addCoreCpu(core_id, core);
    }

    // 第三步读取逻辑cpu
    LogicalCpu lcpu;
    lcpu.setLogicalID(N);
    lcpu.setCoreID(core_id);
    lcpu.setPhysicalID(physical_id);
    lcpu.setArch(m_Arch);
    QDir dir(path);
    // get cpu cache
    if (dir.exists("cache"))
        readCpuCache(dir.filePath("cache"), lcpu);
    // get cpu freq
    if (dir.exists("cpufreq"))
        readCpuFreq(dir.filePath("cpufreq"), lcpu);
    CoreCpu& corecpu = cpu.coreCpu(core_id);
    corecpu.addLogicalCpu(N,lcpu);
}

int CpuInfo::readPhysicalID(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return -1;
    QString info = file.readAll();
    file.close();
    return info.toInt();
}

int CpuInfo::readCoreID(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)){
        return -1;
    }
    QString info = file.readAll();

    file.close();
    return info.toInt();
}

void CpuInfo::readCpuCache(const QString &path, LogicalCpu &lcpu)
{
    QDir dir(path);
    dir.setFilter(QDir::Dirs);
    foreach (const QFileInfo &fileInfo, dir.entryInfoList()) {
        QString tpath = fileInfo.absoluteFilePath();
        if (!tpath.contains(QRegExp("index[0-9]")))
            continue;
        readCpuCacheIndex(tpath, lcpu);
    }
}

void CpuInfo::readCpuCacheIndex(const QString &path, LogicalCpu &lcpu)
{
    int level = -1;
    QString type, value;

    // get level
    QString levelPath = path + "/level";
    QFile file(levelPath);
    if (file.open(QIODevice::ReadOnly)) {
        QString info = file.readAll();
        level = info.toInt();
    }
    file.close();

    // get type
    QString typePath = path + "/type";
    QFile fileT(typePath);
    if (fileT.open(QIODevice::ReadOnly)) {
        type = fileT.readAll();
    }
    fileT.close();

    // get size
    QString valuePath = path + "/size";
    QFile fileV(valuePath);
    if (fileV.open(QIODevice::ReadOnly)) {
        value = fileV.readAll();
    }
    fileV.close();

    if (level == 2) {
        lcpu.setL2Cache(value);
    } else if (level == 3) {
        lcpu.setL3Cache(value);
    } else if (level == 1) {
        if (type.contains("Data", Qt::CaseInsensitive))
            lcpu.setL1dCache(value);
        else
            lcpu.setL1iCache(value);
    }
}

void CpuInfo::readCpuFreq(const QString &path, LogicalCpu &lcpu)
{
    // min freq
    QString minFreqPath = path + "/cpuinfo_min_freq";
    QFile minFile(minFreqPath);
    if (minFile.open(QIODevice::ReadOnly)) {
        QString info = minFile.readAll();
        int value = info.toInt() / 1000;
        lcpu.setMinFreq(QString::number(value) + "MHz");
    }
    minFile.close();

    // cur freq
    QString curFreqPath = path + "/scaling_cur_freq";
    QFile curFile(curFreqPath);
    if (curFile.open(QIODevice::ReadOnly)) {
        QString info = curFile.readAll();
        int value = info.toInt() / 1000;
        lcpu.setCurFreq(QString::number(value) + "MHz");
    }
    curFile.close();

    // maxFreq
    QString maxFreqPath = path + "/cpuinfo_max_freq";
    QFile maxFile(maxFreqPath);
    if (maxFile.open(QIODevice::ReadOnly)) {
        QString info = maxFile.readAll();
        int value = info.toInt() / 1000;
        lcpu.setMaxFreq(QString::number(value) + "MHz");
    }
    maxFile.close();
}
