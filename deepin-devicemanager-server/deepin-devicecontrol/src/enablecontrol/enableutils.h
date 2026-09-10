// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENABLEUTILS_H
#define ENABLEUTILS_H

#include <QString>

class EnableUtils
{
public:
    EnableUtils();

    /**
     * @brief disableDevice 禁用外设
     * @param info
     */
    static void disableOutDevice(const QString &info);


    /**
     * @brief disableInDevice 禁用非外设
     */
    static void disableInDevice();

    /**
     * @brief ioctlOperateNetworkLogicalName 设置网卡是否可用
     * 有线网卡直接通过ioctl方式禁用/启用；
     * 无线网卡需先判断系统上无线网卡的个数：
     * 无线网卡个数为1时，通过NetworkManager的WirelessEnabled属性统一禁用/启用；
     * 无线网卡个数大于1时，通过ioctl方式禁用/启用指定网卡
     * @param logicalName 网卡逻辑名称
     * @param enable true启用，false禁用
     * @return 成功返回true，失败返回false
     */
    static bool ioctlOperateNetworkLogicalName(const QString &logicalName, bool enable);

    /**
     * @brief enableNetworkByIoctl 通过ioctl禁用/启用指定网卡
     * @param logicalName 网卡逻辑名称
     * @param enable true启用，false禁用
     * @return 成功返回true，失败返回false
     */
    static bool enableNetworkByIoctl(const QString &logicalName, bool enable);

    /**
     * @brief isWirelessNetwork 判断网卡是否是无线网卡
     * 通过NetworkManager的GetDeviceByIpIface获取网卡dbus path，
     * 再读取DeviceType属性，值为2表示无线网卡
     * @param logicalName 网卡逻辑名称
     * @return 是无线网卡返回true，获取失败按有线网卡处理返回false
     */
    static bool isWirelessNetwork(const QString &logicalName);

    /**
     * @brief wirelessNetworkCount 统计当前系统上无线网卡的个数
     * 通过NetworkManager的GetDevices获取所有网络设备dbus path，
     * 逐个检查Managed属性（跳过非托管设备），再检查DeviceType属性，值为2表示无线网卡
     * @return 无线网卡个数
     */
    static int wirelessNetworkCount();

    /**
     * @brief enableWirelessByDBus 系统上仅有1个无线网卡时，通过NetworkManager的WirelessEnabled属性统一禁用/启用无线网卡
     * @param enable true启用，false禁用
     * @return 成功返回true，失败返回false
     */
    static bool enableWirelessByDBus(bool enable);

    /**
     * @brief getMapInfo 解析usb信息
     * @param item
     * @param mapInfo
     * @return
     */
    static bool getMapInfo(const QString &item, QMap<QString, QString> &mapInfo);
};

#endif // ENABLEUTILS_H
