#ifndef DEVICEBIOS_H
#define DEVICEBIOS_H
#include"DeviceInfo.h"

class DeviceBios : public DeviceBaseInfo
{
public:
    DeviceBios();
    /**@brief:设置bios信息*/
    bool setBiosInfo(const QString &info);

    /**@brief:设置base信息*/
    bool setBaseBoardInfo(const QString &info, const QString &chipsetFamliy);

    /**@brief:设置system信息*/
    bool setSystemInfo(const QString &info);

    /**@brief:设置Chassis信息*/
    bool setChassisInfo(const QString &info);

    /**@brief:设置memory信息*/
    bool setMemoryInfo(const QString &info);

    // 获取bios相关的属性
    const QString &name()const;
    const QString &productName()const;
    const QString &vendor()const;
    const QString &version()const;
    const QString &chipsetFamily()const;
    bool isBoard()const;

protected:
    void initFilterKey() override;

    /**@brief:获取从dmidecode中获取的设备信息*/
    void getMapInfoFromDmidecode(QMap<QString, QString> &mapInfo, const QString &info);

private:
    QString         m_Name;                   //<! 【名称】
    QString         m_ProductName;
    QString         m_Vendor;                 //<! 【制造商】
    QString         m_Version;                //<! 【版本】
    QString         m_ChipsetFamily;          //<! 【芯片组】
    bool            m_IsBoard;
};

#endif // DEVICEBIOS_H
