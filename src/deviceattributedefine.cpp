#include "deviceattributedefine.h"
#include "deviceinfoparser.h"
#include "DApplication"

DWIDGET_USE_NAMESPACE

void ArticleStruct::queryData(const QString& toolname, const QString& firstKey, const QString& secondKey)
{
    value = DeviceInfoParserInstance.queryData( toolname, firstKey, secondKey );
}

bool ArticleStruct::isValid()
{
    if(value.isEmpty())
    {
        return false;
    }

    if( value == DApplication::translate("Main", "Unknown"))
    {
        return false;
    }

    return true;
}
