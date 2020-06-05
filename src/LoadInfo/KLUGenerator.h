#ifndef KLUGENERATOR_H
#define KLUGENERATOR_H

#include"DeviceGenerator.h"
class KLUGenerator : public DeviceGenerator
{
public:
    KLUGenerator();

protected:
    /**@brief:generator computer info*/
    virtual void generatorComputerDevice();

    /**@brief:generator gpu info*/
    virtual void generatorGpuDevice();

    /**@brief:generator monitor info*/
    virtual void generatorMonitorDevice();

    /**@brief:generator audio info*/
    virtual void generatorAudioDevice();

protected:
    /**@brief:generator keyboard info from hwinfo*/
    virtual void getKeyboardInfoFromHwinfo();

    /**@brief:generator others info from hwinfo*/
    virtual void getOthersInfoFromHwinfo();

    /**@brief:generator disk info from hwinfo*/
    virtual void getDiskInfoFromHwinfo();
    virtual void getDiskInfoFromLsblk();

    /**@brief:generator audio info*/
    virtual void getAudioInfoFromCatAudio();
};

#endif // KLUGENERATOR_H
