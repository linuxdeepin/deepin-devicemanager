#ifndef PAGEOVERVIEW_H
#define PAGEOVERVIEW_H

#include <QObject>
#include <DWidget>
#include <DLabel>

#include "PageInfo.h"

DWIDGET_USE_NAMESPACE

class DetailTreeView;

class PageOverview : public PageInfo
{
public:
    PageOverview(DWidget *parent = nullptr);

    /**
     * @brief updateTable : 更新数据
     * @param lst ：需要更新的数据
     */
    void updateInfo(const QList<DeviceBaseInfo *> &lst) override;

    void updateInfo(const QMap<QString, QString> &map) override;

    void setLabel(const QString &itemstr) override;

    bool isOverview() override {return  true;}

private:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

private:
    DLabel           *mp_PicLabel;
    DLabel           *mp_TextLabel;
    DetailTreeView   *mp_Overview;
};

#endif // PAGEOVERVIEW_H
