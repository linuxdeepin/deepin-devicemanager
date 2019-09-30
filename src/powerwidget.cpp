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
                            DeviceAttributeScreenSuspendDelay,
                            DeviceAttributeComputerSuspendDelay,
                            DeviceAttributeAutoLockScreenDelay,
                            DeviceAttributeVendor,
                            DeviceAttributeMaxPowerCapacity,
                            DeviceAttributeStatu,
                            DeviceAttributeType
                        };

    QStringList switchingpowerList = DeviceInfoParserInstance.getSwitchingpowerList();
    bool bNeedSetTitle = true;
    foreach(const QString& device, switchingpowerList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("lshw", device, "product"),
            DeviceInfoParserInstance.switchingpowerScreenSuspendDelay_==0? "Never" \
                : QString::number(DeviceInfoParserInstance.switchingpowerScreenSuspendDelay_) + " Secs",
            DeviceInfoParserInstance.switchingpowerComputerSuspendDelay_==0? "Never" \
                : QString::number(DeviceInfoParserInstance.switchingpowerComputerSuspendDelay_) + " Secs",
            DeviceInfoParserInstance.switchingpowerAutoLockScreenDelay_==0? "Never" \
                : QString::number(DeviceInfoParserInstance.switchingpowerAutoLockScreenDelay_) + " Secs",
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Manufacturer"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Max Power Capacity"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Status"),
            DeviceInfoParserInstance.qureyData("dmidecode", "System Power Supply", "Type")
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
                            DeviceAttributeScreenSuspendDelay,
                            DeviceAttributeComputerSuspendDelay,
                            DeviceAttributeAutoLockScreenDelay,
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
            DeviceInfoParserInstance.batteryScreenSuspendDelay_==0? "Never" \
                : QString::number(DeviceInfoParserInstance.batteryScreenSuspendDelay_) + " Secs",
            DeviceInfoParserInstance.batteryComputerSuspendDelay_==0? "Never" \
                : QString::number(DeviceInfoParserInstance.batteryComputerSuspendDelay_) + " Secs",
            DeviceInfoParserInstance.batteryAutoLockScreenDelay_==0? "Never" \
                : QString::number(DeviceInfoParserInstance.batteryAutoLockScreenDelay_) + " Secs",
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
