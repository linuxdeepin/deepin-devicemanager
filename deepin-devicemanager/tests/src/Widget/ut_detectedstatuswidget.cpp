// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * ut_detectedstatuswidget.cpp — DetectedStatusWidget 单元测试
 *
 * 分支映射(与 .ut/inventory.json 分级一致, 共 31 方法, slotBackupPath 除外):
 * - high: refreshUI(3 用例, 覆盖 ST_NOT_INSTALL/ST_DOWNLOADING/ST_INSTALL/ST_SUCESS
 *         /ST_FAILED/ST_NetWorkErr/ST_DRIVER_IS_NEW 七个分支 + default)
 * - mid : 构造(1)/initUI(1)/hideAll(1)/setDetectFinishUI(2)/setDownloadUI(2)
 *         /setInstallUI(1)/setInstallSuccessUI(2)/setInstallFailedUI(1)
 *         /setNetworkErrorUI(1)/setNoUpdateDriverUI(1)/setNoBackupDriverUI(1)
 *         /setBackableDriverUI(1)/setBackingUpDriverUI(1)/setBackupSuccessUI(2)
 *         /setRestoreDriverUI(1)/setRestoringUI(1)
 * - low : setInstallBtnEnable(1)/setBackupBtnEnable(1)/setReDetectEnable(1)
 *         /slotInstall(1)/slotBackup(1)/slotReDetectSlot(1)/slotCancel(1)
 *         /slotFeedBack(1)/slotReboot(1)/onUpdateTheme(1)/paintEvent(1)/析构(1)
 *
 * 说明:
 * - 访问 private 成员(mp_UpdateLabel/mp_Progress 等)与 protected paintEvent,
 *   采用 private public / protected public 注入模式。
 * - slotFeedBack/slotReboot 依赖 DBus, 由 test_main 的 ut_dbus_call 打桩承接。
 * - slotBackupPath 调用 DDesktopServices::showFolder 会拉起文件管理器进程,
 *   依赖真实桌面环境, 按技能约定排除(测试不得依赖真实环境)。
 * - 文案断言使用 contains() 关键子串, 避免翻译环境影响。
 */

#define private public
#define protected public
#include "DetectedStatusWidget.h"
#include "BtnLabel.h"
#undef protected
#undef private

#include "MacroDefinition.h"
#include "ut_Head.h"

#include <QString>
#include <QPainter>
#include <QPixmap>
#include <QSignalSpy>

#include <gtest/gtest.h>

class UT_DetectedStatusWidget : public ::testing::Test
{
public:
    void SetUp()
    {
        w = new DetectedStatusWidget;
        w->resize(600, 300);
    }
    void TearDown()
    {
        delete w;
        w = nullptr;
    }

    DetectedStatusWidget *w = nullptr;
};

// ===================== 构造 / initUI / 析构 =====================

TEST_F(UT_DetectedStatusWidget, Constructor_Created_WidgetsAndObjectNamesValid)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 读取控件对象名与成员
    QString installName = w->mp_InstallButton->objectName();
    QString cancelName = w->mp_CancelButton->objectName();

    // Assert
    EXPECT_EQ(installName, "MpInstallButton");
    EXPECT_EQ(cancelName, "MpCancelButton");
    EXPECT_EQ(w->mp_ReDetectedSgButton->objectName(), "MpReDetectedSgButton");
    EXPECT_EQ(w->mp_BackupSgButton->objectName(), "MpBackupSgButton");
    EXPECT_EQ(w->mp_ReDetectedIconButton->objectName(), "MpReDetectedIconButton");
    EXPECT_NE(w->mp_Progress, nullptr);
    EXPECT_NE(w->mp_UpdateLabel, nullptr);
}

TEST_F(UT_DetectedStatusWidget, InitUI_AfterConstruct_LabelsConfigured)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // 构造函数已调用 initUI

    // Act: 读取标签初始配置
    bool picHasWidget = (w->mp_PicLabel->parentWidget() == w);
    bool layoutValid = (w->mp_HLayoutTotal != nullptr);

    // Assert
    EXPECT_TRUE(picHasWidget);
    EXPECT_TRUE(layoutValid);
    EXPECT_NE(w->mp_HLayoutButton, nullptr);
    EXPECT_NE(w->mp_VLayoutLabel, nullptr);
    EXPECT_NE(w->mp_HLayoutLabel, nullptr);
}

TEST_F(UT_DetectedStatusWidget, Destructor_DeleteAfterUse_NoLeakOrCrash)
{
    // Arrange: 独立构造并使用一个实例
    DetectedStatusWidget *tmp = new DetectedStatusWidget;
    ASSERT_NE(tmp, nullptr);
    tmp->setDownloadUI(DR_Gpu, "5MB/s", "1MB", "10MB", 10);

    // Act: 删除后重建一个新实例验证可继续正常构造
    delete tmp;

    // Assert
    DetectedStatusWidget *another = new DetectedStatusWidget;
    EXPECT_NE(another, nullptr);
    EXPECT_NE(another->mp_PicLabel, nullptr);
    delete another;
}

// ===================== hideAll (mid) =====================

TEST_F(UT_DetectedStatusWidget, HideAll_AfterUIBuilt_ResetsLayouts)
{
    // Arrange: 先构建一个界面
    w->setDetectFinishUI("5", "PC", true);
    ASSERT_NE(w->layout(), nullptr);

    // Act: 隐藏全部并重建布局成员
    w->hideAll();

    // Assert: 顶层布局被删除未挂回, 布局成员重新分配
    EXPECT_EQ(w->layout(), nullptr);
    EXPECT_NE(w->mp_HLayoutTotal, nullptr);
    EXPECT_TRUE(w->mp_UpdateLabel->isHidden());
    EXPECT_TRUE(w->mp_InstallButton->isHidden());
}

// ===================== setDetectFinishUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetDetectFinishUI_HasInstall_ButtonEnabledAndTextsSet)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 设置检测完成界面(有可安装驱动)
    w->setDetectFinishUI("5", "Lenovo M90", true);

    // Assert
    EXPECT_TRUE(w->mp_InstallButton->isEnabled());
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("5"));
    EXPECT_EQ(w->mp_ModelLabel->text(), "Lenovo M90");
    EXPECT_EQ(w->mp_ModelLabel->toolTip(), "Lenovo M90");
    EXPECT_FALSE(w->mp_TimeLabel->text().isEmpty());
}

TEST_F(UT_DetectedStatusWidget, SetDetectFinishUI_NoInstall_ButtonDisabled)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 设置检测完成界面(无可安装驱动)
    w->setDetectFinishUI("0", "PC-2024", false);

    // Assert
    EXPECT_FALSE(w->mp_InstallButton->isEnabled());
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("0"));
    EXPECT_EQ(w->mp_ModelLabel->text(), "PC-2024");
}

// ===================== setDownloadUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetDownloadUI_ProgressMidway_CancelEnabled)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 下载中途(进度 50%)
    w->setDownloadUI(DR_Gpu, "5MB/s", "50MB", "100MB", 50);

    // Assert
    EXPECT_EQ(w->mp_Progress->value(), 50);
    EXPECT_TRUE(w->mp_CancelButton->isEnabled());
    EXPECT_TRUE(w->mp_ModelLabel->text().contains("5MB/s"));
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("Downloading"));
}

TEST_F(UT_DetectedStatusWidget, SetDownloadUI_ProgressNearEnd_CancelDisabled)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 下载接近完成(进度 95%)
    w->setDownloadUI(DR_Gpu, "9MB/s", "95MB", "100MB", 95);

    // Assert
    EXPECT_EQ(w->mp_Progress->value(), 95);
    EXPECT_FALSE(w->mp_CancelButton->isEnabled());
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("Downloading"));
}

// ===================== setInstallUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetInstallUI_ProgressAndDescriptionSet_CancelDisabled)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 安装界面, 进度 60
    w->setInstallUI(DR_Sound, "Audio driver v2", 60);

    // Assert
    EXPECT_EQ(w->mp_Progress->value(), 60);
    EXPECT_EQ(w->mp_ModelLabel->text(), "Audio driver v2");
    EXPECT_FALSE(w->mp_CancelButton->isEnabled());
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("Installing"));
}

// ===================== setInstallSuccessUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetInstallSuccessUI_WithFailed_ShowsBothCounts)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 4 成功 2 失败
    w->setInstallSuccessUI("4", "2");

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("4"));
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("2"));
    EXPECT_NE(w->mp_RebootLabel, nullptr);
}

TEST_F(UT_DetectedStatusWidget, SetInstallSuccessUI_AllSuccess_ShowsSuccessCount)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 6 成功 0 失败
    w->setInstallSuccessUI("6", "0");

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("6"));
    EXPECT_FALSE(w->mp_UpdateLabel->text().contains("failed"));
    EXPECT_NE(w->mp_PicLabel, nullptr);
}

// ===================== setInstallFailedUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetInstallFailedUI_Called_ShowsFailedText)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 安装失败界面
    w->setInstallFailedUI();

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("Failed"));
    EXPECT_NE(w->mp_FeedBackLabel, nullptr);
    EXPECT_NE(w->mp_ReDetectedIconButton, nullptr);
}

// ===================== setNetworkErrorUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetNetworkErrorUI_Called_ShowsNetworkErrorAndSpeed)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 网络异常界面
    w->setNetworkErrorUI("0.5MB/s", 40);

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("Network error"));
    EXPECT_TRUE(w->mp_ModelLabel->text().contains("0.5MB/s"));
    EXPECT_TRUE(w->mp_CancelButton->isEnabled());
    EXPECT_NE(w->mp_CancelButton, nullptr);
}

// ===================== setNoUpdateDriverUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetNoUpdateDriverUI_Called_ShowsUpToDateAndModel)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 驱动已是最新界面
    w->setNoUpdateDriverUI("Lenovo M90");

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("up to date"));
    EXPECT_EQ(w->mp_ModelLabel->text(), "Lenovo M90");
    EXPECT_FALSE(w->mp_TimeLabel->text().isEmpty());
}

// ===================== setNoBackupDriverUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetNoBackupDriverUI_Called_ShowsBackupSummary)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // 场景: 3 可备份 + 4 已备份

    // Act: 无可备份驱动界面
    w->setNoBackupDriverUI(3, 4);

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("backed up"));
    EXPECT_TRUE(w->mp_ModelLabel->text().contains("7"));
    EXPECT_NE(w->mp_BackupPathLabel, nullptr);
}

// ===================== setBackableDriverUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetBackableDriverUI_Called_ShowsRestorableCount)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 可备份驱动界面
    w->setBackableDriverUI(5, 2);

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("5"));
    EXPECT_NE(w->mp_BackupSgButton, nullptr);
    EXPECT_NE(w->mp_ReDetectedSgButton, nullptr);
}

// ===================== setBackingUpDriverUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetBackingUpDriverUI_Called_ProgressScaledByTen)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // 进度条默认量程 0-100, 传 5*10=50 确保在量程内

    // Act: 正在备份界面(进度 5/10)
    w->setBackingUpDriverUI("RTL8168", 10, 5);

    // Assert
    EXPECT_EQ(w->mp_Progress->value(), 50);
    EXPECT_TRUE(w->mp_ModelLabel->text().contains("RTL8168"));
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("Backing up"));
}

// ===================== setBackupSuccessUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetBackupSuccessUI_WithFailed_ShowsBothCounts)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 5 成功 1 失败
    w->setBackupSuccessUI(5, 1);

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("5"));
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("1"));
    EXPECT_NE(w->mp_ReDetectedIconButton, nullptr);
}

TEST_F(UT_DetectedStatusWidget, SetBackupSuccessUI_NoSuccess_ShowsFailureText)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // 仅 failed<=0 且 success<=0 时才走失败分支

    // Act: 0 成功 0 失败(备份失败)
    w->setBackupSuccessUI(0, 0);

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("Failed"));
    EXPECT_NE(w->mp_BackupSgButton, nullptr);
}

TEST_F(UT_DetectedStatusWidget, SetBackupSuccessUI_AllSuccess_ShowsSuccessCount)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 3 成功 0 失败
    w->setBackupSuccessUI(3, 0);

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("3"));
    EXPECT_FALSE(w->mp_UpdateLabel->text().contains("failed"));
    EXPECT_NE(w->mp_ReDetectedIconButton, nullptr);
}

// ===================== setRestoreDriverUI / setRestoringUI (mid) =====================

TEST_F(UT_DetectedStatusWidget, SetRestoreDriverUI_Called_ShowsRestorableCount)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 可还原界面
    w->setRestoreDriverUI(3);

    // Assert
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("3"));
    EXPECT_NE(w->mp_ReDetectedSgButton, nullptr);
    EXPECT_NE(w->mp_PicLabel, nullptr);
}

TEST_F(UT_DetectedStatusWidget, SetRestoringUI_Called_ProgressAndCancelEnabled)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 正在还原界面
    w->setRestoringUI(50, "RTL8168");

    // Assert
    EXPECT_EQ(w->mp_Progress->value(), 50);
    EXPECT_TRUE(w->mp_ModelLabel->text().contains("RTL8168"));
    EXPECT_TRUE(w->mp_CancelButton->isEnabled());
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("restoring"));
}

// ===================== refreshUI (high, 7 分支 + default) =====================

TEST_F(UT_DetectedStatusWidget, RefreshUI_NotInstallDownloadingInstall_RoutesToPages)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // 路由验证: ST_NOT_INSTALL->setDetectFinishUI("5",true), ST_DOWNLOADING->15%, ST_INSTALL->75%

    // Act: 未安装状态
    w->refreshUI(ST_NOT_INSTALL);

    // Assert: 路由到检测完成页, 安装按钮可用
    EXPECT_TRUE(w->mp_InstallButton->isEnabled());
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("5"));

    // Act: 下载中状态
    w->refreshUI(ST_DOWNLOADING);

    // Assert: 进度 28.96*100/192.78=15, 取消可用
    EXPECT_EQ(w->mp_Progress->value(), 15);
    EXPECT_TRUE(w->mp_CancelButton->isEnabled());

    // Act: 安装中状态
    w->refreshUI(ST_INSTALL);

    // Assert: 进度 75, 取消禁用
    EXPECT_EQ(w->mp_Progress->value(), 75);
    EXPECT_FALSE(w->mp_CancelButton->isEnabled());
}

TEST_F(UT_DetectedStatusWidget, RefreshUI_SucessFailedNetworkErr_RoutesToPages)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // 路由验证: ST_SUCESS->setInstallSuccessUI("4","2"), ST_FAILED->setInstallFailedUI, NetWorkErr->网络异常页

    // Act: 安装成功状态(4 成功 2 失败)
    w->refreshUI(ST_SUCESS);

    // Assert: 显示成功与失败计数
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("4"));
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("2"));

    // Act: 安装失败状态
    w->refreshUI(ST_FAILED);

    // Assert: 显示失败文案
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("Failed"));

    // Act: 网络异常状态
    w->refreshUI(ST_NetWorkErr);

    // Assert: 显示网络异常文案, 取消可用
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("Network error"));
    EXPECT_TRUE(w->mp_CancelButton->isEnabled());
    EXPECT_NE(w->mp_CancelButton, nullptr);
}

TEST_F(UT_DetectedStatusWidget, RefreshUI_DriverIsNewAndDefault_NoCrashAndRoutes)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // 路由验证: ST_DRIVER_IS_NEW->setNoUpdateDriverUI, 其余状态走 default 分支

    // Act: 最新驱动状态
    w->refreshUI(ST_DRIVER_IS_NEW);

    // Assert: 路由到无更新页
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("up to date"));
    EXPECT_FALSE(w->mp_TimeLabel->text().isEmpty());
    EXPECT_NE(w->mp_TimeLabel, nullptr);

    // Act: default 分支(ST_WAITING 不在 switch 内)
    w->refreshUI(ST_WAITING);

    // Assert: default 不改变界面, 仍为最新驱动文案
    EXPECT_TRUE(w->mp_UpdateLabel->text().contains("up to date"));
}

// ===================== 按钮开关 (low) =====================

TEST_F(UT_DetectedStatusWidget, SetInstallBtnEnable_Toggles_ReflectsState)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 禁用再启用
    w->setInstallBtnEnable(false);
    bool disabled = w->mp_InstallButton->isEnabled();
    w->setInstallBtnEnable(true);

    // Assert: 禁用状态立即生效, 重新启用后恢复
    EXPECT_FALSE(disabled);
    EXPECT_TRUE(w->mp_InstallButton->isEnabled());
    EXPECT_NE(w->mp_InstallButton, nullptr);
}

TEST_F(UT_DetectedStatusWidget, SetBackupBtnEnable_Toggles_ReflectsState)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 禁用再启用
    w->setBackupBtnEnable(false);
    bool disabled = w->mp_BackupSgButton->isEnabled();
    w->setBackupBtnEnable(true);

    // Assert: 禁用状态立即生效, 重新启用后恢复
    EXPECT_FALSE(disabled);
    EXPECT_TRUE(w->mp_BackupSgButton->isEnabled());
    EXPECT_NE(w->mp_BackupSgButton, nullptr);
}

TEST_F(UT_DetectedStatusWidget, SetReDetectEnable_Toggles_BothReDetectButtons)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 禁用再启用
    w->setReDetectEnable(false);
    bool sgDisabled = w->mp_ReDetectedSgButton->isEnabled();
    bool iconDisabled = w->mp_ReDetectedIconButton->isEnabled();
    w->setReDetectEnable(true);

    // Assert: 两个重新检测按钮同步失效, 再同步恢复
    EXPECT_FALSE(sgDisabled);
    EXPECT_FALSE(iconDisabled);
    EXPECT_TRUE(w->mp_ReDetectedSgButton->isEnabled());
    EXPECT_TRUE(w->mp_ReDetectedIconButton->isEnabled());
    EXPECT_NE(w->mp_ReDetectedIconButton, nullptr);
}

// ===================== 信号槽 (low) =====================

TEST_F(UT_DetectedStatusWidget, SlotInstall_Called_EmitsInstallAll)
{
    // Arrange: 监听 installAll 信号
    QSignalSpy spy(w, &DetectedStatusWidget::installAll);
    ASSERT_TRUE(spy.isValid());

    // Act: 触发安装槽
    w->slotInstall();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(w->mp_InstallButton != nullptr);
}

TEST_F(UT_DetectedStatusWidget, SlotBackup_Called_EmitsBackupAll)
{
    // Arrange: 监听 backupAll 信号
    QSignalSpy spy(w, &DetectedStatusWidget::backupAll);
    ASSERT_TRUE(spy.isValid());

    // Act: 触发备份槽
    w->slotBackup();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(w->mp_BackupSgButton != nullptr);
}

TEST_F(UT_DetectedStatusWidget, SlotReDetect_Called_EmitsRedetected)
{
    // Arrange: 监听 redetected 信号
    QSignalSpy spy(w, &DetectedStatusWidget::redetected);
    ASSERT_TRUE(spy.isValid());

    // Act: 触发重新检测槽
    w->slotReDetectSlot();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(w->mp_ReDetectedSgButton != nullptr);
}

TEST_F(UT_DetectedStatusWidget, SlotCancel_Called_EmitsCancelClicked)
{
    // Arrange: 监听 cancelClicked 信号
    QSignalSpy spy(w, &DetectedStatusWidget::cancelClicked);
    ASSERT_TRUE(spy.isValid());

    // Act: 触发取消槽
    w->slotCancel();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(w->mp_CancelButton != nullptr);
}

TEST_F(UT_DetectedStatusWidget, SlotFeedBack_Called_DBusStubbedNoCrash)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 提交反馈(走 CommonTools::feedback 的打桩 DBus)
    w->slotFeedBack();

    // Assert: 控件未被破坏
    EXPECT_NE(w->mp_FeedBackLabel, nullptr);
    EXPECT_NE(w->mp_UpdateLabel, nullptr);
}

TEST_F(UT_DetectedStatusWidget, SlotReboot_Called_DBusStubbedNoCrash)
{
    // Arrange: 夹具已构造完成
    ASSERT_NE(w, nullptr);

    // Act: 触发重启槽
    w->slotReboot();

    // Assert: 控件未被破坏
    EXPECT_NE(w->mp_CancelButton, nullptr);
    EXPECT_NE(w->mp_Progress, nullptr);
}

// ===================== onUpdateTheme / paintEvent =====================

TEST_F(UT_DetectedStatusWidget, OnUpdateTheme_Called_PaletteAppliedNoCrash)
{
    // Arrange: 先设置一个界面
    w->setDetectFinishUI("5", "PC", true);
    ASSERT_NE(w->mp_UpdateLabel, nullptr);

    // Act: 主题更新
    w->onUpdateTheme();

    // Assert
    EXPECT_NE(w->mp_UpdateLabel, nullptr);
    EXPECT_NE(w->mp_ModelLabel, nullptr);
    EXPECT_TRUE(w->mp_UpdateLabel->palette().color(QPalette::WindowText).isValid());
}

TEST_F(UT_DetectedStatusWidget, PaintEvent_Rendered_NoCrash)
{
    // Arrange: 渲染目标
    QPixmap pix(300, 200);
    pix.fill(Qt::white);

    // Act: 真实渲染触发 paintEvent(圆角边框 + DFrame 基类绘制)
    w->render(&pix);

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_NE(w->mp_PicLabel, nullptr);
}
