#ifndef DEVICELISTVIEW_H
#define DEVICELISTVIEW_H

#include <QObject>
#include <QWidget>

#include <DListView>
#include <QStandardItemModel>
#include <DStandardItem>

using namespace Dtk::Widget;



class DeviceListviewDelegate : public DStyledItemDelegate
{
    Q_OBJECT
public:
    DeviceListviewDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const;
};


class DeviceListView : public DListView
{
    Q_OBJECT
public:
    DeviceListView(QWidget *parent = nullptr);
    ~DeviceListView()override;

    /**
     * @brief:添加item
     * @param[in] name : item的文本
     * @param[in] icon : item的图标文件
     */
    void addItem(const QString &name, const QString &iconFile);

    /**
     * @brief:判断当前是否启用
     * @return : 返回是否启用
     */
    bool curItemEnable();

    /**
     * @brief:设置当前是否启用
     * @param[enable] : 是否启用
     */
    void setCurItemEnable(bool enable);

    /**
     * @brief getConcatenateStrings
     * @return
     */
    QString getConcatenateStrings(const QModelIndex &index);

protected:
    /**@brief:事件重写*/
    void paintEvent(QPaintEvent *event) override;

private:
    QStandardItemModel       *mp_ItemModel;                    // Model View 模式里面的 Model
};

#endif // DEVICELISTVIEW_H
