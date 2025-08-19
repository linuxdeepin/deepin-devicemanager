// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverWaitingWidget.h"
#include "DDLog.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QVBoxLayout>

using namespace DDLog;

DriverWaitingWidget::DriverWaitingWidget(const QString &status, QWidget *parent)
    : DWidget(parent)
    , mp_Progress(new DWaterProgress(this))
    , mp_Label(new DLabel(status, this))
{
    qCDebug(appLog) << "DriverWaitingWidget instance created with status:" << status;
    init();
}

void DriverWaitingWidget::init()
{
    qCDebug(appLog) << "Initializing driver waiting widget";
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 70);

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    mainLayout->addItem(verticalSpacer);
    mainLayout->addStretch();
    mainLayout->addWidget(mp_Progress, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    mp_Progress->setFixedSize(80, 80);
    mp_Progress->setValue(50);
    mp_Progress->start();

    DFontSizeManager::instance()->bind(mp_Label, DFontSizeManager::T7, QFont::Medium);
    mainLayout->addWidget(mp_Label, 0, Qt::AlignCenter);
    mainLayout->addSpacing(6);

    mainLayout->addStretch();
    setLayout(mainLayout);

    // 设置文字颜色
    QPalette palette = mp_Label->palette();
    QColor color = DGuiApplicationHelper::adjustColor(palette.color(QPalette::Active, QPalette::BrightText), 0, 0, 0, 0, 0, 0, -10);
    palette.setColor(QPalette::WindowText, color);
    mp_Label->setPalette(palette);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &DriverWaitingWidget::onUpdateTheme);
}
void DriverWaitingWidget::onUpdateTheme()
{
    qCDebug(appLog) << "Updating theme";
    DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    plt.setColor(Dtk::Gui::DPalette::Background, plt.color(Dtk::Gui::DPalette::Base));
#else
    plt.setColor(QPalette::Window, plt.color(QPalette::Base));
#endif
    mp_Label->setPalette(plt);
}
void DriverWaitingWidget::setValue(int value)
{
    qCDebug(appLog) << "Setting progress value:" << value;
    mp_Progress->setValue(value);
    mp_Progress->repaint();
}

void DriverWaitingWidget::setText(const QString &text)
{
    qCDebug(appLog) << "Setting label text:" << text;

    mp_Label->setText(text);
}
