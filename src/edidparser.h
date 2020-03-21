/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     yaobin <yaobin@uniontech.com>
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

#ifndef EDIDPARSER_H
#define EDIDPARSER_H

#include <QString>
#include "monitorwidget.h"
class EDIDParser
{
public:
    EDIDParser();
    void setSource(QString source);

    bool isVaild();
    QString getReadableHexString();
    bool getDate(int &week, int &year);
    QString getManufatureName();
    QString getEDIDVersion();
    QString getMaxImageSize();
    QString getSignalType();
    QStringList getSupportResolutionList();
private:

    bool m_sourceValid = false;
    QByteArray m_edidBin;
};

#endif // EDIDPARSER_H
