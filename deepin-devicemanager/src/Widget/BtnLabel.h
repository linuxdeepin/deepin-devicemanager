/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
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

#ifndef BTNLABEL_H
#define BTNLABEL_H

#include <DLabel>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class BtnLabel : public DLabel
{
    Q_OBJECT
public:
    explicit BtnLabel(DWidget *parent = nullptr);

public:
    void setDesc(const QString &txt);

private:
    QString m_Desc;
};

#endif // BTNLABEL_H
