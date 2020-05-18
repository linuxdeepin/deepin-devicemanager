#ifndef DEVICEMEMORY_H
#define DEVICEMEMORY_H
#include"DeviceInfo.h"

class DeviceMemory : public DeviceBaseInfo
{
public:
    DeviceMemory();

    /**@brief:设置从 sudo lshw -C memory 获取的内存信息*/
    void setInfoFromLshw(const QString &info);

    /**@brief:设置从 sudo dmideocde -t memory 获取的内存信息*/
    bool setInfoFromDmidecode(const QString &info);

    // 获取各种属性
    const QString &name()const;
    const QString &vendor()const;
    const QString &model()const;
    const QString &size()const;
    const QString &type()const;
    const QString &speed()const;
    const QString &totalBandwidth()const;
    const QString &dataBandwidth()const;
    const QString &locator()const;
    const QString &serialNumber()const;
    const QString &configuredSpeed()const;
    const QString &minimumVoltage()const;
    const QString &maximumVoltage()const;
    const QString &configuredVoltage()const;
protected:
    /**@brief:初始化过滤信息*/
    virtual void initFilterKey();

private:
    QString           m_Name;                   //<! 【名称】
    QString           m_Vendor;                 //<! 【制造商】
    QString           m_Model;                  //<! 【型号】
    QString           m_Size;                   //<! 【大小】
    QString           m_Type;                   //<! 【类型】
    QString           m_Speed;                  //<! 【速度】
    QString           m_TotalBandwidth;         //<! 【总带宽】
    QString           m_DataBandwidth;          //<! 【数据带宽】
    QString           m_Locator;                //<! 【内存槽位】
    QString           m_SerialNumber;           //<! 【序列号】
    QString           m_ConfiguredSpeed;        //<! 【配置速度】
    QString           m_MinimumVoltage;         //<! 【最小电压】
    QString           m_MaximumVoltage;         //<! 【最大电压】
    QString           m_ConfiguredVoltage;      //<! 【配置电压】
};

#endif // DEVICEMEMORY_H
