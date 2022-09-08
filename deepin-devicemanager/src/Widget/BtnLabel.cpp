// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BtnLabel.h"

#include <DDialog>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DApplication>

BtnLabel::BtnLabel(DWidget *parent)
    : DLabel(parent)
    , m_Desc("")
{
    // bug132075 安装成功状态此button无法点击
    // 仅安装失败可点击
    connect(this, &QLabel::linkActivated, this, [this]() {
        if (m_Desc.isEmpty())
            return;
        DDialog dialog;
        dialog.setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
        dialog.setTitle(m_Desc);
        dialog.addButton(tr("OK", "button"));
        dialog.exec();
    });
}

void BtnLabel::setDesc(const QString &txt)
{
    m_Desc = txt;
}

void BtnLabel::paintEvent(QPaintEvent* e)
{
    DPalette::ColorGroup cg;
    if (Qt::ApplicationActive == DApplication::applicationState()) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
    DPalette pa = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    pa.setColor(DPalette::Text, pa.color(cg, DPalette::TextTips));
    DApplicationHelper::instance()->setPalette(this, pa);

    return DLabel::paintEvent(e);
}


TipsLabel::TipsLabel(DWidget *parent)
    : DLabel(parent)
{

}

void TipsLabel::paintEvent(QPaintEvent* e)
{
    DPalette::ColorGroup cg;
    if (Qt::ApplicationActive == DApplication::applicationState()) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
    DPalette pa = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    pa.setColor(DPalette::Text, pa.color(cg, DPalette::TextTips));
    DApplicationHelper::instance()->setPalette(this, pa);

    return DLabel::paintEvent(e);
}

TitleLabel::TitleLabel(DWidget *parent)
    : DLabel(parent)
{
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
}

void TitleLabel::paintEvent(QPaintEvent* e)
{
    DPalette::ColorGroup cg;
    if (Qt::ApplicationActive == DApplication::applicationState()) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    DPalette pa = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    pa.setColor(DPalette::Text, pa.color(cg, DPalette::TextTitle));
    DApplicationHelper::instance()->setPalette(this, pa);

    return DLabel::paintEvent(e);
}


