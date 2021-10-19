#include "CoreCpu.h"
#include "LogicalCpu.h"

#include <QDebug>

CoreCpu::CoreCpu()
    : m_CoreId(-1)
{

}

CoreCpu::CoreCpu(int id)
    : m_CoreId(id)
{

}

void CoreCpu::setCoreId(int id)
{
    m_CoreId = id;
}

void CoreCpu::addLogicalCpu(int id, const LogicalCpu& lc)
{
    if(m_MapLogicalCpu.find(id) == m_MapLogicalCpu.end()){
        m_MapLogicalCpu.insert(id,lc);
    }
}

bool CoreCpu::logicalIsExisted(int id)
{
    return m_MapLogicalCpu.find(id) != m_MapLogicalCpu.end();
}

LogicalCpu& CoreCpu::logicalCpu(int id)
{
    return m_MapLogicalCpu[id];
}

void CoreCpu::getInfo(QString& info)
{
    foreach(int id,m_MapLogicalCpu.keys()){
        if(id < 0)
            continue;
        LogicalCpu& logical = m_MapLogicalCpu[id];
        appendKeyValue(info, "processor", logical.logicalID());
        appendKeyValue(info, "core id", logical.coreID());
        appendKeyValue(info, "physical id", logical.physicalID());
        appendKeyValue(info, "L1d cache", logical.l1dCache());
        appendKeyValue(info, "L1i cache", logical.l1iCache());
        appendKeyValue(info, "L2 cache", logical.l2Cache());
        appendKeyValue(info, "L3 cache", logical.l3Cache());
        appendKeyValue(info, "CPU MHz", logical.curFreq());
        appendKeyValue(info, "CPU max MHz", logical.maxFreq());
        appendKeyValue(info, "CPU min MHz", logical.minFreq());
        appendKeyValue(info, "flags", logical.flags());
        appendKeyValue(info, "model", logical.model());
        appendKeyValue(info, "model name", logical.modelName());
        appendKeyValue(info, "vendor_id", logical.vendor());
        appendKeyValue(info, "stepping", logical.stepping());
        appendKeyValue(info, "cpu family", logical.cpuFamliy());
        appendKeyValue(info, "bogomips", logical.bogomips());
        appendKeyValue(info, "Architecture", logical.arch());
        info += QString("\n");
    }
}

void CoreCpu::appendKeyValue(QString &info, const QString &key, const QString &value)
{
    if (value.isEmpty())
        return;
    QString v = value;
    info += QString("%1 : %2\n").arg(key).arg(v.trimmed());
}

void CoreCpu::appendKeyValue(QString &info, const QString &key, int value)
{
    info += QString("%1 : %2\n").arg(key).arg(value);
}

int CoreCpu::coreId()
{
    return m_CoreId;
}
