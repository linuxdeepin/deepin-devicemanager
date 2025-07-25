// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PageDetail.h"
#include "TextBrowser.h"
#include "DeviceInfo.h"
#include "MacroDefinition.h"
#include "DDLog.h"

// Dtk头文件
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DStyle>
#include <DMenu>
#include <DFontSizeManager>

// Qt库文件
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPalette>
#include <QLoggingCategory>
#include <QScrollBar>
#include <QAction>
#include <QClipboard>
#include <QPainterPath>

// 宏定义
#define SPACE_HEIGHT 0  //
#define SEPERATOR_HEIGHT 10  // 分割线上下距离
#define MIN_HEIGHT 50 // 当前Widget的最小高度

DWIDGET_USE_NAMESPACE

using namespace DDLog;

DetailButton::DetailButton(const QString &txt)
    : DCommandLinkButton(txt)
{
    qCDebug(appLog) << "DetailButton constructor";
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

void DetailButton::updateText()
{
    qCDebug(appLog) << "DetailButton::updateText start";
    if (this->text() == tr("More")) {
        qCDebug(appLog) << "DetailButton::updateText set text to Collapse";
        this->setText(tr("Collapse"));
    } else {
        qCDebug(appLog) << "DetailButton::updateText set text to More";
        this->setText(tr("More"));
    }
}

void DetailButton::paintEvent(QPaintEvent *e)
{
    qCDebug(appLog) << "DetailButton::paintEvent";
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid /* && wid == this*/) {
        cg = DPalette::Active;
    } else {
        qCDebug(appLog) << "DetailButton::paintEvent, inactive window";
        cg = DPalette::Inactive;
    }

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    QBrush bgBrush(palette.color(cg, DPalette::Base));
    painter.fillRect(rect, bgBrush);

    painter.restore();
    DCommandLinkButton::paintEvent(e);
}

DetailSeperator::DetailSeperator(DWidget *parent)
    : DWidget(parent)
{
    qCDebug(appLog) << "DetailSeperator constructor";
    // 设置分隔符高度一个像素
    setFixedHeight(LINE_WIDTH);
}

void DetailSeperator::paintEvent(QPaintEvent *e)
{
    // qCDebug(appLog) << "DetailSeperator::paintEvent";
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 分割线两端到边框的边距
    int spacing = 5;

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid /* && wid == this*/) {
        // qCDebug(appLog) << "DetailSeperator::paintEvent, active window";
        cg = DPalette::Active;
    } else {
        // qCDebug(appLog) << "DetailSeperator::paintEvent, inactive window";
        cg = DPalette::Inactive;
    }

    // 清除背景色颜色
    QBrush clearBrush(palette.color(cg, DPalette::Base));
    painter.fillRect(rect, clearBrush);

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    rect.setX(rect.x() + spacing);
    rect.setWidth(rect.width() - spacing);
    rect.setHeight(1);
//    rect.setY(rect.y() + height() - 2);
    QBrush bgBrush(palette.color(cg, DPalette::FrameBorder));
    painter.fillRect(rect, bgBrush);

    painter.restore();
    DWidget::paintEvent(e);
}

ScrollAreaWidget::ScrollAreaWidget(DWidget *parent)
    : DWidget(parent)
{
    // qCDebug(appLog) << "ScrollAreaWidget constructor";
}

void ScrollAreaWidget::paintEvent(QPaintEvent *e)
{
    // qCDebug(appLog) << "ScrollAreaWidget::paintEvent";
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid /* && wid == this*/) {
        // qCDebug(appLog) << "ScrollAreaWidget::paintEvent, active window";
        cg = DPalette::Active;
    } else {
        // qCDebug(appLog) << "ScrollAreaWidget::paintEvent, inactive window";
        cg = DPalette::Inactive;
    }

    // 清除背景色颜色
    QBrush clearBrush(palette.color(cg, DPalette::Base));
    painter.fillRect(rect, clearBrush);

    painter.restore();
    DWidget::paintEvent(e);
}

PageDetail::PageDetail(QWidget *parent)
    : Dtk::Widget::DWidget(parent)
    , mp_ScrollAreaLayout(nullptr)
    , mp_ScrollArea(new QScrollArea(this))
    , mp_ScrollWidget(new ScrollAreaWidget(this))
{
    qCDebug(appLog) << "PageDetail constructor start";
    this->setMinimumHeight(MIN_HEIGHT);

    setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *hLayout = new QVBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);

    // 设置scrollarea的属性
    mp_ScrollArea->setWidgetResizable(true);
    mp_ScrollArea->setFrameShape(QFrame::NoFrame);
    mp_ScrollArea->setContentsMargins(0, 0, 0, 0);

    // 设置ScrollArea里面的widget,这个widget是必须要的
    mp_ScrollWidget->setContentsMargins(15, 0, 0, 0);
    mp_ScrollArea->setWidget(mp_ScrollWidget);
    hLayout->addWidget(mp_ScrollArea);
    setLayout(hLayout);

    clearWidget();
}

void PageDetail::showDeviceInfo(const QList<DeviceBaseInfo *> &lstInfo)
{
    qCDebug(appLog) << "Showing device info for" << lstInfo.size() << "devices";
    // Clear widgets first
    clearWidget();

    // Create widgets for showing device info
    foreach (auto device, lstInfo) {
        // qCDebug(appLog) << "PageDetail::showDeviceInfo process device:" << device->name();
        if (!device) {
            // qCDebug(appLog) << "PageDetail::showDeviceInfo device is null, continue";
            continue;
        }
        TextBrowser *txtBrowser = new TextBrowser(this);
        txtBrowser->showDeviceInfo(device);
        connect(txtBrowser, &TextBrowser::refreshInfo, this, &PageDetail::refreshInfo);
        connect(txtBrowser, &TextBrowser::exportInfo, this, &PageDetail::exportInfo);
        connect(txtBrowser, &TextBrowser::copyAllInfo, this, &PageDetail::slotCopyAllInfo);
        addWidgets(txtBrowser, device->enable() && device->available() && !device->getOtherAttribs().isEmpty());
        // 当添加到最后一个设备详细信息时，隐藏分隔符
        if (device == lstInfo.last()) {
            // qCDebug(appLog) << "PageDetail::showDeviceInfo is last device, hide seperator";
            m_ListDetailSeperator[lstInfo.size() - 1]->setVisible(false);
        }
    }
    // 刷新展示页面时,滚动条还原
    mp_ScrollArea->verticalScrollBar()->setValue(0);

    // 页面布局时将所有的textBrowser靠上显示
    mp_ScrollAreaLayout->addStretch();
}

void PageDetail::showInfoOfNum(int index)
{
    qCDebug(appLog) << "PageDetail::showInfoOfNum start, index:" << index;
    if (index >= m_ListHlayout.size()
            || index >= m_ListTextBrowser.size()
            || index >= m_ListDetailButton.size()
            || index >= m_ListDetailSeperator.size()) {
        qCWarning(appLog) << "PageDetail::showInfoOfNum index out of range";
        return;
    }
    int value = 0;
    for (int i = 0; i <= index - 1; i++) {
        // qCDebug(appLog) << "PageDetail::showInfoOfNum calculate scroll value, i:" << i;
        value += m_ListTextBrowser[i]->height();
        value += m_ListDetailButton[i]->height();
        value += m_ListDetailSeperator[i]->height();
        value += SPACE_HEIGHT;
    }
    mp_ScrollArea->verticalScrollBar()->setValue(value);
    qCDebug(appLog) << "PageDetail::showInfoOfNum end, value:" << value;
}

EnableDeviceStatus PageDetail::enableDevice(int row, bool enable)
{
    qCDebug(appLog) << "Setting device enable state, row:" << row << "enable:" << enable;
    if (m_ListTextBrowser.size() <= row) {
        qCWarning(appLog) << "Invalid row index:" << row;
        return EDS_Cancle;
    }

    // 设置 TextBrowser 可用
    TextBrowser *browser = m_ListTextBrowser.at(row);
    if (!browser) {
        qCWarning(appLog) << "PageDetail::enableDevice browser is null";
        return EDS_Cancle;
    }

    return browser->setDeviceEnabled(enable);
}

void PageDetail::setWakeupMachine(int row, bool wakeup)
{
    qCDebug(appLog) << "Setting wakeup machine state, row:" << row << "wakeup:" << wakeup;
    if (m_ListTextBrowser.size() <= row) {
        qCWarning(appLog) << "Invalid row index:" << row;
        return;
    }
    // 设置 TextBrowser 可用
    TextBrowser *browser = m_ListTextBrowser.at(row);
    if (!browser)
        return ;

    browser->setWakeupMachine(wakeup);
}

void PageDetail::paintEvent(QPaintEvent *e)
{
    // qCDebug(appLog) << "PageDetail::paintEvent";
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取系统默认的圆角半径
    int radius = 8;

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid /* && wid == this*/) {
        qCDebug(appLog) << "PageDetail::paintEvent, active window";
        cg = DPalette::Active;
    } else {
        qCDebug(appLog) << "PageDetail::paintEvent, inactive window";
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

void PageDetail::resizeEvent(QResizeEvent *event)
{
    // qCDebug(appLog) << "PageDetail::resizeEvent";
    DWidget::resizeEvent(event);
}

void PageDetail::addWidgets(TextBrowser *widget, bool enable)
{
    qCDebug(appLog) << "PageDetail::addWidgets start, enable:" << enable;
    // 添加 textBrowser
    if (widget != nullptr && m_ListTextBrowser.size() != 0)
        mp_ScrollAreaLayout->addSpacing(SEPERATOR_HEIGHT);

    mp_ScrollAreaLayout->addWidget(widget);

    // 添加按钮
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addSpacing(SPACE_HEIGHT);
    DetailButton *button = new DetailButton(tr("More"));
    DFontSizeManager::instance()->bind(button, DFontSizeManager::T8);
    connect(button, &DetailButton::clicked, this, &PageDetail::slotBtnClicked);
    if (!enable)
        button->setVisible(false);

    hLayout->addWidget(button);
    hLayout->addStretch();
    mp_ScrollAreaLayout->addLayout(hLayout);

    // 添加分割线
    mp_ScrollAreaLayout->addSpacing(SEPERATOR_HEIGHT);
    DetailSeperator *seperator = new DetailSeperator(widget);
    mp_ScrollAreaLayout->addWidget(seperator);

    // **********************************
    m_ListTextBrowser.append(widget);
    m_ListHlayout.append(hLayout);
    m_ListDetailButton.append(button);
    m_ListDetailSeperator.append(seperator);
    qCDebug(appLog) << "PageDetail::addWidgets end";
}

void PageDetail::clearWidget()
{
    qCDebug(appLog) << "PageDetail::clearWidget start";
    QList<TextBrowser *> listTextBrowser = m_ListTextBrowser;
    m_ListTextBrowser.clear();
    //  清空TextBrowser
    foreach (auto widget, listTextBrowser) {
        widget->deleteLater();
        widget = nullptr;
    }

    QList<QHBoxLayout *> listHlayout = m_ListHlayout;
    m_ListHlayout.clear();
    //  清空layout
    foreach (auto widget, listHlayout) {
        widget->deleteLater();
        widget = nullptr;
    }

    QList<DetailButton *> listDetailButton = m_ListDetailButton;
    m_ListDetailButton.clear();
    // 清空DetailButton
    foreach (auto widget, listDetailButton) {
        widget->deleteLater();
        widget = nullptr;
    }

    QList<DetailSeperator *> listDetailSeperator = m_ListDetailSeperator;
    m_ListDetailSeperator.clear();
    // 清空Seperator
    foreach (auto widget, listDetailSeperator) {
        widget->deleteLater();
        widget = nullptr;
    }

    // 删除最后的一个弹簧
    if (mp_ScrollAreaLayout) {
        while (!mp_ScrollAreaLayout->isEmpty()) {
            QLayoutItem *layoutItem = mp_ScrollAreaLayout->itemAt(0);
            mp_ScrollAreaLayout->removeItem(layoutItem);
            delete  layoutItem;
        }
        delete  mp_ScrollAreaLayout;
    }

    mp_ScrollAreaLayout = new QVBoxLayout();
    mp_ScrollAreaLayout->setContentsMargins(0, 0, 0, 0);
    mp_ScrollAreaLayout->setSpacing(0);
    mp_ScrollWidget->setLayout(mp_ScrollAreaLayout);
    qCDebug(appLog) << "PageDetail::clearWidget end";
}

void PageDetail::slotBtnClicked()
{
    qCDebug(appLog) << "PageDetail::slotBtnClicked start";
    DetailButton *button = qobject_cast<DetailButton *>(sender());
    if (!button) {
        qCWarning(appLog) << "PageDetail::slotBtnClicked, invalid button";
        return;
    }
    int index = 0;
    foreach (DetailButton *b, m_ListDetailButton) {
        if (button == b)
            break;
        index++;
    }

    // 改变按钮的状态，展开和收起的切换
    if (button)
        button->updateText();

    // 显示内容发生相应变化，也就是是否显示其它信息
    TextBrowser *browser = m_ListTextBrowser[index];
    if (browser) {
        browser->updateShowOtherInfo();
        browser->updateInfo();
    }
}

void PageDetail::slotCopyAllInfo()
{
    qCDebug(appLog) << "Copying all device info to clipboard";
    QString str;
    foreach (TextBrowser *tb, m_ListTextBrowser) {
        if (tb)
            str.append(tb->toPlainText());
    }
    QClipboard *clipboard = DApplication::clipboard();
    clipboard->setText(str);
}
