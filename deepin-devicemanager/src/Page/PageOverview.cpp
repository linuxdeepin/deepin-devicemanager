#include "PageOverview.h"
#include "DetailTreeView.h"
#include "DeviceManager.h"

#include <QVBoxLayout>
#include <QTableWidgetItem>
#include <QAction>
#include <QDebug>

#include <DFontSizeManager>
#include <DMenu>
#include <DSysInfo>

DCORE_USE_NAMESPACE

PageOverview::PageOverview(DWidget *parent)
    : PageInfo(parent)
    , mp_PicLabel(new DLabel(this))
    , mp_DeviceLabel(new DLabel(this))
    , mp_OSLabel(new QLabel(this))
    , mp_Overview(new DetailTreeView(this))
    , mp_Refresh(new QAction(QIcon::fromTheme("view-refresh"), tr("Refresh (F5)"), this))
    , mp_Export(new QAction(QIcon::fromTheme("document-new"), tr("Export (E)"), this))
    , mp_Menu(new DMenu(this))
{
    initWidgets();

    mp_Overview->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mp_Overview, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &PageOverview::slotActionRefresh);
    connect(mp_Export, &QAction::triggered, this, &PageOverview::slotActionExport);
}

void PageOverview::updateInfo(const QList<DeviceBaseInfo *> &)
{

}

void PageOverview::updateInfo(const QMap<QString, QString> &map)
{
    int row = map.size();
    mp_Overview->setLimitRow(11);
    mp_Overview->setColumnAndRow(row - 1);

    int i = 0;

    const QList<QPair<QString, QString>> types = DeviceManager::instance()->getDeviceTypes();

    foreach (auto iter, types) {
        QStringList strList = iter.second.split("##");

        if (strList.size() != 2) {
            continue;
        }

        if (strList[1] == "Overview") {
            continue;
        }

        if (map.find(strList[1]) != map.end()) {
            QTableWidgetItem *itemFirst = new QTableWidgetItem(iter.first);
            mp_Overview->setItem(i, 0, itemFirst);
            QTableWidgetItem *itemSecond = new QTableWidgetItem(map.find(strList[1]).value());
            mp_Overview->setItem(i, 1, itemSecond);
            ++i;
        }
    }

    qDebug() << "=======================" << i;
}

void PageOverview::setLabel(const QString &itemstr)
{
    mp_DeviceLabel->setText(itemstr);
    DFontSizeManager::instance()->bind(mp_DeviceLabel, DFontSizeManager::T3);

    // 系统中获取
//    QIcon icon(QIcon::fromTheme("computer"));
//    QPixmap pic = icon.pixmap(96, 96);
//    mp_PicLabel->setPixmap(pic);

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
    QString os = str2;
    QString linkStr = "<a style = \"text-decoration:none\" href = https://www.chinauos.com/home>";
    DSysInfo::DeepinType type = DSysInfo::deepinType();
    if (DSysInfo::DeepinProfessional == type) {
        linkStr += "UnionTech OS 20 </a>" + os.remove("UnionTech OS 20");
    } else if (DSysInfo::DeepinPersonal == type) {
        linkStr += "UOS 20 Home </a>" + os.remove("UOS 20 Home");
    } else if (DSysInfo::DeepinDesktop == type) {
        linkStr += "Deepin 20 RC </a>" + os.remove("Deepin 20 RC");
    }

    // 打开超链接属性
    mp_OSLabel->setOpenExternalLinks(true);

    // 设置操作系统内容，并自动换行
    mp_OSLabel->setText(linkStr);
    mp_OSLabel->setWordWrap(true);

    // 设置设备信息
    mp_DeviceLabel->setText(str1);
    DFontSizeManager::instance()->bind(mp_DeviceLabel, DFontSizeManager::T3);

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
    mp_Menu->addSeparator();
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

void PageOverview::initWidgets()
{
    QVBoxLayout *hLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayoutTop = new QHBoxLayout(this);
    QVBoxLayout *vLayoutLabel = new  QVBoxLayout(this);

    mp_OSLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mp_DeviceLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    vLayoutLabel->addSpacing(20);
    vLayoutLabel->addWidget(mp_DeviceLabel);
    vLayoutLabel->addWidget(mp_OSLabel);
    vLayoutLabel->addSpacing(20);

    hLayoutTop->setMargin(2);
    hLayoutTop->addWidget(mp_PicLabel);
    hLayoutTop->addSpacing(10);
    hLayoutTop->addLayout(vLayoutLabel);
//    hLayoutTop->addStretch();

    hLayout->addLayout(hLayoutTop);
    hLayout->addSpacing(10);
    hLayout->addWidget(mp_Overview);
    hLayout->addStretch();
    setLayout(hLayout);
}
