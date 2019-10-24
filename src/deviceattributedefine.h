#pragma once

#include <QString>

struct ArticleStruct
{
    QString name;
    QString value;

    bool autoHide = true;
    bool externalLinks = false;
    bool valueTranslate = false;

    ArticleStruct(const QString& n = "", bool b = true): name(n), autoHide(b)
    {

    }

    void queryData(const QString& toolname, const QString& firstKey, const QString& secondKey);
};

