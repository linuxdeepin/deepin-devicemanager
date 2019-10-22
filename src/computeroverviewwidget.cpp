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
    QString pName = DeviceInfoParserInstance.queryData("dmidecode", "System Information", "Product Name");
    QString ver = DeviceInfoParserInstance.queryData("dmidecode", "System Information", "Version");
    model.value = pName + " " + ver;
    if(ver.contains("Not Specified", Qt::CaseInsensitive) )
    {
        model.value = pName;
    }
    else if(false == pName.contains(" ") && ver.contains(" "))  //	Product Name: 10N9CTO1WW  Version: ThinkCentre M910t-N000
    {
        model.value = ver;
    }
    articles.push_back(model);

    ArticleStruct os("Operating System");
    DeviceInfoParserInstance.getOSInfo(os.value);
    articles.push_back(os);

//    ArticleStruct space(" ");
//    space.value = " ";
//    articles.push_back(space);

    articles.append(others);

    addInfo("", articles);
}
