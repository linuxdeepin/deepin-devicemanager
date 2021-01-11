// 项目自身文件
#include "PageSingleInfo.h"

// Qt库文件
#include <QVBoxLayout>
#include <QClipboard>

// Dtk头文件
#include <DApplication>
#include <DStandardItem>
#include <DTableWidget>
#include <DMenu>
#include <DMessageManager>
#include <DNotifySender>
#include <DMessageBox>

// 其它头文件
#include "MacroDefinition.h"
#include "DeviceInfo.h"
#include "PageTableWidget.h"

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
    , m_SameDevice(false)
{
    // 初始化页面布局
    initWidgets();

    // 设置右键快捷菜单属性
    mp_Content->setContextMenuPolicy(Qt::CustomContextMenu);

    // 连接槽函数
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
    // 判断是否是同一界面刷新
    if (mp_Label->text() == itemstr) {
        m_SameDevice = true;
    } else {
        m_SameDevice = false;
    }

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

    // 清空内容
    clearContent();

    //获取设备信息
    QList<QPair<QString, QString>> baseInfoMap = lst[0]->getBaseAttribs();
    QList<QPair<QString, QString>> otherInfoMap = lst[0]->getOtherAttribs();
    baseInfoMap = baseInfoMap + otherInfoMap;

    // 加载设备信息
    loadDeviceInfo(baseInfoMap);

    // 设置设备状态
    if (mp_Content) {
        mp_Content->setDeviceEnable(mp_Device->enable());
    }
}

void PageSingleInfo::loadDeviceInfo(const QList<QPair<QString, QString>> &lst)
{
    if (lst.size() < 1) {
        return;
    }

    // 设置单个设备界面信息显示的行数,与表格高度相关
    int maxRow = this->height() / ROW_HEIGHT - 3;
    mp_Content->setLimitRow(std::min(13, maxRow));

    // 设置表格行数
    int row = lst.size();
    mp_Content->setColumnAndRow(row + 1, 2);

    // 设置单元格内容
    for (int i = 0; i < row; ++i) {
        // 第一列
        QTableWidgetItem *itemFirst = new QTableWidgetItem(lst[i].first);
        mp_Content->setItem(i, 0, itemFirst);

        // 第二列
        QTableWidgetItem *itemSecond = new QTableWidgetItem(lst[i].second);
        mp_Content->setItem(i, 1, itemSecond);
    }
}

void PageSingleInfo::clearContent()
{
    // 清空表格内容
    mp_Content->clear();
}

void PageSingleInfo::slotShowMenu(const QPoint &)
{
    // 显示右键菜单
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
    // 刷新
    emit refreshInfo();
}
void PageSingleInfo::slotActionExport()
{
    // 导出
    emit exportInfo();
}

void PageSingleInfo::slotActionCopy()
{
    // 拷贝
    QClipboard *clipboard = DApplication::clipboard();
    clipboard->setText(mp_Content->toString());
}

void PageSingleInfo::slotActionEnable()
{
    if (mp_Content->isCurDeviceEnable()) {
        // 当前设备是不可用状态
        EnableDeviceStatus res = mp_Device->setEnable(false);

        // 除设置成功的情况，其他情况需要提示设置失败
        if (res == EDS_Success) {
            mp_Enable->setText(tr("Enable"));
            mp_Content->setDeviceEnable(false);
        } else {
            QString con = tr("Failed to disable the device");

            // 启用失败提示
            DMessageManager::instance()->sendMessage(this->window(), QIcon::fromTheme("warning"), con);
        }
    } else {
        // 当前设备是可用状态
        EnableDeviceStatus res = mp_Device->setEnable(true);

        // 除设置成功的情况，其他情况需要提示设置失败
        if (res == EDS_Success) {
            mp_Enable->setText(tr("Disable"));
            mp_Content->setDeviceEnable(true);
        } else {
            QString con = tr("Failed to enable the device");

            // 禁用失败提示
            DMessageManager::instance()->sendMessage(this->window(), QIcon::fromTheme("warning"), con);
        }
    }
}

void PageSingleInfo::initWidgets()
{
    // 初始化页面布局
    QVBoxLayout *hLayout = new QVBoxLayout();
    QHBoxLayout *labelLayout = new QHBoxLayout();
    labelLayout->addSpacing(10);
    labelLayout->addWidget(mp_Label);

    // Label距离上下控件的距离 LABEL_MARGIN
    hLayout->addSpacing(LABEL_MARGIN);
    hLayout->addLayout(labelLayout);
    hLayout->addSpacing(LABEL_MARGIN);

    hLayout->addWidget(mp_Content);
    hLayout->addStretch(1);          // 考虑禁用后表格只有一行，靠上显示
    setLayout(hLayout);
}

void PageSingleInfo::expandTable()
{
    // 展开表格
    if (mp_Content) {
        mp_Content->expandTable();
    }
}
