#include "PageSingleInfo.h"
#include "MacroDefinition.h"
#include "DeviceInfo.h"

#include <QVBoxLayout>


#include <DStandardItem>
#include <DTableWidget>

PageSingleInfo::PageSingleInfo(QWidget *parent)
    : PageInfo(parent)
    , mp_Content(new DetailTreeView(this))
    , mp_Label(new DLabel(this))
{
    initWidgets();
}

PageSingleInfo::~PageSingleInfo()
{
    DELETE_PTR(mp_Content);
}

void PageSingleInfo::setLabel(const QString &itemstr)
{
    mp_Label->setText(itemstr);
    DFontSizeManager::instance()->bind(mp_Label, DFontSizeManager::T3);

}

void PageSingleInfo::updateInfo(const QList<DeviceBaseInfo *> &lst)
{
    clearContent();

    QList<QPair<QString, QString>> baseInfoMap = lst[0]->getBaseAttribs();
    QList<QPair<QString, QString>> otherInfoMap = lst[0]->getOtherAttribs();
    baseInfoMap = baseInfoMap + otherInfoMap;

    loadDeviceInfo(baseInfoMap);
}

void PageSingleInfo::loadDeviceInfo(const QList<QPair<QString, QString>> &lst)
{
    if (lst.size() < 1) {
        return;
    }

    int row = lst.size();
    mp_Content->setColumnAndRow(row + 1, 2);

    for (int i = 0; i < row; ++i) {

        QTableWidgetItem *itemFirst = new QTableWidgetItem(lst[i].first);
        mp_Content->setItem(i, 0, itemFirst);
        QTableWidgetItem *itemSecond = new QTableWidgetItem(lst[i].second);
        mp_Content->setItem(i, 1, itemSecond);
    }
}

void PageSingleInfo::clearContent()
{
    mp_Content->clear();
}

void PageSingleInfo::initWidgets()
{
    QVBoxLayout *hLayout = new QVBoxLayout(this);
    hLayout->addWidget(mp_Label);
    hLayout->addWidget(mp_Content);
    hLayout->addStretch();
    setLayout(hLayout);
}
