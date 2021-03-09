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

void PageTableWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);
}

void PageTableWidget::initUI()
{
    // 初始化页面布局
    QVBoxLayout *whLayout = new QVBoxLayout();
    whLayout->addWidget(mp_Table);
    this->setLayout(whLayout);
}
