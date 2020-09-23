// 项目自身文件
#include "PageMultiInfo.h"

// Qt库文件
#include <QVBoxLayout>
#include <QAction>
#include <QIcon>
#include <QDebug>

// Dtk头文件
#include <DFontSizeManager>
#include <DMessageBox>
#include <DMenu>

// 其它头文件
#include "PageTableHeader.h"
#include "PageDetail.h"
#include "MacroDefinition.h"
#include "DeviceManager/DeviceInfo.h"

DWIDGET_USE_NAMESPACE

PageMultiInfo::PageMultiInfo(QWidget *parent)
    : PageInfo(parent)
    , mp_Label(new DLabel(this))
    , mp_Table(new PageTableHeader(this))
    , mp_Detail(new PageDetail(this))
{
    initWidgets();
    connect(mp_Table, &PageTableHeader::itemClicked, this, &PageMultiInfo::slotItemClicked);
    connect(mp_Table, &PageTableHeader::refreshInfo, this, &PageMultiInfo::slotRefreshInfo);
    connect(mp_Table, &PageTableHeader::exportInfo, this, &PageMultiInfo::slotExportInfo);
    connect(mp_Detail, &PageDetail::refreshInfo, this, &PageMultiInfo::slotRefreshInfo);
    connect(mp_Detail, &PageDetail::exportInfo, this, &PageMultiInfo::slotExportInfo);
    connect(mp_Table, &PageTableHeader::enableDevice, this, &PageMultiInfo::slotEnableDevice);
}

PageMultiInfo::~PageMultiInfo()
{
    DELETE_PTR(mp_Table);
    DELETE_PTR(mp_Detail);
}

void PageMultiInfo::updateInfo(const QList<DeviceBaseInfo *> &lst)
{
    QList<QStringList> deviceList;
    deviceList.append(lst[0]->getTableHeader());
    foreach (DeviceBaseInfo *info, lst) {
        if (info->getTableData().size() > 0)
            deviceList.append(info->getTableData());
    }
    mp_Table->updateTable(deviceList);
    mp_Detail->showDeviceInfo(lst);
}

void PageMultiInfo::setLabel(const QString &itemstr)
{
    if (mp_Label) {
        mp_Label->setText(itemstr);

        // 设备类型加粗
        QFont font = mp_Label->font();
        font.setWeight(63);
        mp_Label->setFont(font);

        DFontSizeManager::instance()->bind(mp_Label, DFontSizeManager::T5);
    }
}

void PageMultiInfo::slotItemClicked(int row)
{
    qDebug() << row;
    if (mp_Detail) {
        mp_Detail->showInfoOfNum(row);
    }
}

void PageMultiInfo::slotRefreshInfo()
{
    emit refreshInfo();
}
void PageMultiInfo::slotExportInfo()
{
    emit exportInfo();
}

void PageMultiInfo::slotEnableDevice(int row, bool enable)
{
    if (!mp_Detail) {
        return;
    }

    EnableDeviceStatus res = mp_Detail->enableDevice(row, enable);
    if (res == EDS_Cancle) {
        return;
    } else if (res == EDS_Success) {
        emit updateUI();
    } else {
        QString con;
        if (enable) {
            con = tr("Failed to enable the device");
        } else {
            con = tr("Failed to disable the device");
        }
        DMessageBox::information(this, tr(""), con, DMessageBox::StandardButton::Ok);
    }
}

void PageMultiInfo::initWidgets()
{
    QVBoxLayout *hLayout = new QVBoxLayout();
    QHBoxLayout *labelLayout = new QHBoxLayout();
    labelLayout->addSpacing(10);
    labelLayout->addWidget(mp_Label);

    // Label 距离上下控件的距离LABEL_MARGIN
    hLayout->addSpacing(LABEL_MARGIN);
    hLayout->addLayout(labelLayout);
    hLayout->addSpacing(LABEL_MARGIN);

    mp_Table->setFixedHeight(TABLE_HEIGHT);
    hLayout->addWidget(mp_Table);
//    hLayout->addSpacing(5);
    hLayout->addWidget(mp_Detail);
    hLayout->setContentsMargins(10, 10, 10, 0);
//    qDebug() << hLayout->contentsMargins();
    setLayout(hLayout);
}
