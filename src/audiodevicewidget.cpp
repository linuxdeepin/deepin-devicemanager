#include "audiodevicewidget.h"
#include "deviceinfoparser.h"
#include "math.h"
#include <QDate>
#include <DApplication>

DWIDGET_USE_NAMESPACE

AudiodeviceWidget::AudiodeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Audio Device"))
{
    initWidget();
}

void AudiodeviceWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Audio Device")  + DApplication::translate("Main", " Info"));
    QStringList multimediaList = DeviceInfoParserInstance.getMultimediaList();
    QStringList inputdeviceList = DeviceInfoParserInstance.getInputAudioDeviceList();

    if( multimediaList.size() + inputdeviceList.size() < 1 )
    {
        setCentralInfo("No Audio Device found!");
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(auto multimedia, multimediaList)
    {
        articles.clear();
        existArticles.clear();

//        ArticleStruct type("Type");
//        type.value = "AudioAdapter";
//        articles.push_back(type);

        ArticleStruct busInfo("Bus info");
        busInfo.value = DeviceInfoParserInstance.queryData("lshw", multimedia, "bus info");
        QRegExp reg("^pci@[0-9]*:([\\s\\S]*)$");

        QString pci_bus;
        if(reg.exactMatch(busInfo.value))
        {
            pci_bus= reg.cap(1);
        }

        QString lspciDeviceName;
        DeviceInfoParserInstance.fuzzeyQueryKey("lspci", pci_bus, lspciDeviceName);

        ArticleStruct name("Name");
        name.queryData("lspci", lspciDeviceName, "Name");
        if(name.value == DApplication::translate("Main", "Unknown"))
        {
            name.queryData( "lshw", multimedia, "product");
        }
        name.value.remove( " Corporation", Qt::CaseInsensitive );
        int index = name.value.indexOf('(');
        if(index > 0)
        {
            name.value = name.value.mid(0, index);
        }


        existArticles.insert("product");


        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", multimedia, "vendor");

        ArticleStruct description("Description");
        description.queryData("lshw", multimedia, "description");

        existArticles.insert("description");


        if(name.value == vendor.value)
        {
            name.value = description.value;
        }

        articles.push_back(name);
        articles.push_back(vendor);
        articles.push_back(description);
        existArticles.insert("vendor");


        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct version("Version");
        version.queryData( "lshw", multimedia, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width("Width");
        width.queryData( "lshw", multimedia, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock("Clock");
        clock.queryData( "lshw", multimedia, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData( "lshw", multimedia, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", multimedia, articles, existArticles);
        addDevice( name.value , articles, multimediaList.size() + inputdeviceList.size() );

        if( multimediaList.size() + inputdeviceList.size() > 1 )
        {
            QStringList tab =
            {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }


        if( overviewInfo_.value.isEmpty() == true )
        {
            overviewInfo_.value = vendor.value;
            overviewInfo_.value += " ";
            overviewInfo_.value += name.value;
        }
    }

    foreach(const QString& device, inputdeviceList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        name.queryData( "catinput", device, "Name");
        name.value.remove("\"");
        articles.push_back(name);
        existArticles.insert("Name");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "catinput", device, "Vendor");
        articles.push_back(vendor);
        existArticles.insert("Vendor");

        ArticleStruct vesion("Version");
        vesion.queryData( "catinput", device, "Version");
        articles.push_back(vesion);
        existArticles.insert("Sysfs");

        ArticleStruct sysfs("Sysfs");
        sysfs.queryData( "catinput", device, "Sysfs");
        articles.push_back(sysfs);
        existArticles.insert("Sysfs");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("catinput", device, articles, existArticles);
        addSubInfo( name.value , articles );

        if( multimediaList.size() + inputdeviceList.size() > 1 )
        {
            QStringList tab =
            {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }
    }

    if( multimediaList.size() + inputdeviceList.size() > 1 )
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
}
