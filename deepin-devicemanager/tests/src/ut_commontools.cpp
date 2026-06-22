// Copyright (C) 2019 ~ 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QRegularExpression>

#define private public
#define protected public
#include "commontools.h"

class UT_CommonTools : public UT_HEAD
{
};

// ===================== getDriverType =====================
TEST_F(UT_CommonTools, UT_CommonTools_getDriverType)
{
    EXPECT_STREQ("Display adapter", CommonTools::getDriverType(DR_Gpu).toStdString().c_str());
    EXPECT_STREQ("Bluetooth adapter", CommonTools::getDriverType(DR_Bluetooth).toStdString().c_str());
    EXPECT_STREQ("Network adapter", CommonTools::getDriverType(DR_Network).toStdString().c_str());
    EXPECT_STREQ("Sound card", CommonTools::getDriverType(DR_Sound).toStdString().c_str());
    EXPECT_STREQ("Printer", CommonTools::getDriverType(DR_Printer).toStdString().c_str());
}

// ===================== getDriverPixmap =====================
TEST_F(UT_CommonTools, UT_CommonTools_getDriverPixmap)
{
    EXPECT_STREQ(":/icons/deepin/builtin/icons/GPU.svg", CommonTools::getDriverPixmap(DR_Gpu).toStdString().c_str());
    EXPECT_STREQ(":/icons/deepin/builtin/icons/bluetooth.svg", CommonTools::getDriverPixmap(DR_Bluetooth).toStdString().c_str());
}

// ===================== getStausType =====================
TEST_F(UT_CommonTools, UT_CommonTools_getStausType)
{
    EXPECT_STREQ("Installation successful", CommonTools::getStausType(ST_SUCESS).toStdString().c_str());
    EXPECT_STREQ("Installation failed", CommonTools::getStausType(ST_FAILED).toStdString().c_str());
    EXPECT_STREQ("Not installed", CommonTools::getStausType(ST_NOT_INSTALL).toStdString().c_str());
    EXPECT_STREQ("Waiting", CommonTools::getStausType(ST_WAITING).toStdString().c_str());
}

// ===================== getStatusPixmap =====================
TEST_F(UT_CommonTools, UT_CommonTools_getStatusPixmap)
{
    EXPECT_STREQ(":/icons/deepin/builtin/icons/ok.svg", CommonTools::getStatusPixmap(ST_SUCESS).toStdString().c_str());
    EXPECT_STREQ(":/icons/deepin/builtin/icons/warning.svg", CommonTools::getStatusPixmap(ST_FAILED).toStdString().c_str());
    EXPECT_TRUE(CommonTools::getStatusPixmap(ST_DOWNLOADING).isEmpty());
}

// ===================== getErrorString =====================
TEST_F(UT_CommonTools, UT_CommonTools_getErrorString)
{
    EXPECT_STREQ("Unknown error", CommonTools::getErrorString(EC_NULL).toStdString().c_str());
    EXPECT_STREQ("Network error", CommonTools::getErrorString(EC_NETWORK).toStdString().c_str());
    EXPECT_STREQ("Canceled", CommonTools::getErrorString(EC_CANCEL).toStdString().c_str());
}

// ===================== getBackupPath =====================
TEST_F(UT_CommonTools, UT_CommonTools_getBackupPath)
{
    EXPECT_STREQ("/var/lib/deepin-devicemanager/", CommonTools::getBackupPath().toStdString().c_str());
}

// ===================== getSystemTime =====================
TEST_F(UT_CommonTools, UT_CommonTools_getSystemTime)
{
    QString t = CommonTools::getSystemTime();
    EXPECT_FALSE(t.isEmpty());
    // 格式 "yyyy-M-d hh:mm"，例如 "2026-6-22 12:30"
    QRegularExpression re("^\\d{4}-\\d{1,2}-\\d{1,2} \\d{1,2}:\\d{2}$");
    EXPECT_TRUE(re.match(t).hasMatch());
}

// ===================== getUrl =====================
TEST_F(UT_CommonTools, UT_CommonTools_getUrl)
{
    QString url = CommonTools::getUrl();
    EXPECT_TRUE(url.contains("uniontech.com"));
}

// ===================== 构造/静态表冒烟测试 =====================
TEST_F(UT_CommonTools, UT_CommonTools_staticMaps)
{
    EXPECT_TRUE(CommonTools::m_MapDriverType.contains(DR_Gpu));
    EXPECT_TRUE(CommonTools::m_MapDriverIcon.contains(DR_Network));
    EXPECT_TRUE(CommonTools::m_MapStatusType.contains(ST_SUCESS));
    EXPECT_TRUE(CommonTools::m_MapErrorString.contains(EC_NULL));
}
