/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <changze@uniontech.com>
*
* Maintainer: changze <changze@uniontech.com>
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
#ifndef DRIVERICONWIDGET_H
#define DRIVERICONWIDGET_H

#include <DWidget>
#include <DWaterProgress>

#include <QObject>

DWIDGET_USE_NAMESPACE

class DriverIconWidget : public DWidget
{
    Q_OBJECT
public:
    /**
     * @brief DriverIconWidget 返回有进度条的widget
     * @param strTitle 第一文本
     * @param strDesc 第二文本
     * @param parent
     */
    explicit DriverIconWidget(const QString &strTitle,
                          const QString &strDesc,
                          QWidget *parent = nullptr);
    /**
     * @brief DriverIconWidget 返回有icon的widget
     * @param pixmap 图标
     * @param strTitle 第一文本
     * @param strDesc 第二文本
     * @param parent
     */
    explicit DriverIconWidget(const QPixmap &pixmap,
                          const QString &strTitle,
                          const QString &strDesc,
                          QWidget *parent = nullptr);

    virtual ~DriverIconWidget();

private:
    /**
     * @brief initUI 初始化UI
     * @param iconWidget iconWidget
     * @param strTitle 第一文本
     * @param strDesc 第二文本
     */
    void initUI(QWidget *iconWidget, const QString &strTitle, const QString &strDesc);

public:
    /**
     * @brief updateProgress 更新进度
     * @param progress 当前进度
     * @param detail 描述
     */
    void updateProgress(qint32 progress, const QString &detail);

signals:
    /**
     * @brief sigProgressFinished 进度到100%时，发送此信号
     */
    void sigProgressFinished();

private:
    DWaterProgress *iconWidget = nullptr;
};

#endif // DRIVERICONWIDGET_H
