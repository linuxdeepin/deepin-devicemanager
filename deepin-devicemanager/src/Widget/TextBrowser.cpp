// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "TextBrowser.h"
#include "DeviceNetwork.h"
#include "DeviceInfo.h"
#include "DeviceInput.h"
#include "DBusWakeupInterface.h"
#include "DDLog.h"

// Dtk头文件
#include <DGuiApplicationHelper>
#include <DPaletteHelper>
#include <DApplication>
#include <DFontSizeManager>
#include <DMenu>

// Qt库文件
#include <QTextDocumentFragment>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QScrollBar>
#include <QAction>
#include <QLoggingCategory>

using namespace DDLog;

TextBrowser::TextBrowser(QWidget *parent)
    : DTextBrowser(parent)
    , m_ShowOtherInfo(false)
    , mp_Refresh(new QAction(tr("Refresh"), this))
    , mp_Export(new QAction(tr("Export"), this))
    , mp_Copy(new QAction(tr("Copy"), this))
    , mp_Menu(new DMenu(this))
    , m_IsMenuShowing(false)
{
    qCDebug(appLog) << "TextBrowser instance created";
    DFontSizeManager::instance()->bind(this, DFontSizeManager::SizeType(DFontSizeManager::T7));
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 初始化右键菜单
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotShowMenu(const QPoint &)));
    connect(mp_Refresh, &QAction::triggered, this, &TextBrowser::slotActionRefresh);
    connect(mp_Export, &QAction::triggered, this, &TextBrowser::slotActionExport);
    connect(mp_Copy, &QAction::triggered, this, &TextBrowser::slotActionCopy);
    connect(mp_Menu, &DMenu::aboutToHide, this, &TextBrowser::slotCloseMenu);
}

void TextBrowser::showDeviceInfo(DeviceBaseInfo *info)
{
    qCDebug(appLog) << "Showing device info for:" << (info ? info->name() : "null");

    mp_Info = info;
    // 先清空内容 *************************************************
    clear();
    if (!info) {
        qCWarning(appLog) << "Info is null, clearing browser";
        return;
    }

    // 显示设备的信息 *************************************************
    QDomDocument doc;

    // 添加子标题
    domTitleInfo(doc, mp_Info);

    // 添加一个表格
    if (mp_Info->enable() && mp_Info->available()) {
        const QList<QPair<QString, QString>> &baseInfo = info->getBaseTranslationAttribs();
        domTableInfo(doc, baseInfo);
    }


    // 将设备信息显示到TextBrowser
    setHtml(doc.toString().replace("<h3>", "<h3>&nbsp;"));
}

void TextBrowser::updateInfo()
{
    qCDebug(appLog) << "Updating device info";

    // 先清空内容 *************************************************
    clear();
    if (!mp_Info) {
        qCWarning(appLog) << "Info is null, clearing browser";
        return;
    }

    // 显示设备的信息 *************************************************
    QDomDocument doc;

    // 添加子标题
    domTitleInfo(doc, mp_Info);

    // 添加一个表格
    if (mp_Info->enable() && mp_Info->available()) {
        const QList<QPair<QString, QString>> &baseInfo = mp_Info->getBaseTranslationAttribs();
        domTableInfo(doc, baseInfo);
        if (m_ShowOtherInfo) {
            qCDebug(appLog) << "Showing other info";
            const QList<QPair<QString, QString>> &otherInfo = mp_Info->getOtherTranslationAttribs();
            domTableInfo(doc, otherInfo);
        }
    }
    //将设备信息显示到TextBrowser
    setHtml(doc.toString().replace("<h3>", "<h3>&nbsp;"));
}

EnableDeviceStatus TextBrowser::setDeviceEnabled(bool enable)
{
    qCDebug(appLog) << "Setting device enabled:" << enable;

    if (!mp_Info) {
        qCWarning(appLog) << "Info is null, cannot set device enabled";
        return EDS_Cancle;
    }
    return mp_Info->setEnable(enable);
}

void TextBrowser::setWakeupMachine(bool wakeup)
{
    qCDebug(appLog) << "Setting wakeup machine:" << wakeup;

    // 键盘鼠标唤醒机器
    DeviceInput *input = qobject_cast<DeviceInput*>(mp_Info);
    if(input && !input->wakeupID().isEmpty() && !input->sysPath().isEmpty()){
        qCDebug(appLog) << "Setting wakeup for input device";
        DBusWakeupInterface::getInstance()->setWakeupMachine(input->wakeupID(),input->sysPath(),wakeup, input->name());
    }

    // 网卡的远程唤醒
    DeviceNetwork* network = qobject_cast<DeviceNetwork*>(mp_Info);
    if(network && !network->logicalName().isEmpty()){
        qCDebug(appLog) << "Setting wakeup for network device";
        DBusWakeupInterface::getInstance()->setNetworkWakeup(network->logicalName(),wakeup);
    }
}

void TextBrowser::updateShowOtherInfo()
{
    // qCDebug(appLog) << "Updating show other info";
    m_ShowOtherInfo = !m_ShowOtherInfo;
}

void TextBrowser::fillClipboard()
{
    qCDebug(appLog) << "Filling clipboard with selected text";

    QString str = QTextEdit::textCursor().selectedText();
    if (str.isEmpty()) {
        qCDebug(appLog) << "No text selected";
        return;
    }
    QClipboard *clipboard = DApplication::clipboard();
    QTextDocumentFragment frag = textCursor().selection();
    clipboard->setText(frag.toPlainText().trimmed() + "\n");
}

void TextBrowser::paintEvent(QPaintEvent *event)
{
    // qCDebug(appLog) << "Painting event";
    DPalette pa = DPaletteHelper::instance()->palette(this);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    DPaletteHelper::instance()->setPalette(this, pa);

    int height = int(document()->size().height());
    setFixedHeight(height);

    return DTextBrowser::paintEvent(event);
}

void TextBrowser::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "Key press event";
    if (event->key() == Qt::Key_C) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        if (modifiers != Qt::NoModifier) {
            if (modifiers.testFlag(Qt::ControlModifier)) {
                qCDebug(appLog) << "Ctrl+C pressed, copying text";
                fillClipboard();
                return;
            }
        }
    }
    Dtk::Widget::DTextBrowser::keyPressEvent(event);
}

void TextBrowser::focusInEvent(QFocusEvent *e)
{
    // qCDebug(appLog) << "Focus in event";
    QTextBrowser::focusInEvent(e);
}

void TextBrowser::focusOutEvent(QFocusEvent *e)
{
    // qCDebug(appLog) << "Focus out event";
    if(m_IsMenuShowing){
        DTextBrowser::focusOutEvent(e);
    }
    else {
        qCDebug(appLog) << "Focus lost, simulating mouse press";
        // 模拟单击效果，当焦点失去的时候刷新界面选中效果
        QMouseEvent pressEvent(QEvent::MouseButtonPress, this->mapFromGlobal(QCursor::pos()), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QTextBrowser::mousePressEvent(&pressEvent);
    }
}

void TextBrowser::slotCloseMenu()
{
    // qCDebug(appLog) << "Menu closed";
    m_IsMenuShowing = false;
}

void TextBrowser::slotShowMenu(const QPoint &)
{
    // qCDebug(appLog) << "Menu shown";
    m_IsMenuShowing = true;
    // 右键菜单
    mp_Menu->clear();
    mp_Menu->addAction(mp_Copy);
    mp_Menu->addAction(mp_Refresh);
    mp_Menu->addAction(mp_Export);
    mp_Menu->exec(QCursor::pos());
}
void TextBrowser::slotActionRefresh()
{
    qCDebug(appLog) << "Refresh action triggered";

    emit refreshInfo();
}
void TextBrowser::slotActionExport()
{
    qCDebug(appLog) << "Export action triggered";

    emit exportInfo();
}
void TextBrowser::slotActionCopy()
{
    qCDebug(appLog) << "Copy action triggered";

    QString str = QTextEdit::textCursor().selectedText();
    if (str.isEmpty()) {
        qCDebug(appLog) << "No text selected, copying all info";
        emit copyAllInfo();
    } else {
        qCDebug(appLog) << "Copying selected text";
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
    qCDebug(appLog) << "Creating title info";
    if (!info) {
        qCWarning(appLog) << "Info is null, cannot create title";
        return;
    }
    QString title = info->subTitle();
    if (!title.isEmpty()) {
        QDomElement h3 = doc.createElement("h3");

        if (!info->enable()) {
            qCDebug(appLog) << "Device is disabled, adding disable tag to title";
            title = "(" + tr("Disable") + ")" + title;
            h3.setAttribute("style", "text-indent:2px;text-align:left;font-weight:504;padding:10px;color:#FF5736;");
        } else if (!info->available()) {
            qCDebug(appLog) << "Device is unavailable, adding unavailable tag to title";
            DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
            DPalette palette = dAppHelper->applicationPalette();
            QColor color = palette.color(DPalette::Disabled, DPalette::PlaceholderText);
            QRgb rgb = qRgb(color.red(), color.green(), color.blue());
            QString rgbs = QString::number(rgb, 16);
            title = "(" + tr("Unavailable") + ")" + title;
            QString css = QString("text-indent:2px;text-align:left;font-weight:504;padding:10px;color:#%1;").arg(rgbs);
            h3.setAttribute("style", css);
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
    qCDebug(appLog) << "Creating table info";
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
