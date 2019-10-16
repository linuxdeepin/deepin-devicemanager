#include "diskwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

DiskWidget::DiskWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Disk"))
{
    initWidget();
}

void DiskWidget::initWidget()
{
    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    int i = 0;
    QStringList diskList = DeviceInfoParserInstance.getDisknameList();
    foreach(const QString& disk, diskList)
    {
        QString logicalName = DeviceInfoParserInstance.queryData("lshw", disk, "logical name");
        DeviceInfoParserInstance.loadSmartctlDatabase(logicalName);

        QString modelStr = DeviceInfoParserInstance.queryData("lshw", disk, "product");
        QString vendorStr = DeviceInfoParserInstance.queryData("lshw", disk, "vendor");
        QString mediaTypeStr = "Unknown";
        QString sizeStr = DeviceInfoParserInstance.queryData("lshw", disk, "size");
        sizeStr.replace("GiB","GB");

        articles.clear();
        existArticles.clear();

        ArticleStruct model("Model");
        model.value = modelStr;
        articles.push_back(model);
        existArticles.insert("product");

        ArticleStruct vendor("Vendor");
        vendor.value = vendorStr;
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct mediaType("Media Type");
        if( DeviceInfoParserInstance.isToolSuccess("smartctl") )
        {
            QString rotationRate = DeviceInfoParserInstance.queryData("smartctl", logicalName, "Rotation Rate");
            if(rotationRate == DApplication::translate("Main", "Unknown"))
            {
                mediaTypeStr = "SDD";
            }
            else
            {
                mediaTypeStr = "HDD";
            }
        }

        mediaType.value = mediaTypeStr;
        articles.push_back(mediaType);

        ArticleStruct size("Size");
        size.value = sizeStr;
        articles.push_back(size);
        existArticles.insert("size");

        ArticleStruct interface("Interface");
        QStringList lst = disk.split("_");
        interface.value = DApplication::translate("Main", "Unknown");
        if( lst.size() > 2 )
        {
            interface.value = lst.at(lst.size() -2);
        }
        articles.push_back(interface);

        ArticleStruct serial("Serial");
        serial.queryData("lshw", disk, "serial");
        articles.push_back(serial);
        existArticles.insert("serial");

        if( DeviceInfoParserInstance.isToolSuccess("smartctl") )
        {
            QString rotationRate = DeviceInfoParserInstance.queryData("smartctl", logicalName, "Rotation Rate");
            if(rotationRate == DApplication::translate("Main", "Unknown"))
            {
                mediaTypeStr = "SDD";
            }
            else
            {
                mediaTypeStr = "HDD";
                QString sataVersion = DeviceInfoParserInstance.queryData("smartctl", logicalName, "SATA Version is");
                QString version;
                QString speed;


                ArticleStruct rr("Rotation Rate");
                rr.value = rotationRate;
                articles.push_back(rr);
                int index = sataVersion.indexOf("current:");
                if(index>0)
                {
                    ArticleStruct speed("Speed");
                    speed.value = sataVersion.mid( index + sizeof("current:") );
                    speed.value.remove(")");
                    speed.value = speed.value.trimmed();
                    articles.push_back(speed);

                    interface.value = sataVersion.mid(0, index);
                    interface.value.remove("(");
                    interface.value = interface.value.trimmed();
                }

            }

            ArticleStruct rr("Rotation Rate");
            rr.value = rotationRate;
            articles.push_back(rr);

            ArticleStruct fromFactor("Form Factor");
            fromFactor.queryData("smartctl", logicalName, "Form Factor");
            fromFactor.value.replace("inches", DApplication::translate("Main", "inch"));
            articles.push_back(fromFactor);

            ArticleStruct powerOnHours("Power On Hours");
            powerOnHours.queryData("smartctl", logicalName, "Power_On_Hours");
            articles.push_back(powerOnHours);

            ArticleStruct powerCycleCount("Power Cycle Count");
            powerCycleCount.queryData("smartctl", logicalName, "Power_Cycle_Count");
            articles.push_back(powerCycleCount);
        }

        ArticleStruct version("Version");
        version.queryData("lshw", disk, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData("lshw", disk, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        ArticleStruct description("Description");
        description.queryData("lshw", disk, "description");
        articles.push_back(description);
        existArticles.insert("description");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", disk, articles, existArticles);

        addSubInfo( model.value, articles );

        QStringList tab =
        {
            modelStr,
            vendorStr,
            mediaTypeStr,
            sizeStr
        };

        tabList.push_back(tab);

        if( i == 0)
        {
            overviewInfo_.value = model.value;
            overviewInfo_.value += " (";
            QString diskSize = size.value;
            QRegExp reg("^[\\s\\S]*\\(([\\s\\S]+)\\)$");
            if(reg.exactMatch(diskSize))
            {
                overviewInfo_.value += reg.cap(1);
            }
            else
            {
                overviewInfo_.value += diskSize;
            }

            overviewInfo_.value += ")";
        }
    }

    if(diskList.size() > 1)
    {
        setTitle(DApplication::translate("Main", "Disk")  + DApplication::translate("Main", " Info"));
        QStringList headers = { "Model",  "Vendor", "Media Type", "Size"};
        addTable(headers, tabList);
    }
}

