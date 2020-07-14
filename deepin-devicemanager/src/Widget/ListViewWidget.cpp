#include "ListViewWidget.h"

#include <QHBoxLayout>
#include <DApplicationHelper>

#include "DeviceListView.h"
#include "MacroDefinition.h"

ListViewWidget::ListViewWidget(DWidget *parent)
    : DWidget(parent)
    , mp_ListView(new DeviceListView(this))
    , mp_Enable(new QAction(tr("Enable"), this))
    , mp_Disable(new QAction(tr("Disable"), this))
    , mp_Menu(new QMenu(this))
{
    //初始化界面
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->addWidget(mp_ListView);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    // 初始化右键菜单
    mp_ListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mp_Enable, &QAction::triggered, this, &ListViewWidget::slotActionEnable);
    connect(mp_Disable, &QAction::triggered, this, &ListViewWidget::slotActionDisable);
    connect(mp_ListView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(slotShowMenu(const QPoint &)));
}

ListViewWidget::~ListViewWidget()
{

}

void ListViewWidget::paintEvent(QPaintEvent *event)
{
    // 让背景色适合主题颜色
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::ItemBackground, pa.brush(DPalette::Base));
    pa.setBrush(DPalette::Background, pa.brush(DPalette::Base));
    DApplicationHelper::instance()->setPalette(this, pa);
    return DWidget::paintEvent(event);
}

void ListViewWidget::slotShowMenu(const QPoint &)
{
    mp_Menu->clear();
    bool enable = mp_ListView->curItemEnable();
    if (enable) {
        mp_Menu->addAction(mp_Disable);
    } else {
        mp_Menu->addAction(mp_Enable);
    }
    mp_Menu->exec(QCursor::pos());
}

void ListViewWidget::slotActionEnable()
{
    mp_ListView->setCurItemEnable(true);
}

void ListViewWidget::slotActionDisable()
{
    mp_ListView->setCurItemEnable(false);
}
