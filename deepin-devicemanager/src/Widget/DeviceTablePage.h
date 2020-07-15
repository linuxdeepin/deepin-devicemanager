#ifndef DEVICETABLEPAGE_H
#define DEVICETABLEPAGE_H

#include <QObject>
#include <DWidget>

class DeviceTableView;

using namespace Dtk::Widget;
class DeviceTablePage : public DWidget
{
public:
    DeviceTablePage(QWidget *parent = nullptr);

private:
    /**
     * @brief initWidgets : 初始化界面布局
     */
    void initWidgets();

private:
    DeviceTableView        *mp_Table;
};

#endif // DEVICETABLEPAGE_H
