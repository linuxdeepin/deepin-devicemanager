// 项目自身文件
#include "PageTableHeader.h"

// Qt库文件
#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>
#include <QDebug>

// Dtk头文件
#include <DFontSizeManager>
#include <DStandardItem>
#include <DApplication>
#include <DApplicationHelper>

// 其它头文件
#include "TableWidget.h"
#include "MacroDefinition.h"

PageTableHeader::PageTableHeader(QWidget *parent)
    : DWidget(parent)
    , mp_Table(new TableWidget(this))
{
    // 初始化页面布局
    initWidgets();

    // 连接槽函数函数
    connect(mp_Table, &TableWidget::itemClicked, this, &PageTableHeader::slotItemClicked);
    connect(mp_Table, &TableWidget::refreshInfo, this, &PageTableHeader::slotRefreshInfo);
    connect(mp_Table, &TableWidget::exportInfo, this, &PageTableHeader::slotExportInfo);
    connect(mp_Table, &TableWidget::enableDevice, this, &PageTableHeader::slotEnableDevice);
}

PageTableHeader::~PageTableHeader()
{

}

void PageTableHeader::initWidgets()
{
    // 布局
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 10);
    hLayout->addWidget(mp_Table);
    setLayout(hLayout);
}

void PageTableHeader::updateTable(const QList<QStringList> &lst)
{
    // 提前清楚内容
    mp_Table->clear();

    // 如果lst.size() == 1 则说明改设备只有一个
    if (lst.size() == 1) {
        return;
    }

    // 设置表头
    mp_Table->setHeaderLabels(lst[0]);

    // 设置表格的内容
    int row = lst.size();
    int column = lst[0].size() - 1;

    // 设置表格行数以及背景Widget高度
    //(+1)表示包含表头高度,(*2)表示上下边距,
    if (row - 1 < ROW_NUM) {
        // 表格内容行数小于4,表格高度与行数一致，为保证treewidget横向滚动条与item不重叠，添加滚动条高度
        mp_Table->setRowNum(row - 1);
        this->setFixedHeight(TREE_ROW_HEIGHT * row + HORSCROLL_WIDTH + WIDGET_MARGIN * 2 + BOTTOM_MARGIN);
    } else {
        // 表格内容行数大于等于4,表格行数固定为4，为保证treewidget横向滚动条与item不重叠，添加滚动条高度
        mp_Table->setRowNum(4);
        this->setFixedHeight(TREE_ROW_HEIGHT * (ROW_NUM + 1) + HORSCROLL_WIDTH + WIDGET_MARGIN * 2  + BOTTOM_MARGIN);
    }

    for (int i = 0; i < row - 1; i++) {
        bool enable = lst[i + 1][0].startsWith("(" + tr("Disable") + ")");
        int co = column;

        if (enable) {co = 1;}
        for (int j = 0; j < co; j++) {
            DStandardItem *item = new DStandardItem(lst[i + 1][j]);
            mp_Table->setItem(i, j, item);
        }
    }

    // 列宽平均分配
    mp_Table->setColumnAverage();
}

void PageTableHeader::setColumnAverage()
{
    // 列宽平均分配
    if (mp_Table)
        mp_Table->setColumnAverage();
}

void PageTableHeader::updateCurItemEnable(int row, int enable)
{
    // 禁用/启用当前设备
    mp_Table->updateCurItemEnable(row, enable);
}

void PageTableHeader::paintEvent(QPaintEvent *e)
{
    DWidget::paintEvent(e);
}

void PageTableHeader::slotItemClicked(int row)
{
    // 点击行
    emit itemClicked(row);
}

void PageTableHeader::slotRefreshInfo()
{
    // 刷新
    emit refreshInfo();
}
void PageTableHeader::slotExportInfo()
{
    // 导出
    emit exportInfo();
}

void PageTableHeader::slotEnableDevice(int row, bool enable)
{
    // 启用/禁用设备
    emit enableDevice(row, enable);
}


