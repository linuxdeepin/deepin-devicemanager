/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
* Maintainer: xxx.xx <xxxxxxx@uniontech.com>
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

#ifndef THREADEXECXRANDR_H
#define THREADEXECXRANDR_H

#include <QThread>

class ThreadExecXrandr : public QThread
{
public:
    ThreadExecXrandr(bool gpu);

    /**
     * @brief run
     */
    virtual void run() override;

private:
    /**
     * @brief runCmd
     * @param info
     * @param cmd
     */
    void runCmd(QString &info, const QString &cmd);

    /**
     * @brief loadXrandrInfo:加载xrandr获取的信息
     * @param lstMap
     * @param cmd:调试文件名
     */
    void loadXrandrInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd);

    /**
     * @brief loadXrandrVerboseInfo:加载xrandr --verbose获取的信息
     * @param lstMap
     * @param cmd:调试文件名
     */
    void loadXrandrVerboseInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd);

    /**
     * @brief getMonitorInfoFromXrandrVerbose:从xrandr --verbose获取显示设备信息
     */
    void getMonitorInfoFromXrandrVerbose();

    /**
     * @brief getMonitorRefreshRateFromXrandr:从xrandr获取显示设备刷新率
     */
    void getMonitorRefreshRateFromXrandr();

    /**
     * @brief getGpuInfoFromXrandr:从xrandr获取显卡信息
     */
    void getGpuInfoFromXrandr();


private:
    bool m_Gpu;    //<!  判断是否是gpu
};

#endif // THREADEXECXRANDR_H
