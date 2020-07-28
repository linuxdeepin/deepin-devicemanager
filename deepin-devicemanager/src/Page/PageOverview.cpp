#include "PageOverview.h"
#include "DetailTreeView.h"
#include "DeviceManager.h"

#include <QVBoxLayout>
#include <QTableWidgetItem>

#include <DFontSizeManager>

PageOverview::PageOverview(DWidget *parent)
    : PageInfo(parent)
    , mp_PicLabel(new DLabel(this))
    , mp_TextLabel(new DLabel(this))
    , mp_Overview(new DetailTreeView(this))
{
    initWidgets();
}

void PageOverview::updateInfo(const QList<DeviceBaseInfo *> &lst)
{

}

void PageOverview::updateInfo(const QMap<QString, QString> &map)
{
    int row = map.size();
    mp_Overview->setColumnAndRow(row);

    int i = 0;

    const QList<QPair<QString, QString>> types = DeviceManager::instance()->getDeviceTypes();

    foreach (auto iter, types) {
        QStringList strList = iter.second.split("##");

        if (strList.size() != 2) {
            return;
        }

        if (map.find(strList[1]) != map.end()) {
            QTableWidgetItem *itemFirst = new QTableWidgetItem(map.find(strList[1]).key());
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

    QPixmap pic(96, 96);
    pic.load(QString::fromLocal8Bit("/home/jxm/Desktop/device/台式.svg"));
    mp_PicLabel->setPixmap(pic);
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
