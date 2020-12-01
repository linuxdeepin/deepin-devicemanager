// 项目自身文件
#include "DeviceWidget.h"

// Qt库文件
#include <QHBoxLayout>
#include <QDebug>

// 其它头文件
#include "PageListView.h"
#include "PageInfoWidget.h"
#include "../DeviceManager/DeviceInfo.h"
#include "MacroDefinition.h"
#include "DeviceManager.h"
#include "DeviceBios.h"

DeviceWidget::DeviceWidget(QWidget *parent)
    : DWidget(parent)
    , mp_ListView(new PageListView(this))
    , mp_PageInfo(new PageInfoWidget(this))
    , m_CurItemStr("")
{
    // 初始化界面布局
    initWidgets();

    // 连接槽函数
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

QString DeviceWidget::currentIndex() const
{
    // 当前设备类型
    return mp_ListView->currentType();
}

void DeviceWidget::slotListViewWidgetItemClicked(const QString &itemStr)
{
    // ListView Item 点击
    m_CurItemStr = itemStr;
    emit itemClicked(itemStr);
}

void DeviceWidget::slotRefreshInfo()
{
    // 刷新信息
    emit refreshInfo();
}
void DeviceWidget::slotExportInfo()
{
    // 导出信息
    emit exportInfo();
}

void DeviceWidget::slotUpdateUI()
{
    // 更新当前UI界面
    emit itemClicked(m_CurItemStr);
}

void DeviceWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
    // 获取所有设备类别
    const QList<QPair<QString, QString>> types = DeviceManager::instance()->getDeviceTypes();

    // 根据右侧Listview当前Index获取当前设备类别的
    QString userStr = mp_ListView->currentIndex();

    QString deviceType = "";
    foreach (auto iter, types) {
        if (iter.second.contains(userStr)) {
            deviceType = iter.first;
            break;
        }
    }

    // 根据设备类别获取设备指针
    QList<DeviceBaseInfo *> lst;
    bool ret = DeviceManager::instance()->getDeviceList(deviceType, lst);
    if (ret) {
        // 更新设备信息界面
        if (lst.size() > 1) {
            // 判断是否是BIOS界面
            DeviceBios *bios = dynamic_cast<DeviceBios *>(lst[0]);
            if (bios) {
                mp_PageInfo->updateTable(deviceType, lst);
            }
        } else {
            mp_PageInfo->updateTable(deviceType, lst);
        }

    } else {
        // 更新Overview界面
        QMap<QString, QString> overviewMap = DeviceManager::instance()->getDeviceOverview();
        mp_PageInfo->updateTable(deviceType, overviewMap);
    }
}

void DeviceWidget::initWidgets()
{
    // 初始化页面布局
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->addWidget(mp_ListView);
    hLayout->addWidget(mp_PageInfo);
    setLayout(hLayout);
}
