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
    setTitle(DApplication::translate("Main", "Computer Overview"));

    QStringList names = {   DApplication::translate("Main", "Model"),
                            DApplication::translate("Main", "Operating System"),
                            " ",
                            DApplication::translate("Main", "CPU"),
                            DApplication::translate("Main", "Motherboard"),
                            DApplication::translate("Main", "Memory"),
                            DApplication::translate("Main", "Disk"),
                            DApplication::translate("Main", "DisplayAdapter"),
                            DApplication::translate("Main", "Monitor"),
                            DApplication::translate("Main", "AudioAdapter"),
                            DApplication::translate("Main", "NetworkAdapter")
                        };
    QString pName = DeviceInfoParserInstance.qureyData("dmidecode", "System Information", "Product Name");
    QString ver = DeviceInfoParserInstance.qureyData("dmidecode", "System Information", "Version");
    QString model = pName + " " + ver;
    if(ver.contains("Not Specified", Qt::CaseInsensitive) )
    {
        model = pName;
    }

    else if(false == pName.contains(" ") && ver.contains(" "))  //	Product Name: 10N9CTO1WW
                                                                //  Version: ThinkCentre M910t-N000
    {
        model = ver;
    }

    QString os;
    DeviceInfoParserInstance.getOSInfo(os);
    QString chipsetFamily = DeviceInfoParserInstance.fuzzyQueryData("lspci", "ISA bridge", "Subsystem");
    QRegExp rx("^[\\s\\S]*\\(([\\S]*)\\)$");
    if( rx.exactMatch(chipsetFamily) )
    {
        chipsetFamily = "(" + rx.cap(1)+" "+DApplication::translate("Main", "Chipset Family") + ")";
    }

    QString motherboard =  DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Manufacturer") \
            + " " + DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Product Name") + chipsetFamily;

    QString cpu = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Model name");
    cpu.remove(" CPU", Qt::CaseInsensitive);

    QString memory = DeviceInfoParserInstance.qureyData("lshw", "Computer_core_memory", "size");
    memory.replace("GiB","GB");

    QStringList memList = DeviceInfoParserInstance.getMemorynameList();
    QStringList detailMem;
    foreach(const QString& mem, memList)
    {
        QString rank = DeviceInfoParserInstance.qureyData("dmidecode", mem, "Rank");
        if(rank == DApplication::translate("Main", "Unknown")|| rank == "Unknown" )
        {
            continue;
        }

        auto vendor = DeviceInfoParserInstance.qureyData("dmidecode", mem, "Manufacturer");
        vendor += " ";
        vendor += DeviceInfoParserInstance.qureyData("dmidecode", mem, "Type");
        vendor += " ";
        vendor += DeviceInfoParserInstance.qureyData("dmidecode", mem, "Speed");
        if( false == detailMem .contains(vendor) )
        {
            detailMem.push_back(vendor);
        }
    }

    if(detailMem.size() > 0)
    {
        memory += " (";
        memory += detailMem.join("/");
        memory += ")";
    }

    QString disk;
    QStringList diskList = DeviceInfoParserInstance.getDisknameList();
    if(diskList.size() > 0)
    {
        QString primaryDisk = diskList[0];
        disk = DeviceInfoParserInstance.qureyData("lshw", primaryDisk, "product");
        disk += " (";
        QString diskSize = DeviceInfoParserInstance.qureyData("lshw", primaryDisk, "size");
        diskSize.replace("GiB","GB");
        QRegExp reg("^[\\s\\S]*\\(([\\s\\S]+)\\)$");
        if(reg.exactMatch(diskSize))
        {
            disk += reg.cap(1);
        }
        else
            disk += diskSize;
        disk += ")";
    }

    QString displayAdapter;
        QStringList displayadapterList = DeviceInfoParserInstance.getDiaplayadapterList();
    if(displayadapterList.size() > 0)
    {
        displayAdapter = DeviceInfoParserInstance.qureyData("lspci", displayadapterList[0], "Name");
        displayAdapter.remove(" Corporation", Qt::CaseInsensitive);
        int index = displayAdapter.indexOf('(');
        if(index > 0)
        {
            displayAdapter = displayAdapter.mid(0, index);
        }

        displayAdapter += " (";
        displayAdapter += DeviceInfoParserInstance.qureyData("lspci", displayadapterList[0], "Memory");
        QString manufacturer = DeviceInfoParserInstance.qureyData("lspci", displayadapterList[0], "Subsystem").split(" ").first();
        if(DApplication::translate("Main", "Unknown") != manufacturer)
        {
            displayAdapter += " / ";
            displayAdapter += manufacturer;
        }

        displayAdapter += ")";
    }


    QString monitor;
    QStringList monitorList = DeviceInfoParserInstance.getMonitorList();
    if(monitorList.size() > 0)
    {
        QString vendor = DeviceInfoParserInstance.qureyData("hwinfo", monitorList[0], "Vendor");
        QString abb;
        QRegExp rx("(^[\\s\\S]*)\"([\\s\\S]+)\"$");
        if( rx.exactMatch(vendor) )
        {
            abb = rx.cap(1).trimmed();
            vendor = rx.cap(2).trimmed();
        }

        monitor = DeviceInfoParserInstance.qureyData("hwinfo", monitorList[0], "Model");
        monitor = monitor.remove("\"");
        monitor.remove(abb);

        QString size = DeviceInfoParserInstance.qureyData("hwinfo", monitorList[0], "Size");
        QRegExp re("^([\\d]*)x([\\d]*) mm$");
        if( re.exactMatch(size) )
        {
            double width = re.cap(1).toDouble()/2.54;
            double height = re.cap(2).toDouble()/2.54;
            double inch = std::sqrt(width*width + height*height)/10.0;
            monitor += " (";
            monitor += QString::number(inch,10, 1);
            monitor += " " + DApplication::translate("Main", "inch");
            monitor += ")";
        }
        else
        {
            monitor += " (";
            monitor += size;
            monitor += ")";
        }
    }

    QString networkAdapter;
    QStringList networkadapterList = DeviceInfoParserInstance.getNetworkadapterList();
    if(networkadapterList.size() > 0)
    {
        networkAdapter = DeviceInfoParserInstance.qureyData("lshw", networkadapterList[0], "vendor");
        networkAdapter += " ";
        networkAdapter += DeviceInfoParserInstance.qureyData("lshw", networkadapterList[0], "product");
    }

    QStringList contents = {
        model,
        os,
        " ",
        cpu,
        motherboard,
        memory,
        disk,
        displayAdapter,
        monitor,
        DeviceInfoParserInstance.fuzzyQueryData("lspci", "Audio device", "Name"),
        networkAdapter,
    };

    addInfo( names, contents);
}
