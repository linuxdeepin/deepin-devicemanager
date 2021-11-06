#include "CoreCpu.h"

CoreCpu::CoreCpu(int id)
    : m_CoreId(id)
{

}

void CoreCpu::setCoreId(int id)
{
    m_CoreId = id;
}
