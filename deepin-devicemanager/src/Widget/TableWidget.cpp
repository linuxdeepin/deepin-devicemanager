// 项目自身文件
#include "TableWidget.h"

// Dtk头文件
#include <DFontSizeManager>
#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>
#include <DHeaderView>
#include <DMenu>

// Qt库文件
#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>
#include <QPainterPath>

// 其它头文件
#include "MacroDefinition.h"
#include "logviewitemdelegate.h"
#include "logtreeview.h"



TableWidget::TableWidget(QWidget *parent)
    : DWidget(parent)
    , mp_Table(new LogTreeView(this))
    , mp_Enable(new QAction(tr("Disable"), this))
    , mp_Refresh(new QAction(/*QIcon::fromTheme("view-refresh"), */tr("Refresh"), this))
    , mp_Export(new QAction(/*QIcon::fromTheme("document-new"), */tr("Export"), this))
    , mp_Menu(new DMenu(this))
    , m_Enable(false)

{
    initWidget();

    // 连接信号和曹函数
    connect(mp_Table, &LogTreeView::clicked, this, &TableWidget::slotItemClicked);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &TableWidget::slotActionRefresh);
    connect(mp_Export, &QAction::triggered, this, &TableWidget::slotActionExport);
    connect(mp_Enable, &QAction::triggered, this, &TableWidget::slotActionEnable);
}

void TableWidget::setHeaderLabels(const QStringList &lst)
{
    QStringList headers;
    for (int i = 0; i < lst.size(); i++) {
        if (i < lst.size() - 1) {
            headers.append(lst[i]);
        } else {
            m_Enable = lst[i] == "yes" ? true : false;
        }
    }
    if (mp_Table) {
        mp_Table->setHeaderLabels(headers);
    }
}

void TableWidget::setItem(int row, int column, DStandardItem *item)
{
    if (mp_Table) {
        mp_Table->setItem(row, column, item);
    }
}

void TableWidget::setColumnAverage()
{
    if (mp_Table) {
        mp_Table->setColumnAverage();
    }
}

void TableWidget::updateCurItemEnable(int row, bool enable)
{
    if (mp_Table)
        mp_Table->updateCurItemEnable(row, enable);
}

void TableWidget::clear()
{
    if (mp_Table) {
        mp_Table->clear();
    }
}

void TableWidget::setRowNum(int row)
{
    // 设置表格行数
    mp_Table->setRowNum(row);
}

void TableWidget::paintEvent(QPaintEvent *e)
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
    this->setFixedHeight(TREE_ROW_HEIGHT * (mp_Table->RowNum() + 1) + HORSCROLL_WIDTH + WIDGET_MARGIN * 2);
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

}

void TableWidget::slotShowMenu(const QPoint &)
{
    mp_Menu->clear();
    QModelIndex index = mp_Table->currentIndex();
    if (m_Enable && index.row() >= 0) {
        if (mp_Table->currentRowEnable()) {
            mp_Enable->setText(tr("Disable"));
        } else {
            mp_Enable->setText(tr("Enable"));
        }
        mp_Menu->addAction(mp_Enable);
    }
    mp_Menu->addAction(mp_Refresh);
    mp_Menu->addAction(mp_Export);
    mp_Menu->exec(QCursor::pos());
}

void TableWidget::slotActionRefresh()
{
    emit refreshInfo();
}

void TableWidget::slotActionExport()
{
    emit exportInfo();
}

void TableWidget::slotActionEnable()
{
    if (!mp_Table) {
        return;
    }
    if (mp_Enable->text() == tr("Enable")) {
        emit enableDevice(mp_Table->currentRow(), true);
    } else {
        emit enableDevice(mp_Table->currentRow(), false);
    }
}

void TableWidget::slotItemClicked(const QModelIndex &index)
{
    int row = index.row();
    if (row >= 0) {
        emit itemClicked(row);
    }
}

void TableWidget::initWidget()
{
    QHBoxLayout *hLayout = new QHBoxLayout();
    int margin = 2;
    hLayout->setContentsMargins(margin, margin, margin, margin);
    hLayout->addWidget(mp_Table);
    setLayout(hLayout);
}
