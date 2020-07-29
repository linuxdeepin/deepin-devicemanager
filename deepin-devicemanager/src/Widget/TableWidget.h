#ifndef HEADERTABLEVIEW_H
#define HEADERTABLEVIEW_H

#include <QObject>
#include <DTableView>
#include <QStandardItemModel>
#include <DStandardItem>
#include <DHeaderView>

class LogTreeView;

using namespace Dtk::Widget;

class TableWidget : public DWidget
{
    Q_OBJECT
public:
    TableWidget(QWidget *parent = nullptr);

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

    void setColumnAverage();

    /**
     * @brief clear : 清空数据
     */
    void clear();

signals:
    void itemClicked(int row);

protected:
    void paintEvent(QPaintEvent *e) override;

private slots:
    void slotItemClicked(const QModelIndex &index);

private:
    void initWidget();

private:
    LogTreeView      *mp_Table;
};






#endif // HEADERTABLEVIEW_H
