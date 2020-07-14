#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>

#include <DMainWindow>
#include <DStackedWidget>

class WaitingWidget;
class DetailWidget;

using namespace Dtk::Widget;
class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    /**
     * @brief: 事件的重写
     */
    virtual void resizeEvent(QResizeEvent *event)override;

private:
    /** @brief initWindow:对窗口进行一系列的初始化操作*/
    void initWindow();

    /** @brief initWindowSize:初始化窗口大小*/
    void initWindowSize();

    /** @brief initWidgets:初始化界面相关的内容*/
    void initWidgets();




private:
    /**@brief:主窗口的stackWidget，主要有两个widget，一个是等待界面，还有一个是信息显示界面*/
    DStackedWidget        *mp_MainStackWidget;
    WaitingWidget         *mp_WaitingWidget;
    DetailWidget          *mp_DetailWidget;
};

#endif // MAINWINDOW_H
