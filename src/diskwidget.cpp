#include "diskwidget.h"
#include "deviceinfoparser.h"

DiskWidget::DiskWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeDisk)
{
    //setTitle(DeviceAttributeDisk + " " + DeviceAttributeInfo);

    QStringList mechanicalHardDiskNames = {
                            DeviceAttributeModel,
                            DeviceAttributeSize,
                            DeviceAttributeManufacturer,
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
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("lshw", disk, "product"),
            DeviceInfoParserInstance.qureyData("lshw", disk, "size"),
            DeviceInfoParserInstance.qureyData("lshw", disk, "vendor"),
            DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "Rotation Rate"),
            DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "Form Factor"),
            DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "Power_On_Hours") + " Hours, "
            + DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "Power_Cycle_Count") + " Power Cycle Count",
            DeviceInfoParserInstance.qureyData("lshw", disk, "serial"),
            DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "SATA Version is"),
            DeviceInfoParserInstance.qureyData("smartctl", "smartctl", "SATA Version is"),
        };

        addSubInfo( disk, mechanicalHardDiskNames, contents);
    }

    float maxMHz = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "CPU max MHz").toFloat()/1000.0;
    float minMHz = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "CPU min MHz").toFloat()/1000.0;
    QString speed = QString::number(maxMHz) + "GHz";
    if(minMHz != maxMHz)
    {
        speed += "(";
        speed += QString::number(minMHz);
        speed += "GHz -";
        speed += QString::number(maxMHz);
        speed += "GHz)";
    }

    int cpus = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "CPU(s)").toInt();
    int threadsPerCore = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Thread(s) per core").toInt();
    QString corePlus = QString::number(cpus) + "x ";

    QStringList contents = {
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Model name"),
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Architecture"),
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Vendor ID"),
        speed,
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "CPU(s)"),
        QString::number(cpus*threadsPerCore),
        corePlus+DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "L1d cache"),
        corePlus+DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "L1i cache"),
        corePlus+DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "L2 cache"),
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "L3 cache"),
    };

    //addSubInfo( "", names, contents);
}

