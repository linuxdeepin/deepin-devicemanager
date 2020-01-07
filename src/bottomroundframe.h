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
#ifndef BOTTOMROUNDFRAME_H
#define BOTTOMROUNDFRAME_H

#include <QFrame>
#include <QTextBrowser>
class BottomRoundFrame : public QFrame
{
    Q_OBJECT
public:
    explicit BottomRoundFrame(QWidget *parent = nullptr);
    void setTextBroser(QTextBrowser * textBrowser);
signals:

public slots:

private:
    QTextBrowser * m_textBrowser;

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
private:
    int m_roundedRadius;
};

#endif // BOTTOMROUNDFRAME_H
