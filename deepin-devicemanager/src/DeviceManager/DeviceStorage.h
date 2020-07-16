#ifndef DEVICESTORAGE_H
#define DEVICESTORAGE_H
#include"DeviceInfo.h"

class DeviceStorage: public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceStorage)
public:
    DeviceStorage();

    /**
       * @brief setHwinfoInfo:设置由hwinfo --disk命令获取的设备信息
       * @param mapInfo:由hwinfo获取的信息map
       * @return 布尔值，true:信息设置成功；false:信息设置失败
       */
    bool setHwinfoInfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setKLUHwinfoInfo:设置由hwinfo --disk命令获取的设备信息，KLU专用
       * @param mapInfo:由hwinfo获取的信息map
       * @return 尔值，true:信息设置成功；false:信息设置失败
       */
    bool setKLUHwinfoInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setMediaType:设置存储设备的介质类型
     * @param name:存储设备逻辑名称
     * @param value:类型值0/1
     * @return 布尔值:true-设置成功；false--设置失败
     */
    bool setMediaType(const QString &name, const QString &value);

    /**
     * @brief setKLUMediaType:设置存储设备的介质类型,KLU专用
     * @param name:存储设备逻辑名称
     * @param value:类型值0/1
     * @return 布尔值:true-设置成功；false--设置失败
     */
    bool setKLUMediaType(const QString &name, const QString &value);

    /**
     * @brief addInfoFromlshw:将lshw获取的信息与存储设备进行匹配
     * @param mapInfo:由lshw获取的信息map
     * @return 布尔值:true-设置成功；false--设置失败
     */
    bool addInfoFromlshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief addInfoFromSmartctl:将sudo smartctl --all /dev/xxx中获取的信息与存储设备进行匹配
     * @param name:存储设备逻辑名称
     * @param mapInfo:由smartctl获取的信息map
     * @return 布尔值:true-设置成功；false--设置失败
     */
    bool addInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo);

    /**
     * @brief getInfoFromLshw:设置由lshw获取的信息
     * @param mapInfo:lshw获取的信息map
     */
    void getInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief getInfoFromsmartctl:设置由smartctl获取的信息
     * @param mapInfo:smartctl获取的信息map
     */
    void getInfoFromsmartctl(const QMap<QString, QString> &mapInfo);

    /**
     * @brief isValid:判断设备信息是否有效
     * @return 布尔值:true-设备信息有效；false--设备信息无效
     */
    bool isValid();

    /**
     * @brief setDiskSerialID:设置序列号属性 from Device Files
     * @param deviceFiles:hwinfo --disk中获取的device Files属性值
     */
    void setDiskSerialID(const QString &deviceFiles);

    /**
     * @brief compareSize:比较nvme提供的两个容量值的大小
     * @param size1:Total NVM Capacity
     * @param size2:Namespace 1 Size/Capacity
     * @return 返回较大的容量值
     */
    QString compareSize(const QString &size1, const QString &size2);

    /**
       * @brief model:获取型号属性值
       * @param 无
       * @return QString:型号属性值
       */
    const QString &model()const;

    /**
       * @brief vendor:获取制造商属性值
       * @param 无
       * @return QString:制造商属性值
       */
    const QString &vendor()const;

    /**
       * @brief mediaType:获取介质类型属性值
       * @param 无
       * @return QString:mediaType属性值
       */
    const QString &mediaType()const;

    /**
       * @brief size:获取容量属性值
       * @param 无
       * @return QString:容量属性值
       */
    const QString &size()const;

    /**
       * @brief rotationRate:获取转速属性值
       * @param 无
       * @return QString:转速属性值
       */
    const QString &rotationRate()const;

    /**
       * @brief interface:获取接口属性值
       * @param 无
       * @return QString:接口属性值
       */
    const QString &interface()const;

    /**
       * @brief serialNumber:获取序列号属性值
       * @param 无
       * @return QString:序列号属性值
       */
    const QString &serialNumber()const;

    /**
       * @brief version:获取版本号属性值
       * @param 无
       * @return QString:版本号属性值
       */
    const QString &version()const;

    /**
       * @brief capabilities:获取功能属性值
       * @param 无
       * @return QString:功能属性值
       */
    const QString &capabilities()const;

    /**
       * @brief description:获取描述属性值
       * @param 无
       * @return QString:描述属性值
       */
    const QString &description()const;

    /**
       * @brief keyFromStorage:获取存储设备标识符
       * @param 无
       * @return QString:存储设备标识符
       */
    const QString &keyFromStorage()const;

    /**
       * @brief firmwareVersion:获取固件版本属性值
       * @param 无
       * @return QString:固件版本属性值
       */
    const QString &firmwareVersion()const;

    /**
       * @brief speed:获取速率属性值
       * @param 无
       * @return QString:速率属性值
       */
    const QString &speed()const;

    /**
       * @brief powerOnHours:获取通电时长属性值
       * @param 无
       * @return QString:通电时长属性值
       */
    const QString &powerOnHours()const;

    /**
       * @brief powerCycleCount:获取通电次数属性值
       * @param 无
       * @return QString:通电次数属性值
       */
    const QString &powerCycleCount()const;

protected:

    /**
       * @brief initFilterKey:初始化可现实的可显示的属性,m_FilterKey
       */
    void initFilterKey() override;

    /**
     * @brief loadBaseDeviceInfo:加载基本信息
     */
    void loadBaseDeviceInfo() override;

    /**
     * @brief loadOtherDeviceInfo:加载基本信息
     */
    void loadOtherDeviceInfo() override;

    /**
     * @brief loadTableHeader : 过去表格的表头数据
     */
    void loadTableHeader() override;

    /**
     * @brief loadTableData:加载表头信息
     */
    void loadTableData() override;

private:
    QString               m_Model;              //<! 【型号】1
    QString               m_Vendor;             //<! 【制造商】2
    QString               m_MediaType;          //<! 【介质类型】3
    QString               m_Size;               //<! 【大小】4
    QString               m_RotationRate;       //<! 【转速】
    QString               m_Interface;          //<! 【接口】6
    QString               m_SerialNumber;       //<! 【序列号】7
    QString               m_Version;            //<! 【版本】
    QString               m_Capabilities;       //<! 【功能】
    QString               m_Description;        //<! 【描述】
    QString               m_PowerOnHours;       //<! 【通电时间】9
    QString               m_PowerCycleCount;    //<! 【通电次数】10
    QString               m_FirmwareVersion;    //<! 【固件版本】8
    QString               m_Speed;              //<! 【速度】5

    QString               m_DeviceFile;         //<
    QString               m_KeyToLshw;          //<!  用于表示存储文件
    QString               m_KeyFromStorage;     //<!  有些其他设备与usb重复，需要用这个进行唯一标识
};

#endif // DEVICESTORAGE_H
