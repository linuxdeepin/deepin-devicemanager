#include "LongTextLabel.h"

#include <DFontSizeManager>
#include <DApplication>
#include <DSysInfo>

#include <QDebug>
#include <QAbstractTextDocumentLayout>
#include <QDomDocument>

DCORE_USE_NAMESPACE

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

    QString html;
    QString OS;

    QRegExp reg("[\\s\\S]*</a>");
    if (reg.indexIn(this->text()) != -1) {
        html = reg.cap(0);

        reg.setPattern("<[\\s\\S]*>([\\s\\S]*)</a>");

        if (reg.indexIn(html) != -1) {
            OS = reg.cap(1);
        }
    }

    QString src = this->text();
    if (!html.isEmpty() && !OS.isEmpty()) {
        src  = OS + this->text().remove(html);
    }

    QString text = fm.elidedText(src, Qt::TextElideMode::ElideRight, this->rect().width());

    QString dst = text;
    if (!html.isEmpty() && !OS.isEmpty()) {
        dst = html + text.remove(OS);
    }

    docText.setHtml(dst);
    docText.setDefaultFont(fo);
    docText.drawContents(&painter, this->rect());
//    return DWidget::paintEvent(event);
}
