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

    /**@brief:generator monitor info*/
    virtual void generatorMonitorDevice();

protected:
    /**@brief:generator keyboard info from hwinfo*/
    virtual void getKeyboardInfoFromHwinfo();

    /**@brief:generator others info from hwinfo*/
    virtual void getOthersInfoFromHwinfo();

    /**@brief:generator disk info from hwinfo*/
    virtual void getDiskInfoFromHwinfo();
    virtual void getDiskInfoFromLsblk();
};

#endif // KLUGENERATOR_H
