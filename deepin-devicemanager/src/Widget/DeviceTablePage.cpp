#include "DeviceTablePage.h"

#include <QHBoxLayout>

#include "DeviceTableView.h"

DeviceTablePage::DeviceTablePage(QWidget *parent)
    : DWidget(parent)
    , mp_Table(new DeviceTableView(this))
{
    initWidgets();
}

void DeviceTablePage::initWidgets()
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(mp_Table);
    setLayout(hLayout);
}
