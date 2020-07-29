#include "PageInfoWidget.h"

#include <QHBoxLayout>
#include <QDebug>

#include "PageMultiInfo.h"
#include "PageSingleInfo.h"
#include "PageOverview.h"

DWIDGET_USE_NAMESPACE
PageInfoWidget::PageInfoWidget(QWidget *parent)
    : DWidget(parent)
    , mp_PageInfo(nullptr)
    , mp_PageSignalInfo(new PageSingleInfo(this))
    , mp_PageMutilInfo(new PageMultiInfo(this))
    , mp_PageOverviewInfo(new PageOverview(this))
{
    initWidgets();
}

void PageInfoWidget::updateTable(const QString &itemStr, const QList<DeviceBaseInfo *> &lst)
{
    if (lst.size() == 0) {
        mp_PageOverviewInfo->setVisible(true);
        mp_PageSignalInfo->setVisible(false);
        mp_PageMutilInfo->setVisible(false);
        mp_PageInfo = mp_PageOverviewInfo;
    } else if (lst.size() == 1) {
        mp_PageOverviewInfo->setVisible(false);
        mp_PageSignalInfo->setVisible(true);
        mp_PageMutilInfo->setVisible(false);
        mp_PageInfo = mp_PageSignalInfo;
    } else {
        mp_PageOverviewInfo->setVisible(false);
        mp_PageSignalInfo->setVisible(false);
        mp_PageMutilInfo->setVisible(true);
        mp_PageInfo = mp_PageMutilInfo;

    }

    if (mp_PageInfo) {
        mp_PageInfo->updateInfo(lst);
        mp_PageInfo->setLabel(itemStr);
    }
}

void PageInfoWidget::updateTable(const QString &itemStr, const QMap<QString, QString> &map)
{
    mp_PageOverviewInfo->setVisible(true);
    mp_PageSignalInfo->setVisible(false);
    mp_PageMutilInfo->setVisible(false);
    mp_PageInfo = mp_PageOverviewInfo;
    if (mp_PageInfo) {
        mp_PageInfo->updateInfo(map);
        mp_PageInfo->setLabel(map["Overview"]);
    }
}

void PageInfoWidget::initWidgets()
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->addWidget(mp_PageSignalInfo);
    hLayout->addWidget(mp_PageMutilInfo);
    hLayout->addWidget(mp_PageOverviewInfo);
    setLayout(hLayout);

    mp_PageSignalInfo->setVisible(false);
    mp_PageMutilInfo->setVisible(false);
}
