#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H

#include <QObject>
#include <DWidget>
#include <DSplitter>

class ListViewWidget;
class DeviceDetailPage;

using namespace Dtk::Widget;

class DetailWidget : public DWidget
{
public:
    DetailWidget(QWidget *parent);
    ~DetailWidget();

private:
    /**@brief:初始化界面布局*/
    void initWidgets();

private:
    ListViewWidget            *mp_ListView;          //<! 左边的list
    DeviceDetailPage          *mp_DetailPage;        //<! 右边的详细内容
    DSplitter                 *mp_Splitter;          //<! 左右两边分页器
};

#endif // DETAILWIDGET_H
