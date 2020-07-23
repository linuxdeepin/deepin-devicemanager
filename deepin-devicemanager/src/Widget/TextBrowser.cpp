#include "TextBrowser.h"

#include <QDebug>
#include <DApplicationHelper>
#include <DApplication>
#include <DFontSizeManager>
#include <QTextDocumentFragment>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QScrollBar>

#include "DeviceManager/DeviceInfo.h"


TextBrowser::TextBrowser(QWidget *parent)
    : DTextBrowser(parent)
{
    DFontSizeManager::instance()->bind(this, DFontSizeManager::SizeType(DFontSizeManager::T7));
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void TextBrowser::showDeviceInfo(DeviceBaseInfo *info)
{
    // 先清空内容 *************************************************
    clear();
    if (!info) {
        return;
    }

    // 显示设备的信息 *************************************************
    QDomDocument doc;
    const QList<QPair<QString, QString>> &baseInfo = info->getBaseAttribs();
    if (baseInfo.size() < 1) {return;}

    // 添加一个表格
    QDomElement table = doc.createElement("table");
    table.setAttribute("border", "0");
    table.setAttribute("width", "100%");
    table.setAttribute("cellpadding", "3");

    foreach (auto pair, baseInfo) {
        // 添加一行
        QDomElement tr = doc.createElement("tr");

        // 该行的第一列
        QDomElement td = doc.createElement("td");
        td.setAttribute("width", "20%");
        td.setAttribute("style", "text-align:left;");
        td.setAttribute("style", "text-indent:25px;");

        QDomText nameText = doc.createTextNode(pair.first + ":");
        td.appendChild(nameText);
        tr.appendChild(td);

        // 改行的第二列
        QDomElement td2 = doc.createElement("td");
        td2.setAttribute("width", "80%");
        QDomText valueText;
        valueText = doc.createTextNode(pair.second);
        td2.appendChild(valueText);
        tr.appendChild(td2);

        // 将改行添加到表格
        table.appendChild(tr);
    }

    // 添加该表格到doc
    doc.appendChild(table);

    // 将设备信息显示到TextBrowser
    setHtml(doc.toString().replace("<h3>", "<h3>&nbsp;"));
}

void TextBrowser::fillClipboard()
{
    QString str = QTextEdit::textCursor().selectedText();
    if (str.isEmpty()) {
        return;
    }
    QClipboard *clipboard = DApplication::clipboard();
    QTextDocumentFragment frag = textCursor().selection();
    clipboard->setText(frag.toPlainText().trimmed() + "\n");
}

void TextBrowser::paintEvent(QPaintEvent *event)
{
    DPalette pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    DApplicationHelper::instance()->setPalette(this, pa);


    int height = int(document()->size().height());
    setFixedHeight(height);

    return DTextBrowser::paintEvent(event);
}

void TextBrowser::contextMenuEvent(QContextMenuEvent *event)
{
//    QMenu *standMenu = new QMenu(this);

//    QString str = QTextEdit::textCursor().selectedText();

//    QAction *copyAction = new QAction(QIcon::fromTheme("edit-copy"), tr("Copy (C)"), this);
//    if (str.isEmpty()) {
//        copyAction->setDisabled(true);
//    } else {
//        connect(copyAction, &QAction::triggered, this, &TextBrowser::fillClipboard);
//    }
//    standMenu->addAction(copyAction);
//    standMenu->addSeparator();

//    standMenu->addAction(m_pDeviceInfoWidget->refreshAction_);
//    standMenu->addAction(m_pDeviceInfoWidget->exportAction_);

//    standMenu->exec(event->globalPos());

//    delete standMenu;
}

void TextBrowser::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_C) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        if (modifiers != Qt::NoModifier) {
            if (modifiers.testFlag(Qt::ControlModifier)) {
                fillClipboard();
                return;
            }
        }
    }
    Dtk::Widget::DTextBrowser::keyPressEvent(event);
}

void TextBrowser::wheelEvent(QWheelEvent *event)
{

}
