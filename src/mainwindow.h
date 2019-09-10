#pragma once

#include <DMainWindow>

class MainWindow : public Dtk::Widget::DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

};
