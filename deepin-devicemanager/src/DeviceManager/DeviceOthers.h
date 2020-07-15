#ifndef DEVICEOTHERS_H
#define DEVICEOTHERS_H
#include"DeviceInfo.h"

class DeviceOthers : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceOthers)
public:
    DeviceOthers();

    /**@brief:设置从lshw获取的信息*/
    void setInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**@brief:设置从hwinfo获取的信息*/
    void setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

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
     * @brief maxinumPower:获取最大电压属性值
     * @return QString:最大电压属性值
     */
    const QString &maxinumPower()const;

    /**
     * @brief speed:获取速率属性值
     * @return QString:速率属性值
     */
    const QString &speed()const;

    /**
     * @brief logicalName:获取逻辑名称属性值
     * @return QString:逻辑名称属性值
     */
    const QString &logicalName()const;

protected:

    /**
     * @brief initFilterKey:初始化可现实的可显示的属性,m_FilterKey
     */
    void initFilterKey() override;

    /**
     * @brief loadBaseDeviceInfo:加载基本信息
     */
    void loadBaseDeviceInfo() override;

private:
    QString             m_Name;                         //<! 【名称】
    QString             m_Vendor;                       //<! 【制造商】
    QString             m_Model;                        //<! 【型号】
    QString             m_Version;                      //<! 【版本】
    QString             m_BusInfo;                      //<! 【总线信息】
    QString             m_Capabilities;                 //<! 【功能】
    QString             m_Driver;                       //<! 【驱动】
    QString             m_MaximumPower;                 //<! 【最大功耗】
    QString             m_Speed;                        //<! 【速度】
    QString             m_UniqID;                       //<! 【唯一ID】
    QString             m_BusID;                        //<! 【总线ID】
    QString             m_LogicalName;
};

#endif // DEVICEOTHERS_H
