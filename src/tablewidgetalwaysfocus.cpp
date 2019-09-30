#include "tablewidgetalwaysfocus.h"
#include "mainwindow.h"
#include <QStyledItemDelegate>
DWIDGET_USE_NAMESPACE

class NoFocusDelegate: public QStyledItemDelegate
{
public:
    NoFocusDelegate()
    {

    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if( !(option.type & QStyleOption::SO_FocusRect) )
        {
            QStyleOptionViewItem option2 = option;
            option2.type |= QStyleOption::SO_FocusRect;
            return QStyledItemDelegate::paint(painter, option2, index);
        }


        QStyledItemDelegate::paint(painter, option, index);
    }
};

TableWidgetAlwaysFocus::TableWidgetAlwaysFocus(QWidget *parent):DTableWidget(parent)
{
    MainWindow* mainWindow = dynamic_cast<MainWindow*>(parent->parent());
    //setFocusProxy(mainWindow);
    //setAutoFillBackground(false);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    //setEnabled(true);
    //setWindowFlags(/*Qt::Popup |*/ Qt::FramelessWindowHint);
    //setModel(true);
    //setModal(true);
    //setMouseTracking(false);
    //setStyleSheet("background-color: #FFFFCC;");
    //QPalette pal = palette();

    // set black background
    //pal.setColor(QPalette::Foreground, Qt::black);

    //setAutoFillBackground(true);
    //setPalette(pal);
    //setStyleSheet("QTableView{outline: 0;}");
    //setFrameShape(Shape::NoFrame);
    //setItemDelegate(new NoFocusDelegate());
}

void TableWidgetAlwaysFocus::focusOutEvent(QFocusEvent *event)
{
    //setFocus(Qt::FocusReason::OtherFocusReason);
    DTableWidget::focusOutEvent(event);
    //setBackgroundRole(QPalette::Dark);
    //setPalette(QPalette(QColor(255,255,255)));
    //focusInEvent(event);
    //setFocus(Qt::FocusReason::OtherFocusReason);
}

void TableWidgetAlwaysFocus::focusInEvent(QFocusEvent *event)
{
    //setFocus(Qt::FocusReason::OtherFocusReason);
    DTableWidget::focusInEvent(event);
}

void TableWidgetAlwaysFocus::showEvent(QShowEvent *event)
{
    DTableWidget::showEvent(event);
    //setFocus(Qt::FocusReason::OtherFocusReason);
    setCurrentItem(nullptr);
}

void TableWidgetAlwaysFocus::enterEvent(QEvent *event)
{
    DTableWidget::enterEvent(event);
}

void TableWidgetAlwaysFocus::leaveEvent(QEvent *event)
{
    DTableWidget::leaveEvent(event);
    //setFocus(Qt::FocusReason::OtherFocusReason);
}
