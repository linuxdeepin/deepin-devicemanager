#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>

#include <DMainWindow>
#include <DStackedWidget>

class WaitingWidget;
class DeviceWidget;
class ThreadPool;

using namespace Dtk::Widget;
class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    /**
     * @brief refresh:界面刷新
     */
    void refresh();

    /**
     * @brief exportTo:导出设备信息
     * @return true:导出成功，false:导出失败
     */
    bool exportTo();

    /**
     * @brief showDisplayShortcutsHelpDialog:显示快捷键帮主窗口
     */
    void showDisplayShortcutsHelpDialog();

    /**
     * @brief addJsonArrayItem:添加json键值对
     * @param windowJsonItems:保存json键值对数组
     * @param name:json名称
     * @param value:json值
     */
    void addJsonArrayItem(QJsonArray &windowJsonItems, const QString &name, const QString &value);

    /**
     * @brief getJsonDoc:获取json文本信息
     * @return json文本信息
     */
    void getJsonDoc(QJsonDocument &doc);

    /**
     * @brief windowMaximizing:窗口最大化
     */
    void windowMaximizing();

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

    /**
     * @brief refreshDataBase:刷新设备信息
     */
    void refreshDataBase();

    /**
     * @brief keyPressEvent:相应键盘按键事件
     * @param e:按键：E导出，F大小调整，F5刷新，C复制
     */
    void keyPressEvent(QKeyEvent *e) override;

private slots:

    /**
     * @brief loadingFinishSlot:加载设备信息结束 槽
     * @param message:提示信息
     */
    void loadingFinishSlot(const QString &message);

private:
    /**@brief:主窗口的stackWidget，主要有两个widget，一个是等待界面，还有一个是信息显示界面*/
    DStackedWidget        *mp_MainStackWidget;
    WaitingWidget         *mp_WaitingWidget;
    DeviceWidget          *mp_DetailWidget;

    ThreadPool            *mp_ThreadPool;

    bool                  m_refreshing = false;                  // 判断界面是否正在刷新
};

#endif // MAINWINDOW_H
