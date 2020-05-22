#ifndef DEVICENETWORK_H
#define DEVICENETWORK_H
#include"DeviceInfo.h"


class DeviceNetwork : public DeviceBaseInfo
{
public:
    DeviceNetwork();

    // 设置从 lshw 获取的信息
    void setInfoFromLshw(const QString &info);
    void setInfoFromLshw(const QMap<QString, QString> &mapInfo);
    // 设置从 hwinfo 获取的信息
    bool setInfoFromHwinfo(const QString &info);
    bool setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    // 获取设备属性
    const QString &name()const;
    const QString &vendor()const;
    const QString &model()const;
    const QString &version()const;
    const QString &busInfo()const;
    const QString &logicalName()const;
    const QString &MACAddress()const;
    const QString &irq()const;
    const QString &memory()const;
    const QString &width()const;
    const QString &clock()const;
    const QString &capabilities()const;
    const QString &autoNegotiation()const;
    const QString &broadcast()const;
    const QString &driver()const;
    const QString &driverVersion()const;
    const QString &duplex()const;
    const QString &firmware()const;
    const QString &port()const;
    const QString &link()const;
    const QString &ip()const;
    const QString &speed()const;
    const QString &capacity()const;
    const QString &latency()const;
    const QString &multicast()const;

protected:
    void initFilterKey() override;

private:
    QString             m_Name;                         //<! 【名称】
    QString             m_Vendor;                       //<! 【制造商】
    QString             m_Model;                        //<! 【型号】
    QString             m_Version;                      //<! 【版本】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_LogicalName;                  //<! 【逻辑名称】
    QString             m_MACAddress;                   //<! 【物理地址】
    QString             m_Irq;                          //<! 【中断】
    QString             m_Memory;                       //<! 【内存地址】
    QString             m_Width;                        //<! 【位宽】
    QString             m_Clock;                        //<! 【时钟频率】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_Autonegotiation;              //<! 【自动协商】
    QString             m_Broadcast;                    //<! 【广播】
    QString             m_Driver;                       //<! 【驱动】
    QString             m_DriverVersion;                //<! 【驱动】
    QString             m_Duplex;                       //<! 【双工】
    QString             m_Firmware;                     //<! 【固件】
    QString             m_Port;                         //<! 【port】
    QString             m_Link;                         //<! 【链接】
    QString             m_Ip;                           //<! 【网址】
    QString             m_Speed;                        //<! 【速度】
    QString             m_Capacity;                     //<! 【最大速度】
    QString             m_Latency;                      //<! 【延迟】
    QString             m_Multicast;                    //<! 【组播】
};

#endif // DEVICENETWORK_H
