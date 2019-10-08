#include "powerwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

PowerWidget::PowerWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Power"))
{
    initWidget();
}

void PowerWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Power")  + DApplication::translate("Main", " Info"));

    QStringList switchingpowerNames = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Screen Suspend Delay"),
                            DApplication::translate("Main", "Computer Suspend Delay"),
                            DApplication::translate("Main", "AutoLock Screen Delay"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Max Power Capacity"),
                            DApplication::translate("Main", "Statu"),
                            DApplication::translate("Main", "Type")
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
            addSubInfo(DApplication::translate("Main", "Switching Power"), switchingpowerNames, contents);
            bNeedSetTitle = false;
        }
        else
        {
            addSubInfo("", switchingpowerNames, contents);
        }
    }

    QStringList batteryNames = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Screen Suspend Delay"),
                            DApplication::translate("Main", "Computer Suspend Delay"),
                            DApplication::translate("Main", "AutoLock Screen Delay"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Slot"),
                            DApplication::translate("Main", "Max Power Capacity"),
                            DApplication::translate("Main", "configuration")
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
            addSubInfo(DApplication::translate("Main", "Battery"), batteryNames, contents);
            bNeedSetTitle = false;
        }
        else
        {
            addSubInfo("", switchingpowerNames, contents);
        }

    }
}
