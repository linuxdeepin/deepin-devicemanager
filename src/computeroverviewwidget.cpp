#include "computeroverviewwidget.h"
#include "deviceinfoparser.h"
#include "QRegExp"

ComputerOverviewWidget::ComputerOverviewWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeComputerOverview)
{
    setTitle(DeviceAttributeComputerOverview);

    QStringList names = {   DeviceAttributeModel,
                            DeviceAttributeOperatingSystem,
                            DeviceAttributeCPU,
                            DeviceAttributeMotherboard,
                            DeviceAttributeMemory,
                            DeviceAttributeDisk,
                            DeviceAttributeDisplayAdapter,
                            DeviceAttributeMonitor,
                            DeviceAttributeAudioAdapter,
                            DeviceAttributeNetworkAdapter
                        };

    QString os;
    DeviceInfoParserInstance.getOSInfo(os);
    QString chipsetFamily = DeviceInfoParserInstance.fuzzyQueryData("lspci", "ISA bridge", "Subsystem");
    QRegExp rx("^[\\s\\S]*\\(([\\S]*)\\)$");
    if( rx.exactMatch(chipsetFamily) )
    {
        chipsetFamily = "(" + rx.cap(1)+" "+DeviceAttributeChipsetFamily + ")";
    }

    QString motherboard =  DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Manufacturer") \
            + DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Product Name") + chipsetFamily;

    QString monitor = DeviceInfoParserInstance.fuzzyQueryData("hwinfo", "Monitor", "Model");
    monitor.remove("\"");

    QString cpu = DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Model name");
    cpu.remove(" CPU", Qt::CaseInsensitive);

    QStringList contents = {
        DeviceInfoParserInstance.qureyData("dmidecode", "System Information", "Version"),
        os,
        cpu,
        motherboard,
        DeviceInfoParserInstance.qureyData("lshw", "Computer_core_memory", "size"),
        DeviceInfoParserInstance.qureyData("lshw", "Computer_core_pci_sata_disk", "size"),
        DeviceInfoParserInstance.fuzzyQueryData("lspci", "VGA compatible controller", "Subsystem"),
        monitor,
        DeviceInfoParserInstance.fuzzyQueryData("lspci", "Audio device", "Subsystem"),
        DeviceInfoParserInstance.fuzzyQueryData("lspci", "Ethernet controller", "Subsystem"),
    };

    addInfo( names, contents);
}
