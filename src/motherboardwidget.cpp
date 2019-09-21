#include "motherboardwidget.h"
#include "deviceinfoparser.h"

MotherboardWidget::MotherboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeMotherboard)
{
    setTitle(DeviceAttributeMotherboard + " " + DeviceAttributeInfo);

    QStringList names = {   DeviceAttributeModel,
                            DeviceAttributeManufacturer,
                            DeviceAttributeChipsetFamily,
                            DeviceAttributeBios+DeviceAttributeVersion,
                            DeviceAttributeFeatures
                        };

    QString chipsetFamily = DeviceInfoParserInstance.fuzzyQueryData("lspci", "ISA bridge", "Subsystem");
    QRegExp rx("^[\\s\\S]*\\(([\\S]*)\\)$");
    if( rx.exactMatch(chipsetFamily) )
    {
        chipsetFamily =  rx.cap(1)+" " + DeviceAttributeChipsetFamily;
    }

    QString manufactor = DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Manufacturer");

    QStringList contents = {
        manufactor + " "+ DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Product Name"),
        manufactor,
        chipsetFamily,
        DeviceInfoParserInstance.qureyData("dmidecode", "BIOS Information", "Version") + "("+ DeviceInfoParserInstance.qureyData("dmidecode", "BIOS Information", "Release Date") + ")",
        DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Features")
    };

    addInfo(names, contents);
}
