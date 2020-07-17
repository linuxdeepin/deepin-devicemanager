#ifndef LISTVIEWWIDGET_H
#define LISTVIEWWIDGET_H

#include <DWidget>
#include <QAction>
#include <QMenu>
#include <QList>

class DeviceListView;

using namespace Dtk::Widget;
class PageListView : public DWidget
{
    Q_OBJECT
public:
    PageListView(DWidget *parent = nullptr);
    ~PageListView() override;

    void updateListItems(const QList<QPair<QString, QString> > &lst);

protected:
    /**@brief:事件重写*/
    void paintEvent(QPaintEvent *event) override;

signals:
    void itemClicked(const QString &txt);

private slots:
    void slotShowMenu(const QPoint &point);
    void slotActionEnable();
    void slotActionDisable();
    void slotListViewItemClicked(const QModelIndex &index);

private:
    DeviceListView            *mp_ListView;
    QAction                   *mp_Enable;
    QAction                   *mp_Disable;
    QMenu                     *mp_Menu;
};

#endif // LISTVIEWWIDGET_H
