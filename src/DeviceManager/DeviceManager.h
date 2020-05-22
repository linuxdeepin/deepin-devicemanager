#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H
#include<QList>
class DeviceMouse;
class DeviceCpu;
class DeviceStorage;
class DeviceGpu;
class DeviceMemory;
class DeviceMonitor;
class DeviceBios;
class DeviceBluetooth;
class DeviceAudio;
class DeviceNetwork;
class DeviceImage;
class DeviceKeyboard;
class DeviceOthers;
class DevicePower;
class DevicePrint;
class DeviceOtherPCI;
class DeviceComputer;
class DeviceCdrom;

/*管理设备信息的单例*/

class DeviceManager
{
public:
    static DeviceManager *instance()
    {
        if (!sInstance) {
            sInstance = new DeviceManager;
        }

        return sInstance;
    }

    /**@brief:清楚所有设备信息*/
    void clear();


    // 鼠标设备相关 **************************************************************************************
    /*@brief:添加鼠标设备信息*/
    void addMouseDevice(const DeviceMouse &device);
    /*@brief:获取鼠标设备信息*/
    const QList<DeviceMouse> &getMouseDevices();
    /*@brief:将从sudo hwinfo --mouse获取的信息添加到设备里面*/
    void setCatDevicesIntoMouseDevice(const QString &info);
    /*@brief:将从sudo lshw -C input获取的信息添加到设备里面*/
    bool setLshwinfoIntoMouseDevice(const QString &info);


    // CPU设备相关 **************************************************************************************
    void addCpuDevice(const DeviceCpu &device);
    // 获取CPU设备
    const QList<DeviceCpu> &getCPUDevices();

    // 存储设备相关 **************************************************************************************
    void addStorageDeivce(const DeviceStorage &device);
    void addLshwinfoIntoStorageDevice(const QString &info);
    void addLshwinfoIntoStorageDevice(const QMap<QString, QString> &mapInfo);
    const QList<DeviceStorage> &getStorageDevices();
    bool setStorageDeviceMediaType(const QString &name, const QString &value);
    void setStorageDeviceAttribute(const QString &);
    void addsmartctlinfoIntoStorageDevice(const QString &info);
    void setStorageInfoFromSmartctl(const QString &name, const QString &info);
    void setStorageInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo);

    // GPU设备相关 **************************************************************************************
    void addGpuDevice(const DeviceGpu &device);
    const QList<DeviceGpu> &getGPUDevices();
    /**@brief:设置从lshw获取的gup信息，内部会做判断是否是该设备*/
    void setGpuInfoFromLshw(const QString &info);
    void setGpuInfoFromLshw(const QMap<QString, QString> &mapInfo);
    void setGpuInfoFromXrandr(const QString &info);
    void setGpuInfoFromXrandr(const QMap<QString, QString> &mapInfo);
    void setGpuSizeFromDmesg(const QString &info);

    // 内存设备相关 *************************************************************************************
    void addMemoryDevice(const DeviceMemory &device);
    const QList<DeviceMemory> &getMemoryDevices();
    void setMemoryInfoFromDmidecode(const QString &info);
    void setMemoryInfoFromDmidecode(const QMap<QString, QString> &mapInfo);

    // 显示设备相关 *************************************************************************************
    void addMonitor(const DeviceMonitor &device);
    const QList<DeviceMonitor> &getMonitorDevices();
    void setMonitorInfoFromXrandr(const QString &main, const QString &edid);
    void setMonitorRefreshRate(const QString &main, const QString &rate);
    void setCurrentResolution(const QString &resolution, const QString &rate);


    // Bios设备相关 ************************************************************************************
    void addBiosDevice(const DeviceBios &device);
    const QList<DeviceBios> &getBiosDevices();

    // 蓝牙设备相关
    void addBluetoothDevice(const DeviceBluetooth &device);
    const QList<DeviceBluetooth> &getBluetoothDevices();
    void setBluetoothInfoFromLshw(const QString &info);
    void setBluetoothInfoFromLshw(const QMap<QString, QString> &mapInfo);
    void setBluetoothInfoFromHwinfo(const QString &info);
    void setBluetoothInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    // 音频设备相关
    void addAudioDevice(const DeviceAudio &device);
    const QList<DeviceAudio> &getAudioDevices();
    void setAudioInfoFromLshw(const QString &info);
    void setAudioInfoFromLshw(const QMap<QString, QString> &mapInfo);

    // 网络设备相关
    void addNetworkDevice(const DeviceNetwork &device);
    const QList<DeviceNetwork> &getNetworkDevices();
    void setNetworkInfoFromHwinfo(const QString &info);

    // 图像设备相关
    void addImageDevice(const DeviceImage &device);
    const QList<DeviceImage> &getImageDevices();
    void setCameraInfoFromLshw(const QString &info);

    // 键盘设备相关
    void addKeyboardDevice(const DeviceKeyboard &device);
    const QList<DeviceKeyboard> &getKeyboardDevices();
    void setKeyboardInfoFromCatDevices(const QString &info);
    void setKeyboardInfoFromLshw(const QString &info);

    // 其它设备相关
    void addOthersDevice(const DeviceOthers &device);
    void addOthersDeviceFromHwinfo(const DeviceOthers &device);
    const QList<DeviceOthers> &getOthersDevices();

    // 电源设备相关
    void addPowerDevice(const DevicePower &device);
    const QList<DevicePower> &getPowerDevices();
    void setPowerDaemonInfo(const QString &info);

    // 打印机设备相关
    void addPrintDevice(const DevicePrint &device);
    const QList<DevicePrint> &getPrintDevices();
    void setPrinterInfoFromHwinfo(const QString &info);

    // 其它pci设备相关
    void addOtherPCIDevice(const DeviceOtherPCI &device);
    const QList<DeviceOtherPCI> &getOtherPCIDevices();

    // 计算机概况相关
    void addComputerDevice(const DeviceComputer &device);
    const QList<DeviceComputer> &getComputerDevices();
    void setComputerBoardInfoFromCatBoardinfo(const QString &info);
    void setComputerUOSInfoFromDSysInfo(const QString &info);
    void setComputerInfoFromCatVersion(const QString &info);
    void setComputerChassisInfoFromDmidecode(const QString &info);
    void setComputerInfoFromLshw(const QString &info);

    // 获取cdrom设备
    void addCdromDevice(const DeviceCdrom &device);
    const QList<DeviceCdrom> getCdromDevices();
    void setCdromInfoFromLshw(const QString &info);

    // 其他设备属性的翻译
    void addField(const QString &key, const QString &address);
    void generateTs();


protected:
    DeviceManager();

private:
    static DeviceManager    *sInstance;

    QList<DeviceMouse>              m_ListDeviceMouse;                //<! 鼠标设备
    QList<DeviceCpu>                m_ListDeviceCPU;                  //<! cpu设备
    QList<DeviceStorage>            m_ListDeviceStorage;              //<! 存储设备
    QList<DeviceGpu>                m_ListDeviceGPU;                  //<! gpu设备
    QList<DeviceMemory>             m_ListDeviceMemory;               //<! memory设备
    QList<DeviceMonitor>            m_ListDeviceMonitor;              //<! monitor设备
    QList<DeviceBios>               m_ListDeviceBios;                 //<! bios设备
    QList<DeviceBluetooth>          m_ListBluetooth;                  //<! 蓝牙设备
    QList<DeviceAudio>              m_ListDeviceAudio;                //<! 音频设备
    QList<DeviceNetwork>            m_ListDeviceNetwork;              //<! 网络设备
    QList<DeviceImage>              m_ListDeviceImage;                //<! 图像设备
    QList<DeviceKeyboard>           m_ListDeviceKeyboard;             //<! 键盘设备
    QList<DeviceOthers>             m_ListDeviceOthers;               //<! 其它设备
    QList<DevicePower>              m_ListDevicePower;                //<! 电源
    QList<DevicePrint>              m_ListDevicePrint;                //<! 打印机
    QList<DeviceOtherPCI>           m_ListDeviceOtherPCI;             //<! 其它PCI设备
    QList<DeviceComputer>           m_ListDeviceComputer;             //<! 计算机基本信息
    QList<DeviceCdrom>              m_ListDeviceCdrom;                //<! cdrom设备

    QMap<QString, QSet<QString> >                   m_TsField;
};

#endif // DEVICEMANAGER_H
