#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QObject>
#include <QWidget>
#include <DTextBrowser>
#include <QDomDocument>

DWIDGET_USE_NAMESPACE

class DeviceBaseInfo;

class TextBrowser : public DTextBrowser
{
    Q_OBJECT
public:
    TextBrowser(QWidget *parent = nullptr);

    /**
     * @brief showDeviceInfo : 显示设备信息
     * @param info ：需要显示的设备信息
     */
    void showDeviceInfo(DeviceBaseInfo *info);

    /**
     * @brief showBoardInfo
     * @param info
     */
    void showBoardInfo(DeviceBaseInfo *info);

    /**
     * @brief updateInfo : 点击详细按钮之后的展开和收起操作
     */
    void updateInfo();


public slots:
    void fillClipboard();

protected:
    virtual void paintEvent(QPaintEvent *event)override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;


signals:
    void refreshInfo();
    void exportInfo();

private slots:
    void slotShowMenu(const QPoint &);
    void slotActionRefresh();
    void slotActionExport();

private:
    void domTitleInfo(QDomDocument &doc, DeviceBaseInfo *info);
    void domTableInfo(QDomDocument &doc, const QList<QPair<QString, QString>> &info);

private:
    bool                    m_ShowOtherInfo;
    DeviceBaseInfo          *mp_Info;
    QAction                 *mp_Refresh;     //<! 右键刷新
    QAction                 *mp_Export;      //<! 右键导出
    QMenu                   *mp_Menu;        //<! 右键菜单
};

#endif // TEXTBROWSER_H
