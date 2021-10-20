#include "PhysicalCpu.h"
#include "CoreCpu.h"
#include "LogicalCpu.h"

#include <QDebug>

PhysicalCpu::PhysicalCpu()
    : m_PhysicalCpu(-1)
{

}

PhysicalCpu::PhysicalCpu(int id)
    : m_PhysicalCpu(id)
{

}

void PhysicalCpu::addCoreCpu(int id, const CoreCpu& cpu)
{
    if(m_MapCoreCpu.find(id) == m_MapCoreCpu.end()){
        m_MapCoreCpu.insert(id,cpu);
    }
}

bool PhysicalCpu::coreIsExisted(int id)
{
    return m_MapCoreCpu.find(id) != m_MapCoreCpu.end();
}

CoreCpu& PhysicalCpu::coreCpu(int id)
{
    return m_MapCoreCpu[id];
}

bool PhysicalCpu::logicalIsExisted(int id)
{
    foreach(int i, m_MapCoreCpu.keys()){
        return m_MapCoreCpu[i].logicalIsExisted(id);
    }
    return false;
}

LogicalCpu& PhysicalCpu::logicalCpu(int id)
{
    foreach(int i, m_MapCoreCpu.keys()){
        if(m_MapCoreCpu[i].logicalIsExisted(id))
            return m_MapCoreCpu[i].logicalCpu(id);
    }
    return m_MapCoreCpu[-1].logicalCpu(-1);
}

void PhysicalCpu::getInfo(QString& info)
{
    foreach(int i, m_MapCoreCpu.keys()){
        if(m_MapCoreCpu[i].coreId() >=0 )
            m_MapCoreCpu[i].getInfo(info);
    }
}

int PhysicalCpu::coreNum()
{
    if(m_MapCoreCpu.find(-1) == m_MapCoreCpu.end()){
        return m_MapCoreCpu.size();
    }
    else{
        return m_MapCoreCpu.size() - 1;
    }
}

int PhysicalCpu::logicalNum()
{
    int num = 0;
    foreach (int id, m_MapCoreCpu.keys()) {
        if(id < 0)
            continue;
        CoreCpu& core = m_MapCoreCpu[id];
        num += core.logicalNum();
    }
    return num;
}
