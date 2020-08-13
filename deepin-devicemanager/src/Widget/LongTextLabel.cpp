#include "LongTextLabel.h"

#include <DFontSizeManager>
#include <DApplication>

#include <QDebug>
#include <QAbstractTextDocumentLayout>

LongTextLabel::LongTextLabel(DWidget *parent)
    : DLabel(parent)
    , m_width(width())
{
    setOpenExternalLinks(true);
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
}

void LongTextLabel::setLinkText(const QString &linkstr)
{

}

void LongTextLabel::paintEvent(QPaintEvent *event)
{
//    QPainter painter(this);

//    painter.save();
//    int margin = 2;   // 边距
//    QRect rect = this->rect();
//    rect.setX(rect.x() + margin);
//    qDebug() << painter.font();
//    QFontMetrics fm = this->fontMetrics();

//    QString text = fm.elidedText(this->text(), Qt::ElideRight, rect.width());
//    setOpenExternalLinks(true);
//    painter.drawText(rect, Qt::TextSingleLine, text);

//    m_width = rect.width();
//    setLinkText(text());
//    painter.restore();

    QStyleOption opt;
    opt.initFrom(this);

    QPainter painter(this);
    painter.save();

    QTextDocument doc;
    doc.setHtml(text());

//    DStyle *style = this->style();


    painter.restore();


}
