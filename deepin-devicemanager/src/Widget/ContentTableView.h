#ifndef CONTENTTABLEVIEW_H
#define CONTENTTABLEVIEW_H

#include <QObject>
#include <DTableView>
#include <QStandardItemModel>
#include <DStandardItem>

using namespace Dtk::Widget;
class ContentTableView : public DTableView
{
    Q_OBJECT
public:
    ContentTableView(QWidget *parent = nullptr);
    ~ContentTableView();

    /**
     * @brief setItem : 设置表格的item
     * @param row : item设置到哪一行
     * @param column : item设置到哪一列
     * @param item ：需要设置的item
     */
    void setItem(int row, int column, DStandardItem *item);

    /**
     * @brief clear : 清空数据
     */
    void clear();

private:
    QStandardItemModel       *mp_Model;

};

#endif // CONTENTTABLEVIEW_H
