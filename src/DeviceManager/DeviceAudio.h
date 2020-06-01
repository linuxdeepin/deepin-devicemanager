#ifndef DEVICEAUDIO_H
#define DEVICEAUDIO_H
#include"DeviceInfo.h"


class DeviceAudio : public DeviceBaseInfo
{
public:
    DeviceAudio();

    /**@brief:设置从hwinfo里面获取的信息*/
    void setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**@brief:设置从lshw里面获取的信息*/
    bool setInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**@brief:设置从cat /proc/bus/input/devices里面获取的信息*/
    bool setInfoFromCatDevices(const QMap<QString, QString> &mapInfo);

    bool setAudioChipFromDmesg(const QString &info);

    // 获取设备属性
    const QString &name()const;
    const QString &vendor()const;
    const QString &model()const;
    const QString &version()const;
    const QString &busInfo()const;
    const QString &irq()const;
    const QString &memory()const;
    const QString &width()const;
    const QString &clock()const;
    const QString &capabilities()const;
    const QString &description()const;
    const QString &chip() const;

protected:
    void initFilterKey() override;

private:
    QString             m_Name;                         //<! 【名称】
    QString             m_Vendor;                       //<! 【制造商】
    QString             m_Model;                        //<! 【型号】
    QString             m_Version;                      //<! 【版本】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_Irq;                          //<! 【中断】
    QString             m_Memory;                       //<! 【内存地址】
    QString             m_Width;                        //<! 【位宽】
    QString             m_Clock;                        //<! 【时钟频率】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_Description;                  //<! 【描述】
    QString             m_Chip;                         //<! 【芯片型号】

    QString             m_UniqueKey;                    //<! 【标识唯一设备】
};

#endif // DEVICEAUDIO_H
