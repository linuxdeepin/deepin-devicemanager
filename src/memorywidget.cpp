#include "memorywidget.h"
#include "deviceinfoparser.h"
#include "DTableWidget"
#include <QVBoxLayout>
#include "tablewidgetalwaysfocus.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

MemoryWidget::MemoryWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Memory"))
{
    initWidget();
    connect(tableWidget_, &DTableWidget::currentItemChanged, this, &MemoryWidget::OnCurrentItemChanged);
}

void MemoryWidget::initWidget()
{
    initTableWdiget();
    updateWholeDownWidget();
}


void MemoryWidget::initTableWdiget()
{
    canUpgrade_ = false;
    QStringList headers = {DApplication::translate("Main", "Bank"),  DApplication::translate("Main", "Size"), DApplication::translate("Main", "Type"),DApplication::translate("Main", "Speed"), DApplication::translate("Main", "Statu")};
    QList<QStringList> tabList;

    QStringList memList = DeviceInfoParserInstance.getMemorynameList();
    foreach(const QString& mem, memList)
    {
        QString rank = DeviceInfoParserInstance.qureyData("dmidecode", mem, "Rank");
        if(rank == DApplication::translate("Main", "Unknown") || rank == "Unknown" )
        {
            canUpgrade_ = true;
        }

        QString rankStr = DeviceInfoParserInstance.qureyData("dmidecode", mem, "Rank");
        QStringList tab = {
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Locator"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Size"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Type"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Speed"),
            (  rankStr == DApplication::translate("Main", "Unknown")\
            || rankStr ==  "Unknown" ) ? DApplication::translate("Main", "Bad"):DApplication::translate("Main", "Good")
        };

        tabList.push_back(tab);
    }

    addTable(headers, tabList);
}

void MemoryWidget::updateWholeDownWidget()
{
    setTitle(DApplication::translate("Main", "Memory")  + DApplication::translate("Main", " Info"));

    QStringList names = {   DApplication::translate("Main", "Slot"),
                             DApplication::translate("Main", "Size"),
                            DApplication::translate("Main", "Maximum Capacity"),
                            DApplication::translate("Main", "Upgradeable")
                        };

    QString memSize = DeviceInfoParserInstance.qureyData("lshw", "Computer_core_memory", "size");
    memSize.replace("GiB", " GB");
    QStringList contents = {
        DeviceInfoParserInstance.qureyData("dmidecode", "Physical Memory Array", "Number Of Devices"),
        memSize,
        DeviceInfoParserInstance.qureyData("dmidecode", "Physical Memory Array", "Maximum Capacity"),
        canUpgrade_?DApplication::translate("Main", "Yes"):DApplication::translate("Main", "No")
    };

    addInfo(names, contents);

    QStringList subNames = {    DApplication::translate("Main", "Vendor"),
                                 DApplication::translate("Main", "Size"),
                                DApplication::translate("Main", "Type"),
                                DApplication::translate("Main", "Speed"),
                                DApplication::translate("Main", "Serial Number"),
                                DApplication::translate("Main", "Model"),
                                DApplication::translate("Main", "Statu"),
                                DApplication::translate("Main", "Configured Voltage")
                        };

    QStringList memList = DeviceInfoParserInstance.getMemorynameList();
    foreach(const QString& mem, memList)
    {
        QString rank = DeviceInfoParserInstance.qureyData("dmidecode", mem, "Rank");
        if(rank == DApplication::translate("Main", "Unknown") || rank == "Unknown" )
        {
            continue;
        }

        QStringList contents = {
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Manufacturer"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Size"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Type"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Speed"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Serial Number"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Part Number"),
            DApplication::translate("Main", "Good"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Configured Voltage")
        };

        addSubInfo(DeviceInfoParserInstance.qureyData("dmidecode", mem, "Locator"), subNames, contents);
    }
}

void MemoryWidget::updateDownWidget(const QString& currentChannel)
{
    QStringList memList = DeviceInfoParserInstance.getMemorynameList();
    QStringList subNames = {    DApplication::translate("Main", "Vendor"),
                                 DApplication::translate("Main", "Size"),
                                DApplication::translate("Main", "Type"),
                                DApplication::translate("Main", "Speed"),
                                DApplication::translate("Main", "Serial Number"),
                                DApplication::translate("Main", "Model"),
                                DApplication::translate("Main", "Statu"),
                                DApplication::translate("Main", "Configured Voltage")
                        };

    foreach(const QString& mem, memList)
    {
        QString locator = DeviceInfoParserInstance.qureyData("dmidecode", mem, "Locator");
        if(currentChannel == locator)
        {
            QString rank = DeviceInfoParserInstance.qureyData("dmidecode", mem, "Rank");
            QStringList contents = {
                DeviceInfoParserInstance.qureyData("dmidecode", mem, "Manufacturer"),
                DeviceInfoParserInstance.qureyData("dmidecode", mem, "Size"),
                DeviceInfoParserInstance.qureyData("dmidecode", mem, "Type"),
                DeviceInfoParserInstance.qureyData("dmidecode", mem, "Speed"),
                DeviceInfoParserInstance.qureyData("dmidecode", mem, "Serial Number"),
                DeviceInfoParserInstance.qureyData("dmidecode", mem, "Part Number"),
                DeviceInfoParserInstance.qureyData("dmidecode", mem, "Rank") == "Unknown"?DApplication::translate("Main", "Bad"):DApplication::translate("Main", "Good"),
                DeviceInfoParserInstance.qureyData("dmidecode", mem, "Configured Voltage")
            };

            setTitle(locator);
            addSubInfo("", subNames, contents);
            break;
        }
    }
}

void MemoryWidget::deviceListClicked()
{
    if(-1 == currentRow_)
    {
        return;
    }

    currentRow_ = -1;
    if(downWidgetScrollArea_)
    {
        deviceInfos_.clear();
        titleInfo_->nameLabels.clear();
        titleInfo_->contentLabels.clear();
        vLayout_->removeWidget(downWidgetScrollArea_);
        delete downWidget_;
        delete downWidgetScrollArea_;
        downWidgetScrollArea_ = nullptr;
    }

    initDownWidget();
    updateWholeDownWidget();
    tableWidget_->setCurrentItem(nullptr);
    return;
}

void MemoryWidget::OnCurrentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    if(current == nullptr)
    {
        return;
    }

    int row = current->row();

    if(row == currentRow_)
    {
        return;
    }

    if(downWidgetScrollArea_)
    {
        deviceInfos_.clear();
        titleInfo_->nameLabels.clear();
        titleInfo_->contentLabels.clear();
        vLayout_->removeWidget(downWidgetScrollArea_);
        delete downWidget_;
        delete downWidgetScrollArea_;
        downWidgetScrollArea_ = nullptr;
    }

    currentRow_ = row;
    QString memroy = tableWidget_->item( row, 0 )->text();
    initDownWidget();
    updateDownWidget( memroy );
}
