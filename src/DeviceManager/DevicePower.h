#ifndef DEVICEPOWER_H
#define DEVICEPOWER_H
#include"DeviceInfo.h"

class DevicePower : public DeviceBaseInfo
{
public:
    DevicePower();

    /**@brief:设置从upower获取的信息*/
    void setInfoFromUpower(const QMap<QString, QString> &mapInfo);
    void setDaemonInfo(const QMap<QString, QString> &mapInfo);

    // 获取设备属性
    const QString &name()const;
    const QString &vendor()const;
    const QString &model()const;
    const QString &type()const;
    const QString &serialNumber()const;
    const QString &electircType()const;
    const QString &maxPower()const;
    const QString &status()const;
    const QString &enabled()const;
    const QString &hotSwitch()const;
    const QString &capacity()const;
    const QString &voltage()const;
    const QString &slot()const;
    const QString &designCapacity()const;
    const QString &designVoltage()const;
    const QString &SBDSChemistry()const;
    const QString &SBDSManufactureDate()const;
    const QString &SBDSSerialNumber()const;
    const QString &SBDSVersion()const;

protected:
    void initFilterKey() override;

private:
    QString             m_Name;                     //<! 【名称】
    QString             m_Vendor;                   //<! 【制造商】
    QString             m_Model;                    //<! 【设备类型】
    QString             m_Type;                     //<! 【设备类型，电池还是电源】
    QString             m_SerialNumber;             //<! 【序列号】
    QString             m_ElectricType;             //<! 【电流类型】
    QString             m_MaxPower;                 //<! 【最大功率】
    QString             m_Status;                   //<! 【状态】
    QString             m_Enabled;                  //<! 【是否启用】
    QString             m_HotSwitch;                //<! 【热切换】
    QString             m_Capacity;                 //<! 【容量】
    QString             m_Voltage;                  //<! 【电压】
    QString             m_Slot;                     //<! 【插槽】
    QString             m_DesignCapacity;           //<! 【设计容量】
    QString             m_DesignVoltage;            //<! 【设计电压】
    QString             m_SBDSChemistry;            //<! 【SBDS材料】
    QString             m_SBDSManufactureDate;      //<! 【SBDS制造日期】
    QString             m_SBDSSerialNumber;         //<! 【SBDS序列号】
    QString             m_SBDSVersion;              //<! 【SBDS版本】
};

#endif // DEVICEPOWER_H
