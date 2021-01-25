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
#include <DMessageManager>

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
    // 初始化界面布局
    initWidgets();

    // 连接槽函数
    connect(mp_Table, &PageTableHeader::itemClicked, this, &PageMultiInfo::slotItemClicked);
    connect(mp_Table, &PageTableHeader::refreshInfo, this, &PageMultiInfo::slotRefreshInfo);
    connect(mp_Table, &PageTableHeader::exportInfo, this, &PageMultiInfo::slotExportInfo);
    connect(mp_Detail, &PageDetail::refreshInfo, this, &PageMultiInfo::slotRefreshInfo);
    connect(mp_Detail, &PageDetail::exportInfo, this, &PageMultiInfo::slotExportInfo);
    connect(mp_Table, &PageTableHeader::enableDevice, this, &PageMultiInfo::slotEnableDevice);
}

PageMultiInfo::~PageMultiInfo()
{
    // 清空指针
    DELETE_PTR(mp_Table)
    DELETE_PTR(mp_Detail)
}

void PageMultiInfo::updateInfo(const QList<DeviceBaseInfo *> &lst)
{
    //  获取多个设备界面表格信息
    QList<QStringList> deviceList;
    deviceList.append(lst[0]->getTableHeader());
    foreach (DeviceBaseInfo *info, lst) {
        if (info->getTableData().size() > 0)
            deviceList.append(info->getTableData());
    }

    // 更新表格
    mp_Table->updateTable(deviceList);

    // 更新详细信息
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
    // 显示表格中选择设备的详细信息
    if (mp_Detail) {
        mp_Detail->showInfoOfNum(row);
    }
}

void PageMultiInfo::slotRefreshInfo()
{
    // 刷新
    emit refreshInfo();
}
void PageMultiInfo::slotExportInfo()
{
    // 导出
    emit exportInfo();
}

void PageMultiInfo::slotEnableDevice(int row, bool enable)
{
    if (!mp_Detail) {
        return;
    }

    // 禁用/启用设备
    EnableDeviceStatus res = mp_Detail->enableDevice(row, enable);

    // 除设置成功的情况，其他情况需要提示设置失败
    if (res == EDS_Success) {
        // 设置成功,更新界面
        emit updateUI();
    } else {
        // 设置失败
        QString con;
        if (enable) {
            // 无法启用设备
            con = tr("Failed to enable the device");
        } else {
            // 无法禁用设备
            con = tr("Failed to disable the device");
        }

        // 禁用、启用失败提示
        DMessageManager::instance()->sendMessage(this->window(), QIcon::fromTheme("warning"), con);

    }
}

void PageMultiInfo::initWidgets()
{
    // 初始化界面布局
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
    hLayout->addWidget(mp_Detail);
    hLayout->setContentsMargins(10, 10, 10, 0);

    setLayout(hLayout);
}
