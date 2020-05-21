/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "deviceattributedefine.h"
#include "deviceinfoparser.h"
#include "DApplication"

DWIDGET_USE_NAMESPACE

//void ArticleStruct::queryData(const QString& toolname, const QString& firstKey, const QString& secondKey)
//{
//    if( isValid() )
//    {
//        return;
//    }

//    value = DeviceInfoParser::Instance().queryData( toolname, firstKey, secondKey);
//}

//void ArticleStruct::queryData(const QString& toolname, const QString& firstKey, const QString& secondKey, QSet<QString>& existSet)
//{
//    existSet.insert(secondKey);

//    if( isValid() )
//    {
//        return;
//    }

//    value = DeviceInfoParser::Instance().queryData( toolname, firstKey, secondKey);
//}

//void ArticleStruct::queryData(const QString& toolname, const QString& firstKey, const QString& secondKey, QSet<QString>& existSet, QList<ArticleStruct>& articles)
//{
//    existSet.insert(secondKey);

//    if( isValid() )
//    {
//        return;
//    }

//    value = DeviceInfoParser::Instance().queryData( toolname, firstKey, secondKey);

//    if( isValid() )
//    {
//        articles.push_back(*this);
//    }
//}

bool ArticleStruct::isValid()
{
    if (value.isEmpty()) {
        return false;
    }

    if ( value == QObject::tr("Unknown")) {
        return false;
    }

    if ( value == QString("Unknown")) {
        return false;
    }

    if ( value == QString("Unknown")) {
        return false;
    }

    if ( value.compare(QString("no"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if ( value.compare(QString("N/A"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if ( value == QString("")) {
        return false;
    }

    if ( value.compare(QString("Null"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if ( value.compare(QString("none"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if ( value.compare(QString("Not Provided"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if ( value.compare(QString("Not Specified"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if ( value.compare(QString("Default string"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if ( value.compare(QString("Unspecified"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if ( value.compare(QString("Not Present"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    return true;
}

bool ArticleStruct::operator==(const ArticleStruct &other)
{
    return this->name == other.name && this->value == other.value;
}

bool operator==(const ArticleStruct &a1, const ArticleStruct &a2)
{
    return a1.name == a2.name && a1.value == a2.value;
}
