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

    ArticleStruct model("Model");
    QString manufactor = DeviceInfoParserInstance.queryData("dmidecode", "Base Board Information", "Manufacturer");
    model.value = manufactor + " "+ DeviceInfoParserInstance.queryData("dmidecode", "Base Board Information", "Product Name");
    articles.push_back(model);
    existArticles.insert("Product Name");

    ArticleStruct vendor("Vendor");
    vendor.queryData("dmidecode", "Base Board Information", "Manufacturer");
    articles.push_back(vendor);
    existArticles.insert("Manufacturer");

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

    overviewInfo_.value =  model.value + chipsetFamily.value;

    ArticleStruct serial("Serial Number");
    serial.queryData("dmidecode", "Base Board Information", "Serial Number");
    articles.push_back(serial);
    existArticles.insert("Serial Number");

    ArticleStruct features("Features");
    features.queryData("dmidecode", "Base Board Information", "Features");
    articles.push_back(features);
    existArticles.insert("Features");

    DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "Base Board Information", articles, existArticles);

    existArticles.clear();
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

    DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "System Information", articles, existArticles);

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

    ArticleStruct characteristics("Characteristics");
    characteristics.queryData("dmidecode", "BIOS Information", "Characteristics");
    articles.push_back(characteristics);
    existArticles.insert("Characteristics");
    DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "BIOS Information", articles, existArticles);

    addInfo( "", articles );
}
