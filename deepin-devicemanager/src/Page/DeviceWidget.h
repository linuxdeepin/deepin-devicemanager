#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H

#include <QObject>
#include <DWidget>
#include <DSplitter>

class PageListView;
class PageInfo;
class DeviceBaseInfo;

using namespace Dtk::Widget;

class DeviceWidget : public DWidget
{
    Q_OBJECT
public:
    DeviceWidget(QWidget *parent = nullptr);
    ~DeviceWidget();

    void updateListView(const QList<QPair<QString, QString> > &lst);
    void updateDevice(const QList<DeviceBaseInfo *> &lst);

signals:
    void itemClicked(const QString &itemStr);

private slots:
    void slotListViewWidgetItemClicked(const QString &itemStr);

private:
    /**@brief:初始化界面布局*/
    void initWidgets();

private:
    PageListView              *mp_ListView;          //<! 左边的list
    PageInfo                  *mp_PageInfo;          //<! 右边的详细内容
    DSplitter                 *mp_Splitter;          //<! 左右两边分页器
};

#endif // DETAILWIDGET_H
