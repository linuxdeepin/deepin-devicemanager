#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H
#include<QList>
#include<QMap>
#include<QMutex>
#include<QDomDocument>
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
class DeviceBaseInfo;

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
    void addMouseDevice(const DeviceMouse &device);
    const QList<DeviceMouse> &getMouseDevices();
    bool addMouseInfoFromLshw(const QMap<QString, QString> &mapInfo);


    // CPU设备相关 **************************************************************************************
    void addCpuDevice(const DeviceCpu &device);
    const QList<DeviceCpu> &getCPUDevices();

    // 存储设备相关 **************************************************************************************
    void addStorageDeivce(const DeviceStorage &device);
    void addLshwinfoIntoStorageDevice(const QMap<QString, QString> &mapInfo);
    const QList<DeviceStorage> &getStorageDevices();
    bool setStorageDeviceMediaType(const QString &name, const QString &value);
    bool setKLUStorageDeviceMediaType(const QString &name, const QString &value);
    void setStorageInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo);

    // GPU设备相关 **************************************************************************************
    void addGpuDevice(const DeviceGpu &device);
    const QList<DeviceGpu> &getGPUDevices();
    /**@brief:设置从lshw获取的gup信息，内部会做判断是否是该设备*/
    void setGpuInfoFromLshw(const QMap<QString, QString> &mapInfo);
    void setGpuInfoFromXrandr(const QMap<QString, QString> &mapInfo);
    void setGpuSizeFromDmesg(const QString &info);

    // 内存设备相关 *************************************************************************************
    void addMemoryDevice(const DeviceMemory &device);
    const QList<DeviceMemory> &getMemoryDevices();
    void setMemoryInfoFromDmidecode(const QMap<QString, QString> &mapInfo);

    // 显示设备相关 *************************************************************************************
    void addMonitor(const DeviceMonitor &device);
    const QList<DeviceMonitor> &getMonitorDevices();
    void setMonitorInfoFromXrandr(const QString &main, const QString &edid);
    void setCurrentResolution(const QString &resolution, const QString &rate);


    // Bios设备相关 ************************************************************************************
    void addBiosDevice(const DeviceBios &device);
    const QList<DeviceBios> &getBiosDevices();
    void setLanguageInfo(const QMap<QString, QString> &mapInfo);

    // 蓝牙设备相关
    void addBluetoothDevice(const DeviceBluetooth &device);
    const QList<DeviceBluetooth> &getBluetoothDevices();
    void setBluetoothInfoFromLshw(const QMap<QString, QString> &mapInfo);
    bool setBluetoothInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    // 音频设备相关
    void addAudioDevice(const DeviceAudio &device);
    const QList<DeviceAudio> &getAudioDevices();
    void setAudioInfoFromLshw(const QMap<QString, QString> &mapInfo);
    void setAudioChipFromDmesg(const QString &info);

    // 网络设备相关
    void addNetworkDevice(const DeviceNetwork &device);
    const QList<DeviceNetwork> &getNetworkDevices();
    void setNetworkInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    // 图像设备相关
    void addImageDevice(const DeviceImage &device);
    const QList<DeviceImage> &getImageDevices();
    void setCameraInfoFromLshw(const QMap<QString, QString> &mapInfo);

    // 键盘设备相关
    void addKeyboardDevice(const DeviceKeyboard &device);
    const QList<DeviceKeyboard> &getKeyboardDevices();
    void setKeyboardInfoFromLshw(const QMap<QString, QString> &mapInfo);

    // 其它设备相关
    void addOthersDevice(const DeviceOthers &device);
    void addOthersDeviceFromHwinfo(const DeviceOthers &device);
    const QList<DeviceOthers> &getOthersDevices();
    void setOthersDeviceInfoFromLshw(const QMap<QString, QString> &mapInfo);

    // 电源设备相关
    void addPowerDevice(const DevicePower &device);
    const QList<DevicePower> &getPowerDevices();
    void setPowerDaemonInfo(const QMap<QString, QString> &mapInfo);

    // 打印机设备相关
    void addPrintDevice(const DevicePrint &device);
    const QList<DevicePrint> &getPrintDevices();

    // 其它pci设备相关
    void addOtherPCIDevice(const DeviceOtherPCI &device);
    const QList<DeviceOtherPCI> &getOtherPCIDevices();

    // 计算机概况相关
    void addComputerDevice(const DeviceComputer &device);
    const QList<DeviceComputer> &getComputerDevices();

    // 获取cdrom设备
    void addCdromDevice(const DeviceCdrom &device);
    const QList<DeviceCdrom> getCdromDevices();
    void addLshwinfoIntoCdromDevice(const QMap<QString, QString> &mapInfo);

    void addBusId(const QStringList &busId);
    const QStringList &getBusId();

    void addCmdInfo(const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo);
    const QList<QMap<QString, QString>> &cmdInfo(const QString &key);


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

    QStringList                     m_BusIdList;
    QMap<QString, QList<QMap<QString, QString> > > m_cmdInfo;
};

#endif // DEVICEMANAGER_H
