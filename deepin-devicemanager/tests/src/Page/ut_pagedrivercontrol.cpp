// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageDriverControl.h"
#include "DBusDriverInterface.h"
#include "DriverWaitingWidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <DWaterProgress>
#include <DTitlebar>
#include <DIconButton>

#include <QKeyEvent>
#include <QEvent>
#include <QCloseEvent>
#include <QSignalSpy>
#include <QDBusUnixFileDescriptor>
#include <QTemporaryFile>
#include <QTest>

#include "UrlChooserEdit.h"
#include "GetDriverPathWidget.h"

#include <gtest/gtest.h>

class UT_PageDriverControl : public UT_HEAD
{
public:
    void SetUp()
    {
        m_PageDriverControlInstall = new PageDriverControl(nullptr, "Update Drivers", true, "camera", "");
        m_PageDriverControlUninstall = new PageDriverControl(nullptr, "Update Drivers", false, "camera", "");
    }
    void TearDown()
    {
        delete m_PageDriverControlInstall;
        delete m_PageDriverControlUninstall;
    }

    PageDriverControl *m_PageDriverControlInstall = nullptr;
    PageDriverControl *m_PageDriverControlUninstall = nullptr;
};

TEST_F(UT_PageDriverControl, UT_PageDriverControl_isRunning)
{
    EXPECT_FALSE(m_PageDriverControlInstall->isRunning());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotBtnCancel)
{
    m_PageDriverControlInstall->slotBtnCancel();
    EXPECT_TRUE(m_PageDriverControlInstall->isHidden());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotBtnNext_002)
{
    m_PageDriverControlUninstall->slotBtnNext();
    EXPECT_EQ(1, m_PageDriverControlUninstall->mp_stackWidget->currentIndex());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotProcessChange)
{
    m_PageDriverControlInstall->slotProcessChange(40, "ee");
    EXPECT_EQ(40, m_PageDriverControlInstall->mp_WaitDialog->mp_Progress->value());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotProcessEnd)
{
    m_PageDriverControlInstall->slotProcessEnd(false, "Unknown error");
    EXPECT_EQ(1, m_PageDriverControlInstall->mp_stackWidget->currentIndex());

}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotClose)
{
    m_PageDriverControlInstall->slotClose();
    EXPECT_TRUE(m_PageDriverControlInstall->isHidden());

}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotBackPathPage)
{
    m_PageDriverControlInstall->slotBackPathPage();
    EXPECT_EQ(0, m_PageDriverControlInstall->mp_stackWidget->currentIndex());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_removeBtn)
{
    m_PageDriverControlInstall->removeBtn();
    EXPECT_EQ(0, m_PageDriverControlInstall->buttonCount());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_installErrorTips)
{
    bool ret = m_PageDriverControlInstall->installErrorTips("devicemanager.deb");
    EXPECT_FALSE(ret);
}

bool ut_pagedrivercontrol_isDebValidFd(const QDBusUnixFileDescriptor &)
{
    return false;
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_installErrorTips_002)
{
    Stub stub;
    stub.set(ADDR(DBusDriverInterface, isDebValidFd), ut_pagedrivercontrol_isDebValidFd);

    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    tmpFile.close();
    bool ret = m_PageDriverControlInstall->installErrorTips(tmpFile.fileName());
    EXPECT_FALSE(ret);
}

bool ut_pagedrivercontrol_isDebValidFd_true(const QDBusUnixFileDescriptor &)
{
    return true;
}

bool ut_pagedrivercontrol_isArchMatchedFd(const QDBusUnixFileDescriptor &)
{
    return true;
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_installErrorTips_003)
{
    Stub stub;
    stub.set(ADDR(DBusDriverInterface, isDebValidFd), ut_pagedrivercontrol_isDebValidFd_true);
    stub.set(ADDR(DBusDriverInterface, isArchMatchedFd), ut_pagedrivercontrol_isArchMatchedFd);

    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    tmpFile.close();
    bool ret = m_PageDriverControlInstall->installErrorTips(tmpFile.fileName());
    EXPECT_TRUE(ret);
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_F4, Qt::AltModifier);
    EXPECT_TRUE(m_PageDriverControlInstall->event(&keyPressEvent));
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_installDriverLogical_001)
{
    m_PageDriverControlInstall->installDriverLogical();
    EXPECT_GE(1, m_PageDriverControlInstall->mp_stackWidget->currentIndex());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_installDriverLogical_002)
{
    m_PageDriverControlInstall->mp_stackWidget->setCurrentIndex(1);
    m_PageDriverControlInstall->installDriverLogical();
    EXPECT_EQ(1, m_PageDriverControlInstall->mp_stackWidget->currentIndex());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_closeEvent_Doing)
{
    m_PageDriverControlInstall->setProperty("DriverProcessStatus", "Doing");
    QCloseEvent closeEvent;
    QApplication::sendEvent(m_PageDriverControlInstall, &closeEvent);
    EXPECT_FALSE(closeEvent.isAccepted());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_closeEvent_Done)
{
    m_PageDriverControlInstall->setProperty("DriverProcessStatus", "Done");
    QSignalSpy spy(m_PageDriverControlInstall, &PageDriverControl::refreshInfo);
    QCloseEvent closeEvent;
    QApplication::sendEvent(m_PageDriverControlInstall, &closeEvent);
    // Done 状态下触发 slotClose:发 refreshInfo 后 close();close 内部再次触发 closeEvent
    // (属性仍为 Done)导致 slotClose 重入,refreshInfo 共发出 2 次
    EXPECT_EQ(2, spy.count());
    EXPECT_TRUE(m_PageDriverControlInstall->isHidden());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_closeEvent_normal)
{
    // 未设置 DriverProcessStatus 属性时,closeEvent 既不忽略也不触发刷新
    QSignalSpy spy(m_PageDriverControlInstall, &PageDriverControl::refreshInfo);
    QCloseEvent closeEvent;
    QApplication::sendEvent(m_PageDriverControlInstall, &closeEvent);
    EXPECT_TRUE(closeEvent.isAccepted());
    EXPECT_EQ(0, spy.count());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_enableCloseBtn)
{
    // 禁用关闭按钮
    m_PageDriverControlInstall->enableCloseBtn(false);
    DTitlebar *titlebar = m_PageDriverControlInstall->findChild<DTitlebar *>();
    ASSERT_NE(nullptr, titlebar);
    DIconButton *closeBtn = titlebar->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    if (closeBtn) {
        EXPECT_FALSE(closeBtn->isEnabled());
    }
    // 恢复关闭按钮
    m_PageDriverControlInstall->enableCloseBtn(true);
    if (closeBtn) {
        EXPECT_TRUE(closeBtn->isEnabled());
    }
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_errMsg_known)
{
    // 已知错误码 ENOENT(2) -> "The driver module was not found"
    EXPECT_STREQ("The driver module was not found", m_PageDriverControlInstall->errMsg("2").toStdString().c_str());
    // EAGAIN(11) -> 驱动模块被依赖
    EXPECT_STREQ("The driver module has dependencies", m_PageDriverControlInstall->errMsg("11").toStdString().c_str());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_errMsg_unknown)
{
    // 未知错误码 -> 返回 Unknown error
    EXPECT_STREQ("Unknown error", m_PageDriverControlInstall->errMsg("99999").toStdString().c_str());
    EXPECT_STREQ("Unknown error", m_PageDriverControlInstall->errMsg("abc").toStdString().c_str());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotBtnNext_001)
{
    // 安装流程:路径页时点击下一步;路径为空应停留在路径页并提示
    ASSERT_EQ(2, m_PageDriverControlInstall->buttonCount());
    m_PageDriverControlInstall->mp_PathDialog->mp_fileChooseEdit->mp_folderPath = "";
    m_PageDriverControlInstall->getButton(1)->click();
    EXPECT_EQ(0, m_PageDriverControlInstall->mp_stackWidget->currentIndex());
    EXPECT_EQ(2, m_PageDriverControlInstall->buttonCount());

    // 路径指向不存在的目录时同样停留路径页
    m_PageDriverControlInstall->mp_PathDialog->mp_fileChooseEdit->mp_folderPath = "/no/such/dir/ut-t11";
    m_PageDriverControlInstall->getButton(1)->click();
    EXPECT_EQ(0, m_PageDriverControlInstall->mp_stackWidget->currentIndex());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_uninstallDriverLogical)
{
    m_PageDriverControlUninstall->uninstallDriverLogical();
    // 进入正在卸载页(索引 0 -> 1),按钮清空,状态置为 Doing
    EXPECT_EQ(1, m_PageDriverControlUninstall->mp_stackWidget->currentIndex());
    EXPECT_EQ(0, m_PageDriverControlUninstall->buttonCount());
    EXPECT_STREQ("Doing", m_PageDriverControlUninstall->property("DriverProcessStatus").toString().toStdString().c_str());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_uninstallDriverLogical_printer)
{
    // 打印机驱动卸载走 uninstallPrinter 分支
    PageDriverControl printerControl(nullptr, "Uninstall Drivers", false, "printer1", "drv1", "HP", "LaserJet");
    printerControl.uninstallDriverLogical();
    EXPECT_EQ(1, printerControl.mp_stackWidget->currentIndex());
    EXPECT_STREQ("Doing", printerControl.property("DriverProcessStatus").toString().toStdString().c_str());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotProcessEnd_success)
{
    m_PageDriverControlInstall->slotProcessEnd(true, "");
    // 成功后新增结果页并切换,状态置为 Done,新增 OK 按钮
    EXPECT_EQ(1, m_PageDriverControlInstall->mp_stackWidget->currentIndex());
    EXPECT_EQ(3, m_PageDriverControlInstall->buttonCount());
    EXPECT_STREQ("Done", m_PageDriverControlInstall->property("DriverProcessStatus").toString().toStdString().c_str());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_slotProcessEnd_fail_knownErr)
{
    m_PageDriverControlInstall->slotProcessEnd(false, "2");
    // 失败后展示错误信息页,状态置为 Done
    EXPECT_EQ(1, m_PageDriverControlInstall->mp_stackWidget->currentIndex());
    EXPECT_STREQ("Done", m_PageDriverControlInstall->property("DriverProcessStatus").toString().toStdString().c_str());
}

TEST_F(UT_PageDriverControl, UT_PageDriverControl_isRunning_visible)
{
    // isRunning 为静态方法:遍历 qApp->allWindows() 的 QWindow 对象名
    // (生产代码从未给任何窗口设置 "PageDriverControlWindow",此处验证可识别已标记窗口)
    PageDriverControl dlg(nullptr, "Update Drivers", true, "camera", "");
    dlg.show();
    QWindow *win = dlg.windowHandle();
    ASSERT_NE(nullptr, win);
    win->setObjectName("PageDriverControlWindow");
    EXPECT_TRUE(PageDriverControl::isRunning());
    EXPECT_TRUE(dlg.isRunning());
    // 清理标记,避免影响其它用例的 allWindows 遍历
    win->setObjectName("PageDriverControl");
    EXPECT_FALSE(PageDriverControl::isRunning());
}
