// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 其它头文件
#include "DeviceWidget.h"
#include "PageListView.h"
#include "PageInfoWidget.h"
#include "DeviceInfo.h"
#include "MacroDefinition.h"
#include "DeviceManager.h"
#include "DeviceBios.h"
#include "DDLog.h"

// Qt库文件
#include <QHBoxLayout>
#include <QLoggingCategory>

using namespace DDLog;


DeviceWidget::DeviceWidget(QWidget *parent)
    : DWidget(parent)
    , mp_ListView(new PageListView(this))
    , mp_PageInfo(new PageInfoWidget(this))
    , m_CurItemStr("")
    , m_Layout(nullptr)
{
    qCDebug(appLog) << "DeviceWidget constructor start";
    // 初始化界面布局
    initWidgets();
    qCDebug(appLog) << "DeviceWidget constructor end";

    // 连接槽函数
    connect(mp_ListView, &PageListView::itemClicked, this, &DeviceWidget::slotListViewWidgetItemClicked);
    connect(mp_PageInfo, &PageInfoWidget::refreshInfo, this, &DeviceWidget::refreshInfo);
    connect(mp_PageInfo, &PageInfoWidget::exportInfo, this, &DeviceWidget::exportInfo);
    connect(mp_PageInfo, &PageInfoWidget::updateUI, this, &DeviceWidget::refreshInfo);

    connect(mp_ListView, &PageListView::refreshActionTrigger, this, &DeviceWidget::refreshInfo);
    connect(mp_ListView, &PageListView::exportActionTrigger, this, &DeviceWidget::exportInfo);
}

DeviceWidget::~DeviceWidget()
{
    qCDebug(appLog) << "DeviceWidget destructor start";
    if (mp_ListView) {
        qCDebug(appLog) << "DeviceWidget destructor delete list view";
        delete mp_ListView;
        mp_ListView = nullptr;
    }
    if (mp_PageInfo) {
        qCDebug(appLog) << "DeviceWidget destructor delete page info";
        delete mp_PageInfo;
        mp_PageInfo = nullptr;
    }
    if (m_Layout) {
        qCDebug(appLog) << "DeviceWidget destructor delete layout";
        delete m_Layout;
        m_Layout = nullptr;
    }
    qCDebug(appLog) << "DeviceWidget destructor end";
}

void DeviceWidget::updateListView(const QList<QPair<QString, QString> > &lst)
{
    qCDebug(appLog) << "DeviceWidget::updateListView start";
    // 更新左边的列表
    if (mp_ListView) {
        qCDebug(appLog) << "DeviceWidget::updateListView update list items";
        mp_ListView->updateListItems(lst);
    }
    qCDebug(appLog) << "DeviceWidget::updateListView end";
}

void DeviceWidget::updateDevice(const QString &itemStr, const QList<DeviceBaseInfo *> &lst)
{
    qCDebug(appLog) << "DeviceWidget::updateDevice start, item:" << itemStr << "device count:" << lst.size();
    if (lst.size() == 0) {
        qCWarning(appLog) << "DeviceWidget::updateDevice empty device list";
        return;
    }

    // 更新右边的详细内容
    if (mp_PageInfo) {
        qCDebug(appLog) << "DeviceWidget::updateDevice update table";
        mp_PageInfo->updateTable(itemStr, lst);
    }
    qCDebug(appLog) << "DeviceWidget::updateDevice end";
}

void DeviceWidget::updateOverview(const QMap<QString, QString> &map)
{
    qCDebug(appLog) << "DeviceWidget::updateOverview start, map size:" << map.size();
    if (map.size() == 0) {
        qCWarning(appLog) << "DeviceWidget::updateOverview empty map";
        return;
    }

    // 更新概况
    if (mp_PageInfo) {
        qCDebug(appLog) << "DeviceWidget::updateOverview update table";
        mp_PageInfo->updateTable(map);
    }
    qCDebug(appLog) << "DeviceWidget::updateOverview end";
}

QString DeviceWidget::currentIndex() const
{
    // 当前设备类型
    return mp_ListView->currentType();
}

void DeviceWidget::setFontChangeFlag()
{
    // 设置字体变化标志
    mp_PageInfo->setFontChangeFlag();
}

void DeviceWidget::clear()
{
    qCDebug(appLog) << "DeviceWidget::clear start";
    mp_ListView->clear();
    mp_PageInfo->clear();
    qCDebug(appLog) << "DeviceWidget::clear end";
}

void DeviceWidget::slotListViewWidgetItemClicked(const QString &itemStr)
{
    qCDebug(appLog) << "DeviceWidget::slotListViewWidgetItemClicked item:" << itemStr;
    // ListView Item 点击
    m_CurItemStr = itemStr;
    emit itemClicked(itemStr);
}

void DeviceWidget::slotUpdateUI()
{
    qCDebug(appLog) << "DeviceWidget::slotUpdateUI start";
    // 更新当前UI界面
    emit itemClicked(m_CurItemStr);
}

void DeviceWidget::resizeEvent(QResizeEvent *event)
{
    // qCDebug(appLog) << "DeviceWidget::resizeEvent start";
    DWidget::resizeEvent(event);
    // 获取所有设备类别
    const QList<QPair<QString, QString>> &types = DeviceManager::instance()->getDeviceTypes();

    // 根据右侧Listview当前Index获取当前设备类别的
    QString userStr = mp_ListView->currentIndex();
    // qCDebug(appLog) << "DeviceWidget::resizeEvent current index:" << userStr;

    QString deviceType = "";
    foreach (auto iter, types) {
        // qCDebug(appLog) << "DeviceWidget::resizeEvent check type:" << iter.second;
        if (iter.second.contains(userStr)) {
            deviceType = iter.first;
            // qCDebug(appLog) << "DeviceWidget::resizeEvent found device type:" << deviceType;
            break;
        }
    }

    // 根据设备类别获取设备指针
    QList<DeviceBaseInfo *> lst;
    bool ret = DeviceManager::instance()->getDeviceList(deviceType, lst);
    if (! ret) {
        qCWarning(appLog) << "DeviceWidget::resizeEvent failed to get device list for type:" << deviceType;
        // 更新Overview界面
        QMap<QString, QString> overviewMap = DeviceManager::instance()->getDeviceOverview();
        mp_PageInfo->updateTable(overviewMap);
        qCDebug(appLog) << "DeviceWidget::resizeEvent update overview table";
    }

    // 更新设备信息界面
    if (lst.size() == 1) {
        qCDebug(appLog) << "DeviceWidget::resizeEvent update single device table";
        mp_PageInfo->updateTable(deviceType, lst);
    } else if (lst.size() > 1) {
        qCDebug(appLog) << "DeviceWidget::resizeEvent update multi device table";
        // 判断是否是BIOS界面
        DeviceBios *bios = dynamic_cast<DeviceBios *>(lst[0]);
        if (bios) {
            qCDebug(appLog) << "DeviceWidget::resizeEvent is bios device, update table";
            mp_PageInfo->updateTable(deviceType, lst);
        }
    }
    qCDebug(appLog) << "DeviceWidget::resizeEvent end";
}

void DeviceWidget::initWidgets()
{
    qCDebug(appLog) << "DeviceWidget::initWidgets start";
    // 初始化页面布局
    m_Layout = new QHBoxLayout();
    m_Layout->setContentsMargins(0, 0, 0, 0);
    m_Layout->setSpacing(0);
    m_Layout->addWidget(mp_ListView);
    m_Layout->addWidget(mp_PageInfo);
    setLayout(m_Layout);
}
