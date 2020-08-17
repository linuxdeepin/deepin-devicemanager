#include "PageListView.h"

#include <QHBoxLayout>
#include <DApplicationHelper>

#include <QDebug>

#include "DeviceListView.h"
#include "MacroDefinition.h"

PageListView::PageListView(DWidget *parent)
    : DWidget(parent)
    , mp_ListView(new DeviceListView(this))
    , mp_Enable(new QAction(tr("Enable"), this))
    , mp_Disable(new QAction(tr("Disable"), this))
    , mp_Menu(new QMenu(this))
    , m_CurType(tr("Overview"))
{
    //初始化界面
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->addWidget(mp_ListView);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    this->setFixedWidth(152);
    // 初始化右键菜单
    mp_ListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mp_ListView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Enable, &QAction::triggered, this, &PageListView::slotActionEnable);
    connect(mp_Disable, &QAction::triggered, this, &PageListView::slotActionDisable);

    // 连接item点击事件
    connect(mp_ListView, &DListView::clicked, this, &PageListView::slotListViewItemClicked);
}

PageListView::~PageListView()
{

}

void PageListView::updateListItems(const QList<QPair<QString, QString> > &lst)
{
    if (! mp_ListView) {
        return;
    }

    // 更新之前先清理
    mp_ListView->clearItem();

    // 更新 list
    foreach (auto it, lst) {
        mp_ListView->addItem(it.first, it.second);
    }

    // 更新之后恢复之前显示的设备
    mp_ListView->setCurItem(m_CurType);
    emit itemClicked(m_CurType);
}

void PageListView::paintEvent(QPaintEvent *event)
{
    // 让背景色适合主题颜色
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::ItemBackground, pa.brush(DPalette::Base));
    pa.setBrush(DPalette::Background, pa.brush(DPalette::Base));
    DApplicationHelper::instance()->setPalette(this, pa);
    return DWidget::paintEvent(event);
}

void PageListView::slotShowMenu(const QPoint &)
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

void PageListView::slotActionEnable()
{
    mp_ListView->setCurItemEnable(true);
}

void PageListView::slotActionDisable()
{
    mp_ListView->setCurItemEnable(false);
}

void PageListView::slotListViewItemClicked(const QModelIndex &index)
{
    QString concateStr = mp_ListView->getConcatenateStrings(index);
    if (!concateStr.isEmpty()) {
        emit itemClicked(concateStr);
        m_CurType = concateStr;
    }
}
