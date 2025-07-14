// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PageTableWidget.h"
#include "DetailTreeView.h"
#include "PageInfo.h"
#include "RichTextDelegate.h"
#include "DDLog.h"

// Dtk头文件
#include <DGuiApplicationHelper>
#include <DPalette>
#include <DApplication>

// Qt库文件
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QLoggingCategory>
#include <QHeaderView>

using namespace DDLog;

PageTableWidget::PageTableWidget(DWidget *parent)
    : DWidget(parent)
    , mp_Table(new DetailTreeView(parent))
{
    qCDebug(appLog) << "PageTableWidget constructor start";
    // 初始化UI
    initUI();
}

void PageTableWidget::setLimitRow(int row)
{
    qCDebug(appLog) << "Setting table row limit to:" << row;
    // 设置限制行数
    mp_Table->setLimitRow(row);
}

void PageTableWidget::setColumnAndRow(int row, int column)
{
    qCDebug(appLog) << "Setting table dimensions - rows:" << row << "columns:" << column;
    // 设置行列数
    mp_Table->setColumnAndRow(row, column);
}

void PageTableWidget::setItem(int row, int column, QTableWidgetItem *item)
{
    // qCDebug(appLog) << "Setting item at row:" << row << "column:" << column;
    // 设置Item
    mp_Table->setItem(row, column, item);
}

QString PageTableWidget::toString()
{
    // qCDebug(appLog) << "Converting table content to QString";
    // table 内容转为QString
    return mp_Table->toString();
}

bool PageTableWidget::isOverview()
{
    // qCDebug(appLog) << "Checking if it is an overview page";
    // 是否是概况界面
    PageInfo *par = dynamic_cast<PageInfo *>(this->parent());
    return par->isOverview();
}

bool PageTableWidget::isBaseBoard()
{
    // qCDebug(appLog) << "Checking if it is a baseboard page";
    // 是否是主板界面
    PageInfo *par = dynamic_cast<PageInfo *>(this->parent());
    return par->isBaseBoard();
}

void PageTableWidget::clear()
{
    // qCDebug(appLog) << "Clearing table content";
    // 清空Table内容
    mp_Table->clear();
}

void PageTableWidget::setRowHeight(int row, int height)
{
    // qCDebug(appLog) << "Setting row height:" << height;
    // 设置行高
    mp_Table->setRowHeight(row, height);
}

void PageTableWidget::setItemDelegateForRow(int row, RichTextDelegate *itemDelegate)
{
    qCDebug(appLog) << "Setting item delegate for row:" << row;
    // 设置单元格代理
    if (itemDelegate == nullptr) {
        qCDebug(appLog) << "Using default item delegate.";
        mp_Table->setItemDelegateForRow(row, mp_Table->itemDelegate());
    } else {
        qCDebug(appLog) << "Using rich text item delegate.";
        mp_Table->setItemDelegateForRow(row, itemDelegate);
    }
}

bool PageTableWidget::isCurDeviceEnable()
{
    qCDebug(appLog) << "Checking if the current device is enabled";
    // 当前设备是否可用
    return mp_Table->isCurDeviceEnable();
}

void PageTableWidget::setCurDeviceState(bool enable, bool available)
{
    qCDebug(appLog) << "Setting current device state - enabled:" << enable << "available:" << available;
    // 设置当前设备状态
    mp_Table->setCurDeviceState(enable, available);
}

void PageTableWidget::expandTable()
{
    qCDebug(appLog) << "Expanding table";
    // 表格展开
    if (mp_Table) {
        qCDebug(appLog) << "Expanding table";
        mp_Table->expandCommandLinkClicked();
    }
}

bool PageTableWidget::isExpanded()
{
    // qCDebug(appLog) << "Checking if the table is expanded";
    if (mp_Table) {
        // qCDebug(appLog) << "Table is expanded";
        return mp_Table->isExpanded();
    }
    // qCDebug(appLog) << "Table is not expanded";
    return false;
}

void PageTableWidget::setDeviceEnable(bool enable, bool available)
{
    qCInfo(appLog) << "Setting device state - enabled:" << enable << "available:" << available;
    // 设置设备状态
    mp_Table->setCurDeviceState(enable, available);
}

void PageTableWidget::paintEvent(QPaintEvent *event)
{
    // qCDebug(appLog) << "Painting table";
    DWidget::paintEvent(event);
}

void PageTableWidget::initUI()
{
    qCDebug(appLog) << "Initializing UI";
    // 初始化页面布局
    QVBoxLayout *whLayout = new QVBoxLayout();

    // set Contents margin 0,bug66137
    whLayout->setContentsMargins(0, 0, 0, 0);
    whLayout->addWidget(mp_Table);
    whLayout->addStretch();
    this->setLayout(whLayout);
}
