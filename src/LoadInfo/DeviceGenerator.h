#ifndef DEVICEGENERATOR_H
#define DEVICEGENERATOR_H

#include <QObject>
#include <DApplication>
#include <DSysInfo>
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class DeviceGenerator
{
public:
    static void clear();

    DeviceGenerator();
    virtual ~DeviceGenerator();

    /**@brief:generator computer info*/
    virtual void generatorComputerDevice();

    /**@brief:generator cpu info*/
    virtual void generatorCpuDevice();

    /**@brief:generator bios info*/
    virtual void generatorBiosDevice();

    /**@brief:generator memory info*/
    virtual void generatorMemoryDevice();

    /**@brief:generator disk info*/
    virtual void generatorDiskDevice();

    /**@brief:generator gpu info*/
    virtual void generatorGpuDevice();

    /**@brief:generator monitor info*/
    virtual void generatorMonitorDevice();

    /**@brief:generator network info*/
    virtual void generatorNetworkDevice();

    /**@brief:generator audio info*/
    virtual void generatorAudioDevice();

    /**@brief:generator bluetooth info*/
    virtual void generatorBluetoothDevice();

    /**@brief:generator keyboard info*/
    virtual void generatorKeyboardDevice();

    /**@brief:generator mouse info*/
    virtual void generatorMouseDevice();

    /**@brief:generator printer info*/
    virtual void generatorPrinterDevice();

    /**@brief:generator camera info*/
    virtual void generatorCameraDevice();

    /**@brief:generator cdrom info*/
    virtual void generatorCdromDevice();

    /**@brief:generator others info*/
    virtual void generatorOthersDevice();

    /**@brief:generator power info*/
    virtual void generatorPowerDevice();


protected:
    /**@brief:generator bios info*/
    void getBiosInfo();
    void getSystemInfo();
    void getBaseBoardInfo();
    void getChassisInfo();
    void getBiosMemoryInfo();
    /**@brief:generator memory info*/
    void getMemoryInfoFromLshw();
    void getMemoryInfoFromDmidecode();
    /**@brief:generator disk info*/
    void getDiskInfoFromHwinfo();
    void getDiskInfoFromLshw();
    void getDiskInfoFromLsblk();
    void getDiskInfoFromSmartCtl();
    /**@brief:generator gpu info*/
    void getGpuInfoFromHwinfo();
    void getGpuInfoFromLshw();
    void getGpuInfoFromXrandr();
    void getGpuSizeFromDmesg();
    /**@brief:generator monitor info*/
    void getMonitorInfoFromHwinfo();
    void getMonitorInfoFromXrandrVerbose();
    void getMonitorRefreshRateFromXrandr();
    /**@brief:generator audio info*/
    void getAudioInfoFromHwinfo();
    void getAudioInfoFromLshw();
    /**@brief:generator bluetooth info*/
    void getBluetoothInfoFromHciconfig();
    void getBlueToothInfoFromHwinfo();
    void getBluetoothInfoFromLshw();
    /**@brief:generator keyboard info*/
    void getKeyboardInfoFromHwinfo();
    void getKeyboardInfoFromLshw();
    void getKeyboardInfoFromCatDevices();
    /**@brief:generator mouse info*/
    void getMouseInfoFromHwinfo();
    void getMouseInfoFromLshw();
    void getMouseInfoFromCatDevices();
    /**@brief:generator camera info*/
    void getImageInfoFromHwinfo();
    void getImageInfoFromLshw();
    /**@brief:generator cdrom info*/
    void getCdromInfoFromHwinfo();
    void getCdromInfoFromLshw();
    /**@brief:generator others info*/
    virtual void getOthersInfoFromHwinfo();
    void getOthersInfoFromLshw();

    /**@brief:get BusID unique from Hwinfo*/
    void getBusIDFromHwinfo(const QString &sysfsBusID);
protected:
    static QStringList m_ListBusID;
};

#endif // DEVICEGENERATOR_H
