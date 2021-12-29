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

#ifndef DRIVERWAITINGWIDGET_H
#define DRIVERWAITINGWIDGET_H

#include <DWidget>
#include <DWaterProgress>
#include <DLabel>

DWIDGET_USE_NAMESPACE
class DriverWaitingWidget : public DWidget
{
public:
    explicit DriverWaitingWidget(QString status, QWidget *parent = nullptr);

    /**
     * @brief setValue 设置进度条的数值
     * @param value 0 ~ 100
     */
    void setValue(int value);

    /**
     * @brief setText 设置进度条下面的字符串
     * @param text 例如：正在卸载、正在更新
     */
    void setText(const QString &text);
private:
    /**
     * @brief init 初始化界面
     */
    void init();
    void onUpdateTheme();
private:
    DWaterProgress        *mp_Progress;
    DLabel                *mp_Label;
};
#endif // DRIVERWAITINGWIDGET_H
