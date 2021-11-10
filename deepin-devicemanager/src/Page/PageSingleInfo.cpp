// 项目自身文件
#include "PageSingleInfo.h"
#include "MacroDefinition.h"
#include "DeviceInfo.h"
#include "PageTableWidget.h"
#include "PageDriverControl.h"

// Dtk头文件
#include <DApplication>
#include <DStandardItem>
#include <DTableWidget>
#include <DMenu>
#include <DMessageManager>
#include <DNotifySender>
#include <DMessageBox>

// Qt库文件
#include <QVBoxLayout>
#include <QClipboard>
#include <QProcess>


PageSingleInfo::PageSingleInfo(QWidget *parent)
    : PageInfo(parent)
    , mp_Content(new PageTableWidget(this))
    , mp_Label(new DLabel(this))
    , mp_Refresh(new QAction(/*QIcon::fromTheme("view-refresh"), */tr("Refresh"), this))
    , mp_Export(new QAction(/*QIcon::fromTheme("document-new"), */tr("Export"), this))
    , mp_Copy(new QAction(/*QIcon::fromTheme("edit-copy"), */tr("Copy"), this))
    , mp_Enable(new QAction(/*QIcon::fromTheme("edit-copy"), */tr("Enable"), this))
    , mp_updateDriver(new QAction(tr("Update drivers"), this))
    , mp_removeDriver(new QAction(tr("Uninstall drivers"), this))
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
    connect(mp_Refresh, &QAction::triggered, this, &PageSingleInfo::refreshInfo);
    connect(mp_Export, &QAction::triggered, this, &PageSingleInfo::exportInfo);
    connect(mp_Copy, &QAction::triggered, this, &PageSingleInfo::slotActionCopy);
    connect(mp_Enable, &QAction::triggered, this, &PageSingleInfo::slotActionEnable);
    connect(mp_updateDriver, &QAction::triggered, this, &PageSingleInfo::slotActionUpdateDriver);
    connect(mp_removeDriver, &QAction::triggered, this, &PageSingleInfo::slotActionRemoveDriver);
}

PageSingleInfo::~PageSingleInfo()
{
    DELETE_PTR(mp_Content);
}

void PageSingleInfo::setLabel(const QString &itemstr)
{
    // 判断是否是同一界面刷新
    if (mp_Label->text() == itemstr)
        m_SameDevice = true;
    else
        m_SameDevice = false;

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
    if (lst.size() < 1)
        return;
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
        mp_Content->setDeviceEnable(mp_Device->enable(), mp_Device->available());
    }
}

void PageSingleInfo::clearWidgets()
{
    return;
}

void PageSingleInfo::loadDeviceInfo(const QList<QPair<QString, QString>> &lst)
{
    if (lst.size() < 1)
        return;

    // 设置单个设备界面信息显示的行数,与表格高度相关
    int maxRow = this->height() / ROW_HEIGHT - 3;
    if (maxRow < 0)
        maxRow = 13;
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

bool PageSingleInfo::isExpanded()
{
    if (mp_Content)
        return mp_Content->isExpanded();
    return false;
}

void PageSingleInfo::slotShowMenu(const QPoint &)
{
    mp_Menu->clear();
    // 不管什么状态 导出、刷新、复制 都有
    mp_Refresh->setEnabled(true);
    mp_Export->setEnabled(true);
    mp_Copy->setEnabled(true);
    mp_Enable->setEnabled(true);
    mp_updateDriver->setEnabled(true);
    mp_removeDriver->setEnabled(true);

    // 不可用状态：卸载和启用禁用置灰
    if(!mp_Device->available()){
        mp_removeDriver->setEnabled(false);
        mp_Enable->setEnabled(false);
    }
    // 禁用状态：更新卸载置灰
    if(!mp_Device->enable()){
        mp_Enable->setEnabled(true);
        mp_updateDriver->setEnabled(false);
        mp_removeDriver->setEnabled(false);
        mp_Enable->setText(tr("Enable"));
    }else{
        mp_Enable->setText(tr("Disable"));
    }
    // 驱动界面打开状态： 驱动的更新卸载和设备的启用禁用置灰
    if(m_driverPagedOpened){
        mp_updateDriver->setEnabled(false);
        mp_removeDriver->setEnabled(false);
        mp_Enable->setEnabled(false);
    }

    // 添加按钮到菜单
    mp_Menu->addAction(mp_Copy);
    mp_Menu->addAction(mp_Refresh);
    mp_Menu->addAction(mp_Export);
    if (mp_Device->canEnable()){
        mp_Menu->addAction(mp_Enable);
    }
    // 主板、内存、cpu等没有驱动，无需右键按钮
    if(mp_Device->canUninstall()){
        mp_Menu->addSeparator();
        mp_Menu->addAction(mp_updateDriver);
        mp_Menu->addAction(mp_removeDriver);
    }
    mp_Menu->exec(QCursor::pos());
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
            // 返回成功之前再次更新数据
            emit refreshInfo();
        }  else if(res == EDS_NoSerial){
            QString con = tr("Failed to disable it: unable to get the device SN");
            // 禁用失败提示
            DMessageManager::instance()->sendMessage(this->window(), QIcon::fromTheme("warning"), con);
        }else {
            QString con = tr("Failed to disable the device");

            // 启用失败提示
            DMessageManager::instance()->sendMessage(this->window(), QIcon::fromTheme("warning"), con);
        }
    } else {
        // 当前设备是可用状态
        EnableDeviceStatus res = mp_Device->setEnable(true);

        // 除设置成功的情况，其他情况需要提示设置失败
        if (res == EDS_Success) {
            emit refreshInfo();
        } else if(res == EDS_NoSerial){
            QString con = tr("Failed to disable it: unable to get the device SN");
            // 禁用失败提示
            DMessageManager::instance()->sendMessage(this->window(), QIcon::fromTheme("warning"), con);
        }else {
            QString con = tr("Failed to enable the device");
            // 禁用失败提示
            DMessageManager::instance()->sendMessage(this->window(), QIcon::fromTheme("warning"), con);
        }
    }
}

void PageSingleInfo::slotActionUpdateDriver()
{
    //打印设备更新驱动时，通过dde-printer来操作
    if(mp_Device->hardwareClass() == "printer") {
        if(!QProcess::startDetached("dde-printer"))
            qInfo() << "dde-printer startDetached error";
        return;
    }
    PageDriverControl* installDriver = new PageDriverControl(tr("Update Drivers"), mp_Device->name(), "", true, this);
    installDriver->show();
    m_driverPagedOpened = true;
    connect(installDriver, &PageDriverControl::finished, this, [=]{m_driverPagedOpened = false;});
}

void PageSingleInfo::slotActionRemoveDriver()
{
    //打印设备卸载驱动时，通过dde-printer来操作
    if(mp_Device->hardwareClass() == "printer") {
        if(!QProcess::startDetached("dde-printer"))
            qInfo() << "dde-printer startDetached error";
        return;
    }

    PageDriverControl* rmDriver = new PageDriverControl(tr("Uninstall Drivers"), mp_Device->name(), mp_Device->driver(), false, this);
    rmDriver->show();
    m_driverPagedOpened = true;
    connect(rmDriver, &PageDriverControl::finished, this, [=]{m_driverPagedOpened = false;});
    connect(rmDriver, &PageDriverControl::refreshInfo, this, &PageSingleInfo::refreshInfo);
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
    hLayout->addStretch(1); // 考虑禁用后表格只有一行，靠上显示
    setLayout(hLayout);
}

void PageSingleInfo::expandTable()
{
    // 展开表格
    if (mp_Content)
        mp_Content->expandTable();
}
