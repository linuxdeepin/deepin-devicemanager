#ifndef DEVICEDETAILPAGE_H
#define DEVICEDETAILPAGE_H

#include <QObject>
#include <DWidget>

using namespace Dtk::Widget;
class DeviceDetailPage : public DWidget
{
    Q_OBJECT
public:
    explicit DeviceDetailPage(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // DEVICEDETAILPAGE_H
