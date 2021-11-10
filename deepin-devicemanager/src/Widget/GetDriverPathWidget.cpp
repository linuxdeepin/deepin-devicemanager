#include "GetDriverPathWidget.h"
#include "UrlChooserEdit.h"

#include <DApplicationHelper>

GetDriverPathWidget::GetDriverPathWidget(QWidget *parent)
    : DWidget(parent)
    , mp_fileChooseEdit(new UrlChooserEdit(this))
    , mp_includeCheckBox(new DCheckBox(QObject::tr("Include subfolders"), this))
    , mp_tipLabel(new DLabel)
{
    init();
    connect(mp_fileChooseEdit, &UrlChooserEdit::signalNotLocalFolder, this, &GetDriverPathWidget::signalNotLocalFolder);
    connect(mp_fileChooseEdit, &UrlChooserEdit::signalNotLocalFolder, this, &GetDriverPathWidget::clearTipLabelText);
}

GetDriverPathWidget::~GetDriverPathWidget()
{
}

QString GetDriverPathWidget::path()
{
    return mp_fileChooseEdit->text();
}

bool GetDriverPathWidget::includeSubdir()
{
    return mp_includeCheckBox->checkState() == Qt::Checked;
}

void GetDriverPathWidget::updateTipLabelText(const QString &text)
{
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
    DLabel *titleLable = new DLabel(QObject::tr("Search for drivers in this path"), this);
    hLayout1->addStretch();
    hLayout1->addWidget(titleLable);
    hLayout1->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(hLayout1);
    mainLayout->addWidget(mp_fileChooseEdit);
    mainLayout->addWidget(mp_includeCheckBox);
    mp_includeCheckBox->setCheckState(Qt::Checked);
    mainLayout->addStretch();
    mainLayout->addWidget(mp_tipLabel);

    mp_tipLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    mp_tipLabel->setElideMode(Qt::ElideRight);
    mp_tipLabel->setMinimumHeight(20);

    DPalette pa = DApplicationHelper::instance()->palette(mp_tipLabel);
    pa.setColor(DPalette::WindowText, pa.color(DPalette::TextWarning));
    DApplicationHelper::instance()->setPalette(mp_tipLabel, pa);

    this->setLayout(mainLayout);
}
