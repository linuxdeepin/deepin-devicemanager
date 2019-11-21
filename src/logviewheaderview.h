/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     LZ <zhou.lu@archermind.com>
 *
 * Maintainer: LZ <zhou.lu@archermind.com>
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

#ifndef SYSTEM_SERVICE_TABLE_HEADER_VIEW_H
#define SYSTEM_SERVICE_TABLE_HEADER_VIEW_H

#include <DHeaderView>
#include <DStyle>

DWIDGET_USE_NAMESPACE

class LogViewHeaderView : public DHeaderView
{
public:
    LogViewHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    QSize sizeHint() const override;

    inline int getSpacing() const { return m_spacing; }
    inline void setSpacing(int spacing) { m_spacing = spacing; }

protected:
    void paintEvent(QPaintEvent *e) override;
    virtual void paintSection(QPainter *painter, const QRect &rect,
                              int logicalIndex) const override;

private:
    //QStyleOptionViewItem viewOptions() const override;

private:
    int m_spacing {1};
};

#endif  // SYSTEM_SERVICE_TABLE_HEADER_VIEW_H
