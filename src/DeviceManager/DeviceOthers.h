#ifndef DEVICEOTHERS_H
#define DEVICEOTHERS_H
#include"DeviceInfo.h"

class DeviceOthers : public DeviceBaseInfo
{
public:
    DeviceOthers();

    /**@brief:设置从lshw获取的信息*/
    void setInfoFromLshw(const QString &info);

    /**@brief:设置从hwinfo获取的信息*/
    void setInfoFromHwinfo(const QString &info);

    // 获取设备属性
    const QString &name()const;
    const QString &vendor()const;
    const QString &model()const;
    const QString &version()const;
    const QString &busInfo()const;
    const QString &capabilities()const;
    const QString &driver()const;
    const QString &maxinumPower()const;
    const QString &speed()const;
    const QString &logicalName()const;

protected:
    void initFilterKey() override;

private:
    QString             m_Name;                         //<! 【名称】
    QString             m_Vendor;                       //<! 【制造商】
    QString             m_Model;                        //<! 【型号】
    QString             m_Version;                      //<! 【版本】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_Driver;                       //<! 【驱动】
    QString             m_MaximumPower;                 //<! 【最大功耗】
    QString             m_Speed;                        //<! 【速度】

    QString             m_LogicalName;
};

#endif // DEVICEOTHERS_H
