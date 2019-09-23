#include "cpuwidget.h"
#include "deviceinfoparser.h"

CpuWidget::CpuWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeCPU)
{
    setTitle(DeviceAttributeCPU + " " + DeviceAttributeInfo);

    QStringList names = {   DeviceAttributeModel,
                            DeviceAttributeArchitecture,
                            DeviceAttributeVendor,
                            DeviceAttributeSpeed,
                            DeviceAttributeCores + " " + DeviceAttributeNumber,
                            DeviceAttributeThreads + " " + DeviceAttributeNumber,
                            DeviceAttributeL1d,
                            DeviceAttributeL1i,
                            DeviceAttributeL2,
                            DeviceAttributeL3
                        };

    QString modelName = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Model name");

    float maxMHz = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "CPU max MHz").toFloat()/1000.0;
    float minMHz = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "CPU min MHz").toFloat()/1000.0;

    QString speed;
    int index = modelName.indexOf('@');
    if(index > 0)
    {
        speed = modelName.mid(index+1).trimmed();
    }
    else
    {
        speed = QString::number(maxMHz) + "GHz";
    }

    if(minMHz != maxMHz)
    {
        speed += " (";
        speed += QString::number(minMHz);
        speed += "GHz - ";
        speed += QString::number(maxMHz);
        speed += "GHz)";
    }

    int cpus = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "CPU(s)").toInt();
    int threadsPerCore = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Thread(s) per core").toInt();
    QString corePlus = QString::number(cpus) + "x ";

    QStringList contents = {
        modelName,
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

    addInfo( names, contents);
}
