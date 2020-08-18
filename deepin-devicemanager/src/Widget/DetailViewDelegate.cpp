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

    // 确定绘制区域的形状，单元格
    if (index.column() == 0) {
        // 第一列右空一个像素
        rectpath.setWidth(rect.width() - 1);
    }

    // 最后一行是更多信息按钮行，背景色为白色，单元格上边框要绘制横线以
    if (index.row() == dynamic_cast<DetailTreeView *>(this->parent())->rowCount() - 1
            && index.row() != 0 && dynamic_cast<DetailTreeView *>(this->parent())->hasExpendInfo()) {
        // 上方要空一个像素用来绘制横线
        rectpath.setY(rect.y() + 1);
        path.addRect(rectpath);

        // 展开 行背景色为白色
        painter->fillPath(path, palette.color(cg, DPalette::Base));


    } else {
        path.addRect(rectpath);
        painter->fillPath(path, background);
    }

    // 绘制文字信息
    QRect textRect = rectpath;
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);   // 边距

    textRect.setX(textRect.x() + margin);
    textRect.setWidth(textRect.width() - margin);

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

    if (dynamic_cast<DetailTreeView *>(this->parent())->isCurDeviceEnable()) {
        if (index.row() == 0 && index.column() == 1) {
            text = text.remove("(" + tr("Disable") + ")");
        }
    } else {
        if (index.row() == 0 && index.column() == 1) {
            text = "(" + tr("Disable") + ")" + text;
            QPen pen = painter->pen();
            pen.setColor(QColor("#FF5736"));
            painter->setPen(pen);
        }
    }

    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);


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
