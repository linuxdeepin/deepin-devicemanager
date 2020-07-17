#ifndef CONTENTTABLEVIEW_H
#define CONTENTTABLEVIEW_H

#include <QObject>
#include <DTableView>
#include <QStandardItemModel>
#include <DStandardItem>

using namespace Dtk::Widget;
class ContentTableView : public DTableView
{
    Q_OBJECT
public:
    ContentTableView(QWidget *parent = nullptr);
};

#endif // CONTENTTABLEVIEW_H
