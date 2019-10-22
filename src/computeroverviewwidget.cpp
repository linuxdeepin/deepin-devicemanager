#include "computeroverviewwidget.h"
#include "deviceinfoparser.h"
#include "QRegExp"
#include "math.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

ComputerOverviewWidget::ComputerOverviewWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Computer Overview"))
{
    initWidget();
}

void ComputerOverviewWidget::initWidget()
{
    //setTitle( "Computer Overview" );

    QList<ArticleStruct> articles;

    ArticleStruct model("Model");
    QString pName = DeviceInfoParserInstance.queryData("dmidecode", "System Information", "Product Name");
    QString ver = DeviceInfoParserInstance.queryData("dmidecode", "System Information", "Version");
    model.value = pName + " " + ver;
    if(ver.contains("Not Specified", Qt::CaseInsensitive) )
    {
        model.value = pName;
    }
    else if(false == pName.contains(" ") && ver.contains(" "))  //	Product Name: 10N9CTO1WW  Version: ThinkCentre M910t-N000
    {
        model.value = ver;
    }
    articles.push_back(model);

    QStringList names = {
                            " ",
                            "",
                            "",
                            "",
                            "",
                            "",
                            "",
                            "AudioAdapter",
                            "",
                            "Bluetooth",
                            "Camera",
                            "Usb Device",
                            "Keyboard",
                            "Mouse",
                            "Other Inputdevice",
                            "Power",
                            "Printer",
                            "Other Inputdevice"
                        };


    ArticleStruct os("Operating System");
    DeviceInfoParserInstance.getOSInfo(os.value);
    articles.push_back(os);

    QString chipsetFamily = DeviceInfoParserInstance.fuzzyQueryData("lspci", "ISA bridge", "Subsystem");
    QRegExp rx("^[\\s\\S]*\\(([\\S]*)\\)$");
    if( rx.exactMatch(chipsetFamily) )
    {
        chipsetFamily = "(" + rx.cap(1)+" "+DApplication::translate("Main", "Chipset Family") + ")";
    }
    ArticleStruct motherboard("Motherboard");
    motherboard.value =  DeviceInfoParserInstance.queryData("dmidecode", "Base Board Information", "Manufacturer") \
            + " " + DeviceInfoParserInstance.queryData("dmidecode", "Base Board Information", "Product Name") + chipsetFamily;
    articles.push_back(motherboard);

    ArticleStruct memory("Memory");
    memory.value  = DeviceInfoParserInstance.queryData("lshw", "Computer_core_memory", "size");
    memory.value.replace("GiB","GB");

    QStringList memList = DeviceInfoParserInstance.getMemorynameList();
    QStringList detailMem;
    foreach(const QString& mem, memList)
    {
        QString rank = DeviceInfoParserInstance.queryData("dmidecode", mem, "Rank");
        if(rank == DApplication::translate("Main", "Unknown")|| rank == "Unknown" )
        {
            continue;
        }

        auto vendor = DeviceInfoParserInstance.queryData("dmidecode", mem, "Manufacturer");
        vendor += " ";
        vendor += DeviceInfoParserInstance.queryData("dmidecode", mem, "Type");
        vendor += " ";
        vendor += DeviceInfoParserInstance.queryData("dmidecode", mem, "Speed");
        if( false == detailMem .contains(vendor) )
        {
            detailMem.push_back(vendor);
        }
    }
    if(detailMem.size() > 0)
    {
        memory.value += " (";
        memory.value += detailMem.join("/");
        memory.value += ")";
    }
    articles.push_back(memory);

    ArticleStruct disk("Disk");
    QStringList diskList = DeviceInfoParserInstance.getDisknameList();
    if(diskList.size() > 0)
    {
        QString primaryDisk = diskList[0];
        disk.value = DeviceInfoParserInstance.queryData("lshw", primaryDisk, "product");
        disk.value += " (";
        QString diskSize = DeviceInfoParserInstance.queryData("lshw", primaryDisk, "size");
        diskSize.replace("GiB","GB");
        QRegExp reg("^[\\s\\S]*\\(([\\s\\S]+)\\)$");
        if(reg.exactMatch(diskSize))
        {
            disk.value += reg.cap(1);
        }
        else
            disk.value += diskSize;
        disk.value += ")";
    }
    articles.push_back(disk);

    ArticleStruct displayAdapter("DisplayAdapter");
    QStringList displayadapterList = DeviceInfoParserInstance.getDiaplayadapterList();
    if(displayadapterList.size() > 0)
    {
        displayAdapter = DeviceInfoParserInstance.queryData("lspci", displayadapterList[0], "Name");
        displayAdapter.value.remove(" Corporation", Qt::CaseInsensitive);
        int index = displayAdapter.value.indexOf('(');
        if(index > 0)
        {
            displayAdapter = displayAdapter.value.mid(0, index);
        }

        displayAdapter.value += " (";
        displayAdapter.value += DeviceInfoParserInstance.queryData("lspci", displayadapterList[0], "Memory");
        QString manufacturer = DeviceInfoParserInstance.queryData("lspci", displayadapterList[0], "Subsystem").split(" ").first();
        if(DApplication::translate("Main", "Unknown") != manufacturer)
        {
            displayAdapter.value += " / ";
            displayAdapter.value += manufacturer;
        }

        displayAdapter.value += ")";
    }
    articles.push_back(displayAdapter);

    ArticleStruct monitor("Monitor");
    QStringList monitorList = DeviceInfoParserInstance.getHwinfoMonitorList();
    if(monitorList.size() > 0)
    {
        QString vendor = DeviceInfoParserInstance.queryData("hwinfo", monitorList[0], "Vendor");
        QString abb;
        QRegExp rx("(^[\\s\\S]*)\"([\\s\\S]+)\"$");
        if( rx.exactMatch(vendor) )
        {
            abb = rx.cap(1).trimmed();
            vendor = rx.cap(2).trimmed();
        }

        monitor = DeviceInfoParserInstance.queryData("hwinfo", monitorList[0], "Model");
        monitor = monitor.value.remove("\"");
        monitor.value.remove(abb);

        QString size = DeviceInfoParserInstance.queryData("hwinfo", monitorList[0], "Size");
        QRegExp re("^([\\d]*)x([\\d]*) mm$");
        if( re.exactMatch(size) )
        {
            double width = re.cap(1).toDouble()/2.54;
            double height = re.cap(2).toDouble()/2.54;
            double inch = std::sqrt(width*width + height*height)/10.0;
            monitor.value += " (";
            monitor.value += QString::number(inch,10, 1);
            monitor.value += " " + DApplication::translate("Main", "inch");
            monitor.value += ")";
        }
        else
        {
            monitor.value += " (";
            monitor.value += size;
            monitor.value += ")";
        }
    }
    articles.push_back(monitor);

    ArticleStruct networkAdapter("NetworkAdapter");
    QStringList networkadapterList = DeviceInfoParserInstance.getNetworkadapterList();
    if(networkadapterList.size() > 0)
    {
        networkAdapter = DeviceInfoParserInstance.queryData("lshw", networkadapterList[0], "vendor");
        networkAdapter.value += " ";
        networkAdapter.value += DeviceInfoParserInstance.queryData("lshw", networkadapterList[0], "product");
    }
}

void ComputerOverviewWidget::setOverviewInfos( const QList<ArticleStruct>& others )
{
    //setTitle( "Computer Overview" );

    QList<ArticleStruct> articles;

    ArticleStruct model("Model");
    QString pName = DeviceInfoParserInstance.queryData("dmidecode", "System Information", "Product Name");
    QString ver = DeviceInfoParserInstance.queryData("dmidecode", "System Information", "Version");
    model.value = pName + " " + ver;
    if(ver.contains("Not Specified", Qt::CaseInsensitive) )
    {
        model.value = pName;
    }
    else if(false == pName.contains(" ") && ver.contains(" "))  //	Product Name: 10N9CTO1WW  Version: ThinkCentre M910t-N000
    {
        model.value = ver;
    }
    articles.push_back(model);


    ArticleStruct os("Operating System");
    DeviceInfoParserInstance.getOSInfo(os.value);
    articles.push_back(os);

    articles.append(others);

    addInfo("", articles);
}
