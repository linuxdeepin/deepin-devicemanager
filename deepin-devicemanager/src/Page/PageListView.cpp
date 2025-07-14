// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PageListView.h"
#include "DeviceListView.h"
#include "MacroDefinition.h"
#include "DDLog.h"

// Dtk头文件
#include <DGuiApplicationHelper>
#include <DPaletteHelper>

// Qt库文件
#include <QHBoxLayout>
#include <QLoggingCategory>

using namespace DDLog;

PageListView::PageListView(DWidget *parent)
    : DWidget(parent)
    , mp_ListView(new DeviceListView(this))
    , mp_Refresh(new QAction(tr("Refresh"), this))
    , mp_Export(new QAction(tr("Export"), this))
    , mp_Menu(new QMenu(this))
    , m_CurType(tr("Overview"))
{
    qCDebug(appLog) << "PageListView constructor start";
    //初始化界面
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(mp_ListView);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    this->setFixedWidth(152);
    // 初始化右键菜单
    mp_ListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mp_ListView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &PageListView::refreshActionTrigger);
    connect(mp_Export, &QAction::triggered, this, &PageListView::exportActionTrigger);

    // 连接item点击事件
    connect(mp_ListView, &DListView::clicked, this, &PageListView::slotListViewItemClicked);
}

PageListView::~PageListView()
{
    qCDebug(appLog) << "PageListView destructor";
}

void PageListView::updateListItems(const QList<QPair<QString, QString> > &lst)
{
    qCDebug(appLog) << "Updating list items, count:" << lst.size();
    if (! mp_ListView) {
        qCWarning(appLog) << "ListView is null";
        return;
    }

    // 更新之前先清理
    mp_ListView->clearItem();

    // 更新 list
    foreach (auto it, lst) {
        qCDebug(appLog) << "Adding item:" << it.first;
        mp_ListView->addItem(it.first, it.second);
    }

    // 更新之后恢复之前显示的设备
    mp_ListView->setCurItem(m_CurType);
    qCDebug(appLog) << "Setting current item:" << m_CurType;
    emit itemClicked(m_CurType);
}

QString PageListView::currentIndex()
{
    QString index = mp_ListView->currentIndex().data(Qt::UserRole).toString();
    // qCDebug(appLog) << "PageListView::currentIndex, index:" << index;
    // 获取当前Index的UserRole
    return index;
}

QString PageListView::currentType()
{
    // qCDebug(appLog) << "PageListView::currentType, type:" << m_CurType;
    // 获取当前设备类型
    return m_CurType;
}

void PageListView::clear()
{
    qCDebug(appLog) << "PageListView::clear";
    if (!mp_ListView) {
        qCWarning(appLog) << "ListView is null";
        return;
    }

    // 更新之前先清理
    mp_ListView->clearItem();
}

void PageListView::setCurType(QString type)
{
    qCDebug(appLog) << "PageListView::setCurType, type:" << type;
    m_CurType = type;
    mp_ListView->setCurItem(m_CurType);
}

void PageListView::paintEvent(QPaintEvent *event)
{
    // qCDebug(appLog) << "PageListView::paintEvent"; //This log is too frequent
    // 让背景色适合主题颜色
    DPalette pa = DPaletteHelper::instance()->palette(this);
    pa.setBrush(DPalette::ItemBackground, pa.brush(DPalette::Base));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    pa.setBrush(QPalette::Background, pa.brush(QPalette::Base));
#else
    pa.setColor(QPalette::Window, pa.color(QPalette::Base));
#endif
    DPaletteHelper::instance()->setPalette(this, pa);

    return DWidget::paintEvent(event);
}

void PageListView::slotShowMenu(const QPoint &point)
{
    qCDebug(appLog) << "PageListView::slotShowMenu";
    // 右键菜单
    mp_Menu->clear();

    if (m_CurType == tr("Driver Install") || m_CurType == tr("Driver Backup") || m_CurType == tr("Driver Restore")) {
        qCDebug(appLog) << "Driver related item, no context menu";
        return;
    }

    // 导出/刷新
    if (mp_ListView->indexAt(point).isValid()) {
        qCDebug(appLog) << "Show context menu";
        mp_Menu->addAction(mp_Export);
        mp_Menu->addAction(mp_Refresh);

        mp_Menu->exec(QCursor::pos());
    }
}

void PageListView::slotListViewItemClicked(const QModelIndex &index)
{
    // qCDebug(appLog) << "PageListView::slotListViewItemClicked";
    // Item 点击事件
    QString concateStr = mp_ListView->getConcatenateStrings(index);
    qCDebug(appLog) << "List item clicked:" << concateStr;
    if (!concateStr.isEmpty() && concateStr != QString("Separator")) {
        qCDebug(appLog) << "Emit itemClicked signal";
        emit itemClicked(concateStr);
        m_CurType = concateStr;
    }
}
