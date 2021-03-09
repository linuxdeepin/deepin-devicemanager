/*
* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
*
* Author:     leiyu <leiyu@live.com>
* Maintainer: leiyu <leiyu@deepin.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TIPSWIDGET_H
#define TIPSWIDGET_H

#include <DWidget>

DWIDGET_USE_NAMESPACE
class TipsWidget : public DWidget
{
    Q_OBJECT
public:
    explicit TipsWidget(QWidget *parent = nullptr);

    /**
     * @brief setText: set Tips text
     * @param text: text
     */
    void setText(const QString &text);

    /**
     * @brief setAlignment:set alignment
     */
    void setAlignment(Qt::Alignment);

private:
    /**
     * @brief initWidget: init widget layout
     */
    void initWidget();

    /**
     * @brief adjustContent: adjust content
     * @param text:content
     */
    void adjustContent(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onUpdateTheme();

private:
    QString m_text; // text

    int m_lrMargin; // left and right margin

    int m_tbMargin; // top and bottom margin

    int m_maxLineCount; // content lines count

    Qt::Alignment m_alignment; // alignment
};

#endif // TIPSWIDGET_H
