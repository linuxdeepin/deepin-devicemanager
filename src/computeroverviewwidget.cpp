#include "computeroverviewwidget.h"
#include "deviceinfoparser.h"

ComputerOverviewWidget::ComputerOverviewWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeComputerOverview)
{
    setTitle(DeviceAttributeComputerOverview);

    QStringList names = {   DeviceAttributeComputerModel,
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

    QStringList contents = {
        DeviceInfoParserInstance.qureyData("dmidecode", "System Information", "Version"),
        os,
        DeviceInfoParserInstance.qureyData("lscpu", "lscpu", "Model name"),
        DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Manufacturer") \
        + DeviceInfoParserInstance.qureyData("dmidecode", "Base Board Information", "Product Name"),
        DeviceInfoParserInstance.qureyData("lshw", "Computer_core_memory", "size"),
        DeviceInfoParserInstance.qureyData("lshw", "Computer_core_pci_sata_disk", "size"),
        DeviceInfoParserInstance.fuzzyQueryData("lspci", "VGA compatible controller", "Subsystem"),
        DeviceInfoParserInstance.fuzzyQueryData("dmidecode", "System Information", "Version"),
        DeviceInfoParserInstance.fuzzyQueryData("lspci", "Audio device", "Subsystem"),
        DeviceInfoParserInstance.fuzzyQueryData("lspci", "Ethernet controller", "Subsystem"),
    };

    addSubInfo("", names, contents);
}
