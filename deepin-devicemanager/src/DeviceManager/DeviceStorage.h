#ifndef DEVICESTORAGE_H
#define DEVICESTORAGE_H
#include"DeviceInfo.h"

class DeviceStorage: public DeviceBaseInfo
{
public:
    DeviceStorage();

    /**@brief:解析从 sudo hwinfo --disk 中获取的消息*/
    bool setHwinfoInfo(const QMap<QString, QString> &mapInfo);
    /**@brief:解析从 sudo hwinfo --disk 中获取的消息,KLU专用*/
    bool setKLUHwinfoInfo(const QMap<QString, QString> &mapInfo);
    /**@brief:设置存储设备的介质类型*/
    bool setMediaType(const QString &name, const QString &value);
    /**@brief:设置存储设备的介质类型,KLU专用*/
    bool setKLUMediaType(const QString &name, const QString &value);

    /**@brief:解析从 sudo lshw -C disk 中获取的消息*/
    bool addInfoFromlshw(const QMap<QString, QString> &mapInfo);

    /**@brief:解析从 sudo smartctl --all /dev/sda中获取的消息*/
    bool addInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo);

    /**@brief:从sudo lshw -C disk 中获取信息*/
    void getInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**@brief:从sudo smartctl --all /dev/sda 中获取信息*/
    void getInfoFromsmartctl(const QMap<QString, QString> &mapInfo);

    /**@brief:判断设备信息是否有效*/
    bool isValid();

    /**@brief:设置序列号属性 from Device Files*/
    void setDiskSerialID(const QString &deviceFiles);

    /**@brief:比较nvme提供的两个容量值的大小*/
    QString compareSize(const QString &size1, const QString &size2);

    /**@brief:获取各种设备属性*/
    const QString &model()const;
    const QString &vendor()const;
    const QString &mediaType()const;
    const QString &size()const;
    const QString &rotationRate()const;
    const QString &interface()const;
    const QString &serialNumber()const;
    const QString &version()const;
    const QString &capabilities()const;
    const QString &description()const;
    const QString &keyFromStorage()const;
    const QString &firmwareVersion()const;
    const QString &speed()const;
    const QString &powerOnHours()const;
    const QString &powerCycleCount()const;
protected:
    void initFilterKey() override;

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
