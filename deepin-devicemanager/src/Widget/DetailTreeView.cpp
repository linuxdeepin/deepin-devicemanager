#include "DetailTreeView.h"
#include "PageDetail.h"
#include "DetailViewDelegate.h"
#include "MacroDefinition.h"

#include <QHeaderView>
#include <QPainter>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QDebug>

#include <DApplication>
#include <DApplicationHelper>
#include <DStyle>
#include "PageInfo.h"

DetailTreeView::DetailTreeView(DWidget *parent)
    : DTableWidget(parent)
    , mp_ItemDelegate(nullptr)
    , mp_CommandBtn(nullptr)
    , m_LimitRow(13)
    , m_IsExpand(false)
{
    // 初始化界面
    initUI();
}

void DetailTreeView::setColumnAndRow(int row, int column)
{
    // 设置表格行数列数
    setRowCount(row);
    setColumnCount(column);

    // 表格行数大于限制行数时，添加展开button
    if (row > m_LimitRow + 1) {
        setCommanLinkButton(row);
    }
    // 如果行数少于限制行数，则影藏最后一行
    if (row <= m_LimitRow + 1) {
        hideRow(row - 1);
    }
}

void DetailTreeView::setItem(int row, int column, QTableWidgetItem *item)
{
    // 设置表格高度
    setFixedHeight(ROW_HEIGHT * std::min((row + 1), m_LimitRow + 1));

    // 添加表格内容
    DTableWidget::setItem(row, column, item);

    // 行数大于限制行数隐藏信息，展示展开button
    if (row >= m_LimitRow) {
        hideRow(row);
        showRow(this->rowCount() - 1);
    }
}

void DetailTreeView::clear()
{
    // 清空表格内容
    DTableWidget::clear();

    // 删除表格行列
    setRowCount(0);
    setColumnCount(0);
}

void DetailTreeView::setCommanLinkButton(int row)
{
    // 设置mp_CommandBtn属性
    mp_CommandBtn = new DCommandLinkButton(tr("More"), this);
    mp_CommandBtn->setMinimumSize(150, 100);

    //  合并最后一行
    this->setSpan(row - 1, 0, 1, 2);

    // 将mp_CommandBtn放到布局中，居中
    QHBoxLayout *pHBoxLayout = new QHBoxLayout();
    pHBoxLayout->addStretch();
    pHBoxLayout->addWidget(mp_CommandBtn);
    pHBoxLayout->addStretch();

    QVBoxLayout *pVBoxLayout = new QVBoxLayout(this);
    pVBoxLayout->addLayout(pHBoxLayout);

    DWidget *btnwidget = new DWidget();
    btnwidget->setLayout(pVBoxLayout);

    // 将btnwidget填充到表格中，并隐藏
    setCellWidget(row - 1, 0, btnwidget);
//    hideRow(row - 1);

    connect(mp_CommandBtn, &DCommandLinkButton::clicked, this, &DetailTreeView::expandCommandLinkClicked);
}

int DetailTreeView::setTableHeight(int paintHeight)
{
    // 父窗口
    PageInfo *p = dynamic_cast<PageInfo *>(this->parent());

    // 父窗口可显示的最大表格行数
    int maxRow = 0;
    if (p->isOverview()) {
        maxRow = p->height() / ROW_HEIGHT - 4;
    } else {
        maxRow = p->height() / ROW_HEIGHT - 3;
    }

    // 主板界面的表格高度
    if (p->isBaseBoard()) {
        // 表格未展开
        if (m_IsExpand == false) {
            this->setFixedHeight(ROW_HEIGHT * (m_LimitRow + 1));
            return this->height();
        } else {
            // 表格展开,父窗口最大容纳高度
            this->setFixedHeight(ROW_HEIGHT * maxRow);
            return this->height();
        }

    }


    // 信息行 <= m_LimitRow + 1 不影响表格大小
    if (rowCount() <= m_LimitRow + 1) {
        return paintHeight;
    } else {
        // 未展开,窗口高度始终等于ROW_HEIGHT * (m_LimitRow+1)
        if (m_IsExpand == false) {
            this->setFixedHeight(ROW_HEIGHT * (m_LimitRow + 1));
            return this->height();

        } else {
            // 展开，表格高度<父窗口高度
            this->setFixedHeight(ROW_HEIGHT * std::min(rowCount(), maxRow));
            return this->height();
        }
    }
}

bool DetailTreeView::hasExpendInfo()
{
    // 指针不为空，设备有其他信息
    if (mp_CommandBtn != nullptr) {
        return true;
    } else {
        // 指针为空，设备没有更多信息
        return false;
    }
}

void DetailTreeView::setLimitRow(int row)
{
    m_LimitRow = row;
}

void DetailTreeView::expandCommandLinkClicked()
{
    // 当前已展开详细信息
    if (m_IsExpand) {
        mp_CommandBtn->setText("More");
        m_IsExpand = false;
        for (int i = m_LimitRow; i < rowCount() - 1; ++i) {
            hideRow(i);
        }
    } else { // 当前未展开详细信息
        mp_CommandBtn->setText("Collapse");
        m_IsExpand = true;
        for (int i = m_LimitRow; i < rowCount() - 1; ++i) {
            showRow(i);
        }
    }
}

void DetailTreeView::initUI()
{
    // 设置Item自定义代理
    mp_ItemDelegate = new DetailViewDelegate(this);
    setItemDelegate(mp_ItemDelegate);

    // 设置不可编辑模式
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置表格一次滚动一个Item，dtk默认一次滚动一个像素
    this->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);

    // 设置不可选择
    setSelectionMode(QAbstractItemView::NoSelection);

    // 设置无边框
    this->setFrameStyle(QFrame::NoFrame);

    // 隐藏网格线
    this->setShowGrid(false);
//    this->viewport()->setAutoFillBackground(false);

    // 设置各行变色
    setAlternatingRowColors(true);

    // 隐藏表头
    this->verticalHeader()->setVisible(false);
    this->horizontalHeader()->setVisible(false);

    // 设置不可排序
    this->setSortingEnabled(false);

    // 设置最后一个section拉伸填充
    this->horizontalHeader()->setStretchLastSection(true);

    // 设置行高
    this->verticalHeader()->setDefaultSectionSize(ROW_HEIGHT);

    // 设置section行宽
    this->horizontalHeader()->setDefaultSectionSize(180);
}

void DetailTreeView::paintEvent(QPaintEvent *event)
{
    DTableView::paintEvent(event);

    QPainter painter(viewport());
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setOpacity(1);
    painter.setClipping(true);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }

    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();

    QStyleOptionFrame option;
    initStyleOption(&option);

    QRect rect = viewport()->rect();
    int pHeight = setTableHeight(rect.height());

    // 窗口大小发生变化时，需重新设置表格大小
    rect.setHeight(pHeight);

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);

    // 圆角矩形路径
    QPainterPath clipPath;
    clipPath.addRoundedRect(rect, radius, radius);

    // 绘制圆角矩形
    QPen pen(palette.color(cg, DPalette::FrameShadowBorder));
    pen.setWidth(LINE_WIDTH);
    painter.setPen(pen);
    painter.drawPath(clipPath);
    painter.restore();

}

//void DetailTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
//{
//    painter->save();
//    painter->setRenderHint(QPainter::Antialiasing);

//#ifdef ENABLE_INACTIVE_DISPLAY
//    QWidget *wnd = DApplication::activeWindow();
//#endif
//    DPalette::ColorGroup cg;
//    if (!(options.state & DStyle::State_Enabled)) {
//        cg = DPalette::Disabled;
//    } else {
//#ifdef ENABLE_INACTIVE_DISPLAY
//        if (!wnd) {
//            cg = DPalette::Inactive;
//        } else {
//            if (wnd->isModal()) {
//                cg = DPalette::Inactive;
//            } else {
//                cg = DPalette::Active;
//            }
//        }
//#else
//        cg = DPalette::Active;
//#endif
//    }

//    auto *style = dynamic_cast<DStyle *>(DApplication::style());

//    auto radius = style->pixelMetric(DStyle::PM_FrameRadius, &options);
//    auto margin = style->pixelMetric(DStyle::PM_ContentsMargins, &options);

//    auto palette = options.palette;
//    QBrush background;

//    if (index.row() == m_LimitRow - 1) {

//    }

//    painter->restore();
//}

void DetailTreeView::keyPressEvent(QKeyEvent *event)
{
    DTableView::keyPressEvent(event);
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
        emit clicked(this->currentIndex());
    }
}
