/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Yaobin <yao.bin@archermind.com>
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
#include "bottomroundframe.h"
#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <DPalette>
#include <DApplication>
#include <QStyleOptionFrame>
#include <DApplicationHelper>
#include <QDebug>
DWIDGET_USE_NAMESPACE
BottomRoundFrame::BottomRoundFrame(QWidget *parent) : QFrame(parent),m_textBrowser(nullptr),m_roundedRadius(18)
{
    this->setFrameStyle(QFrame::Plain);
    this->setLineWidth(0);
    this->setFrameShape(QFrame::NoFrame);
    this->setAutoFillBackground(true);
    this->setMinimumHeight(1);
}

void BottomRoundFrame::setTextBroser(QTextBrowser *textBrowser)
{
   m_textBrowser = textBrowser;
//   setBackgroundRole(m_textBrowser->foregroundRole());
}


void BottomRoundFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setOpacity(1);
    painter.setClipping(true);

    QPainterPath roundedPath,topPath;
    QRect rect = this->rect();
    topPath.addRect(QRect(rect.x(),rect.y(),width(),height()/2));
    roundedPath.addRoundedRect(this->rect(),m_roundedRadius,m_roundedRadius);
    roundedPath += topPath;
    painter.setClipPath(roundedPath);

    painter.fillRect(QRect(rect.x(),rect.y(),rect.width(),rect.height()),m_textBrowser->viewport()->palette().base());
    painter.restore();

    QFrame::paintEvent(event);
}
