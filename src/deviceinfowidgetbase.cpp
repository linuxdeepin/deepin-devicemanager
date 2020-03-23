/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "deviceinfowidgetbase.h"
#include "bottomroundframe.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include "DScrollBar"
#include <QSizePolicy>
#include "DFileDialog"
#include <QTextStream>
#include <QFile>
#include "mainwindow.h"
#include <QDate>
#include "DApplication"
#include "DApplicationHelper"
#include "table.h"
#include "DTitlebar"
#include "commondefine.h"
#include <QSizePolicy>
#include "cloumnwidget.h"
#include <QSet>
#include "logtreeview.h"
#include "QShortcut"
#include <QTextDocumentFragment>
#include "DFontSizeManager"
#include "DStandardItem"
#include "deviceinfoparser.h"

DWIDGET_USE_NAMESPACE

bool DeviceInfoWidgetBase::isFontInit_ = false;
QFont DeviceInfoWidgetBase::titleFont_;
QFont DeviceInfoWidgetBase::subTitleFont_;
QFont DeviceInfoWidgetBase::infoFont_;
QFont DeviceInfoWidgetBase::labelFont_;
QFont DeviceInfoWidgetBase::tableHeaderFont_;
QFont DeviceInfoWidgetBase::tableContentFont_;
QFont DeviceInfoWidgetBase::centralFont_;

bool DeviceInfoWidgetBase::isPaletteInit_ = false;
DPalette DeviceInfoWidgetBase::defaultPa_;


int currentXlsRow_ = 1;

DeivceInfoBrower::DeivceInfoBrower( DeviceInfoWidgetBase* parent ): DTextBrowser(parent), deviceInfoWidget_(parent)
{

}

void DeivceInfoBrower::fillClipboard()
{
    QString str = QTextEdit::textCursor().selectedText();
    if(str.isEmpty()){
        return;
    }
    QClipboard *clipboard = DApplication::clipboard();
    QTextDocumentFragment frag = textCursor().selection();
    clipboard->setText(frag.toPlainText().trimmed()+"\n");
}

void DeivceInfoBrower::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu* standMenu = new QMenu(this);

    QString str = QTextEdit::textCursor().selectedText();

    QAction* copyAction = new QAction(QIcon::fromTheme("edit-copy"), tr("Copy (C)"), this);
    if(str.isEmpty())
    {
        copyAction->setDisabled(true);
    }
    else {
        connect(copyAction, &QAction::triggered,this,&DeivceInfoBrower::fillClipboard);
    }
    standMenu->addAction(copyAction);
    standMenu->addSeparator();

    standMenu->addAction(deviceInfoWidget_->refreshAction_);
    standMenu->addAction(deviceInfoWidget_->exportAction_);

    standMenu->exec(event->globalPos());

    delete standMenu;
}

void DeivceInfoBrower::paintEvent(QPaintEvent *event)
{
//    QPainter painter(viewport());
//    painter.save();
//    painter.setRenderHints(QPainter::Antialiasing);
//    painter.setOpacity(1);
//    painter.setClipping(true);

//    QWidget *wnd = DApplication::activeWindow();
//    DPalette::ColorGroup cg;
//    if (!wnd) {
//        cg = DPalette::Inactive;
//    } else {
//        cg = DPalette::Active;
//    }

//    //    auto style = dynamic_cast<DStyle *>(DApplication::style());
//    auto *dAppHelper = DApplicationHelper::instance();
//    auto palette = dAppHelper->applicationPalette();

//    QBrush bgBrush(palette.color(cg, DPalette::Background));

//    QStyleOptionFrame option;
//    initStyleOption(&option);

//    QRect rect = viewport()->rect();
//    QRectF clipRect(rect.x(), rect.y() - rect.height(), rect.width(), rect.height() * 2);
//    QRectF subRect(rect.x(), rect.y() - rect.height(), rect.width(), rect.height());
//    QPainterPath clipPath, subPath;

//    clipPath.addRoundedRect(clipRect, 8, 8);
//    subPath.addRect(subRect);
//    clipPath = clipPath.subtracted(subPath);
//    clipPath.addRect(rect);

//    painter.fillPath(clipPath, bgBrush);

//    painter.restore();
    return Dtk::Widget::DTextBrowser::paintEvent(event);
}

void DeivceInfoBrower::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_C)
    {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        if (modifiers != Qt::NoModifier)
        {
            if ( modifiers.testFlag(Qt::ControlModifier))
            {
                fillClipboard();
                return;
            }
        }
    }
    Dtk::Widget::DTextBrowser::keyPressEvent(event);
}

DeviceInfoWidgetBase::DeviceInfoWidgetBase(DWidget *parent_, const QString& deviceName) : DWidget(parent_)
{
    overviewInfo_.name = deviceName;

    setContentsMargins( DeviceWidgetMargin_, DeviceWidgetMargin_, DeviceWidgetMargin_, DeviceWidgetMargin_);

    vLayout_ = new QVBoxLayout;
    vLayout_->setSpacing(2);
    vLayout_->setMargin(0);

    setLayout(vLayout_);

    initContextMenu();

    initFont();

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &DeviceInfoWidgetBase::changeTheme );
}

void DeviceInfoWidgetBase::initFont()
{
    if( isFontInit_ == true )
    {
        return;
    }

    isFontInit_ = true;

    QLabel* nameLabel = new DLabel("", this);

    DFontSizeManager::instance()->bind( nameLabel, DFontSizeManager::T5);

    titleFont_ = nameLabel->font();
    titleFont_.setWeight(QFont::DemiBold);

    DFontSizeManager::instance()->bind( nameLabel, DFontSizeManager::T5);
    subTitleFont_ = nameLabel->font();
    //subTitleFont_.setPixelSize(14);
    subTitleFont_.setWeight(QFont::DemiBold);

    DFontSizeManager::instance()->bind( nameLabel, DFontSizeManager::T5);
    infoFont_ = nameLabel->font();
    //infoFont_.setPixelSize(14);
    infoFont_.setWeight(QFont::Normal);

    DFontSizeManager::instance()->bind( nameLabel, DFontSizeManager::T8);
    labelFont_ = nameLabel->font();
    //labelFont_.setPixelSize(12);
    labelFont_.setWeight(QFont::Normal);

    DFontSizeManager::instance()->bind( nameLabel, DFontSizeManager::T5);
    tableHeaderFont_ = nameLabel->font();
    tableHeaderFont_.setWeight(QFont::Medium);
    //tableHeaderFont_.setPixelSize(14);

    DFontSizeManager::instance()->bind( nameLabel, DFontSizeManager::T8);
    tableContentFont_ = nameLabel->font();
    tableContentFont_.setWeight(QFont::Medium);
    //tableContentFont_.setPixelSize(12);

    DFontSizeManager::instance()->bind( nameLabel, DFontSizeManager::T4);
    centralFont_ = nameLabel->font();
    centralFont_.setWeight(QFont::Normal);
    //centralFont_.setPixelSize(20);

    delete nameLabel;
}

bool DeviceInfoWidgetBase::getOverViewInfo(ArticleStruct& info)
{
    info = overviewInfo_;

    return info.value.isEmpty() == false;
}

void DeviceInfoWidgetBase::initContextMenu()
{
    contextMenu_ = new DMenu(this);

    contextMenu_->setAutoFillBackground(true);

    //contextMenu_->setMinimumWidth(contextMenuWidth_);

    refreshAction_ = new QAction( QIcon::fromTheme("view-refresh"), tr("Refresh (F5)"), this);

    connect(refreshAction_, &QAction::triggered, \
            [this]()
            {
                MainWindow* mainWindow = dynamic_cast<MainWindow*>(parent()->parent()->parent());
                if(mainWindow)
                {
                    mainWindow->refresh();
                }
            }
    );
    contextMenu_->addAction(refreshAction_);


    //exportAction_ = new QAction( QIcon::fromTheme("document-save-as"), tr("Export (E)")  );
    exportAction_ = new QAction( QIcon::fromTheme("document-new"), tr("Export (E)"));

    QShortcut *temp = new QShortcut(this);
    temp->setKey(QKeySequence("ctrl+e"));
    connect( temp, &QShortcut::activated, this, &DeviceInfoWidgetBase::onExportToFile);

    connect(exportAction_, &QAction::triggered, this, &DeviceInfoWidgetBase::onExportToFile);
    contextMenu_->addAction(exportAction_);
}

void DeviceInfoWidgetBase::setCentralInfo(const QString& info)
{
    firstShow_ = false;

    if(titleInfo_ == nullptr)
    {
        titleInfo_ = new DeviceInfo;
    }

    if(tableWidget_ != nullptr)
    {
        delete tableWidget_;
        tableWidget_ = nullptr;
    }

    if(downFrame_)
    {
        delete downFrame_;
        downFrame_ = nullptr;
    }

    downFrame_ = new DFrame(this);
    changeTheme();

    QVBoxLayout* ly = new QVBoxLayout;

    downFrame_->setLayout(ly);

    if( false == info.isEmpty() )
    {
        titleInfo_->title_ = info;
        auto label = new DLabel(titleInfo_->title_, this );
        label->setFont(centralFont_);
        DFontSizeManager::instance()->bind(label, DFontSizeManager::T4);

        auto hLayout = new QHBoxLayout;
        hLayout->addStretch(1);
        hLayout->addWidget(label);
        hLayout->addStretch(1);

        ly->addStretch(1);
        ly->addLayout(hLayout);
        ly->addStretch(1);
    }

    vLayout_->addWidget( downFrame_ );
}

void DeviceInfoWidgetBase::toHtmlString(QDomDocument& doc, const DeviceInfo& di )
{
    if( di.title_.isEmpty() == false)
    {
        QDomElement h3 = doc.createElement("h3");
        QDomText valueText = doc.createTextNode(di.title_);
        h3.appendChild(valueText);
        doc.appendChild(h3);
    }

    if( di.articles_.size() < 1 )
    {
        return;
    }

    QDomElement table = doc.createElement("table");
    table.setAttribute("border", "0");
    table.setAttribute("width", "100%");
    table.setAttribute("cellpadding", "3");

    foreach( auto article, di.articles_ )
    {
        if(article.isValid() == false)
        {
            continue;
        }

        QDomElement tr = doc.createElement("tr");

        QDomElement td = doc.createElement("td");
        td.setAttribute("width", "15%");
        td.setAttribute("style", "text-align:left;");
        QDomText nameText = doc.createTextNode(article.name + ":");
        td.appendChild(nameText);
        tr.appendChild(td);

        QDomElement td2 = doc.createElement("td");
        td2.setAttribute("width", "85%");

        QDomText valueText;
        if(article.externalLinks == false)
        {
            valueText = doc.createTextNode(article.value);
            td2.appendChild(valueText);
        }
        else
        {
             QDomElement style = doc.createElement("style");
             QDomText decoration = doc.createTextNode("a {text-decoration: none; }");
             style.appendChild(decoration);
             td2.appendChild(style);

             QDomElement a = doc.createElement("a");
             a.setAttribute("href", DeviceInfoParser::Instance().getHomeUrl() );
             QDomText  href = doc.createTextNode(DeviceInfoParser::Instance().getLsbRelease() + " ");
             a.appendChild(href);
             td2.appendChild(a);

             QString osInfo = DeviceInfoParser::Instance().getOsInfo();
             valueText = doc.createTextNode(osInfo);
             td2.appendChild(valueText);
        }

        tr.appendChild(td2);

        table.appendChild(tr);
    }

    doc.appendChild(table);
}

void DeviceInfoWidgetBase::addInfo(const QString &title, const QList<ArticleStruct> &articles)
{
    if(titleInfo_ == nullptr)
    {
        titleInfo_ = new DeviceInfo;
    }

    titleInfo_->font_ = infoFont_;

    if( title.isEmpty()== false)
    {
        titleInfo_->title_ = title;
    }
    foreach(auto article, articles)
    {
        ArticleStruct inserArt(article);
        titleInfo_->articles_.push_back(inserArt);
    }

}

void DeviceInfoWidgetBase::addSubInfo(const QString& subTitle, const QList<ArticleStruct>& articles)
{
    DeviceInfo di;

    di.font_ = labelFont_;

    if(subTitle.isEmpty() == false)
    {
        di.title_ = subTitle;
    }
    foreach(auto article, articles)
    {
        di.articles_.push_back(article);
    }
    deviceInfos_.push_back(di);
}

void DeviceInfoWidgetBase::addTable(const QStringList& headers, const QList<QStringList>& contentsList)
{
    if(contentsList.size() == 0)
    {
        return;
    }

    if(tableWidget_ == nullptr)
    {
        tableWidget_ = new LogTreeView(this);
        DFontSizeManager::instance()->bind( tableWidget_, DFontSizeManager::T8);

        tableWidget_->setSortingEnabled(true);

        tableWidget_->setFixedHeight(TableViewRowHeight_*5 + 4/*+ tableWidget_->verticalHeader()->height()+3*/);

        tableWidget_->m_headerDelegate->setDefaultSectionSize(TableViewRowHeight_+4);
        tableWidget_->m_headerDelegate->setFixedHeight(TableViewRowHeight_+4);

        DFontSizeManager::instance()->bind( tableWidget_->m_headerDelegate, DFontSizeManager::T6);

        QStringList translaterHeaders;
        foreach(auto header, headers)
        {
            translaterHeaders.push_back(header);
        }
        tableWidget_->m_pModel->setHorizontalHeaderLabels(translaterHeaders);

        connect(tableWidget_, &DTreeView::clicked, this, &DeviceInfoWidgetBase::OnCurrentItemClicked);
    }

    //vLayout_->insertWidget(0, tableWidget_);
    QVBoxLayout* tableLay = new QVBoxLayout;
    tableLay->setContentsMargins(1,1,1,1);
    tableLay->addWidget(tableWidget_);

    vLayout_->insertLayout(0,tableLay);

    for(int i = 0; i < contentsList.size(); ++i)
    {
        const QStringList& contents = contentsList[i];
        for(int j = 0; j < contents.size(); ++j )
        {
            QString strContent = contents[j];
            if( strContent.isEmpty() == true)
            {
                strContent = "--";
            }
            if (j == 3){
                int brackets = strContent.indexOf('(');
                QString strContent1 = strContent.mid(0,brackets);
                if (strContent1.contains("GiB")){
                    strContent1.replace("GiB","GB");
                }
                if (strContent1.contains("MiB")){
                    strContent1.replace( "MiB", "MB" );
                }
                DStandardItem* item = new DStandardItem(strContent1);
                if(j == 0)
                {
                    item->setData( i, Qt::UserRole + 90 );
                }

                tableWidget_->m_pModel->setItem(i, j, item);
                continue;
            }

            DStandardItem* item = new DStandardItem(strContent);
            if(j == 0)
            {
                item->setData( i, Qt::UserRole + 90 );
            }

            tableWidget_->m_pModel->setItem(i, j, item);
        }
    }
}

void DeviceInfoWidgetBase::addDevice( const QString& subTitle, const QList<ArticleStruct>& articles, int deviceNumber, bool showTitle )
{
    QList<ArticleStruct> lst;
    QSet<QString> existString;
    foreach(auto article, articles)
    {
        if(existString.contains(article.name + ":" + article.value ) == false )
        {
            lst.push_back(article);
            existString.insert(article.name + ":" + article.value);
        }
    }

    if( deviceNumber < 2 )
    {
        addInfo( showTitle ? subTitle: "", lst );
    }
    else
    {
        addSubInfo( subTitle, lst );
    }
}

void DeviceInfoWidgetBase::initDownWidget()
{
    if( htmlBrower_ )
    {
        return;
    }
    downFrame_ = new DFrame(this);

    changeTheme();

    htmlBrower_ = new DeivceInfoBrower(this);
    htmlBrower_->setFrameShape(QFrame::NoFrame);
    htmlBrower_->setOpenExternalLinks(true);

    QVBoxLayout* ly = new QVBoxLayout;

    ly->setContentsMargins(10,10,0,0);
    ly->addWidget(htmlBrower_);
    downFrame_->setLayout(ly);
    vLayout_->addWidget(downFrame_);
}

QString DeviceInfoWidgetBase::getDeviceName()
{
    return overviewInfo_.name;
}

int DeviceInfoWidgetBase::maxDeviceSize(const QStringList& list1, const QStringList& list2, const QStringList& list3)
{
    int max = list1.size();

    if(max < list2.size())
    {
        max = list2.size();
    }

    if(max < list3.size())
    {
        max = list3.size();
    }

    return max;
}


void DeviceInfoWidgetBase::getContextMenu(DMenu** contextMenu)
{
    *contextMenu = contextMenu_;
}

QString DeviceInfoWidgetBase::joinArticle(QList<ArticleStruct>& articles, const QString& split)
{
    QString res;
    foreach(auto article, articles)
    {
        if(article.isValid())
        {
            if(res.isEmpty() == false)
            {
                res += split;
            }

            res +=  article.value.trimmed();
        }
    }

    return res;
}

void DeviceInfoWidgetBase::contextMenuEvent(QContextMenuEvent *event)
{
    MainWindow* mainWindow = dynamic_cast<MainWindow*>(parent()->parent()->parent());
    if(mainWindow)
    {
        refreshAction_->setDisabled(mainWindow->isRefreshing());
    }

    contextMenu_->exec(event->globalPos());
}

void DeviceInfoWidgetBase::showEvent(QShowEvent *event)
{
    if(firstShow_ == false)
    {
        return DWidget::showEvent(event);
    }

    firstShow_ = false;

    if(htmlBrower_ == nullptr){
        htmlBrower_ = new DeivceInfoBrower(this);
        htmlBrower_->setFrameShape(QFrame::NoFrame);
        htmlBrower_->setOpenExternalLinks(true);

        QHBoxLayout *htmlBroswerLayout = new QHBoxLayout;
        htmlBroswerLayout->setContentsMargins(7,19,0,20);
        htmlBroswerLayout->addWidget(htmlBrower_);

        auto bottomRoundFrame = new BottomRoundFrame;
        bottomRoundFrame->setTextBroser(htmlBrower_);
        bottomRoundFrame->setLayout(htmlBroswerLayout);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(bottomRoundFrame/*new QTextEdit*/);
        layout->setMargin(1);
        QWidget * widget = new QWidget;
        widget->setLayout(layout);
        vLayout_->addWidget(widget);
    }

    int fontSize = DFontSizeManager::T7;

    QDomDocument doc;

    if(titleInfo_)
    {
        toHtmlString(doc, *titleInfo_);
        fontSize = DFontSizeManager::T6;
    }

    DFontSizeManager::instance()->bind( htmlBrower_, DFontSizeManager::SizeType(fontSize));

    if(deviceInfos_.size() < 1)
    {
        htmlBrower_->setHtml(doc.toString().replace("<h3>", "<h3>&nbsp;"));
    }

    foreach(auto di, deviceInfos_)
    {
        textCursorList_.push_back(htmlBrower_->document()->characterCount());
        toHtmlString(doc, di);
        htmlBrower_->setHtml(doc.toString().replace("<h3>", "<h3>&nbsp;"));
    }

    DWidget::showEvent(event);
}

void DeviceInfoWidgetBase::OnCurrentItemClicked(const QModelIndex &index)
{
    QStandardItem* item = tableWidget_->m_pModel->item( index.row() );
    if(item == nullptr)
    {
        return;
    }

    int row = item->data(Qt::UserRole+90).toInt();

    QTextCursor cursor = htmlBrower_->textCursor();
    cursor.setPosition( htmlBrower_->document()->characterCount() -1 );
    htmlBrower_->setTextCursor(cursor);
    cursor.setPosition( textCursorList_.at(row) );
    htmlBrower_->setTextCursor(cursor);
}

bool DeviceInfoWidgetBase::onExportToFile()
{
   MainWindow* mainWindow = dynamic_cast<MainWindow*>(this->parent()->parent()->parent());
   if( nullptr == mainWindow )
   {
       return false;
   }

   return mainWindow->exportTo();
}

void DeviceInfoWidgetBase::changeTheme()
{
    if(downFrame_ == nullptr)
    {
        return;
    }

    DPalette pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(QPalette::Background, pa.foreground());
    downFrame_->setPalette(pa);
}

QTextStream& operator<<(QTextStream& ds, const DeviceInfo& di)
{
    if(di.title_.isEmpty() == false)
    {
        ds << di.title_ << "\n";
    }

    foreach(auto article, di.articles_)
    {
        if(article.isValid() == false){
            continue;
        }
        ds.setFieldWidth(21);
        ds.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
        ds << article.name + ": ";
        ds.setFieldWidth(0);
        if(article.externalLinks )
        {
            ds << DeviceInfoParser::Instance().getLsbRelease() + "(" +  DeviceInfoParser::Instance().getHomeUrl() + ") " + DeviceInfoParser::Instance().getOsInfo() << "\n";
        }
        else
        {
            ds << article.value << "\n";
        }
    }

    return ds;
}

QTextStream& operator<<(QTextStream& ds, LogTreeView* tableWidget)
{
    if(tableWidget == nullptr || tableWidget->m_pModel->rowCount() < 1)
    {
        return ds;
    }

    for(int col = 0; col < tableWidget->m_pModel->columnCount(); ++col)
    {
        auto item = tableWidget->m_pModel->horizontalHeaderItem(col);

        QString text = tableWidget->m_pModel->horizontalHeaderItem(col)->text();
        ds.setFieldWidth( text.size() * 1.5);
        ds.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
        if(item)
        {
             ds << text;
        }
        else
        {
            ds << "";
        }
        ds.setFieldWidth(0);
    }
    ds << "\n";


    for( int row = 0; row < tableWidget->m_pModel->rowCount(); ++row )
    {
        for( int col = 0; col < tableWidget->m_pModel->columnCount(); ++col )
        {
            QString text = tableWidget->m_pModel->item(row,col)->text();

            ds.setFieldWidth( text.size() * 1.5);
            ds.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
            ds << text;
            ds.setFieldWidth(0);
        }

        ds << "\n";
    }

    return ds;
}

bool DeviceInfoWidgetBase::exportToTxt(QFile& txtFile)
{
    QTextStream out(&txtFile);

        out <<  "[" << overviewInfo_.name << "]\n-------------------------------------------------";

        if(tableWidget_)
        {
            out << "\n";
            out << tableWidget_;
        }

        if(titleInfo_)
        {
            out << "\n";
            out << *titleInfo_;
        }

        foreach(const DeviceInfo& di, deviceInfos_)
        {
            out << "\n";
            out << di;
        }
        out << "\n";



        return true;

}

bool DeviceInfoWidgetBase::exportToTxt(const QString& txtFile)
{
    QFile file( txtFile );
    if( false == file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    exportToTxt(file);


    file.close();

    return true;
}

bool writeTabwidgetToDoc(LogTreeView* tableWidget, Docx::Document& doc)
{
    if(tableWidget == nullptr || tableWidget->m_pModel->rowCount() < 1)
    {
        return false;
    }

    Docx::Table* tab = doc.addTable(tableWidget->m_pModel->rowCount()+1, tableWidget->m_pModel->columnCount());
    //tab->setAlignment(Docx::WD_TABLE_ALIGNMENT::LEFT);

    for(int col = 0; col < tableWidget->m_pModel->columnCount(); ++col)
    {
        auto item = tableWidget->m_pModel->horizontalHeaderItem(col);
        auto cel = tab->cell(0, col);
        if(item)
        {
             cel->addText(tableWidget->m_pModel->horizontalHeaderItem(col)->text());
        }
    }

    for( int row = 0; row < tableWidget->m_pModel->rowCount(); ++row )
    {
        for( int col = 0; col < tableWidget->m_pModel->columnCount(); ++col )
        {
            auto cel = tab->cell(row+1, col);
            cel->addText(tableWidget->m_pModel->item(row,col)->text());
        }
    }

    return true;
}

bool writeDeviceInfoToDoc(const DeviceInfo& di, Docx::Document& doc)
{
    if(di.title_.isEmpty() == false)
    {
        doc.addHeading( di.title_, 4);
    }

    foreach(auto article, di.articles_)
    {
        if(!article.isValid()){
            continue;
        }
        QString name = article.name;
        QString content;
        if( article.externalLinks )
        {
            content = DeviceInfoParser::Instance().getLsbRelease() + "(" +  DeviceInfoParser::Instance().getHomeUrl() + ") " + DeviceInfoParser::Instance().getOsInfo();
        }
        else
        {
            content = article.value;
        }

        QString line;
        if(name.trimmed().isEmpty() == false || false == content.trimmed().isEmpty())
        {
            line = name + ":   " + content;
        }

        doc.addParagraph(line);
    }

    return true;
}

bool DeviceInfoWidgetBase::exportToDoc(Docx::Document& doc)
{
    doc.addHeading("[" + overviewInfo_.name + "]", 2);
    doc.addParagraph("-------------------------------------------------");

    if(tableWidget_)
    {
        writeTabwidgetToDoc(tableWidget_, doc);
        doc.addParagraph("\n");
    }

    if(titleInfo_)
    {
        writeDeviceInfoToDoc(*titleInfo_, doc);
        doc.addParagraph("\n");
    }

    foreach(auto di, deviceInfos_)
    {
        writeDeviceInfoToDoc(di, doc);
        doc.addParagraph("\n");
    }

    //doc.addParagraph("\n");
    return true;
}

bool DeviceInfoWidgetBase::exportToDoc(const QString& docFile)
{
    Docx::Document doc(":/thirdlib/docx/doc_template/docx");
    exportToDoc(doc);
    doc.save(docFile);
    return true;
}

void DeviceInfoWidgetBase::resetXlsRowCount()
{
    currentXlsRow_ = 1;
}

bool writeTabwidgetToXls(LogTreeView* tableWidget, QXlsx::Document& xlsx)
{
    if(tableWidget == nullptr || tableWidget->m_pModel->rowCount() < 1)
    {
        return false;
    }


    for(int col = 0; col < tableWidget->m_pModel->columnCount(); ++col)
    {
        auto item = tableWidget->m_pModel->horizontalHeaderItem(col);
        if(item)
        {
            QXlsx::Format boldFont;
            boldFont.setFontSize(10);
            boldFont.setFontBold(true);
            xlsx.write( currentXlsRow_, col+1, item->text(), boldFont );
        }
    }
    ++currentXlsRow_;

    for( int row = 0; row < tableWidget->m_pModel->rowCount(); ++row )
    {
        for( int col = 0; col < tableWidget->m_pModel->columnCount(); ++col )
        {
            xlsx.write( currentXlsRow_, col+1, tableWidget->m_pModel->item(row,col)->text());
        }
        ++currentXlsRow_;
    }

    ++currentXlsRow_;

    return true;
}

bool writeDeviceInfoToXls(const DeviceInfo& di, QXlsx::Document& xlsx)
{
    if(di.title_.isEmpty() == false)
    {
        QXlsx::Format boldFont;
        boldFont.setFontSize(10);
        boldFont.setFontBold(true);
        xlsx.write( currentXlsRow_++, 1, di.title_, boldFont);
    }

    foreach(auto article, di.articles_)
    {
        if(false == article.isValid()){
            continue;
        }
        xlsx.write(currentXlsRow_, 1, article.name);
        if( article.externalLinks )
        {
            xlsx.write(currentXlsRow_++, 2,  DeviceInfoParser::Instance().getLsbRelease() + "(" +  DeviceInfoParser::Instance().getHomeUrl() + ")" + DeviceInfoParser::Instance().getOsInfo() );
        }
        else
        {
            xlsx.write(currentXlsRow_++, 2, article.value );
        }
    }

    ++currentXlsRow_;

    return true;
}


bool DeviceInfoWidgetBase::exportToXls(QXlsx::Document& xlsFile)
{
    QXlsx::Format boldFont;
    boldFont.setFontBold(true);
    xlsFile.write( currentXlsRow_++, 1, overviewInfo_.name, boldFont);

    if(tableWidget_)
    {
        writeTabwidgetToXls(tableWidget_, xlsFile);
    }

    if(titleInfo_)
    {
        writeDeviceInfoToXls(*titleInfo_, xlsFile);
    }

    foreach(auto di, deviceInfos_)
    {
        writeDeviceInfoToXls(di, xlsFile);
    }

    return true;
}


bool DeviceInfoWidgetBase::exportToXls(const QString& xlsFile)
{
    currentXlsRow_ = 1;
    QXlsx::Document xlsx;

    exportToXls(xlsx);

    xlsx.saveAs(xlsFile);

    return true;
}

bool writeTabwidgetToHtml(LogTreeView* tableWidget, QFile& html)
{
    if(tableWidget == nullptr || tableWidget->m_pModel->rowCount() < 1)
    {
        return false;
    }

    html.write("<table border=\"0\" white-space:pre>\n");
    html.write("<thead><tr>\n");
    for(int col = 0; col < tableWidget->m_pModel->columnCount(); ++col)
    {
        auto item = tableWidget->m_pModel->horizontalHeaderItem(col);
        if(item)
        {
            html.write( QString("<th style=\"width:200px;text-align:left; white-space:pre;\">" + item->text() + "</th>").toUtf8().data() );
        }
        else {
             html.write( "<td style=\"width:200px;text-align:left; white-space:pre;\"> </td>" );
        }
    }
    html.write("</tr></thead>\n");

    for( int row = 0; row < tableWidget->m_pModel->rowCount(); ++row )
    {
        html.write("<tr>\n");
        for( int col = 0; col < tableWidget->m_pModel->columnCount(); ++col )
        {
            html.write( QString("<td style=\"width:200px;text-align:left;\">" + tableWidget->m_pModel->item(row,col)->text() + "</td>").toUtf8().data() );
        }
        html.write("</tr>\n");
    }


    html.write("</table>\n");
    return true;
}

bool DeviceInfoWidgetBase::exportToHtml(QFile& htmlFile)
{
    htmlFile.write("<!DOCTYPE html>\n");
    htmlFile.write("<html>\n");
    htmlFile.write("<body>\n");

    htmlFile.write( (QString("<h2>") + overviewInfo_.name + "</h2>").toUtf8() );

    if(tableWidget_)
    {
        writeTabwidgetToHtml(tableWidget_, htmlFile);
        htmlFile.write("<br />\n");
    }

    QDomDocument doc;

    if(titleInfo_)
    {
        toHtmlString(doc, *titleInfo_);
        doc.appendChild( doc.createElement("br") );
    }

    foreach(auto di, deviceInfos_)
    {
        toHtmlString(doc, di);
    }

    htmlFile.write(doc.toString().toStdString().data());

    htmlFile.write("</body>\n");
    htmlFile.write("</html>\n");
    return true;
}

bool DeviceInfoWidgetBase::exportToHtml(const QString& htmlFile)
{
    QFile html(htmlFile);
    if(false == html.open(QIODevice::OpenModeFlag::WriteOnly))
    {
        return false;
    }

    exportToHtml(html);
    html.close();
    return true;
}
