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

#ifndef DEVICEMONITOR_H
#define DEVICEMONITOR_H
#include "DeviceInfo.h"

/**
 * @brief The DeviceMonitor class
 * 用来描述显示屏的类
 */

class DeviceMonitor : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceMonitor)
public:
    DeviceMonitor();

    /**
     * @brief setInfoFromHwinfo:设置由hwinfo --monitor命令获取的设备信息
     * @param mapInfo:由hwinfo获取的信息map
     */
    void setInfoFromHwinfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setInfoFromXradr:设置从xrandr中获取的信息
     * @param main:主显示器信息
     * @param edid:edid信息
     * @return 布尔值，true:信息设置成功；false:信息设置失败
     */
    bool setInfoFromXradr(const QString &main, const QString &edid);

    /**
     * @brief setCurrentResolution:设置从xrandr中获取的当前分辨率信息
     * @param resolution:当前分辨率
     * @param rate:当前刷新率
     * @return 布尔值，true:信息设置成功；false:信息设置失败
     */
    bool setCurrentResolution(const QString &resolution, const QString &rate);

    // 将年周转化为年月
    /**
     * @brief transWeekToDate:将年周转化为年月
     * @param year:年
     * @param week:周
     * @return QString:年月字符串
     */
    QString transWeekToDate(const QString &year, const QString &week);

    // 获取屏幕大小英寸
    /**
     * @brief parseMonitorSize:获取屏幕大小英寸
     * @param sizeDescription:【in】屏幕尺寸mm×mm or cm×cm
     * @param inch:【out】屏幕尺寸 英尺
     * @param retSize:【out】QSize
     * @return QString:屏幕尺寸mm*mm(inch)
     */
    QString parseMonitorSize(const QString &sizeDescription, double &inch, QSize &retSize);

    /**@brief:华为KLU项目里面的显示屏信息是写死的*/
    /**
     * @brief setInfoFromSelfDefine:设置华为KLU项目里面的显示屏信息部分为固定的值
     * @param mapInfo:固定值信息map
     */
    void setInfoFromSelfDefine(const QMap<QString, QString> &mapInfo);

    /**@brief:华为PanGuV项目里面的显示屏信息是从edid里面获取的*/
    /**
     * @brief setInfoFromEdid:设置华为PanGuV项目里面的显示屏信息（从edid里面获取的）
     * @param mapInfo:edid中获取的信息map
     */
    void setInfoFromEdid(const QMap<QString, QString> &mapInfo);

    /**
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &name()const override;

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
    /**@brief:设置是否主显示屏幕等信息*/
    /**
     * @brief setMainInfoFromXrandr:设置主显示器信息
     * @param info:由xrandr获取的主显示器有关信息
     * @return 布尔值，true:信息设置成功；false:信息设置失败
     */
    bool setMainInfoFromXrandr(const QString &info);

    /**
     * @brief caculateScreenRatio:计算屏幕比例
     */
    void caculateScreenRatio();

    /**
     * @brief gcd:计算最大公约数
     * @param a:屏幕宽
     * @param b:屏幕高
     * @return 最大公约数
     */
    int gcd(int a, int b);

    /**
     * @brief findAspectRatio:计算屏幕宽高比
     * @param width:宽
     * @param height:高
     * @param ar_w:宽
     * @param ar_h:高
     * @return布尔值，true:信息设置成功；false:信息设置失败
     */
    bool findAspectRatio(int width, int height, int &ar_w, int &ar_h);

    /**
     * @brief caculateScreenSize:计算屏幕大小
     */
    void caculateScreenSize();

    /**@brief:*/

    /**
     * @brief caculateScreenSize:根据edid计算屏幕大小
     * @param edid:edid
     */
    void caculateScreenSize(const QString &edid);



private:
    QString         m_Name;              //<! 【名称】
    QString         m_Vendor;            //<! 【制造商】
    QString         m_Model;             //<! 【型号】
    QString         m_DisplayInput;      //<! 【显示输入】
    QString         m_VGA;               //<! 【VGA】
    QString         m_HDMI;              //<! 【HDMI】
    QString         m_DVI;               //<! 【DVI】
    QString         m_Interface;         //<! 【显示屏借口类型】
    QString         m_ScreenSize;        //<! 【屏幕尺寸】
    QString         m_AspectRatio;       //<! 【显示比例】
    QString         m_MainScreen;        //<! 【主显示器】
    QString         m_CurrentResolution; //<! 【当前分辨率】
    QString         m_SerialNumber;      //<! 【流水号】
    QString         m_ProductionWeek;    //<! 【生产年月】
    QString         m_SupportResolution; //<!  支持的屏幕分辨率
    QString         m_Driver;

    int             m_Width;             //<!  屏幕的宽度
    int             m_Height;            //<!  屏幕的高度
};

#endif // DEVICEMONITOR_H
