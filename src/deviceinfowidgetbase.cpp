/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
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
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include "DScrollBar"
#include <QSizePolicy>
#include "DFileDialog"
#include <QTextStream>
#include <QFile>
#include "mainwindow.h"
#include "tablewidgetalwaysfocus.h"
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

//bool DeviceInfo::isPaletteInit_ = false;
//DPalette DeviceInfo::defaultPa_;

int currentXlsRow_ = 1;

DeviceInfo::DeviceInfo()
{
    changeTheme();
}

void DeviceInfo::changeTheme()
{ 
//    if(title)
//    {
//        if(isPaletteInit_ == false)
//        {
//            defaultPa_ = DApplicationHelper::instance()->palette(title);
//            auto color = defaultPa_.brightText().color();
//            defaultPa_.setBrush(QPalette::WindowText, defaultPa_.brightText() );
//            isPaletteInit_ = true;
//        }

//        DApplicationHelper::instance()->setPalette(title, defaultPa_);
//    }

    foreach(auto widget, columnWidgets)
    {
        widget->changeTheme();
    }
}

DeviceInfoWidgetBase::DeviceInfoWidgetBase(DWidget *parent_, const QString& deviceName) : DWidget(parent_)
{
    overviewInfo_.name = deviceName;

    vLayout_ = new QVBoxLayout;

    setContentsMargins( DeviceWidgetMargin_, DeviceWidgetMargin_, DeviceWidgetMargin_, DeviceWidgetMargin_);

    vLayout_->setSpacing(0);
    vLayout_->setMargin(0);

    //setAutoFillBackground(false);

    setLayout(vLayout_);

    initContextMenu();

    //changeTheme();

//    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this,
//            [this](){
//                DeviceInfoWidgetBase::isPaletteInit_ = false;
//                //DeviceInfo::isPaletteInit_ = false;

//                //DeviceInfoWidgetBase::changeTheme();
//            }
//        );

    initFont();
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

//    auto modifyTheme = [this](){
//        DPalette pa = DApplicationHelper::instance()->palette(contextMenu_);

////        QColor base_color = palette().base().color();
////        base_color.setAlpha(20);

//        //QColor base_color(1,1,1,1);

//        //pa.setColor(QPalette::Background, base_color);
//        //pa.setColor(QPalette::Button, base_color);

//        DApplicationHelper::instance()->setPalette(contextMenu_, pa);
//    };

//    modifyTheme();

//    setAutoFillBackground(false);
//    setBackgroundRole(DPalette::Base);

    //connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, modifyTheme);

    contextMenu_->setMinimumWidth(contextMenuWidth_);

    refreshAction_ = new QAction(DApplication::translate("Main", "Refresh"), this);
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


    QAction* exportAction = new QAction( DApplication::translate("Main", "Export") );
    connect(exportAction, &QAction::triggered, this, &DeviceInfoWidgetBase::onExportToFile);
    contextMenu_->addAction(exportAction);
}

//void DeviceInfoWidgetBase::addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QList<ArticleStruct>& articles, const QFont& font, const QPalette& pa)
//{
//    ly->setSizeConstraint(QLayout::SetNoConstraint);

//    int i = 0;
//    foreach(auto article, articles)
//    {
//        if( article.autoHide &&  article.isValid() == false )
//        {
//            continue;
//        }

//        DLabel* nameLabel = new DLabel( DApplication::translate("Main", article.name.toStdString().data()) + ":", downWidget_ );
//    #ifdef TEST_DATA_FROM_FILE
//        nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
//    #endif
//        //nameLabel->setScaledContents(true); //默认是false
//        nameLabel->setWordWrap(true);
//        nameLabel->setMinimumWidth(NameLength_);
//        //nameLabel->setMinimumHeight(SubRowHeight_);
//        nameLabel->setFont(font);

//        DApplicationHelper::instance()->setPalette(nameLabel, pa);

//        DLabel* contentLabel = new DLabel( article.valueTranslate ? \
//                                           DApplication::translate("Main", article.value.toStdString().data()): \
//                                           article.value, downWidget_ );
//    #ifdef TEST_DATA_FROM_FILE
//        contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
//    #endif
//        //contentLabel->setScaledContents(true); //默认是false
//        if(article.externalLinks)
//        {
//            contentLabel->setOpenExternalLinks(true);
//        }

//        contentLabel->setWordWrap(true);
//        contentLabel->setMinimumWidth(DeviceWidgetContentWidth_);
//        //contentLabel->setMinimumHeight(SubRowHeight_);
//        contentLabel->setFont(font);
//        DApplicationHelper::instance()->setPalette(contentLabel, pa);

//        di->columnWidgets.push_back(nameLabel);

//        ly->addWidget(nameLabel, i, 0, Qt::AlignTop);
//        ly->setColumnStretch(0, 1);
//        ly->addWidget(contentLabel, i, 1, Qt::AlignTop);
//        ly->setColumnStretch(1, 3);
//        ++i;
//    }
//}

void DeviceInfoWidgetBase::addCloumnToLayout(DeviceInfo* di, QVBoxLayout* vly, const QList<ArticleStruct>& articles, const QFont& font, int fontSizetype, int columnHeight, const QPalette& pa)
{

    vly->setSizeConstraint(QLayout::SetNoConstraint);

    foreach(auto article, articles)
    {
        if( article.autoHide &&  article.isValid() == false )
        {
            continue;
        }

        QString value = article.valueTranslate? DApplication::translate("Main", article.value.toStdString().data()): article.value;

        ColumnWidget* widget = new ColumnWidget( DApplication::translate("Main", article.name.toStdString().data())+ ":", value, font, fontSizetype, columnHeight, article.externalLinks, downWidget_, this);

        DApplicationHelper::instance()->setPalette(widget->l1, pa);
        DApplicationHelper::instance()->setPalette(widget->l2, pa);

        di->columnWidgets.push_back( widget );
        vly->addWidget(widget);
    }
}

void DeviceInfoWidgetBase::setCentralInfo(const QString& info)
{
    if(titleInfo_ == nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    if(downWidget_)
    {
        delete downWidget_;
    }

    downWidget_ = new DWidget(this);
    downWidget_->setAutoFillBackground(true);
    downWidgetLayout = new QVBoxLayout(downWidget_);
    downWidget_->setLayout(downWidgetLayout);

    if( false == info.isEmpty() )
    {
        titleInfo_->title = new DLabel( DApplication::translate("Main", info.toStdString().data()), downWidget_);
        // titleInfo_->title->setFont(centralFont_);
        DFontSizeManager::instance()->bind(titleInfo_->title, DFontSizeManager::T4);

        auto hLayout = new QHBoxLayout;
        hLayout->addStretch(1);
        hLayout->addWidget(titleInfo_->title/*, 1, Qt::AlignmentFlag::AlignCenter*/);
        hLayout->addStretch(1);

        downWidgetLayout->addStretch(1);
        downWidgetLayout->addLayout(hLayout);
        downWidgetLayout->addStretch(1);
    }
    DFrame* f = new DFrame(this);
    //f->setAutoFillBackground(false);
    QVBoxLayout* ly = new QVBoxLayout;
//    ly->setMargin(0);
//    ly->setSpacing(0);
    ly->addWidget(downWidget_);
    f->setLayout(ly);

//    vLayout_->insertWidget( vLayout_->count(), f);

    vLayout_->addWidget( f );
}

//void DeviceInfoWidgetBase::addInfo(const QString& title, const QList<ArticleStruct>& articles)
//{
//    initDownWidget();

//    if(titleInfo_ == nullptr)
//    {
//       titleInfo_ = new DeviceInfo;
//    }

//    QVBoxLayout* vly = new QVBoxLayout;
//    vly->setContentsMargins(DeviceWidgetContentMarginLeft_, DeviceWidgetContentMarginTop_, DeviceWidgetContentMarginRight_, DeviceWidgetContentMarginBottom_);

//    if( false == title.isEmpty() )
//    {
//        titleInfo_->title = new DLabel( DApplication::translate("Main", title.toStdString().data()) + ":", downWidget_);
//        titleInfo_->title->setFont(titleFont_);
//        vly->addWidget(titleInfo_->title);
//    }

//    QHBoxLayout* hly = new QHBoxLayout;

//    QGridLayout* gridLayout = new QGridLayout;
//    gridLayout->setSpacing(10);

//    gridLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
//    hly->addLayout(gridLayout);
//    vly->addLayout(hly);

//    DPalette pa = DApplicationHelper::instance()->palette(this);
//    addLabelToGridLayout(titleInfo_, gridLayout, articles, infoFont_, pa);

//    infoWidget_ = new DWidget(this);
//    infoWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    infoWidget_->setLayout(vly);
//    downWidgetLayout->insertWidget( downWidgetLayout->count(), infoWidget_ );

//    downWidget_->adjustSize();

//    verticalScrollBarMaxValue += infoWidget_->height();
//    downWidgetScrollArea_->verticalScrollBar()->setRange( 0, verticalScrollBarMaxValue );
//}

// Html version

QString toHtmlString(const QList<ArticleStruct>& articles)
{
    QString str = "<table border=\"0\">\n";

    foreach( auto article, articles )
    {
        str += "<tr>\n";
        str +=( QString("<td style=\"width:200px;text-align:left;white-space:pre;\">" + article.name + "</td>").toUtf8().data() );

        str +=( QString( "<td>" + article.value + "</td>\n").toUtf8().data() );

        str +=("</tr>\n");
    }

    str +=("</table>\n");

    return str;
}

void DeviceInfoWidgetBase::addHtmlInfo(const QString& title, const QList<ArticleStruct>& articles)
{
    initDownWidget();

    if(titleInfo_ == nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    QVBoxLayout* vly = new QVBoxLayout;
    vly->setContentsMargins(20, 15, 20, 20);

    if( false == title.isEmpty() )
    {
        titleInfo_->title = new DLabel( DApplication::translate("Main", title.toStdString().data()) + ":", downWidget_);
        titleInfo_->title->setFont(titleFont_);
        DFontSizeManager::instance()->bind(titleInfo_->title, DFontSizeManager::T5);
        vly->addWidget(titleInfo_->title);
    }

    QString html = toHtmlString(articles);
    DLabel* htmlLabel = new DLabel( html, downWidget_);
    htmlLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    htmlLabel->setOpenExternalLinks(true);

    vly->addWidget(htmlLabel);

    infoWidget_ = new DWidget(this);
    infoWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    infoWidget_->setLayout(vly);

    downWidgetLayout->insertWidget( downWidgetLayout->count(), infoWidget_ );

    downWidget_->adjustSize();

    verticalScrollBarMaxValue += infoWidget_->height();
    downWidgetScrollArea_->verticalScrollBar()->setRange( 0, verticalScrollBarMaxValue );
}

void DeviceInfoWidgetBase::addInfo(const QString& title, const QList<ArticleStruct>& articles, bool main)
{
    initDownWidget();

    if(titleInfo_ == nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    QVBoxLayout* vly = new QVBoxLayout;
    vly->setContentsMargins(DeviceWidgetContentMarginLeft_, DeviceWidgetContentMarginTop_, DeviceWidgetContentMarginRight_, DeviceWidgetContentMarginBottom_);

    if( false == title.isEmpty() )
    {
        titleInfo_->title = new DLabel( DApplication::translate("Main", title.toStdString().data()) + ":", downWidget_);
        titleInfo_->title->setFont(titleFont_);
        DFontSizeManager::instance()->bind(titleInfo_->title, DFontSizeManager::T5);
        vly->addWidget(titleInfo_->title);
    }

    DPalette pa = DApplicationHelper::instance()->palette(this);
    if(main == true)
    {
        addCloumnToLayout(titleInfo_, vly, articles, infoFont_, DFontSizeManager::T6, 29, pa);
    }

    else
    {
        addCloumnToLayout(titleInfo_, vly, articles, labelFont_, DFontSizeManager::T8, 21, pa);
    }

    infoWidget_ = new DWidget(this);
    infoWidget_->setAutoFillBackground(true);
    //infoWidget_->setFixedWidth(DeviceWidgetDownWidgehWidth_);
    infoWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    infoWidget_->setLayout(vly);

    downWidgetLayout->insertWidget( downWidgetLayout->count(), infoWidget_ );

    //downWidgetLayout->addStretch(1);

    downWidget_->adjustSize();

    verticalScrollBarMaxValue += infoWidget_->height();
    downWidgetScrollArea_->verticalScrollBar()->setRange( 0, verticalScrollBarMaxValue );
}

//void DeviceInfoWidgetBase::addSubInfo(const QString& subTitle, const QList<ArticleStruct>& articles)
//{
//    initDownWidget();
//    QVBoxLayout* vly = new QVBoxLayout;

//    vly->setContentsMargins(20, 0, 20, 20);

//    DeviceInfo subInfo;
//    if(false == subTitle.isEmpty())
//    {
//        subInfo.title = new DLabel( DApplication::translate("Main", subTitle.toStdString().data()) + ":", downWidget_);
//        subInfo.title->setFont(subTitleFont_);
//        vly->addWidget( subInfo.title );
//    }

//    QHBoxLayout* hly = new QHBoxLayout;

//    //hly->addSpacing(10);
//    QGridLayout* gridLayout = new QGridLayout;

//    DPalette pa = DApplicationHelper::instance()->palette(this);
//    //QColor color = pa.textTips().color();
//    //pa.setColor(QPalette::WindowText, QColor("#526A7F"));
//    addLabelToGridLayout(&subInfo, gridLayout, articles, labelFont_, pa);

//    hly->addLayout(gridLayout);
//    vly->addLayout(hly);

//    DWidget* subInfoWidget = new DWidget(this);
//    subInfoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    subInfoWidget->setLayout(vly);
////    if( infoWidget_ || subinfoWidgetList_.size() > 0 )
////    {
////        vly->setContentsMargins(0, 20, 0, 0);
////    }

//    subinfoWidgetList_.push_back(subInfoWidget);
//    downWidgetLayout->insertWidget( downWidgetLayout->count(), subInfoWidget);
//    deviceInfos_.push_back(subInfo);
//    downWidget_->adjustSize();

//    verticalScrollBarMaxValue += subInfoWidget->height();
//    downWidgetScrollArea_->verticalScrollBar()->setRange(0, verticalScrollBarMaxValue);
//}

void DeviceInfoWidgetBase::addSubInfo(const QString& subTitle, const QList<ArticleStruct>& articles, int margin)
{
    initDownWidget();
    QVBoxLayout* vly = new QVBoxLayout;

    vly->setContentsMargins(margin, 0, 0, 20);

    DeviceInfo subInfo;
    if(false == subTitle.isEmpty())
    {
        subInfo.title = new DLabel( DApplication::translate("Main", subTitle.toStdString().data()) + ":", downWidget_);
        subInfo.title->setFont(subTitleFont_);
        DFontSizeManager::instance()->bind(subInfo.title, DFontSizeManager::T6);
        vly->addWidget( subInfo.title );
    }

    DPalette pa = DApplicationHelper::instance()->palette(this);

    addCloumnToLayout(&subInfo, vly, articles, labelFont_, DFontSizeManager::T8, 21, pa);

    DWidget* subInfoWidget = new DWidget(this);
    subInfoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    subInfoWidget->setLayout(vly);


    subinfoWidgetList_.push_back(subInfoWidget);
    downWidgetLayout->insertWidget( downWidgetLayout->count(), subInfoWidget);
    deviceInfos_.push_back(subInfo);
    downWidget_->adjustSize();

    verticalScrollBarMaxValue += subInfoWidget->height();
    downWidgetScrollArea_->verticalScrollBar()->setRange(0, verticalScrollBarMaxValue);
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
        //tableWidget_ = new DTableWidget(this);
        //tableWidget_->setHorizontalHeader(new TableWidgetAlwaysActiveHeaderView(Qt::Orientation::Horizontal, this) );

        //tableWidget_->setMinimumHeight(183);
        //tableWidget_->setMaximumHeight(500);
        tableWidget_->setFixedHeight(TableViewRowHeight_*5 +3 /*+ tableWidget_->verticalHeader()->height()+3*/);

        tableWidget_->m_headerDelegate->setDefaultSectionSize(TableViewRowHeight_);
        tableWidget_->m_headerDelegate->setFixedHeight(TableViewRowHeight_);

        //auto scrollBar = new DScrollBar(tableWidget_);

//        tableWidget_->setVerticalScrollBar(scrollBar);

        //tableWidget_->horizontalHeader()->setClickable(false);
        //tableWidget_->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        //tableWidget_->setAutoFillBackground(true);


        //tableWidget_->setAttribute(Qt::WA_ShowWithoutActivating);

        //tableWidget_->setWindowFlags(/*Qt::Tool | Qt::FramelessWindowHint|*/Qt::WindowStaysOnTopHint);
        //tableWidget_->overrideWindowState(Qt::WindowState::WindowActive);
        //tableWidget_->setEnabled(true);
        //tableWidget_->horizontalHeader()->clearMask();
        //tableWidget_->horizontalHeader()->setFont(tableHeaderFont_);
        DFontSizeManager::instance()->bind( tableWidget_->m_headerDelegate, DFontSizeManager::T6);

        //tableWidget_->setRowCount(contentsList.size());
        //tableWidget_->setColumnCount(headers.size());
        QStringList translaterHeaders;
        foreach(auto header, headers)
        {
            translaterHeaders.push_back(DApplication::translate("Main", header.toStdString().data()));
        }
        tableWidget_->m_pModel->setHorizontalHeaderLabels(translaterHeaders);

        //tableWidget_->horizontalHeader()->setObjectName("DeviceInfoHeaderView");

//        auto changeTheme = [this](){
//            DPalette pa = this->palette();

////            //QBrush bash_brush( QColor(0,0,0,255) );

//            pa.setColorGroup(QPalette::Inactive, pa.windowText(), pa.button(),pa.light(),pa.dark(),pa.mid(),pa.text(),pa.brightText(),pa.base(), pa.window());
////            //pa.setColorGroup(QPalette::Active, pa.base(), pa.button(),pa.light(),pa.dark(),pa.mid(),pa.text(),pa.brightText(),pa.base(), pa.window());

////            QColor base_color = palette().base().color();

////            //pa.setColor(QPalette::Background, base_color);
////            //pa.setBrush(QPalette::Inactive, DPalette::Base, pa.brush(QPalette::Active, DPalette::Dark));
////            //pa.setBrush(QPalette::Inactive, DPalette::Base, bash_brush);

////            DApplicationHelper::instance()->setPalette(tableWidget_, pa);
////            //DApplicationHelper::instance()->setPalette(tableWidget_->horizontalHeader(), pa);

////            //pa.setBrush(QPalette::Highlight, bash_brush);

//            tableWidget_->setPalette(pa);

////            //tableWidget_->horizontalHeader()->setAutoFillBackground(true);

//////            for(int itemIndex = 0; itemIndex < tableWidget_->horizontalHeader()->count(); ++itemIndex)
//////            {
//////                tableWidget_->horizontalHeaderItem(itemIndex)->setBackground(bash_brush);
//////                //tableWidget_->horizontalHeader()->model()->setData(itemIndex, 0, Qt::BackgroundRole);
//////            }

////            //tableWidget_->horizontalHeader()->setStyleSheet( "QHeaderView#DeviceInfoHeaderView{ background-color: white; }");
//        };

//        changeTheme();

        //connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, changeTheme);


        //tableWidget_->horizontalHeader()->setContentsMargins(0,0,0,0);
        //tableWidget_->horizontalHeader()->setHighlightSections(false);
        //tableWidget_->horizontalHeader()->setFrameShape(QFrame::Shape::NoFrame);
        //tableWidget_->setAttribute(Qt::WA_TranslucentBackground);
//        tableWidget_->horizontalHeader()->setAttribute(Qt::WA_TranslucentBackground);

//        tableWidget_->horizontalHeader()->setAttribute(Qt::WA_TranslucentBackground);


//        tableWidget_->horizontalHeader()->setAttribute(Qt::WA_OpaquePaintEvent);
        //tableWidget_->setAttribute(Qt::WA_NoSystemBackground);


        //tableWidget_->horizontalHeader()->setFrameShadow(QFrame::Shadow::Plain);
        //tableWidget_->horizontalHeader()->setAutoFillBackground(false);

//        tableWidget_->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
//        tableWidget_->setSelectionMode(QAbstractItemView::SingleSelection);

        //tableWidget_->verticalHeader()->setVisible(false);
        //tableWidget_->setGridStyle( Qt::PenStyle::NoPen);
        //tableWidget_->setShowGrid(false);

        //tableWidget_->setAlternatingRowColors(true);
//        tableWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);

//        tableWidget_->setFrameShape(QFrame::Shape::NoFrame);

        connect(tableWidget_, &DTreeView::clicked, this, &DeviceInfoWidgetBase::OnCurrentItemClicked);
    }

    //tableWidget_->horizontalHeader()->setSectionResizeMode(headers.size() - 2, QHeaderView::Stretch);
    //tableWidget_->horizontalHeader()->setDefaultAlignment(Qt::AlignmentFlag::AlignLeft);
//    tableWidget_->setColumnWidth(0, 100);
//    tableWidget_->setColumnWidth(1, 220);
//    tableWidget_->setColumnWidth(2, 140);
//    tableWidget_->setColumnWidth(3, 100);
//    tableWidget_->setColumnWidth(4, 50);

//    DFrame* f = new DFrame(this);
//    QVBoxLayout* vly = new QVBoxLayout;
//    vly->addWidget(tableWidget_);
//    vly->setMargin(3);
//    f->setLayout(vly);

//    vLayout_->insertWidget(0, f);
//    QHBoxLayout* hly = new QHBoxLayout;
//    hly->addWidget(tableWidget_);
//    hly->addSpacing(10);
//    vLayout_->insertLayout(0, hly);

    vLayout_->insertWidget(0, tableWidget_);
    vLayout_->insertSpacing(1, 2);

    for(int i = 0; i < contentsList.size(); ++i)
    {
        //tableWidget_->setRowHeight(i, TableViewRowHeight_);
        const QStringList& contents = contentsList[i];
        for(int j = 0; j < contents.size(); ++j )
        {
            DStandardItem* item = new DStandardItem(contents[j]);
            if(j == 0)
            {
                item->setData( i, Qt::UserRole + 90 );
            }

//            const QMargins ListViweItemMargin(10, 8, 0, 8);
//            //const QMargins ListViweItemMargin( 10, 5, 10, 5 );
//            const QVariant VListViewItemMargin = QVariant::fromValue(ListViweItemMargin);

//            item->setData(VListViewItemMargin, Dtk::MarginsRole);

            //int flags = item->flags();
            //item->setFlags(flags | Qt::ItemFlag::ItemIsEditable);
            //item->setFont(tableContentFont_);
            //DFontSizeManager::instance()->bind( item, DFontSizeManager::T6);
            //tableWidget_->setItem(i, j, item);

            tableWidget_->m_pModel->setItem(i, j, item);
        }
    }

    //tableWidget_->setModel();

    //tableWidget_->clearSpans();

    //tableWidget_->m_headerDelegate->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    //tableWidget_->m_headerDelegate->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget_->m_headerDelegate->setStretchLastSection(true);
    tableWidget_->m_headerDelegate->resizeSections(QHeaderView::ResizeMode::ResizeToContents);
    //tableWidget_->resizeColumnsToContents();
    //tableWidget_->resizeRowsToContents();

    //tableWidget_->setContentsMargins(50, 0, 10, 0);
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
    if( downWidgetScrollArea_ )
    {
        return;
    }

    DFrame* f = new DFrame(this);
    //auto f = new DWidget(this);

    downWidgetScrollArea_ = new DScrollArea(this);
    downWidgetScrollArea_->setAutoFillBackground(true);

    downWidgetScrollArea_->setFrameShape(QFrame::NoFrame);
    downWidget_ = new DWidget(downWidgetScrollArea_);
    downWidget_->setAutoFillBackground(true);

    //downWidget_->setBackgroundRole(QPalette::Base);
    //downWidget_->setAutoFillBackground(true);

    //downWidget_->setFixedHeight(100);
    //downWidget_->setBaseSize(700, 100);
    //downWidget_->setFixedWidth(700);
    //downWidget_->setFixedHeight(20);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    downWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //downWidget_->setFixedWidth(DeviceWidgetDownWidgehWidth_);
    //downWidget_->setFixedHeight(mainWindowMinHeight_);

    downWidgetLayout = new QVBoxLayout;
    downWidgetLayout->setMargin(0);
    downWidgetLayout->setSpacing(0);
    downWidget_->setLayout(downWidgetLayout);
    downWidgetScrollArea_->setWidget(downWidget_);


    //f->setAutoFillBackground(false);
    QVBoxLayout* ly = new QVBoxLayout;
//    ly->setMargin(0);
//    ly->setSpacing(0);
    ly->addWidget(downWidgetScrollArea_);
    f->setLayout(ly);
    vLayout_->insertWidget( vLayout_->count(), f);

    //vLayout_->insertWidget( vLayout_->count(), downWidgetScrollArea_);

    //connect(downWidgetScrollArea_->verticalScrollBar(), &QScrollBar::valueChanged, this, &DeviceInfoWidgetBase::onScroll);
}

QString DeviceInfoWidgetBase::getDeviceName()
{
    return overviewInfo_.name;
}

void DeviceInfoWidgetBase::deviceListClicked()
{
    if(downWidgetScrollArea_ == nullptr)
    {
        return;
    }

    downWidgetScrollArea_->verticalScrollBar()->setValue(0);
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

void DeviceInfoWidgetBase::selectColumnWidget(ColumnWidget* sw)
{
    if(selectColumnWidget_ == sw)
    {
        return;
    }

    if(selectColumnWidget_)
    {
        selectColumnWidget_->setHilight(false);
    }

    selectColumnWidget_ = sw;
}

void DeviceInfoWidgetBase::getContextMenu(DMenu** contextMenu)
{
    *contextMenu = contextMenu_;
}

QString DeviceInfoWidgetBase::joinArticle(QList<ArticleStruct>& articles)
{
    QString res;
    foreach(auto article, articles)
    {
        if(article.isValid())
        {
            if(res.isEmpty() == false)
            {
                res += " ";
            }

            res += article.value;
        }
    }

    return res;
}

void DeviceInfoWidgetBase::changeTheme()
{
    if(isPaletteInit_ == false)
    {
        defaultPa_ = DApplicationHelper::instance()->palette(this);
        defaultPa_.setBrush(QPalette::Background, palette().base() );
        isPaletteInit_ = true;
    }

    DApplicationHelper::instance()->setPalette(this, defaultPa_);

    if(titleInfo_)
    {
        titleInfo_->changeTheme();
    }

    foreach(auto dv, deviceInfos_)
    {
        dv.changeTheme();
    }

    selectColumnWidget_ = nullptr;
}

void DeviceInfoWidgetBase::mousePressEvent(QMouseEvent *event)
{
    selectColumnWidget(nullptr);

    DWidget::mousePressEvent(event);
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

void DeviceInfoWidgetBase::resizeEvent(QResizeEvent *event)
{
//    if(tableWidget_)
//    {
//        tableWidget_->adjustSize();
//    }

//    if(downWidget_)
//    {
//        if(titleInfo_)
//        {
//            if(titleInfo_->title)
//            {
//                titleInfo_->title->adjustSize();
//            }

//            foreach(auto namelabel, titleInfo_->nameLabels)
//            {
//                if(namelabel)
//                {
//                    namelabel->adjustSize();
//                }
//            }

//            foreach(auto content, titleInfo_->contentLabels)
//            {
//                if(content)
//                {
//                    content->adjustSize();
//                }
//            }
//        }

//        foreach(auto info, deviceInfos_)
//        {
//            if(info.title)
//            {
//                info.title->adjustSize();
//            }

//            foreach(auto namelabel, info.nameLabels)
//            {
//                if(namelabel)
//                {
//                    namelabel->adjustSize();
//                }
//            }

//            foreach(auto content, info.contentLabels)
//            {
//                if(content)
//                {
//                    content->adjustSize();
//                }
//            }
//        }
//    }

    if(infoWidget_)
    {
        infoWidget_->adjustSize();
    }

    DWidget::resizeEvent(event);
}

void DeviceInfoWidgetBase::OnCurrentItemClicked(const QModelIndex &index)
{
    if(downWidgetScrollArea_ == nullptr)
    {
        return;
    }

    QStandardItem* item = tableWidget_->m_pModel->item( index.row() );
    if(item == nullptr)
    {
        return;
    }


    int height = 0;
    if(infoWidget_)
    {
        height += infoWidget_->height();
    }


    int row = item->data(Qt::UserRole+90).toInt();

    for(int i = 0; i < row; ++i )
    {
        height += subinfoWidgetList_.at(i)->height();
    }

    downWidgetScrollArea_->verticalScrollBar()->setValue(height);
}

void DeviceInfoWidgetBase::onScroll(int value)
{
    int height = 0;

    for(int i = 0; i < subinfoWidgetList_.size(); ++i )
    {
        height += subinfoWidgetList_.at(i)->height();;
        if( height == value )
        {
            if(tableWidget_)
            {
                //tableWidget_->setCurrentIndex(i);
            }

            continue;
        }

        if(height > value)
        {
            return;
        }
    }
}

bool DeviceInfoWidgetBase::onExportToFile()
{
   QString selectFilter;

   QString saveDir = "./";
   QDir dir( QDir::homePath() + "/Documents/");
   if(dir.exists())
   {
        saveDir = QDir::homePath() + "/Documents/";
   }

   QString exportFile = DFileDialog::getSaveFileName(this,
                                                     DApplication::translate("Main", "Export"), saveDir + DApplication::translate("Main", "deviceInfo") + \
                                                     QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") .remove(QRegExp("\\s")) + ".txt", \
                                                     tr("Text (*.txt);; Doc (*.docx);; Xls (*.xls);; Html (*.html)"), &selectFilter);

   if(exportFile.isEmpty() == true)
   {
       return true;
   }

   MainWindow* mainWindow = dynamic_cast<MainWindow*>(this->parent()->parent()->parent());
   if( nullptr == mainWindow )
   {
       return false;
   }

   return mainWindow->exportTo(exportFile, selectFilter);
}

QString getOsInfoWithoutHtml(const QString& str )
{
    int index = str.indexOf("href=\"");
    QString href;
    QString osName;
    QString osOther =  str;
    if(index > 0)
    {
        int index_maohao = str.indexOf("\">", index);
        if(index_maohao  > index )
        {
            href = str.mid( index+5, index_maohao - index - 5);
            int index_last_a = str.indexOf("</a>", index);
            if(index_last_a > index_maohao)
            {
                osName = str.mid(index_maohao + 2, index_last_a - index_maohao -2);
                osOther = str.mid(index_last_a + 4).trimmed();
            }
        }
    }

    return osName + "(" + href + ")" + " " + osOther;
}

QTextStream& operator<<(QTextStream& ds, const DeviceInfo& di)
{
    if(di.title)
    {
        ds << di.title->text() << "\n";
    }


    for(int i = 0; i < di.columnWidgets.size(); ++i)
    {
        ds.setFieldWidth(20);
        ds.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
        ds << di.columnWidgets[i]->l1->text();
        ds.setFieldWidth(0);
        if(di.columnWidgets[i]->l2->openExternalLinks() )
        {
            ds << getOsInfoWithoutHtml(di.columnWidgets[i]->l2->text()) << "\n";
        }
        else
        {
            ds << di.columnWidgets[i]->l2->text()<< "\n";
        }
    }

    return ds;
}

QTextStream& operator<<(QTextStream& ds, DTableWidget* tableWidget)
{
    if(tableWidget == nullptr || tableWidget->rowCount() < 1)
    {
        return ds;
    }

    for(int col = 0; col < tableWidget->columnCount(); ++col)
    {
        auto item = tableWidget->horizontalHeaderItem(col);
        ds.setFieldWidth(tableWidget->columnWidth(col) *25.0 / tableWidget->width());
        ds.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
        if(item)
        {
             ds << tableWidget->horizontalHeaderItem(col)->text();
        }
        else
        {
            ds << "";
        }
        ds.setFieldWidth(0);
    }
    ds << "\n";


    for( int row = 0; row < tableWidget->rowCount(); ++row )
    {
        for( int col = 0; col < tableWidget->columnCount(); ++col )
        {
            ds.setFieldWidth(tableWidget->columnWidth(col) *25.0 / tableWidget->width());
            ds.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
            ds << tableWidget->item(row,col)->text();
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

bool writeTabwidgetToDoc(DTableWidget* tableWidget, Docx::Document& doc)
{
    if(tableWidget == nullptr || tableWidget->rowCount() < 1)
    {
        return false;
    }

    Docx::Table* tab = doc.addTable(tableWidget->rowCount()+1, tableWidget->columnCount());
    //tab->setAlignment(Docx::WD_TABLE_ALIGNMENT::LEFT);

    for(int col = 0; col < tableWidget->columnCount(); ++col)
    {
        auto item = tableWidget->horizontalHeaderItem(col);
        auto cel = tab->cell(0, col);
        if(item)
        {
             cel->addText(tableWidget->horizontalHeaderItem(col)->text());
        }
    }

    for( int row = 0; row < tableWidget->rowCount(); ++row )
    {
        for( int col = 0; col < tableWidget->columnCount(); ++col )
        {
            auto cel = tab->cell(row+1, col);
            cel->addText(tableWidget->item(row,col)->text());
        }
    }

    return true;
}

bool writeDeviceInfoToDoc(const DeviceInfo& di, Docx::Document& doc)
{
    if(di.title)
    {
        doc.addHeading(di.title->text(), 4);
    }

    for(int i = 0; i < di.columnWidgets.size(); ++i)
    {
        QString name = di.columnWidgets[i]->l1->text();
        QString content;
        if( di.columnWidgets[i]->l2->openExternalLinks() )
        {
            content = getOsInfoWithoutHtml( di.columnWidgets[i]->l2->text() );
        }
        else
        {
            content = di.columnWidgets[i]->l2->text();
        }

        QString line;
        if(name.trimmed().isEmpty() == false || false == content.trimmed().isEmpty())
        {
            line = name + " " + content;
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
        //writeTabwidgetToDoc(tableWidget_, doc);
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

    doc.addParagraph("\n");
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

bool writeTabwidgetToXls(DTableWidget* tableWidget, QXlsx::Document& xlsx)
{
    if(tableWidget == nullptr || tableWidget->rowCount() < 1)
    {
        return false;
    }


    for(int col = 0; col < tableWidget->columnCount(); ++col)
    {
        auto item = tableWidget->horizontalHeaderItem(col);
        if(item)
        {
            QXlsx::Format boldFont;
            boldFont.setFontSize(10);
            boldFont.setFontBold(true);
            xlsx.write( currentXlsRow_, col+1, item->text(), boldFont );
        }
    }
    ++currentXlsRow_;

    for( int row = 0; row < tableWidget->rowCount(); ++row )
    {
        for( int col = 0; col < tableWidget->columnCount(); ++col )
        {
            xlsx.write( currentXlsRow_, col+1, tableWidget->item(row,col)->text());
        }
        ++currentXlsRow_;
    }

    ++currentXlsRow_;

    return true;
}

bool writeDeviceInfoToXls(const DeviceInfo& di, QXlsx::Document& xlsx)
{
    if(di.title)
    {
        QXlsx::Format boldFont;
        boldFont.setFontSize(10);
        boldFont.setFontBold(true);
        xlsx.write( currentXlsRow_++, 1, di.title->text(), boldFont);
    }

    for(int i = 0; i < di.columnWidgets.size(); ++i)
    {
        xlsx.write(currentXlsRow_, 1, di.columnWidgets[i]->l1->text());
        if( di.columnWidgets[i]->l2->openExternalLinks() )
        {
            xlsx.write(currentXlsRow_++, 2, getOsInfoWithoutHtml(di.columnWidgets[i]->l2->text()) );
        }
        else
        {
            xlsx.write(currentXlsRow_++, 2, di.columnWidgets[i]->l2->text());
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
        //writeTabwidgetToXls(tableWidget_, xlsFile);
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

bool writeTabwidgetToHtml(DTableWidget* tableWidget, QFile& html)
{
    if(tableWidget == nullptr || tableWidget->rowCount() < 1)
    {
        return false;
    }

    html.write("<table border=\"0\" white-space:pre>\n");
    html.write("<thead><tr>\n");
    for(int col = 0; col < tableWidget->columnCount(); ++col)
    {
        auto item = tableWidget->horizontalHeaderItem(col);
        if(item)
        {
            html.write( QString("<th style=\"width:200px;text-align:left; white-space:pre;\">" + item->text() + "</th>").toUtf8().data() );
        }
        else {
             html.write( "<td style=\"width:200px;text-align:left; white-space:pre;\"> </td>" );
        }
    }
    html.write("</tr></thead>\n");

    for( int row = 0; row < tableWidget->rowCount(); ++row )
    {
        html.write("<tr>\n");
        for( int col = 0; col < tableWidget->columnCount(); ++col )
        {
            html.write( QString("<td style=\"width:200px;text-align:left;\">" + tableWidget->item(row,col)->text() + "</td>").toUtf8().data() );
        }
        html.write("</tr>\n");
    }


    html.write("</table>\n");
    return true;
}

bool writeDeviceInfoToHtml(const DeviceInfo& di, QFile& html)
{
    html.write("<table border=\"0\">\n");
    if(di.title)
    {
        html.write("<thead><tr>\n");
        html.write( QString("<th style=\"text-align:left;\">" + di.title->text() + "</th>\n").toUtf8().data() );
        html.write("</tr></thead>\n");
    }

    for(int i = 0; i < di.columnWidgets.size(); ++i)
    {
        html.write("<tr>\n");
        html.write( QString("<td style=\"width:200px;text-align:left;white-space:pre;\">" + di.columnWidgets[i]->l1->text() + "</td>").toUtf8().data() );
        if( di.columnWidgets[i]->l2->openExternalLinks() )
        {
            html.write( QString( "<td>" + getOsInfoWithoutHtml(di.columnWidgets[i]->l2->text()) + "</td>\n").toUtf8().data() );
        }
        else
        {
            html.write( QString( "<td>" + di.columnWidgets[i]->l2->text() + "</td>\n").toUtf8().data() );
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
        //writeTabwidgetToHtml(tableWidget_, htmlFile);
        htmlFile.write("<br />\n");
    }

    if(titleInfo_)
    {
        writeDeviceInfoToHtml(*titleInfo_, htmlFile);
        htmlFile.write("<br />\n");
    }

    foreach(auto di, deviceInfos_)
    {
        writeDeviceInfoToHtml(di, htmlFile);
    }

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
