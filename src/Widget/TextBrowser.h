#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QObject>
#include <QWidget>
#include <DTextBrowser>

DWIDGET_USE_NAMESPACE

class DeviceInfoWidgetBase;

class TextBrowser : public DTextBrowser
{
    Q_OBJECT
public:
    TextBrowser(DeviceInfoWidgetBase *parent = nullptr);


public slots:
    void fillClipboard();

protected:
    virtual void paintEvent(QPaintEvent *event)override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    DeviceInfoWidgetBase *m_pDeviceInfoWidget;
};

#endif // TEXTBROWSER_H
