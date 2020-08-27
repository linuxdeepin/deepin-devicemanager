#include "DeviceWidget.h"

#include "PageListView.h"
#include "PageInfoWidget.h"
#include "../DeviceManager/DeviceInfo.h"
#include "MacroDefinition.h"

#include <QHBoxLayout>
#include <QDebug>

DeviceWidget::DeviceWidget(QWidget *parent)
    : DWidget(parent)
    , mp_ListView(new PageListView(this))
    , mp_PageInfo(new PageInfoWidget(this))
    , m_CurItemStr("")
{
    // 初始化界面布局
    initWidgets();

    // 连接曹函数
    connect(mp_ListView, &PageListView::itemClicked, this, &DeviceWidget::slotListViewWidgetItemClicked);
    connect(mp_PageInfo, &PageInfoWidget::refreshInfo, this, &DeviceWidget::slotRefreshInfo);
    connect(mp_PageInfo, &PageInfoWidget::exportInfo, this, &DeviceWidget::slotExportInfo);
    connect(mp_PageInfo, &PageInfoWidget::updateUI, this, &DeviceWidget::slotUpdateUI);

    connect(mp_ListView, &PageListView::refreshActionTrigger, this, &DeviceWidget::slotRefreshInfo);
    connect(mp_ListView, &PageListView::exportActionTrigger, this, &DeviceWidget::slotExportInfo);
}

DeviceWidget::~DeviceWidget()
{
    DELETE_PTR(mp_ListView);
    DELETE_PTR(mp_PageInfo);
}

void DeviceWidget::updateListView(const QList<QPair<QString, QString> > &lst)
{
    // 更新左边的列表
    if (mp_ListView)
        mp_ListView->updateListItems(lst);
}

void DeviceWidget::updateDevice(const QString &itemStr, const QList<DeviceBaseInfo *> &lst)
{
    if (lst.size() == 0) {
        return;
    }

    // 更新右边的详细内容
    if (mp_PageInfo) {
        mp_PageInfo->updateTable(itemStr, lst);
    }
}

void DeviceWidget::updateOverview(const QString &itemStr, const QMap<QString, QString> &map)
{
    if (map.size() == 0) {
        return;
    }

    // 更新概况
    if (mp_PageInfo) {
        mp_PageInfo->updateTable(itemStr, map);
    }
}

void DeviceWidget::slotListViewWidgetItemClicked(const QString &itemStr)
{
    m_CurItemStr = itemStr;
    emit itemClicked(itemStr);
}

void DeviceWidget::slotRefreshInfo()
{
    emit refreshInfo();
}
void DeviceWidget::slotExportInfo()
{
    emit exportInfo();
}

void DeviceWidget::slotUpdateUI()
{

    emit itemClicked(m_CurItemStr);
}

void DeviceWidget::initWidgets()
{
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(mp_ListView);
    hLayout->addWidget(mp_PageInfo);
    setLayout(hLayout);
}

