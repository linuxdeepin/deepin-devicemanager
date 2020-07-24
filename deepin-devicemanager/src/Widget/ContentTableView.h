#ifndef CONTENTTABLEVIEW_H
#define CONTENTTABLEVIEW_H

#include <DWidget>
#include <DStandardItem>

DWIDGET_USE_NAMESPACE

class DetailTreeView;

class ContentWidget : public DWidget
{
public:
    ContentWidget(QWidget *parent = nullptr);

    /**
     * @brief setItem : 设置表格的item
     * @param row : item设置到哪一行
     * @param column : item设置到哪一列
     * @param item ：需要设置的item
     */
    void setItem(int row, int column, DStandardItem *item);

    /**
     * @brief clear : 清空数据
     */
    void clear();

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    void initWidget();

private:
    DetailTreeView      *mp_Table;
};





#endif // CONTENTTABLEVIEW_H
