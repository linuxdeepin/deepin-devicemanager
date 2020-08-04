#include "RichTextDelegate.h"

#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>

#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QDebug>

DWIDGET_USE_NAMESPACE

RichTextDelegate::RichTextDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}
void RichTextDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            cg = DPalette::Active;
        }
    }

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QBrush background;

    if (opt.features & QStyleOptionViewItem::Alternate) {
        background = palette.color(cg, DPalette::AlternateBase);
    } else {
        background = palette.color(cg, DPalette::Base);
    }

    // 绘制背景色
    QPainterPath path;
    path.addRect(opt.rect);
    painter->fillPath(path, background);

    QStringList lstStr = opt.text.split("\n");
    if (lstStr.size() > 1) {
        QDomDocument doc;
        getDocFromLst(doc, lstStr);
        QAbstractTextDocumentLayout::PaintContext paintContext;
        QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt);
        // 坐标变换，将左上角设置为原点
        painter->translate(textRect.topLeft());
        // 设置HTML绘制区域
        painter->setClipRect(textRect.translated(-textRect.topLeft()));
        QTextDocument docText;
        docText.setHtml(doc.toString());

        paintContext.palette.setCurrentColorGroup(cg);
        docText.documentLayout()->draw(painter, paintContext);
    } else {
        QFont fo = opt.font;
        if (index.column() == 0) {
            fo.setBold(true);
        } else {
            fo.setBold(false);
        }
        painter->setFont(fo);
        QRect textRect = opt.rect;
        textRect.setX(textRect.x() + margin);
        QFontMetrics fm(opt.font);
        QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
        painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);
    }

    if (index.column() == 0) {
        painter->setPen(palette.color(cg, DPalette::FrameShadowBorder));
        painter->drawLine(opt.rect.topRight(), opt.rect.bottomRight());
    }

    painter->restore();
}

QWidget *RichTextDelegate::createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const
{
    return nullptr;
}

QSize RichTextDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(50, size.height()));

    if (index.column()) {
        size.setWidth(std::max(150, size.width()));
    }
    return size;
}

void RichTextDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->showDecorationSelected = true;
    bool ok = false;
    if (index.data(Qt::TextAlignmentRole).isValid()) {
        uint value = index.data(Qt::TextAlignmentRole).toUInt(&ok);
        option->displayAlignment = static_cast<Qt::Alignment>(value);
    }

    if (!ok)
        option->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    option->textElideMode = Qt::ElideRight;
    option->features = QStyleOptionViewItem::HasDisplay;
    if (index.row() % 2 == 0)
        option->features |= QStyleOptionViewItem::Alternate;
    if (index.data(Qt::DisplayRole).isValid())
        option->text = index.data().toString();
}

void RichTextDelegate::getDocFromLst(QDomDocument &doc, const QStringList &lst)const
{
    QDomElement table = doc.createElement("table");
    table.setAttribute("border", "0");
    table.setAttribute("width", "100%");
    table.setAttribute("cellpadding", "3");

    foreach (auto kv, lst) {
        QStringList keyValue = kv.split(":");
        if (keyValue.size() != 2) {
            return;
        }
        QPair<QString, QString> pair;
        pair.first = keyValue[0];
        pair.second = keyValue[1];

        // 添加一行
//        qDebug() << pair.first << ":" << strLenth(pair.first);
        addRow(doc, table, pair);
    }
    // 添加该表格到doc
    doc.appendChild(table);
}

void RichTextDelegate::roundStr(QString &str)const
{
    int qLenth = str.length();
    int cLenth = str.toLocal8Bit().length();
    int chLenth = (cLenth - qLenth) / 2;
    cLenth = cLenth - chLenth;
    str = str + QString(14 - cLenth, QChar(' '));
}

void RichTextDelegate::addRow(QDomDocument &doc, QDomElement &table, const QPair<QString, QString> &pair)const
{
    QDomElement tr = doc.createElement("tr");

    // 该行的第一列
    QDomElement td = doc.createElement("td");
    td.setAttribute("width", "50%");
    td.setAttribute("style", "text-align:left;");
//        td.setAttribute("style", "text-indent:25px;");
    td.setAttribute("style", "font-weight:bold;");
    QString nt = pair.first.isEmpty() ? "" : pair.first + ":";
    roundStr(nt);
    QDomText nameText = doc.createTextNode(nt);
    td.appendChild(nameText);
    tr.appendChild(td);

    // 该行的第二列
    // 如果该列的内容很多则分行显示
    QStringList strList = pair.second.split("  /  \t\t");
    if (strList.size() > 2) {

        QStringList::iterator it = strList.begin();
        QDomElement td2 = doc.createElement("td");
//        td2.setAttribute("width", "50%");
        QDomText valueText;
        valueText = doc.createTextNode(*it);
        td2.appendChild(valueText);
        tr.appendChild(td2);
        table.appendChild(tr);
        ++it;

        for (; it != strList.end(); ++it) {
            QPair<QString, QString> tempPair;
            tempPair.first = "";
            tempPair.second = *it;
            addRow(doc, table, tempPair);
        }
    } else {
        QDomElement td2 = doc.createElement("td");
//        td2.setAttribute("width", "70%");
        QDomText valueText;
        valueText = doc.createTextNode(pair.second);
        td2.appendChild(valueText);
        tr.appendChild(td2);
        table.appendChild(tr);
    }
}
