#include "DeviceWidget.h"

#include "ListViewWidget.h"
#include "DeviceInfoPage.h"
#include "MacroDefinition.h"

#include <QHBoxLayout>

DeviceWidget::DeviceWidget(QWidget *parent)
    : DWidget(parent)
    , mp_ListView(new ListViewWidget(this))
    , mp_InfoPage(new DeviceInfoPage(this))
    , mp_Splitter(new DSplitter(Qt::Horizontal, this))
{
    // 初始化界面布局
    initWidgets();

    // 连接曹函数
    connect(mp_ListView, &ListViewWidget::itemClicked, this, &DeviceWidget::slotListViewWidgetItemClicked);
}

DeviceWidget::~DeviceWidget()
{
    DELETE_PTR(mp_ListView);
    DELETE_PTR(mp_InfoPage);
    DELETE_PTR(mp_Splitter);
}

void DeviceWidget::updateData(const QList<QPair<QString, QString> > &lst)
{
    // 更新左边的列表
    if (mp_ListView)
        mp_ListView->updateListItems(lst);

    // 更新右边的详细内容
    if (mp_InfoPage) {

    }
}

void DeviceWidget::slotListViewWidgetItemClicked(const QString &itemStr)
{
    emit itemClicked(itemStr);
}

void DeviceWidget::initWidgets()
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(mp_Splitter);
    mp_Splitter->addWidget(mp_ListView);
    mp_Splitter->addWidget(mp_InfoPage);
    mp_Splitter->setStretchFactor(0, 1);
    mp_Splitter->setStretchFactor(1, 4);
    setLayout(hLayout);
}

