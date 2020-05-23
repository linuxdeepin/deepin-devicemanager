/**
* @brief: 鼠标设备信息
* @class[DeviceMouse]: 一个DeviceMouse的实例 表示的就是一个鼠标设备
* @author: liujun
* @date:2020-04-09
*/

#ifndef DEVICEMOUSE_H
#define DEVICEMOUSE_H
#include"DeviceInfo.h"


// 鼠标设备信息
class DeviceMouse : public DeviceBaseInfo
{
public:
    DeviceMouse();

    /**@brief:添加 从 sudo hwinfo --mouse 中获取的信息 */
    void setInfoFromHwinfo(QMap<QString, QString> mapInfo);

    /**@brief:添加 从 sudo lshw -C input 中获取的信息 */
    bool setInfoFromlshw(QMap<QString, QString> mapInfo);

    /**@brief:获取基本设备属性*/
    const QString &name()const;
    const QString &vendor()const;
    const QString &model()const;
    const QString &interface()const;
    const QString &busInfo()const;
    const QString &version()const;
    const QString &capability()const;
    const QString &driver()const;
    const QString &maxPower()const;
    const QString &speed()const;

protected:
    void initFilterKey() override;

private:
    QString     m_Name;                             //<! 名称    #制造商+型号#
    QString     m_Vendor;                           //<! 制造商
    QString     m_Model;                            //<! 型号
    QString     m_Interface;                        //<! 接口
    QString     m_BusInfo;                          //<! 总线信息
    QString     m_Version;                          //<! 版本
    QString     m_Capabilities;                     //<! 功能
    QString     m_Driver;                           //<! 驱动
    QString     m_MaximumPower;                     //<! 最大功耗
    QString     m_Speed;                            //<! 速度

    /*通过该标识匹配从 cat /proc/bus/input/devices 中获取的信息 */
    QString     m_KeysToCatDevices;

    /*通过该标识匹配从 lshw 中获取的信息*/
    QString     m_KeyToLshw;
};

#endif // DEVICEMOUSE_H
