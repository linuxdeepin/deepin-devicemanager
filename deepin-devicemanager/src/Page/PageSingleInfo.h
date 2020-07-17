#ifndef PAGESINGLEINFO_H
#define PAGESINGLEINFO_H

#include <QObject>
#include <QWidget>

#include <DLabel>

#include "PageInfo.h"
#include "ContentTableView.h"

DWIDGET_USE_NAMESPACE
using namespace Dtk::Widget;

class PageSingleInfo : public PageInfo
{
public:
    PageSingleInfo(QWidget *parent = nullptr);
    ~PageSingleInfo() override;

    void updateTable(const QList<DeviceBaseInfo *> &lst)override;

    void loadDeviceInfo(const QList<QPair<QString, QString>> &lst);
private:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

private:
    ContentTableView          *mp_Content;
    DLabel                    *mpLabel;
};

#endif // PAGESINGLEINFO_H
