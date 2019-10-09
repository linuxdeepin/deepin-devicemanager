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
    setTitle(DApplication::translate("Main", "Disk")  + DApplication::translate("Main", " Info"));

    QStringList mechanicalHardDiskNames = {
                            DApplication::translate("Main", "Model"),
                            DApplication::translate("Main", "Size"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Rotate Speed"),
                            DApplication::translate("Main", "Shape Characteristics"),
                            DApplication::translate("Main", "Interface"),
                            DApplication::translate("Main", "Power On Times"),
                            DApplication::translate("Main", "Serial Number"),
                            DApplication::translate("Main", "Speed")
                        };

    QStringList solidDiskNames = {
                            DApplication::translate("Main", "Model"),
                            DApplication::translate("Main", "Size"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Power On Times"),
                            DApplication::translate("Main", "Serial Number"),
                            DApplication::translate("Main", "Interface")
                        };

    QStringList diskList = DeviceInfoParserInstance.getDisknameList();

    foreach(const QString& disk, diskList)
    {   
        QString logicalName = DeviceInfoParserInstance.qureyData("lshw", disk, "logical name");
        DeviceInfoParserInstance.loadSmartctlDatabase(logicalName);

        QStringList lst = disk.split("_");
        QString interface = DApplication::translate("Main", "Unknown");
        if( lst.size() > 2 )
        {
            interface = lst.at(lst.size() -2);
        }

        QString size = DeviceInfoParserInstance.qureyData("lshw", disk, "size");
        size.replace("GiB","GB");

        QString rotationRate = DeviceInfoParserInstance.qureyData("smartctl", logicalName, "Rotation Rate");

        if(rotationRate != DApplication::translate("Main", "Unknown")) //mechanicalHardDisk
        {
            QString sataVersion = DeviceInfoParserInstance.qureyData("smartctl", logicalName, "SATA Version is");
            QString version;
            QString speed;
            int index = sataVersion.indexOf("current:");
            if(index>0)
            {
                speed = sataVersion.mid( index + sizeof("current:") );
                speed.remove(")");
                speed = speed.trimmed();

                interface = sataVersion.mid(0, index);
                interface.remove("(");
                interface = interface.trimmed();
            }

            QString factor = DeviceInfoParserInstance.qureyData("smartctl", logicalName, "Form Factor");
            factor.replace("inches", DApplication::translate("Main", "inch"));

            QStringList contents = {
                DeviceInfoParserInstance.qureyData("lshw", disk, "product"),
                size,
                DeviceInfoParserInstance.qureyData("lshw", disk, "vendor"),
                rotationRate,
                factor,
                interface,
                DeviceInfoParserInstance.qureyData("smartctl", logicalName, "Power_On_Hours") + DApplication::translate("Main", " Hours") + ", "
                + DeviceInfoParserInstance.qureyData("smartctl", logicalName, "Power_Cycle_Count") + DApplication::translate("Main", " Power Cycle Count"),
                DeviceInfoParserInstance.qureyData("lshw", disk, "serial"),
                speed
            };

            addSubInfo( "", mechanicalHardDiskNames, contents);
        }
        else    // SolidDisk
        {
            QStringList contents = {
                DeviceInfoParserInstance.qureyData("lshw", disk, "product"),
                size,
                DeviceInfoParserInstance.qureyData("lshw", disk, "vendor"),
                DeviceInfoParserInstance.qureyData("smartctl", logicalName, "Power_On_Hours") + DApplication::translate("Main", " Hours") + ", "
                + DeviceInfoParserInstance.qureyData("smartctl", logicalName, "Power_Cycle_Count") + DApplication::translate("Main", " Power Cycle Count"),
                DeviceInfoParserInstance.qureyData("lshw", disk, "serial"),
                interface
            };

            addSubInfo( "", solidDiskNames, contents);
        }


    }
}

