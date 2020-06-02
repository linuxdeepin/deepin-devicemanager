#ifndef KLUGENERATOR_H
#define KLUGENERATOR_H

#include"DeviceGenerator.h"
class KLUGenerator : public DeviceGenerator
{
public:
    KLUGenerator();

protected:
    /**@brief:generator gpu info*/
    virtual void generatorGpuDevice();
};

#endif // KLUGENERATOR_H
