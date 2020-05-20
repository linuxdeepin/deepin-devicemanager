#ifndef DEVICEPRINT_H
#define DEVICEPRINT_H
#include"DeviceInfo.h"


class DevicePrint : public DeviceBaseInfo
{
public:
    DevicePrint();

    void setInfo(const QMap<QString, QString> &info);
    void setInfoFromHwinfo(const QString &info);
    void setInfoFromHwinfo(QMap<QString, QString> mapInfo);

    // 获取设备属性
    const QString &name()const;
    const QString &vendor()const;
    const QString &model()const;
    const QString &serialNumber()const;
    const QString &interfaceType()const;
    const QString &URI()const;
    const QString &status()const;
    const QString &shared()const;

protected:
    void initFilterKey() override;

private:
    QString             m_Name;                         //<! 【名称】
    QString             m_Vendor;                       //<! 【制造商】
    QString             m_Model;                        //<! 【型号】
    QString             m_SerialNumber;                 //<! 【序列号】
    QString             m_InterfaceType;                //<! 【接口类型】
    QString             m_URI;                          //<! 【URI】
    QString             m_Status;                       //<! 【状态】
    QString             m_Shared;                       //<! 【共享】
};

#endif // DEVICEPRINT_H
