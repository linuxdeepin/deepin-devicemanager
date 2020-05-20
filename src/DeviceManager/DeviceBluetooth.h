#ifndef DEVICEBLUETOOTH_H
#define DEVICEBLUETOOTH_H
#include"DeviceInfo.h"

class DeviceBluetooth : public DeviceBaseInfo
{
public:
    DeviceBluetooth();

    /**设置从hciconfig里面读取的内容*/
    void setInfoFromHciconfig(const QString &info);

    /**设置从hwinfo里面读取的内容*/
    bool setInfoFromHwinfo(const QString &info);
    bool setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**设置从lshw里面获取的内容*/
    bool setInfoFromLshw(const QString &info);



    const QString &name()const;
    const QString &vendor()const;
    const QString &version()const;
    const QString &model()const;
    const QString &MAC()const;
    const QString &logicalName()const;
    const QString &busInfo()const;
    const QString &capabilities()const;
    const QString &driver()const;
    const QString &driverVersion()const;
    const QString &maximumPower()const;
    const QString &speed()const;

protected:
    void initFilterKey() override;

private:
    /**@brief:获取映射到lshw的key*/
    void parseKeyToLshw(const QString &info);
    void getMapInfoFromHciconfig(QMap<QString, QString> &mapInfo, const QString &info);

private:
    QString         m_Name;                   //<! 【名称】
    QString         m_Vendor;                 //<! 【制造商】
    QString         m_Version;                //<! 【版本】
    QString         m_Model;                  //<! 【型号】
    QString         m_MAC;                    //<! 【物理地址】
    QString         m_LogicalName;            //<! 【逻辑名称】
    QString         m_BusInfo;                //<! 【总线信息】
    QString         m_Capabilities;           //<! 【功能】
    QString         m_Driver;                 //<! 【驱动】
    QString         m_DriverVersion;          //<! 【驱动版本】
    QString         m_MaximumPower;           //<! 【最大功耗】
    QString         m_Speed;                  //<! 【速度】

    QString         m_UniqueKey;              //<!  鼠标设备的唯一标识
};

#endif // DEVICEBLUETOOTH_H
