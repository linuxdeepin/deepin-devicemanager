#ifndef DEVICEKEYBOARD_H
#define DEVICEKEYBOARD_H

#include"DeviceInfo.h"

class DeviceKeyboard : public DeviceBaseInfo
{
public:
    DeviceKeyboard();

    /**@brief:添加 从 sudo lshw -C input 中获取的信息 */
    bool setInfoFromlshw(const QMap<QString, QString> &mapInfo);

    /**@brief:添加 从 sudo hwinfo --keyboard 中获取的信息 */
    void setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**@brief:特殊处理KLU设备 */
    void setKLUInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    // 获取设备属性
    const QString &name()const;
    const QString &vendor()const;
    const QString &model()const;
    const QString &version()const;
    const QString &interface()const;
    const QString &busInfo()const;
    const QString &capabilities()const;
    const QString &driver()const;
    const QString &maxinumPower()const;
    const QString &speed()const;
    const QString &description()const;

protected:
    void initFilterKey() override;

private:
    void getKeyboardMapInfoFromInputDevice(QMap<QString, QString> &mapInfo, const QString &info);

    /**
     * @brief setInfoFromBluetoothctl:由bluetoothctl paired-devices设置设备接口
     */
    void setInfoFromBluetoothctl();

    /**
     * @brief setNameFromInput:从cat /proc/bus/input/devices获取设备信息
     */
    void setInfoFromInput();

private:
    QString             m_Name;                         //<! 【名称】
    QString             m_Vendor;                       //<! 【制造商】
    QString             m_Model;                        //<! 【型号】
    QString             m_Version;                      //<! 【版本】
    QString             m_Interface;                    //<! 【接口】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_Description;                  //<! 【描述】
    QString             m_Driver;                       //<! 【驱动】
    QString             m_MaximumPower;                 //<! 【最大功耗】
    QString             m_Speed;                        //<! 【速度】
    QString             m_KeyToLshw;                    //<!

    QString             m_keysToPairedDevice;           //<! 【用来标识蓝牙键盘】
    QString             m_KeysToCatDevices;                 //<! 【用来标识唯一键盘】
};

#endif // DEVICEKEYBOARD_H
