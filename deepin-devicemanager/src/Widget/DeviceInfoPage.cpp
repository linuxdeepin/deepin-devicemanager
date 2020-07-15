#include "DeviceInfoPage.h"

#include <QVBoxLayout>

#include "DeviceTablePage.h"
#include "DeviceDetailPage.h"
#include "MacroDefinition.h"

DeviceInfoPage::DeviceInfoPage(QWidget *parent)
    : DWidget(parent)
    , mp_Table(new DeviceTablePage(this))
    , mp_Detail(new DeviceDetailPage(this))
    , mp_Splitter(new DSplitter(Qt::Vertical, this))

{
    initWidgets();
}

DeviceInfoPage::~DeviceInfoPage()
{
    DELETE_PTR(mp_Table);
    DELETE_PTR(mp_Detail);
}

void DeviceInfoPage::initWidgets()
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    mp_Splitter->addWidget(mp_Table);
    mp_Splitter->addWidget(mp_Detail);
    mp_Splitter->setStretchFactor(0, 1);
    mp_Splitter->setStretchFactor(1, 4);
    hLayout->addWidget(mp_Splitter);
    setLayout(hLayout);
}
