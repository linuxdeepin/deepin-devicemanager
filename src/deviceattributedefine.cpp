#include "deviceattributedefine.h"
#include "deviceinfoparser.h"

void ArticleStruct::queryData(const QString& toolname, const QString& firstKey, const QString& secondKey)
{
    value = DeviceInfoParserInstance.queryData( toolname, firstKey, secondKey );
}
