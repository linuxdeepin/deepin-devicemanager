#ifndef WAKEUPUTILS_H
#define WAKEUPUTILS_H

#include <QString>

class WakeupUtils
{
public:
    WakeupUtils();

    /**
     * @brief updateWakeupDeviceInfo
     * @param info
     */
    static void updateWakeupDeviceInfo(const QString& info);

    /**
     * @brief wakeupPath : get wakeup path by sys path
     * @param syspath : sys path
     * @param wakeuppath : wakeup path
     * @return
     */
    static bool wakeupPath(const QString& syspath, QString& wakeuppath);

    /**
     * @brief writeWakeupFile : write wakeup file
     * @param path : wakeup file path
     * @param wakeup : enable or disable
     * @return
     */
    static bool writeWakeupFile(const QString& path, bool wakeup);

private:
    /**
     * @brief getMapInfo 解析usb信息
     * @param item
     * @param mapInfo
     * @return
     */
    static bool getMapInfo(const QString& item,QMap<QString,QString>& mapInfo);

    /**
     * @brief getPS2Syspath 获取ps2鼠标键盘的syspath
     * @param dfs : Device Files 属性
     * @return
     */
    static QString getPS2Syspath(const QString& dfs);
};

#endif // WAKEUPUTILS_H
