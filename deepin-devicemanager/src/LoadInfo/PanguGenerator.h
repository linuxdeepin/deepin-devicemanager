#ifndef PANGUGENERATOR_H
#define PANGUGENERATOR_H
#include"DeviceGenerator.h"
class PanguGenerator : public DeviceGenerator
{
public:
    PanguGenerator();

    /**@brief:generator computer info*/
    virtual void generatorComputerDevice();
};

#endif // PANGUGENERATOR_H
