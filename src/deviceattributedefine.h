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

#pragma once

#include <QString>
#include <QSet>

struct ArticleStruct
{
    QString name;
    QString value;
    QString transContextName;


    bool autoHide = true;
    bool externalLinks = false;
    bool valueTranslate = false;

    QString lastKey;

    explicit ArticleStruct(const QString n = "",bool hide = false): name(n), autoHide(hide),lastKey("")
    {

    }
    ArticleStruct(const QString& n ,QString lsKey): name(n), autoHide(false),lastKey(lsKey)
    {

    }
    ArticleStruct(const ArticleStruct& other)
    {
        name = other.name;
        value = other.value;
        autoHide = other.autoHide;
        externalLinks = other.externalLinks;
        valueTranslate = other.valueTranslate;
        transContextName = other.transContextName;
        lastKey = other.lastKey;
    }
    bool isValid();

    bool operator==(const ArticleStruct& other);
};

bool operator==(const ArticleStruct& a1, const ArticleStruct& a2);
