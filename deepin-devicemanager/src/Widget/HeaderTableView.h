#ifndef HEADERTABLEVIEW_H
#define HEADERTABLEVIEW_H

#include <QObject>
#include <DTableView>
#include <QStandardItemModel>
#include <DStandardItem>

using namespace Dtk::Widget;
class HeaderTableView : public DTableView
{
    Q_OBJECT
public:
    HeaderTableView(QWidget *parent = nullptr);
    ~HeaderTableView();

private:
    QStandardItemModel       *mp_Model;
};

#endif // HEADERTABLEVIEW_H
