/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
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
#include "commondefine.h"
#include "QSizePolicy"

DWIDGET_USE_NAMESPACE

ColumnLabel::ColumnLabel(const QString& text, ColumnWidget* parent): DLabel(text, parent),columnWidget_(parent)
{

}

void ColumnLabel::mousePressEvent(QMouseEvent *event)
{
    columnWidget_->labelMousePressEvent(event);
}

void ColumnLabel::contextMenuEvent(QContextMenuEvent *event)
{
    columnWidget_->labelContextMenuEvent(event);
}

ColumnWidget::ColumnWidget(const QString& strLeft, const QString& strRight, const QFont& font, int columnHeight, Dtk::Widget::DWidget* parent , DeviceInfoWidgetBase* di):\
    DWidget(parent), deviceInfoWidget_(di)
{
    setAutoFillBackground(true);

    QHBoxLayout* hly = new QHBoxLayout;

    setLayout(hly);

    hly->setSpacing(0);

    hly->setMargin(0);


    l1 = new ColumnLabel( strLeft, this);
    l1->setFont(font);
    l1->setWordWrap(true);
    l1->setMinimumWidth(NameLength_);
    l1->setAutoFillBackground(true);
    l1->setMinimumHeight(columnHeight);

    l2 = new ColumnLabel( strRight, this);
    l2->setFont(font);
    l2->setWordWrap(true);
    int textWidth = QFontMetrics( font ).width(strRight) + 10;
    if(textWidth > DeviceWidgetContentWidth_)
    {
        textWidth = DeviceWidgetContentWidth_;
    }
    l2->setMinimumWidth( textWidth );
    l2->setAutoFillBackground(true);
    l2->setMinimumHeight(columnHeight);

    hly->addWidget(l1);
    hly->addWidget(l2);
    hly->addStretch();
}

void ColumnWidget::labelMousePressEvent(QMouseEvent *event)
{
    if(deviceInfoWidget_)
    {
        deviceInfoWidget_->selectColumnWidget(this);
    }

    if(event->button() == Qt::MouseButton::LeftButton )
    {
        setHilight(!st);
    }

    else if(event->button() == Qt::MouseButton::RightButton )
    {
        setHilight(true);
    }

    //DWidget::mousePressEvent(event);
}

void ColumnWidget::labelContextMenuEvent(QContextMenuEvent *event)
{
    DMenu* contextMenu;

    deviceInfoWidget_->getContextMenu(&contextMenu);

    contextMenu->setAutoFillBackground(true);

    QAction copyAction(DApplication::translate("Main", "Copy"), this);
    connect(&copyAction, &QAction::triggered, \
            [this]()
            {
                QClipboard *clipboard = QApplication::clipboard(); //获取系统剪贴板指针
                clipboard->setText(l1->text() + l2->text());//设置剪贴板内容
            }
    );

    QList<QAction*> lst = contextMenu->actions();

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

    DPalette pa = DApplicationHelper::instance()->palette(this);
    QColor link = pa.link().color();

    if(highLight == false )
    {
        pa.setBrush(QPalette::Background, pa.background());
        pa.setBrush(QPalette::WindowText, pa.windowText());
        pa.setBrush(QPalette::Link, pa.link() );
    }
    else
    {
        pa.setBrush(QPalette::Background, pa.highlight());
        pa.setBrush(QPalette::WindowText, pa.highlightedText());
        pa.setBrush(QPalette::Link, pa.highlightedText() );
    }

    DApplicationHelper::instance()->setPalette(l1, pa);
    DApplicationHelper::instance()->setPalette(l2, pa);

    st = highLight;
}

void ColumnWidget::changeTheme()
{
    setHilight(false);
}
