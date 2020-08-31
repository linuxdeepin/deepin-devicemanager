#include "PageTableHeader.h"


#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>
#include <QDebug>

#include <DFontSizeManager>
#include <DStandardItem>
#include <DApplication>
#include <DApplicationHelper>

#include "TableWidget.h"
#include "MacroDefinition.h"

PageTableHeader::PageTableHeader(QWidget *parent)
    : DWidget(parent)
    , mp_Table(new TableWidget(this))
{
    initWidgets();

    // 连接曹函数
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
    for (int i = 0; i < row - 1; i++) {
        bool enable = lst[i + 1][0].startsWith("(" + tr("Disable") + ")");
        qDebug() << enable << "**********" << lst[i + 1][0];
        int co = column;
        if (enable) {co = 1;}
        for (int j = 0; j < co; j++) {
            DStandardItem *item = new DStandardItem(lst[i + 1][j]);
            mp_Table->setItem(i, j, item);
        }
    }

    mp_Table->setColumnAverage();
}

void PageTableHeader::setColumnAverage()
{
    if (mp_Table)
        mp_Table->setColumnAverage();
}

void PageTableHeader::updateCurItemEnable(int row, int enable)
{
    mp_Table->updateCurItemEnable(row, enable);
}

void PageTableHeader::paintEvent(QPaintEvent *e)
{
    this->setFixedHeight(36 * 5 + 4 + 11);
    DWidget::paintEvent(e);
}

void PageTableHeader::slotItemClicked(int row)
{
    emit itemClicked(row);
}

void PageTableHeader::slotRefreshInfo()
{
    emit refreshInfo();
}
void PageTableHeader::slotExportInfo()
{
    emit exportInfo();
}

void PageTableHeader::slotEnableDevice(int row, bool enable)
{
    emit enableDevice(row, enable);
}


