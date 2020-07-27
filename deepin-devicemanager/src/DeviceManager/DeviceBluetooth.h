#ifndef DEVICEBLUETOOTH_H
#define DEVICEBLUETOOTH_H
#include"DeviceInfo.h"

class DeviceBluetooth : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceBluetooth)
public:
    DeviceBluetooth();

    /**
       * @brief setInfoFromHciconfig:设置由hciconfig命令获取的设备信息
       * @param mapInfo:由hciconfig获取的信息map
       */
    void setInfoFromHciconfig(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setInfoFromHwinfo:设置由hwinfo --sound命令获取的设备信息
       * @param mapInfo:由hwinfo获取的信息map
       * @return 布尔值，true:信息设置成功；false:信息设置失败
       */
    bool setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setInfoFromLshw:设置从lshw里面获取的信息
       * @param mapInfo:由lshw获取的信息map
       * @return 布尔值，true:信息设置成功；false:信息设置失败
       */
    bool setInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
       * @brief name:获取名称属性值
       * @return QString:名称属性值
       */
    const QString &name()const;

    /**
       * @brief vendor:获取制造商属性值
       * @return QString:制造商属性值
       */
    const QString &vendor()const;

    /**
       * @brief version:获取名称属性值
       * @return QString:版本号属性值
       */
    const QString &version()const;

    /**
       * @brief model:获取型号属性值
       * @return QString:型号属性值
       */
    const QString &model()const;

    /**
       * @brief MAC:获取MAC属性值
       * @return QString:MAC属性值
       */
    const QString &MAC()const;

    /**
       * @brief logicalName:获取逻辑名称属性值
       * @return QString:逻辑名称属性值
       */
    const QString &logicalName()const;

    /**
       * @brief busInfo:获取总线信息属性值
       * @return QString:总线信息属性值
       */
    const QString &busInfo()const;

    /**
       * @brief capabilities:获取功能属性值
       * @return QString:功能属性值
       */
    const QString &capabilities()const;

    /**
       * @brief driver:获取驱动属性值
       * @return QString:驱动属性值
       */
    const QString &driver()const;

    /**
       * @brief driverVersion:获取驱动版本号属性值
       * @return QString:驱动版本号属性值
       */
    const QString &driverVersion()const;

    /**
       * @brief maximumPower:获取最大电压属性值
       * @return QString:最大电压号属性值
       */
    const QString &maximumPower()const;

    /**
       * @brief speed:获取速率属性值
       * @return QString:速率属性值
       */
    const QString &speed()const;

    /**
     * @brief subTitle:获取子标题
     * @return 子标题
     */
    QString subTitle() override;

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
     * @brief loadTableData:加载表头信息
     */
    void loadTableData() override;


private:

    /**
     * @brief parseKeyToLshw:获取映射到lshw的key
     * @param info:hwinfo 中的SysFS BusID属性值
     */
    void parseKeyToLshw(const QString &info);

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
