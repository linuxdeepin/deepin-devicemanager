// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 commontools 未测方法: parseEDID(桩 QProcess) + preGenerateGpuInfo(冒烟)。

#include "commontools.h"
#include "ut_Head.h"
#include "stub.h"

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QByteArray>

#include <gtest/gtest.h>

class UT_CommonToolsMore : public UT_HEAD
{
};

// QProcess 桩(成员方法,首参 this)
static void ut_cm_start(QProcess * /*self*/, const QString & /*p*/, const QStringList & /*a*/, QIODevice::OpenMode /*m*/)
{
    return;
}
static bool ut_cm_wait(QProcess * /*self*/, int /*msecs*/)
{
    return true;
}
static QByteArray ut_cm_readEmpty(QProcess * /*self*/)
{
    return QByteArray();
}

// parseEDID: 空 list 不触发 QProcess
TEST_F(UT_CommonToolsMore, parseEDID_emptyList)
{
    EXPECT_NO_FATAL_FAILURE(CommonTools::parseEDID(QStringList(), "DP-1", false));
    EXPECT_NO_FATAL_FAILURE(CommonTools::parseEDID(QStringList(), "DP-1", true));
}

// parseEDID: 有 list + 桩 QProcess 空输出(continue,不崩)
TEST_F(UT_CommonToolsMore, parseEDID_stubbedEmptyOutput)
{
    Stub stub;
    stub.set(((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start)),
             ut_cm_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_cm_wait);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_cm_readEmpty);
    EXPECT_NO_FATAL_FAILURE(CommonTools::parseEDID(QStringList() << "/tmp/fake.edid", "DP-1", false));
}

// preGenerateGpuInfo: 冒烟(static 缓存,首次真实 glxinfo 可能慢,桩 QProcess)
TEST_F(UT_CommonToolsMore, preGenerateGpuInfo_stubbed)
{
    Stub stub;
    stub.set(((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start)),
             ut_cm_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_cm_wait);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_cm_readEmpty);
    EXPECT_NO_FATAL_FAILURE({ QString r = CommonTools::preGenerateGpuInfo(); (void)r; });
}
