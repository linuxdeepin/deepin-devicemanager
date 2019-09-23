#include "monitorwidget.h"
#include "deviceinfoparser.h"
#include "math.h"
#include <QDate>

int gcd(int a,int b){
    if(a<b)
        std::swap(a,b);
    if(a%b==0)
        return b;
    else
        return gcd(b,a%b);
}

MonitorWidget::MonitorWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeMonitor)
{
    setTitle(DeviceAttributeMonitor + " " + DeviceAttributeInfo);

    QStringList monitorNames = {
                            DeviceAttributeName,
                            DeviceAttributeVendor,
                            DeviceAttributeManufactureDate,
                            DeviceAttributeSize,
                            DeviceAttributeResolution,
                            DeviceAttributeDisplayRate,
                            DeviceAttributeConnectType
                            };

    QStringList monitorList = DeviceInfoParserInstance.getMonitorList();
    QStringList connectedMonitorList = DeviceInfoParserInstance.getConnectedMonitorList();

    for(int i = 0; i < monitorList.size(); ++i)
    {
        const QString& monitor = monitorList.at(i);
        QString manufactureDate = DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Year of Manufacture");
        manufactureDate += DeviceAttributeYear;
        manufactureDate += " ";
        manufactureDate += DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Week of Manufacture");
        manufactureDate += DeviceAttributeWeek;

        QString monitorSize;

        QString size = DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Size");
        QRegExp re("^([\\d]*)x([\\d]*) mm$");
        if( re.exactMatch(size) )
        {
            double width = re.cap(1).toFloat()/2.54;
            double height = re.cap(2).toFloat()/2.54;
            double inch = std::sqrt(width*width + height*height)/10.0;
            monitorSize = QString::number(inch,10, 1) + " " + DeviceAttributeInch + " (";
            monitorSize += size;
            monitorSize += ")";
        }
        else
        {
            monitorSize += size;
        }

        QString currentResolution = DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Current Resolution");
        QString displayRete;
        re.setPattern("^([\\d]*)x([\\d]*)$");
        if( re.exactMatch(currentResolution) )
        {
            int width = re.cap(1).toInt();
            int height = re.cap(2).toInt();
            int gys = gcd(width, height);
            displayRete = QString::number(width/gys) + " : " + QString::number(height/gys);
        }

        QString connectType = DeviceAttributeUnknown;
        if( i < connectedMonitorList.size())
        {
            connectType = connectedMonitorList.at(i);
            int index = connectType.indexOf('-');
            if( index > 0 )
            {
                connectType = connectType.mid(0, index);
            }
        }

        QStringList contents = {
            DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Model"),
            DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Vendor"),
            manufactureDate,
            monitorSize,
            currentResolution,
            displayRete,
            connectType
        };

        addSubInfo( "", monitorNames, contents);
    }
}

