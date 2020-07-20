#include "HeaderTableView.h"

#include <DFontSizeManager>
#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>
#include <DHeaderView>

#include <QPainter>
#include <QDebug>

#include "MacroDefinition.h"






HeaderView::HeaderView(Qt::Orientation orientation, QWidget *parent)
    : DHeaderView(orientation, parent)
{
    // 设置无边框
    setFrameStyle(QFrame::NoFrame);
}

void HeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 获取系统边距
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    QStyleOptionHeader option;
    initStyleOption(&option);
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    // 绘制文本 *******************************************************************************
    // 计算文本在一个item里面的绘制的区域
    QRect txtRect(rect.x() + margin, rect.y(), rect.width(), rect.height());
    // 定义文本的剧中方式
    int align = Qt::AlignLeft | Qt::AlignVCenter;
    // 设置绘制文本的笔
    QPen txtPen;
    txtPen.setColor(palette.color(cg, DPalette::Text));
    painter->setPen(txtPen);
    // 获取文本
    QString txt = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
    // 绘制文本
    painter->drawText(txtRect, static_cast<int>(align), txt);


    // 绘制表头下边的那条线,其实就是绘制一个及细的巨型 ********************************************************************************
    // 计算该巨型
    QRect bottomLine(rect.x(), rect.height() - LINE_WIDTH, rect.width(), LINE_WIDTH);
    // 设置画笔
    QBrush bottomLineBrush(palette.color(cg, DPalette::FrameBorder));
    painter->fillRect(bottomLine, bottomLineBrush);

    // 绘制没一列之间的那条竖线
    int space = 5;  // 竖线到表头上下的距离
    QRect verticalLine(rect.x() - LINE_WIDTH, rect.y() + space, LINE_WIDTH, rect.height() - space * 2);
    painter->fillRect(verticalLine, bottomLineBrush);


    painter->restore();
}

void HeaderView::paintEvent(QPaintEvent *e)
{
    QPainter painter(viewport());
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = viewport()->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取系统默认的圆角半径
    QStyleOptionHeader option;
    initStyleOption(&option);
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 开始绘制背景色 *********************************************************
    // 绘制之前需要clear之前的颜色,使其与窗口色保持一直
    QBrush clearBrush(palette.color(cg, DPalette::Window));
    painter.fillRect(rect, clearBrush);
    // 计算绘制区域
    QPainterPath paintPath, subPath;
    paintPath.addRoundedRect(QRect(rect.x(), rect.y(), rect.width(), rect.height() * 2), radius, radius);
    subPath.addRect(QRect(rect.x(), rect.y() + rect.height(), rect.width(), rect.height()));
    paintPath = paintPath.subtracted(subPath);
    // 绘制
    QBrush bgBrush(palette.color(cg, DPalette::Base));
    painter.fillPath(paintPath, bgBrush);

    // 开始绘制边框线
    // 设置画笔
    QPen linePen;
    linePen.setWidth(LINE_WIDTH);
    linePen.setColor(palette.color(cg, DPalette::FrameShadowBorder));
    painter.setPen(linePen);
    painter.drawPath(paintPath);

    painter.restore();
    DHeaderView::paintEvent(e);
}




TableViewItemDelegate::TableViewItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void TableViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);
    QRect rect = option.rect;

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取系统边距
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    QStyleOptionViewItem optionTmp = option;
    initStyleOption(&optionTmp, index);

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 绘制每个item的背景色 ************************************************
    // 获取隔行的背景色
    QBrush bgBrush;
    if (option.features & QStyleOptionViewItem::Alternate) {
        bgBrush = palette.color(cg, DPalette::AlternateBase);
    } else {
        bgBrush = palette.color(cg, DPalette::Base);
    }
    // 计算绘制区域
    QPainterPath path;
//    switch (optionTmp.viewItemPosition) {  // 判断这个item是两边的还是，中间的
//    case QStyleOptionViewItem::Beginning: {
//        rect.setX(rect.x() + margin);
//        path.addRect(rect);
//    } break;
//    case QStyleOptionViewItem::Middle: {
//        path.addRect(rect);
//    } break;
//    case QStyleOptionViewItem::End: {
//        path.addRect(rect);
//    } break;
//    case QStyleOptionViewItem::OnlyOne: {
//        path.addRect(rect);
//    } break;
//    default: {
//        painter->restore();
//        QStyledItemDelegate::paint(painter, option, index);
//        return;
//    }
//}
    rect.setX(rect.x() + margin);
    rect.setY(rect.y() + margin);
    path.addRect(rect);
    painter->fillPath(path, bgBrush);

// 绘制文本 ***********************************************************
    rect = option.rect;
    QRect txtRect = QRect(rect.x() + margin, rect.y(), rect.width(), rect.height());
    QFontMetrics fm(option.font);
    int leftSpace = fm.width(QString("xx")); // ... 后面留间隙
    QString txt = fm.elidedText(index.data().toString(), option.textElideMode, txtRect.width() - leftSpace);
    QPen txtPen;
    txtPen.setColor(palette.color(cg, DPalette::Text));
    painter->setPen(txtPen);
    painter->drawText(txtRect, Qt::TextSingleLine | static_cast<int>(option.displayAlignment), txt);


    painter->restore();
}

QWidget *TableViewItemDelegate::createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const
{
    return nullptr;
}

QSize TableViewItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(36, size.height()));
    return size;
}

void TableViewItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
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


HeaderTableView::HeaderTableView(QWidget *parent)
    : DTableView(parent)
    , mp_Model(new QStandardItemModel(this))
    , mp_HeaderView(new HeaderView(Qt::Horizontal, this))
    , mp_ItemDelegate(new TableViewItemDelegate(this))
{
    initWidgets();
}

HeaderTableView::~HeaderTableView()
{
    DELETE_PTR(mp_Model);
}

void HeaderTableView::setHeaderLabels(const QStringList &lst)
{
    if (mp_Model) {
        mp_Model->setHorizontalHeaderLabels(lst);
    }
}

void HeaderTableView::setItem(int row, int column, DStandardItem *item)
{
    if (mp_Model) {
        mp_Model->setItem(row, column, item);

        // 设置每一行等宽
        int colCount = mp_HeaderView->count();
        int avgColWidth = width() / colCount;
        for (int i = 0; i < colCount; i++) {
            setColumnWidth(i, avgColWidth);
        }
    }
}

void HeaderTableView::clear()
{
    if (mp_Model) {
        mp_Model->clear();
    }
}

void HeaderTableView::paintEvent(QPaintEvent *e)
{
    QPainter painter(viewport());
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = viewport()->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取系统默认的圆角半径
    QStyleOptionFrame option;
    initStyleOption(&option);
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    QPainterPath paintPath, subPath;
    paintPath.addRoundedRect(QRect(rect.x(), rect.y() - rect.height(), rect.width(), rect.height() * 2), radius, radius);
    subPath.addRect(QRect(rect.x(), rect.y() - rect.height(), rect.width(), rect.height()));
    paintPath = paintPath.subtracted(subPath);
    // 设置画笔
    QPen linePen;
    linePen.setWidth(LINE_WIDTH);
    linePen.setColor(palette.color(cg, DPalette::FrameShadowBorder));
    painter.setPen(linePen);
    painter.drawPath(paintPath);

    painter.restore();
    DTableView::paintEvent(e);
}

void HeaderTableView::initWidgets()
{
    //设置无边框
    setFrameStyle(QFrame::NoFrame);
    //设置竖直表头不可见
    verticalHeader()->setVisible(false);
    // 设置水平表头
    mp_HeaderView->setFixedHeight(ROW_HEIGHT);
    setHorizontalHeader(mp_HeaderView);
    setShowGrid(false);
    // 绑定 model
    setModel(mp_Model);
    // 设置委托
    setItemDelegate(mp_ItemDelegate);
    // 绑定到系统字体
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
    // 设置隔行变色
    setAlternatingRowColors(true);
    // 设置表格可以排序
    setSortingEnabled(true);
    // 设置不可编辑
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置表头等宽
//    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    // 设置表的每一行高度
    verticalHeader()->setDefaultSectionSize(ROW_HEIGHT);
}
