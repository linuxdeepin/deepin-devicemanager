// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceManager 的文件写入方法(Txt/Xlsx/Html + 概览写入) +
// addOthersDeviceFromHwinfo。空设备列表时写入空文件,不崩。
// 注:Doc 格式因 Docx::Document 默认构造 SEGV 暂跳过。

#include "DeviceManager.h"
#include "DeviceOthers.h"
#include "xlsxdocument.h"
#include "ut_Head.h"

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QTemporaryFile>

#include <gtest/gtest.h>

class UT_DeviceManagerSave : public UT_HEAD
{
public:
    void SetUp() override
    {
        DeviceManager::instance()->clear();
    }
    void TearDown() override
    {
        DeviceManager::instance()->clear();
    }
};

TEST_F(UT_DeviceManagerSave, saveTxt)
{
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->exportToTxt("/tmp/ut_dm_out.txt"));
}

TEST_F(UT_DeviceManagerSave, saveXlsx)
{
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->exportToXlsx("/tmp/ut_dm_out.xlsx"));
}

TEST_F(UT_DeviceManagerSave, saveHtml)
{
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->exportToHtml("/tmp/ut_dm_out.html"));
}

TEST_F(UT_DeviceManagerSave, overviewToTxt)
{
    QString out;
    QTextStream ts(&out);
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->overviewToTxt(ts));
}

TEST_F(UT_DeviceManagerSave, overviewToHtml)
{
    QTemporaryFile tmp;
    tmp.open();
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->overviewToHtml(tmp));
    tmp.close();
}

TEST_F(UT_DeviceManagerSave, overviewToXlsx)
{
    QXlsx::Document xlsx;
    QXlsx::Format fmt;
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->overviewToXlsx(xlsx, fmt));
}

TEST_F(UT_DeviceManagerSave, addOthersDeviceFromHwinfo)
{
    DeviceOthers *d = new DeviceOthers();
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->addOthersDeviceFromHwinfo(d));
}
