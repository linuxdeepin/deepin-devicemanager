#ifndef DEVICECDROM_H
#define DEVICECDROM_H
#include "DeviceInfo.h"

class DeviceCdrom : public DeviceBaseInfo
{
public:
    DeviceCdrom();

    //
    bool setInfoFromLshw(const QString& info);
    void setInfoFromHwinfo(const QString& info);

    //获取属性
    const QString& name()const;
    const QString& vendor()const;
    const QString& type()const;
    const QString& version()const;
    const QString& busInfo()const;
    const QString& capabilities()const;
    const QString& driver()const;
    const QString& maxPower()const;
    const QString& speed()const;

protected:
    void initFilterKey() override;

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
};

#endif // DEVICECDROM_H
