#ifndef PAGEINFO_H
#define PAGEINFO_H

#include <QObject>
#include <DWidget>

class DeviceBaseInfo;

using namespace Dtk::Widget;
class PageInfo : public DWidget
{
public:
    PageInfo(QWidget *parent = nullptr);

    virtual void updateTable(const QList<DeviceBaseInfo *> &lst) = 0;
};

#endif // PAGEINFO_H
