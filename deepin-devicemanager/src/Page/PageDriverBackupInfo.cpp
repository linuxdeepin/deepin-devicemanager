// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QBoxLayout>

#include <DScrollArea>

#include "PageDriverBackupInfo.h"
#include "DetectedStatusWidget.h"
#include "PageDriverTableView.h"
#include "driveritem.h"
#include "DDLog.h"

using namespace DDLog;

PageDriverBackupInfo::PageDriverBackupInfo(QWidget *parent)
    : DFrame(parent)
    , mp_HeadWidget(new DetectedStatusWidget(this))
    , mp_NotBackupWidget(new DWidget(this))
    , mp_ViewBackable(new PageDriverTableView(this))
    , mp_BackableDriverLabel(new DLabel(this))
    , mp_BackedUpWidget(new DWidget(this))
    , mp_ViewBackedUp(new PageDriverTableView(this))
    , mp_BackedUpDriverLabel(new DLabel(this))
{
    qCDebug(appLog) << "PageDriverBackupInfo constructor start";
    initUI();

    connect(mp_ViewBackable, &PageDriverTableView::operatorClicked, this, &PageDriverBackupInfo::operatorClicked);
    connect(mp_ViewBackable, &PageDriverTableView::itemChecked, this, &PageDriverBackupInfo::itemChecked);
    connect(mp_HeadWidget, &DetectedStatusWidget::backupAll, this, &PageDriverBackupInfo::backupAll);
    connect(mp_HeadWidget, &DetectedStatusWidget::backupAll, this, [=](){
        qCDebug(appLog) << "PageDriverBackupInfo backupAll triggered, set checked cb disnable";
        mp_ViewBackable->setCheckedCBDisnable();
    });
    connect(mp_HeadWidget, &DetectedStatusWidget::cancelClicked, this, &PageDriverBackupInfo::undoBackup);
    connect(mp_HeadWidget, &DetectedStatusWidget::redetected, this, &PageDriverBackupInfo::redetected);
}

void PageDriverBackupInfo::initUI()
{
    qCDebug(appLog) << "PageDriverBackupInfo::initUI start";
    this->setLineWidth(0);
    initTable();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 0);
    mainLayout->setSpacing(0);

    // 上方的表头
    QHBoxLayout *headerLayout = new QHBoxLayout(this);
    headerLayout->setSpacing(0);
    headerLayout->addWidget(mp_HeadWidget);

    // 下方的可滑动区域
    DScrollArea *area = new DScrollArea(this);
    area->setMinimumHeight(10);
    area->setFrameShape(QFrame::NoFrame);
    area->setWidgetResizable(true);

    DWidget *frame = new DWidget(this);
    QVBoxLayout *frameLayout = new QVBoxLayout();
    frame->setContentsMargins(0, 0, 0, 0);
    frameLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *backableLayout = new QVBoxLayout(this);
    backableLayout->setContentsMargins(0, 0, 0, 0);
    backableLayout->addWidget(mp_BackableDriverLabel);
    backableLayout->addWidget(mp_ViewBackable);
    backableLayout->addSpacing(7);
    backableLayout->addStretch();
    mp_NotBackupWidget->setLayout(backableLayout);

    QVBoxLayout *updateLayout = new QVBoxLayout(this);
    updateLayout->setContentsMargins(0, 0, 0, 0);
    updateLayout->addWidget(mp_BackedUpDriverLabel);
    updateLayout->addWidget(mp_ViewBackedUp);
    updateLayout->addSpacing(7);
    updateLayout->addStretch();
    mp_BackedUpWidget->setLayout(updateLayout);

    frameLayout->addWidget(mp_NotBackupWidget);
    frameLayout->addWidget(mp_BackedUpWidget);
    frameLayout->addSpacing(17);
    frameLayout->addStretch();
    frame->setLayout(frameLayout);
    area->setWidget(frame);

    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(16);
    mainLayout->addWidget(area);
    this->setLayout(mainLayout);
    qCDebug(appLog) << "PageDriverBackupInfo::initUI end";
}

void PageDriverBackupInfo::initTable()
{
    qCDebug(appLog) << "PageDriverBackupInfo::initTable start";
    mp_ViewBackable->initHeaderView(QStringList() << "" << tr("Name")
                                    << tr("Current Version") << tr("Driver Platform Version")
                                    << tr("Status") << tr("Action"), true);
    mp_ViewBackable->setHeaderCbStatus(false);
    mp_ViewBackable->setColumnWidth(0, 41);
    mp_ViewBackable->setColumnWidth(1, 290);
    mp_ViewBackable->setColumnWidth(2, 120);
    mp_ViewBackable->setColumnWidth(3, 120);
//    mp_ViewBackable->setColumnWidth(4, 80);
    mp_ViewBackable->setColumnWidth(4, 120);
    mp_ViewBackable->setColumnWidth(5, 80);

    mp_ViewBackedUp->initHeaderView(QStringList() << tr("Name") << tr("Current Version") << tr("Driver Platform Version"));
    mp_ViewBackedUp->setColumnWidth(0, 418);
    mp_ViewBackedUp->setColumnWidth(1, 185);
    qCDebug(appLog) << "PageDriverBackupInfo::initTable end";
}

void PageDriverBackupInfo::addDriverInfoToTableView(DriverInfo *info, int index)
{
    qCDebug(appLog) << "Adding driver info to backup table, name:" << info->name() << "index:" << index;
    PageDriverTableView *view = nullptr;
    if (info->debBackupVersion() != info->debVersion()) {
        qCDebug(appLog) << "Driver is backupable, version:" << info->version() << "debVersion:" << info->debVersion();
        view = mp_ViewBackable;
        view->appendRowItems(6);

        int row = view->model()->rowCount() - 1;

        // 设置CheckBtn
        DriverCheckItem *cbItem = new DriverCheckItem(this);
        connect(cbItem, &DriverCheckItem::sigChecked, view, [index, view](bool checked) {
            qCDebug(appLog) << "PageDriverBackupInfo check item " << index << " checked:" << checked;
            Q_UNUSED(index)
            view->setHeaderCbStatus(checked);
        });
        cbItem->setChecked(ST_NOT_INSTALL == info->status());
        view->setWidget(row, 0, cbItem);

        // 设置设备信息
        DriverNameItem *nameItem = new DriverNameItem(this, info->type());
        nameItem->setName(info->name());
        nameItem->setIndex(index);
        view->setWidget(row, 1, nameItem);

        // 设置版本
        DriverLabelItem *versionItem = new DriverLabelItem(this, info->version());
        view->setWidget(row, 2, versionItem);

        // 设置版本
        DriverLabelItem *debVersionItem = new DriverLabelItem(this, info->debVersion());
        view->setWidget(row, 3, debVersionItem);

//        // 设置大小
//        DriverLabelItem *sizeItem = new DriverLabelItem(this, info->size());
//        view->setWidget(row, 4, sizeItem);

        // 设置状态
        DriverStatusItem *statusItem = new DriverStatusItem(this, Status::ST_DRIVER_NOT_BACKUP);
        view->setWidget(row, 4, statusItem);

        // 添加操作按钮
        DriverOperationItem *operateItem = new DriverOperationItem(this,  DriverOperationItem::BACKUP);
        view->setWidget(row, 5, operateItem);
    }

    if (!info->debBackupVersion().isEmpty()) {
        qCDebug(appLog) << "Driver is backed up, version:" << info->debBackupVersion();
        view = mp_ViewBackedUp;
        view->appendRowItems(3);

        int row = view->model()->rowCount() - 1;

        // 设置设备信息
        DriverNameItem *nameItem = new DriverNameItem(this, info->type());
        nameItem->setName(info->name());
        nameItem->setIndex(index);
        view->setWidget(row, 0, nameItem);

        // 设置版本
        DriverLabelItem *versionItem = new DriverLabelItem(this, info->version());
        view->setWidget(row, 1, versionItem);

        // 设置版本
        DriverLabelItem *debVersionItem = new DriverLabelItem(this, info->debVersion());
        view->setWidget(row, 2, debVersionItem);
    }
}

void PageDriverBackupInfo::showTables(int backableLength, int backedupLength)
{
    qCDebug(appLog) << "Showing backup tables, backable:" << backableLength << "backedup:" << backedupLength;
    // Label显示
    mp_BackableDriverLabel->setText(tr("Backupable Drivers"));
    mp_BackedUpDriverLabel->setText(tr("Backed up Drivers"));

    mp_BackedUpDriverLabel->setVisible(backedupLength != 0);
    mp_BackedUpWidget->setVisible(backedupLength != 0);

    mp_BackableDriverLabel->setVisible(backableLength != 0);
    mp_NotBackupWidget->setVisible(backableLength != 0);

    // 显示表头显示的内容
    if (backableLength == 0) {
        qCDebug(appLog) << "PageDriverBackupInfo::showTables no backable driver";
        mp_HeadWidget->setNoBackupDriverUI(backableLength, backedupLength);
    } else {
        qCDebug(appLog) << "PageDriverBackupInfo::showTables has backable driver";
        mp_HeadWidget->setBackableDriverUI(backableLength, backedupLength);
    }
}

void PageDriverBackupInfo::getCheckedDriverIndex(QList<int> &lstIndex)
{
    qCDebug(appLog) << "PageDriverBackupInfo::getCheckedDriverIndex start";
    mp_ViewBackable->getCheckedDriverIndex(lstIndex);
    qCDebug(appLog) << "PageDriverBackupInfo::getCheckedDriverIndex end, count:" << lstIndex.size();
}

void PageDriverBackupInfo::clearAllData()
{
    qCDebug(appLog) << "PageDriverBackupInfo::clearAllData start";
    mp_ViewBackable->clear();
    mp_ViewBackedUp->clear();

    initTable();
    qCDebug(appLog) << "PageDriverBackupInfo::clearAllData end";
}

void PageDriverBackupInfo::updateItemStatus(int index, Status status)
{
    qCDebug(appLog) << "Updating backup item status, index:" << index << "status:" << status;
    mp_ViewBackable->setItemStatus(index, status);
    if (status == ST_DRIVER_BACKUP_FAILED) {
        qCWarning(appLog) << "Driver backup failed for index:" << index;
        mp_ViewBackable->setErrorMsg(index, QString("Backup Failed"));
    }
}

void PageDriverBackupInfo::setCheckedCBDisnable()
{
    qCDebug(appLog) << "PageDriverBackupInfo::setCheckedCBDisnable";
    mp_ViewBackable->setCheckedCBDisnable();
}

void PageDriverBackupInfo::setHeaderCbEnable(bool enable)
{
    qCDebug(appLog) << "PageDriverBackupInfo::setHeaderCbEnable, enable:" << enable;
    mp_ViewBackable->setHeaderCbEnable(enable);
}
