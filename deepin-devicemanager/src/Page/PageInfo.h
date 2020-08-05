#ifndef PAGEINFO_H
#define PAGEINFO_H

#include <QObject>
#include <DWidget>

class DeviceBaseInfo;

using namespace Dtk::Widget;
class PageInfo : public DWidget
{
    Q_OBJECT
public:
    PageInfo(QWidget *parent = nullptr);

    /**
     * @brief updateTable : 更新数据
     * @param lst ：需要更新的数据
     */
    virtual void updateInfo(const QList<DeviceBaseInfo *> &lst) = 0;

    virtual void updateInfo(const QMap<QString, QString> &map);

    virtual void setLabel(const QString &itemstr) = 0;

    virtual void setLabel(const QString &str1, const QString &str2);

    virtual void clearContent();

    virtual bool isOverview() {return false;}

    virtual bool isBaseBoard() {return false;}

protected:
    void paintEvent(QPaintEvent *e) override;
};

#endif // PAGEINFO_H
