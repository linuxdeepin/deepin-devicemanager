#include "ContentTableView.h"
#include "MacroDefinition.h"

#include <QHeaderView>

ContentTableView::ContentTableView(QWidget *parent)
    : DTableView(parent)
    , mp_Model(new QStandardItemModel(this))
{
    // 绑定 model
    setModel(mp_Model);

    // 绑定到系统字体
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);

    // 设置表格不可以排序
    setSortingEnabled(false);

    // 设置不可编辑
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 隐藏第一行第一列
    this->horizontalHeader()->setVisible(false);
    this->verticalHeader()->setVisible(false);
    this->horizontalHeader()->setStretchLastSection(true); //设置充满表宽度
    this->verticalHeader()->setDefaultSectionSize(36); //设置行高
//    this->horizontalHeader()->setDefaultSectionSize(QHeaderView::Stretch);
}

ContentTableView::~ContentTableView()
{
    DELETE_PTR(mp_Model);
}

void ContentTableView::setItem(int row, int column, DStandardItem *item)
{
    if (mp_Model) {
        mp_Model->setItem(row, column, item);
    }
}

void ContentTableView::clear()
{
    if (mp_Model) {
        mp_Model->clear();
    }
}


