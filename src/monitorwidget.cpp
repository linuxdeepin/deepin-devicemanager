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
    //setTitle(DApplication::translate("Main", "Monitor")  + DApplication::translate("Main", " Info"));

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    QStringList monitorNames = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Manufacture Date"),
                            DApplication::translate("Main", "Size"),
                            DApplication::translate("Main", "Resolution"),
                            DApplication::translate("Main", "Display Rate"),
                            DApplication::translate("Main", "Connect Type")
                            };

    QStringList hwinfMonitorList = DeviceInfoParserInstance.getHwinfoMonitorList();
    QStringList xrandrMonitorList = DeviceInfoParserInstance.getXrandrMonitorList();

    for(int i = 0; i < hwinfMonitorList.size(); ++i)
    {
        const QString& monitor = hwinfMonitorList.at(i);

        ArticleStruct name("Name");
        name.queryData("hwinfo", monitor, "Model");
        name.value = name.value.remove("\"");

        existArticles.insert("Model");

        ArticleStruct vendor("Vendor");
        vendor.queryData("hwinfo", monitor, "Vendor");
        QString abb;
        QRegExp rx("(^[\\s\\S]*)\"([\\s\\S]+)\"$");
        if( rx.exactMatch(vendor.value) )
        {
            abb = rx.cap(1).trimmed();
            vendor.value = rx.cap(2).trimmed();
        }
        existArticles.insert("Vendor");

        name.value.remove(abb);
        articles.push_back(name);
        articles.push_back(vendor);

        ArticleStruct serial("Serial Number");
        serial.queryData("hwinfo", monitor, "Serial ID");
        serial.value.remove("\"");
        articles.push_back(serial);
        existArticles.insert("Serial ID");

        ArticleStruct mDate("Manufacture Date");
        mDate.value = DeviceInfoParserInstance.queryData("hwinfo", monitor, "Year of Manufacture");
        mDate.value += DApplication::translate("Main", "Year");
        mDate.value += " ";
        mDate.value += DeviceInfoParserInstance.queryData("hwinfo", monitor, "Week of Manufacture");
        mDate.value += DApplication::translate("Main", "Week");
        articles.push_back(mDate);
        existArticles.insert("Year of Manufacture");
        existArticles.insert("Week of Manufacture");

        ArticleStruct monitorSize("Size");
        QString size = DeviceInfoParserInstance.queryData("hwinfo", monitor, "Size");
        double inch = 0.0;
        QRegExp re("^([\\d]*)x([\\d]*) mm$");
        if( re.exactMatch(size) )
        {
            double width = re.cap(1).toDouble()/2.54;
            double height = re.cap(2).toDouble()/2.54;
            inch = std::sqrt(width*width + height*height)/10.0;
            monitorSize.value = QString::number(inch,10, 1) + " " + DApplication::translate("Main", "inch") + " (";
            monitorSize.value += size;
            monitorSize.value += ")";
        }
        else
        {
            monitorSize.value += size;
        }
        articles.push_back(monitorSize);
        existArticles.insert("Size");

        ArticleStruct currentResolution("Resolution");

        ArticleStruct displayRete("Display Rate");

        currentResolution.queryData("hwinfo", monitor, "Current Resolution");

        re.setPattern("^([\\d]*)x([\\d]*)$");
        if( re.exactMatch(currentResolution.value) )
        {
            int width = re.cap(1).toInt();
            int height = re.cap(2).toInt();
            int gys = gcd(width, height);
            displayRete.value = QString::number(width/gys) + " : " + QString::number(height/gys);
        }

        articles.push_back(currentResolution);
        existArticles.insert("Current Resolution");

        articles.push_back(displayRete);

        ArticleStruct primaryMonitor("Primary Monitor");
        primaryMonitor.value = "No";
        if( i < xrandrMonitorList.size())
        {
            if( true == xrandrMonitorList.at(i).contains("primary", Qt::CaseInsensitive) )
            {
                primaryMonitor.value = "Yes";
            }
        }
        articles.push_back(primaryMonitor);

        ArticleStruct connectType("Connect Type");
        connectType.value = DApplication::translate("Main", "Unknown");
        if( i < xrandrMonitorList.size())
        {
            connectType.value = xrandrMonitorList.at(i);
            int index = connectType.value.indexOf(' ');
            if( index > 0 )
            {
                connectType.value = connectType.value.mid(0, index);
            }
            index = connectType.value.indexOf('-');
            if( index > 0 )
            {
                connectType.value = connectType.value.mid(0, index);
            }
        }
        articles.push_back(connectType);

        addSubInfo( name.value, articles );

        if( hwinfMonitorList.size() > 1 )
        {
            QStringList tab =
            {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }

        if(i == 0)
        {
            overviewInfo_.value = name.value;
            if(inch != 0.0)
            {
                overviewInfo_.value += "(";
                overviewInfo_.value += QString::number(inch,10, 1) + " " + DApplication::translate("Main", "inch");
                overviewInfo_.value += ")";
            }
        }
    }

    if( hwinfMonitorList.size() > 1 )
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
}
