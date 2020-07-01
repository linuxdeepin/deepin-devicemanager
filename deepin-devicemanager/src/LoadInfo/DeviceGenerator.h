#ifndef DEVICEGENERATOR_H
#define DEVICEGENERATOR_H

#include <QObject>
#include <DApplication>
#include <DSysInfo>
#include <QMutex>
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class DeviceGenerator : public QObject
{
    Q_OBJECT
public:
    DeviceGenerator(QObject *parent = nullptr);
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

    /**@brief:get BusID unique from Hwinfo*/
    void addBusIDFromHwinfo(const QString &sysfsBusID);
    const QStringList &getBusIDFromHwinfo();

protected:
    /**@brief:generator bios info*/
    virtual void getBiosInfo();
    virtual void getSystemInfo();
    virtual void getBaseBoardInfo();
    virtual void getChassisInfo();
    virtual void getBiosMemoryInfo();
    /**@brief:generator memory info*/
    virtual void getMemoryInfoFromLshw();
    virtual void getMemoryInfoFromDmidecode();
    /**@brief:generator disk info*/
    virtual void getDiskInfoFromHwinfo();
    virtual void getDiskInfoFromLshw();
    virtual void getDiskInfoFromLsblk();
    virtual void getDiskInfoFromSmartCtl();
    /**@brief:generator gpu info*/
    virtual void getGpuInfoFromHwinfo();
    virtual void getGpuInfoFromLshw();
    virtual void getGpuInfoFromXrandr();
    virtual void getGpuSizeFromDmesg();
    /**@brief:generator monitor info*/
    virtual void getMonitorInfoFromHwinfo();
    virtual void getMonitorInfoFromXrandrVerbose();
    virtual void getMonitorRefreshRateFromXrandr();
    /**@brief:generator audio info*/
    virtual void getAudioInfoFromHwinfo();
    virtual void getAudioInfoFromLshw();
    virtual void getAudioInfoFromCatInput();
    virtual void getAudioChipInfoFromDmesg();
    /**@brief:generator bluetooth info*/
    virtual void getBluetoothInfoFromHciconfig();
    virtual void getBlueToothInfoFromHwinfo();
    virtual void getBluetoothInfoFromLshw();
    /**@brief:generator keyboard info*/
    virtual void getKeyboardInfoFromHwinfo();
    virtual void getKeyboardInfoFromLshw();
    virtual void getKeyboardInfoFromCatDevices();
    /**@brief:generator mouse info*/
    virtual void getMouseInfoFromHwinfo();
    virtual void getMouseInfoFromLshw();
    virtual void getMouseInfoFromCatDevices();
    /**@brief:generator camera info*/
    virtual void getImageInfoFromHwinfo();
    virtual void getImageInfoFromLshw();
    /**@brief:generator cdrom info*/
    virtual void getCdromInfoFromHwinfo();
    virtual void getCdromInfoFromLshw();
    /**@brief:generator others info*/
    virtual void getOthersInfoFromHwinfo();
    virtual void getOthersInfoFromLshw();
protected:
    QStringList m_ListBusID;
};

#endif // DEVICEGENERATOR_H
