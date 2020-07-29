#include "TableWidget.h"

#include <DFontSizeManager>
#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>
#include <DHeaderView>

#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>

#include "MacroDefinition.h"
#include "logviewitemdelegate.h"
#include "logtreeview.h"



TableWidget::TableWidget(QWidget *parent)
    : DWidget(parent)
    , mp_Table(new LogTreeView(this))
{
    initWidget();

    // 连接信号和曹函数
    connect(mp_Table, &LogTreeView::clicked, this, &TableWidget::slotItemClicked);
}

void TableWidget::setHeaderLabels(const QStringList &lst)
{
    if (mp_Table) {
        mp_Table->setHeaderLabels(lst);
    }
}

void TableWidget::setItem(int row, int column, DStandardItem *item)
{
    if (mp_Table) {
        mp_Table->setItem(row, column, item);
    }
}

void TableWidget::setColumnAverage()
{
    if (mp_Table) {
        qDebug() << "*************PageTableHeader::updateTable**************" << mp_Table->width();
        mp_Table->setColumnAverage();
    }
}

void TableWidget::clear()
{
    if (mp_Table) {
        mp_Table->clear();
    }
}

void TableWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取系统默认的圆角半径
//    QStyleOptionFrame option;
//    initStyleOption(&option);
//    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
//    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);
    int radius = 8;

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
    int width = 2;
    QPainterPath paintPath, paintPathOut, paintPathIn;
    paintPathOut.addRoundedRect(rect, radius, radius);

    QRect rectIn = QRect(rect.x() + width, rect.y() + width, rect.width() - width * 2, rect.height() - width * 2);
    paintPathIn.addRoundedRect(rectIn, radius, radius);

    paintPath = paintPathOut.subtracted(paintPathIn);

    QBrush bgBrush(palette.color(cg, DPalette::FrameShadowBorder));
    painter.fillPath(paintPath, bgBrush);

    painter.restore();
    DWidget::paintEvent(e);
}

void TableWidget::slotItemClicked(const QModelIndex &index)
{
    int row = index.row();
    if (row >= 0) {
        emit itemClicked(row);
    }
}

void TableWidget::initWidget()
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    int margin = 3;
    hLayout->setContentsMargins(margin, margin, margin, margin);
    hLayout->addWidget(mp_Table);
    setLayout(hLayout);
}
