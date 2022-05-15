/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     XiaoMei.Ji <jixiaomei@uniontech.com>
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

#ifndef ANIMATIONLABEL_H
#define ANIMATIONLABEL_H

#include <DLabel>

#include <QWidget>
#include <QPixmap>

DWIDGET_USE_NAMESPACE

/**
 * @brief The AnimationLabel class   用于实现检测界面的动画
 */
class AnimationLabel : public DLabel
{
    Q_OBJECT
    Q_PROPERTY(int icon READ currentIcon WRITE setcurrentIcon)

public:
    explicit AnimationLabel(QWidget * parent = nullptr);

    /**
     * @brief setImageList: 设置图片列表
     * @param iconList:图片列表
     */
    void setImageList(const QStringList &iconList);

    /**
     * @brief setcurrentIcon: 设置当前icon
     * @param iconIndex: 图片Index
     */
    void setcurrentIcon(int iconIndex);

    /**
     * @brief currentIcon: 当前icon
     * @return 图片Index
     */
    int currentIcon();

private:
    QStringList  m_ImageList;       // 图片列表
    int          m_CurrentIndex;    // 当前Index

};

#endif // ANIMATIONLABEL_H
