/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
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

#pragma once

#include <QString>
#include <QSet>

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

    void queryData(const QString& toolname, const QString& firstKey, const QString& secondKey );
    void queryData(const QString& toolname, const QString& firstKey, const QString& secondKey, QSet<QString>& existSet);
    void queryData(const QString& toolname, const QString& firstKey, const QString& secondKey, QSet<QString>& existSet, QList<ArticleStruct>& articles);
    bool isValid();

    bool operator==(const ArticleStruct& other);
};

bool operator==(const ArticleStruct& a1, const ArticleStruct& a2);
