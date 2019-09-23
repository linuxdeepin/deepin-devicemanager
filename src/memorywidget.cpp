#include "memorywidget.h"
#include "deviceinfoparser.h"

MemoryWidget::MemoryWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeMemory)
{
    bool canUpgrade = false;

    setTitle(DeviceAttributeMemory + " " + DeviceAttributeInfo);
    QStringList headers = {DeviceAttributeBank, DeviceAttributeSize, DeviceAttributeType,DeviceAttributeSpeed, DeviceAttributeStatu};
    QList<QStringList> tabList;

    QStringList memList = DeviceInfoParserInstance.getMemorynameList();
    foreach(const QString& mem, memList)
    {
        QString rank = DeviceInfoParserInstance.qureyData("dmidecode", mem, "Rank");
        if(rank == "Unknown")
        {
            canUpgrade = true;
        }

        QStringList tab = {
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Locator"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Size"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Type"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Speed"),
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Rank") == "Unknown"?DeviceAttributeBad:DeviceAttributeGood
        };

        tabList.push_back(tab);
    }

    addTable(headers, tabList);

    QStringList names = {   DeviceAttributeSlot,
                            DeviceAttributeSize,
                            DeviceAttributeMaximumCapacity,
                            DeviceAttributeUpgradeable
                        };

    QStringList contents = {
        DeviceInfoParserInstance.qureyData("dmidecode", "Physical Memory Array", "Number Of Devices"),
        DeviceInfoParserInstance.qureyData("lshw", "Computer_core_memory", "size"),
        DeviceInfoParserInstance.qureyData("dmidecode", "Physical Memory Array", "Maximum Capacity"),
        canUpgrade?DeviceAttributeYes:DeviceAttributeNo
    };

    addInfo(names, contents);

    QStringList subNames = {   DeviceAttributeVendor,
                            DeviceAttributeSize,
                            DeviceAttributeType,
                            DeviceAttributeSpeed,
                            DeviceAttributeSerialNumber,
                            DeviceAttributeModel,
                            DeviceAttributeStatu,
                            DeviceAttributeConfiguredVoltage
                        };

    foreach(const QString& mem, memList)
    {
        QString rank = DeviceInfoParserInstance.qureyData("dmidecode", mem, "Rank");
        if(rank == "Unknown")
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
            DeviceAttributeGood,
            DeviceInfoParserInstance.qureyData("dmidecode", mem, "Configured Voltage")
        };

        addSubInfo(DeviceInfoParserInstance.qureyData("dmidecode", mem, "Locator"), subNames, contents);
    }
}
