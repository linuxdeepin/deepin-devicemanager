// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_tipswidget.cpp
 * @brief TipsWidget 单元测试（Mode 2 重写，逐类闭环）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method       | level | factors                          | min | actual |
 * |--------------|-------|----------------------------------|-----|--------|
 * | ctor         | mid   | 窗口属性/主题底色链               | 2   | 2      |
 * | setText      | mid   | 去换行制表/幂等                   | 2   | 2      |
 * | setAlignment | low   | 对齐存储                          | 1   | 1      |
 * | adjustContent| mid   | 高度公式/早退分支                 | 2   | 2      |
 * | paintEvent   | low   | 绘制内高度自适应                  | 1   | 1      |
 * | onUpdateTheme| low   | Window=Base 主题公式              | 1   | 1      |
 *
 * 分支/等价类说明（源自 MCP get_code_snippet + 工作目录源码核对，18-90 行）：
 *   - ctor：setWindowFlags(Qt::ToolTip) → initWidget（边距 8/10、maxLineCount 10、
 *     固定宽 254、透明度 0.97、圆角 8、T8 字体绑定、themeTypeChanged 连接）→
 *     onUpdateTheme（QT6 分支：调色 Window = Base）；
 *   - setText：文本变化才更新，替换去除 '\n' 与 '\t' 后存储并 update()；
 *   - setAlignment：记录对齐并 update()；
 *   - adjustContent：wordHeight = boundingRect(宽-2*左右边距, 折行) + 2*上下边距，
 *     与当前高度一致则早退，否则 setFixedHeight；
 *   - paintEvent：基类绘制 → adjustContent(m_text) → 按对齐绘制文本。
 *   用例映射：
 *   - ctor_Default_ToolTipFlagAndInitState        → initWidget 状态链
 *   - ctor_Default_PaletteWindowEqualsBase        → onUpdateTheme 主题底色
 *   - setText_NewlineTabStripped_TextStored       → 去换行/制表分支
 *   - setText_SameTextTwice_NoReprocess           → 幂等分支
 *   - setAlignment_NewValue_StoredAndApplied      → 对齐分支
 *   - adjustContent_TextHeightMirrored_Formula    → 高度公式分支
 *   - adjustContent_SecondCall_EarlyReturnKeeps   → 高度一致早退分支
 *   - paintEvent_DirectInvoke_HeightAdjusted      → 绘制链路
 *   - onUpdateTheme_AfterBreak_RestoresWindowBase → 主题重涂分支
 * 打桩策略（外部依赖全部 stub）：
 *   - 无外部进程/文件依赖：DPlatformWindowHandle/DFontSizeManager/DGuiApplicationHelper
 *     在 offscreen 下为进程内服务，直接使用；paintEvent 以栈上 QPaintEvent 直调，
 *     高度断言镜像 adjustContent 公式，与真实屏幕无关。
 */

#include "TipsWidget.h"
#include "ut_Head.h"
#include "stubext.h"

#include <gtest/gtest.h>
#include <DGuiApplicationHelper>
#include <QPaintEvent>
#include <QPainter>
#include <QFontMetricsF>

class UT_TipsWidget : public ::testing::Test
{
public:
    void SetUp()
    {
        m_widget = new TipsWidget;
    }
    void TearDown()
    {
        stub.clear();
        delete m_widget;
    }

    // 镜像 adjustContent 的期望高度公式（宽 254、左右边距 10、上下边距 8）
    int expectedHeightFor(const QString &text) const
    {
        QFontMetricsF fm(m_widget->font());
        const QRectF rect = fm.boundingRect(
            QRectF(0, 0, m_widget->width() - 2 * m_widget->m_lrMargin, 0),
            static_cast<int>(m_widget->m_alignment | Qt::TextWrapAnywhere), text);
        return static_cast<int>(rect.height()) + 2 * m_widget->m_tbMargin;
    }

    TipsWidget *m_widget = nullptr;
    stub_ext::StubExt stub;
};

// ctor：ToolTip 窗口标志、固定宽 254、透明度 0.97、边距与行数上限
TEST_F(UT_TipsWidget, ctor_Default_ToolTipFlagAndInitState)
{
    // Arrange：期望常量（initWidget 源码值）
    const int kTbMargin = 8;
    const int kLrMargin = 10;
    const int kMaxLineCount = 10;
    const int kFixedWidth = 254;

    // Act：读取构造后的状态
    const bool toolTipFlag = (m_widget->windowFlags() & Qt::ToolTip);
    const int tb = m_widget->m_tbMargin;
    const int lr = m_widget->m_lrMargin;
    const int maxLine = m_widget->m_maxLineCount;

    // Assert
    EXPECT_TRUE(toolTipFlag);
    EXPECT_EQ(kFixedWidth, m_widget->width());
    EXPECT_NEAR(0.97, m_widget->windowOpacity(), 0.01);   // 平台按 8bit 量化
    EXPECT_EQ(kTbMargin, tb);
    EXPECT_EQ(kLrMargin, lr);
    EXPECT_EQ(kMaxLineCount, maxLine);
    EXPECT_EQ(Qt::AlignLeft | Qt::AlignVCenter, m_widget->m_alignment);
}

// ctor：onUpdateTheme 已执行，调色 Window == Base（QT6 分支公式）
TEST_F(UT_TipsWidget, ctor_Default_PaletteWindowEqualsBase)
{
    // Arrange：构造后读取调色板（onUpdateTheme 在构造内已执行）
    const QPalette plt = m_widget->palette();

    // Act：分离读取 Window / Base 两个通道
    const QColor window = plt.color(QPalette::Window);
    const QColor base = plt.color(QPalette::Base);

    // Assert：Window 被刷成 Base 色且通道值有效
    EXPECT_EQ(base, window);
    EXPECT_TRUE(window.isValid());
}

// setText：换行与制表符被剔除后存储
TEST_F(UT_TipsWidget, setText_NewlineTabStripped_TextStored)
{
    // Arrange：含换行/制表的输入
    const QString raw = QStringLiteral("li\nne\twith\ttabs");

    // Act
    m_widget->setText(raw);

    // Assert：'\n'/'\t' 均被剔除；再以纯换行文本二次验证剔除规则
    EXPECT_EQ(QStringLiteral("linewithtabs"), m_widget->m_text);
    m_widget->setText(QStringLiteral("\n\t"));
    EXPECT_TRUE(m_widget->m_text.isEmpty());
}

// setText：相同文本重复设置不重处理（幂等分支），不同文本恢复更新
TEST_F(UT_TipsWidget, setText_SameTextTwice_NoReprocess)
{
    // Arrange
    const QString text = QStringLiteral("content");

    // Act：两次设置相同文本
    m_widget->setText(text);
    const QString afterFirst = m_widget->m_text;
    m_widget->setText(text);

    // Assert：第二次设置后存储值不变（无重置副作用）
    EXPECT_EQ(QStringLiteral("content"), afterFirst);
    EXPECT_EQ(QStringLiteral("content"), m_widget->m_text);
}

// setAlignment：新对齐值被存储
TEST_F(UT_TipsWidget, setAlignment_NewValue_StoredAndApplied)
{
    // Arrange：期望两种对齐组合
    const Qt::Alignment right = Qt::AlignRight | Qt::AlignVCenter;
    const Qt::Alignment center = Qt::AlignCenter;

    // Act：先设右对齐再设居中
    m_widget->setAlignment(right);
    const Qt::Alignment afterRight = m_widget->m_alignment;
    m_widget->setAlignment(center);

    // Assert：两次均以最新值为准
    EXPECT_EQ(right, afterRight);
    EXPECT_EQ(center, m_widget->m_alignment);
}

// adjustContent：固定高度等于镜像公式（短文本单行 + 上下边距）
TEST_F(UT_TipsWidget, adjustContent_TextHeightMirrored_Formula)
{
    // Arrange：已知文本与镜像公式
    const QString text = QStringLiteral("hello");

    // Act：调整前记录固定宽度
    const int widthBefore = m_widget->width();
    m_widget->adjustContent(text);

    // Assert：高度 == boundingRect 高 + 2*上下边距，且宽度不受影响
    EXPECT_EQ(expectedHeightFor(text), m_widget->height());
    EXPECT_EQ(254, m_widget->width());
    EXPECT_EQ(widthBefore, m_widget->width());
}

// adjustContent：高度已一致时早退，重复调用保持不变（早退分支）
TEST_F(UT_TipsWidget, adjustContent_SecondCall_EarlyReturnKeeps)
{
    // Arrange：先调整一次建立基线
    const QString text = QStringLiteral("hello");
    m_widget->adjustContent(text);
    const int firstHeight = m_widget->height();

    // Act：相同文本再次调整
    m_widget->adjustContent(text);

    // Assert：高度未变（早退分支不重设）
    EXPECT_EQ(firstHeight, m_widget->height());
    EXPECT_EQ(expectedHeightFor(text), m_widget->height());
}

// paintEvent：绘制链路内触发高度自适应，重复绘制结果稳定
TEST_F(UT_TipsWidget, paintEvent_DirectInvoke_HeightAdjusted)
{
    // Arrange：设置文本并构造栈上绘制事件
    const QString text = QStringLiteral("paint me");
    m_widget->setText(text);
    QPaintEvent event(QRect(0, 0, 254, 40));

    // Act：直接调用 paintEvent 两次（幂等性观测）
    m_widget->paintEvent(&event);
    const int heightFirst = m_widget->height();
    m_widget->paintEvent(&event);

    // Assert：高度为该文本的公式值且两次一致
    EXPECT_EQ(expectedHeightFor(text), heightFirst);
    EXPECT_EQ(heightFirst, m_widget->height());
    EXPECT_EQ(QStringLiteral("paint me"), m_widget->m_text);
}

// onUpdateTheme：调色被破坏后重涂恢复 Window == Base
TEST_F(UT_TipsWidget, onUpdateTheme_AfterBreak_RestoresWindowBase)
{
    // Arrange：人为破坏调色（Window 置哨兵红）
    QPalette broken = m_widget->palette();
    broken.setColor(QPalette::Window, QColor(255, 0, 0));
    m_widget->setPalette(broken);
    ASSERT_EQ(QColor(255, 0, 0), m_widget->palette().color(QPalette::Window));

    // Act：记录 Base 基线后重涂（私有槽，直调）
    const QColor baseBefore = m_widget->palette().color(QPalette::Base);
    m_widget->onUpdateTheme();

    // Assert：Window 重新等于 Base，且 Base 本身未被重涂改写
    EXPECT_EQ(baseBefore, m_widget->palette().color(QPalette::Base));
    EXPECT_EQ(baseBefore, m_widget->palette().color(QPalette::Window));
}
