#ifndef DEVICEINPUT_H
#define DEVICEINPUT_H

#include "DeviceInfo.h"

class DeviceInput : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceInput)

public:
    DeviceInput();

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
    const QString &name()const override;

    /**
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &driver()const override;

    /**
     * @brief subTitle:获取子标题
     * @return 子标题
     */
    QString subTitle() override;

    /**
     * @brief getOverviewInfo:获取概况信息
     * @return 概况信息
     */
    const QString getOverviewInfo() override;

protected:
    /**
     * @brief initFilterKey:初始化可现实的可显示的属性,m_FilterKey
     */
    void initFilterKey() override;

    /**
     * @brief loadBaseDeviceInfo:加载基本信息
     */
    void loadBaseDeviceInfo() override;

    /**
     * @brief loadOtherDeviceInfo:加载基本信息
     */
    void loadOtherDeviceInfo() override;

    /**
     * @brief loadTableData:加载表头信息
     */
    void loadTableData() override;

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

#endif // DEVICEINPUT_H
