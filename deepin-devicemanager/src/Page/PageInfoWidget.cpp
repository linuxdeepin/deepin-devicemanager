#include "PageInfoWidget.h"

#include <QHBoxLayout>

#include "PageMultiInfo.h"
#include "PageSingleInfo.h"

DWIDGET_USE_NAMESPACE
PageInfoWidget::PageInfoWidget(QWidget *parent)
    : DWidget(parent)
    , mp_PageInfo(new PageMultiInfo(this))
{
    initWidgets();
}

void PageInfoWidget::updateTable(const QString &itemStr, const QList<DeviceBaseInfo *> &lst)
{
    if (mp_PageInfo) {

        mp_PageInfo->updateInfo(lst);
        mp_PageInfo->setLabel(itemStr);

    }
}

void PageInfoWidget::initWidgets()
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
//    hLayout->setContentsMargins(5, 5, 5, 5);
    hLayout->addWidget(mp_PageInfo);
    setLayout(hLayout);
}
