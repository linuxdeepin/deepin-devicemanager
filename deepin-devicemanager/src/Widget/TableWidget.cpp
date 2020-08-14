#include "TableWidget.h"

#include <DFontSizeManager>
#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>
#include <DHeaderView>
#include <DMenu>

#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>

#include "MacroDefinition.h"
#include "logviewitemdelegate.h"
#include "logtreeview.h"



TableWidget::TableWidget(QWidget *parent)
    : DWidget(parent)
    , mp_Table(new LogTreeView(this))
    , mp_Enable(new QAction(tr("Disable"), this))
    , mp_Refresh(new QAction(/*QIcon::fromTheme("view-refresh"), */tr("Refresh (F5)"), this))
    , mp_Export(new QAction(/*QIcon::fromTheme("document-new"), */tr("Export (E)"), this))
    , mp_Menu(new DMenu(this))

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
    if (mp_Table) {
        mp_Table->setHeaderLabels(lst);
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
        qDebug() << "*************PageTableHeader::updateTable**************" << mp_Table->height();
        mp_Table->setColumnAverage();
    }
}

void TableWidget::clear()
{
    if (mp_Table) {
        mp_Table->clear();
    }
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

//     获取系统默认的圆角半径
//    QStyleOptionFrame option;
//    initStyleOption(&option);
//    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
//    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);
    int radius = 8;

//     获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

//    this->setFixedHeight(mp_Table->height() + 10);
    this->setFixedHeight(36 * 5 + 4);
    QRect rect  = this->rect();

//    qDebug() << this->rect();
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

//    QPen pen = painter.pen();
//    pen.setColor(palette.color(cg, DPalette::TextWarning));
//    pen.setWidth(1);

//    painter.setPen(pen);

//    QRect rect = mp_Table->geometry();
//    this->setFixedHeight(rect.height() + 2);

//    painter.drawRect(this->rect());
//    qDebug() << rect << this->rect();
    painter.restore();
//    DWidget::paintEvent(e);
}

void TableWidget::slotShowMenu(const QPoint &)
{
    mp_Menu->clear();
//    mp_Menu->addSeparator();
    if (mp_Table->currentRowEnable()) {
        mp_Enable->setText(tr("Disable"));
    } else {
        mp_Enable->setText(tr("Enable"));
    }
    mp_Menu->addAction(mp_Enable);
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
