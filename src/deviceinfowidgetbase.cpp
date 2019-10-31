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
//#include "document.h"
//#include "thirdlib/docx/include/table.h"
//#include "xlsxdocument.h"
#include <QFile>
#include "mainwindow.h"
#include "tablewidgetalwaysfocus.h"
#include <QDate>
#include "DApplication"
#include "DApplicationHelper"
#include "table.h"
#include "DTitlebar"

DWIDGET_USE_NAMESPACE

bool DeviceInfoWidgetBase::isFontInit_ = false;
QFont DeviceInfoWidgetBase::titleFont_;
QFont DeviceInfoWidgetBase::subTitleFont_;
QFont DeviceInfoWidgetBase::infoFont_;
QFont DeviceInfoWidgetBase::labelFont_;
QFont DeviceInfoWidgetBase::tableHeaderFont_;
QFont DeviceInfoWidgetBase::tableContentFont_;


int currentXlsRow_ = 1;

static const int NameLength_ = 130;
static const int RowHeight_ = 30;
static const int SubRowHeight_ = 40;

static const int WidgetWidth = 640;
static const int WidgetHeight = 740;

DeviceInfoWidgetBase::DeviceInfoWidgetBase(DWidget *parent_, const QString& deviceName) : DWidget(parent_)
{
    //setStyleSheet("QWidget{border-top-left-radius:15px;border-top-right-radius:5px;}");
    if(isFontInit_ == false)
    {
        QLabel* nameLabel = new DLabel("", this);
        titleFont_ = nameLabel->font();
        delete nameLabel;
        subTitleFont_ = titleFont_;
        labelFont_ = titleFont_;
        infoFont_ = titleFont_;
        tableHeaderFont_ = titleFont_;
        tableContentFont_ = titleFont_;

        titleFont_.setPixelSize(16);
        titleFont_.setWeight(QFont::DemiBold);

        infoFont_.setPixelSize(15);

        subTitleFont_.setPixelSize(15);
        subTitleFont_.setWeight(QFont::DemiBold);

        labelFont_.setPixelSize(14);

        tableHeaderFont_.setWeight(QFont::Medium);
        tableHeaderFont_.setPixelSize(15);

        tableContentFont_.setWeight(QFont::Medium);
        tableContentFont_.setPixelSize(14);

        isFontInit_ = true;
    }

    overviewInfo_.name = deviceName;

    vLayout_ = new QVBoxLayout;

    vLayout_->setSpacing(0);
    vLayout_->setMargin(8);

    setAutoFillBackground(false);

    setLayout(vLayout_);
    //setMinimumHeight(WidgetHeight);

    initContextMenu();

    //setAutoFillBackground(true);

    auto modifyTheme = [this](){
        DPalette pa = DApplicationHelper::instance()->palette(this);
        QColor base_color = palette().base().color();

        pa.setColor(QPalette::Background, base_color);
        pa.setBrush(DPalette::ItemBackground, base_color);

        //setPalette(pa);

        DApplicationHelper::instance()->setPalette(this, pa);
    };

    modifyTheme();

//    setAutoFillBackground(false);
//    setBackgroundRole(DPalette::Base);

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, modifyTheme);
}

bool DeviceInfoWidgetBase::getOverViewInfo(ArticleStruct& info)
{
    info = overviewInfo_;

    return info.value.isEmpty() == false;
}

void DeviceInfoWidgetBase::initContextMenu()
{
    contextMenu_ = new QMenu;
    QAction* refreshAction = new QAction(DApplication::translate("Main", "Refresh"), this);
    connect(refreshAction, &QAction::triggered, \
            [this]()
            {
                MainWindow* mainWindow = dynamic_cast<MainWindow*>(parent()->parent()->parent());
                if(mainWindow)
                {
                    mainWindow->refresh();
                }
            }
    );
    contextMenu_->addAction(refreshAction);


    QAction* exportAction = new QAction(DApplication::translate("Main", "Export"));
    connect(exportAction, &QAction::triggered, this, &DeviceInfoWidgetBase::onExportToFile);
    contextMenu_->addAction(exportAction);
}

void DeviceInfoWidgetBase::addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QStringList& names, const QStringList& contents, const QFont& font)
{
    for(int i = 0; i < names.size(); ++i)
    {
        QLabel* nameLabel = new DLabel( DApplication::translate("Main", names.at(i).toStdString().data() ) + ":", downWidget_);
        //nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        nameLabel->setMinimumWidth(NameLength_);
        nameLabel->setFont(font);
        //nameLabel->setMinimumHeight(SubRowHeight_);
        //nameLabel->setReadOnly(true);

        QLabel* contentLabel = new DLabel(contents.at(i), downWidget_);
        //contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        contentLabel->setWordWrap(true);
        contentLabel->setMinimumWidth(WidgetWidth - NameLength_);
        //contentLabel->setMinimumHeight(SubRowHeight_);
        contentLabel->setFont(font);

        //QSizePolicy::Policy policy = QSizePolicy::Minimum;
        //contentLabel->setSizePolicy( policy );
        //nameLabel->setFixedHeight(RowHeight_);
        //contentLabel->setSizePolicy(policy, policy);
        //contentLabel->setMargin(0);
        //contentLabel->setContentsMargins(0,0,0,0);
        //contentLabel->setFixedWidth(200);
        //contentLabel->setFixedHeight(RowHeight_);

        di->nameLabels.push_back(nameLabel);
        di->contentLabels.push_back(contentLabel);
        //contentLabel->adjustSize();
        ly->addWidget(nameLabel, i, 0, Qt::AlignTop);
        ly->addWidget(contentLabel, i, 1, Qt::AlignTop);
        //gridLayout->setRowMinimumHeight(i, RowHeight_);
        //nameLabel->setFixedHeight(contentLabel->height());
        //increaseHeight+= contentLabel->height();
    }
}

void DeviceInfoWidgetBase::addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QList<ArticleStruct>& articles, const QFont& font)
{
    int i = 0;
    foreach(auto article, articles)
    {
        if( article.autoHide &&  article.isValid() == false )
        {
            continue;
        }

        QLabel* nameLabel = new DLabel( DApplication::translate("Main", article.name.toStdString().data()) + ":", downWidget_ );
        //nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        nameLabel->setMinimumWidth(NameLength_);
        //nameLabel->setMinimumHeight(SubRowHeight_);
        nameLabel->setFont(font);

        QLabel* contentLabel = new DLabel( article.valueTranslate ? \
                                           DApplication::translate("Main", article.value.toStdString().data()): \
                                           article.value, downWidget_ );
        //contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        if(article.externalLinks)
        {
            contentLabel->setOpenExternalLinks(true);
        }

        contentLabel->setWordWrap(true);
        contentLabel->setMinimumWidth(WidgetWidth - NameLength_);
        //contentLabel->setMinimumHeight(SubRowHeight_);
        contentLabel->setFont(font);

        di->nameLabels.push_back(nameLabel);
        di->contentLabels.push_back(contentLabel);
        ly->addWidget(nameLabel, i, 0, Qt::AlignTop);
        ly->addWidget(contentLabel, i, 1, Qt::AlignTop);
        ++i;
    }
}

//void DeviceInfoWidgetBase::DeviceInfoWidgetBase::setTitle(const QString& title)
//{
////    QStringList list;
////    addSubInfo( title, list, list);
//    return;

//    if(titleInfo_ && titleInfo_->title)
//    {
//        titleInfo_->title->setText(title);
//        return;
//    }

//    if(titleInfo_ == nullptr)
//    {
//       titleInfo_ = new DeviceInfo;
//    }

//    if(nullptr == titleInfo_->title)
//    {
//        titleInfo_->title = new DLabel(this);
//    }

//    if(isTitleFontInit_== false)
//    {
//        titleFont_ = titleInfo_->title->font();
//        titleFont_.setBold(true);
//        isTitleFontInit_ = true;
//    }

//    titleInfo_->title->setText( DApplication::translate("Main", title.toStdString().data() ));
//    titleInfo_->title->setFont(titleFont_);

//    vLayout_->insertWidget( vLayout_->count(), titleInfo_->title );
//}

void DeviceInfoWidgetBase::setCentralInfo(const QString& info)
{
    //initDownWidget();

    if(titleInfo_ == nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    downWidget_ = new DWidget(this);
    downWidget_->setAutoFillBackground(true);
    downWidgetLayout = new QVBoxLayout;
    downWidget_->setLayout(downWidgetLayout);

    if( false == info.isEmpty() )
    {
        titleInfo_->title = new DLabel( DApplication::translate("Main", info.toStdString().data()), downWidget_);
        titleInfo_->title->setFont(titleFont_);

        auto hLayout = new QHBoxLayout;
        hLayout->addWidget(titleInfo_->title, 1, Qt::AlignmentFlag::AlignCenter);

        downWidgetLayout->addStretch(2);
        downWidgetLayout->addLayout(hLayout, 1);
        downWidgetLayout->addStretch(3);
    }

    vLayout_->addWidget(downWidget_ );
}

void DeviceInfoWidgetBase::addInfo(const QString& title, const QStringList& names, const QStringList& contents)
{
    initDownWidget();

    if(titleInfo_ == nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    QVBoxLayout* vly = new QVBoxLayout;

    if( false == title.isEmpty() )
    {
        titleInfo_->title = new DLabel( DApplication::translate("Main", title.toStdString().data()) + ":", downWidget_);
        titleInfo_->title->setFont(titleFont_);
        titleInfo_->title->setFixedHeight(RowHeight_);
        vly->addWidget(titleInfo_->title);
    }

    QHBoxLayout* hly = new QHBoxLayout;
//    hly->setMargin(0);
//    hly->setContentsMargins(0,0,0,0);

    hly->addSpacing(10);

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setSpacing(10);
//    gridLayout->setMargin(0);
//    gridLayout->setContentsMargins(0,0,0,0);
    gridLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    hly->addLayout(gridLayout);
    vly->addLayout(hly);

    addLabelToGridLayout(titleInfo_, gridLayout, names, contents, infoFont_);
    infoWidget_ = new DWidget(this);
    infoWidget_->setLayout(vly);
    downWidgetLayout->insertWidget(downWidgetLayout->count(), infoWidget_);

    downWidget_->adjustSize();

    verticalScrollBarMaxValue += infoWidget_->height();
    downWidgetScrollArea_->verticalScrollBar()->setRange(0, verticalScrollBarMaxValue);
}

void DeviceInfoWidgetBase::addInfo(const QString& title, const QList<ArticleStruct>& articles)
{
    initDownWidget();

    if(titleInfo_ == nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    QVBoxLayout* vly = new QVBoxLayout;

    if( false == title.isEmpty() )
    {
        titleInfo_->title = new DLabel( DApplication::translate("Main", title.toStdString().data()) + ":", downWidget_);
        titleInfo_->title->setFont(titleFont_);
        titleInfo_->title->setFixedHeight(RowHeight_);
        vly->addWidget(titleInfo_->title);
    }

    QHBoxLayout* hly = new QHBoxLayout;
//    hly->setMargin(0);
//    hly->setContentsMargins(0,0,0,0);
    hly->addSpacing(10);

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setSpacing(10);
//    gridLayout->setMargin(0);
//    gridLayout->setContentsMargins(0,0,0,0);
    gridLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    hly->addLayout(gridLayout);
    vly->addLayout(hly);

    addLabelToGridLayout(titleInfo_, gridLayout, articles, infoFont_);
    infoWidget_ = new DWidget(this);
    infoWidget_->setLayout(vly);
    downWidgetLayout->insertWidget(downWidgetLayout->count(), infoWidget_);

    downWidget_->adjustSize();

    verticalScrollBarMaxValue += infoWidget_->height();
    downWidgetScrollArea_->verticalScrollBar()->setRange(0, verticalScrollBarMaxValue);
}

//void DeviceInfoWidgetBase::addSubInfo(const QString& subTitle, const QStringList& names, const QStringList& contents)
//{
//    initDownWidget();

//    QVBoxLayout* vly = new QVBoxLayout;
//    vly->setMargin(0);
//    vly->setContentsMargins(0,0,0,0);

//    DeviceInfo subInfo;
//    if( false == subTitle.isEmpty() )
//    {
//        subInfo.title = new DLabel(subTitle, downWidget_);
//        if(isSubTitleFontInit_== false)
//        {
//            subTitleFont_ = subInfo.title->font();
//            subTitleFont_.setBold(true);
//            isSubTitleFontInit_ = true;
//        }

//        subInfo.title->setFont(subTitleFont_);
//        subInfo.title->setFixedHeight(RowHeight_);
//        vly->addWidget(subInfo.title);
//    }

//    QHBoxLayout* hly = new QHBoxLayout;
//    hly->setMargin(0);
//    hly->setContentsMargins(0,0,0,0);
//    hly->addSpacing(20);
//    QGridLayout* gridLayout = new QGridLayout;

//    gridLayout->setSpacing(0);
//    gridLayout->setMargin(0);
//    gridLayout->setContentsMargins(0,0,0,0);

//    addLabelToGridLayout(&subInfo, gridLayout, names, contents);

//    hly->addLayout(gridLayout);
//    vly->addLayout(hly);
//    DWidget* subInfoWidget = new DWidget(this);
//    subInfoWidget->setLayout(vly);
//    if( (titleInfo_ && titleInfo_->nameLabels.size() > 0) || deviceInfos_.size() > 0 )
//    {
//        vly->setContentsMargins(0, 0, 0, 0);
//    }
//    subinfoWidgetList_.push_back(subInfoWidget);

//    downWidgetLayout->insertWidget(downWidgetLayout->count()-1, subInfoWidget);
//    deviceInfos_.push_back(subInfo);
//    downWidget_->adjustSize();
//}

void DeviceInfoWidgetBase::addSubInfo(const QString& subTitle, const QList<ArticleStruct>& articles)
{
    initDownWidget();
    QVBoxLayout* vly = new QVBoxLayout;

    DeviceInfo subInfo;
    if(false == subTitle.isEmpty())
    {
        subInfo.title = new DLabel( DApplication::translate("Main", subTitle.toStdString().data()) + ":", downWidget_);
        subInfo.title->setFont(subTitleFont_);
        subInfo.title->setFixedHeight(RowHeight_);
        vly->addWidget( subInfo.title );
    }

    QHBoxLayout* hly = new QHBoxLayout;

    hly->addSpacing(10);
    QGridLayout* gridLayout = new QGridLayout;

    addLabelToGridLayout(&subInfo, gridLayout, articles, labelFont_);

    hly->addLayout(gridLayout);
    vly->addLayout(hly);

    DWidget* subInfoWidget = new DWidget(this);
    subInfoWidget->setLayout(vly);
//    if( infoWidget_ || subinfoWidgetList_.size() > 0 )
//    {
//        vly->setContentsMargins(0, 20, 0, 0);
//    }

    subinfoWidgetList_.push_back(subInfoWidget);
    downWidgetLayout->insertWidget( downWidgetLayout->count(), subInfoWidget);
    deviceInfos_.push_back(subInfo);
    downWidget_->adjustSize();

    verticalScrollBarMaxValue += subInfoWidget->height();
    downWidgetScrollArea_->verticalScrollBar()->setRange(0, verticalScrollBarMaxValue);
}

void DeviceInfoWidgetBase::addTable(const QStringList& headers, const QList<QStringList>& contentsList)
{
    if(tableWidget_ == nullptr)
    {
        tableWidget_ = new DTableWidget(this);
        tableWidget_->setHorizontalHeader(new TableWidgetAlwaysActiveHeaderView(Qt::Orientation::Horizontal, this) );

        tableWidget_->setMinimumHeight(178);
        tableWidget_->setMaximumHeight(500);
        tableWidget_->setVerticalScrollBar(new DScrollBar(this));
        tableWidget_->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        tableWidget_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        //tableWidget_->horizontalHeader()->setClickable(false);
        //tableWidget_->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        //tableWidget_->setAutoFillBackground(true);


        //tableWidget_->setAttribute(Qt::WA_ShowWithoutActivating);

        tableWidget_->setWindowFlags(/*Qt::Tool | Qt::FramelessWindowHint|*/Qt::WindowStaysOnTopHint);
        //tableWidget_->overrideWindowState(Qt::WindowState::WindowActive);
        //tableWidget_->setEnabled(true);
        tableWidget_->horizontalHeader()->clearMask();
        tableWidget_->horizontalHeader()->setFont(tableHeaderFont_);

        tableWidget_->setRowCount(contentsList.size());
        tableWidget_->setColumnCount(headers.size());
        QStringList translaterHeaders;
        foreach(auto header, headers)
        {
            translaterHeaders.push_back(DApplication::translate("Main", header.toStdString().data()));
        }
        tableWidget_->setHorizontalHeaderLabels(translaterHeaders);

        tableWidget_->horizontalHeader()->setObjectName("DeviceInfoHeaderView");

        auto changeTheme = [this](){
            QPalette pa = this->palette();

            QBrush bash_brush( QColor(255,255,255,255) );

            pa.setColorGroup(QPalette::Inactive, pa.base(), pa.button(),pa.light(),pa.dark(),pa.mid(),pa.text(),pa.brightText(),pa.base(), pa.window());
            pa.setColorGroup(QPalette::Active, pa.base(), pa.button(),pa.light(),pa.dark(),pa.mid(),pa.text(),pa.brightText(),pa.base(), pa.window());

            DApplicationHelper::instance()->setPalette(tableWidget_, pa);
            DApplicationHelper::instance()->setPalette(tableWidget_->horizontalHeader(), pa);

            //tableWidget_->horizontalHeader()->setAutoFillBackground(true);

//            for(int itemIndex = 0; itemIndex < tableWidget_->horizontalHeader()->count(); ++itemIndex)
//            {
//                tableWidget_->horizontalHeaderItem(itemIndex)->setBackground(bash_brush);
//                //tableWidget_->horizontalHeader()->model()->setData(itemIndex, 0, Qt::BackgroundRole);
//            }

            tableWidget_->horizontalHeader()->setStyleSheet( "QHeaderView#DeviceInfoHeaderView{ background-color: white; }");
        };

        changeTheme();

        connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, changeTheme);


        tableWidget_->horizontalHeader()->setContentsMargins(0,0,0,0);
        //tableWidget_->horizontalHeader()->setHighlightSections(false);
        //tableWidget_->horizontalHeader()->setFrameShape(QFrame::Shape::NoFrame);
        //tableWidget_->setAttribute(Qt::WA_TranslucentBackground);
        tableWidget_->horizontalHeader()->setAttribute(Qt::WA_TranslucentBackground);

        tableWidget_->horizontalHeader()->setAttribute(Qt::WA_TranslucentBackground);


        tableWidget_->horizontalHeader()->setAttribute(Qt::WA_OpaquePaintEvent);
        //tableWidget_->setAttribute(Qt::WA_NoSystemBackground);


        //tableWidget_->horizontalHeader()->setFrameShadow(QFrame::Shadow::Plain);
        tableWidget_->horizontalHeader()->setAutoFillBackground(false);

        tableWidget_->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        tableWidget_->setSelectionMode(QAbstractItemView::SingleSelection);

        tableWidget_->verticalHeader()->setVisible(false);
        //tableWidget_->setGridStyle( Qt::PenStyle::NoPen);
        tableWidget_->setShowGrid(false);

        tableWidget_->setAlternatingRowColors(true);
        //tableWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);

        tableWidget_->setFrameShape(QFrame::Shape::NoFrame);

        connect(tableWidget_, &DTableWidget::itemClicked, this, &DeviceInfoWidgetBase::OnCurrentItemClicked);
    }

    //tableWidget_->horizontalHeader()->setSectionResizeMode(headers.size() - 2, QHeaderView::Stretch);
    tableWidget_->horizontalHeader()->setDefaultAlignment(Qt::AlignmentFlag::AlignLeft);
//    tableWidget_->setColumnWidth(0, 100);
//    tableWidget_->setColumnWidth(1, 220);
//    tableWidget_->setColumnWidth(2, 140);
//    tableWidget_->setColumnWidth(3, 100);
//    tableWidget_->setColumnWidth(4, 50);

    vLayout_->insertWidget(0, tableWidget_);
    vLayout_->insertSpacing(1, 8);

    for(int i = 0; i < contentsList.size(); ++i)
    {
        const QStringList& contents = contentsList[i];
        for(int j = 0; j < contents.size(); ++j )
        {
            QTableWidgetItem* item = new QTableWidgetItem(contents[j]);
            item->setFont(tableContentFont_);
            tableWidget_->setItem(i, j, item);
        }
    }

    //tableWidget_->clearSpans();

    //tableWidget_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    tableWidget_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //tableWidget_->resizeColumnsToContents();
    //tableWidget_->resizeRowsToContents();
}

void DeviceInfoWidgetBase::addStrecch()
{
    //vLayout_->addStretch(1);
}

void DeviceInfoWidgetBase::initDownWidget()
{
    if( downWidgetScrollArea_ )
    {
        return;
    }

    downWidgetScrollArea_ = new DScrollArea(this);
    //downWidgetScrollArea_->verticalScrollBar()->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    //setVerticalScrollMode(ScrollPerPixel);
    downWidgetScrollArea_->setFrameShape(QFrame::NoFrame);
    downWidget_ = new DWidget(downWidgetScrollArea_);

    //downWidget_->setBackgroundRole(QPalette::Base);
    //downWidget_->setAutoFillBackground(true);

    //downWidget_->setFixedHeight(100);
    //downWidget_->setBaseSize(700, 100);
    //downWidget_->setFixedWidth(700);
    //downWidget_->setFixedHeight(20);
    downWidget_->setMinimumWidth(WidgetWidth);
    downWidgetLayout = new QVBoxLayout;
    downWidgetLayout->setMargin(0);
    //downWidgetLayout->setContentsMargins(0, 8, 0, 0);
    downWidgetLayout->setSpacing(0);
    downWidget_->setLayout(downWidgetLayout);
    //downWidgetLayout->addStretch(1);
    downWidgetScrollArea_->setWidget(downWidget_);
    //downWidget_->setSizePolicy(QSizePolicy::Minimum);
    //downWidgetScrollArea_->setFixedHeight(100);
    downWidgetScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    //downWidgetScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    vLayout_->insertWidget( vLayout_->count(), downWidgetScrollArea_);
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

//void DeviceInfoWidgetBase::paintEvent(QPaintEvent *event)
//{
//    QPainter painter(this);
//        painter.setRenderHint(QPainter::Antialiasing, true);

//        // Save pen
//        QPen oldPen = painter.pen();

//        painter.setRenderHint(QPainter::Antialiasing);
//        DPalette pa = DApplicationHelper::instance()->palette(this);
//        painter.setBrush(QBrush(pa.color(DPalette::Base)));
//        QColor penColor = pa.color(DPalette::FrameBorder);
//        penColor.setAlphaF(0.05);
//        painter.setPen(QPen(penColor));

//        QRectF rect = this->rect();
//        rect.setX(0.5);
//        rect.setY(0.5);
//        rect.setWidth(rect.width() - 0.5);
//        rect.setHeight(rect.height() - 0.5);

//        QPainterPath painterPath;
//        painterPath.addRoundedRect(rect, 8, 8);
//        painter.drawPath(painterPath);

//        // Restore the pen
//        painter.setPen(oldPen);
//}

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

void DeviceInfoWidgetBase::contextMenuEvent(QContextMenuEvent *event)
{
    contextMenu_->exec(event->globalPos());
}

void DeviceInfoWidgetBase::OnCurrentItemClicked(QTableWidgetItem *item)
{
    if(item == nullptr)
    {
        return;
    }

    int height = 0;
    if(infoWidget_)
    {
        height += infoWidget_->height();
    }


    int row = item->row();

    for(int i = 0; i < row; ++i )
    {
        height += subinfoWidgetList_.at(i)->height();
    }

    downWidgetScrollArea_->verticalScrollBar()->setValue(height);
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
                                                    tr("Text (*.txt);; Doc (*.doc);; Xls (*.xls);; Html (*.html)"), &selectFilter);

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

QTextStream& operator<<(QTextStream& ds, const DeviceInfo& di)
{
    if(di.title)
    {
        ds << di.title->text() << "\n";
    }


    for(int i = 0; i < di.nameLabels.size(); ++i)
    {
        ds.setFieldWidth(20);
        ds.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
        ds << di.nameLabels[i]->text();
        ds.setFieldWidth(0);
        ds << di.contentLabels[i]->text()<< "\n";
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

    for(int i = 0; i < di.nameLabels.size(); ++i)
    {
        QString name = di.nameLabels[i]->text();
        QString content = di.contentLabels[i]->text();
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

    doc.addParagraph("\n");
    return true;
}

bool DeviceInfoWidgetBase::exportToDoc(const QString& docFile)
{
    Docx::Document doc(":/thirdlib/docx/doc_template/template.doc");
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

    for(int i = 0; i < di.nameLabels.size(); ++i)
    {
        xlsx.write(currentXlsRow_, 1, di.nameLabels[i]->text());
        xlsx.write(currentXlsRow_++, 2, di.contentLabels[i]->text());
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

    for(int i = 0; i < di.nameLabels.size(); ++i)
    {
        html.write("<tr>\n");
        html.write( QString("<td style=\"width:200px;text-align:left;white-space:pre;\">" + di.nameLabels[i]->text() + "</td>").toUtf8().data() );
        html.write( QString( "<td>" + di.contentLabels[i]->text() + "</td>\n").toUtf8().data() );
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
