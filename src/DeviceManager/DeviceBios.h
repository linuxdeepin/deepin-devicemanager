#ifndef DEVICEBIOS_H
#define DEVICEBIOS_H
#include"DeviceInfo.h"

class DeviceBios : public DeviceBaseInfo
{
public:
    DeviceBios();
    /**@brief:设置bios信息*/
    bool setBiosInfo(const QMap<QString, QString> &mapInfo);

    /**@brief:设置base信息*/
    bool setBaseBoardInfo(const QMap<QString, QString> &mapInfo);

    /**@brief:设置system信息*/
    bool setSystemInfo(const QMap<QString, QString> &mapInfo);

    /**@brief:设置Chassis信息*/
    bool setChassisInfo(const QMap<QString, QString> &mapInfo);

    /**@brief:设置memory信息*/
    bool setMemoryInfo(const QMap<QString, QString> &mapInfo);

    // 获取bios相关的属性
    const QString &name()const;
    const QString &productName()const;
    const QString &vendor()const;
    const QString &version()const;
    const QString &chipsetFamily()const;
    bool isBoard()const;

protected:
    void initFilterKey() override;

private:
    QString         m_Name;                   //<! 【名称】
    QString         m_ProductName;
    QString         m_Vendor;                 //<! 【制造商】
    QString         m_Version;                //<! 【版本】
    QString         m_ChipsetFamily;          //<! 【芯片组】
    bool            m_IsBoard;
};

#endif // DEVICEBIOS_H
