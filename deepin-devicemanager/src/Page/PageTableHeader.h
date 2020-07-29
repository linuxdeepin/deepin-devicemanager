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

protected:
    void paintEvent(QPaintEvent *e) override;

private slots:
    void slotShowMenu(const QPoint &);
    void slotActionRefresh();
    void slotActionExport();
    void slotActionCopy();
    void slotItemClicked(int row);

private:
    /**
     * @brief initWidgets : 初始化界面布局
     */
    void initWidgets();

private:
    TableWidget               *mp_Table;

    QAction                   *mp_Refresh;     //<! 右键刷新
    QAction                   *mp_Export;      //<! 右键导出
    QAction                   *mp_Copy;        //<! 右键拷贝
    QMenu                     *mp_Menu;        //<! 右键菜单
};

#endif // DEVICETABLEPAGE_H
