#ifndef PAGEINFOWIDGET_H
#define PAGEINFOWIDGET_H

#include <QObject>
#include <DWidget>
//#include <QHBoxLayout>

class DeviceBaseInfo;
class PageInfo;
class PageMultiInfo;
class PageSingleInfo;
class PageOverview;

using namespace Dtk::Widget;
class PageInfoWidget : public DWidget
{
    Q_OBJECT
public:
    PageInfoWidget(QWidget *parent = nullptr);

    /**
     * @brief updateTable : 更新数据
     * @param lst ：需要更新的数据
     */
    void updateTable(const QString &itemStr, const QList<DeviceBaseInfo *> &lst);

    /**
     * @brief updateTable
     * @param itemStr
     * @param map
     */
    void updateTable(const QString &itemStr, const QMap<QString, QString> &map);

signals:
    void refreshInfo();
    void exportInfo();

private slots:
    void slotRefreshInfo();
    void slotExportInfo();

private:
    void initWidgets();

private:
    PageInfo       *mp_PageInfo;
    PageSingleInfo *mp_PageSignalInfo;
    PageMultiInfo  *mp_PageMutilInfo;
    PageOverview   *mp_PageOverviewInfo;
};

#endif // PAGEINFOWIDGET_H
