#ifndef DETAILTREEVIEW_H
#define DETAILTREEVIEW_H

#include <QStandardItem>

#include <DTreeView>
#include <DWidget>
#include <DCommandLinkButton>
#include <DTableView>
#include <DTableWidget>

DWIDGET_USE_NAMESPACE

class DetailViewDelegate;
class ButtonDelegate;
class DetailButton;

class DetailTreeView: public DTableWidget//DTreeView
{
public:
    DetailTreeView(DWidget *parent = nullptr);

    /**
     * @brief setColumnAndRow:设置表格行数和列数
     * @param row:表格行数
     * @param column:表格列数
     */
    void setColumnAndRow(int row, int column = 2);

    /**
     * @brief setItem : 设置表格的item
     * @param row : item设置到哪一行
     * @param column : item设置到哪一列
     * @param item ：需要设置的item
     */
    void setItem(int row, int column, QTableWidgetItem *item);

    /**
     * @brief clear : 清空数据
     */
    void clear();

    /**
     * @brief setCommanLinkButton:设置扩展按钮属性，及布局
     * @param row:扩展按钮在表格中的行数
     */
    void setCommanLinkButton(int row);

    /**
     * @brief setTableHeight:设置表格高度
     * @return 重绘矩形的高度
     */
    int setTableHeight(int paintHeight);

    /**
     * @brief hasExpendInfo:是否由详细信息按钮
     * @return 布尔值：true有详细信息按钮，false没有xiangzx详细信息按钮
     */
    bool hasExpendInfo();

public slots:
    /**
     * @brief expandCommandLinkClicked:展开button点击槽函数
     */
    void expandCommandLinkClicked();

protected:
    /**
     * @brief initUI:初始化界面
     */
    void initUI();

    void paintEvent(QPaintEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    DetailViewDelegate        *mp_ItemDelegate;   // Item自定义代理

    DCommandLinkButton        *mp_CommandBtn;    // 展开命令Button

    int            m_LimitRow;     // 正常状态下，表格显示的行数

    bool           m_IsExpand;     // 是否展开
};

#endif // DETAILTREEVIEW_H
