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
#include "edidparser.h"
#include <QFile>
EDIDParser::EDIDParser()
{
}

void EDIDParser::setSource(QString source)
{
    QFile file(source);
    if (file.open(QFile::ReadOnly)) {
        m_edidBin = file.readAll();
        file.close();
    }
}

bool EDIDParser::isVaild()
{
    if (m_sourceValid) {
        return m_sourceValid;
    }
    if (m_edidBin.size() == 256 || m_edidBin.size() == 128) {
        m_sourceValid = true;
    }
    return m_sourceValid;
}

QString EDIDParser::getReadableHexString()
{
    if (!isVaild()) {
        return "";
    }
    QString res("");
    for (int i = 0; i < m_edidBin.size(); i++) {
        QString first = QString::number(((0xf0 & m_edidBin[i])) >> 4, 16);
        QString second = QString::number((0x0f & m_edidBin[i]), 16);
        QString c = QString("%1%2").arg(first).arg(second);
        res.append(c);
    }
    return res;
}

bool EDIDParser::getDate(int &week, int &year)
{
    if (!isVaild()) {
        return false;
    }
    week = m_edidBin[0x10];
    year = m_edidBin[0x11];
    return true;
}

QString EDIDParser::getManufatureName()
{
    QString res = "";
    res.append(((m_edidBin[0x08] & char(0x7c)) >> 2 ) + 'A' - 1);
    res.append(((m_edidBin[0x08] & char(0x03)) << 3 )  + ((m_edidBin[0x09] & char(0xe0)) >> 5 ) + 'A' - 1);
    res.append(((m_edidBin[0x09] & char(0x1f)) >> 0 ) + 'A' - 1);
    return res;
}

QString EDIDParser::getEDIDVersion()
{
    if (!isVaild()) {
        return "";
    }
    return QString("%1.%2").arg(m_edidBin[0x12]).arg(m_edidBin[0x13]);
}

QString EDIDParser::getMaxImageSize()
{
    if (!isVaild()) {
        return "";
    }
    return QString("%1x%2 (cm)").arg(m_edidBin[0x15]).arg(m_edidBin[0x16]);
}

QString EDIDParser::getSignalType()
{
    if (!isVaild()) {
        return "";
    }
    if (m_edidBin[0x14] & char(0x80))
        return QString("digital");
    else
        return QString("analog");
}

QStringList EDIDParser::getSupportResolutionList()
{
    static const QStringList list = {
        //23h
        "720x400@70Hz",//item 0
        "720x400@88Hz",
        "640x480@60Hz",
        "640x480@67Hz",
        "640x480@72Hz",
        "640x480@75Hz",
        "800x600@56Hz",
        "800x600@60Hz",
        //24h
        "800x600@72Hz",
        "800x600@75Hz",
        "832x624@75Hz",
        "1024x768@87Hz",
        "1024x768@60Hz",
        "1024x768@70Hz",
        "1024x768@75Hz",
        "1280x1024@75Hz",
        //25h
        "1152x870@75Hz" //item 16

    };
    QStringList res;
    if (!isVaild()) {
        return res;
    }
    for ( int i = 0; i <= 7; i++) {
        if ((m_edidBin[0x23] & char(0x80 >> i))) {
            res.append(list.at(i));
        }
    }
    for ( int i = 0; i <= 7; i++) {
        if ((m_edidBin[0x24] & char(0x80 >> i))) {
            res.append(list.at(i + 8));
        }
    }
    if ((m_edidBin[0x25] & char(0x80 >> 0))) {
        res.append(list.at(16));
    }
    return res;
}

