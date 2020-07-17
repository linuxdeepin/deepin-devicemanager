#include "PageSingleInfo.h"
#include "MacroDefinition.h"
#include "DeviceInfo.h"

#include <QHBoxLayout>

PageSingleInfo::PageSingleInfo(QWidget *parent)
    : PageInfo(parent)
    , mp_Content(new ContentTableView(this))
    , mpLabel(new DLabel(this))
{
    initWidgets();
}

PageSingleInfo::~PageSingleInfo()
{
    DELETE_PTR(mp_Content);
}

void PageSingleInfo::updateTable(const QList<DeviceBaseInfo *> &lst)
{
    QList<QPair<QString, QString>> baseInfoMap = lst[0]->getBaseAttribs();
    QList<QPair<QString, QString>> otherInfoMap = lst[0]->getBaseAttribs();
    baseInfoMap = baseInfoMap + otherInfoMap;

    loadDeviceInfo(baseInfoMap);
}

void PageSingleInfo::loadDeviceInfo(const QList<QPair<QString, QString>> &lst)
{
    if (lst.size() < 1) {
        return;
    }

    int row = lst.size();

    for (int i = 0; i < row; ++i) {
        DStandardItem *itemFirst = new DStandardItem(lst[i].first);
        mp_Content->setItem(i, 0, itemFirst);
        DStandardItem *itemSecond = new DStandardItem(lst[i].second);
        mp_Content->setItem(i, 1, itemSecond);
    }
}

void PageSingleInfo::initWidgets()
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    //mp_Content->setFixedHeight(200);
    hLayout->addWidget(mp_Content);
    setLayout(hLayout);
}
