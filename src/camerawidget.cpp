#include "camerawidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

CameraWidget::CameraWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Camera"))
{
    initWidget();
}

void CameraWidget::initWidget()
{
    setTitle(DApplication::translate("Main", "Camera")  + DApplication::translate("Main", " Info"));

    QStringList cameraNames = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Capabilities")
                        };

    QStringList cameraList = DeviceInfoParserInstance.getCameraList();

    if(cameraList.size() < 1)
    {
        setTitle("No " + DApplication::translate("Main", "Camera") + " found!");
        addStrecch();
        return;
    }

    foreach(const QString& device, cameraList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.queryData("lshw", device, "product"),
            DeviceInfoParserInstance.queryData("lshw", device, "vendor"),
            DeviceInfoParserInstance.queryData("lshw", device, "capabilities")
        };

        addSubInfo( "", cameraNames, contents);
    }
}

