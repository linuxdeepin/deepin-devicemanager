#include "DetailViewDelegate.h"

#include <QPainter>
#include <QDebug>

#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>
#include <DFontSizeManager>

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
    rectpath.setX(rect.x() + 1);
    rectpath.setY(rect.y() + 1);
    rectpath.setWidth(rect.width() - 2);
    rectpath.setHeight(rect.height() - 2);
    path.addRect(rectpath);

    // 更多信息按钮行，背景色为白色，单元格上边框要绘制横线以
    if (index.row() == dynamic_cast<DetailTreeView *>(this->parent())->rowCount() - 1
            && index.row() != 0 && dynamic_cast<DetailTreeView *>(this->parent())->hasExpendInfo()) {
        // 展开 行背景色为白色
        painter->fillPath(path, palette.color(cg, DPalette::Base));

        // 展开行绘制横线
        QPen pen = painter->pen();
        pen.setColor(palette.color(cg, DPalette::FrameShadowBorder));
        pen.setWidth(1);
        painter->setPen(pen);
        painter->drawLine(rect.x(), rect.y() + 1, rect.x() + rect.width() - 1, rect.y() + 1);

    } else {
        painter->fillPath(path, background);
    }

    QRect textRect = rect;
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);   // 边距

    textRect.setX(textRect.x() + margin);

    QFont fo;
    // 表格第一列，字体加粗
    if (index.column() == 0) {
        fo = opt.font;
        fo.setWeight(63);
    } else {
        fo = DFontSizeManager::instance()->t8();
    }

    painter->setFont(fo);
    QFontMetrics fm(fo);
    QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());

    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);


    // 第一列 单元格后画竖线，展开收起行不画竖线
    if (index.column() == 0 && !(index.row() == dynamic_cast<DetailTreeView *>(this->parent())->rowCount() - 1
                                 && index.row() != 0 && dynamic_cast<DetailTreeView *>(this->parent())->hasExpendInfo())) {

        QPen pen = painter->pen();
        pen.setColor(palette.color(cg, DPalette::FrameShadowBorder));
        pen.setWidth(1);
        painter->setPen(pen);

        painter->drawLine(rect.topRight().x() + 1, rect.topRight().y(), rect.bottomRight().x() + 1, rect.bottomRight().y());
    }


    painter->restore();
//    QStyledItemDelegate::paint(painter, option, index);
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
