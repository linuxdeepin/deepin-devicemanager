#include "DriverWaitingWidget.h"

#include <QVBoxLayout>
#include <DFontSizeManager>

DriverWaitingWidget::DriverWaitingWidget(QString status, QWidget *parent)
    : DWidget(parent)
    , mp_Progress(new DWaterProgress(this))
    , mp_Label(new DLabel(status, this))
{
    init();
}

void DriverWaitingWidget::init()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addStretch();

    mp_Progress->setFixedSize(80, 80);
    mp_Progress->setValue(50);
    mp_Progress->start();
    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->addStretch();
    hLayout1->addWidget(mp_Progress);
    hLayout1->addStretch();
    mainLayout->addLayout(hLayout1);

    DFontSizeManager::instance()->bind(mp_Label, DFontSizeManager::T7, QFont::Medium);
    QHBoxLayout *hLayout2 = new QHBoxLayout;
    hLayout2->addStretch();
    hLayout2->addWidget(mp_Label);
    hLayout2->addStretch();
    mainLayout->addLayout(hLayout2);

    mainLayout->addStretch();
    setLayout(mainLayout);
}

void DriverWaitingWidget::setValue(int value)
{
    mp_Progress->setValue(value);
}

void DriverWaitingWidget::setText(const QString &text)
{
    mp_Label->setText(text);
}
