#ifndef HEADERTABLEVIEW_H
#define HEADERTABLEVIEW_H

#include <QObject>
#include <DTableView>
#include <QStandardItemModel>
#include <DStandardItem>

using namespace Dtk::Widget;
class HeaderTableView : public DTableView
{
    Q_OBJECT
public:
    HeaderTableView(QWidget *parent = nullptr);
    ~HeaderTableView();

    /**
     * @brief setHeaderLabels : 设置table的表头
     * @param lst ：表头的内容
     */
    void setHeaderLabels(const QStringList &lst);

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

#endif // HEADERTABLEVIEW_H
