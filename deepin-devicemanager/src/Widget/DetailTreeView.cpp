// 项目自身文件
#include "DetailTreeView.h"

// Qt库文件
#include <QHeaderView>
#include <QPainter>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QDebug>
#include <QThread>
#include <QToolTip>
#include <QDateTime>
#include <QTimer>
#include <QDesktopWidget>

// Dtk头文件
#include <DApplication>
#include <DApplicationHelper>
#include <DStyle>
#include <DFontSizeManager>

// 其它头文件
#include "PageDetail.h"
#include "DetailViewDelegate.h"
#include "MacroDefinition.h"
#include "PageInfo.h"
#include "PageTableWidget.h"
#include "CmdButtonWidget.h"
#include "TipsWidget.h"




BtnWidget::BtnWidget()
{

}
void BtnWidget::enterEvent(QEvent *event)
{
    emit enter();
    return DWidget::enterEvent(event);
}

void BtnWidget::leaveEvent(QEvent *event)
{
    emit leave();
    return DWidget::leaveEvent(event);
}



DetailTreeView::DetailTreeView(DWidget *parent)
    : DTableWidget(parent)
    , mp_ItemDelegate(nullptr)
    , mp_CommandBtn(nullptr)
    , m_LimitRow(13)
    , m_IsExpand(false)
    , m_IsEnable(true)
    , mp_OldItem(nullptr)
    , mp_CurItem(nullptr)
    , m_TimeStep(0)
    , mp_Timer(new QTimer)
    , mp_ToolTips(nullptr)
{
    setMouseTracking(true);
    // 初始化界面
    initUI();
    connect(mp_Timer, &QTimer::timeout, this, &DetailTreeView::slotTimeOut);
    connect(this, &DetailTreeView::itemEntered, this, &DetailTreeView::slotItemEnterd);
    mp_Timer->start(100);
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

    if (mp_CommandBtn != nullptr) {
        delete mp_CommandBtn;
        mp_CommandBtn = nullptr;
    }
    m_IsExpand = false;
}

void DetailTreeView::setCommanLinkButton(int row)
{
    // 设置mp_CommandBtn属性
    mp_CommandBtn = new DCommandLinkButton(tr("More"), this);

    // 设置字号
    DFontSizeManager::instance()->bind(mp_CommandBtn, DFontSizeManager::T8);

    //mp_CommandBtn->setMinimumSize(150, 100);

    //  合并最后一行
    this->setSpan(row - 1, 0, 1, 2);

    // 将mp_CommandBtn放到布局中，居中
    QHBoxLayout *pHBoxLayout = new QHBoxLayout();
    pHBoxLayout->addStretch();
    pHBoxLayout->addWidget(mp_CommandBtn);
    pHBoxLayout->addStretch();

    QVBoxLayout *pVBoxLayout = new QVBoxLayout();
    pVBoxLayout->addLayout(pHBoxLayout);

    BtnWidget *btnwidget = new BtnWidget();
    connect(btnwidget, &BtnWidget::enter, this, &DetailTreeView::slotEnterBtnWidget);
    connect(btnwidget, &BtnWidget::leave, this, &DetailTreeView::slotLeaveBtnWidget);
    btnwidget->setLayout(pVBoxLayout);

    // 将btnwidget填充到表格中，并隐藏
    setCellWidget(row - 1, 1, btnwidget);

    connect(mp_CommandBtn, &DCommandLinkButton::clicked, this, &DetailTreeView::expandCommandLinkClicked);
}

int DetailTreeView::setTableHeight(int paintHeight)
{
    if (!m_IsEnable) {
        paintHeight = 40;
        this->setFixedHeight(paintHeight);
        return paintHeight;
    }

    // 父窗口
    PageTableWidget *p = dynamic_cast<PageTableWidget *>(this->parent());

    // 父窗口可显示的最大表格行数
    int maxRow = 0;
    if (p->isBaseBoard()) {
        PageInfo *par = dynamic_cast<PageInfo *>(p->parent());

        // 最多显示行数与父窗口高度相关,需减去Label以及Spacing占用空间
        maxRow = par->height() / ROW_HEIGHT - 2;
    } else {
        PageInfo *par = dynamic_cast<PageInfo *>(p->parent());
        maxRow = par->height() / ROW_HEIGHT - 2;

        // 当前页面为概况时，展开更多信息，页面显示的表格的最大行数需减一，避免表格边框显示不完整
        if (p->isOverview()) {
            // 有更多信息并且已展开
            if (hasExpendInfo() && m_IsExpand) {
                maxRow--;
            }
        }
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
        this->setFixedHeight((rowCount() - 1)*ROW_HEIGHT);
        return (rowCount() - 1) * ROW_HEIGHT;
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

int DetailTreeView::maxRowofWidget()
{
    return 0;
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

QString DetailTreeView::toString()
{
    QString str;
    int row = rowCount();
    int column = columnCount();
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < column; j++) {
            QTableWidgetItem *sItem = this->item(i, j);
            if (sItem) {
                QString se = (j == 0) ? " : " : "\n";
                str += sItem->text() + se;
            }
        }
//        str += "\n";
    }
    return str;
}

bool DetailTreeView::isCurDeviceEnable()
{
    return m_IsEnable;
}

void DetailTreeView::setCurDeviceState(bool state)
{
    m_IsEnable = state;
    if (!m_IsEnable) {

        for (int i = 1; i < this->rowCount(); ++i) {
            this->hideRow(i);
        }

        this->setTableHeight(40);
    } else {
        this->setTableHeight(40);

        if (m_IsExpand) {
            for (int i = 1; i < this->rowCount(); ++i) {
                this->showRow(i);
            }
        }

        if (!m_IsExpand && hasExpendInfo()) {
            for (int i = 1; i < this->m_LimitRow; ++i) {
                this->showRow(i);
            }

            this->showRow(this->rowCount() - 1);
        }

        if (!hasExpendInfo()) {
            for (int i = 1; i < this->rowCount() - 1; ++i) {
                this->showRow(i);

            }
        }
    }
}

void DetailTreeView::expandCommandLinkClicked()
{
    // 当前已展开详细信息
    if (m_IsExpand) {
        mp_CommandBtn->setText(tr("More"));
        m_IsExpand = false;
        for (int i = m_LimitRow; i < rowCount() - 1; ++i) {
            hideRow(i);
        }
    } else { // 当前未展开详细信息
        mp_CommandBtn->setText(tr("Collapse"));
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
    this->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);

    // 设置不可选择
//    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::NoSelection);

    // 设置无边框
    this->setFrameStyle(QFrame::NoFrame);

    // 隐藏网格线
    this->setShowGrid(false);
    this->viewport()->setAutoFillBackground(true);

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

    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
    this->setWindowFlags(Qt::FramelessWindowHint);   //设置无边框窗口
}

void DetailTreeView::paintEvent(QPaintEvent *event)
{
    DTableView::paintEvent(event);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }

    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();

    QRect rect = viewport()->rect();
    int pHeight = setTableHeight(rect.height());

    // 窗口大小发生变化时，需重新设置表格大小
    this->setFixedHeight(pHeight);
    rect = viewport()->rect();

    QPainter painter(this->viewport());
    painter.save();

    int width = 1;
    int radius = 8;
    QPainterPath paintPath, paintPathOut, paintPathIn;
    paintPathOut.addRoundedRect(rect, radius, radius);

    QRect rectIn = QRect(rect.x() + width, rect.y() + width, rect.width() - width * 2, rect.height() - width * 2);
    paintPathIn.addRoundedRect(rectIn, radius, radius);

    paintPath = paintPathOut.subtracted(paintPathIn);

    QBrush bgBrush(palette.color(cg, DPalette::FrameShadowBorder));
    painter.fillPath(paintPath, bgBrush);

    QPen pen = painter.pen();
    pen.setWidth(1);
    pen.setColor(palette.color(cg, DPalette::FrameShadowBorder));

    painter.setPen(pen);

    QLine line(rect.topLeft().x() + 179, rect.topLeft().y(), rect.bottomLeft().x() + 179, rect.bottomLeft().y());

    // 有展开信息，且未展开，button行不绘制竖线
    if (this->hasExpendInfo() && !m_IsExpand) {
        line.setP2(QPoint(rect.bottomLeft().x() + 179, rect.bottomLeft().y() - 40));

        // 绘制横线

        if (m_IsEnable) {
            QLine hline(rect.bottomLeft().x(), rect.bottomLeft().y() - 39, rect.bottomRight().x(), rect.bottomRight().y() - 39);
            painter.drawLine(hline);
        }

    } else if (hasExpendInfo() && m_IsExpand) {

        QTableWidgetItem *it = this->itemAt(QPoint(this->rect().bottomLeft().x(), this->rect().bottomLeft().y()));
        if (it == nullptr) {
            // 由于展开按钮行是DWidget无法获取item，所以，再在这种情况下，展开按钮行开始出现再可视区域
            for (int i = 1; i <= 40; ++i) {

                // 获取上一行item的下边距离表格边框的像素距离
                QTableWidgetItem *lastItem = this->itemAt(QPoint(this->rect().bottomLeft().x(), this->rect().bottomLeft().y() - i));
                if (lastItem != nullptr) {
                    // 竖线再 button行不显示
                    line.setP2(QPoint(rect.bottomLeft().x() + 179, rect.bottomLeft().y() - i));

                    // 绘制横线
                    if (m_IsEnable) {
                        QLine hline(rect.bottomLeft().x(), rect.bottomLeft().y() - i + 1, rect.bottomRight().x(), rect.bottomRight().y() - i + 1);
                        painter.drawLine(hline);
                    }

                    break;

                }
            }
        }
    }
    painter.drawLine(line);
    painter.restore();
}

void DetailTreeView::resizeEvent(QResizeEvent *event)
{
    DTableWidget::resizeEvent(event);
    emit heightChange();
    // 解决　调整窗口大小时tooltip未及时刷新
    QPoint pt = this->mapFromGlobal(QCursor::pos());
    mp_CurItem = itemAt(pt);
}

void DetailTreeView::mouseMoveEvent(QMouseEvent *event)
{
    mp_Point = event->pos();
    DTableWidget::mouseMoveEvent(event);
}

void DetailTreeView::leaveEvent(QEvent *event)
{
    if (mp_ToolTips) {
        mp_CurItem = nullptr;
        mp_ToolTips->hide();
    }
    DTableWidget::leaveEvent(event);
}

void DetailTreeView::slotTimeOut()
{
    showTips(mp_CurItem);
}

void DetailTreeView::slotItemEnterd(QTableWidgetItem *item)
{
    mp_CurItem = item;
}

void DetailTreeView::slotEnterBtnWidget()
{
    mp_CurItem = nullptr;
}

void DetailTreeView::slotLeaveBtnWidget()
{
    QPoint pt = this->mapFromGlobal(QCursor::pos());
    mp_CurItem = itemAt(pt);
}

void DetailTreeView::showTips(QTableWidgetItem *item)
{
    if (!mp_ToolTips) {
        mp_ToolTips = new TipsWidget(this);
    }

    if (item && item == mp_OldItem) {
        qint64 curMS = QDateTime::currentDateTime().toMSecsSinceEpoch();
        if (curMS - m_TimeStep > 1000 && mp_ToolTips->isHidden()) {
            QString text = item->text();
            QPoint showRealPos(QCursor::pos().x(), QCursor::pos().y() + 10);
            QRect screenRect = QApplication::desktop()->screenGeometry();
            if (mp_ToolTips) {
                // 确保tips不会出现一半在窗口，一半看不到
                if (showRealPos.x() + mp_ToolTips->width() > screenRect.width()) {
                    showRealPos.setX(showRealPos.x() - mp_ToolTips->width());
                }
                mp_ToolTips->setText(text);
                mp_ToolTips->move(showRealPos);
                mp_ToolTips->show();
            }
        }
    } else {
        m_TimeStep = QDateTime::currentDateTime().toMSecsSinceEpoch();
        mp_OldItem = item;
        if (mp_ToolTips) {
            mp_ToolTips->hide();
        }
    }
}
