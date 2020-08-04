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
    Q_OBJECT
public:
    PageSingleInfo(QWidget *parent = nullptr);
    ~PageSingleInfo() override;

    /**
     * @brief updateInfo
     * @param lst
     */
    virtual void updateInfo(const QList<DeviceBaseInfo *> &lst)override;

    /**
     * @brief setLabel
     * @param itemstr
     */
    void setLabel(const QString &itemstr) override;

    /**
     * @brief loadDeviceInfo
     * @param lst
     */
    virtual void loadDeviceInfo(const QList<QPair<QString, QString>> &lst);

    /**
     * @brief clearContent
     */
    void clearContent() override;

signals:
    void refreshInfo();
    void exportInfo();

protected slots:
    void slotShowMenu(const QPoint &);
    void slotActionRefresh();
    void slotActionExport();
protected:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

protected:
    DetailTreeView          *mp_Content;
    DLabel                  *mp_Label;
    QAction                 *mp_Refresh;     //<! 右键刷新
    QAction                 *mp_Export;      //<! 右键导出
    QMenu                   *mp_Menu;        //<! 右键菜单
};

#endif // PAGESINGLEINFO_H
