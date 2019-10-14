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

DWIDGET_USE_NAMESPACE

bool DeviceInfoWidgetBase::isTitleFontInit_ = false;
QFont DeviceInfoWidgetBase::titleFont_;
bool DeviceInfoWidgetBase::isSubTitleFontInit_ = false;
QFont DeviceInfoWidgetBase::subTitleFont_;

static const int NameLength_ = 130;
static const int RowHeight_ = 25;
static const int SubRowHeight_ = 20;

static const int WidgetWidth = 640;
static const int WidgetHeight = 480;

DeviceInfoWidgetBase::DeviceInfoWidgetBase(QWidget *parent, const QString& deviceName) : QWidget(parent), deviceName_(deviceName)
{
    vLayout_ = new QVBoxLayout;
    setLayout(vLayout_);
    setMinimumHeight(WidgetHeight);
    initContextMenu();
    //setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
}

DeviceInfoWidgetBase::~DeviceInfoWidgetBase()
{
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

void DeviceInfoWidgetBase::addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QStringList& names, const QStringList& contents)
{
    for(int i = 0; i < names.size(); ++i)
    {
        QLabel* nameLabel = new DLabel(names.at(i), downWidget_);
        nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        nameLabel->setMinimumWidth(NameLength_);
        //nameLabel->setReadOnly(true);

        QLabel* contentLabel = new DLabel(contents.at(i), downWidget_);
        contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        contentLabel->setWordWrap(true);
        contentLabel->setMinimumWidth(WidgetWidth - NameLength_);

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

void DeviceInfoWidgetBase::addLabelToGridLayout(DeviceInfo* di, QGridLayout* ly, const QList<ArticleStruct>& articles)
{
    int i = 0;
    foreach(auto article, articles)
    {
        if( article.autoHide && article.value == DApplication::translate("Main", "Unknown") )
        {
            continue;
        }

        QLabel* nameLabel = new DLabel( article.name, downWidget_ );
        nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        nameLabel->setMinimumWidth(NameLength_);

        QLabel* contentLabel = new DLabel( article.value, downWidget_ );
        contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        contentLabel->setWordWrap(true);
        contentLabel->setMinimumWidth(WidgetWidth - NameLength_);

        di->nameLabels.push_back(nameLabel);
        di->contentLabels.push_back(contentLabel);
        ly->addWidget(nameLabel, i, 0, Qt::AlignTop);
        ly->addWidget(contentLabel, i, 1, Qt::AlignTop);
        ++i;
    }
}

void DeviceInfoWidgetBase::DeviceInfoWidgetBase::setTitle(const QString& title)
{
    if(titleInfo_ && titleInfo_->title)
    {
        titleInfo_->title->setText(title);
        return;
    }

    if(titleInfo_ == nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    if(nullptr == titleInfo_->title)
    {
        titleInfo_->title = new DLabel(this);
    }

    if(isTitleFontInit_== false)
    {
        titleFont_ = titleInfo_->title->font();
        titleFont_.setBold(true);
        isTitleFontInit_ = true;
    }

    titleInfo_->title->setText(title);
    titleInfo_->title->setFont(titleFont_);
    vLayout_->insertWidget( vLayout_->count(), titleInfo_->title);
}

void DeviceInfoWidgetBase::addInfo(const QStringList& names, const QStringList& contents)
{
    initDownWidget();

    if(titleInfo_ == nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    QHBoxLayout* hly = new QHBoxLayout;

    hly->addSpacing(10);

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    gridLayout->setContentsMargins(0,0,0,0);
    gridLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    hly->addLayout(gridLayout);

    addLabelToGridLayout(titleInfo_, gridLayout, names, contents);
    infoWidget_ = new DWidget(this);
    infoWidget_->setLayout(hly);
    downWidgetLayout->insertWidget(downWidgetLayout->count()-1, infoWidget_);

    downWidget_->adjustSize();
}

void DeviceInfoWidgetBase::addInfo(const QList<ArticleStruct>& articles)
{
    initDownWidget();

    if(titleInfo_ == nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    QHBoxLayout* hly = new QHBoxLayout;

    hly->addSpacing(10);

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    gridLayout->setContentsMargins(0,0,0,0);
    gridLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    hly->addLayout(gridLayout);

    addLabelToGridLayout(titleInfo_, gridLayout, articles);
    infoWidget_ = new DWidget(this);
    infoWidget_->setLayout(hly);
    downWidgetLayout->insertWidget(downWidgetLayout->count()-1, infoWidget_);

    downWidget_->adjustSize();
}

void DeviceInfoWidgetBase::addSubInfo(const QString& subTitle, const QStringList& names, const QStringList& contents)
{
    initDownWidget();

    if( (titleInfo_ && titleInfo_->nameLabels.size() > 0) || deviceInfos_.size() > 0 )
    {
        downWidgetLayout->insertSpacing(downWidgetLayout->count()-1, 20);
    }

    DeviceInfo subInfo;
    if( false == subTitle.isEmpty() )
    {
        subInfo.title = new DLabel(subTitle, downWidget_);
        if(isSubTitleFontInit_== false)
        {
            subTitleFont_ = subInfo.title->font();
            subTitleFont_.setBold(true);
            isSubTitleFontInit_ = true;
        }

        subInfo.title->setFont(subTitleFont_);
        subInfo.title->setFixedHeight(RowHeight_);
        downWidgetLayout->insertWidget(downWidgetLayout->count()-1, subInfo.title);
    }

    QHBoxLayout* hly = new QHBoxLayout;
    hly->addSpacing(20);
    QGridLayout* gridLayout = new QGridLayout;

    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    gridLayout->setContentsMargins(0,0,0,0);

    addLabelToGridLayout(&subInfo, gridLayout, names, contents);

    hly->addLayout(gridLayout);
    DWidget* subInfoWidget = new DWidget(this);
    subInfoWidget->setLayout(hly);
    subinfoWidgetList_.push_back(subInfoWidget);

    downWidgetLayout->insertWidget(downWidgetLayout->count()-1, subInfoWidget);
    deviceInfos_.push_back(subInfo);
    downWidget_->adjustSize();
}

void DeviceInfoWidgetBase::addSubInfo(const QString& subTitle, const QList<ArticleStruct>& articles)
{
    initDownWidget();

    if( (titleInfo_ && titleInfo_->nameLabels.size() > 0) || deviceInfos_.size() > 0 )
    {
        downWidgetLayout->insertSpacing(downWidgetLayout->count()-1, 20);
    }

    DeviceInfo subInfo;
    if(false == subTitle.isEmpty())
    {
        subInfo.title = new DLabel(subTitle, downWidget_);
        if(isSubTitleFontInit_== false)
        {
            subTitleFont_ = subInfo.title->font();
            subTitleFont_.setBold(true);
            isSubTitleFontInit_ = true;
        }

        subInfo.title->setFont(subTitleFont_);
        subInfo.title->setFixedHeight(RowHeight_);
        downWidgetLayout->insertWidget(downWidgetLayout->count()-1, subInfo.title);
    }

    QHBoxLayout* hly = new QHBoxLayout;
    hly->addSpacing(20);
    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    gridLayout->setContentsMargins(0,0,0,0);

    addLabelToGridLayout(&subInfo, gridLayout, articles);

    hly->addLayout(gridLayout);
    DWidget* subInfoWidget = new DWidget(this);
    subInfoWidget->setLayout(hly);
    subinfoWidgetList_.push_back(subInfoWidget);
    downWidgetLayout->insertWidget(downWidgetLayout->count()-1, subInfoWidget);
    deviceInfos_.push_back(subInfo);
    downWidget_->adjustSize();
}

void DeviceInfoWidgetBase::addTable(const QStringList& headers, const QList<QStringList>& contentsList)
{
    if(tableWidget_ == nullptr)
    {
        tableWidget_ = new DTableWidget(this);
        tableWidget_->setFixedHeight(150);
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
        tableWidget_->horizontalHeader()->setHighlightSections(false);
        tableWidget_->horizontalHeader()->setFrameShape(QFrame::Shape::NoFrame);
        tableWidget_->setAttribute(Qt::WA_TranslucentBackground);
        tableWidget_->horizontalHeader()->setAttribute(Qt::WA_TranslucentBackground);
        tableWidget_->horizontalHeader()->setFrameShadow(QFrame::Shadow::Sunken);

        tableWidget_->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        tableWidget_->setSelectionMode(QAbstractItemView::SingleSelection);

        tableWidget_->verticalHeader()->setVisible(false);
        tableWidget_->setGridStyle( Qt::PenStyle::NoPen);
        tableWidget_->setShowGrid(false);

        tableWidget_->setAlternatingRowColors(true);
        tableWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);

        tableWidget_->setFrameShape(QFrame::Shape::NoFrame);

        connect(tableWidget_, &DTableWidget::currentItemChanged, this, &DeviceInfoWidgetBase::OnCurrentItemChanged);
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


    for(int i = 0; i < contentsList.size(); ++i)
    {
        const QStringList& contents = contentsList[i];
        for(int j = 0; j < contents.size(); ++j )
        {
            QTableWidgetItem* item = new QTableWidgetItem(contents[j]);
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
    vLayout_->addStretch(1);
}

void DeviceInfoWidgetBase::initDownWidget()
{
    if( downWidgetScrollArea_ )
    {
        return;
    }

    downWidgetScrollArea_ = new DScrollArea(this);
    downWidgetScrollArea_->setFrameShape(QFrame::NoFrame);
    downWidget_ = new DWidget(downWidgetScrollArea_);
    //downWidget_->setFixedHeight(100);
    //downWidget_->setBaseSize(700, 100);
    //downWidget_->setFixedWidth(700);
    //downWidget_->setFixedHeight(20);
    downWidget_->setMinimumWidth(WidgetWidth);
    downWidgetLayout = new QVBoxLayout;
    downWidgetLayout->setMargin(0);
    downWidgetLayout->setContentsMargins(0,0,0,0);
    downWidgetLayout->setSpacing(0);
    downWidget_->setLayout(downWidgetLayout);
    downWidgetLayout->addStretch(1);
    downWidgetScrollArea_->setWidget(downWidget_);
    //downWidget_->setSizePolicy(QSizePolicy::Minimum);
    //downWidgetScrollArea_->setFixedHeight(100);
    downWidgetScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    //downWidgetScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    vLayout_->insertWidget( vLayout_->count(), downWidgetScrollArea_);
}

QString DeviceInfoWidgetBase::getDeviceName()
{
    return deviceName_;
}

void DeviceInfoWidgetBase::deviceListClicked()
{
    if(currentRow_ == -1)
    {
        return;
    }

    currentRow_ = -1;

    for(int i = 0; i < subinfoWidgetList_.size(); ++i )
    {
        subinfoWidgetList_.at(i)->setVisible(true);
    }

    if(infoWidget_)
    {
        infoWidget_->setVisible(true);
    }

    downWidget_->adjustSize();
}

void DeviceInfoWidgetBase::contextMenuEvent(QContextMenuEvent *event)
{
    contextMenu_->exec(event->globalPos());
}

void DeviceInfoWidgetBase::OnCurrentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    if(current == nullptr)
    {
        return;
    }

    int row = current->row();

    if( row == currentRow_ )
    {
        return;
    }

    currentRow_ = row;

    for(int i = 0; i < subinfoWidgetList_.size(); ++i )
    {
        if(row == -1 || row == i)
        {
            subinfoWidgetList_.at(i)->setVisible(true);
            //subinfoLayoutList_.at(i)-
        }
        else
        {
            subinfoWidgetList_.at(i)->setVisible(false);
        }
    }

    infoWidget_->setVisible(row == -1);

    downWidget_->adjustSize();
}


bool DeviceInfoWidgetBase::onExportToFile()
{
   QString selectFilter;
   QString exportFile = DFileDialog::getSaveFileName(this, tr("Export File"), "./" + deviceName_ + QDate::currentDate().toString("yyyyMMdd") .remove(QRegExp("\\s")) + ".txt", \
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
