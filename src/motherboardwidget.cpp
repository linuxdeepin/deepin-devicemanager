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
    setTitle(DApplication::translate("Main", "Motherboard")  + DApplication::translate("Main", " Info"));

    QStringList names = {   DApplication::translate("Main", "Model"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Chipset Family"),
                            DApplication::translate("Main", "Bios")  + DApplication::translate("Main", "Version"),
                            DApplication::translate("Main", "Features")
                        };

    QString chipsetFamily = DeviceInfoParserInstance.fuzzyQueryData("lspci", "ISA bridge", "Subsystem");
    QRegExp rx("^[\\s\\S]*\\(([\\S]*)\\)$");
    if( rx.exactMatch(chipsetFamily) )
    {
        chipsetFamily =  rx.cap(1)+" " + DApplication::translate("Main", "Chipset Family");
    }

    QString manufactor = DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Manufacturer");

    QStringList contents = {
        manufactor + " "+ DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Product Name"),
        manufactor,
        chipsetFamily,
        DeviceInfoParserInstance.qureyData("dmidecode", "BIOS Information", "Version") + " ("+ DeviceInfoParserInstance.qureyData("dmidecode", "BIOS Information", "Release Date") + ")",
        DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Features")
    };

    addInfo(names, contents);
}
