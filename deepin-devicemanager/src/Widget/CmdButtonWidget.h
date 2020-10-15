/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     XiaoMei.Ji <jixiaomei@uniontech.com>
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

#ifndef CMDBUTTONWIDGET_H
#define CMDBUTTONWIDGET_H

#include <QObject>

#include <DWidget>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

class DetailTreeView;

/**
 * @brief The CmdButtonWidget class
 * 展开详细信息的按钮
 */
class CmdButtonWidget : public DWidget
{
    Q_OBJECT
public:
    explicit CmdButtonWidget(DetailTreeView *parent = nullptr);

signals:
    /**
     * @brief cmdButtonClicked:Button点击信号
     */
    void cmdButtonClicked();

public slots:
    /**
     * @brief expandCommandLinkClicked:展开button点击槽函数
     */
    void expandCommandLinkClicked();

protected:
    /**
     * @brief paintEvent:重绘事件
     * @param event:事件
     */
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief initUI:舒适化UI界面
     */
    void initUI();

private:
    DCommandLinkButton *mp_cmdButton;             // 展开/收起按钮

};

#endif // CMDBUTTONWIDGET_H
