#include "monitorwidget.h"
#include "deviceinfoparser.h"
#include "math.h"
#include <QDate>
#include <DApplication>

DWIDGET_USE_NAMESPACE

int gcd(int a,int b){
    if(a<b)
        std::swap(a,b);
    if(a%b==0)
        return b;
    else
        return gcd(b,a%b);
}

MonitorWidget::MonitorWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Monitor"))
{
    initWidget();
}

void MonitorWidget::initWidget()
{
    setTitle(DApplication::translate("Main", "Monitor")  + DApplication::translate("Main", " Info"));

    QStringList monitorNames = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Manufacture Date"),
                             DApplication::translate("Main", "Size"),
                            DApplication::translate("Main", "Resolution"),
                            DApplication::translate("Main", "Display Rate"),
                            DApplication::translate("Main", "Connect Type")
                            };

    QStringList monitorList = DeviceInfoParserInstance.getMonitorList();
    QStringList connectedMonitorList = DeviceInfoParserInstance.getConnectedMonitorList();

    for(int i = 0; i < monitorList.size(); ++i)
    {
        const QString& monitor = monitorList.at(i);
        QString manufactureDate = DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Year of Manufacture");
        manufactureDate += DApplication::translate("Main", "Year");
        manufactureDate += " ";
        manufactureDate += DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Week of Manufacture");
        manufactureDate += DApplication::translate("Main", "Week");

        QString monitorSize;

        QString size = DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Size");
        QRegExp re("^([\\d]*)x([\\d]*) mm$");
        if( re.exactMatch(size) )
        {
            double width = re.cap(1).toFloat()/2.54;
            double height = re.cap(2).toFloat()/2.54;
            double inch = std::sqrt(width*width + height*height)/10.0;
            monitorSize = QString::number(inch,10, 1) + " " + DApplication::translate("Main", "inch") + " (";
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

        QString connectType = DApplication::translate("Main", "Unknown");
        if( i < connectedMonitorList.size())
        {
            connectType = connectedMonitorList.at(i);
            int index = connectType.indexOf('-');
            if( index > 0 )
            {
                connectType = connectType.mid(0, index);
            }
        }

        QString vendor = DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Vendor");
        QString abb;
        QRegExp rx("(^[\\s\\S]*)\"([\\s\\S]+)\"$");
        if( rx.exactMatch(vendor) )
        {
            abb = rx.cap(1).trimmed();
            vendor = rx.cap(2).trimmed();
        }

        QString model = DeviceInfoParserInstance.qureyData("hwinfo", monitor, "Model");
        model = model.remove("\"");
        model.remove(abb);

        QStringList contents = {
            model,
            vendor,
            manufactureDate,
            monitorSize,
            currentResolution,
            displayRete,
            connectType
        };

        addSubInfo( "", monitorNames, contents);
    }
}
