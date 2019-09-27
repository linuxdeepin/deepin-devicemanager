#include "diskwidget.h"
#include "deviceinfoparser.h"

DiskWidget::DiskWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeDisk)
{
    initWidget();
}

void DiskWidget::initWidget()
{
    setTitle(DeviceAttributeDisk + " " + DeviceAttributeInfo);

    QStringList mechanicalHardDiskNames = {
                            DeviceAttributeModel,
                            DeviceAttributeSize,
                            DeviceAttributeVendor,
                            DeviceAttributeRotateSpeed,
                            DeviceAttributeShapeCharacteristics,
                            DeviceAttributePowerOnTimes,
                            DeviceAttributeSerialNumber,
                            DeviceAttributeSataVersion,
                            DeviceAttributeSpeed
                        };

    QStringList diskList = DeviceInfoParserInstance.getDisknameList();

    foreach(const QString& disk, diskList)
    {
        //todo mechanicalHardDisk, SolidDisk
        QString size = DeviceInfoParserInstance.qureyData("lshw", disk, "size");
        size.replace("GiB","GB");

        QString sataVersion = DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "SATA Version is");
        QString version;
        QString speed;
        int index = sataVersion.indexOf("current:");
        if(index>0)
        {
            speed = sataVersion.mid( index + sizeof("current:") );
            speed.remove(")");
            speed = speed.trimmed();

            version = sataVersion.mid(0, index);
            version.remove("(");
            version = version.trimmed();
        }

        QStringList contents = {
            DeviceInfoParserInstance.qureyData("lshw", disk, "product"),
            size,
            DeviceInfoParserInstance.qureyData("lshw", disk, "vendor"),
            DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "Rotation Rate"),
            DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "Form Factor"),
            DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "Power_On_Hours") + " Hours, "
            + DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "Power_Cycle_Count") + " Power Cycle Count",
            DeviceInfoParserInstance.qureyData("lshw", disk, "serial"),
            version,
            speed
        };

        addSubInfo( "", mechanicalHardDiskNames, contents);
    }
}

