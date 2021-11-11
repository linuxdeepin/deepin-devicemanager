/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
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

#include "cloumnwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "DLabel"
#include "DPalette"
#include "DApplicationHelper"
#include "DMenu"
#include "DApplication"
#include "../commondefine.h"
#include "QSizePolicy"
#include "../deviceinfoparser.h"
#include "DFontSizeManager"

DWIDGET_USE_NAMESPACE

bool ColumnWidget::isPaletteInit_ = false;
DPalette ColumnWidget::paHighlight_;
DPalette ColumnWidget::paNormal_;

ColumnLabel::ColumnLabel(const QString &text, ColumnWidget *parent): DLabel(text, parent), columnWidget_(parent)
{


}

//void ColumnLabel::mousePressEvent(QMouseEvent *event)
//{
//    //DLabel::mousePressEvent(event);
//    columnWidget_->labelMousePressEvent(event);
//}

//void ColumnLabel::contextMenuEvent(QContextMenuEvent *event)
//{
//    columnWidget_->labelContextMenuEvent(event);
//}

ColumnWidget::ColumnWidget(const QString &strLeft, const QString &strRight, const QFont &font, int fontSizetype, int columnHeight, bool isRightLink, Dtk::Widget::DWidget *parent, DeviceInfoWidgetBase *di): \
    DWidget(parent), deviceInfoWidget_(di)
{
    //setAutoFillBackground(true);

    QHBoxLayout *hly = new QHBoxLayout;

    setLayout(hly);

    hly->setSpacing(0);
    hly->setMargin(0);


    l1 = new ColumnLabel( strLeft, this);
    l1->setFont(font);
    DFontSizeManager::instance()->bind(l1, DFontSizeManager::SizeType(fontSizetype) );
    l1->setWordWrap(true);
    l1->setMinimumWidth(NameLength_);
    //l1->setAutoFillBackground(true);
    l1->setScaledContents(true);
    l1->setMinimumHeight(columnHeight);

    l2 = new ColumnLabel( strRight, this);
    l2->setFont(font);
    DFontSizeManager::instance()->bind(l2, DFontSizeManager::SizeType(fontSizetype) );
    l2->setWordWrap(true);
    int textWidth = QFontMetrics( font ).width(strRight);
    l2->setScaledContents(true);

    if ( isRightLink ) {
        l2->setOpenExternalLinks(true);
        //textWidth = QFontMetrics( font ).width(DeviceInfoParser::Instance().getOsInfo());
    }

    if (textWidth > DeviceWidgetContentWidth_) {
        textWidth = DeviceWidgetContentWidth_;
    }

    l2->setMinimumWidth( textWidth + 10);
    //l2->setAutoFillBackground(true);
    l2->setMinimumHeight(columnHeight);

    hly->addWidget(l1, 0, Qt::AlignmentFlag::AlignTop);
    hly->addWidget(l2, 0, Qt::AlignmentFlag::AlignTop);
    hly->addStretch();

    if (isPaletteInit_ == false) {
        isPaletteInit_ = true;
        changeTheme();
    }
}

//void ColumnWidget::labelMousePressEvent(QMouseEvent *event)
//{
//    if(deviceInfoWidget_)
//    {
//        deviceInfoWidget_->selectColumnWidget(this);
//    }

//    if(event->button() == Qt::MouseButton::LeftButton )
//    {
//        setHilight(!st);
//    }

//    else if(event->button() == Qt::MouseButton::RightButton )
//    {
//        setHilight(true);
//    }

//    //DWidget::mousePressEvent(event);
//}

void ColumnWidget::labelContextMenuEvent(QContextMenuEvent *event)
{
    DMenu *contextMenu;

    deviceInfoWidget_->getContextMenu(&contextMenu);

    contextMenu->setAutoFillBackground(true);

    QAction copyAction(QObject::tr("Copy"), this);
    connect(&copyAction, &QAction::triggered, \
    [this]() {
        QClipboard *clipboard = DApplication::clipboard(); //获取系统剪贴板指针
        clipboard->setText(l1->text() + l2->text());//设置剪贴板内容
    }
           );

    QList<QAction *> lst = contextMenu->actions();

    contextMenu->clear();

    contextMenu->addAction( &copyAction);
    contextMenu->addSeparator();
    contextMenu->addActions(lst);

    contextMenu->exec(event->globalPos());

    contextMenu->clear();
    contextMenu->addActions(lst);
}

void ColumnWidget::setHilight(bool highLight)
{
    //setAutoFillBackground(highLight);
    DApplicationHelper::instance()->setPalette(l1, highLight ? paHighlight_ : paNormal_);
    if (l2->openExternalLinks() == true ) {
        if (highLight) {
            //l2->setText( DeviceInfoParser::Instance().getOsInfo() );
        }
//        else
//        {
//            l2->setText( DeviceInfoParser::Instance().getOsHtmlInfo() );
//        }
    }

    DApplicationHelper::instance()->setPalette(l2, highLight ? paHighlight_ : paNormal_);

    st = highLight;
}

void ColumnWidget::changeTheme()
{
    paNormal_ = DApplicationHelper::instance()->palette(this);

    //auto color1 = paNormal_.highlight().color();

    paHighlight_.setBrush(QPalette::Background, paNormal_.highlight());
    paHighlight_.setBrush(QPalette::WindowText, paNormal_.highlightedText());
    paHighlight_.setBrush(QPalette::Link, paNormal_.highlightedText() );

    setHilight(false);
}
