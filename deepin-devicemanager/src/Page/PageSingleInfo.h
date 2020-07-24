#ifndef PAGESINGLEINFO_H
#define PAGESINGLEINFO_H

#include <QObject>
#include <QWidget>

#include <DLabel>

#include "PageInfo.h"
#include "DetailTreeView.h"

DWIDGET_USE_NAMESPACE
using namespace Dtk::Widget;

class PageSingleInfo : public PageInfo
{
public:
    PageSingleInfo(QWidget *parent = nullptr);
    ~PageSingleInfo() override;

    void updateInfo(const QList<DeviceBaseInfo *> &lst)override;

    void setLabel(const QString &itemstr) override;

    void loadDeviceInfo(const QList<QPair<QString, QString>> &lst);

    void clearContent() override;
private:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

private:
    DetailTreeView          *mp_Content;
    DLabel                  *mp_Label;
};

#endif // PAGESINGLEINFO_H
