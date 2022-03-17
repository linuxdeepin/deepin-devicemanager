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

#ifndef LONGTEXTLABEL_H
#define LONGTEXTLABEL_H

#include <QObject>

#include <DLabel>
#include <DWidget>

DWIDGET_USE_NAMESPACE

/**
 * @brief The LongTextLabel class
 * 封装富文本标签
 */
class LongTextLabel : public DLabel
{
    Q_OBJECT

public:
    explicit LongTextLabel(DWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;

private:
    int m_width;

};

#endif // LONGTEXTLABEL_H
