#include "DetailViewDelegate.h"

#include <QPainter>
#include <QDebug>

#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>
#include "DetailTreeView.h"

DWIDGET_USE_NAMESPACE

DetailViewDelegate::DetailViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void DetailViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            cg = DPalette::Active;
        }
    }

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());

    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QBrush background;

    if (opt.features & QStyleOptionViewItem::Alternate) {
        background = palette.color(cg, DPalette::AlternateBase);
    } else {
        background = palette.color(cg, DPalette::Base);
    }

    QRect rect = opt.rect;
    QPainterPath path;
    QRect rectpath = rect;
    rectpath.setWidth(rect.width() - 1);
    path.addRect(rectpath);

    if (index.row() == dynamic_cast<DetailTreeView *>(this->parent())->rowCount() - 1
            && index.row() != 0) {
        painter->fillPath(path, palette.color(cg, DPalette::Base));
        qDebug() << index.row();
    } else {
        painter->fillPath(path, background);
    }

    QRect textRect = rect;
    textRect.setX(textRect.x() + margin);

    QFontMetrics fm(opt.font);
    QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);

    if (index.column() == 0) {
        painter->setPen(palette.color(cg, DPalette::FrameShadowBorder));
        painter->drawLine(rect.topRight(), rect.bottomRight());
    }


    painter->restore();
}

QWidget *DetailViewDelegate::createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const
{
    return nullptr;
}

QSize DetailViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(50, size.height()));

    if (index.column()) {
        size.setWidth(std::max(150, size.width()));
    }
    return size;
}

void DetailViewDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->showDecorationSelected = true;
    bool ok = false;
    if (index.data(Qt::TextAlignmentRole).isValid()) {
        uint value = index.data(Qt::TextAlignmentRole).toUInt(&ok);
        option->displayAlignment = static_cast<Qt::Alignment>(value);
    }

    if (!ok)
        option->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    option->textElideMode = Qt::ElideRight;
    option->features = QStyleOptionViewItem::HasDisplay;
    if (index.row() % 2 == 0)
        option->features |= QStyleOptionViewItem::Alternate;
    if (index.data(Qt::DisplayRole).isValid())
        option->text = index.data().toString();
}
