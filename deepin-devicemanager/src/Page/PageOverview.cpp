#include "PageOverview.h"
#include "DetailTreeView.h"
#include "DeviceManager.h"

#include <QVBoxLayout>
#include <QTableWidgetItem>
#include <QAction>
#include <QDebug>

#include <DFontSizeManager>
#include <DMenu>

PageOverview::PageOverview(DWidget *parent)
    : PageInfo(parent)
    , mp_PicLabel(new DLabel(this))
    , mp_TextLabel(new DLabel(this))
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
    mp_Overview->setColumnAndRow(row);

    QTableWidgetItem *itemFirst = new QTableWidgetItem(tr("OS"));
    mp_Overview->setItem(0, 0, itemFirst);
    QTableWidgetItem *itemSecond = new QTableWidgetItem(map.find("OS").value());
    mp_Overview->setItem(0, 1, itemSecond);

    int i = 1;

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
}

void PageOverview::setLabel(const QString &itemstr)
{
    mp_TextLabel->setText(itemstr);
    DFontSizeManager::instance()->bind(mp_TextLabel, DFontSizeManager::T3);

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
    hLayout->addWidget(mp_PicLabel);
    hLayout->addWidget(mp_TextLabel);
    hLayout->addWidget(mp_Overview);
    hLayout->addStretch();
    setLayout(hLayout);
}
