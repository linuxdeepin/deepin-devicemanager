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

#ifndef DEVICECPU_H
#define DEVICECPU_H
#include "DeviceInfo.h"

/**
 * @brief The DeviceCpu class
 * 用来描述CPU的类
 */
class DeviceCpu : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceCpu)
public:
    DeviceCpu();

    /**
     * @brief setCpuInfo:设置cpu信息
     * @param mapLscpu:由lscpu获取的信息map
     * @param mapLshw:由lshw获取的信息map
     * @param mapDmidecode:由dmidecode -t 4获取的信息的map
     * @param catInfo:由cat /proc/cpuinfo获取的信息
     */
    void setCpuInfo(const QMap<QString, QString> &mapLscpu, const QMap<QString, QString> &mapLshw, const QMap<QString, QString> &mapDmidecode, const QMap<QString, QString> &catInfo, int coreNum, int logicalNum);
    /**
     * @brief vendor:获取制造商属性值
     * @return QString:制造商属性值
     */
    const QString &vendor() const;

    /**
     * @brief name:获取名称属性值
     * @return QString:名称属性值
     */
    const QString &name() const override;

    const QString &driver() const override;

    /**
     * @brief frequencyIsRange:频率是否是范围
     * @return 布尔值:true-频率显示范围;false-频率不显示范围
     */
    bool frequencyIsRange()const;

    /**
     * @brief subTitle : 详细信息里面的子标题
     * @return
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
     * @brief loadTableHeader : 加载表格的头部
     */
    void loadTableHeader() override;

    /**
     * @brief loadTableData:加载表格的内容
     */
    void loadTableData() override;

    void getTrNumber();

private:

    /**
     * @brief setInfoFromLscpu:设置由lscpu获取的CPU信息
     * @param mapInfo:由lscpu获取的CPU信息map
     */
    void setInfoFromLscpu(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setInfoFromLshw:设置由lshw获取的CPU信息
     * @param mapInfo:由lshw获取的CPU信息map
     */
    void setInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setInfoFromDmidecode:设置由dmidecode -t 4获取的CPU信息
     * @param mapInfo:由dmidecode -t 4获取的CPU信息map
     */
    void setInfoFromDmidecode(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setInfoFromCatCpuinfo:设置由cat /proc/cpuinfo获取的信息
     * @param mapInfo:由cat /proc/cpuinfo获取的信息map
     */
    void setInfoFromCatCpuinfo(const QMap<QString, QString> &mapInfo);

private:
    QString           m_Vendor;             //<! 制造商
    QString           m_Name;               //<! 名称
    QString           m_PhysicalID;         //<! 处理器ID
    QString           m_CoreID;             //<! 核心ID
    QString           m_ThreadNum;          //<! 线程数
    QString           m_Frequency;          //<! 频率
    QString           m_CurFrequency;       //<! 当前频率
    QString           m_BogoMIPS;           //<! BogoMIPS
    QString           m_Architecture;       //<! 架构
    QString           m_Familly;            //<! 家族
    QString           m_Model;              //<! 型号
    QString           m_Step;               //<! 步进
    QString           m_CacheL1Data;        //<! L1缓存（数据）
    QString           m_CacheL1Order;       //<! L1缓存（指令）
    QString           m_CacheL2;            //<! L2缓存
    QString           m_CacheL3;            //<! L3缓存
    QString           m_Extensions;         //<! 扩展指令集
    QString           m_Flags;              //<! 特性
    QString           m_HardwareVirtual;    //<! 硬件虚拟化

    // 这个是cpu的逻辑处理器个数，而不是core的逻辑处理器个数
    int               m_LogicalCPUNum;      //<! 逻辑处理器个数
    int               m_CPUCoreNum;         //<! cpu核数
    QString           m_Driver;
    bool              m_FrequencyIsRange;   //<! 频率显示的是范围还是最大值

    QMap<int, QString>        m_trNumber;
};

#endif // DEVICECPU_H
