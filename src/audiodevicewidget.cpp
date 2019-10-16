#include "audiodevicewidget.h"
#include "deviceinfoparser.h"
#include "math.h"
#include <QDate>
#include <DApplication>

DWIDGET_USE_NAMESPACE

AudiodeviceWidget::AudiodeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Audio Device"))
{
    initWidget();
}

void AudiodeviceWidget::initWidget()
{
    setTitle(DApplication::translate("Main", "Audio Device")  + DApplication::translate("Main", " Info"));

    QStringList audioadapterNames = {
                            "Type",
                            "Name",
                            "Vendor",
                            "Statu"
                            };

    QStringList audioadapterContents = {
                            DApplication::translate("Main", "AudioAdapter"),
                            DeviceInfoParserInstance.fuzzyQueryData("lspci", "Audio device", "Name"),
                            DeviceInfoParserInstance.fuzzyQueryData("lspci", "Audio device", "Subsystem"),
                            DApplication::translate("Main", "Good")
                            };

    addSubInfo("", audioadapterNames, audioadapterContents);

    QStringList names = {
                            "Type",
                            "Name",
                            "Vendor"
                            };

    QStringList inputdeviceList = DeviceInfoParserInstance.getInputdeviceList();
    foreach(const QString& device, inputdeviceList)
    {
        QString name = DeviceInfoParserInstance.fuzzyQueryData("catinput", device, "Name");
        if(false == name.contains("Speaker", Qt::CaseInsensitive) && false == name.contains("Headphone", Qt::CaseInsensitive) )
        {
            continue;
        }

        name.remove("\"");
        QStringList contents = {
                                DApplication::translate("Main", "Speaker"),
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
        name.remove("\"");
        QStringList contents = {
                                DApplication::translate("Main", "Microphone"),
                                name,
                                DeviceInfoParserInstance.fuzzyQueryData("catinput", device, "Vendor"),
                                };

        addSubInfo("", names, contents);
    }
}
