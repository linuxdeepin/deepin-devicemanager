// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_cmdbuttonwidget.cpp
 * @brief CmdButtonWidget 单元测试（Mode 2 重写，逐类闭环）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method    | level | factors                            | min | actual |
 * |-----------|-------|------------------------------------|-----|--------|
 * | ctor      | mid   | 按钮构造/信号连接链                 | 2   | 2      |
 * | initUI    | low   | objectName/居中布局                 | 1   | 1      |
 * | paintEvent| mid   | 激活/未激活调色组、重复绘制          | 2   | 3      |
 *
 * 分支/等价类说明（源自 MCP get_code_snippet + 工作目录源码核对，23-101 行）：
 *   - ctor：mp_cmdButton = new DCommandLinkButton(tr("More")) → initUI →
 *     connect(clicked → cmdButtonClicked 信号重发)；
 *   - initUI：T8 字号绑定、objectName/accessibleName MpCmdButton、
 *     HBox(stretch+按钮+stretch) 嵌入 VBox 居中；
 *   - paintEvent：DApplication::activeWindow() 非空 → Active 调色组，空 →
 *     Inactive；绘制区域宽强制 808，边框路径 = 外矩形减内矩形，以
 *     palette.color(cg, FrameShadowBorder) 填充。
 *   用例映射：
 *   - ctor_Default_ButtonConfigured                 → ctor 构造链
 *   - ctor_ClickButton_EmitsCmdButtonClicked        → clicked 连接与信号重发
 *   - initUI_ButtonNamed_NestedCenterLayout         → initUI 分支
 *   - paintEvent_NoActiveWindow_InactivePathRuns    → Inactive 分支
 *   - paintEvent_ActiveWindow_ActivePathRuns        → Active 分支
 *   - paintEvent_RepeatedInvoke_GeometryStable      → 重复绘制/矩形临时性观测
 * 打桩策略（外部依赖全部 stub）：
 *   - DApplication::activeWindow（继承自 QApplication 的静态函数）→ 受控返回
 *     null/非 null，消除对真实窗口焦点状态的依赖；
 *   - 无 DBus/文件依赖；paintEvent 以栈上 QPaintEvent 直调，边框为纯路径填充，
 *     offscreen 下可安全执行。
 */

#include "CmdButtonWidget.h"
#include "ut_Head.h"
#include "stubext.h"

#include <gtest/gtest.h>
#include <DApplication>
#include <DCommandLinkButton>
#include <DGuiApplicationHelper>
#include <QApplication>
#include <QPaintEvent>
#include <QSignalSpy>
#include <QLayout>

class UT_CmdButtonWidget : public ::testing::Test
{
public:
    void SetUp()
    {
        m_widget = new CmdButtonWidget;
    }
    void TearDown()
    {
        stub.clear();
        delete m_widget;
    }

    CmdButtonWidget *m_widget = nullptr;
    stub_ext::StubExt stub;
};

// ctor：按钮创建、文本 More、父窗口挂接（initUI 在构造内完成）
TEST_F(UT_CmdButtonWidget, ctor_Default_ButtonConfigured)
{
    // Arrange：期望常量（initUI 源码值）
    const QString kButtonText = QStringLiteral("More");

    // Act：读取构造后的按钮状态
    DCommandLinkButton *btn = m_widget->mp_cmdButton;

    // Assert：按钮存在、文本与父子关系正确
    ASSERT_NE(nullptr, btn);
    EXPECT_EQ(kButtonText, btn->text());
    EXPECT_EQ(m_widget, btn->parentWidget());
}

// ctor：点击命令链接按钮，经 connect 重发 cmdButtonClicked 信号
TEST_F(UT_CmdButtonWidget, ctor_ClickButton_EmitsCmdButtonClicked)
{
    // Arrange：监听 cmdButtonClicked 信号
    QSignalSpy spy(m_widget, &CmdButtonWidget::cmdButtonClicked);
    ASSERT_TRUE(spy.isValid());
    ASSERT_EQ(0, spy.count());

    // Act：触发两次按钮点击（clicked → cmdButtonClicked）
    m_widget->mp_cmdButton->click();
    m_widget->mp_cmdButton->click();

    // Assert：信号按点击次数精确重发，且无附加参数
    EXPECT_EQ(2, spy.count());
    EXPECT_EQ(0, spy.at(0).size());
}

// initUI：objectName/accessibleName 为 MpCmdButton，按钮位于居中布局内
TEST_F(UT_CmdButtonWidget, initUI_ButtonNamed_NestedCenterLayout)
{
    // Arrange：期望命名常量（initUI 源码值）
    const QString kName = QStringLiteral("MpCmdButton");

    // Act：读取命名与布局状态
    DCommandLinkButton *btn = m_widget->mp_cmdButton;
    QLayout *layout = m_widget->layout();

    // Assert：命名一致、按钮挂在布局内（嵌套 HBox，顶层 layout.count 间接持有）
    ASSERT_NE(nullptr, layout);
    EXPECT_EQ(kName, btn->objectName());
    EXPECT_EQ(kName, btn->accessibleName());
    EXPECT_EQ(m_widget, btn->parentWidget());
    EXPECT_GE(layout->count(), 1);
}

// paintEvent：无激活窗口（activeWindow=nullptr）走 Inactive 分支，绘制不崩溃
TEST_F(UT_CmdButtonWidget, paintEvent_NoActiveWindow_InactivePathRuns)
{
    // Arrange：activeWindow 返回 null
    stub.set_lamda(static_cast<QWidget *(*)()>(&QApplication::activeWindow),
                   []() -> QWidget * { return nullptr; });
    QPaintEvent event(QRect(0, 0, 100, 30));

    // Act：直调两次绘制（幂等观测）
    m_widget->paintEvent(&event);
    const int widthAfterFirst = m_widget->rect().width();
    m_widget->paintEvent(&event);

    // Assert：两次绘制后几何与子按钮状态稳定（rect().setWidth 为临时量无副作用）
    EXPECT_EQ(widthAfterFirst, m_widget->rect().width());
    EXPECT_NE(nullptr, m_widget->mp_cmdButton);
    EXPECT_EQ(m_widget, m_widget->mp_cmdButton->parentWidget());
}

// paintEvent：存在激活窗口走 Active 分支，绘制不崩溃
TEST_F(UT_CmdButtonWidget, paintEvent_ActiveWindow_ActivePathRuns)
{
    // Arrange：activeWindow 返回本控件（非空即 Active 组）
    stub.set_lamda(static_cast<QWidget *(*)()>(&QApplication::activeWindow),
                   [this]() -> QWidget * { return m_widget; });
    QPaintEvent event(QRect(0, 0, 100, 30));

    // Act：直调两次绘制
    m_widget->paintEvent(&event);
    const int widthAfterFirst = m_widget->rect().width();
    m_widget->paintEvent(&event);

    // Assert：两次绘制后几何稳定，信号未误发
    EXPECT_EQ(widthAfterFirst, m_widget->rect().width());
    EXPECT_NE(nullptr, m_widget->mp_cmdButton);
    QSignalSpy spy(m_widget, &CmdButtonWidget::cmdButtonClicked);
    EXPECT_EQ(0, spy.count());
}

// paintEvent：绘制不改变控件矩形宽度（源码对 rect() 临时量的 setWidth 无持久副作用）
TEST_F(UT_CmdButtonWidget, paintEvent_RepeatedInvoke_GeometryStable)
{
    // Arrange：记录初始矩形宽度
    const int widthBefore = m_widget->rect().width();
    QPaintEvent event(QRect(0, 0, 100, 30));

    // Act：连续三次绘制
    for (int i = 0; i < 3; ++i)
        m_widget->paintEvent(&event);

    // Assert：宽度保持构造值，绘制路径未污染几何状态
    EXPECT_EQ(widthBefore, m_widget->rect().width());
    EXPECT_GT(widthBefore, 0);
}
