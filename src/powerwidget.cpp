#include "powerwidget.h"
#include "deviceinfoparser.h"

PowerWidget::PowerWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributePower)
{
    initWidget();
}

void PowerWidget::initWidget()
{
    //setTitle(DeviceAttributePower + " " + DeviceAttributeInfo);

    QStringList switchingpowerNames = {
                            DeviceAttributeName,
                            DeviceAttributeType,
                            DeviceAttributeVendor,
                            DeviceAttributeMaxPowerCapacity,
                            DeviceAttributeStatu,
                            DeviceAttributeType,
                            DeviceAttributeScreenSuspendDelay,
                            DeviceAttributeComputerSuspendDelay,
                            DeviceAttributeAutoLockScreenDelay
                        };

    QStringList switchingpowerList = DeviceInfoParserInstance.getSwitchingpowerList();
    bool bNeedSetTitle = true;
    foreach(const QString& device, switchingpowerList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("lshw", device, "product"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Manufacturer"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Manufacturer"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Max Power Capacity"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Status"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Type"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Manufacturer"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "BD Address"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Link policy")
        };
        if(bNeedSetTitle)
        {
            addSubInfo(DeviceAttributeSwitchingPower, switchingpowerNames, contents);
            bNeedSetTitle = false;
        }
        else
        {
            addSubInfo("", switchingpowerNames, contents);
        }
    }

    QStringList batteryNames = {
                            DeviceAttributeName,
                            DeviceAttributeVendor,
                            DeviceAttributeSlot,
                            DeviceAttributeMaxPowerCapacity,
                            DeviceAttributeConfiguration
                        };

    QStringList batteryList = DeviceInfoParserInstance.getBatteryList();
    bNeedSetTitle = true;
    foreach(const QString& device, batteryList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("lshw", device, "product"),
            DeviceInfoParserInstance.qureyData("lshw", device, "vendor"),
            DeviceInfoParserInstance.qureyData("lshw", device, "slot"),
            DeviceInfoParserInstance.qureyData("lshw", device, "capacity"),
            DeviceInfoParserInstance.qureyData("lshw", device, "configuration")
        };

        if(bNeedSetTitle)
        {
            addSubInfo(DeviceAttributeBattery, batteryNames, contents);
            bNeedSetTitle = false;
        }
        else
        {
            addSubInfo("", switchingpowerNames, contents);
        }

    }
}
