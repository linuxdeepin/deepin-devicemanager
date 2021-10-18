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

    qInfo() << "m_MapCoreCpu.size() *********** " << m_MapCoreCpu.size();
    foreach(int i, m_MapCoreCpu.keys()){
        m_MapCoreCpu[i].getInfo(info);
    }
}
