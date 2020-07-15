#ifndef LISTVIEWWIDGET_H
#define LISTVIEWWIDGET_H

#include <DWidget>
#include <QAction>
#include <QMenu>
#include <QList>

class DeviceListView;

using namespace Dtk::Widget;
class ListViewWidget : public DWidget
{
    Q_OBJECT
public:
    ListViewWidget(DWidget *parent = nullptr);
    ~ListViewWidget();

    void updateListItems(const QList<QPair<QString, QString> > &lst);

protected:
    /**@brief:事件重写*/
    void paintEvent(QPaintEvent *event) override;

private slots:
    void slotShowMenu(const QPoint &point);
    void slotActionEnable();
    void slotActionDisable();

private:
    DeviceListView            *mp_ListView;
    QAction                   *mp_Enable;
    QAction                   *mp_Disable;
    QMenu                     *mp_Menu;
};

#endif // LISTVIEWWIDGET_H
