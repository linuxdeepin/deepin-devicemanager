// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PageTableHeader.h"
#include "TableWidget.h"
#include "MacroDefinition.h"
#include "DDLog.h"

// Dtk头文件
#include <DFontSizeManager>
#include <DStandardItem>
#include <DApplication>
#include <DGuiApplicationHelper>

// Qt库文件
#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>
#include <QLoggingCategory>

using namespace DDLog;

PageTableHeader::PageTableHeader(QWidget *parent)
    : DWidget(parent)
    , mp_Table(new TableWidget(this))
{
    qCDebug(appLog) << "PageTableHeader constructor start";
    // 初始化页面布局
    initWidgets();

    // 连接槽函数函数
    connect(mp_Table, &TableWidget::itemClicked, this, &PageTableHeader::itemClicked);
    connect(mp_Table, &TableWidget::refreshInfo, this, &PageTableHeader::refreshInfo);
    connect(mp_Table, &TableWidget::exportInfo, this, &PageTableHeader::exportInfo);
    connect(mp_Table, &TableWidget::enableDevice, this, &PageTableHeader::enableDevice);
    connect(mp_Table, &TableWidget::installDriver, this, &PageTableHeader::installDriver);
    connect(mp_Table, &TableWidget::uninstallDriver, this, &PageTableHeader::uninstallDriver);
    connect(mp_Table, &TableWidget::wakeupMachine, this, &PageTableHeader::wakeupMachine);
    connect(mp_Table, &TableWidget::signalCheckPrinterStatus, this, &PageTableHeader::signalCheckPrinterStatus);
}

PageTableHeader::~PageTableHeader()
{
    qCDebug(appLog) << "PageTableHeader destructor start";
    if (mp_Table) {
        qCDebug(appLog) << "Deleting table widget";
        delete mp_Table;
        mp_Table = nullptr;
    }
}

void PageTableHeader::initWidgets()
{
    qCDebug(appLog) << "Initializing widgets";
    // 布局
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 10);
    hLayout->addWidget(mp_Table);
    setLayout(hLayout);
}

void PageTableHeader::updateTable(const QList<QStringList> &lst, const QList<QStringList>& lstMenuControl, bool resizeTable, int step)
{
    qCInfo(appLog) << "Updating table with" << lst.size() << "rows, resize:" << resizeTable << "step:" << step;
    int configRowNum = ROW_NUM;
    if(resizeTable) {
        qCDebug(appLog) << "Resizing table, step:" << step;
        configRowNum = ROW_NUM - step;
    }

    // 如果lst.size() == 1 则说明改设备只有一个
    if (lst.size() <= 1) {
        qCDebug(appLog) << "List size <= 1, no data to display.";
        return;
    }

    // 提前清楚内容
    mp_Table->clear();

    // 设置表头
    mp_Table->setHeaderLabels(lst[0]);

    // 设置表格的内容
    int row = lst.size();
    int column = lst[0].size() - 1;

    // 设置表格行数以及背景Widget高度
    //(+1)表示包含表头高度,(*2)表示上下边距,
    if (row - 1 < configRowNum) {
        // 表格内容行数小于4,表格高度与行数一致，为保证treewidget横向滚动条与item不重叠，添加滚动条高度
        mp_Table->setRowNum(row - 1);
        this->setFixedHeight(TREE_ROW_HEIGHT * row + HORSCROLL_WIDTH + WIDGET_MARGIN * 2 + BOTTOM_MARGIN);
        qCDebug(appLog) << "Row count < configRowNum, setting row number to" << row - 1;
    } else {
        // 表格内容行数大于等于4,表格行数固定为4，为保证treewidget横向滚动条与item不重叠，添加滚动条高度
        mp_Table->setRowNum(configRowNum);
        this->setFixedHeight(TREE_ROW_HEIGHT * (configRowNum + 1) + HORSCROLL_WIDTH + WIDGET_MARGIN * 2  + BOTTOM_MARGIN);
        qCDebug(appLog) << "Row count >= configRowNum, setting row number to" << configRowNum;
    }

    for (int i = 0; i < row - 1; i++) {
        for (int j = 0; j < column; j++) {
            DStandardItem *item = new DStandardItem(lst[i + 1][j]);
            if(0 == j){
                for(int index = 0; index < lstMenuControl[i].size(); index++){
                    item->setData(lstMenuControl[i][index],Qt::UserRole+index);
                }
                qCDebug(appLog) << "Set data for header item at row:" << i << "column:" << j;
            }
            mp_Table->setItem(i, j, item);
        }
    }

    // 列宽平均分配
    mp_Table->setColumnAverage();
}

void PageTableHeader::setColumnAverage()
{
    qCDebug(appLog) << "Setting table columns to average width";
    // 列宽平均分配
    if (mp_Table)
        mp_Table->setColumnAverage();
}

void PageTableHeader::paintEvent(QPaintEvent *e)
{
    // qCDebug(appLog) << "Painting table header";
    DWidget::paintEvent(e);
}


