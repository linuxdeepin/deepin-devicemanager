#include "DetailWidget.h"

#include "ListViewWidget.h"
#include "DeviceDetailPage.h"
#include "MacroDefinition.h"

#include <QHBoxLayout>

DetailWidget::DetailWidget(QWidget *parent)
    : DWidget(parent)
    , mp_ListView(new ListViewWidget(this))
    , mp_DetailPage(new DeviceDetailPage(this))
    , mp_Splitter(new DSplitter(Qt::Horizontal, this))
{
    // 初始化界面布局
    initWidgets();
}

DetailWidget::~DetailWidget()
{
    DELETE_PTR(mp_ListView);
    DELETE_PTR(mp_DetailPage);
    DELETE_PTR(mp_Splitter);
}

void DetailWidget::initWidgets()
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(mp_Splitter);
    mp_Splitter->addWidget(mp_ListView);
    mp_Splitter->addWidget(mp_DetailPage);
    mp_Splitter->setStretchFactor(0, 1);
    mp_Splitter->setStretchFactor(1, 10);
    setLayout(hLayout);
}

