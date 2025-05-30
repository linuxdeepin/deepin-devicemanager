// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GetDriverPathWidget.h"
#include "UrlChooserEdit.h"
#include "MacroDefinition.h"
#include "DDLog.h"

#include <DGuiApplicationHelper>
#include <DPaletteHelper>

using namespace DDLog;

GetDriverPathWidget::GetDriverPathWidget(QWidget *parent)
    : DWidget(parent)
    , mp_fileChooseEdit(new UrlChooserEdit(this))
    , mp_includeCheckBox(new DCheckBox(QObject::tr("Include subfolders"), this))
    , mp_tipLabel(new DLabel)
{
    qCDebug(appLog) << "GetDriverPathWidget instance created";
    init();
    connect(mp_fileChooseEdit, &UrlChooserEdit::signalNotLocalFolder, this, &GetDriverPathWidget::signalNotLocalFolder);
    connect(mp_fileChooseEdit, &UrlChooserEdit::signalNotLocalFolder, this, &GetDriverPathWidget::clearTipLabelText);
}

GetDriverPathWidget::~GetDriverPathWidget()
{
    DELETE_PTR(mp_tipLabel);
    DELETE_PTR(mp_titleLabel);
}

QString GetDriverPathWidget::path()
{
    return mp_fileChooseEdit->text();
}

bool GetDriverPathWidget::includeSubdir()
{
    qCDebug(appLog) << "Checking include subdir:" << (mp_includeCheckBox->checkState() == Qt::Checked);

    return mp_includeCheckBox->checkState() == Qt::Checked;
}

void GetDriverPathWidget::updateTipLabelText(const QString &text)
{
    qCDebug(appLog) << "Updating tip label text:" << text;

    mp_tipLabel->setText(text);
    mp_tipLabel->setToolTip(text);
}

void GetDriverPathWidget::clearTipLabelText()
{
    updateTipLabelText("");
}

void GetDriverPathWidget::init()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 10, 0, 0);

    QHBoxLayout *hLayout1 = new QHBoxLayout;
    mp_titleLabel = new DLabel(QObject::tr("Search for drivers in this path"), this);
    hLayout1->addStretch();
    hLayout1->addWidget(mp_titleLabel);
    hLayout1->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(hLayout1);
    mainLayout->addWidget(mp_fileChooseEdit);
    mainLayout->addWidget(mp_includeCheckBox);
    mp_includeCheckBox->setCheckState(Qt::Checked);
    mainLayout->addSpacing(20);
    mainLayout->addStretch();
    mainLayout->addSpacing(20);
    mainLayout->addWidget(mp_tipLabel);

    mp_tipLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    mp_tipLabel->setElideMode(Qt::ElideRight);
    mp_tipLabel->setMinimumHeight(20);


    DPalette pa = DPaletteHelper::instance()->palette(mp_titleLabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    QColor color = DGuiApplicationHelper::adjustColor(pa.color(QPalette::Active, QPalette::BrightText), 0, 0, 0, 0, 0, 0, -30);
    pa.setColor(QPalette::WindowText, color);
    mp_titleLabel->setPalette(pa);

    pa = DPaletteHelper::instance()->palette(mp_titleLabel);
    pa.setColor(DPalette::WindowText, pa.color(DPalette::TextWarning));
    DPaletteHelper::instance()->setPalette(mp_titleLabel, pa);

    this->setLayout(mainLayout);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &GetDriverPathWidget::onUpdateTheme);
}
void GetDriverPathWidget::onUpdateTheme()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    plt.setColor(Dtk::Gui::DPalette::Background, plt.color(Dtk::Gui::DPalette::Base));
#else
    QPalette plt = mp_titleLabel->palette();
    plt.setColor(QPalette::Window, plt.color(QPalette::Base));
#endif
    mp_titleLabel->setPalette(plt);
}
