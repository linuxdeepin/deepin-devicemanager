#include "PageMultiInfo.h"

#include <QVBoxLayout>
#include <QAction>
#include <QIcon>
#include <DMenu>

#include "PageTableHeader.h"
#include "PageDetail.h"
#include "MacroDefinition.h"
#include "DeviceManager/DeviceInfo.h"

PageMultiInfo::PageMultiInfo(QWidget *parent)
    : PageInfo(parent)
    , mp_Table(new PageTableHeader(this))
    , mp_Detail(new PageDetail(this))
{
    initWidgets();
}

PageMultiInfo::~PageMultiInfo()
{
    DELETE_PTR(mp_Table);
    DELETE_PTR(mp_Detail);
}

void PageMultiInfo::updateInfo(const QList<DeviceBaseInfo *> &lst)
{
    QList<QStringList> deviceList;
    deviceList.append(lst[0]->getTableHeader());
    foreach (DeviceBaseInfo *info, lst) {
        if (info->getTableData().size() > 0)
            deviceList.append(info->getTableData());
    }
    mp_Table->updateTable(deviceList);
    mp_Detail->showDeviceInfo(lst);
}

void PageMultiInfo::initWidgets()
{
    QVBoxLayout *hLayout = new QVBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    mp_Table->setFixedHeight(TABLE_HEIGHT);
    hLayout->addWidget(mp_Table);
    hLayout->addWidget(mp_Detail);
    setLayout(hLayout);
}
