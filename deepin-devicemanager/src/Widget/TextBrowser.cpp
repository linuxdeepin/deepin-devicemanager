// 项目自身文件
#include "TextBrowser.h"

// Dtk头文件
#include <DApplicationHelper>
#include <DApplication>
#include <DFontSizeManager>
#include <DMenu>

// Qt库文件
#include <QTextDocumentFragment>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QScrollBar>
#include <QAction>
#include <QDebug>

// 其它头文件
#include "DeviceManager/DeviceInfo.h"

TextBrowser::TextBrowser(QWidget *parent)
    : DTextBrowser(parent)
    , m_ShowOtherInfo(false)
    , mp_Refresh(new QAction(tr("Refresh"), this))
    , mp_Export(new QAction(tr("Export"), this))
    , mp_Copy(new QAction(tr("Copy"), this))
    , mp_Menu(new DMenu(this))
{
    DFontSizeManager::instance()->bind(this, DFontSizeManager::SizeType(DFontSizeManager::T7));
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 初始化右键菜单
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &TextBrowser::slotActionRefresh);
    connect(mp_Export, &QAction::triggered, this, &TextBrowser::slotActionExport);
    connect(mp_Copy, &QAction::triggered, this, &TextBrowser::slotActionCopy);
}

void TextBrowser::showDeviceInfo(DeviceBaseInfo *info)
{
    mp_Info = info;
    // 先清空内容 *************************************************
    clear();
    if (!info) {
        return;
    }

    // 显示设备的信息 *************************************************
    QDomDocument doc;

    // 添加子标题
    domTitleInfo(doc, mp_Info);

    // 添加一个表格
    if (mp_Info->enable()) {
        const QList<QPair<QString, QString>> &baseInfo = info->getBaseAttribs();
        domTableInfo(doc, baseInfo);
    }


    // 将设备信息显示到TextBrowser
    setHtml(doc.toString().replace("<h3>", "<h3>&nbsp;"));
}

void TextBrowser::updateInfo()
{
    // 先清空内容 *************************************************
    clear();
    if (!mp_Info) {
        return;
    }

    // 显示设备的信息 *************************************************
    QDomDocument doc;

    // 添加子标题
    domTitleInfo(doc, mp_Info);

    // 添加一个表格
    if (mp_Info->enable()) {
        const QList<QPair<QString, QString>> &baseInfo = mp_Info->getBaseAttribs();
        domTableInfo(doc, baseInfo);
        if (m_ShowOtherInfo) {
            const QList<QPair<QString, QString>> &otherInfo = mp_Info->getOtherAttribs();
            domTableInfo(doc, otherInfo);
        }
    }
    //将设备信息显示到TextBrowser
    setHtml(doc.toString().replace("<h3>", "<h3>&nbsp;"));
}

EnableDeviceStatus TextBrowser::setDeviceEnabled(bool enable)
{
    if (!mp_Info) {
        return EDS_Cancle;
    }
    return mp_Info->setEnable(enable);
}

void TextBrowser::updateShowOtherInfo()
{
    m_ShowOtherInfo = !m_ShowOtherInfo;
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

void TextBrowser::focusInEvent(QFocusEvent *e)
{
//    setTextInteractionFlags(Qt::TextBrowserInteraction);
    QTextBrowser::focusInEvent(e);
}
void TextBrowser::focusOutEvent(QFocusEvent *e)
{
    if (!this->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        // 先清空内容 *************************************************
        clear();
        if (!mp_Info) {
            return;
        }
        // 显示设备的信息 *************************************************
        QDomDocument doc;

        // 添加子标题
        domTitleInfo(doc, mp_Info);

        // 添加一个表格
        if (mp_Info->enable()) {
            const QList<QPair<QString, QString>> &baseInfo = mp_Info->getBaseAttribs();
            domTableInfo(doc, baseInfo);
            if (m_ShowOtherInfo) {
                const QList<QPair<QString, QString>> &otherInfo = mp_Info->getOtherAttribs();
                domTableInfo(doc, otherInfo);
            }
        }

        // 将设备信息显示到TextBrowser
        setHtml(doc.toString().replace("<h3>", "<h3>&nbsp;"));
    }
    QTextBrowser::focusOutEvent(e);
}

void TextBrowser::slotShowMenu(const QPoint &)
{
//    QString str = QTextEdit::textCursor().selectedText();
//    if (str.isEmpty()) {
//        mp_Copy->setText(tr("Copy (C)"));
//    } else {
//        mp_Copy->setText(tr("Copy Selected Text"));
//    }

    mp_Menu->clear();
    mp_Menu->addAction(mp_Copy);
    mp_Menu->addAction(mp_Refresh);
    mp_Menu->addAction(mp_Export);
    mp_Menu->exec(QCursor::pos());
}
void TextBrowser::slotActionRefresh()
{
    emit refreshInfo();
}
void TextBrowser::slotActionExport()
{
    emit exportInfo();
}
void TextBrowser::slotActionCopy()
{
    QString str = QTextEdit::textCursor().selectedText();
    if (str.isEmpty()) {
        emit copyAllInfo();
    } else {
//        mp_Copy->setText(tr("Copy Selected Text"));
        QClipboard *clipboard = DApplication::clipboard();
        clipboard->setText(str);
    }

//    DUtil::DNotifySender sender(tr("Successfully copied device information"));
//    sender.appIcon("deepin-devicemanager");
//    sender.timeOut(2000);
//    sender.call();
}


void TextBrowser::domTitleInfo(QDomDocument &doc, DeviceBaseInfo *info)
{
    if (!info) {
        return;
    }
    QString title = info->subTitle();
    if (!title.isEmpty()) {
        QDomElement h3 = doc.createElement("h3");

        if (!info->enable()) {
            title = "(" + tr("Disable") + ")" + title;
            h3.setAttribute("style", "text-indent:2px;text-align:left;font-weight:504;padding:10px;color:#FF5736;");
        } else {
            h3.setAttribute("style", "text-indent:2px;text-align:left;font-weight:504;padding:10px;");
        }
        QDomText valueText = doc.createTextNode(title);
        h3.appendChild(valueText);
        doc.appendChild(h3);
    }
}

void TextBrowser::domTableInfo(QDomDocument &doc, const QList<QPair<QString, QString>> &info)
{
    QDomElement table = doc.createElement("table");
    table.setAttribute("border", "0");
    table.setAttribute("width", "100%");
//    table.setAttribute("cellpadding", "3");

    foreach (auto pair, info) {
        // 添加一行
        QDomElement tr = doc.createElement("tr");

        // 该行的第一列
        QDomElement td = doc.createElement("th");
        td.setAttribute("width", "20%");
        td.setAttribute("style", "text-indent:0px;text-align:left;font-weight:504;");
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
}
