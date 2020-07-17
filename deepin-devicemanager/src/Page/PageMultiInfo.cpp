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
    , mp_Splitter(new DSplitter(Qt::Vertical, this))
{
    initWidgets();
}

PageMultiInfo::~PageMultiInfo()
{
    DELETE_PTR(mp_Table);
    DELETE_PTR(mp_Detail);

}

void PageMultiInfo::updateTable(const QList<DeviceBaseInfo *> &lst)
{
    QList<QStringList> deviceList;
    deviceList.append(lst[0]->getTableHeader());
    foreach (DeviceBaseInfo *info, lst) {
        if (info->getTableData().size() > 0)
            deviceList.append(info->getTableData());
    }
    mp_Table->updateTable(deviceList);
}

void PageMultiInfo::initWidgets()
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    mp_Splitter->addWidget(mp_Table);
    mp_Splitter->addWidget(mp_Detail);
    mp_Splitter->setStretchFactor(0, 1);
    mp_Splitter->setStretchFactor(1, 4);
    hLayout->addWidget(mp_Splitter);
    setLayout(hLayout);
}
