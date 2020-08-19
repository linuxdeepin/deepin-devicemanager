#include "PageTableWidget.h"
#include "DetailTreeView.h"
#include "PageInfo.h"
#include "RichTextDelegate.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QDebug>

#include <DApplicationHelper>
#include <DPalette>
#include <DApplication>

PageTableWidget::PageTableWidget(DWidget *parent)
    : DWidget(parent)
    , mp_Table(new DetailTreeView(parent))
{
    initUI();

    connect(this, &PageTableWidget::enableDeviceSignal, mp_Table, &DetailTreeView::enableDevice);
    connect(mp_Table, &DetailTreeView::heightChange, this, &PageTableWidget::changeSize);
}

void PageTableWidget::setLimitRow(int row)
{
    mp_Table->setLimitRow(row);
}

void PageTableWidget::setColumnAndRow(int row, int column)
{
    mp_Table->setColumnAndRow(row, column);
}

void PageTableWidget::setItem(int row, int column, QTableWidgetItem *item)
{
    mp_Table->setItem(row, column, item);
}

QString PageTableWidget::toString()
{
    return mp_Table->toString();
}

bool PageTableWidget::isOverview()
{
    PageInfo *par = dynamic_cast<PageInfo *>(this->parent());
    return par->isOverview();
}

bool PageTableWidget::isBaseBoard()
{
    PageInfo *par = dynamic_cast<PageInfo *>(this->parent());
    return par->isBaseBoard();
}

void PageTableWidget::clear()
{
    mp_Table->clear();
}

void PageTableWidget::setRowHeight(int row, int height)
{
    mp_Table->setRowHeight(row, height);
}

void PageTableWidget::setItemDelegateForRow(int row, RichTextDelegate *itemDelegate)
{
    mp_Table->setItemDelegateForRow(row, itemDelegate);
}

bool PageTableWidget::isCurDeviceEnable()
{
    return mp_Table->isCurDeviceEnable();
}

void PageTableWidget::setCurDeviceState(bool state)
{
    mp_Table->setCurDeviceState(state);
}

void PageTableWidget::enableDevice()
{
    emit enableDeviceSignal();
}

void PageTableWidget::changeSize()
{
    setFixedHeight(mp_Table->height() + 4);
}

void PageTableWidget::paintEvent(QPaintEvent *event)
{
//    DWidget::paintEvent(event);


    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    int radius = 8;

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    this->setFixedHeight(mp_Table->height() + 4);
    QRect rect  = this->rect();
    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    int width = 1;
    QPainterPath paintPath, paintPathOut, paintPathIn;
    paintPathOut.addRoundedRect(rect, radius, radius);

    QRect rectIn = QRect(rect.x() + width, rect.y() + width, rect.width() - width * 2, rect.height() - width * 2);
    paintPathIn.addRoundedRect(rectIn, radius, radius);

    paintPath = paintPathOut.subtracted(paintPathIn);

    QBrush bgBrush(palette.color(cg, DPalette::FrameShadowBorder));
    painter.fillPath(paintPath, bgBrush);

    painter.restore();
    DWidget::paintEvent(event);
}

void PageTableWidget::initUI()
{
    QVBoxLayout *whLayout = new QVBoxLayout();
    whLayout->setContentsMargins(2, 2, 2, 2);
    whLayout->addWidget(mp_Table);
    this->setLayout(whLayout);
}
