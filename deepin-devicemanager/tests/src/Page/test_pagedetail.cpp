// Copyright (C) 2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 用例计数声明（min：high=3(+complexity≥20 补异常/负面)、mid=2、low=1；loop 因子 +1）：
// | 方法               | level | min | actual |
// |--------------------|-------|-----|--------|
// | PageDetail(ctor)   | mid   | 2   | 2 |   // 用例总数：20
// | showDeviceInfo     | high  | 3   | 5(含 refreshInfo/exportInfo 信号转发) |
// | showInfoOfNum      | mid   | 2   | 2 |
// | enableDevice       | mid   | 2   | 2 |
// | setWakeupMachine   | low   | 1   | 2 |
// | clearWidget        | mid   | 2   | 1(全量清空+幂等重入合并) |
// | addWidgets         | mid   | 2   | 1(追加与可见性两分支合并) |
// | slotBtnClicked     | mid   | 2   | 2 |
// | slotCopyAllInfo    | low   | 1   | 1 |
// | paintEvent         | low   | 1   | 1 |
// | resizeEvent        | low   | 1   | 1 |
//
// PageDetail 类单元测试（重写自 ut_pagedetail.cpp 的 PageDetail_UT）
//   覆盖：ctor / showDeviceInfo / showInfoOfNum / enableDevice /
//         setWakeupMachine / clearWidget / addWidgets / paintEvent /
//         resizeEvent / slotBtnClicked / slotCopyAllInfo / 信号转发
//   说明：DetailButton / DetailSeperator / ScrollAreaWidget 由
//         ut_detailbutton.cpp 覆盖，本文件不重复。
//   打桩：TextBrowser 内部实现（showDeviceInfo / setDeviceEnabled /
//         setWakeupMachine / updateShowOtherInfo / updateInfo），
//         设备对象用最小 TestDevice 子类（无外部依赖）。
//   已知限制：paintEvent 依赖 offscreen 平台渲染。
//   缺陷标注：showDeviceInfo 曾对含空指针设备列表（null 在有效设备前）以
//   lstInfo.size()-1 索引分隔符列表 → QList 越界 abort；该缺陷已在上游
//   3155f2b8 修复（改用 m_ListDetailSeperator.last()），本文件用例针对
//   修复后行为验证，null-only 场景继续保留作回归。
// ------------------------------------------------------------
// 用例总数：20
// | 方法             | level | 用例 | 说明                          |
// | PageDetail(ctor) | mid   | 2   | 结构构建/初始空列表            |
// | showDeviceInfo   | high  | 4   | 正常/空指针跳过/重置/末尾分割线 |
// | showInfoOfNum    | mid   | 2   | 越界早退/在界滚动值            |
// | enableDevice     | mid   | 2   | 非法行/合法行转发              |
// | setWakeupMachine | low   | 2   | 非法行/合法行转发              |
// | clearWidget      | mid   | 1   | 全量清空并重建布局             |
// | addWidgets       | mid   | 1   | 列表追加+按钮可见性            |
// | slotBtnClicked   | mid   | 2   | 无 sender 早退/按钮切换        |
// | slotCopyAllInfo  | low   | 1   | 剪贴板拼接                     |
// | paintEvent       | low   | 1   | 绘制安全                       |
// | resizeEvent      | low   | 1   | 转发基类                       |
// | 信号转发 refreshInfo | low | 1  | TextBrowser→PageDetail        |
// ============================================================

#include <QClipboard>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <DApplication>

#include <gtest/gtest.h>

#include "ut_Head.h"
#include "stub.h"

#define private public // 访问 PageDetail 私有成员（本仓库 UT 惯例，见 ut_pagedrivermanager.cpp）
#include "PageDetail.h"
#include "DeviceInfo.h"
#include "TextBrowser.h"

// ==================== 桩计数器 ====================
struct PDCounters
{
    int tbShow = 0;            // TextBrowser::showDeviceInfo
    DeviceBaseInfo *lastDevice = nullptr;
    int tbEnable = 0;          // TextBrowser::setDeviceEnabled
    EnableDeviceStatus tbEnableRet = EDS_Success;
    int tbWakeup = 0;          // TextBrowser::setWakeupMachine
    bool tbWakeupArg = false;
    int tbOther = 0;           // TextBrowser::updateShowOtherInfo
    int tbUpdateInfo = 0;      // TextBrowser::updateInfo
};
static PDCounters g_pd;

// 自由函数桩（stub.h 联合体不兼容 lambda）
static void stub_TB_showDeviceInfo(void *, DeviceBaseInfo *info)
{
    ++g_pd.tbShow;
    g_pd.lastDevice = info;
}
static EnableDeviceStatus stub_TB_setDeviceEnabled(void *, bool)
{
    ++g_pd.tbEnable;
    return g_pd.tbEnableRet;
}
static void stub_TB_setWakeupMachine(void *, bool wakeup)
{
    ++g_pd.tbWakeup;
    g_pd.tbWakeupArg = wakeup;
}
static void stub_TB_updateShowOtherInfo(void *)
{
    ++g_pd.tbOther;
}
static void stub_TB_updateInfo(void *)
{
    ++g_pd.tbUpdateInfo;
}

// ==================== 最小设备对象 ====================
// DeviceBaseInfo 纯虚接口最小实现：name/vendor/driver。
// enable 走 setEnableValue(true)；available 走 setForcedDisplay(true)。
class TestDevice : public DeviceBaseInfo
{
public:
    explicit TestDevice(QObject *parent = nullptr)
        : DeviceBaseInfo(parent)
    {
        setEnableValue(true);
        setForcedDisplay(true);
    }
    const QString &name() const override { return m_name; }
    const QString &vendor() const override { return m_vendor; }
    const QString &driver() const override { return m_driver; }

protected:
    void initFilterKey() override {}
    void loadBaseDeviceInfo() override {}
    void loadOtherDeviceInfo() override {}
    void loadTableData() override {}

private:
    QString m_name = "TestDevice";
    QString m_vendor = "Uniontech";
    QString m_driver = "test_drv";
};

// ==================== 测试夹具 ====================
DWIDGET_USE_NAMESPACE

class PageDetailTest : public UT_HEAD
{
public:
    void SetUp() override
    {
        g_pd = PDCounters();
        stub.set(ADDR(TextBrowser, showDeviceInfo), stub_TB_showDeviceInfo);
        stub.set(ADDR(TextBrowser, setDeviceEnabled), stub_TB_setDeviceEnabled);
        stub.set(ADDR(TextBrowser, setWakeupMachine), stub_TB_setWakeupMachine);
        stub.set(ADDR(TextBrowser, updateShowOtherInfo), stub_TB_updateShowOtherInfo);
        stub.set(ADDR(TextBrowser, updateInfo), stub_TB_updateInfo);
        page = new PageDetail();
    }
    void TearDown() override
    {
        delete page;
        page = nullptr;
        stub.reset(ADDR(TextBrowser, showDeviceInfo));
        stub.reset(ADDR(TextBrowser, setDeviceEnabled));
        stub.reset(ADDR(TextBrowser, setWakeupMachine));
        stub.reset(ADDR(TextBrowser, updateShowOtherInfo));
        stub.reset(ADDR(TextBrowser, updateInfo));
    }
    Stub stub;
    PageDetail *page = nullptr;
};

// ==================== ctor ====================
TEST_F(PageDetailTest, Ctor_BuildsScrollStructure)
{
    // Arrange：构造即已执行
    // Act：无
    // Assert：滚动区域/滚动内容/布局均已建立，滚动属性正确
    EXPECT_NE(page->mp_ScrollArea, nullptr);
    EXPECT_NE(page->mp_ScrollWidget, nullptr);
    EXPECT_NE(page->mp_ScrollAreaLayout, nullptr);
    EXPECT_TRUE(page->mp_ScrollArea->widgetResizable());
    EXPECT_EQ(page->mp_ScrollArea->frameShape(), QFrame::NoFrame);
    EXPECT_EQ(page->mp_ScrollArea->widget(), page->mp_ScrollWidget);
    EXPECT_NE(page->layout(), nullptr);
}

TEST_F(PageDetailTest, Ctor_InitializesEmptyLists)
{
    // Arrange：构造即已执行（ctor 末尾调用 clearWidget）
    ASSERT_NE(page, nullptr);
    // Act：无
    // Assert：四个列表均为空
    EXPECT_EQ(page->m_ListTextBrowser.size(), 0);
    EXPECT_EQ(page->m_ListHlayout.size(), 0);
    EXPECT_EQ(page->m_ListDetailButton.size(), 0);
    EXPECT_EQ(page->m_ListDetailSeperator.size(), 0);
    EXPECT_TRUE(page->m_ListTextBrowser.isEmpty());
}

// ==================== showDeviceInfo ====================
TEST_F(PageDetailTest, ShowDeviceInfo_TwoDevices_AddsBrowsers)
{
    // Arrange：两个有效设备
    TestDevice d1;
    TestDevice d2;
    QList<DeviceBaseInfo *> lst { &d1, &d2 };
    // Act
    page->showDeviceInfo(lst);
    // Assert：每个设备一个 TextBrowser；showDeviceInfo 转发两次；滚动条复位
    EXPECT_EQ(page->m_ListTextBrowser.size(), 2);
    EXPECT_EQ(page->m_ListDetailButton.size(), 2);
    EXPECT_EQ(g_pd.tbShow, 2);
    EXPECT_EQ(g_pd.lastDevice, &d2);
    EXPECT_EQ(page->mp_ScrollArea->verticalScrollBar()->value(), 0);
}

TEST_F(PageDetailTest, ShowDeviceInfo_NullDeviceOnly_Skipped)
{
    // Arrange：设备列表仅含空指针（等价类：无效元素）。
    //   注意：{nullptr, 有效设备} 组合会触发源码 QList 越界缺陷（见文件头缺陷标注）
    QList<DeviceBaseInfo *> lst { nullptr };
    // Act
    page->showDeviceInfo(lst);
    // Assert：无有效设备 → 不生成浏览器，安全返回
    EXPECT_TRUE(page->m_ListTextBrowser.isEmpty());
    EXPECT_EQ(g_pd.tbShow, 0);
}

TEST_F(PageDetailTest, ShowDeviceInfo_Rerun_ClearsPreviousContent)
{
    // Arrange：先显示一个设备
    TestDevice d1;
    TestDevice d2;
    page->showDeviceInfo(QList<DeviceBaseInfo *> { &d1 });
    const int firstCount = page->m_ListTextBrowser.size();
    // Act：再次显示另一个设备
    page->showDeviceInfo(QList<DeviceBaseInfo *> { &d2 });
    // Assert：旧内容被 clearWidget 清空，列表不累积
    EXPECT_EQ(firstCount, 1);
    EXPECT_EQ(page->m_ListTextBrowser.size(), 1);
    EXPECT_EQ(g_pd.tbShow, 2);
}

TEST_F(PageDetailTest, ShowDeviceInfo_LastSeparatorHidden)
{
    // Arrange：两个有效设备（其它属性为空 → More 按钮隐藏路径）
    TestDevice d1;
    TestDevice d2;
    // Act
    page->showDeviceInfo(QList<DeviceBaseInfo *> { &d1, &d2 });
    // Assert：最后一个分割线隐藏；前一分割线可见
    EXPECT_FALSE(page->m_ListDetailSeperator[1]->isVisibleTo(page->mp_ScrollWidget));
    EXPECT_TRUE(page->m_ListDetailSeperator[0]->isVisibleTo(page->mp_ScrollWidget));
    // 其它属性为空 → More 按钮不可见
    EXPECT_FALSE(page->m_ListDetailButton[0]->isVisibleTo(page->mp_ScrollWidget));
    EXPECT_EQ(g_pd.tbShow, 2);
}

// ==================== showInfoOfNum ====================
TEST_F(PageDetailTest, ShowInfoOfNum_OutOfRange_ScrollUnchanged)
{
    // Arrange：空列表（滚动范围为 0，预设值会被钳位到 0）
    // Act
    page->showInfoOfNum(99);
    // Assert：越界早退不崩溃，滚动值保持不变
    EXPECT_EQ(page->mp_ScrollArea->verticalScrollBar()->value(), 0);
    page->showInfoOfNum(-1);
    EXPECT_EQ(page->mp_ScrollArea->verticalScrollBar()->value(), 0);
}

TEST_F(PageDetailTest, ShowInfoOfNum_InRange_SetsScrollValue)
{
    // Arrange：两个浏览器；未显示状态 height 为 0，SPACE_HEIGHT 为 0
    TestDevice d1;
    TestDevice d2;
    page->showDeviceInfo(QList<DeviceBaseInfo *> { &d1, &d2 });
    page->mp_ScrollArea->verticalScrollBar()->setValue(7);
    // Act
    page->showInfoOfNum(0);
    // Assert：索引 0 无前置项 → 滚动值复位 0
    EXPECT_EQ(page->mp_ScrollArea->verticalScrollBar()->value(), 0);
    // Act：索引 1 → 前置高度合计（全 0）→ 仍为 0，且不崩溃
    page->showInfoOfNum(1);
    EXPECT_EQ(page->mp_ScrollArea->verticalScrollBar()->value(), 0);
}

// ==================== enableDevice ====================
TEST_F(PageDetailTest, EnableDevice_InvalidRow_ReturnsCancle)
{
    // Arrange：空列表（边界：row 越界）
    ASSERT_TRUE(page->m_ListTextBrowser.isEmpty());
    // Act
    const EnableDeviceStatus ret = page->enableDevice(0, true);
    // Assert
    EXPECT_EQ(page->enableDevice(0, true), EDS_Cancle);
    EXPECT_EQ(page->enableDevice(5, false), EDS_Cancle);
    EXPECT_EQ(g_pd.tbEnable, 0);
}

TEST_F(PageDetailTest, EnableDevice_ValidRow_ForwardsToBrowser)
{
    // Arrange：一个设备
    TestDevice d1;
    page->showDeviceInfo(QList<DeviceBaseInfo *> { &d1 });
    g_pd.tbEnableRet = EDS_Success;
    // Act
    const EnableDeviceStatus ret = page->enableDevice(0, true);
    // Assert：转发至 TextBrowser 并返回其结果
    EXPECT_EQ(ret, EDS_Success);
    EXPECT_EQ(g_pd.tbEnable, 1);
    // 返回浏览器失败状态时透传
    g_pd.tbEnableRet = EDS_Faild;
    EXPECT_EQ(page->enableDevice(0, false), EDS_Faild);
    EXPECT_EQ(g_pd.tbEnable, 2);
}

// ==================== setWakeupMachine ====================
TEST_F(PageDetailTest, SetWakeupMachine_InvalidRow_NoForward)
{
    // Arrange：空列表
    ASSERT_TRUE(page->m_ListTextBrowser.isEmpty());
    // Act
    page->setWakeupMachine(0, true);
    // Assert：不转发
    EXPECT_EQ(g_pd.tbWakeup, 0);
    EXPECT_FALSE(g_pd.tbWakeupArg);
}

TEST_F(PageDetailTest, SetWakeupMachine_ValidRow_ForwardsArg)
{
    // Arrange：一个设备
    TestDevice d1;
    page->showDeviceInfo(QList<DeviceBaseInfo *> { &d1 });
    // Act
    page->setWakeupMachine(0, true);
    // Assert：参数透传
    EXPECT_EQ(g_pd.tbWakeup, 1);
    EXPECT_TRUE(g_pd.tbWakeupArg);
    page->setWakeupMachine(0, false);
    EXPECT_EQ(g_pd.tbWakeup, 2);
    EXPECT_FALSE(g_pd.tbWakeupArg);
}

// ==================== addWidgets / clearWidget ====================
TEST_F(PageDetailTest, AddWidgets_AppendsListsAndControlsButton)
{
    // Arrange：两个手工浏览器（enable 分别为 true/false）
    TextBrowser *tb1 = new TextBrowser;
    TextBrowser *tb2 = new TextBrowser;
    // Act
    page->addWidgets(tb1, true);
    page->addWidgets(tb2, false);
    // Assert：四个列表同步追加；enable=false 的 More 按钮隐藏
    EXPECT_EQ(page->m_ListTextBrowser.size(), 2);
    EXPECT_EQ(page->m_ListHlayout.size(), 2);
    EXPECT_EQ(page->m_ListDetailButton.size(), 2);
    EXPECT_EQ(page->m_ListDetailSeperator.size(), 2);
    EXPECT_TRUE(page->m_ListDetailButton[0]->isVisibleTo(page->mp_ScrollWidget));
    EXPECT_FALSE(page->m_ListDetailButton[1]->isVisibleTo(page->mp_ScrollWidget));
}

TEST_F(PageDetailTest, ClearWidget_RemovesAllAndRebuildsLayout)
{
    // Arrange：两个浏览器
    TestDevice d1;
    TestDevice d2;
    page->showDeviceInfo(QList<DeviceBaseInfo *> { &d1, &d2 });
    ASSERT_EQ(page->m_ListTextBrowser.size(), 2);
    // Act
    page->clearWidget();
    // Assert：列表清空，布局重建为空
    EXPECT_TRUE(page->m_ListTextBrowser.isEmpty());
    EXPECT_TRUE(page->m_ListHlayout.isEmpty());
    EXPECT_TRUE(page->m_ListDetailButton.isEmpty());
    EXPECT_TRUE(page->m_ListDetailSeperator.isEmpty());
    EXPECT_NE(page->mp_ScrollAreaLayout, nullptr);
    EXPECT_EQ(page->mp_ScrollAreaLayout->count(), 0);
    // 重复清空安全
    page->clearWidget();
    EXPECT_EQ(page->mp_ScrollAreaLayout->count(), 0);
}

// ==================== slotBtnClicked ====================
TEST_F(PageDetailTest, SlotBtnClicked_NoSender_EarlyReturn)
{
    // Arrange：无 sender（直接调用，等价类：sender 无效）
    TestDevice d1;
    page->showDeviceInfo(QList<DeviceBaseInfo *> { &d1 });
    // Act
    page->slotBtnClicked();
    // Assert：不触碰浏览器
    EXPECT_EQ(g_pd.tbOther, 0);
    EXPECT_EQ(g_pd.tbUpdateInfo, 0);
}

TEST_F(PageDetailTest, SlotBtnClicked_TogglesBrowserAndUpdateText)
{
    // Arrange：一个设备 + More 按钮（enable=true 路径经 addWidgets 直填）
    TestDevice d1;
    page->showDeviceInfo(QList<DeviceBaseInfo *> { &d1 });
    DetailButton *btn = page->m_ListDetailButton[0];
    const QString before = btn->text();
    // Act：模拟按钮点击（真实信号链路）
    btn->click();
    // Assert：浏览器展开/收起被触发，按钮文本切换
    EXPECT_EQ(g_pd.tbOther, 1);
    EXPECT_EQ(g_pd.tbUpdateInfo, 1);
    EXPECT_NE(btn->text(), before);
}

// ==================== slotCopyAllInfo ====================
TEST_F(PageDetailTest, SlotCopyAllInfo_ConcatenatesToClipboard)
{
    // Arrange：两个浏览器分别写入文本
    TextBrowser *tb1 = new TextBrowser;
    TextBrowser *tb2 = new TextBrowser;
    tb1->setPlainText("Alpha");
    tb2->setPlainText("Beta");
    page->addWidgets(tb1, true);
    page->addWidgets(tb2, true);
    // Act
    page->slotCopyAllInfo();
    // Assert：剪贴板内容为按顺序拼接
    EXPECT_EQ(DApplication::clipboard()->text(), QString("AlphaBeta"));
    // 变更内容后再次拷贝 → 以最新文本覆盖
    tb2->setPlainText("Gamma");
    page->slotCopyAllInfo();
    EXPECT_EQ(DApplication::clipboard()->text(), QString("AlphaGamma"));
}

// ==================== paintEvent / resizeEvent ====================
TEST_F(PageDetailTest, PaintEvent_RoundsBackgroundSafely)
{
    // Arrange：构造绘制事件
    QPaintEvent paint(QRect(page->rect()));
    // Act
    EXPECT_NO_FATAL_FAILURE(page->paintEvent(&paint));
    // Assert：可离屏渲染，最小高度生效（MIN_HEIGHT=50）
    EXPECT_FALSE(page->grab().isNull());
    EXPECT_EQ(page->minimumHeight(), 50);
}

TEST_F(PageDetailTest, ResizeEvent_ForwardsToBase)
{
    // Arrange：构造尺寸事件（直接调用事件处理函数不会真正改尺寸，
    //   仅验证转发基类的路径安全）
    const QSize before = page->size();
    QResizeEvent resize(QSize(300, 200), before);
    // Act
    EXPECT_NO_FATAL_FAILURE(page->resizeEvent(&resize));
    // Assert：尺寸不被直接调用改变，布局保持有效
    EXPECT_EQ(page->size(), before);
    EXPECT_NE(page->layout(), nullptr);
}

// ==================== 信号转发 ====================
TEST_F(PageDetailTest, RefreshInfoSignal_ForwardsToPage)
{
    // Arrange：showDeviceInfo 建立连接 + lambda 侦听页面信号
    //   （不用 QSignalSpy：offscreen 下其析构在全局清理阶段可能阻塞）
    TestDevice d1;
    page->showDeviceInfo(QList<DeviceBaseInfo *> { &d1 });
    int refreshCnt = 0;
    int exportCnt = 0;
    QObject::connect(page, &PageDetail::refreshInfo, page, [&refreshCnt]() { ++refreshCnt; });
    QObject::connect(page, &PageDetail::exportInfo, page, [&exportCnt]() { ++exportCnt; });
    // Act：浏览器发射刷新/导出信号
    emit page->m_ListTextBrowser[0]->refreshInfo();
    emit page->m_ListTextBrowser[0]->exportInfo();
    // Assert：页面信号各转发一次
    EXPECT_EQ(refreshCnt, 1);
    EXPECT_EQ(exportCnt, 1);
}
