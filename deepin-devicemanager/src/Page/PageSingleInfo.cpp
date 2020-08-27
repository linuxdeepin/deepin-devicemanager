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
#include <DMessageBox>

PageSingleInfo::PageSingleInfo(QWidget *parent)
    : PageInfo(parent)
    , mp_Content(new PageTableWidget(this))
    , mp_Label(new DLabel(this))
    , mp_Refresh(new QAction(/*QIcon::fromTheme("view-refresh"), */tr("Refresh"), this))
    , mp_Export(new QAction(/*QIcon::fromTheme("document-new"), */tr("Export"), this))
    , mp_Copy(new QAction(/*QIcon::fromTheme("edit-copy"), */tr("Copy"), this))
    , mp_Enable(new QAction(/*QIcon::fromTheme("edit-copy"), */tr("Enable"), this))
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
    if (lst.size() < 1) {
        return;
    }
    mp_Device = lst[0];

    clearContent();

    QList<QPair<QString, QString>> baseInfoMap = lst[0]->getBaseAttribs();
    QList<QPair<QString, QString>> otherInfoMap = lst[0]->getOtherAttribs();
    baseInfoMap = baseInfoMap + otherInfoMap;

    loadDeviceInfo(baseInfoMap);

    if (mp_Content) {
        mp_Content->setDeviceEnable(mp_Device->enable());
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
//        itemSecond->setToolTip(lst[i].second);
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
    mp_Refresh->setEnabled(true);
    mp_Export->setEnabled(true);
    mp_Copy->setEnabled(true);
    mp_Menu->addAction(mp_Copy);

    if (!mp_Device) {
        return;
    }

    if (mp_Device->canEnable()) {
        if (mp_Content->isCurDeviceEnable()) {
            mp_Enable->setText(tr("Disable"));
        } else {
            mp_Enable->setText(tr("Enable"));
            mp_Refresh->setEnabled(false);
            mp_Export->setEnabled(false);
            mp_Copy->setEnabled(false);
        }
        mp_Menu->addAction(mp_Enable);
    }
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
        EnableDeviceStatus res = mp_Device->setEnable(false);
        if (res == EDS_Cancle) {
            return;
        } else if (res == EDS_Success) {
            mp_Enable->setText(tr("Enable"));
            mp_Content->setDeviceEnable(false);
        } else {
            QString con = tr("Failed to disable the device");
            DMessageBox::information(this, tr(""), con, DMessageBox::StandardButton::Ok);
        }
    } else {
        // 当前设备是可用状态
        EnableDeviceStatus res = mp_Device->setEnable(true);
        if (res == EDS_Cancle) {
            return;
        } else if (res == EDS_Success) {
            mp_Enable->setText(tr("Disable"));
            mp_Content->setDeviceEnable(true);
        } else {
            QString con = tr("Failed to enable the device");
            DMessageBox::information(this, tr(""), con, DMessageBox::StandardButton::Ok);
        }
    }
}

void PageSingleInfo::initWidgets()
{
    QVBoxLayout *hLayout = new QVBoxLayout();
    QHBoxLayout *labelLayout = new QHBoxLayout();
    labelLayout->addSpacing(10);
    labelLayout->addWidget(mp_Label);

    hLayout->addSpacing(5);
    hLayout->addLayout(labelLayout);
    hLayout->addSpacing(5);
    hLayout->addWidget(mp_Content);
    hLayout->addStretch();
    setLayout(hLayout);
}
