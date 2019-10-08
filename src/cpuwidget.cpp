#include "cpuwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

CpuWidget::CpuWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "CPU"))
{
    initWidget();
}

void CpuWidget::initWidget()
{
    setTitle(DApplication::translate("Main", "CPU")  + DApplication::translate("Main", " Info"));

    QStringList names = {   DApplication::translate("Main", "Model"),
                            DApplication::translate("Main", "Architecture"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Speed"),
                            DApplication::translate("Main", "Cores")  + DApplication::translate("Main", "Number"),
                            DApplication::translate("Main", "Threads")  + DApplication::translate("Main", "Number"),
                            DApplication::translate("Main", "L1d") + DApplication::translate("Main", "Cache"),
                            DApplication::translate("Main", "L1i")+ DApplication::translate("Main", "Cache"),
                            DApplication::translate("Main", "L2")+ DApplication::translate("Main", "Cache"),
                            DApplication::translate("Main", "L3")+ DApplication::translate("Main", "Cache")
                        };

    QString modelName = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Model name");

    double maxMHz = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "CPU max MHz").toDouble()/1000.0;
    double minMHz = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "CPU min MHz").toDouble()/1000.0;

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
    QString corePlus = " x " + QString::number(cpus);

    QStringList contents = {
        modelName,
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Architecture"),
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Vendor ID"),
        speed,
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "CPU(s)"),
        QString::number(cpus*threadsPerCore),
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "L1d cache") + corePlus,
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "L1i cache") + corePlus,
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "L2 cache") + corePlus,
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "L3 cache"),
    };

    addInfo( names, contents);
}
