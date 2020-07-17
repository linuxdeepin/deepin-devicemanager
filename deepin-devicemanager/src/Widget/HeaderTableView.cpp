#include "HeaderTableView.h"

#include <DFontSizeManager>

#include "MacroDefinition.h"

HeaderTableView::HeaderTableView(QWidget *parent)
    : DTableView(parent)
    , mp_Model(new QStandardItemModel(this))
{
    // 绑定 model
    setModel(mp_Model);

    // 绑定到系统字体
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);

    // 设置表格可以排序
    setSortingEnabled(true);

    // 设置不可编辑
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

HeaderTableView::~HeaderTableView()
{
    DELETE_PTR(mp_Model);
}

void HeaderTableView::setHeaderLabels(const QStringList &lst)
{
    if (mp_Model) {
        mp_Model->setHorizontalHeaderLabels(lst);
    }
}

void HeaderTableView::setItem(int row, int column, DStandardItem *item)
{
    if (mp_Model) {
        mp_Model->setItem(row, column, item);
    }
}

void HeaderTableView::clear()
{
    if (mp_Model) {
        mp_Model->clear();
    }
}
