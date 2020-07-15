#include "DeviceListView.h"

#include <DApplicationHelper>

#include <qdrawutil.h>
#include <QPainter>
#include <QDebug>

#include "MacroDefinition.h"

DWIDGET_USE_NAMESPACE



DeviceListviewDelegate::DeviceListviewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
    setItemSpacing(10);
}

void DeviceListviewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data().toString() == "Seperator") {
        return paintSeparator(painter, option);
    }
    return DStyledItemDelegate::paint(painter, option, index);
}

QSize DeviceListviewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data().toString() == "Seperator") {
        return QSize(option.rect.width(), 5);
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



DeviceListView::DeviceListView(QWidget *parent)
    : DListView(parent), mp_ItemModel(new QStandardItemModel(this))
{
    // 设置View的Model
    setModel(mp_ItemModel);

    // 设置item的代理
    setItemDelegate(new DeviceListviewDelegate(this));

    // 设置item的其它属性
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setItemSpacing(0);
    setAutoFillBackground(true); //与背景色有关
    setViewportMargins(10, 2, 10, 5);
}

DeviceListView::~DeviceListView()
{
    DELETE_PTR(mp_ItemModel);
}

void DeviceListView::addItem(const QString &name, const QString &iconFile)
{
    DStandardItem *item = new DStandardItem;
    item->setText(name);
    item->setTextAlignment(Qt::AlignLeft);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    item->setIcon(QIcon::fromTheme(iconFile));
    mp_ItemModel->appendRow(item);
}

bool DeviceListView::curItemEnable()
{
    const QModelIndex &index =  this->currentIndex();
    int rowNum = index.row();
    QStandardItem *item = mp_ItemModel->item(rowNum);
    if (!item)
        return false;
    return item->isEnabled();
}

void DeviceListView::setCurItemEnable(bool enable)
{
    const QModelIndex &index =  this->currentIndex();
    int rowNum = index.row();
    QStandardItem *item = mp_ItemModel->item(rowNum);
    if (item)
        item->setEnabled(enable);
}

void DeviceListView::paintEvent(QPaintEvent *event)
{
    // 让背景色适合主题颜色
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::ItemBackground, pa.brush(DPalette::Base));
    DApplicationHelper::instance()->setPalette(this, pa);

    DListView::paintEvent(event);
}
