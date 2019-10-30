#include "motherboardwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

MotherboardWidget::MotherboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Motherboard"))
{
    initWidget();
}

void MotherboardWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Motherboard")  + DApplication::translate("Main", " Info"));


    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    QStringList names = {   DApplication::translate("Main", "Model"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Chipset Family"),
                            DApplication::translate("Main", "Bios")  + DApplication::translate("Main", "Version"),
                            DApplication::translate("Main", "Features")
                        };

    ArticleStruct vendor("Vendor");
    vendor.queryData("dmidecode", "Base Board Information", "Manufacturer");
    articles.push_back(vendor);
    existArticles.insert("Manufacturer");

    ArticleStruct model("Model");
    //QString manufactor = DeviceInfoParserInstance.queryData("dmidecode", "Base Board Information", "Manufacturer");
    model.value = /*manufactor + " "+*/ DeviceInfoParserInstance.queryData("dmidecode", "Base Board Information", "Product Name");
    articles.push_back(model);
    existArticles.insert("Product Name");

    ArticleStruct version("Version");
    version.queryData("dmidecode", "Base Board Information", "Version");
    articles.push_back(version);
    existArticles.insert("Version");

    ArticleStruct chipsetFamily("Chipset Family");
    chipsetFamily.value = DeviceInfoParserInstance.fuzzyQueryData("lspci", "ISA bridge", "Subsystem");
    QRegExp rx("^[\\s\\S]*\\(([\\S]*)\\)$");
    if( rx.exactMatch(chipsetFamily.value) )
    {
        chipsetFamily.value =  rx.cap(1)+" " + DApplication::translate("Main", "Chipset Family");
    }
    articles.push_back(chipsetFamily);

    overviewInfo_.value =  model.value + " " + chipsetFamily.value;

    overviewInfo_.value.remove( vendor.value, Qt::CaseInsensitive);
    overviewInfo_.value = vendor.value + " " + overviewInfo_.value;

    ArticleStruct serial("Serial Number");
    serial.queryData("dmidecode", "Base Board Information", "Serial Number");
    articles.push_back(serial);
    existArticles.insert("Serial Number");

    existArticles.insert("Features");

    bool res = DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "Base Board Information", articles, existArticles);

    ArticleStruct SMBIOSVersion("SMBIOS Version");
    SMBIOSVersion.queryData("dmidecode", "SMBIOS", "version");
    articles.push_back(SMBIOSVersion);

    ArticleStruct features("Features");
    features.queryData("dmidecode", "Base Board Information", "Features");
    articles.push_back(features);

    if(res)
    {
        addInfo( "", articles );
    }

    articles.clear();
    existArticles.clear();

    vendor.queryData("dmidecode", "System Information", "Manufacturer");
    articles.push_back(vendor);
    existArticles.insert("Manufacturer");

    ArticleStruct productName("Product Name");
    productName.queryData("dmidecode", "System Information", "Product Name");
    articles.push_back(productName);
    existArticles.insert("Product Name");

    serial.queryData("dmidecode", "System Information", "Serial Number");
    articles.push_back(serial);
    existArticles.insert("Serial Number");

    version.queryData("dmidecode", "System Information", "Version");
    articles.push_back(version);
    existArticles.insert("Version");

    ArticleStruct uuid("UUID");
    uuid.queryData("dmidecode", "System Information", "UUID");
    articles.push_back(uuid);
    existArticles.insert("UUID");

    ArticleStruct wakeupType("Wake-up Type");
    wakeupType.queryData("dmidecode", "System Information", "Wake-up Type");
    articles.push_back(wakeupType);
    existArticles.insert("Wake-up Type");

    ArticleStruct sku("SKU Number");
    sku.queryData("dmidecode", "System Information", "SKU Number");
    articles.push_back(sku);
    existArticles.insert("SKU Number");

    existArticles.insert("Features");

    res = DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "System Information", articles, existArticles);

    //features.queryData("dmidecode", "System Information", "Features");
    //articles.push_back(features);

    if(res)
    {
        addSubInfo( "System Information", articles );
    }

    articles.clear();
    existArticles.clear();

    ArticleStruct biosVersion("Bios Version");
    biosVersion.queryData("dmidecode", "BIOS Information", "Version");
    articles.push_back(biosVersion);
    existArticles.insert("Version");

    ArticleStruct releaseDate("Release Date");
    releaseDate.queryData("dmidecode", "BIOS Information", "Release Date");
    articles.push_back(releaseDate);
    existArticles.insert("Release Date");

    ArticleStruct biosVendor("Bios Vendor");
    biosVendor.queryData("dmidecode", "BIOS Information", "Vendor");
    articles.push_back(biosVendor);
    existArticles.insert("Vendor");

    existArticles.insert("Characteristics");
    res = DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "BIOS Information", articles, existArticles);

    ArticleStruct characteristics("Characteristics");
    characteristics.queryData("dmidecode", "BIOS Information", "Characteristics");
    articles.push_back(characteristics);

    if(res)
    {
        addSubInfo( "Bios", articles );
    }

    articles.clear();
    existArticles.clear();

    articles.clear();
    existArticles.clear();

    ArticleStruct chassisVendor("Vendor");
    chassisVendor.queryData("dmidecode", "Chassis Information", "Manufacturer");
    articles.push_back(chassisVendor);
    existArticles.insert("Manufacturer");

    ArticleStruct chassisType("Type");
    chassisType.queryData("dmidecode", "Chassis Information", "Type");
    articles.push_back(chassisType);
    existArticles.insert("Type");

    ArticleStruct chassisSN("Serial Number");
    chassisSN.queryData("dmidecode", "Chassis Information", "Serial Number");
    articles.push_back(chassisSN);
    existArticles.insert("Serial Number");

    ArticleStruct chassisVersion("Version");
    chassisVersion.queryData("dmidecode", "Chassis Information", "Version");
    articles.push_back(chassisVersion);
    existArticles.insert("Version");

    res = DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "Chassis Information", articles, existArticles);

    if(res)
    {
        addSubInfo( "Chassis Information", articles );
    }

}
