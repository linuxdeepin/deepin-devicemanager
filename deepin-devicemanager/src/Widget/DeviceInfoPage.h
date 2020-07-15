#ifndef DEVICEPAGE_H
#define DEVICEPAGE_H

#include <QObject>
#include <DWidget>
#include <DSplitter>

class DeviceTablePage;
class DeviceDetailPage;

using namespace Dtk::Widget;
class DeviceInfoPage : public DWidget
{
public:
    DeviceInfoPage(QWidget *parent = nullptr);
    ~DeviceInfoPage()override;

private:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

private:
    DeviceTablePage           *mp_Table;       //<! 上面的表格
    DeviceDetailPage          *mp_Detail;      //<! 下面的详细内容
    DSplitter                 *mp_Splitter;          //<! 左右两边分页器
};

#endif // DEVICEPAGE_H
