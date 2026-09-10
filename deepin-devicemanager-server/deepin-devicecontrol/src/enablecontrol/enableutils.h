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
     * @brief The NetworkOperateSource enum 网卡启用/禁用操作的调用来源
     * 用于日志中区分调用链路
     */
    enum NetworkOperateSource {
        NOS_UI_OPERATE,      //!< 链路一：用户在设备管理器UI界面启用/禁用网卡（经DBus的enable接口进入）
        NOS_BOOT_RESTORE     //!< 链路二：系统重启/服务唤起时恢复禁用状态（经disableInDevice/disableOutDevice进入）
    };

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
     * 首先判断NetworkManager服务是否已经启动：
     * 服务已启动时，直接执行启用/禁用逻辑；
     * 服务未启动时，通过QDBusServiceWatcher监控该服务，待其启动后再等待3秒异步执行启用/禁用逻辑
     * （此时函数立即返回true表示操作已被接受，执行结果无法同步返回）
     * 启用/禁用逻辑：
     * 有线网卡直接通过ioctl方式禁用/启用；
     * 无线网卡需先判断系统上无线网卡的个数：
     * 无线网卡个数为1时，通过rfkill直接block/unblock当前无线网卡；
     * 无线网卡个数大于1时，通过ioctl方式禁用/启用指定网卡
     * @param logicalName 网卡逻辑名称
     * @param enable true启用，false禁用
     * @param source 调用来源，用于日志中区分调用链路（UI操作/开机恢复）
     * @return 成功返回true，失败返回false；服务未启动时返回true表示操作已被接受（异步执行）
     */
    static bool ioctlOperateNetworkLogicalName(const QString &logicalName, bool enable, NetworkOperateSource source);

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
     * @brief enableWirelessByRfkill 通过rfkill直接block/unblock当前无线网卡
     * 禁用时将当前无线网卡rfkill soft block（射频下电），启用时解除block
     * @param logicalName 网卡逻辑名称
     * @param enable true启用（解除block），false禁用（block）
     * @return 成功返回true，失败返回false
     */
    static bool enableWirelessByRfkill(const QString &logicalName, bool enable);

    /**
     * @brief getMapInfo 解析usb信息
     * @param item
     * @param mapInfo
     * @return
     */
    static bool getMapInfo(const QString &item, QMap<QString, QString> &mapInfo);

private:
    /**
     * @brief ioctlOperateNetworkLogicalNameImpl 实际执行网卡启用/禁用逻辑（有线/无线判断及分流处理）
     * 由ioctlOperateNetworkLogicalName在NetworkManager服务就绪后调用
     * @param logicalName 网卡逻辑名称
     * @param enable true启用，false禁用
     * @param source 调用来源，用于日志中区分调用链路（UI操作/开机恢复）
     * @return 成功返回true，失败返回false
     */
    static bool ioctlOperateNetworkLogicalNameImpl(const QString &logicalName, bool enable, NetworkOperateSource source);

    /**
     * @brief setRfkillBlocked 通过rfkill block/unblock指定无线网卡
     * 读取 /sys/class/net/<iface>/phy80211/name 获取phy名称，
     * 在 /sys/class/rfkill/ 下查找 type=wlan 且 name 与phy名称一致的rfkill设备，
     * 写其soft文件：1表示block，0表示unblock
     * @param interfaceName 网卡逻辑名称
     * @param blocked true表示block，false表示unblock
     * @return 成功返回true；网卡名格式非法或非无线网卡（无phy80211信息）返回false；未找到对应rfkill设备时返回true（仅告警）
     */
    static bool setRfkillBlocked(const QString &interfaceName, bool blocked);
};

#endif // ENABLEUTILS_H
