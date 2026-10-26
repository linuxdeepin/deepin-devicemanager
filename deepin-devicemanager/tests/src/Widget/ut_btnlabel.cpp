// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_btnlabel.cpp
 * @brief BtnLabel / TipsLabel / TitleLabel 单元测试（Mode 2 重写，逐类闭环）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method                     | level | factors             | min | actual |
 * |----------------------------|-------|---------------------|-----|--------|
 * | BtnLabel.ctor              | low   | default             | 1   | 1      |
 * | BtnLabel.linkActivated     | high  | desc:empty/backup/other, exec:ok/feedback | 3 | 3 |
 * | BtnLabel.setDesc           | low   | set-flag            | 1   | 1      |
 * | BtnLabel.paintEvent        | low   | app-state           | 1   | 2      |
 * | TipsLabel.ctor             | low   | default             | 1   | 1      |
 * | TipsLabel.paintEvent       | low   | app-state           | 1   | 2      |
 * | TitleLabel.ctor            | low   | default             | 1   | 1      |
 * | TitleLabel.paintEvent      | low   | app-state           | 1   | 2      |
 *
 * 分支/等价类说明（源自 MCP get_code_snippet 真实源码，BtnLabel.cpp 18-147 行）：
 *   - linkActivated（构造函数连接的 lambda，linkActivated 信号驱动）：
 *     B1: m_Desc 为空 → 直接 return，不弹任何对话框；
 *     B2: m_Desc == "Backup Failed" → 弹备份失败对话框（OK + Feedback 两按钮）；
 *     B3: B2 中 dialog.exec() 返回 1（点击 Feedback）→ CommonTools::feedback()；
 *     B4: 其他描述 → 通用警告对话框（title=m_Desc，仅 OK 按钮）；
 *   - paintEvent（BtnLabel/TipsLabel/TitleLabel 同构，仅取色角色不同）：
 *     B5: Qt::ApplicationActive == applicationState() → Active : Inactive，
 *         Text 色被改为 color(cg, TextTips)（TitleLabel 为 TextTitle）。
 *   用例映射：
 *   - linkActivated_EmptyDesc_NoDialogShown            → B1
 *   - linkActivated_BackupFailed_OkButtonNoFeedback    → B2 + B3(else)
 *   - linkActivated_BackupFailed_FeedbackButtonCallsFeedback → B3(then)
 *   - linkActivated_OtherDesc_GenericDialogShown       → B4
 *   - paintEvent_{Inactive,Active}State_*              → B5 两侧
 * 打桩策略（外部依赖全部 stub，测试不弹真实模态框、不打开浏览器）：
 *   - DDialog::exec（虚函数重写）→ 经 ut_real_fn_addr 还原真实代码地址后
 *     Stub::set 打桩（stub-ext 无法直接 patch GCC 虚函数指针的 vtable 槽位编码），
 *     返回受控值并计数，避免模态事件循环；
 *   - CommonTools::feedback（静态成员）→ 计数器，不触发真实浏览器/邮件客户端；
 *   - QGuiApplication::applicationState（静态继承成员）→ 控制 Active/Inactive
 *     取色分支，不依赖真实窗口焦点状态；
 *   - DPaletteHelper 为 DTK 进程内服务，offscreen 下正常工作，调色断言镜像源码
 *     取色公式（paintEvent 前后对比），主题无关。
 */

#include "BtnLabel.h"
#include "commontools.h"
#include "ut_Head.h"
#include "stubext.h"
#include "ut_stub_helper.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <DDialog>
#include <QPaintEvent>
#include <DPalette>
#include <DPaletteHelper>

// ---- stub 观测点：对话框弹出与反馈调用 ----
static int g_utExecCount = 0;
static int g_utExecResult = 0;
static int g_utFeedbackCount = 0;
static int ut_ddialog_exec(DDialog *self)
{
    Q_UNUSED(self);
    ++g_utExecCount;
    return g_utExecResult;
}

class UT_BtnLabel : public ::testing::Test
{
public:
    void SetUp()
    {
        g_utExecCount = 0;
        g_utExecResult = 0;
        g_utFeedbackCount = 0;
        m_widget = new BtnLabel;
    }
    void TearDown()
    {
        stub.clear();
        delete m_widget;
    }

    void triggerLink()
    {
        // Act 辅助：触发 linkActivated 信号（Qt6 信号为公开成员，可直接发射）
        emit m_widget->linkActivated(QString());
    }

    BtnLabel *m_widget = nullptr;
    stub_ext::StubExt stub;
    DDialog m_probeDialog;   // 供 ut_real_fn_addr 解析 DDialog::exec 虚函数地址
};

// 构造函数：默认描述为空，仅完成信号连接（B1 前置状态）
TEST_F(UT_BtnLabel, ctor_DefaultState_DescEmpty)
{
    // Arrange：fixture 已构造 BtnLabel
    ASSERT_NE(nullptr, m_widget);

    // Act：读取构造后的默认状态
    const QString desc = m_widget->m_Desc;

    // Assert：描述为空（此后 linkActivated 走 B1 早退分支），标签无文本
    EXPECT_EQ(QString(), desc);
    EXPECT_TRUE(m_widget->text().isEmpty());
}

// setDesc：描述写入私有状态，空串可清空
TEST_F(UT_BtnLabel, setDesc_NewText_DescriptionStored)
{
    // Arrange：默认描述为空
    ASSERT_TRUE(m_widget->m_Desc.isEmpty());

    // Act：写入新描述
    m_widget->setDesc(QStringLiteral("Driver Error"));

    // Assert：描述已存储
    EXPECT_EQ(QStringLiteral("Driver Error"), m_widget->m_Desc);

    // Act：再置空
    m_widget->setDesc(QString());

    // Assert：描述恢复为空
    EXPECT_TRUE(m_widget->m_Desc.isEmpty());
}

// linkActivated 空描述：早退分支，不弹任何对话框（B1）
TEST_F(UT_BtnLabel, linkActivated_EmptyDesc_NoDialogShown)
{
    // Arrange：描述为空，exec 打桩计数
    stub.set(ut_real_fn_addr(&m_probeDialog, static_cast<int (DDialog::*)()>(&DDialog::exec)),
             ut_ddialog_exec);

    // Act
    triggerLink();

    // Assert：未弹出对话框
    EXPECT_EQ(0, g_utExecCount);
    EXPECT_EQ(0, g_utFeedbackCount);
}

// linkActivated 备份失败 + exec 返回 0：弹对话框但点击 OK，不触发反馈（B2 + B3 else）
TEST_F(UT_BtnLabel, linkActivated_BackupFailed_OkButtonNoFeedback)
{
    // Arrange：描述置为备份失败；对话框打桩返回 0（非 Feedback 按钮）
    m_widget->setDesc(QStringLiteral("Backup Failed"));
    g_utExecResult = 0;
    stub.set(ut_real_fn_addr(&m_probeDialog, static_cast<int (DDialog::*)()>(&DDialog::exec)),
             ut_ddialog_exec);
    stub.set_lamda(&CommonTools::feedback, []() { ++g_utFeedbackCount; });

    // Act
    triggerLink();

    // Assert：弹了 1 次对话框，未触发反馈
    EXPECT_EQ(1, g_utExecCount);
    EXPECT_EQ(0, g_utFeedbackCount);
}

// linkActivated 备份失败 + exec 返回 1：点击 Feedback 触发 CommonTools::feedback（B3 then）
TEST_F(UT_BtnLabel, linkActivated_BackupFailed_FeedbackButtonCallsFeedback)
{
    // Arrange：描述置为备份失败；对话框打桩返回 1（Feedback 按钮）
    m_widget->setDesc(QStringLiteral("Backup Failed"));
    g_utExecResult = 1;
    stub.set(ut_real_fn_addr(&m_probeDialog, static_cast<int (DDialog::*)()>(&DDialog::exec)),
             ut_ddialog_exec);
    stub.set_lamda(&CommonTools::feedback, []() { ++g_utFeedbackCount; });

    // Act
    triggerLink();

    // Assert：弹 1 次对话框且反馈恰好触发 1 次
    EXPECT_EQ(1, g_utExecCount);
    EXPECT_EQ(1, g_utFeedbackCount);
}

// linkActivated 其他描述：走通用警告对话框分支，不触发反馈（B4）
TEST_F(UT_BtnLabel, linkActivated_OtherDesc_GenericDialogShown)
{
    // Arrange：描述为任意非空、非备份失败文本
    m_widget->setDesc(QStringLiteral("Driver Error"));
    g_utExecResult = 0;
    stub.set(ut_real_fn_addr(&m_probeDialog, static_cast<int (DDialog::*)()>(&DDialog::exec)),
             ut_ddialog_exec);
    stub.set_lamda(&CommonTools::feedback, []() { ++g_utFeedbackCount; });

    // Act
    triggerLink();

    // Assert：弹 1 次通用对话框，反馈不触发（B4 无 Feedback 按钮）
    EXPECT_EQ(1, g_utExecCount);
    EXPECT_EQ(0, g_utFeedbackCount);
}

// paintEvent 非激活态：Text 色改为 Inactive 的 TextTips 色（B5 Inactive 侧）
TEST_F(UT_BtnLabel, paintEvent_InactiveState_TextTipsPaletteApplied)
{
    // Arrange：应用状态打桩为非激活
    stub.set_lamda(static_cast<Qt::ApplicationState (*)()>(&QGuiApplication::applicationState),
                   []() { return Qt::ApplicationInactive; });
    const auto paletteBefore = DPaletteHelper::instance()->palette(m_widget);
    QPaintEvent event(m_widget->rect());

    // Act
    m_widget->paintEvent(&event);

    // Assert：绘制后 Text 色等于源码公式的目标色（Inactive + TextTips）
    const auto paletteAfter = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteAfter.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Inactive, Dtk::Gui::DPalette::TextTips));

    // Act：再次绘制（幂等性）
    m_widget->paintEvent(&event);

    // Assert：重复绘制后取色公式结果保持一致
    const auto paletteTwice = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteTwice.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Inactive, Dtk::Gui::DPalette::TextTips));
}

// paintEvent 激活态：Text 色改为 Active 的 TextTips 色（B5 Active 侧）
TEST_F(UT_BtnLabel, paintEvent_ActiveState_TextTipsPaletteApplied)
{
    // Arrange：应用状态打桩为激活
    stub.set_lamda(static_cast<Qt::ApplicationState (*)()>(&QGuiApplication::applicationState),
                   []() { return Qt::ApplicationActive; });
    const auto paletteBefore = DPaletteHelper::instance()->palette(m_widget);
    QPaintEvent event(m_widget->rect());

    // Act
    m_widget->paintEvent(&event);

    // Assert：绘制后 Text 色等于源码公式的目标色（Active + TextTips）
    const auto paletteAfter = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteAfter.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Active, Dtk::Gui::DPalette::TextTips));

    // Act：再次绘制（幂等性）
    m_widget->paintEvent(&event);

    // Assert：重复绘制后取色公式结果保持一致
    const auto paletteTwice = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteTwice.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Active, Dtk::Gui::DPalette::TextTips));
}

class UT_TipsLabel : public ::testing::Test
{
public:
    void SetUp()
    {
        m_widget = new TipsLabel;
    }
    void TearDown()
    {
        stub.clear();
        delete m_widget;
    }

    TipsLabel *m_widget = nullptr;
    stub_ext::StubExt stub;
};

// TipsLabel 构造函数：默认无文本，仅继承 DLabel 行为
TEST_F(UT_TipsLabel, ctor_DefaultState_TextEmpty)
{
    // Arrange：fixture 已构造 TipsLabel
    ASSERT_NE(nullptr, m_widget);

    // Act：读取构造后的默认状态
    const QString text = m_widget->text();

    // Assert：默认无文本且无父对象
    EXPECT_TRUE(text.isEmpty());
    EXPECT_EQ(nullptr, m_widget->parentWidget());
}

// TipsLabel paintEvent 非激活态：Text 色改为 Inactive 的 TextTips 色（B5 Inactive 侧）
TEST_F(UT_TipsLabel, paintEvent_InactiveState_TextTipsPaletteApplied)
{
    // Arrange：应用状态打桩为非激活
    stub.set_lamda(static_cast<Qt::ApplicationState (*)()>(&QGuiApplication::applicationState),
                   []() { return Qt::ApplicationInactive; });
    const auto paletteBefore = DPaletteHelper::instance()->palette(m_widget);
    QPaintEvent event(m_widget->rect());

    // Act
    m_widget->paintEvent(&event);

    // Assert：绘制后 Text 色等于源码公式的目标色（Inactive + TextTips）
    const auto paletteAfter = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteAfter.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Inactive, Dtk::Gui::DPalette::TextTips));

    // Act：再次绘制（幂等性）
    m_widget->paintEvent(&event);

    // Assert：重复绘制后取色公式结果保持一致
    const auto paletteTwice = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteTwice.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Inactive, Dtk::Gui::DPalette::TextTips));
}

// TipsLabel paintEvent 激活态：Text 色改为 Active 的 TextTips 色（B5 Active 侧）
TEST_F(UT_TipsLabel, paintEvent_ActiveState_TextTipsPaletteApplied)
{
    // Arrange：应用状态打桩为激活
    stub.set_lamda(static_cast<Qt::ApplicationState (*)()>(&QGuiApplication::applicationState),
                   []() { return Qt::ApplicationActive; });
    const auto paletteBefore = DPaletteHelper::instance()->palette(m_widget);
    QPaintEvent event(m_widget->rect());

    // Act
    m_widget->paintEvent(&event);

    // Assert：绘制后 Text 色等于源码公式的目标色（Active + TextTips）
    const auto paletteAfter = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteAfter.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Active, Dtk::Gui::DPalette::TextTips));

    // Act：再次绘制（幂等性）
    m_widget->paintEvent(&event);

    // Assert：重复绘制后取色公式结果保持一致
    const auto paletteTwice = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteTwice.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Active, Dtk::Gui::DPalette::TextTips));
}

class UT_TitleLabel : public ::testing::Test
{
public:
    void SetUp()
    {
        m_widget = new TitleLabel;
    }
    void TearDown()
    {
        stub.clear();
        delete m_widget;
    }

    TitleLabel *m_widget = nullptr;
    stub_ext::StubExt stub;
};

// TitleLabel 构造函数：默认无文本，仅继承 DLabel 行为
TEST_F(UT_TitleLabel, ctor_DefaultState_TextEmpty)
{
    // Arrange：fixture 已构造 TitleLabel
    ASSERT_NE(nullptr, m_widget);

    // Act：读取构造后的默认状态
    const QString text = m_widget->text();

    // Assert：默认无文本且无父对象
    EXPECT_TRUE(text.isEmpty());
    EXPECT_EQ(nullptr, m_widget->parentWidget());
}

// TitleLabel paintEvent 非激活态：Text 色改为 Inactive 的 TextTitle 色（B5 Inactive 侧）
TEST_F(UT_TitleLabel, paintEvent_InactiveState_TextTitlePaletteApplied)
{
    // Arrange：应用状态打桩为非激活
    stub.set_lamda(static_cast<Qt::ApplicationState (*)()>(&QGuiApplication::applicationState),
                   []() { return Qt::ApplicationInactive; });
    const auto paletteBefore = DPaletteHelper::instance()->palette(m_widget);
    QPaintEvent event(m_widget->rect());

    // Act
    m_widget->paintEvent(&event);

    // Assert：绘制后 Text 色等于源码公式的目标色（Inactive + TextTitle）
    const auto paletteAfter = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteAfter.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Inactive, Dtk::Gui::DPalette::TextTitle));

    // Act：再次绘制（幂等性）
    m_widget->paintEvent(&event);

    // Assert：重复绘制后取色公式结果保持一致
    const auto paletteTwice = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteTwice.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Inactive, Dtk::Gui::DPalette::TextTitle));
}

// TitleLabel paintEvent 激活态：Text 色改为 Active 的 TextTitle 色（B5 Active 侧）
TEST_F(UT_TitleLabel, paintEvent_ActiveState_TextTitlePaletteApplied)
{
    // Arrange：应用状态打桩为激活
    stub.set_lamda(static_cast<Qt::ApplicationState (*)()>(&QGuiApplication::applicationState),
                   []() { return Qt::ApplicationActive; });
    const auto paletteBefore = DPaletteHelper::instance()->palette(m_widget);
    QPaintEvent event(m_widget->rect());

    // Act
    m_widget->paintEvent(&event);

    // Assert：绘制后 Text 色等于源码公式的目标色（Active + TextTitle）
    const auto paletteAfter = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteAfter.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Active, Dtk::Gui::DPalette::TextTitle));

    // Act：再次绘制（幂等性）
    m_widget->paintEvent(&event);

    // Assert：重复绘制后取色公式结果保持一致
    const auto paletteTwice = DPaletteHelper::instance()->palette(m_widget);
    EXPECT_EQ(paletteTwice.color(Dtk::Gui::DPalette::Text),
              paletteBefore.color(Dtk::Gui::DPalette::Active, Dtk::Gui::DPalette::TextTitle));
}
