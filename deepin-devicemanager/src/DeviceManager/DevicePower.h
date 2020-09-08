/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DEVICEPOWER_H
#define DEVICEPOWER_H
#include"DeviceInfo.h"

class DevicePower : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DevicePower)
public:
    DevicePower();

    /**
     * @brief setInfoFromUpower:设置从upower获取的电池信息
     * @param mapInfo:由upower --dump获取的信息map
     * @return 布尔值，true-信息设置成功；false-信息设置失败
     */
    bool setInfoFromUpower(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setDaemonInfo:设置从upower获取的与电池有关的守护进程信息
     * @param mapInfo:由upower --dump获取的守护进程信息信息map
     */
    void setDaemonInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &name()const override;

    /**
     * @brief driver:获取驱动名称
     * @return  QString 驱动名称
     */
    const QString &driver()const override;

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
     * @brief type:获取类型属性值
     * @return QString:类型属性值
     */
    const QString &type()const;

    /**
     * @brief serialNumber:获取序列号属性值
     * @return QString:序列号属性值
     */
    const QString &serialNumber()const;

    /**
     * @brief electircType:获取electircType属性值
     * @return QString:electircType属性值
     */
    const QString &electircType()const;

    /**
     * @brief maxPower:获取最大电压属性值
     * @return QString:最大电压属性值
     */
    const QString &maxPower()const;

    /**
     * @brief status:获取状态属性值
     * @return QString:状态属性值
     */
    const QString &status()const;

    /**
     * @brief enabled:获取enabled属性值
     * @return QString:enabled属性值
     */
    const QString &enabled()const;

    /**
     * @brief hotSwitch:获取hotswitch属性值
     * @return QString:hotswitch属性值
     */
    const QString &hotSwitch()const;

    /**
     * @brief capacity:获取电池容量属性值
     * @return QString:电源容量属性值
     */
    const QString &capacity()const;

    /**
     * @brief voltage:获取电压属性值
     * @return QString:电压属性值
     */
    const QString &voltage()const;

    /**
     * @brief slot:获取slot属性值
     * @return QString:slot属性值
     */
    const QString &slot()const;

    /**
     * @brief designCapacity:获取designCapacity属性值
     * @return QString:designCapacity属性值
     */
    const QString &designCapacity()const;

    /**
     * @brief designVoltage:获取designVoltage属性值
     * @return QString:designVoltage属性值
     */
    const QString &designVoltage()const;

    /**
     * @brief SBDSChemistry:获取SBDSChemistry属性值
     * @return QString:SBDSChemistry属性值
     */
    const QString &SBDSChemistry()const;

    /**
     * @brief SBDSManufactureDate:获取SBDSManufactureDate属性值
     * @return QString:SBDSManufactureDate属性值
     */
    const QString &SBDSManufactureDate()const;

    /**
     * @brief SBDSSerialNumber:获取SBDSSerialNumber属性值
     * @return QString:SBDSSerialNumber属性值
     */
    const QString &SBDSSerialNumber()const;

    /**
     * @brief SBDSVersion:获取SBDSVersion属性值
     * @return QString:SBDSVersion属性值
     */
    const QString &SBDSVersion()const;

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
    QString             m_Driver;

};

#endif // DEVICEPOWER_H
