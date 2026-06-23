// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DetectedStatusWidget 的状态切换 UI 方法(setXxxUI) + refreshUI(7分支) + slot。

#include "DetectedStatusWidget.h"
#include "MacroDefinition.h"
#include "ut_Head.h"

#include <QString>

#include <gtest/gtest.h>

class UT_DetectedStatusWidget : public UT_HEAD
{
public:
    void SetUp() override
    {
        w = new DetectedStatusWidget;
    }
    void TearDown() override
    {
        delete w;
    }
    DetectedStatusWidget *w = nullptr;
};

TEST_F(UT_DetectedStatusWidget, setDetectFinishUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setDetectFinishUI("6", "Lenovo M90", true));
    EXPECT_NO_FATAL_FAILURE(w->setDetectFinishUI("0", "Lenovo M90", false));
}

TEST_F(UT_DetectedStatusWidget, setDownloadUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setDownloadUI(DR_Gpu, "5MB/s", "10MB", "100MB", 50));
}

TEST_F(UT_DetectedStatusWidget, setInstallUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setInstallUI(DR_Sound, "Audio driver", 60));
}

TEST_F(UT_DetectedStatusWidget, setInstallSuccessUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setInstallSuccessUI("5", "1"));   // failed>0
    EXPECT_NO_FATAL_FAILURE(w->setInstallSuccessUI("6", "0"));   // failed<=0
}

TEST_F(UT_DetectedStatusWidget, setInstallFailedUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setInstallFailedUI());
}

TEST_F(UT_DetectedStatusWidget, setNetworkErrorUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setNetworkErrorUI("5MB/s", 50));
}

TEST_F(UT_DetectedStatusWidget, setNoUpdateDriverUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setNoUpdateDriverUI("Lenovo M90"));
}

TEST_F(UT_DetectedStatusWidget, refreshUI_allBranches)
{
    Status types[] = {ST_NOT_INSTALL, ST_DOWNLOADING, ST_INSTALL, ST_SUCESS,
                      ST_FAILED, ST_NetWorkErr, ST_DRIVER_IS_NEW};
    for (Status s : types) {
        EXPECT_NO_FATAL_FAILURE(w->refreshUI(s));
    }
}

TEST_F(UT_DetectedStatusWidget, setInstallBackupBtnEnable)
{
    EXPECT_NO_FATAL_FAILURE(w->setInstallBtnEnable(true));
    EXPECT_NO_FATAL_FAILURE(w->setBackupBtnEnable(false));
    EXPECT_NO_FATAL_FAILURE(w->setReDetectEnable(true));
}

TEST_F(UT_DetectedStatusWidget, backupUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setNoBackupDriverUI(0, 0));
    EXPECT_NO_FATAL_FAILURE(w->setBackableDriverUI(5, 2));    // backedup>0
    EXPECT_NO_FATAL_FAILURE(w->setBackableDriverUI(0, 0));    // backedup<=0
    EXPECT_NO_FATAL_FAILURE(w->setBackingUpDriverUI("driver", 100, 50));
    EXPECT_NO_FATAL_FAILURE(w->setBackupSuccessUI(5, 1));     // success>0
    EXPECT_NO_FATAL_FAILURE(w->setBackupSuccessUI(0, 1));     // success<=0
}

TEST_F(UT_DetectedStatusWidget, restoreUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setRestoreDriverUI(3));
    EXPECT_NO_FATAL_FAILURE(w->setRestoringUI(50, "driver"));
}

TEST_F(UT_DetectedStatusWidget, slots_emitSignals)
{
    EXPECT_NO_FATAL_FAILURE(w->slotInstall());
    EXPECT_NO_FATAL_FAILURE(w->slotBackup());
    EXPECT_NO_FATAL_FAILURE(w->slotReDetectSlot());
    EXPECT_NO_FATAL_FAILURE(w->slotCancel());
    EXPECT_NO_FATAL_FAILURE(w->onUpdateTheme());
}
