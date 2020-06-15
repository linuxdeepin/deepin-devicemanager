#include "TextBrowser.h"
#include <QDebug>
#include<DApplicationHelper>
TextBrowser::TextBrowser(QWidget *parent)
    : DTextBrowser (parent)
{

}

void TextBrowser::paintEvent(QPaintEvent *event)
{
    DPalette pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    DApplicationHelper::instance()->setPalette(this, pa);
    return DTextBrowser::paintEvent(event);
}
