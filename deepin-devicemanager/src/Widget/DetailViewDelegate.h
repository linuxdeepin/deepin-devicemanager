#ifndef DETAILVIEWDELEGATE_H
#define DETAILVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>

class DetailViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    DetailViewDelegate(QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &,
                          const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

};

#endif // DETAILVIEWDELEGATE_H
