#include "camerawidget.h"
#include "deviceinfoparser.h"

CameraWidget::CameraWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeCamera)
{
    setTitle(DeviceAttributeCamera + " " + DeviceAttributeInfo);

    QStringList cameraNames = {
                            DeviceAttributeName,
                            DeviceAttributeVendor,
                            DeviceAttributeCapabilities
                        };

    QStringList cameraList = DeviceInfoParserInstance.getCameraList();

    foreach(const QString& device, cameraList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("lshw", device, "product"),
            DeviceInfoParserInstance.qureyData("lshw", device, "vendor"),
            DeviceInfoParserInstance.qureyData("lshw", device, "capabilities")
        };

        addSubInfo( "", cameraNames, contents);
    }
}

