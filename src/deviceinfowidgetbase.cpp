#include "deviceinfowidgetbase.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include "DScrollBar"
#include <QSizePolicy>

DWIDGET_USE_NAMESPACE

DeviceInfoWidgetBase::DeviceInfoWidgetBase(QWidget *parent, const QString& deviceName) : QWidget(parent), deviceName_(deviceName)
{
    vLayout_ = new QVBoxLayout;
    //vLayout_->addStretch(1);
    setLayout(vLayout_);
    setMinimumWidth(700);
    //setAttribute(Qt::WidgetAttribute::WA_NoBackground);
}

DeviceInfoWidgetBase::~DeviceInfoWidgetBase()
{

}

void DeviceInfoWidgetBase::DeviceInfoWidgetBase::setTitle(const QString& title)
{
    if(titleInfo_==nullptr)
    {
       titleInfo_ = new DeviceInfo;
    }

    if(nullptr == titleInfo_->title)
    {
        titleInfo_->title = new QLabel(this);
    }

    titleInfo_->title->setText(title);
    vLayout_->insertWidget(vLayout_->count(), titleInfo_->title);
}

void DeviceInfoWidgetBase::addInfo(const QStringList& names, const QStringList& contents)
{
    initDownWidget();

    if(titleInfo_)
    {
       titleInfo_ = new DeviceInfo;
    }

    QHBoxLayout* hly = new QHBoxLayout;

    hly->addSpacing(10);
    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setMargin(0);
    //gridLayout->setSizePo(QLayout::QSizePolicy::Minimum);
    hly->addLayout(gridLayout);
    downWidgetLayout->insertLayout(downWidgetLayout->count()-1, hly);

    int increaseHeight = 0;

    for(int i = 0; i < names.size(); ++i)
    {
        QLabel* nameLabel = new QLabel(names.at(i), this);
        QLabel* contentLabel = new QLabel(contents.at(i), this);
        nameLabel->setFixedHeight(30);
        contentLabel->setFixedHeight(30);
        titleInfo_->nameLabels.push_back(nameLabel);
        titleInfo_->contentLabels.push_back(contentLabel);
        gridLayout->addWidget(nameLabel, i+1, 0);
        gridLayout->addWidget(contentLabel, i+1, 1);
        increaseHeight+= 30;
    }

    downWidget_->setFixedHeight(downWidget_->height() + increaseHeight);
}

void DeviceInfoWidgetBase::addLinefeed()
{

}

void DeviceInfoWidgetBase::addSubInfo(const QString& subTitle, const QStringList& names, const QStringList& contents)
{
    initDownWidget();

    downWidgetLayout->insertSpacing(downWidgetLayout->count()-1, 25);

    int increaseHeight = 25;

    DeviceInfo subInfo;
    if(false == subTitle.isEmpty())
    {
        subInfo.title = new QLabel(subTitle, this);
        subInfo.title->setFixedHeight(30);
        downWidgetLayout->insertWidget(downWidgetLayout->count()-1, subInfo.title);
        increaseHeight += 30;
    }

    QHBoxLayout* hly = new QHBoxLayout;

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setMargin(0);
    hly->setMargin(0);
    hly->addSpacing(10);
    hly->addLayout(gridLayout);

    downWidgetLayout->insertLayout(downWidgetLayout->count()-1, hly);

    for(int i = 0; i < names.size(); ++i)
    {
        QLabel* nameLabel = new QLabel(names.at(i), this);
        QLabel* contentLabel = new QLabel(contents.at(i), this);
        nameLabel->setFixedHeight(30);
        contentLabel->setFixedHeight(30);
        subInfo.nameLabels.push_back(nameLabel);
        subInfo.contentLabels.push_back(contentLabel);
        gridLayout->addWidget(nameLabel, i+1, 1);
        gridLayout->addWidget(contentLabel, i+1, 2);
        increaseHeight+= 30;
    }

    downWidget_->setFixedHeight(downWidget_->height() + increaseHeight);

    deviceInfos_.push_back(subInfo);
}

void DeviceInfoWidgetBase::addTable(const QStringList& headers, const QList<QStringList>& contentsList)
{
    if(tableWidget_ == nullptr)
    {
        tableWidget_ = new Dtk::Widget::DTableWidget(this);
        tableWidget_->setFixedHeight(150);
        tableWidget_->setVerticalScrollBar(new DScrollBar(this));
        tableWidget_->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        tableWidget_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        //tableWidget_->horizontalHeader()->setClickable(false);
    }

    tableWidget_->setRowCount(contentsList.size());
    tableWidget_->setColumnCount(headers.size());

    tableWidget_->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    tableWidget_->setSelectionMode(QAbstractItemView::SingleSelection);

    tableWidget_->verticalHeader()->setVisible(false);
    tableWidget_->setGridStyle( Qt::PenStyle::NoPen);
    tableWidget_->setShowGrid(false);

    tableWidget_->setHorizontalHeaderLabels(headers);
    tableWidget_->resizeRowsToContents();
    tableWidget_->horizontalHeader()->setSectionResizeMode(headers.size() - 2, QHeaderView::Stretch);
    tableWidget_->horizontalHeader()->setDefaultAlignment(Qt::AlignmentFlag::AlignLeft);
    tableWidget_->setColumnWidth(0, 200);
    tableWidget_->setAlternatingRowColors(true);
    tableWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    vLayout_->insertWidget(0, tableWidget_);


    for(int i = 0; i < contentsList.size(); ++i)
    {
        const QStringList& contents = contentsList[i];
        for(int j = 0; j < contents.size(); ++j )
        {
            tableWidget_->setItem(i, j,new QTableWidgetItem(contents[j]));
        }
    }

}

void DeviceInfoWidgetBase::initDownWidget()
{
    if(downWidget_)
    {
        return;
    }

    DScrollArea* downWidgetScrollArea_ = new DScrollArea(this);
    downWidgetScrollArea_->setFrameShape(QFrame::NoFrame);
    downWidget_ = new DWidget(downWidgetScrollArea_);
    //downWidget_->setFixedHeight(100);
    downWidget_->setBaseSize(700, 100);
    //downWidget_->setFixedWidth(700);
    downWidget_->setMinimumWidth(700);
    downWidgetLayout = new QVBoxLayout;
    downWidget_->setLayout(downWidgetLayout);
    downWidgetLayout->addStretch(1);
    downWidgetScrollArea_->setWidget(downWidget_);
    //downWidget_->setSizePolicy(QSizePolicy::Minimum);
    //downWidgetScrollArea_->setFixedHeight(100);
    downWidgetScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    //downWidgetScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    vLayout_->insertWidget(vLayout_->count(), downWidgetScrollArea_);
}

QString DeviceInfoWidgetBase::getDeviceName()
{
    return deviceName_;
}
