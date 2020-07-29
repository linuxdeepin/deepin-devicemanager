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

private:
    void domTitleInfo(QDomDocument &doc, DeviceBaseInfo *info);
    void domTableInfo(QDomDocument &doc, const QList<QPair<QString, QString>> &info);

private:
    bool                   m_ShowOtherInfo;
    DeviceBaseInfo        *mp_Info;
};

#endif // TEXTBROWSER_H
