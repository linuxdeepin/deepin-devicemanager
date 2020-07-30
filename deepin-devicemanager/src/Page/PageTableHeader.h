#ifndef DEVICETABLEPAGE_H
#define DEVICETABLEPAGE_H

#include <QObject>
#include <DWidget>

class TableWidget;

using namespace Dtk::Widget;
class PageTableHeader : public DWidget
{
    Q_OBJECT
public:
    PageTableHeader(QWidget *parent = nullptr);
    ~PageTableHeader();

    void updateTable(const QList<QStringList> &lst);

    void setColumnAverage();

signals:
    void itemClicked(int row);
    void refreshInfo();
    void exportInfo();

protected:
    void paintEvent(QPaintEvent *e) override;

private slots:
    void slotItemClicked(int row);
    void slotRefreshInfo();
    void slotExportInfo();

private:
    /**
     * @brief initWidgets : 初始化界面布局
     */
    void initWidgets();

private:
    TableWidget               *mp_Table;
};

#endif // DEVICETABLEPAGE_H
