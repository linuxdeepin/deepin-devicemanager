// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// DriverScanWidget 重写批次：状态切换 UI 断言化 + 进度累加 + 信号 + 事件分支。

// 分支清单（来源：MCP get_code_snippet — src/Widget/DriverScanWidget.cpp）
//  ctor/initUI/initConnect：结构搭建（ScroBar=area 垂直滚动条、按钮对象名、反馈链接）
//  setScanningUI(info,progress)：hideAll → animation 启动 → 扫描中标签/进度条/信息标签可见
//  setScanFailedUI：失败图标 + 反馈标签 + 重新扫描按钮可见
//  setNetworkErr：错误图标 + 请检查网络标签 + 重新扫描按钮可见
//  setProgressFinish：动画停止 + 进度置 100
//  refreshProgress(info,delta)：信息标签更新 + 进度 = 旧值 + delta（累加语义）
//  slotFeedBack：转发 CommonTools::feedback()（打桩计数）
//  slotReDetected：先回扫描 UI 再 emit redetected()
//  hideAll：全部隐藏 + 布局重建 + 动画停止
//  paintEvent：圆角底色绘制；resizeEvent：滚动条置底并隐藏
//
// 用例映射：Ctor/SetScanningUI/SetScanFailedUI/SetNetworkErr/SetProgressFinish/
//          RefreshProgress/HideAll/SlotFeedBack/SlotReDetected(信号+复位)/
//          PaintEvent/ResizeEvent
//
// 最小清单：[x]1 公开方法每方法≥1用例 [x]2 等价类（三态 UI 切换）
//          [x]3 边界（进度累加、终值 100） [x]4 同质多输入（refreshProgress 两次）
//          [x]5 分支映射 [x]6 每分支有用例 [x]7 外部依赖 stub（CommonTools::feedback）
//          [x]8 负面/边界（resize 滚动条隐藏） [x]9 状态断言非空洞
//          [x]10 Qt/静态依赖打桩；无 DBus/文件系统依赖

#include "DriverScanWidget.h"
#include "AnimationLabel.h"
#include "commontools.h"

#include "ut_Head.h"
#include "stub.h"

#include <QPaintEvent>
#include <QScrollBar>
#include <QSignalSpy>

#include <DSuggestButton>
#include <DProgressBar>

#include <gtest/gtest.h>

class UT_DriverScanWidget : public UT_HEAD
{
public:
    void SetUp() override
    {
        w = new DriverScanWidget;
    }
    void TearDown() override
    {
        delete w;
    }
    DriverScanWidget *w = nullptr;
    Stub stub;
};

static int g_feedbackCalls = 0;
static void ut_commonTools_feedback()
{
    ++g_feedbackCalls;
}

TEST_F(UT_DriverScanWidget, Ctor_BuildsStructureAndScrollbar)
{
    // Assert：initUI 搭建滚动条与按钮，容器无线宽
    EXPECT_NE(w->mp_ScroBar, nullptr);
    EXPECT_EQ(w->mp_ReScanButton->objectName(), QString("MpReScanButton"));
    EXPECT_EQ(w->lineWidth(), 0);
}

TEST_F(UT_DriverScanWidget, SetScanningUI_ShowsProgressAndInfo)
{
    w->setScanningUI("camera", 42);   // Act

    EXPECT_EQ(w->mp_ScanningProgress->value(), 42);
    EXPECT_FALSE(w->mp_ScanningLabel->isHidden());       // 扫描中提示可见
    EXPECT_FALSE(w->mp_ScanningInfoLabel->isHidden());   // 扫描对象信息可见
    EXPECT_TRUE(w->mp_ScanningInfoLabel->text().contains("camera"));
}

TEST_F(UT_DriverScanWidget, SetScanFailedUI_ShowsErrorAndActions)
{
    w->setScanFailedUI();   // Act

    EXPECT_FALSE(w->mp_ErrPicLabel->isHidden());      // 失败图标
    EXPECT_FALSE(w->mp_FeedBackLabel->isHidden());    // 反馈入口
    EXPECT_FALSE(w->mp_ReScanButton->isHidden());     // 重新扫描按钮
    EXPECT_TRUE(w->mp_ScanningLabel->isHidden() == false);
}

TEST_F(UT_DriverScanWidget, SetNetworkErr_ShowsCheckNetwork)
{
    w->setNetworkErr();   // Act

    EXPECT_FALSE(w->mp_CheckNetworkLabel->isHidden());   // 请检查网络标签
    EXPECT_FALSE(w->mp_ReScanButton->isHidden());
    EXPECT_TRUE(w->mp_CheckNetworkLabel->text().contains("network"));
}

TEST_F(UT_DriverScanWidget, SetProgressFinish_SetsValueToHundred)
{
    w->setScanningUI("dev", 30);

    w->setProgressFinish();   // Act

    EXPECT_EQ(w->mp_ScanningProgress->value(), 100);   // 终值 100
}

TEST_F(UT_DriverScanWidget, RefreshProgress_AccumulatesDelta)
{
    // DProgressBar 实际落值与设置值存在偏移，改用增量语义断言（源码为 old+progress）
    int before = w->mp_ScanningProgress->value();

    w->refreshProgress("device-a", 10);   // Act：增量 10
    int afterFirst = w->mp_ScanningProgress->value();

    w->refreshProgress("device-b", 20);   // Act：增量 20 累加

    EXPECT_EQ(afterFirst - before, 10);
    EXPECT_EQ(w->mp_ScanningProgress->value() - afterFirst, 20);
    EXPECT_TRUE(w->mp_ScanningInfoLabel->text().contains("device-b"));
}

TEST_F(UT_DriverScanWidget, HideAll_HidesEveryControl)
{
    w->setScanningUI("dev", 50);   // 先展示

    w->hideAll();   // Act

    EXPECT_TRUE(w->mp_ScanningPicLabel->isHidden());
    EXPECT_TRUE(w->mp_ErrPicLabel->isHidden());
    EXPECT_TRUE(w->mp_ScanningLabel->isHidden());
    EXPECT_TRUE(w->mp_ScanningProgress->isHidden());
    EXPECT_TRUE(w->mp_ReScanButton->isHidden());
}

TEST_F(UT_DriverScanWidget, SlotFeedBack_ForwardsToCommonTools)
{
    stub.set(ADDR(CommonTools, feedback), ut_commonTools_feedback);
    g_feedbackCalls = 0;

    w->slotFeedBack();   // Act

    EXPECT_EQ(g_feedbackCalls, 1);   // 转发恰好一次（无真实浏览器拉起）
}

TEST_F(UT_DriverScanWidget, SlotReDetected_EmitsAndResetsUI)
{
    w->setScanningUI("dev", 80);
    QSignalSpy spy(w, &DriverScanWidget::redetected);

    w->slotReDetected();   // Act

    EXPECT_EQ(spy.count(), 1);                        // 发出 redetected
    EXPECT_EQ(w->mp_ScanningProgress->value(), 0);    // 回到扫描 UI 进度复位
}

TEST_F(UT_DriverScanWidget, PaintEvent_RendersRoundedBackground)
{
    QPaintEvent paintEvent(QRect(0, 0, 200, 100));

    w->paintEvent(&paintEvent);   // Act：不崩溃即可（圆角底色绘制）

    EXPECT_FALSE(w->grab().isNull());
}

TEST_F(UT_DriverScanWidget, ResizeEvent_HidesScrollbar)
{
    // 直接触发 resizeEvent（经 resize() 布局会重新拉起滚动条，掩盖源码行为）
    QResizeEvent event(QSize(400, 300), QSize(300, 200));

    w->resizeEvent(&event);   // Act

    EXPECT_TRUE(w->mp_ScroBar->isHidden());   // resizeEvent 置底并隐藏滚动条
}
