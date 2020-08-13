#include "LongTextLabel.h"

#include <DFontSizeManager>
#include <DApplication>

#include <QDebug>
#include <QAbstractTextDocumentLayout>
#include <QDomDocument>

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
    QPainter painter(this);
    QTextDocument docText;

    QFont fo = painter.font();
    QFontMetrics fm(fo);
    QString text = fm.elidedText(this->text(), Qt::TextElideMode::ElideRight, this->width());

    docText.setHtml(text);
    docText.drawContents(&painter, this->rect());
    return DWidget::paintEvent(event);
}
