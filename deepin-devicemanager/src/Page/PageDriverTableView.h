#ifndef PAGEDRIVERTABLEVIEW_H
#define PAGEDRIVERTABLEVIEW_H

#include <DWidget>

#include <QObject>
#include <QAbstractItemModel>

#include "MacroDefinition.h"

class DriverTableView;

DWIDGET_USE_NAMESPACE

class PageDriverTableView : public DWidget
{
    Q_OBJECT
public:
    explicit PageDriverTableView(DWidget *parent = nullptr);

    /**
     * @brief setColumnWidth
     * @param row
     * @param column
     */
    void setColumnWidth(int row, int column);

    /**
     * @brief appendRowItems 添加郑行item
     */
    void appendRowItems(int column);

    /**
     * @brief setIndexWidget 设置 item widget
     * @param row 行
     * @param column 列
     * @param widget widget
     */
    void setWidget(int row, int column, DWidget* widget);

    /**
     * @brief model
     * @return
     */
    QAbstractItemModel* model() const;

    /**
     * @brief initHeaderView
     * @param headerList
     */
    void initHeaderView(const QStringList& headerList, bool check = false);

    /**
     * @brief setHeaderCbStatus 该函数的作用是先判断是否所有item都被选中，如果都被选中则表头选中，如果有没有被选中则表头不选中
     * @param checked
     */
    void setHeaderCbStatus(bool checked);

    /**
     * @brief setTableInstallStatus
     * 安装过程中，所有已经选中的勾选框置灰
     * 安装过程中，勾选框已经选中的，操作按钮置灰
     * 安装过程中，勾选框没有选中的，操作按钮常规显示，如果点击，勾选框选中
     */
    void setCheckedCBDisnable();

    /**
     * @brief getCheckedDriverIndex 获取选中的index
     * @param lstIndex
     */
    void getCheckedDriverIndex(QList<int> &lstIndex);

    /**
     * @brief setItemStatus
     * @param index
     * @param s
     */
    void setItemStatus(int index, Status s);

    /**
     * @brief setErrorMsg
     * @param index
     * @param msg
     */
    void setErrorMsg(int index, const QString& msg);

    /**
     * @brief hasItemDisabled
     * @return
     */
    bool hasItemDisabled();

    /**
     * @brief clear 清楚表格所有内容
     */
    void clear();

signals:
    void operatorClicked(int index);
    /**
     * @brief itemChecked 选中的信号
     * @param index
     * @param checked
     */
    void itemChecked(int index, bool checked);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    /**
     * @brief initWidgets 初始化界面
     */
    void initWidgets();

private:
    DriverTableView  *mp_View;
};

#endif // PAGEDRIVERTABLEVIEW_H
