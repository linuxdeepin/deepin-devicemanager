/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LOADCPUINFOTHREAD_H
#define LOADCPUINFOTHREAD_H

#include <QThread>

class LoadCpuInfoThread : public QThread
{
    Q_OBJECT
public:
    explicit LoadCpuInfoThread();
    /**
     * @brief run
     */
    virtual void run() override;

signals:

public slots:

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
     * @param key:命令
     */
    void loadCpuInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd);

    /**
     * @brief getMapInfoFromCmd:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getCpuInfoFromLscpu:根据lscpu获取CPU信息
     */
    void getCpuInfoFromLscpu();
};

#endif // LOADCPUINFOTHREAD_H
