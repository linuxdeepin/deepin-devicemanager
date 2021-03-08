#ifndef DEVICENETWORK_H
#define DEVICENETWORK_H
#include"DeviceInfo.h"


class DeviceNetwork : public DeviceBaseInfo
{
public:
    DeviceNetwork();

    // 设置从 lshw 获取的信息
    void setInfoFromLshw(const QMap<QString, QString> &mapInfo);
    // 设置从 hwinfo 获取的信息
    bool setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setInfoFromWifiInfo:设置从cat /sys/hisys/wal/wifi_devices_info里面获取的信息
     * @param mapInfo: 由cat /sys/hisys/wal/wifi_devices_info获取的信息map
     * @return 布尔值，true：信息设置成功；false：信息设置失败
     */
    bool setInfoFromWifiInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &name()const;

    /**
     * @brief vendor:获取制造商属性值
     * @return QString:制造商属性值
     */
    const QString &vendor()const;

    /**
     * @brief model:获取型号属性值
     * @return QString:型号属性值
     */
    const QString &model()const;

    /**
       * @brief version:获取版本号属性值
       * @return QString:版本号属性值
       */
    const QString &version()const;

    /**
     * @brief busInfo:获取总线信息属性值
     * @return QString:总线信息属性值
     */
    const QString &busInfo()const;

    /**
     * @brief logicalName:获取逻辑名称属性值
     * @return QString:逻辑名称属性值
     */
    const QString &logicalName()const;

    /**
     * @brief MACAddress
     * @return
     */
    const QString &MACAddress()const;

    /**
       * @brief irq:获取中断属性值
       * @return QString:中断属性值
       */
    const QString &irq()const;

    /**
       * @brief memory:获取内存地址属性值
       * @return QString:内存地址属性值
       */
    const QString &memory()const;

    /**
       * @brief width获取位宽属性值
       * @return QString:位宽属性值
       */
    const QString &width()const;

    /**
       * @brief clock:获取时钟频率属性值
       * @return QString:时钟频率属性值
       */
    const QString &clock()const;

    /**
       * @brief capabilities:获取功能属性值
       * @return QString:功能属性值
       */
    const QString &capabilities()const;

    /**
     * @brief autoNegotiation
     * @return
     */
    const QString &autoNegotiation()const;

    /**
     * @brief broadcast
     * @return
     */
    const QString &broadcast()const;

    /**
     * @brief driver:获取驱动属性值
     * @return QString:驱动属性值
     */
    const QString &driver()const;

    /**
     * @brief driverVersion
     * @return
     */
    const QString &driverVersion()const;

    /**
     * @brief duplex
     * @return
     */
    const QString &duplex()const;

    /**
     * @brief firmware
     * @return
     */
    const QString &firmware()const;

    /**
     * @brief port
     * @return
     */
    const QString &port()const;

    /**
     * @brief link
     * @return
     */
    const QString &link()const;

    /**
     * @brief ip
     * @return
     */
    const QString &ip()const;

    /**
       * @brief speed:获取速率属性值
       * @return QString:速率属性值
       */
    const QString &speed()const;

    /**
     * @brief capacity
     * @return
     */
    const QString &capacity()const;

    /**
     * @brief latency:获取latency属性值
     * @return QString:latency属性值
     */
    const QString &latency()const;

    /**
     * @brief multicast:获取组播属性值
     * @return QString:组播属性值
     */
    const QString &multicast()const;

protected:

    /**
     * @brief initFilterKey:初始化可现实的可显示的属性,m_FilterKey
     */
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
