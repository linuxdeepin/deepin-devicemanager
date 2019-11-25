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

#include "devicelistview.h"
#include <QStandardItemModel>
#include <QMargins>
#include <QVariant>
#include <QPainter>
#include <qdrawutil.h>
#include "DApplicationHelper"
#include "DMenu"
#include "DApplication"
#include "mainwindow.h"
#include <QDir>
#include "DFileDialog"
#include <QDateTime>
#include <QContextMenuEvent>
#include "commondefine.h"

DWIDGET_USE_NAMESPACE

Q_DECLARE_METATYPE(QMargins)

DeviceListviewDelegate::DeviceListviewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
    //setMargins(QMargins(10, 2, 10, 10));
    setItemSpacing(10);
}

void DeviceListviewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if( index.data().toString() == "Seperator" )
    {
        return paintSeparator(painter, option);
    }

    return DStyledItemDelegate::paint(painter, option, index);
}

QSize DeviceListviewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if( index.data().toString() == "Seperator" )
    {
        return QSize( option.rect.width(), 5);
    }

    return DStyledItemDelegate::sizeHint(option, index);
}

void DeviceListviewDelegate::paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();

    int yPoint = option.rect.top() + option.rect.height() / 2;
    qDrawShadeLine(painter, 0, yPoint, option.rect.width() - 0, yPoint, option.palette);

    painter->restore();
}

DeviceListView::DeviceListView(DWidget* parent):DListView(parent)
{
//    setResizeMode(QListView::Adjust);
//    setOrientation(QListView::LeftToRight, true);
//    setTextElideMode(Qt::ElideMiddle);
//    setSelectionBehavior(QAbstractItemView::SelectRows);

    //DListView::setSelectionRectVisible(false);

    navModel_ = new QStandardItemModel(navModel_);

    setVerticalScrollMode(ScrollPerPixel);
    //setIconSize(QSize(24, 24));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    setMouseTracking(true);

    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(false);

    setModel(navModel_);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setFrameShape(QFrame::Shape::NoFrame);

    setEditTriggers(QListView::NoEditTriggers);
    setResizeMode(QListView::Adjust);

    //setContentsMargins( 20, 8, 20, 8);
    //setAutoScroll(false);

    //setSpacing(0);
    //setViewMode(QListView::ListMode);
    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    //viewport()->setAutoFillBackground(true);

    setFocus(Qt::FocusReason::NoFocusReason);

    //DPalette pa = DApplicationHelper::instance()->palette(this);
    //auto ba = QPalette::Base;
    //ba.setBrush(DPalette::ItemBackground, palette().base());
    //setBackgroundRole(QPalette::Base);
    //setAutoFillBackground(true);

    setItemDelegate( new DeviceListviewDelegate(this) );

    setBackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground);

    //setMar


    auto modifyTheme = [this](){
//        DPalette pa;
//        pa.setColor(DPalette::Background,QColor(0, 248, 248));
//        setAutoFillBackground(true);
//        setPalette(pa);

        DPalette pa = DApplicationHelper::instance()->palette(this);
        QColor base_color = palette().base().color();

        pa.setBrush(DPalette::ItemBackground, base_color);
        pa.setColor(QPalette::Background, base_color);

        setPalette(pa);

        DApplicationHelper::instance()->setPalette(this, pa);
        //DApplicationHelper::instance()->setPalette(this->viewport(), pa);

        changeThemeIcon();
    };

    modifyTheme();

    this->setAutoFillBackground(true);

    //this->setBackgroundRole(DPalette::Base);

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, modifyTheme);
    //setMaximumWidth(150);

    initContextMenu();

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(OnlvOpRightBtn(const QPoint&)));

    setItemSpacing(0);

    setIconSize( QSize(20, 20) );

    setViewportMargins(10,2,10,5);
    this->viewport()->setAutoFillBackground(false);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    DFontSizeManager::instance()->bind(this, DFontSizeManager::T6);
}

void DeviceListView::addDevice(const QString& deviceName, const QString& iconFile)
{
    DStandardItem* item = new DStandardItem;
    //DFontSizeManager::instance()->bind(nameLabel, DFontSizeManager::T8);
    //QFont itemFont = item->font();
    //itemFont.setPixelSize(14);
    //itemFont.setWeight(QFont::Medium);
    //itemFont.setFamily(commonFontFamily_);

    //item->setFont(itemFont);

    item->setSizeHint( QSize(20, 38) );

    DGuiApplicationHelper::ColorType ct = DApplicationHelper::instance()->themeType();

    QString icon = ":images/";
    icon += ((ct == DGuiApplicationHelper::LightType) ? "light/normal/":"dark/normal/");
    icon += iconFile;

    item->setIcon(QIcon(icon));
    item->setData(icon, Qt::UserRole+89);

    item->setText(deviceName);

    item->setTextAlignment(Qt::AlignLeft);

    const QMargins ListViweItemMargin(10, 8, 0, 8);
    //const QMargins ListViweItemMargin( 10, 5, 10, 5 );
    const QVariant VListViewItemMargin = QVariant::fromValue(ListViweItemMargin);

    item->setData(VListViewItemMargin, Dtk::MarginsRole);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);

    navModel_->appendRow(item);
}

void DeviceListView::addSeperator()
{
    DStandardItem* item = new DStandardItem;
    //item->setBackgroundRole(QPalette::Base);
    //item->setAutoFillBackground(true);
    item->setText("Seperator");
    navModel_->appendRow(item);
    item->setCheckable(false);
    item->setSelectable(false);
    item->setEnabled(false);
}

QString DeviceListView::currentDevice()
{
    return currentDevice_;
}

void DeviceListView::setCurrentDevice(const QString& device)
{
    currentDevice_ = device;
}

void DeviceListView::setFistSelected()
{
    QModelIndex index = navModel_->index(0, 0);
//    selectionModel()->select(index, QItemSelectionModel::Select);

//    QStandardItem* currentItem = navModel_->itemFromIndex(index);
//    if(currentItem)
//    {
//        QString icon = currentItem->data(Qt::UserRole+89).toString();
//        if(icon.isEmpty() == false)
//        {
//            icon.replace("normal", "press");
//            currentItem->setIcon(QIcon(icon));
//            currentItem->setData(icon, Qt::UserRole+89);
//        }
//    }

    setCurrentIndex(index);
}

QString DeviceListView::indexString(int index)
{
    return navModel_->index(index, 0).data().toString();
}

void DeviceListView::setAllItemsEnable(bool eable)
{
    for(int i = 0; i < navModel_->rowCount(); ++i)
    {
        QStandardItem* item = navModel_->item(i);
        {
            if(item && item->isEnabled() != eable && item->isSelectable() )
            {
                item->setEnabled(eable);
            }
        }
    }
}

bool DeviceListView::onExportToFile()
{
//   QString selectFilter;

//   QString saveDir = "./";
//   QDir dir( QDir::homePath() + "/Documents/");
//   if(dir.exists())
//   {
//        saveDir = QDir::homePath() + "/Documents/";
//   }

//   QString exportFile = DFileDialog::getSaveFileName(this,
//                                                     DApplication::translate("Main", "Export"), saveDir + DApplication::translate("Main", "deviceInfo") + \
//                                                     QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") .remove(QRegExp("\\s")) + ".txt", \
//                                                    tr("Text (*.txt);; Doc (*.doc);; Xls (*.xls);; Html (*.html)"), &selectFilter);

//   if(exportFile.isEmpty() == true)
//   {
//       return true;
//   }

   MainWindow* mainWindow = dynamic_cast<MainWindow*>(this->parent()->parent());
   if( nullptr == mainWindow )
   {
       return false;
   }

   return mainWindow->exportTo();
}

void DeviceListView::OnlvOpRightBtn( const QPoint& point)
{
    MainWindow* mainWindow = dynamic_cast<MainWindow*>(parent()->parent());
    if(mainWindow)
    {
        refreshAction_->setDisabled(mainWindow->isRefreshing());
    }

    contextMenu_->exec(mapToGlobal(point));
}

void DeviceListView::changeThemeIcon()
{
    auto type = DApplicationHelper::instance()->themeType();

    QString currentFolderName = (type == DApplicationHelper::DarkType )? "/dark/":"/light/";
    QString oldFolderName = (type == DApplicationHelper::DarkType) ? "/light/":"/dark/";

    for(int i = 0; i < navModel_->rowCount(); ++i)
    {
        QStandardItem* item = navModel_->item(i);
        {
            if(item)
            {
                QString icon = item->data(Qt::UserRole+89).toString();
                if(icon.isEmpty() == false)
                {
                    icon.replace(oldFolderName, currentFolderName);
                    item->setIcon(QIcon(icon));
                    item->setData(icon, Qt::UserRole+89);
                }
            }
        }
    }
}

void DeviceListView::initContextMenu()
{
    contextMenu_ = new DMenu(this);

    contextMenu_->setMinimumWidth(contextMenuWidth_);

    refreshAction_ = new QAction( QIcon::fromTheme("view-refresh"), DApplication::translate("Main", "Refresh"), this);
    connect(refreshAction_, &QAction::triggered, \
            [this]()
            {
                MainWindow* mainWindow = dynamic_cast<MainWindow*>(parent()->parent());
                if(mainWindow)
                {
                    mainWindow->refresh();
                }
            }
    );
    contextMenu_->addAction(refreshAction_);


    QAction* exportAction = new QAction(QIcon::fromTheme("document-new"), DApplication::translate("Main", "Export(E)"));

    connect(exportAction, &QAction::triggered, this, &DeviceListView::onExportToFile);

    contextMenu_->addAction(exportAction);
}

void DeviceListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QStandardItem* currentItem = navModel_->itemFromIndex(current);
    if(currentItem)
    {
        QString icon = currentItem->data(Qt::UserRole+89).toString();
        if(icon.isEmpty() == false)
        {
            icon.replace("normal", "press");
            currentItem->setIcon(QIcon(icon));
            currentItem->setData(icon, Qt::UserRole+89);
        }
    }

    QStandardItem* previousItem = navModel_->itemFromIndex(previous);
    if(previousItem)
    {
        QString icon = previousItem->data(Qt::UserRole+89).toString();
        if(icon.isEmpty() == false)
        {
            icon.replace("press", "normal");
            previousItem->setIcon(QIcon(icon));
            previousItem->setData(icon, Qt::UserRole+89);
        }
    }

    DListView::currentChanged(current, previous);

    QString device = current.data().toString();
    MainWindow* mainWindow = dynamic_cast<MainWindow*>(this->parent()->parent());
    if( nullptr == mainWindow )
    {
        return;
    }

    return mainWindow->currentDeviceChanged(device);
}

//void DeviceListView::contextMenuEvent(QContextMenuEvent *event)
//{
//    contextMenu_->exec(event->globalPos());
//}


