#ifndef RICHTEXTDELEGATE_H
#define RICHTEXTDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QDomDocument>

class RichTextDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    RichTextDelegate(QObject *parent);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

private:
    void getDocFromLst(QDomDocument &doc, const QStringList &lst)const;
    void addRow(QDomDocument &doc, QDomElement &table, const QPair<QString, QString> &pair)const;
    void roundStr(QString &str)const;
};

#endif // RICHTEXTDELEGATE_H
