#include "GetDriverPathWidget.h"
#include "UrlChooserEdit.h"

DWIDGET_BEGIN_NAMESPACE

GetDriverPathWidget::GetDriverPathWidget(QWidget *parent)
    : DWidget (parent)
    , mp_fileChooseEdit(new UrlChooserEdit(this))
    , mp_includeCheckBox(new DCheckBox(QObject::tr("Include subfolders"), this))
{
    init();
    connect(mp_fileChooseEdit, &UrlChooserEdit::signalNotLocalFolder, this, &GetDriverPathWidget::signalNotLocalFolder);
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

void GetDriverPathWidget::init()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0,10,0,10);

    QHBoxLayout *hLayout1 = new QHBoxLayout;
    DLabel *titleLable = new DLabel(QObject::tr("Search for drivers in this path"),this);
    hLayout1->addStretch();
    hLayout1->addWidget(titleLable);
    hLayout1->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(hLayout1);
    mainLayout->addWidget(mp_fileChooseEdit);
    mainLayout->addWidget(mp_includeCheckBox);
    mp_includeCheckBox->setCheckState(Qt::Checked);
    mainLayout->addStretch();
    this->setLayout(mainLayout);
}

DWIDGET_END_NAMESPACE
