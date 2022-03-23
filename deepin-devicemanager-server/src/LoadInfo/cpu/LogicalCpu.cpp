#include "LogicalCpu.h"

LogicalCpu::LogicalCpu()
    : d_ptr(new LogicalCpuPrivate(this))
{

}
void LogicalCpu::setPhysicalID(int value)
{
    Q_D(LogicalCpu);
    d->physicalID = value;
}
void LogicalCpu::setCoreID(int value)
{
    Q_D(LogicalCpu);
    d->coreID = value;
}
void LogicalCpu::setLogicalID(int value)
{
    Q_D(LogicalCpu);
    d->logicalID = value;
}
void LogicalCpu::setL1dCache(const QString &value)
{
    Q_D(LogicalCpu);
    d->l1d_cache = value;
}
void LogicalCpu::setL1iCache(const QString &value)
{
    Q_D(LogicalCpu);
    d->l1i_cache = value;
}
void LogicalCpu::setL2Cache(const QString &value)
{
    Q_D(LogicalCpu);
    d->l2_cache = value;
}
void LogicalCpu::setL3Cache(const QString &value)
{
    Q_D(LogicalCpu);
    d->l3_cache = value;
}
void LogicalCpu::setMinFreq(const QString &value)
{
    Q_D(LogicalCpu);
    d->min_freq = value;
}
void LogicalCpu::setCurFreq(const QString &value)
{
    Q_D(LogicalCpu);
    d->cur_freq = value;
}
void LogicalCpu::setMaxFreq(const QString &value)
{
    Q_D(LogicalCpu);
    d->max_freq = value;
}
void LogicalCpu::setModel(const QString &value)
{
    Q_D(LogicalCpu);
    d->model = value;
}
void LogicalCpu::setModelName(const QString &value)
{
    Q_D(LogicalCpu);
    d->modelName = value;
}
void LogicalCpu::setStepping(const QString &value)
{
    Q_D(LogicalCpu);
    d->setpping = value;
}
void LogicalCpu::setVendor(const QString &value)
{
    Q_D(LogicalCpu);
    d->vendor = value;
}
void LogicalCpu::setcpuFamily(const QString &value)
{
    Q_D(LogicalCpu);
    d->cpuFamily = value;
}
void LogicalCpu::setFlags(const QString &value)
{
    Q_D(LogicalCpu);
    d->flags = value;
}
void LogicalCpu::setBogomips(const QString &value)
{
    Q_D(LogicalCpu);
    d->bogoMips = value;
}

void LogicalCpu::setArch(const QString& value)
{
    Q_D(LogicalCpu);
    d->arch = value;
}

int LogicalCpu::physicalID()
{
    Q_D(LogicalCpu);
    return d->physicalID;
}
int LogicalCpu::coreID()
{
    Q_D(LogicalCpu);
    return d->coreID;
}
int LogicalCpu::logicalID()
{
    Q_D(LogicalCpu);
    return d->logicalID;
}
const QString &LogicalCpu::l1dCache()
{
    Q_D(LogicalCpu);
    return d->l1d_cache;
}
const QString &LogicalCpu::l1iCache()
{
    Q_D(LogicalCpu);
    return d->l1i_cache;
}
const QString &LogicalCpu::l2Cache()
{
    Q_D(LogicalCpu);
    return d->l2_cache;
}
const QString &LogicalCpu::l3Cache()
{
    Q_D(LogicalCpu);
    return d->l3_cache;
}
const QString &LogicalCpu::minFreq()
{
    Q_D(LogicalCpu);
    return d->min_freq;
}
const QString &LogicalCpu::curFreq()
{
    Q_D(LogicalCpu);
    return d->cur_freq;
}
const QString &LogicalCpu::maxFreq()
{
    Q_D(LogicalCpu);
    return d->max_freq;
}
const QString &LogicalCpu::model()
{
    Q_D(LogicalCpu);
    return d->model;
}
const QString &LogicalCpu::modelName()
{
    Q_D(LogicalCpu);
    return d->modelName;
}
const QString &LogicalCpu::stepping()
{
    Q_D(LogicalCpu);
    return d->setpping;
}
const QString &LogicalCpu::vendor()
{
    Q_D(LogicalCpu);
    return d->vendor;
}
const QString &LogicalCpu::cpuFamliy()
{
    Q_D(LogicalCpu);
    return d->cpuFamily;
}
const QString &LogicalCpu::flags()
{
    Q_D(LogicalCpu);
    return d->flags;
}
const QString &LogicalCpu::bogomips()
{
    Q_D(LogicalCpu);
    return d->bogoMips;
}
const QString &LogicalCpu::arch()
{
    Q_D(LogicalCpu);
    return d->arch;
}

