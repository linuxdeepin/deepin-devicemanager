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

    ArticleStruct vendor;
    vendor.queryData("dmidecode", "System Information", "Manufacturer");
    if(vendor.isValid() == false)
    {
        vendor.queryData("catbaseboard", "Base Board Information", "Manufacturer");
    }
    if(vendor.value.contains("System manufacturer"))
    {
        vendor.queryData("dmidecode", "Base Board Information", "Manufacturer");
    }

    ArticleStruct pName;
    pName.queryData("dmidecode", "System Information", "Product Name");
    if(pName.isValid() == false)
    {
        pName.queryData("catbaseboard", "Base Board Information", "Board name");
    }
    if( pName.value.contains("System Product Name"))
    {
        pName.queryData("dmidecode", "Base Board Information", "Board name");
    }

    ArticleStruct ver;
    ver.queryData("dmidecode", "System Information", "Version");
    if(ver.isValid() == false || ver.value.contains("System Version"))
    {
        ver.value = "";
    }

    model.value = vendor.value + " " + pName.value + " " + ver.value;

    if(ver.value.contains("Not Specified", Qt::CaseInsensitive) || ver.value.contains("x.x", Qt::CaseInsensitive) || ver.value.contains("Not Applicable", Qt::CaseInsensitive))
    {
        model.value = vendor.value + " " + pName.value;
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
