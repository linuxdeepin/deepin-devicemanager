/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "src/DeviceManager/DeviceInfo.h"
#include "src/DeviceManager/DeviceBios.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class DeviceInfo_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
        delete bios;
    }
    DeviceBaseInfo *m_deviceBaseInfo;
    DeviceBios *bios = new DeviceBios;
};

TEST_F(DeviceInfo_UT, DeviceInfo_UT_getOtherAttribs)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    m_deviceBaseInfo->getBaseAttribs();
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_isValueValid)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QString value = "";
    m_deviceBaseInfo->isValueValid(value);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_toHtmlString)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QDomDocument doc;
    m_deviceBaseInfo->toHtmlString(doc);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_baseInfoToHTML)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QDomDocument doc;
    QList<QPair<QString, QString>> infoLst;
    QPair<QString, QString> pair = QPair<QString, QString>("/", "/");
    infoLst.append(pair);
    m_deviceBaseInfo->baseInfoToHTML(doc, infoLst);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_subTitleToHTML)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QDomDocument doc;
    m_deviceBaseInfo->subTitleToHTML(doc);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_toDocString)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    Docx::Document doc;
    m_deviceBaseInfo->toDocString(doc);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_baseInfoToDoc)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    Docx::Document doc;
    QList<QPair<QString, QString>> infoLst;
    QPair<QString, QString> pair = QPair<QString, QString>("/", "/");
    infoLst.append(pair);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_toXlsxString)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QXlsx::Document xlsx;
    QXlsx::Format boldFont;
    m_deviceBaseInfo->toXlsxString(xlsx, boldFont);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_baseInfoToXlsx)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QXlsx::Document xlsx;
    QXlsx::Format boldFont;
    QList<QPair<QString, QString>> infoLst;
    QPair<QString, QString> pair = QPair<QString, QString>("/", "/");
    infoLst.append(pair);
    m_deviceBaseInfo->baseInfoToXlsx(xlsx, boldFont, infoLst);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_toTxtString)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QTextStream out;
    m_deviceBaseInfo->toTxtString(out);
    m_deviceBaseInfo->tableInfoToTxt(out);
    m_deviceBaseInfo->tableHeaderToTxt(out);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_baseInfoToTxt)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QTextStream out;
    QList<QPair<QString, QString>> infoLst;
    QPair<QString, QString> pair = QPair<QString, QString>("/", "/");
    infoLst.append(pair);
    m_deviceBaseInfo->baseInfoToTxt(out, infoLst);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_tableHeaderToHtml)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QFile html;
    m_deviceBaseInfo->tableHeaderToHtml(html);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_setEnable)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    //    m_deviceBaseInfo->setEnable(true);
    //    m_deviceBaseInfo->enable();
    m_deviceBaseInfo->setCanEnale(true);
    ASSERT_TRUE(m_deviceBaseInfo->canEnable());
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_getOverviewInfo)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    m_deviceBaseInfo->getOverviewInfo();
    m_deviceBaseInfo->addFilterKey("");
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_getOtherMapInfo)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "1@n");
    m_deviceBaseInfo->getOtherMapInfo(mapinfo);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_addBaseDeviceInfo)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    m_deviceBaseInfo->addBaseDeviceInfo("name", "abc@123");
    m_deviceBaseInfo->addOtherDeviceInfo("name", "abc@123");
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_setAttribute)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "1@n");
    QString value = "abc@123";
    m_deviceBaseInfo->setAttribute(mapinfo, "name", value, true);
}

TEST_F(DeviceInfo_UT, DeviceInfo_UT_mapInfoToList)
{
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(bios);
    m_deviceBaseInfo->mapInfoToList();
}
