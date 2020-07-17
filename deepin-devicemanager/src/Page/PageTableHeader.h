#ifndef DEVICETABLEPAGE_H
#define DEVICETABLEPAGE_H

#include <QObject>
#include <DWidget>

class HeaderTableView;

using namespace Dtk::Widget;
class PageTableHeader : public DWidget
{
public:
    PageTableHeader(QWidget *parent = nullptr);
    ~PageTableHeader();

    void updateTable(const QList<QStringList> &lst);

private slots:
    void slotShowMenu(const QPoint &);
    void slotActionRefresh();
    void slotActionExport();
    void slotActionCopy();

private:
    /**
     * @brief initWidgets : 初始化界面布局
     */
    void initWidgets();

private:
    HeaderTableView           *mp_Table;

    QAction                   *mp_Refresh;     //<! 右键刷新
    QAction                   *mp_Export;      //<! 右键导出
    QAction                   *mp_Copy;        //<! 右键拷贝
    QMenu                     *mp_Menu;        //<! 右键菜单
};

#endif // DEVICETABLEPAGE_H
