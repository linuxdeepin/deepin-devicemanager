#ifndef DEVICECDROM_H
#define DEVICECDROM_H
#include "DeviceInfo.h"

class DeviceCdrom : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceCdrom)
public:
    DeviceCdrom();

    /**
       * @brief setInfoFromLshw:设置从lshw里面获取的信息
       * @param mapInfo:由lshw获取的信息map
       * @return 布尔值，true:信息设置成功；false:信息设置失败
       */
    bool setInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setInfoFromHwinfo:设置由hwinfo --cdrom命令获取的设备信息
       * @param mapInfo:由hwinfo获取的信息map
       */
    void setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

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
       * @brief type:获取类型属性值
       * @return QString:类型属性值
       */
    const QString &type()const;

    /**
       * @brief version:获取名称属性值
       * @return QString:版本号属性值
       */
    const QString &version()const;

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
       * @brief maxPower:获取maxPower属性值
       * @return QString:maxPower属性值
       */
    const QString &maxPower()const;

    /**
       * @brief speed:获取速率属性值
       * @return QString:速率属性值
       */
    const QString &speed()const;

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

private:
    QString        m_Name;                  //<!名称
    QString        m_Vendor;                //<!制造商
    QString        m_Type;                  //<!型号
    QString        m_Version;               //<!版本
    QString        m_BusInfo;               //<!总线信息
    QString        m_Capabilities;          //<!功能
    QString        m_Driver;                //<!驱动
    QString        m_MaxPower;              //<!最大功耗
    QString        m_Speed;                 //<!速度

    QString        m_UnikeyKey;
    QString        m_KeyToLshw;            //<!
};

#endif // DEVICECDROM_H
