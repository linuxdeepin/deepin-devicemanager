#pragma once
#include "DTableWidget"
#include "DHeaderView"
#include <DStyledItemDelegate>

class TableWidgetAlwaysActiveHeaderViewDelegate : public Dtk::Widget::DStyledItemDelegate
{
    Q_OBJECT
public:
    TableWidgetAlwaysActiveHeaderViewDelegate(QAbstractItemView *parent = nullptr);

    //void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class TableWidgetAlwaysActiveHeaderView: public Dtk::Widget::DHeaderView
{
public:
    TableWidgetAlwaysActiveHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

private:
    //void paintEvent(QPaintEvent *e) override;
    QStyleOptionViewItem viewOptions() const override;
};



class TableWidgetAlwaysActive: public Dtk::Widget::DTableWidget
{
public:
    TableWidgetAlwaysActive(QWidget *parent = nullptr);

private:
    QStyleOptionViewItem viewOptions() const override;
};
