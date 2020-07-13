#ifndef PANGUGENERATOR_H
#define PANGUGENERATOR_H
#include"DeviceGenerator.h"
class PanguGenerator : public DeviceGenerator
{
public:
    PanguGenerator();

    /**
     * @brief generatorComputerDevice:生成概况设备信息
     */
    virtual void generatorComputerDevice();
};

#endif // PANGUGENERATOR_H
