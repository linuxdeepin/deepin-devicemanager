#include "computeroverviewwidget.h"
#include "deviceinfoparser.h"
#include "QRegExp"
#include "math.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

ComputerOverviewWidget::ComputerOverviewWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Overview"))
{
    initWidget();
}

void ComputerOverviewWidget::setOverviewInfos( const QList<ArticleStruct>& others )
{
    //setTitle( "Computer Overview" );

    QList<ArticleStruct> articles;

    ArticleStruct model("Model");
    QString vendor = DeviceInfoParserInstance.queryData("dmidecode", "System Information", "Manufacturer");
    if(vendor.contains("System manufacturer"))
    {
        vendor = DeviceInfoParserInstance.queryData("dmidecode", "Base Board Information", "Manufacturer");
    }
    QString pName = DeviceInfoParserInstance.queryData("dmidecode", "System Information", "Product Name");
    if( pName.contains("System Product Name"))
    {
        pName = DeviceInfoParserInstance.queryData("dmidecode", "Base Board Information", "Product Name");
    }

    QString ver = DeviceInfoParserInstance.queryData("dmidecode", "System Information", "Version");
    if(ver.contains("System Version"))
    {
        ver = "";
    }

    model.value = vendor + " " + pName + " " + ver;
    if(ver.contains("Not Specified", Qt::CaseInsensitive) || ver.contains("x.x", Qt::CaseInsensitive) || ver.contains("Not Applicable", Qt::CaseInsensitive))
    {
        model.value = vendor + " " + pName;
    }
//    else if(false == pName.contains(" ") && ver.contains(" "))  //	Product Name: 10N9CTO1WW  Version: ThinkCentre M910t-N000
//    {
//        model.value = vendor + " " + pName + " " + ver;
//    }

    articles.push_back(model);

    ArticleStruct os("Operating System");
    DeviceInfoParserInstance.getOSInfo(os.value);
    os.externalLinks = true;
    articles.push_back(os);

//    ArticleStruct space(" ");
//    space.value = " ";
//    articles.push_back(space);

    articles.append(others);

    addInfo("", articles);
}
