#pragma once
#include "DTableWidget"

class TableWidgetAlwaysFocus: public Dtk::Widget::DTableWidget
{
public:
    TableWidgetAlwaysFocus(QWidget *parent = nullptr);

private:
    void focusOutEvent(QFocusEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
//    void enterEvent(QEvent *event) override;
//    void leaveEvent(QEvent *event) override;
//    void showEvent(QShowEvent *event) override;
};
