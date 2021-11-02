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

#ifndef GETDRIVERNAMEDIALOG_H
#define GETDRIVERNAMEDIALOG_H

#include <DWidget>

DWIDGET_BEGIN_NAMESPACE

class DriverListView;
class GetDriverNameWidget : public DWidget
{
    Q_OBJECT
public:
    explicit GetDriverNameWidget(QWidget *parent = nullptr);

    /**
     * @brief loadAllDrivers 通过给的路径，查找路径下的所有驱动文件
     * @param includeSub 是否查找目录下的子目录
     * @param path 给定的目录
     */
    void loadAllDrivers(bool includeSub, const QString& path);

    /**
     * @brief selectName 用户选中的驱动名称
     * @return
     */
    QString selectName();

private:
    /**
     * @brief init 初始化界面
     */
    void init();

    /**
     * @brief traverseFolders 遍历目录下的文件
     * @param includeSub 是否查找目录下的子目录
     * @param path 给定的目录
     * @param lstDrivers 出参，符合条件的驱动文件
     */
    void traverseFolders(bool includeSub, const QString& path, QStringList& lstDrivers);

private:
    DriverListView    *mp_ListView;
};
DWIDGET_END_NAMESPACE
#endif // GETDRIVERNAMEDIALOG_H
