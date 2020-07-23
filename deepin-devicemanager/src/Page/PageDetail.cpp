#include "PageDetail.h"
#include "TextBrowser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPalette>
#include <DApplicationHelper>
#include <DApplication>
#include <DStyle>

DWIDGET_USE_NAMESPACE

DetailButton::DetailButton(const QString &txt)
    : DCommandLinkButton(txt)
{

}

void DetailButton::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    QBrush bgBrush(palette.color(cg, DPalette::Base));
    painter.fillRect(rect, bgBrush);

    painter.restore();
    DCommandLinkButton::paintEvent(e);
}

void DetailButton::initWidgets()
{

}

DetailSeperator::DetailSeperator(DWidget *parent)
    : DWidget(parent)
{
    setFixedHeight(2);
}

void DetailSeperator::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 分割线两端到边框的边距
    int spacing = 20;

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 清除背景色颜色
    QBrush clearBrush(palette.color(cg, DPalette::Base));
    painter.fillRect(rect, clearBrush);

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    rect.setX(rect.x() + spacing);
    rect.setWidth(rect.width() - spacing);
    QBrush bgBrush(palette.color(cg, DPalette::FrameShadowBorder));
    painter.fillRect(rect, bgBrush);

    painter.restore();
    DWidget::paintEvent(e);
}


PageDetail::PageDetail(QWidget *parent)
    : Dtk::Widget::DWidget(parent)
    , mp_ScrollAreaLayout(new QVBoxLayout(this))
    , mp_ScrollArea(new QScrollArea(this))
{
    setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *hLayout = new QVBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);

    // 设置scrollarea的属性
    mp_ScrollArea->setWidgetResizable(true);
    mp_ScrollArea->setFrameShape(QFrame::NoFrame);
    mp_ScrollArea->setContentsMargins(0, 0, 0, 0);
    mp_ScrollAreaLayout->setContentsMargins(0, 0, 0, 0);
    mp_ScrollAreaLayout->setSpacing(0);


    // 设置ScrollArea里面的widget,这个widget是必须要的
    DWidget *widget = new DWidget(this);
    widget->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(mp_ScrollAreaLayout);
    mp_ScrollArea->setWidget(widget);
    hLayout->addWidget(mp_ScrollArea);
    setLayout(hLayout);

//    mp_ToolBox->setContentsMargins(0, 0, 0, 0);
}

void PageDetail::showDeviceInfo(const QList<DeviceBaseInfo *> &lstInfo)
{
    foreach (auto device, lstInfo) {
        if (!device) {continue;}
        TextBrowser *txtBrowser = new TextBrowser(this);
        txtBrowser->showDeviceInfo(device);
        addWidgets(txtBrowser);
    }
}

void PageDetail::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取系统默认的圆角半径
//    QStyleOptionFrame option;
//    initStyleOption(&option);
//    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
//    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);
    int radius = 8;

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid/* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    QPainterPath paintPath;
    paintPath.addRoundedRect(rect, radius, radius);
    QBrush bgBrush(palette.color(cg, DPalette::Base));
    painter.fillPath(paintPath, bgBrush);

    painter.restore();
    DWidget::paintEvent(e);
}

void PageDetail::addWidgets(QWidget *widget)
{
    mp_ScrollAreaLayout->addWidget(widget);

    // 添加按钮
    QHBoxLayout *vLayout = new QHBoxLayout(this);
    vLayout->addSpacing(30);
    DetailButton *button = new DetailButton(tr("Details"));
    vLayout->addWidget(button);
    vLayout->addStretch(-1);
    mp_ScrollAreaLayout->addLayout(vLayout);

    // 添加分割线
    DetailSeperator *seperator = new DetailSeperator(widget);
    mp_ScrollAreaLayout->addWidget(seperator);
//    widget->setMinimumHeight(500);
}
