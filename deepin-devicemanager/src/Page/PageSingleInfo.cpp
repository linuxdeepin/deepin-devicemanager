#include "PageSingleInfo.h"
#include "MacroDefinition.h"
#include "DeviceInfo.h"

#include <QVBoxLayout>
#include <QClipboard>

#include <DApplication>
#include <DStandardItem>
#include <DTableWidget>
#include <DMenu>
#include <DMessageManager>
#include <DNotifySender>

PageSingleInfo::PageSingleInfo(QWidget *parent)
    : PageInfo(parent)
    , mp_Content(new DetailTreeView(this))
    , mp_Label(new DLabel(this))
    , mp_Refresh(new QAction(QIcon::fromTheme("view-refresh"), tr("Refresh (F5)"), this))
    , mp_Export(new QAction(QIcon::fromTheme("document-new"), tr("Export (E)"), this))
    , mp_Copy(new QAction(QIcon::fromTheme("edit-copy"), tr("Copy (C)"), this))
    , mp_Menu(new DMenu(this))
{
    initWidgets();

    mp_Content->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mp_Content, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &PageSingleInfo::slotActionRefresh);
    connect(mp_Export, &QAction::triggered, this, &PageSingleInfo::slotActionExport);
    connect(mp_Copy, &QAction::triggered, this, &PageSingleInfo::slotActionCopy);
}

PageSingleInfo::~PageSingleInfo()
{
    DELETE_PTR(mp_Content);
}

void PageSingleInfo::setLabel(const QString &itemstr)
{
    mp_Label->setText(itemstr);

    // 设备类型加粗
    QFont font = mp_Label->font();
    font.setBold(true);
    mp_Label->setFont(font);

    // 设置字号
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

void PageSingleInfo::slotShowMenu(const QPoint &)
{
    mp_Menu->clear();
    mp_Menu->addAction(mp_Copy);
    mp_Menu->addSeparator();
    mp_Menu->addAction(mp_Refresh);
    mp_Menu->addAction(mp_Export);
    mp_Menu->exec(QCursor::pos());
}
void PageSingleInfo::slotActionRefresh()
{
    emit refreshInfo();
}
void PageSingleInfo::slotActionExport()
{
    emit exportInfo();
}

void PageSingleInfo::slotActionCopy()
{
    QClipboard *clipboard = DApplication::clipboard();
    clipboard->setText(mp_Content->toString());
//    DMessageManager::instance()->sendMessage(mp_Content, QIcon::fromTheme("emblem-checked"), tr("Successfully copied device information"));
//    DUtil::DNotifySender sender(tr("Successfully copied device information"));
//    sender.appIcon("deepin-devicemanager");
//    sender.timeOut(2000);
//    sender.call();
}

void PageSingleInfo::initWidgets()
{
    QVBoxLayout *hLayout = new QVBoxLayout(this);
    hLayout->addWidget(mp_Label);
    hLayout->addWidget(mp_Content);
    hLayout->addStretch();
    setLayout(hLayout);
}
