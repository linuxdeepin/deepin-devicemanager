#include "TextBrowser.h"
#include <QDebug>
#include<DApplicationHelper>
#include<DApplication>
#include<QTextDocumentFragment>
#include<QClipboard>
#include<QContextMenuEvent>
#include "deviceinfowidgetbase.h"
TextBrowser::TextBrowser(DeviceInfoWidgetBase *parent)
    : DTextBrowser (parent), m_pDeviceInfoWidget(parent)
{

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
    return DTextBrowser::paintEvent(event);
}

void TextBrowser::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *standMenu = new QMenu(this);

    QString str = QTextEdit::textCursor().selectedText();

    QAction *copyAction = new QAction(QIcon::fromTheme("edit-copy"), tr("Copy (C)"), this);
    if (str.isEmpty()) {
        copyAction->setDisabled(true);
    } else {
        connect(copyAction, &QAction::triggered, this, &TextBrowser::fillClipboard);
    }
    standMenu->addAction(copyAction);
    standMenu->addSeparator();

    standMenu->addAction(m_pDeviceInfoWidget->refreshAction_);
    standMenu->addAction(m_pDeviceInfoWidget->exportAction_);

    standMenu->exec(event->globalPos());

    delete standMenu;
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
