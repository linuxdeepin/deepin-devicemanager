#ifndef DEVICEKEYBOARD_H
#define DEVICEKEYBOARD_H

#include"DeviceInfo.h"

class DeviceKeyboard : public DeviceBaseInfo
{
public:
    DeviceKeyboard();

    /**
     * @brief setInfoFromlshw:设置从lshw里面获取的信息
     * @param mapInfo:由lshw获取的信息map
     * @return 布尔值，true:信息设置成功；false:信息设置失败
     */
    bool setInfoFromlshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setInfoFromHwinfo:设置由hwinfo --keyboard 命令获取的设备信息
     * @param mapInfo:由hwinfo获取的信息map
     */
    void setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setKLUInfoFromHwinfo:特殊处理KLU设备,设置由hwinfo --keyboard 命令获取的设备信息
     * @param mapInfo由hwinfo获取的信息map
     */
    void setKLUInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &name()const;

    /**
     * @brief vendor:获取制造商属性值
     * @return QString:制造商属性值
     */
    const QString &vendor()const;

    /**
     * @brief model:获取型号属性值
     * @return QString:型号属性值
     */
    const QString &model()const;

    /**
       * @brief version:获取版本号属性值
       * @return QString:版本号属性值
       */
    const QString &version()const;

    /**
     * @brief interface:获取接口属性值
     * @return QString:接口属性值
     */
    const QString &interface()const;

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
     * @brief maxinumPower:获取最大功耗属性值
     * @return QString:最大功耗属性值
     */
    const QString &maxinumPower()const;

    /**
     * @brief speed:获取速度属性值
     * @return QString:速度属性值
     */
    const QString &speed()const;

    /**
     * @brief description:获取描述属性值
     * @return QString:描述属性值
     */
    const QString &description()const;

protected:
    void initFilterKey() override;

private:
    void getKeyboardMapInfoFromInputDevice(QMap<QString, QString> &mapInfo, const QString &info);

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

    QString             m_KeysToCatDevices;                 //<! 【用来标识唯一键盘】
};

#endif // DEVICEKEYBOARD_H
