#include "audiodevicewidget.h"
#include "deviceinfoparser.h"
#include "math.h"
#include <QDate>

AudiodeviceWidget::AudiodeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeAudioDevice)
{
    setTitle(DeviceAttributeAudioDevice + " " + DeviceAttributeInfo);

    QStringList audioadapterNames = {
                            DeviceAttributeType,
                            DeviceAttributeName,
                            DeviceAttributeVendor,
                            DeviceAttributeStatu
                            };

    QStringList audioadapterContents = {
                            DeviceAttributeAudioAdapter,
                            DeviceInfoParserInstance.fuzzyQueryData("lspci", "Audio device", "Name"),
                            DeviceInfoParserInstance.fuzzyQueryData("lspci", "Audio device", "Subsystem"),
                            DeviceAttributeGood
    };

    addSubInfo("", audioadapterNames, audioadapterContents);

    QStringList names = {
                            DeviceAttributeType,
                            DeviceAttributeName,
                            DeviceAttributeVendor
                            };

    QStringList inputdeviceList = DeviceInfoParserInstance.getInputdeviceList();
    foreach(const QString& device, inputdeviceList)
    {
        QString name = DeviceInfoParserInstance.fuzzyQueryData("catinput", device, "Name");
        if(false == name.contains("Speaker", Qt::CaseInsensitive) && false == name.contains("Headphone", Qt::CaseInsensitive) )
        {
            continue;
        }

        QStringList contents = {
                                DeviceAttributeSpeaker,
                                name,
                                DeviceInfoParserInstance.fuzzyQueryData("catinput", device, "Vendor"),
                                };

        addSubInfo("", names, contents);
    }

    foreach(const QString& device, inputdeviceList)
    {
        QString name = DeviceInfoParserInstance.fuzzyQueryData("catinput", device, "Name");
        if(false == name.contains("Mic", Qt::CaseInsensitive))
        {
            continue;
        }

        QStringList contents = {
                                DeviceAttributeMicrophone,
                                name,
                                DeviceInfoParserInstance.fuzzyQueryData("catinput", device, "Vendor"),
                                };

        addSubInfo("", names, contents);
    }
}

