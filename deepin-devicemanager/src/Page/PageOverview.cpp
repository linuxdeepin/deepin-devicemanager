#include "PageOverview.h"
#include "PageTableWidget.h"
#include "DeviceManager.h"
#include "PageInfoWidget.h"
#include "LongTextLabel.h"

#include <QVBoxLayout>
#include <QTableWidgetItem>
#include <QAction>
#include <QDebug>
#include <QClipboard>

#include <DApplication>
#include <DFontSizeManager>
#include <DMenu>
#include <DSysInfo>
#include <DMessageManager>
#include <DNotifySender>
#include <DTextBrowser>

PageOverview::PageOverview(DWidget *parent)
    : PageInfo(parent)
    , mp_PicLabel(new DLabel(this))
    , mp_DeviceLabel(new LongTextLabel(this))
    , mp_OSLabel(new LongTextLabel(this))
    , mp_Overview(new PageTableWidget(this))
    , mp_Refresh(new QAction(/*QIcon::fromTheme("view-refresh"), */tr("Refresh"), this))
    , mp_Export(new QAction(/*QIcon::fromTheme("document-new"), */tr("Export"), this))
    , mp_Copy(new QAction(/*QIcon::fromTheme("edit-copy"), */tr("Copy"), this))
    , mp_Menu(new DMenu(this))
{
    initWidgets();

    mp_Overview->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mp_Overview, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &PageOverview::slotActionRefresh);
    connect(mp_Export, &QAction::triggered, this, &PageOverview::slotActionExport);
    connect(mp_Copy, &QAction::triggered, this, &PageOverview::slotActionCopy);
}

void PageOverview::updateInfo(const QList<DeviceBaseInfo *> &)
{

}

void PageOverview::updateInfo(const QMap<QString, QString> &map)
{
    mp_Overview->clear();

    int row = map.size();
    mp_Overview->setLimitRow(11);
    mp_Overview->setColumnAndRow(row - 1);

    int i = 0;

    const QList<QPair<QString, QString>> types = DeviceManager::instance()->getDeviceTypes();

    foreach (auto iter, types) {

        if (iter.first == tr("Overview")) {
            continue;
        }

        if (map.find(iter.first) != map.end()) {
            QTableWidgetItem *itemFirst = new QTableWidgetItem(iter.first);
            mp_Overview->setItem(i, 0, itemFirst);
            QTableWidgetItem *itemSecond = new QTableWidgetItem(map.find(iter.first).value());
            mp_Overview->setItem(i, 1, itemSecond);
            ++i;
        }
    }
}

void PageOverview::setLabel(const QString &itemstr)
{
    mp_DeviceLabel->setText(itemstr);
    DFontSizeManager::instance()->bind(mp_DeviceLabel, DFontSizeManager::T5);

    // 资源文件获取、
    QPixmap pic(96, 96);

    QString path = ":/icons/deepin/builtin/device/";
    if (itemstr.contains("desktop", Qt::CaseInsensitive)) {
        path += "desktop.svg";
    }
    if (itemstr.contains("server", Qt::CaseInsensitive)) {
        path += "server.svg";
    }
    if (itemstr.contains("ternimal", Qt::CaseInsensitive)) {
        path += "ternimal.svg";
    }
    if (itemstr.contains("laptop", Qt::CaseInsensitive) ||
            itemstr.contains("notebook", Qt::CaseInsensitive)) {
        path += "laptop.svg";
    }
    if (itemstr.contains("Tablet", Qt::CaseInsensitive)) {
        path += "Tablet.svg";
    }

    pic.load(path);
    mp_PicLabel->setPixmap(pic);
}

void PageOverview::setLabel(const QString &str1, const QString &str2)
{
    // 打开超链接属性
    mp_OSLabel->setOpenExternalLinks(true);

    // 设置ToolTip
    mp_OSLabel->setToolTip(str2);



    // 超过控件长度用...代替
    QString os = str2;
    QString linkStr = "<a style=\"text-decoration:none\" href=https://www.chinauos.com/home>";

    DSysInfo::DeepinType type = DSysInfo::deepinType();
    if (DSysInfo::DeepinProfessional == type) {
        linkStr += "UnionTech OS 20 </a>" + os.remove("UnionTech OS 20");
    } else if (DSysInfo::DeepinPersonal == type) {
        linkStr += "UnionTech OS 20 Home </a>" + os.remove("UnionTech OS 20 Home");
    } else if (DSysInfo::DeepinDesktop == type) {
        linkStr += "Deepin 20 </a>" + os.remove("Deepin 20");
    } else {
        linkStr += "UOS </a>" + os.remove("UOS");
    }

    // 设置自动换行
    //mp_OSLabel->setWordWrap(false);
    qDebug() << linkStr;
    mp_OSLabel->setText(linkStr);

    // 设置设备信息
    mp_DeviceLabel->setText(str1);

    // 设备信息加粗
    QFont font = mp_DeviceLabel->font();
    font.setWeight(63);
    mp_DeviceLabel->setFont(font);

    // 设置字号
    DFontSizeManager::instance()->bind(mp_DeviceLabel, DFontSizeManager::T5);

    // 系统中获取
//    QIcon icon(QIcon::fromTheme("computer"));
//    QPixmap pic = icon.pixmap(96, 96);
//    mp_PicLabel->setPixmap(pic);

    // 资源文件获取、
    QPixmap pic(96, 96);

    QString path = ":/icons/deepin/builtin/device/";
    if (str1.contains("desktop", Qt::CaseInsensitive)) {
        path += "desktop.svg";
    }
    if (str1.contains("server", Qt::CaseInsensitive)) {
        path += "server.svg";
    }
    if (str1.contains("ternimal", Qt::CaseInsensitive)) {
        path += "ternimal.svg";
    }
    if (str1.contains("laptop", Qt::CaseInsensitive) ||
            str1.contains("notebook", Qt::CaseInsensitive)) {
        path += "laptop.svg";
    }
    if (str1.contains("Tablet", Qt::CaseInsensitive)) {
        path += "Tablet.svg";
    }

    pic.load(path);
    mp_PicLabel->setPixmap(pic);
}

void PageOverview::slotShowMenu(const QPoint &)
{
    mp_Menu->clear();
    mp_Menu->addAction(mp_Copy);
//    mp_Menu->addSeparator();
    mp_Menu->addAction(mp_Refresh);
    mp_Menu->addAction(mp_Export);
    mp_Menu->exec(QCursor::pos());
}

void PageOverview::slotActionRefresh()
{
    emit refreshInfo();
}

void PageOverview::slotActionExport()
{
    emit exportInfo();
}

void PageOverview::slotActionCopy()
{
    QClipboard *clipboard = DApplication::clipboard();
    clipboard->setText(mp_Overview->toString());
}

void PageOverview::initWidgets()
{
    mp_OSLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mp_DeviceLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *vLayout = new QVBoxLayout(this);

    // 上部分
    QHBoxLayout *uphLayout = new QHBoxLayout(this);
    // 上左
    uphLayout->addWidget(mp_PicLabel);
    // 上右
    QVBoxLayout *uplvLayout = new QVBoxLayout(this);
    uplvLayout->addWidget(mp_DeviceLabel);
    uplvLayout->addWidget(mp_OSLabel);
    uphLayout->addLayout(uplvLayout);
    vLayout->addLayout(uphLayout);

    // 下部分
    vLayout->addWidget(mp_Overview);
    setLayout(vLayout);
}
