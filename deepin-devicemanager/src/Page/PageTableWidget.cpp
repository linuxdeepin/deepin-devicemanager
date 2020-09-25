// 项目自身文件
#include "PageTableWidget.h"

// Qt库文件
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QDebug>
#include <QHeaderView>

// Dtk头文件
#include <DApplicationHelper>
#include <DPalette>
#include <DApplication>

// 其它头文件
#include "DetailTreeView.h"
#include "PageInfo.h"
#include "RichTextDelegate.h"

PageTableWidget::PageTableWidget(DWidget *parent)
    : DWidget(parent)
    , mp_Table(new DetailTreeView(parent))
{
    // 初始化UI
    initUI();

    // 连接槽函数
    connect(mp_Table, &DetailTreeView::heightChange, this, &PageTableWidget::changeSize);
}

void PageTableWidget::setLimitRow(int row)
{
    // 设置限制行数
    mp_Table->setLimitRow(row);
}

void PageTableWidget::setColumnAndRow(int row, int column)
{
    // 设置行列数
    mp_Table->setColumnAndRow(row, column);
}

void PageTableWidget::setItem(int row, int column, QTableWidgetItem *item)
{
    // 设置Item
    mp_Table->setItem(row, column, item);
}

QString PageTableWidget::toString()
{
    // table 内容转为QString
    return mp_Table->toString();
}

bool PageTableWidget::isOverview()
{
    // 是否是概况界面
    PageInfo *par = dynamic_cast<PageInfo *>(this->parent());
    return par->isOverview();
}

bool PageTableWidget::isBaseBoard()
{
    // 是否是主板界面
    PageInfo *par = dynamic_cast<PageInfo *>(this->parent());
    return par->isBaseBoard();
}

void PageTableWidget::clear()
{
    // 清空Table内容
    mp_Table->clear();
}

void PageTableWidget::setRowHeight(int row, int height)
{
    // 设置行高
    mp_Table->setRowHeight(row, height);
}

void PageTableWidget::resizeRowToContent(int row)
{
    // 设置表格宽度随内容变化
    mp_Table->resizeRowToContents(row);
}

void PageTableWidget::setItemDelegateForRow(int row, RichTextDelegate *itemDelegate)
{
    // 设置单元格代理
    mp_Table->setItemDelegateForRow(row, itemDelegate);
}

bool PageTableWidget::isCurDeviceEnable()
{
    // 当前设备是否可用
    return mp_Table->isCurDeviceEnable();
}

void PageTableWidget::setCurDeviceState(bool state)
{
    // 设置当前设备状态
    mp_Table->setCurDeviceState(state);
}

void PageTableWidget::expandTable()
{
    // 表格展开
    if (mp_Table) {
        mp_Table->expandCommandLinkClicked();
    }
}

void PageTableWidget::setDeviceEnable(bool e)
{
    // 设置设备状态
    mp_Table->setCurDeviceState(e);
}

void PageTableWidget::changeSize()
{
    // 设置表格固定高度
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
//    painter.fillPath(paintPath, bgBrush);

    painter.restore();
    DWidget::paintEvent(event);
}

void PageTableWidget::initUI()
{
    // 初始化页面布局
    QVBoxLayout *whLayout = new QVBoxLayout();
    whLayout->setContentsMargins(2, 2, 2, 2);
    whLayout->addWidget(mp_Table);
    this->setLayout(whLayout);
}
