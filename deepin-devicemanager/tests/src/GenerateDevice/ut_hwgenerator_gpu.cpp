// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 HWGenerator::generatorGpuDevice 的 QProcess 解析分支(桩 QProcess,使 exitCode!=127/126 进入解析)。

#include "ut_Head.h"
#include "stub.h"
#include "DeviceManager.h"

#include <QProcess>
#include <QByteArray>

#include <gtest/gtest.h>

#define private public
#define protected public
#include "HWGenerator.h"

static void ut_hwg_start(QProcess * /*self*/, const QString & /*p*/, const QStringList & /*a*/, QIODevice::OpenMode /*m*/)
{
    return;
}
static bool ut_hwg_wait(QProcess * /*self*/, int /*msecs*/)
{
    return true;
}
static int ut_hwg_exitCode(QProcess * /*self*/)
{
    return 0;   // 非 127/126,进入解析
}
static QByteArray ut_hwg_read(QProcess * /*self*/)
{
    return QByteArray("Intel UHD Graphics\n");
}

class UT_HWGeneratorGpu : public UT_HEAD
{
public:
    void SetUp() override
    {
        DeviceManager::instance()->clear();
        m_gen = new HWGenerator;
    }
    void TearDown() override
    {
        delete m_gen;
        DeviceManager::instance()->clear();
    }
    HWGenerator *m_gen = nullptr;
};

TEST_F(UT_HWGeneratorGpu, generatorGpuDevice_qprocessStubbed)
{
    Stub stub;
    stub.set(((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start)),
             ut_hwg_start);
    stub.set(((bool (QProcess::*)(int))ADDR(QProcess, waitForFinished)), ut_hwg_wait);
    stub.set(((int (QProcess::*)())ADDR(QProcess, exitCode)), ut_hwg_exitCode);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_hwg_read);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorGpuDevice());
}
