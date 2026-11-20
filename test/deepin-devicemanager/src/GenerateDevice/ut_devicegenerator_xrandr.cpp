// Copyright (C) 2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// DeviceGenerator::getMonitorWithoutEdidFromXrandr 单元测试
// 验证 QRegExp -> QRegularExpression 替换后解析行为不变。
// ============================================================================

#include "DeviceGenerator.h"
#include "DeviceManager.h"
#include "DeviceMonitor.h"
#include "stub.h"
#include "ut_Head.h"

#include <QProcess>
#include <QByteArray>
#include <QStringList>

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

// ============================ QProcess 桩 ============================

static int s_dgxr_exitCode = 0;
static QByteArray s_dgxr_output;

static void ut_dgxr_start(void *, const QString &, const QStringList &, QIODevice::OpenMode) {}
static bool ut_dgxr_waitForFinished(void *, int) { return true; }
static int ut_dgxr_exitCode(void *) { return s_dgxr_exitCode; }
static QByteArray ut_dgxr_readAllStandardOutput(void *) { return s_dgxr_output; }

// ============================ Fixture ============================

class UT_DeviceGeneratorXrandr : public UT_HEAD
{
public:
    void SetUp() override
    {
        DeviceManager::instance()->clear();
        s_dgxr_exitCode = 0;
        s_dgxr_output.clear();
        m_stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(
                       &QProcess::start), ut_dgxr_start);
        m_stub.set(ADDR(QProcess, waitForFinished), ut_dgxr_waitForFinished);
        m_stub.set(ADDR(QProcess, exitCode), ut_dgxr_exitCode);
        m_stub.set(ADDR(QProcess, readAllStandardOutput), ut_dgxr_readAllStandardOutput);
        m_dg = new DeviceGenerator;
    }

    void TearDown() override
    {
        delete m_dg;
        m_dg = nullptr;
        DeviceManager::instance()->clear();
    }

    Stub m_stub;
    DeviceGenerator *m_dg = nullptr;
};

// 已连接但携带 EDID -> 本函数不补充建设备（EDID 正则替换回归）
TEST_F(UT_DeviceGeneratorXrandr, withEdid_noMonitor)
{
    QString out = "HDMI-1 connected primary 1920x1080+0+0 (0x46) normal 527mm x 296mm\n"
                  "\t\t00ffffffffffff005a63384001010101\n"
                  "  1920x1080 (0x46) 148.500MHz +HSync +VSync *current +preferred\n"
                  "        v: height 1080 start 1084 end 1089 total 1125           clock 60.00Hz\n";
    s_dgxr_output = out.toUtf8();

    m_dg->getMonitorWithoutEdidFromXrandr();

    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMonitor.size(), 0);
}

// 已连接、无 EDID -> 建设备并校验关键字段（分辨率/刷新率正则替换回归）
TEST_F(UT_DeviceGeneratorXrandr, connectedWithoutEdid_createsMonitor)
{
    QString out = "HDMI-1 connected primary 1920x1080+0+0 (0x46) normal 527mm x 296mm\n"
                  "  1920x1080 (0x46) 148.500MHz +HSync +VSync *current +preferred\n"
                  "        v: height 1080 start 1084 end 1089 total 1125           clock 60.00Hz\n";
    s_dgxr_output = out.toUtf8();

    m_dg->getMonitorWithoutEdidFromXrandr();

    ASSERT_EQ(DeviceManager::instance()->m_ListDeviceMonitor.size(), 1);
    DeviceMonitor *mon = dynamic_cast<DeviceMonitor *>(DeviceManager::instance()->m_ListDeviceMonitor[0]);
    ASSERT_NE(mon, nullptr);
    EXPECT_EQ(mon->m_Interface, QString("HDMI"));
    EXPECT_EQ(mon->m_MainScreen, QString("Yes"));
    EXPECT_EQ(mon->m_RefreshRate, QString("60.00Hz"));
    EXPECT_EQ(mon->m_SupportResolution, QString("1920×1080@60.00Hz"));
}

// 空输出 -> 早退，不建设备
TEST_F(UT_DeviceGeneratorXrandr, emptyOutput_noMonitor)
{
    m_dg->getMonitorWithoutEdidFromXrandr();

    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceMonitor.size(), 0);
}
