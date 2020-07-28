#ifndef PAGEINFOWIDGET_H
#define PAGEINFOWIDGET_H

#include <QObject>
#include <DWidget>

class DeviceBaseInfo;
class PageInfo;

using namespace Dtk::Widget;
class PageInfoWidget : public DWidget
{
public:
    PageInfoWidget(QWidget *parent = nullptr);

    /**
     * @brief updateTable : 更新数据
     * @param lst ：需要更新的数据
     */
    void updateTable(const QString &itemStr, const QList<DeviceBaseInfo *> &lst);

    void updateTable(const QString &itemStr, const QMap<QString, QString> &map);


private:
    void initWidgets();

private:
    PageInfo     *mp_PageInfo;
};

#endif // PAGEINFOWIDGET_H
