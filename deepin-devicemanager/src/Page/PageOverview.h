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
    Q_OBJECT
public:
    PageOverview(DWidget *parent = nullptr);

    /**
     * @brief updateTable : 更新数据
     * @param lst ：需要更新的数据
     */
    void updateInfo(const QList<DeviceBaseInfo *> &) override;

    /**
     * @brief updateInfo
     * @param map
     */
    void updateInfo(const QMap<QString, QString> &map) override;

    /**
     * @brief setLabel
     * @param itemstr
     */
    void setLabel(const QString &itemstr) override;

    void setLabel(const QString &str1, const QString &str2) override;

    /**
     * @brief isOverview
     * @return
     */
    bool isOverview() override {return  true;}

signals:
    void refreshInfo();
    void exportInfo();

private slots:
    void slotShowMenu(const QPoint &);
    void slotActionRefresh();
    void slotActionExport();

private:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

private:
    DLabel                  *mp_PicLabel;
    DLabel                  *mp_DeviceLabel;
    DLabel                  *mp_OSLabel;
    DetailTreeView          *mp_Overview;
    QAction                 *mp_Refresh;     //<! 右键刷新
    QAction                 *mp_Export;      //<! 右键导出
    QMenu                   *mp_Menu;        //<! 右键菜单
};

#endif // PAGEOVERVIEW_H
