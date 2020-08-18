#include "PageSingleInfo.h"
#include "MacroDefinition.h"
#include "DeviceInfo.h"
#include "PageTableWidget.h"

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
    , mp_Content(new PageTableWidget(this))
    , mp_Label(new DLabel(this))
    , mp_Refresh(new QAction(QIcon::fromTheme("view-refresh"), tr("Refresh (F5)"), this))
    , mp_Export(new QAction(QIcon::fromTheme("document-new"), tr("Export (E)"), this))
    , mp_Copy(new QAction(QIcon::fromTheme("edit-copy"), tr("Copy (C)"), this))
    , mp_Enable(new QAction(QIcon::fromTheme("edit-copy"), tr("Enabled"), this))
    , mp_Menu(new DMenu(this))
    , mp_Device(nullptr)
{
    initWidgets();

    mp_Content->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mp_Content, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &PageSingleInfo::slotActionRefresh);
    connect(mp_Export, &QAction::triggered, this, &PageSingleInfo::slotActionExport);
    connect(mp_Copy, &QAction::triggered, this, &PageSingleInfo::slotActionCopy);
    connect(mp_Enable, &QAction::triggered, this, &PageSingleInfo::slotActionEnable);
    connect(this, &PageSingleInfo::enableDevice, mp_Content, &PageTableWidget::enableDevice);
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
    font.setWeight(63);
    mp_Label->setFont(font);

    // 设置字号
    DFontSizeManager::instance()->bind(mp_Label, DFontSizeManager::T5);

}

void PageSingleInfo::updateInfo(const QList<DeviceBaseInfo *> &lst)
{
    mp_Device = lst[0];

    clearContent();

    QList<QPair<QString, QString>> baseInfoMap = lst[0]->getBaseAttribs();
    QList<QPair<QString, QString>> otherInfoMap = lst[0]->getOtherAttribs();
    baseInfoMap = baseInfoMap + otherInfoMap;

    loadDeviceInfo(baseInfoMap);

    if (!lst[0]->enable()) {
        emit enableDevice();

        mp_Enable->setText(tr("Enable"));
        mp_Copy->setEnabled(false);
        mp_Export->setEnabled(false);
        mp_Refresh->setEnabled(false);


        mp_Device->setEnable(false);
    }
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

    if (mp_Content->isCurDeviceEnable()) {
        mp_Enable->setText(tr("Disable"));
    } else {
        mp_Enable->setText(tr("Enable"));
    }

    mp_Menu->addAction(mp_Enable);
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
}

void PageSingleInfo::slotActionEnable()
{
    if (mp_Content->isCurDeviceEnable()) {

        // 当前设备是可用状态
        mp_Enable->setText(tr("Enable"));
        mp_Copy->setEnabled(false);
        mp_Export->setEnabled(false);
        mp_Refresh->setEnabled(false);


        mp_Device->setEnable(false);
    } else {
        mp_Enable->setText(tr("Disable"));
        mp_Copy->setEnabled(true);
        mp_Export->setEnabled(true);
        mp_Refresh->setEnabled(true);

        mp_Device->setEnable(true);
    }

    emit enableDevice();
}

void PageSingleInfo::initWidgets()
{
    QVBoxLayout *hLayout = new QVBoxLayout(this);
    hLayout->addWidget(mp_Label);
    hLayout->addWidget(mp_Content);
    hLayout->addStretch();
    setLayout(hLayout);
}
