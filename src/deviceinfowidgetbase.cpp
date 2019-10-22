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
#include "document.h"
#include "table.h"
//#include "thirdlib/docx/include/table.h"
#include "xlsxdocument.h"
#include <QFile>
#include "mainwindow.h"
#include "tablewidgetalwaysfocus.h"
#include <QDate>
#include "DApplication"
#include "DApplicationHelper"

DWIDGET_USE_NAMESPACE

bool DeviceInfoWidgetBase::isFontInit_ = false;
QFont DeviceInfoWidgetBase::titleFont_;
QFont DeviceInfoWidgetBase::subTitleFont_;
QFont DeviceInfoWidgetBase::infoFont_;
QFont DeviceInfoWidgetBase::labelFont_;
QFont DeviceInfoWidgetBase::tableContentFont_;

static const int NameLength_ = 130;
static const int RowHeight_ = 30;
static const int SubRowHeight_ = 40;

static const int WidgetWidth = 640;
static const int WidgetHeight = 770;

DeviceInfoWidgetBase::DeviceInfoWidgetBase(DWidget *parent, const QString& deviceName) : QWidget(parent)
{
    //setStyleSheet("QWidget{border-top-left-radius:15px;border-top-right-radius:5px;}");
    if(isFontInit_ == false)
    {
        QLabel* nameLabel = new DLabel("", this);
        titleFont_ = nameLabel->font();
        subTitleFont_ = titleFont_;
        labelFont_ = titleFont_;
        infoFont_ = titleFont_;
        tableContentFont_ = titleFont_;

        titleFont_.setPointSize(14);
        titleFont_.setBold(true);

        subTitleFont_.setPointSize(11);
        subTitleFont_.setBold(true);

        infoFont_.setPointSize(12);

        labelFont_.setPointSize(11);

        tableContentFont_.setPointSize(11);
        tableContentFont_.setBold(true);

        isFontInit_ = true;
    }

    overviewInfo_.name = deviceName;

    vLayout_ = new QVBoxLayout;

    vLayout_->setSpacing(0);
    vLayout_->setMargin(8);

    setAutoFillBackground(false);

    setLayout(vLayout_);
    setMinimumHeight(WidgetHeight);
    initContextMenu();

    DPalette pa = DApplicationHelper::instance()->palette(this);
    QColor base_color = palette().base().color();
    DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(base_color);

    if (ct == DGuiApplicationHelper::LightType) {
        pa.setColor(QPalette::Background, base_color);
        pa.setBrush(DPalette::LightLively, palette().base());
        pa.setBrush(DPalette::DarkLively, palette().base());
        //pa.setColor(QPalette::Light, base_color);
        //pa.setC
    } else {
        base_color = DGuiApplicationHelper::adjustColor(base_color, 0, 0, +5, 0, 0, 0, 0);
        pa.setColor(QPalette::Background, base_color);
    }

    DApplicationHelper::instance()->setPalette(this, pa);
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


    QAction* exportAction = new QAction(DApplication::translate("Main", "Export to File"));
    connect(exportAction, &QAction::triggered, this, &DeviceInfoWidgetBase::onExportToFile);
    contextMenu_->addAction(exportAction);
}

void DeviceInfoWidgetBase::addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QStringList& names, const QStringList& contents, const QFont& font)
{
    for(int i = 0; i < names.size(); ++i)
    {
        QLabel* nameLabel = new DLabel( DApplication::translate("Main", names.at(i).toStdString().data() ), downWidget_);
        nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        nameLabel->setMinimumWidth(NameLength_);
        nameLabel->setFont(font);
        //nameLabel->setMinimumHeight(SubRowHeight_);
        //nameLabel->setReadOnly(true);

        QLabel* contentLabel = new DLabel(contents.at(i), downWidget_);
        contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
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
        if( article.autoHide && ( article.value.isEmpty() == true || \
                                  article.value == DApplication::translate("Main", "Unknown")) )
        {
            continue;
        }

        QLabel* nameLabel = new DLabel( DApplication::translate("Main", article.name.toStdString().data() ), downWidget_ );
        nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        nameLabel->setMinimumWidth(NameLength_);
        //nameLabel->setMinimumHeight(SubRowHeight_);
        nameLabel->setFont(font);

        QLabel* contentLabel = new DLabel( article.value, downWidget_ );
        contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
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

void DeviceInfoWidgetBase::addInfo(const QString& title, const QStringList& names, const QStringList& contents)
{
    initDownWidget();

    if(titleInfo_ == nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    QVBoxLayout* vly = new QVBoxLayout;

    DeviceInfo subInfo;
    if( false == title.isEmpty() )
    {
        subInfo.title = new DLabel(title, downWidget_);
        subInfo.title->setFont(titleFont_);
        subInfo.title->setFixedHeight(RowHeight_);
        vly->addWidget(subInfo.title);
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

    DeviceInfo subInfo;
    if( false == title.isEmpty() )
    {
        subInfo.title = new DLabel(title, downWidget_);
        subInfo.title->setFont(titleFont_);
        subInfo.title->setFixedHeight(RowHeight_);
        vly->addWidget(subInfo.title);
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
        subInfo.title = new DLabel(subTitle, downWidget_);
        subInfo.title->setFont(subTitleFont_);
        subInfo.title->setFixedHeight(RowHeight_);
        vly->addWidget(subInfo.title);
    }

    QHBoxLayout* hly = new QHBoxLayout;

    hly->addSpacing(20);
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
        tableWidget_->setMinimumHeight(200);
        tableWidget_->setMaximumHeight(500);
        tableWidget_->setVerticalScrollBar(new DScrollBar(this));
        tableWidget_->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        tableWidget_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        //tableWidget_->horizontalHeader()->setClickable(false);
        //tableWidget_->setFocusPolicy(Qt::FocusPolicy::WheelFocus);
        //tableWidget_->setAutoFillBackground(false);
        //tableWidget_->setAttribute(Qt::WA_ShowWithoutActivating);
        //tableWidget_->setWindowFlags(/*Qt::Tool | Qt::FramelessWindowHint|*/Qt::WindowStaysOnTopHint);
        //tableWidget_->overrideWindowState(Qt::WindowState::WindowActive);
        //tableWidget_->setEnabled(true);
        //tableWidget_->horizontalHeader()->clearMask();
        DPalette pa = DApplicationHelper::instance()->palette(this);
        QColor base_color = palette().base().color();
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(base_color);

        if (ct == DGuiApplicationHelper::LightType) {
            pa.setColor(QPalette::Background, base_color);
            pa.setBrush(DPalette::DarkLively, palette().base());
            pa.setBrush(DPalette::FrameBorder, palette().base());
            pa.setColor(QPalette::Button, base_color);
            //pa.setColor(QPalette::Link, base_color);
            //pa.setColor(QPalette::LinkVisited, base_color);
            //pa.setColor(QPalette::AlternateBase, base_color);
            pa.setColor(QPalette::NoRole, base_color);
            //pa.setColor(QPalette::NColorRoles, base_color);
            //pa.setColor(QPalette::Light, base_color);
            //pa.setC
        } else {
            base_color = DGuiApplicationHelper::adjustColor(base_color, 0, 0, +5, 0, 0, 0, 0);
            pa.setColor(QPalette::Background, base_color);
        }

        DApplicationHelper::instance()->setPalette(tableWidget_->horizontalHeader(), pa);

        tableWidget_->horizontalHeader()->setContentsMargins(0,0,0,0);
        tableWidget_->horizontalHeader()->setHighlightSections(false);
        tableWidget_->horizontalHeader()->setFrameShape(QFrame::Shape::NoFrame);
        tableWidget_->setAttribute(Qt::WA_TranslucentBackground);
        tableWidget_->horizontalHeader()->setAttribute(Qt::WA_TranslucentBackground);
        //tableWidget_->horizontalHeader()->setFrameShadow(QFrame::Shadow::Plain);
        tableWidget_->horizontalHeader()->setAutoFillBackground(true);

        tableWidget_->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        tableWidget_->setSelectionMode(QAbstractItemView::SingleSelection);

        tableWidget_->verticalHeader()->setVisible(false);
        tableWidget_->setGridStyle( Qt::PenStyle::NoPen);
        tableWidget_->setShowGrid(false);

        tableWidget_->setAlternatingRowColors(true);
        tableWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);

        tableWidget_->setFrameShape(QFrame::Shape::NoFrame);

        //QItemSelectionModel *selectionModel = tableWidget_->selectionModel();
        //selectionModel->set

        connect(tableWidget_, &DTableWidget::itemClicked, this, &DeviceInfoWidgetBase::OnCurrentItemClicked);
    }

    tableWidget_->setRowCount(contentsList.size());
    tableWidget_->setColumnCount(headers.size());
    tableWidget_->setHorizontalHeaderLabels(headers);

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

    tableWidget_->clearSpans();

    //tableWidget_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    tableWidget_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget_->resizeColumnsToContents();
    tableWidget_->resizeRowsToContents();
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
   QString exportFile = DFileDialog::getSaveFileName(this, tr("Export File"), "./" + overviewInfo_.name + QDate::currentDate().toString("yyyyMMdd") .remove(QRegExp("\\s")) + ".txt", \
                        tr("TEXT (*.txt);; Doc (*.doc);; Xls (*.xls);; Html (*.html)"), &selectFilter);

   if(exportFile.isEmpty() == true)
   {
       return true;
   }

    if(selectFilter == "Text (*.txt)")
    {
        return exportToTxt(exportFile);
    }

    if(selectFilter == "Doc (*.doc)")
    {
        return exportToDoc(exportFile);
    }

    if(selectFilter == "Xls (*.xls)")
    {
        return exportToXls(exportFile);
    }

    if(selectFilter == "Html (*.html)")
    {
        return exportToHtml(exportFile);
    }

    return false;
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
        ds.setFieldWidth(tableWidget->columnWidth(col) *100.0 / tableWidget->width());
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
            ds.setFieldWidth(tableWidget->columnWidth(col) *100.0 / tableWidget->width());
            ds.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
            ds << tableWidget->item(row,col)->text();
            ds.setFieldWidth(0);
        }

        ds << "\n";
    }

    return ds;
}

bool DeviceInfoWidgetBase::exportToTxt(const QString& txtFile)
{
    QFile file( txtFile );
    if( false == file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QTextStream out(&file);

    if(tableWidget_)
    {
        out << tableWidget_ << "\n";
    }

    if(titleInfo_)
    {
        out << *titleInfo_ << "\n";
    }

    foreach(const DeviceInfo& di, deviceInfos_)
    {
        out << di << "\n";
    }

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
        doc.addHeading(di.title->text(), 5);
    }

    for(int i = 0; i < di.nameLabels.size(); ++i)
    {
        QString name = di.nameLabels[i]->text();
        QString content = di.contentLabels[i]->text();
        QString line;
        if(name.trimmed().isEmpty() == false || false == content.trimmed().isEmpty())
        {
            line = name + " : " + content;
        }

        doc.addParagraph(line);
    }

    return true;
}

bool DeviceInfoWidgetBase::exportToDoc(const QString& docFile)
{
    Docx::Document doc(":/thirdlib/docx/doc_template/template.doc");
    if(tableWidget_)
    {
        writeTabwidgetToDoc(tableWidget_, doc);
    }

    if(titleInfo_)
    {
        doc.addParagraph("\n");
        writeDeviceInfoToDoc(*titleInfo_, doc);
    }

    foreach(auto di, deviceInfos_)
    {
        doc.addParagraph("\n");
        writeDeviceInfoToDoc(di, doc);
    }

    doc.save(docFile);
    return true;
}

static int currentXlsRow = 1;

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
            boldFont.setFontBold(true);
            xlsx.write( currentXlsRow, col+1, item->text(), boldFont );
        }
    }
    ++currentXlsRow;

    for( int row = 0; row < tableWidget->rowCount(); ++row )
    {
        for( int col = 0; col < tableWidget->columnCount(); ++col )
        {
            xlsx.write( currentXlsRow, col+1, tableWidget->item(row,col)->text());
        }
        ++currentXlsRow;
    }

    ++currentXlsRow;

    return true;
}

bool writeDeviceInfoToXls(const DeviceInfo& di, QXlsx::Document& xlsx)
{
    if(di.title)
    {
        QXlsx::Format boldFont;
        boldFont.setFontBold(true);
        xlsx.write( currentXlsRow++, 1, di.title->text(), boldFont);
    }

    for(int i = 0; i < di.nameLabels.size(); ++i)
    {
        xlsx.write(currentXlsRow, 1, di.nameLabels[i]->text());
        xlsx.write(currentXlsRow++, 2, di.contentLabels[i]->text());
    }

    ++currentXlsRow;

    return true;
}

bool DeviceInfoWidgetBase::exportToXls(const QString& xlsFile)
{
    currentXlsRow = 1;
    QXlsx::Document xlsx;

    if(tableWidget_)
    {
        writeTabwidgetToXls(tableWidget_, xlsx);
    }

    if(titleInfo_)
    {
        writeDeviceInfoToXls(*titleInfo_, xlsx);
    }

    foreach(auto di, deviceInfos_)
    {
        writeDeviceInfoToXls(di, xlsx);
    }

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

bool DeviceInfoWidgetBase::exportToHtml(const QString& htmlFile)
{
    QFile html(htmlFile);
    if(false == html.open(QIODevice::OpenModeFlag::WriteOnly))
    {
        return false;
    }

    html.write("<!DOCTYPE html>\n");
    html.write("<html>\n");
    html.write("<body>\n");

    if(tableWidget_)
    {
        writeTabwidgetToHtml(tableWidget_, html);
        html.write("<br />\n");
    }

    if(titleInfo_)
    {
        writeDeviceInfoToHtml(*titleInfo_, html);
        html.write("<br />\n");
    }

    foreach(auto di, deviceInfos_)
    {
        writeDeviceInfoToHtml(di, html);
    }

    html.write("</body>\n");
    html.write("</html>\n");
    html.close();
    return true;
}
