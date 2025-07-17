// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TipsWidget.h"
#include "DDLog.h"

#include <DPlatformWindowHandle>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QPainter>
#include <QLoggingCategory>

using namespace DDLog;

TipsWidget::TipsWidget(QWidget *parent) : DWidget(parent)
{
    qCDebug(appLog) << "TipsWidget instance created";

    setWindowFlags(Qt::ToolTip);
    initWidget();
    onUpdateTheme();
}

void TipsWidget::initWidget()
{
    qCDebug(appLog) << "initWidget";
    m_tbMargin = 8;
    m_lrMargin = 10;
    m_maxLineCount = 10;
    m_alignment = Qt::AlignLeft | Qt::AlignVCenter;
    setFixedWidth(254);
    setWindowOpacity(0.97);
    DPlatformWindowHandle handle(this);
    handle.setWindowRadius(8);

    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &TipsWidget::onUpdateTheme);
}

void TipsWidget::onUpdateTheme()
{
    qCDebug(appLog) << "onUpdateTheme";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    plt.setColor(Dtk::Gui::DPalette::Background, plt.color(Dtk::Gui::DPalette::Base));
#else
    DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    plt.setColor(QPalette::Window, plt.color(QPalette::Base));
#endif
    setPalette(plt);
}

void TipsWidget::setText(const QString &text)
{
    qCDebug(appLog) << "Setting tips text:" << text;

    if (text != m_text) {
        qCDebug(appLog) << "Text changed, updating widget";
        m_text = text;
        m_text.replace(QChar('\n'), QString(""));
        m_text.replace(QChar('\t'), QString(""));
        update();
    }
}

void TipsWidget::setAlignment(Qt::Alignment alignment)
{
    qCDebug(appLog) << "Setting alignment:" << alignment;
    m_alignment = alignment;
    update();
}

void TipsWidget::paintEvent(QPaintEvent *event)
{
    // qCDebug(appLog) << "Painting widget";
    DWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QTextOption option;
    const QString &text = m_text;
    adjustContent(text);
    option.setAlignment(m_alignment);
    option.setWrapMode(QTextOption::WrapAnywhere);
    painter.setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().text().color()));
    painter.drawText(QRect(m_lrMargin, m_tbMargin, this->width() -  2 * m_lrMargin, this->height() -  2 * m_tbMargin), text, option);
}

void TipsWidget::adjustContent(const QString &text)
{
    qCDebug(appLog) << "Adjusting content";
    QFontMetricsF fontMetris(this->font());
    int wordHeight = static_cast<int>(fontMetris.boundingRect(QRectF(0, 0, this->width() - 2 * m_lrMargin, 0),
                                                              static_cast<int>(m_alignment | Qt::TextWrapAnywhere), text).height() + 2 * m_tbMargin);
    if (this->height() == wordHeight) {
        qCDebug(appLog) << "Height is already correct";
        return;
    }
    qCDebug(appLog) << "Adjusting widget height to" << wordHeight;
    setFixedHeight(wordHeight);
}
