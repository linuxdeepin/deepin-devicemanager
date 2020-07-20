#ifndef HEADERTABLEVIEW_H
#define HEADERTABLEVIEW_H

#include <QObject>
#include <DTableView>
#include <QStandardItemModel>
#include <DStandardItem>
#include <DHeaderView>

using namespace Dtk::Widget;

/**
 * @brief The HeaderView class : HeaderTableView的表头
 */
class HeaderView : public DHeaderView
{
public:
    HeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

protected:
    // 重新绘制单元格
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    // 重新绘制表头
    void paintEvent(QPaintEvent *e) override;

};



class TableViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TableViewItemDelegate(QObject *parent);
protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};


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

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    /**
     * @brief initWidgets : 表格的初始化工作
     */
    void initWidgets();

private:
    QStandardItemModel       *mp_Model;
    HeaderView               *mp_HeaderView;        //<! 设置自己的表头
    TableViewItemDelegate    *mp_ItemDelegate;      //<! 委托代理
};

#endif // HEADERTABLEVIEW_H
