#include "ContentTableView.h"
#include "MacroDefinition.h"

#include <DApplicationHelper>
#include <DApplication>

#include <QPainter>
#include <QVBoxLayout>

#include "DetailTreeView.h"

ContentWidget::ContentWidget(QWidget *parent)
    : DWidget(parent)
    , mp_Table(new DetailTreeView(this))
{
    initWidget();
}

void ContentWidget::setItem(int row, int column, DStandardItem *item)
{
//    if (mp_Table) {
//        mp_Table->setItem(row, column, item);
//    }
}

void ContentWidget::clear()
{
    if (mp_Table) {
        mp_Table->clear();
    }
}

void ContentWidget::paintEvent(QPaintEvent *e)
{
#if 0
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
    int width = 0;
    QPainterPath paintPath, paintPathOut, paintPathIn;
    paintPathOut.addRoundedRect(rect, radius, radius);

    QRect rectIn = QRect(rect.x() + width, rect.y() + width, rect.width() - width * 2, rect.height() - width * 2);
    paintPathIn.addRoundedRect(rectIn, radius, radius);

    paintPath = paintPathOut.subtracted(paintPathIn);

    QBrush bgBrush(palette.color(cg, DPalette::TextWarning));
    painter.fillPath(paintPath, bgBrush);
    painter.restore();

#endif
    DWidget::paintEvent(e);
}

void ContentWidget::initWidget()
{
    QHBoxLayout *vLayout = new QHBoxLayout(this);
//    hLayout->setContentsMargins(5, 5, 5, 5);
    vLayout->addWidget(mp_Table);
    setLayout(vLayout);
}
