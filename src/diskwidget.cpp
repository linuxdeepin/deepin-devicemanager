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
                            DApplication::translate("Main", "Power On Times"),
                            DApplication::translate("Main", "Serial Number"),
                            DApplication::translate("Main", "SATA Version"),
                            DApplication::translate("Main", "Speed")
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
            DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "Power_On_Hours") + DApplication::translate("Main", " Hours") + ", "
            + DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "Power_Cycle_Count") + DApplication::translate("Main", " Power Cycle Count"),
            DeviceInfoParserInstance.qureyData("lshw", disk, "serial"),
            version,
            speed
        };

        addSubInfo( "", mechanicalHardDiskNames, contents);
    }
}

