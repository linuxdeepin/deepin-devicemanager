#include "deviceinfowidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>

DWIDGET_USE_NAMESPACE

DeviceInfoWidget::DeviceInfoWidget(QWidget *parent) : QWidget(parent)
{
    vLayout_ = new QVBoxLayout;
    vLayout_->addStretch(1);
    setLayout(vLayout_);
    setMinimumWidth(700);
}

void DeviceInfoWidget::DeviceInfoWidget::setTitle(const QString& title)
{
    if(nullptr == title_)
    {
        title_ = new QLabel(this);
    }

    title_->setText(title);
    vLayout_->insertWidget(vLayout_->count() -1, title_);
}

void DeviceInfoWidget::addSubInfo(const QString& subTitle, const QStringList& names, const QStringList& contents)
{
    QHBoxLayout* hly = new QHBoxLayout;
    hly->addSpacing(10);
    QGridLayout* gridLayout = new QGridLayout;
    hly->addLayout(gridLayout);
    vLayout_->insertLayout(vLayout_->count() -1,hly);

    DeviceInfo subInfo;
    if(false == subTitle.isEmpty())
    {
        subInfo.subTitle = new QLabel(subTitle, this);
        gridLayout->addWidget(subInfo.subTitle, 0, 0);
    }

    for(int i = 0; i < names.size(); ++i)
    {
        QLabel* nameLabel = new QLabel(names.at(i), this);
        QLabel* contentLabel = new QLabel(contents.at(i), this);
        subInfo.nameLabels.push_back(nameLabel);
        subInfo.contentLabels.push_back(contentLabel);
        gridLayout->addWidget(nameLabel, i+1, 0);
        gridLayout->addWidget(contentLabel, i+1, 1);
    }

    deviceInfos_.push_back(subInfo);
}

void DeviceInfoWidget::addTable(const QStringList& headers, const QList<QStringList>& contentsList)
{
    if(tableWidget_ == nullptr)
    {
        tableWidget_ = new Dtk::Widget::DTableWidget(this);
    }

    tableWidget_->setRowCount(contentsList.size());
    tableWidget_->setColumnCount(headers.size());

    tableWidget_->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

    tableWidget_->verticalHeader()->setVisible(false);
    tableWidget_->setGridStyle( Qt::PenStyle::NoPen);

    tableWidget_->setHorizontalHeaderLabels(headers);
    tableWidget_->resizeRowsToContents();
    tableWidget_->horizontalHeader()->setSectionResizeMode(headers.size() - 2, QHeaderView::Stretch);
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
