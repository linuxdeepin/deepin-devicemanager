// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceListView.h"
#include "DDLog.h"

// Dtk头文件
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DPaletteHelper>

// Qt库文件
#include <qdrawutil.h>
#include <QPainter>
#include <QLoggingCategory>
#include <QKeyEvent>
#include <QPainterPath>

// 其它头文件
#include "MacroDefinition.h"

DWIDGET_USE_NAMESPACE

using namespace DDLog;

DeviceListviewDelegate::DeviceListviewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
    qCDebug(appLog) << "DeviceListviewDelegate instance created";
    //setItemSpacing(10);
}

void DeviceListviewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // qCDebug(appLog) << "Painting DeviceListviewDelegate for index:" << index.row();
    if (index.data().toString() == "Separator") {
        qCDebug(appLog) << "Painting separator";
        return paintSeparator(painter, option);
    }
    return DStyledItemDelegate::paint(painter, option, index);
}

QSize DeviceListviewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // qCDebug(appLog) << "Getting size hint for index:" << index.row();
    if (index.data().toString() == "Separator") {
        qCDebug(appLog) << "Setting size hint for separator";
        return QSize(option.rect.width(), 5);
    }
    return DStyledItemDelegate::sizeHint(option, index);
}

void DeviceListviewDelegate::paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const
{
    // qCDebug(appLog) << "Painting separator line";
    painter->save();

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        qCDebug(appLog) << "No active window";
        cg = DPalette::Inactive;
    } else {
        qCDebug(appLog) << "Window is active";
        cg = DPalette::Active;
    }

    auto *dAppHelper = DGuiApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();


    int yPoint = option.rect.top() + option.rect.height() / 2;

    QRect paintRect(0, yPoint, option.rect.width(), 2);
    QPainterPath path;
    path.addRect(paintRect);

    QBrush bgBrush(palette.color(cg, DPalette::FrameBorder));
    painter->fillPath(path, bgBrush);
    painter->restore();
}



DeviceListView::DeviceListView(QWidget *parent)
    : DListView(parent), mp_ItemModel(new QStandardItemModel(this))
{
    qCDebug(appLog) << "DeviceListView instance created";
    // 设置View的Model
    setModel(mp_ItemModel);

    // 设置item的代理
    setItemDelegate(new DeviceListviewDelegate(this));

    // 设置item的其它属性
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setItemSpacing(0);
    setAutoFillBackground(true); //与背景色有关

    // bug51444左侧菜单和滚动条不重叠重叠
    setViewportMargins(11, 2, 11, 5);

    setMovement(QListView::Static);
}

DeviceListView::~DeviceListView()
{
    // qCDebug(appLog) << "DeviceListView instance destroyed";
    DELETE_PTR(mp_ItemModel);
}

void DeviceListView::addItem(const QString &name, const QString &iconFile)
{
    qCDebug(appLog) << "Adding item:" << name << "icon:" << iconFile;

    QStringList lst = iconFile.split("##");
    if (lst.size() != 2) {
        qCWarning(appLog) << "Invalid icon file format";
        return;
    }

    DStandardItem *item = new DStandardItem;
    item->setData(name, Qt::DisplayRole);
    item->setData(lst[1], Qt::UserRole);
    item->setTextAlignment(Qt::AlignVCenter);
    if (name != QString("Separator")) {
        item->setToolTip(name);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    } else {
        qCDebug(appLog) << "Item is a separator, setting flags accordingly";
        item->setFlags(Qt::ItemNeverHasChildren);
    }
    item->setIcon(QIcon::fromTheme(lst[0]));
    setIconSize(QSize(20, 20));
    mp_ItemModel->appendRow(item);
}

void DeviceListView::setCurItemEnable(bool enable)
{
    qCDebug(appLog) << "Setting current item enable state to:" << enable;
    const QModelIndex &index =  this->currentIndex();
    int rowNum = index.row();
    QStandardItem *item = mp_ItemModel->item(rowNum);
    if (item)
        item->setEnabled(enable);
}

void DeviceListView::setCurItem(const QString &str)
{
    qCDebug(appLog) << "Setting current item to:" << str;
    if (!mp_ItemModel) {
        qCWarning(appLog) << "Item model is null";
        return;
    }

    QList<QStandardItem *> lstItems = mp_ItemModel->findItems(str);
    if (lstItems.size() != 1) {
        qCWarning(appLog) << "Found" << lstItems.size() << "items for" << str << ", expected 1";
        return;
    }

    QModelIndex index = mp_ItemModel->indexFromItem(lstItems[0]);
    setCurrentIndex(index);
}

QString DeviceListView::getConcatenateStrings(const QModelIndex &index)
{
    qCDebug(appLog) << "Getting concatenated strings for index:" << index.row();
    QStandardItem *item = mp_ItemModel->item(index.row());
    if (item) {
        return item->data(Qt::DisplayRole).toString();
    }
    qCWarning(appLog) << "Item not found at index" << index.row();
    return "";
}

void DeviceListView::clearItem()
{
    qCDebug(appLog) << "Clearing all items in DeviceListView";
    mp_ItemModel->clear();
}

void DeviceListView::paintEvent(QPaintEvent *event)
{
    // qCDebug(appLog) << "DeviceListView paint event called.";
    // 让背景色适合主题颜色
    DPalette pa = DPaletteHelper::instance()->palette(this);
    pa.setBrush(DPalette::ItemBackground, pa.brush(DPalette::Base));
    DPaletteHelper::instance()->setPalette(this, pa);

    DListView::paintEvent(event);
}

void DeviceListView::mousePressEvent(QMouseEvent *event)
{
    // qCDebug(appLog) << "Mouse pressed at position:" << event->pos();
    if ((QApplication::keyboardModifiers() == Qt::ControlModifier) && (event->button() == Qt::LeftButton)) {
        // 当键盘按住ctrl，不响应鼠标点击事件
        qCDebug(appLog) << "Ctrl key pressed, ignoring mouse press event";
    } else {
        DListView::mousePressEvent(event);
    }
}

void DeviceListView::mouseMoveEvent(QMouseEvent *event)
{
    // qCDebug(appLog) << "Mouse moved to position:" << event->pos();
    if ((QApplication::keyboardModifiers() == Qt::ControlModifier)/* && (event->button() == Qt::LeftButton)*/) {
        // 当键盘按住ctrl，不响应鼠标移动事件
        qCDebug(appLog) << "Ctrl key pressed, ignoring mouse move event";
    } else {
        DListView::mouseMoveEvent(event);
    }
}

void DeviceListView::keyPressEvent(QKeyEvent *keyEvent)
{
    // qCDebug(appLog) << "Key pressed:" << keyEvent->key();

    DListView::keyPressEvent(keyEvent);

    // 当前Item 为Separator时，需要跳过Separator
    if (this->currentIndex().data(Qt::DisplayRole) == "Separator") {
        qCDebug(appLog) << "Current item is a separator, handling key events for navigation";
        // 按键：下一个
        if (keyEvent->key() == Qt::Key_Down) {
            int curRow = this->currentIndex().row();
            qCDebug(appLog) << "Key down on separator, moving to next item";

            // 当前 Separator 不是最后一个Item，显示下一个Item
            if (curRow != mp_ItemModel->rowCount() - 1) {
                QModelIndex index = this->currentIndex().siblingAtRow(curRow + 1);
                this->setCurrentIndex(index);
            }
        }

        // 按键：上一个
        if (keyEvent->key() == Qt::Key_Up) {
            int curRow = this->currentIndex().row();
            qCDebug(appLog) << "Key up on separator, moving to previous item";
            // 当前 Separator 不是第一个Item，显示上一个Item
            if (curRow != 0) {
                QModelIndex index = this->currentIndex().siblingAtRow(curRow - 1);
                this->setCurrentIndex(index);
            }
        }
    }

    // 切换对应设备信息页面
    emit clicked(this->currentIndex());
}
