#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QObject>
#include <QWidget>
#include <DTextBrowser>

DWIDGET_USE_NAMESPACE

class TextBrowser : public DTextBrowser
{
public:
    TextBrowser(QWidget *parent = nullptr);

protected:
    virtual void paintEvent(QPaintEvent *event);


};

#endif // TEXTBROWSER_H
