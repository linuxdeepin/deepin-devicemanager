// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "LongTextLabel.h"
#include "DDLog.h"

// Dtk头文件
#include <DFontSizeManager>
#include <DApplication>
#include <DSysInfo>

// Qt库文件
#include <QLoggingCategory>
#include <QAbstractTextDocumentLayout>
#include <QDomDocument>
#include <QRegularExpression>
DCORE_USE_NAMESPACE

using namespace DDLog;

LongTextLabel::LongTextLabel(DWidget *parent)
    : DLabel(parent)
    , m_width(width())
{
    qCDebug(appLog) << "LongTextLabel instance created";
    setAlignment(Qt::AlignVCenter);
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
}

void LongTextLabel::paintEvent(QPaintEvent *)
{
    // qCDebug(appLog) << "Painting LongTextLabel";
    QPainter painter(this);
    QTextDocument docText;
    painter.save();

    QFont fo = painter.font();
    QFontMetrics fm(fo);

    QString html;
    QString OS;

    QRegularExpression reg("([\\s\\S]*</a>)");
    QRegularExpressionMatch match = reg.match(this->text());
    if (match.hasMatch()) {
        // qCDebug(appLog) << "Found HTML tag in label text";
        html = match.captured(1);

        QRegularExpression contentReg("<[\\s\\S]*>([\\s\\S]*)</a>");
        QRegularExpressionMatch contentMatch = contentReg.match(html);
        if (contentMatch.hasMatch()) {
            // qCDebug(appLog) << "Extracted OS from HTML tag";
            OS = contentMatch.captured(1);
        }
    }

    QString src = this->text();
    if (!html.isEmpty() && !OS.isEmpty()) {
        // qCDebug(appLog) << "Processing text with HTML removed";
        src  = OS + this->text().remove(html);
    }

    QString text = fm.elidedText(src, Qt::TextElideMode::ElideRight, this->rect().width());

    QString dst = text;
    if (!html.isEmpty() && !OS.isEmpty()) {
        // qCDebug(appLog) << "Re-adding HTML to elided text";
        dst = html + text.remove(OS);
    }

    docText.setHtml(dst);
    docText.setDefaultFont(fo);
    docText.drawContents(&painter, this->rect());
    painter.restore();
//    return DLabel::paintEvent(event);
}
