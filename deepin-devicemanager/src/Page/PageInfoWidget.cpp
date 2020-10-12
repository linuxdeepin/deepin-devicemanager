// 项目自身文件
#include "PageInfoWidget.h"

// Qt库文件
#include <QHBoxLayout>
#include <QDebug>
#include <QAction>

// Dtk头文件
#include <DMenu>

// 其它头文件
#include "PageMultiInfo.h"
#include "PageSingleInfo.h"
#include "PageOverview.h"
#include "PageBoardInfo.h"
#include "DeviceBios.h"


PageInfoWidget::PageInfoWidget(QWidget *parent)
    : DWidget(parent)
    , mp_PageInfo(nullptr)
    , mp_PageSignalInfo(new PageSingleInfo(this))
    , mp_PageMutilInfo(new PageMultiInfo(this))
    , mp_PageOverviewInfo(new PageOverview(this))
    , mp_PageBoardInfo(new PageBoardInfo(this))
{
    // 初始化界面布局
    initWidgets();

    // 连接槽函数
    connect(mp_PageMutilInfo, &PageMultiInfo::refreshInfo, this, &PageInfoWidget::slotRefreshInfo);
    connect(mp_PageMutilInfo, &PageMultiInfo::exportInfo, this, &PageInfoWidget::slotExportInfo);
    connect(mp_PageMutilInfo, &PageMultiInfo::updateUI, this, &PageInfoWidget::slotUpdateUI);
    connect(mp_PageSignalInfo, &PageSingleInfo::refreshInfo, this, &PageInfoWidget::slotRefreshInfo);
    connect(mp_PageSignalInfo, &PageSingleInfo::exportInfo, this, &PageInfoWidget::slotExportInfo);
    connect(mp_PageOverviewInfo, &PageOverview::refreshInfo, this, &PageInfoWidget::slotRefreshInfo);
    connect(mp_PageOverviewInfo, &PageOverview::exportInfo, this, &PageInfoWidget::slotExportInfo);
    connect(mp_PageBoardInfo, &PageBoardInfo::refreshInfo, this, &PageInfoWidget::slotRefreshInfo);
    connect(mp_PageBoardInfo, &PageBoardInfo::exportInfo, this, &PageInfoWidget::slotExportInfo);
}

void PageInfoWidget::updateTable(const QString &itemStr, const QList<DeviceBaseInfo *> &lst)
{
    // 设备个数为0,是概况界面
    if (lst.size() == 0) {
        mp_PageOverviewInfo->setVisible(true);
        mp_PageSignalInfo->setVisible(false);
        mp_PageMutilInfo->setVisible(false);
        mp_PageBoardInfo->setVisible(false);
        mp_PageInfo = mp_PageOverviewInfo;
    } else if (lst.size() == 1) {                 // 设备个数为1,是单个设备界面
        mp_PageOverviewInfo->setVisible(false);
        mp_PageSignalInfo->setVisible(true);
        mp_PageMutilInfo->setVisible(false);
        mp_PageBoardInfo->setVisible(false);
        mp_PageInfo = mp_PageSignalInfo;
    } else {
        mp_PageOverviewInfo->setVisible(false);
        mp_PageSignalInfo->setVisible(false);
        DeviceBios *bios = dynamic_cast<DeviceBios *>(lst[0]);

        // 判断是否是BIOS界面
        if (bios) {
            mp_PageInfo = mp_PageBoardInfo;
            mp_PageBoardInfo->setVisible(true);
            mp_PageMutilInfo->setVisible(false);
        } else {
            // 多设备界面
            mp_PageInfo = mp_PageMutilInfo;
            mp_PageBoardInfo->setVisible(false);
            mp_PageMutilInfo->setVisible(true);
        }
    }

    // 设置页面Label以及显示设备信息
    if (mp_PageInfo) {
        mp_PageInfo->setLabel(itemStr);
        mp_PageInfo->updateInfo(lst);
    }
}

void PageInfoWidget::updateTable(const QString &itemStr, const QMap<QString, QString> &map)
{
    // 更新概况界面
    mp_PageOverviewInfo->setVisible(true);
    mp_PageSignalInfo->setVisible(false);
    mp_PageMutilInfo->setVisible(false);
    mp_PageBoardInfo->setVisible(false);
    mp_PageInfo = mp_PageOverviewInfo;
    if (mp_PageInfo) {
        mp_PageInfo->updateInfo(map);
        mp_PageInfo->setLabel(map["Overview"], map["OS"]);
    }
}

void PageInfoWidget::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
}

void PageInfoWidget::slotRefreshInfo()
{
    // 刷新信息
    emit refreshInfo();
}
void PageInfoWidget::slotExportInfo()
{
    // 导出信息
    emit exportInfo();
}

void PageInfoWidget::slotUpdateUI()
{
    // 更新UI
    emit updateUI();
}

void PageInfoWidget::initWidgets()
{
    // 初始化页面布局
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(10, 10, 10, 10);
    hLayout->addWidget(mp_PageSignalInfo);
    hLayout->addWidget(mp_PageMutilInfo);
    hLayout->addWidget(mp_PageOverviewInfo);
    hLayout->addWidget(mp_PageBoardInfo);
    setLayout(hLayout);

    mp_PageSignalInfo->setVisible(false);
    mp_PageMutilInfo->setVisible(false);
    mp_PageBoardInfo->setVisible(false);
}
