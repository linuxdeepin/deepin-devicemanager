/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
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

#ifndef DEVICEBIOS_H
#define DEVICEBIOS_H
#include"DeviceInfo.h"

/**
 * @brief The DeviceBios class
 * 用来描述主板的类
 */
class DeviceBios : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceBios)
public:
    DeviceBios();

    /**
       * @brief setBiosInfo:设置bios信息
       * @param mapInfo:bios信息map
       * @return 布尔值:true-设置成功；false-设置失败
       */
    bool setBiosInfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setBiosLanguageInfo:设置bios语言信息
       * @param mapInfo:bios语言信息map
       * @return 布尔值:true-设置成功；false-设置失败
       */
    bool setBiosLanguageInfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setBaseBoardInfo:设置主板信息
       * @param mapInfo:主板信息map
       * @return 布尔值:true-设置成功；false-设置失败
       */
    bool setBaseBoardInfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setSystemInfo:设置系统信息
       * @param mapInfo:系统信息map
       * @return 布尔值:true-设置成功；false-设置失败
       */
    bool setSystemInfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setChassisInfo:设置机箱信息
       * @param mapInfo:机箱信息map
       * @return 布尔值:true-设置成功；false-设置失败
       */
    bool setChassisInfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setChassisInfo:设置内存插槽信息
       * @param mapInfo:内存插槽信息map
       * @return 布尔值:true-设置成功；false-设置失败
       */
    bool setMemoryInfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief name:获取名称属性值
       * @param 无
       * @return QString:名称属性值
       */
    const QString &name()const override;

    const QString &driver() const override;

    /**
       * @brief vendor:获取制造商属性值
       * @param 无
       * @return QString:制造商属性值
       */
    const QString &vendor()const;

    /**
       * @brief version:获取版本号属性值
       * @param 无
       * @return QString:版本号属性值
       */
    const QString &version()const;

    /**
       * @brief isBoard:判断是否有主板信息
       * @param 无
       * @return 布尔值:true-有主板信息；false-无主板信息
       */
    bool isBoard()const;

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
     * @brief loadTableHeader : 过去表格的表头数据
     */
    void loadTableHeader() override;

    /**
     * @brief loadTableData:加载表头信息
     */
    void loadTableData() override;


private:
    QString         m_Name;                   //<! 【名称】
    QString         m_ProductName;
    QString         m_Vendor;                 //<! 【制造商】
    QString         m_Version;                //<! 【版本】
    QString         m_ChipsetFamily;          //<! 【芯片组】
    QString         m_Driver;

    bool            m_IsBoard;
};

#endif // DEVICEBIOS_H
