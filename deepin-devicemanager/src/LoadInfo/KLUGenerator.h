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

    /**@brief:generator power info*/
    virtual void generatorPowerDevice();

    /**@brief:generator memory info*/
    virtual void getMemoryInfoFromLshw();

    /**@brief:generator cpu info*/
    virtual void generatorCpuDevice();

    /**@brief:generator bluetooth info*/
    virtual void generatorBluetoothDevice();

    /**@brief:generator network info*/
    virtual void generatorNetworkDevice();

protected:
    /**@brief:generator keyboard info from hwinfo*/
    virtual void getKeyboardInfoFromHwinfo();

    /**@brief:generator others info from hwinfo*/
    virtual void getOthersInfoFromHwinfo();

    /**@brief:generator disk info from hwinfo*/
    virtual void getDiskInfoFromHwinfo();
    virtual void getDiskInfoFromLshw();
    virtual void getDiskInfoFromLsblk();
    virtual void getDiskInfoFromSmartCtl();

    /**@brief:generator audio info*/
    virtual void getAudioInfoFromCatAudio();

    /**@brief:generator bluetooth info*/
    virtual void getBluetoothInfoFromCatWifiInfo();

    /**@brief:generator network info*/
    virtual void getNetworkInfoFromCatWifiInfo();
};

#endif // KLUGENERATOR_H
