#include "PageDriverTableView.h"
#include "drivertableview.h"
#include "MacroDefinition.h"

#include <DApplicationHelper>
#include <DApplication>

#include <QPainter>
#include <QHBoxLayout>

PageDriverTableView::PageDriverTableView(DWidget *parent)
    : DWidget(parent)
    , mp_View(new DriverTableView(this))
{
    initWidgets();
    connect(mp_View, &DriverTableView::operatorClicked, this, &PageDriverTableView::operatorClicked);
    connect(mp_View, &DriverTableView::itemChecked, this, &PageDriverTableView::itemChecked);
}

void PageDriverTableView::setColumnWidth(int row, int column)
{
    mp_View->setColumnWidth(row, column);
}

void PageDriverTableView::appendRowItems(int column)
{
    mp_View->appendRowItems(column);
    this->setFixedHeight(height() + DRIVER_TABLE_ROW_HEIGHT);
}

void PageDriverTableView::setWidget(int row, int column, DWidget *widget)
{
    mp_View->setWidget(row, column, widget);
    mp_View->resizeColumn(column);
}

QAbstractItemModel *PageDriverTableView::model() const
{
    return mp_View->model();
}

void PageDriverTableView::initHeaderView(const QStringList &headerList, bool check)
{
    mp_View->initHeaderView(headerList, check);
}

void PageDriverTableView::setHeaderCbStatus(bool checked)
{
    mp_View->setHeaderCbStatus(checked);
}

void PageDriverTableView::setCheckedCBDisnable()
{
    mp_View->setCheckedCBDisable();
}

void PageDriverTableView::getCheckedDriverIndex(QList<int> &lstIndex)
{
    mp_View->getCheckedDriverIndex(lstIndex);
}

void PageDriverTableView::setItemStatus(int index, Status s)
{
    mp_View->setItemStatus(index, s);
}

void PageDriverTableView::setErrorMsg(int index, const QString &msg)
{
    mp_View->setErrorMsg(index, msg);
}

bool PageDriverTableView::hasItemDisabled()
{
    return mp_View->hasItemDisabled();
}

void PageDriverTableView::clear()
{
    this->setMaximumHeight(DRIVER_TABLE_HEADER_HEIGHT + 4);
    mp_View->clear();
}

void PageDriverTableView::paintEvent(QPaintEvent *e)
{
    DWidget::paintEvent(e);
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    int radius = 8;

//     获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 设置Widget固定高度,(+1)表示包含表头高度,(*2)表示上下边距，为保证treewidget横向滚动条与item不重叠，添加滚动条高度
    QRect rect  = this->rect();

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    int width = 1;
    QPainterPath paintPath, paintPathOut, paintPathIn;
    paintPathOut.addRoundedRect(rect, radius, radius);

    QRect rectIn = QRect(rect.x() + width, rect.y() + width, rect.width() - width * 2, rect.height() - width * 2);
    paintPathIn.addRoundedRect(rectIn, radius, radius);

    paintPath = paintPathOut.subtracted(paintPathIn);

    QBrush bgBrush(palette.color(cg, DPalette::FrameBorder));
    painter.fillPath(paintPath, bgBrush);

    painter.restore();
}

void PageDriverTableView::initWidgets()
{
    this->setFixedHeight(DRIVER_TABLE_HEADER_HEIGHT + 5);
    setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 2, 6, 0);
    mainLayout->addWidget(mp_View);
    mainLayout->addStretch();
    this->setLayout(mainLayout);
}
