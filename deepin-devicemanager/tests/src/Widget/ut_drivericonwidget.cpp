// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_drivericonwidget.cpp
 * @brief DriverIconWidget 与 palrtteTransparency 单元测试（Mode 2 重写，逐类闭环）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method | level | factors                        | min | actual |
 * |--------|-------|--------------------------------|-----|--------|
 * | ctor   | mid   | progress/pixmap 两条构造链      | 2   | 3      |
 * | initUI | mid   | title/desc/调色/主题            | 2   | 4      |
 * | dtor   | low   | 构造析构循环                    | 1   | 1      |
 *
 * 说明：MCP 图谱仅索引了 QPixmap 构造函数（QString 构造函数 L36-46 未入图），
 * 源码以工作目录 drivericonwidget.cpp 为准；两个构造函数均经 initUI 完成布局，
 * initUI 用例同时覆盖共享链路。
 *
 * 分支/等价类说明（源自 MCP get_code_snippet + 工作目录源码核对，36-122 行）：
 *   - ctor#1（QString 标题）：iconWidget = new DWaterProgress（100×100）→
 *     setValue(50) → start() → initUI；
 *   - ctor#2（QPixmap）：iconWidget = new DLabel（100×100、pixmap、scaledContents）
 *     → initUI；
 *   - initUI：图标组件居中加入垂直布局；标题 label（宽 440、T7 Medium、
 *     palrtteTransparency(-10)）；描述 label（ElideRight 截断、tooltip 保留全文、
 *     palrtteTransparency(-30)）；两个 label 均连接 DGuiApplicationHelper::
 *     themeTypeChanged 重涂透明度；
 *   - palrtteTransparency（static 自由函数）：WindowText =
 *     adjustColor(Active BrightText, alpha)，标题 -10 / 描述 -30。
 *   用例映射：
 *   - ctor_WithTitleDesc_WaterProgressCreatedValue50     → ctor#1 全链
 *   - ctor_WithTitleDesc_IconCenteredInLayout            → initUI 图标分支
 *   - ctor_WithPixmap_IconLabelScaledAndSized            → ctor#2 全链
 *   - initUI_TitleLabel_FixedWidthAndFullText            → 标题分支
 *   - initUI_DescLabel_TooltipFullTextElidedText         → 描述截断/tooltip 分支
 *   - initUI_LabelsPalette_WindowTextAdjusted            → palrtteTransparency(-10/-30)
 *   - initUI_ThemeChange_RestoresAdjustedColors          → themeTypeChanged 重涂
 *   - dtor_RepeatedConstructDestroy_FreshInstanceUsable  → 析构
 * 打桩策略（外部依赖全部 stub，不启动真实动画计时器）：
 *   - DWaterProgress::start（非虚成员）→ 计数桩，不启动进度动画计时器，
 *     setValue(50) 后 value 状态可直接断言；
 *   - DGuiApplicationHelper/DFontSizeManager 为 DTK 进程内服务，offscreen 正常；
 *     主题信号在用例内手动重发，调色断言镜像 adjustColor 公式，主题无关。
 */

#include "drivericonwidget.h"
#include "ut_Head.h"
#include "stubext.h"

#include <gtest/gtest.h>
#include <DGuiApplicationHelper>
#include <DLabel>
#include <DWaterProgress>
#include <QLabel>
#include <QVBoxLayout>

// ---- stub：拦截 DWaterProgress::start，避免真实动画计时器 ----
static int g_utWaterStartCount = 0;

class UT_DriverIconWidget : public ::testing::Test
{
public:
    void SetUp()
    {
        g_utWaterStartCount = 0;
    }
    void TearDown()
    {
        stub.clear();
    }

    // 找到文本匹配的子 DLabel（initUI 内部创建，仅能经对象树访问）
    DLabel *findLabelByText(const QString &text)
    {
        const auto labels = m_widget->findChildren<DLabel *>();
        for (DLabel *lbl : labels) {
            if (lbl->text() == text || lbl->toolTip() == text)
                return lbl;
        }
        return nullptr;
    }

    DriverIconWidget *m_widget = nullptr;
    stub_ext::StubExt stub;
};

// QString 构造函数：DWaterProgress 创建、尺寸与初值 50、start 恰好一次（ctor#1 全链）
TEST_F(UT_DriverIconWidget, ctor_WithTitleDesc_WaterProgressCreatedValue50)
{
    // Arrange：拦截 start 避免动画计时器
    stub.set_lamda(&DWaterProgress::start, [](DWaterProgress *) { ++g_utWaterStartCount; });

    // Act
    m_widget = new DriverIconWidget(QStringLiteral("正在安装"),
                                    QStringLiteral("请稍候"));

    // Assert：进度控件创建、100×100、初值 50、start 一次
    ASSERT_NE(nullptr, m_widget->iconWidget);
    EXPECT_EQ(50, m_widget->iconWidget->value());
    EXPECT_EQ(QSize(100, 100), m_widget->iconWidget->size());
    EXPECT_EQ(1, g_utWaterStartCount);
}

// QString 构造函数：图标组件被居中挂入布局（initUI 图标分支）
TEST_F(UT_DriverIconWidget, ctor_WithTitleDesc_IconCenteredInLayout)
{
    // Arrange：拦截 start
    stub.set_lamda(&DWaterProgress::start, [](DWaterProgress *) { ++g_utWaterStartCount; });

    // Act
    m_widget = new DriverIconWidget(QStringLiteral("标题"),
                                    QStringLiteral("描述"));

    // Assert：布局存在且图标组件的父窗口为本控件（已加入布局）
    ASSERT_NE(nullptr, m_widget->layout());
    ASSERT_NE(nullptr, m_widget->iconWidget);
    EXPECT_EQ(m_widget, m_widget->iconWidget->parentWidget());
    EXPECT_EQ(QSize(100, 100), m_widget->iconWidget->size());
}

// QPixmap 构造函数：图标 label 100×100、pixmap 生效、scaledContents 开启（ctor#2 全链）
TEST_F(UT_DriverIconWidget, ctor_WithPixmap_IconLabelScaledAndSized)
{
    // Arrange：50×50 的实心图（放大到 100×100 显示）
    QPixmap pixmap(50, 50);
    pixmap.fill(Qt::red);

    // Act
    m_widget = new DriverIconWidget(pixmap, QStringLiteral("标题"),
                                    QStringLiteral("描述"));

    // Assert：存在带 pixmap 的子 label、固定 100×100、scaledContents 为真
    DLabel *iconLabel = m_widget->findChild<DLabel *>();
    ASSERT_NE(nullptr, iconLabel);
    EXPECT_FALSE(iconLabel->pixmap().isNull());
    EXPECT_EQ(QSize(100, 100), iconLabel->size());
    EXPECT_TRUE(iconLabel->hasScaledContents());
    EXPECT_EQ(nullptr, m_widget->iconWidget);   // QPixmap 构造链不创建进度控件
}

// initUI 标题分支：标题 label 宽 440、显示完整标题文本
TEST_F(UT_DriverIconWidget, initUI_TitleLabel_FixedWidthAndFullText)
{
    // Arrange
    const QString title = QStringLiteral("驱动管理");

    // Act
    m_widget = new DriverIconWidget(QPixmap(10, 10), title,
                                    QStringLiteral("描述"));

    // Assert
    DLabel *titleLabel = findLabelByText(title);
    ASSERT_NE(nullptr, titleLabel);
    EXPECT_EQ(title, titleLabel->text());
    EXPECT_EQ(440, titleLabel->width());
}

// initUI 描述分支：长描述被 ElideRight 截断显示，tooltip 保留全文
TEST_F(UT_DriverIconWidget, initUI_DescLabel_TooltipFullTextElidedText)
{
    // Arrange：长描述（超过 440px 宽度必被截断）
    const QString longDesc = QStringLiteral("这是一段非常长的描述文本，"
                                            "用于验证 ElideRight 截断逻辑，"
                                            "反复重复以超过宽度限制反复重复以超过宽度限制，"
                                            "反复重复以超过宽度限制反复重复以超过宽度限制");

    // Act
    m_widget = new DriverIconWidget(QPixmap(10, 10), QStringLiteral("标题"),
                                    longDesc);

    // Assert：tooltip 为全文；显示文本被截断（含省略号、短于全文）
    DLabel *descLabel = findLabelByText(longDesc);
    ASSERT_NE(nullptr, descLabel);
    EXPECT_EQ(longDesc, descLabel->toolTip());
    EXPECT_NE(longDesc, descLabel->text());
    EXPECT_TRUE(descLabel->text().contains(QStringLiteral("…")));
}

// initUI 调色分支：标题/描述 WindowText = adjustColor(BrightText, -10/-30)
// （覆盖 palrtteTransparency 两个调用点）
TEST_F(UT_DriverIconWidget, initUI_LabelsPalette_WindowTextAdjusted)
{
    // Arrange：构造并取得两个 label
    const QString title = QStringLiteral("标题");
    m_widget = new DriverIconWidget(QPixmap(10, 10), title,
                                    QStringLiteral("描述文本"));
    DLabel *titleLabel = findLabelByText(title);
    DLabel *descLabel = findLabelByText(QStringLiteral("描述文本"));
    ASSERT_NE(nullptr, titleLabel);
    ASSERT_NE(nullptr, descLabel);

    // Act：读取构造后的 WindowText 颜色（palrtteTransparency 在 initUI 中已执行）
    const QColor titleColor = titleLabel->palette().color(QPalette::WindowText);
    const QColor descColor = descLabel->palette().color(QPalette::WindowText);

    // Assert：与源码公式镜像（Active BrightText 按 alpha 调整）
    const QPalette refPalette = titleLabel->palette();
    const QColor base = refPalette.color(QPalette::Active, QPalette::BrightText);
    EXPECT_EQ(DGuiApplicationHelper::adjustColor(base, 0, 0, 0, 0, 0, 0, -10), titleColor);
    EXPECT_EQ(DGuiApplicationHelper::adjustColor(base, 0, 0, 0, 0, 0, 0, -30), descColor);
}

// initUI 主题信号分支：themeTypeChanged 重发后，被改坏的调色被重新计算恢复
TEST_F(UT_DriverIconWidget, initUI_ThemeChange_RestoresAdjustedColors)
{
    // Arrange：构造后人为破坏标题 label 调色（哨兵色）
    const QString title = QStringLiteral("标题");
    m_widget = new DriverIconWidget(QPixmap(10, 10), title,
                                    QStringLiteral("描述文本"));
    DLabel *titleLabel = findLabelByText(title);
    DLabel *descLabel = findLabelByText(QStringLiteral("描述文本"));
    ASSERT_NE(nullptr, titleLabel);
    ASSERT_NE(nullptr, descLabel);
    QPalette broken = titleLabel->palette();
    broken.setColor(QPalette::WindowText, QColor(255, 0, 0));
    titleLabel->setPalette(broken);
    QPalette brokenDesc = descLabel->palette();
    brokenDesc.setColor(QPalette::WindowText, QColor(0, 0, 255));
    descLabel->setPalette(brokenDesc);
    const QColor base = titleLabel->palette().color(QPalette::Active, QPalette::BrightText);
    const QColor expectedTitle = DGuiApplicationHelper::adjustColor(base, 0, 0, 0, 0, 0, 0, -10);
    const QColor expectedDesc = DGuiApplicationHelper::adjustColor(base, 0, 0, 0, 0, 0, 0, -30);
    ASSERT_EQ(QColor(255, 0, 0), titleLabel->palette().color(QPalette::WindowText));
    ASSERT_EQ(QColor(0, 0, 255), descLabel->palette().color(QPalette::WindowText));

    // Act：重发 themeTypeChanged（Qt6 信号为公开成员），触发重涂 lambda
    emit DGuiApplicationHelper::instance()->themeTypeChanged(
        DGuiApplicationHelper::instance()->themeType());

    // Assert：两个 label 的 WindowText 均恢复为公式色（哨兵色被覆盖）
    EXPECT_EQ(expectedTitle, titleLabel->palette().color(QPalette::WindowText));
    EXPECT_EQ(expectedDesc, descLabel->palette().color(QPalette::WindowText));
}

// 析构：反复构造/析构不崩溃，新实例功能完整
TEST_F(UT_DriverIconWidget, dtor_RepeatedConstructDestroy_FreshInstanceUsable)
{
    // Arrange：QPixmap 构造链（无计时器依赖）
    QPixmap icon(10, 10);
    icon.fill(Qt::gray);

    // Act：连续 3 次构造/析构
    for (int i = 0; i < 3; ++i) {
        DriverIconWidget *w = new DriverIconWidget(icon,
                                                   QStringLiteral("标题"),
                                                   QStringLiteral("描述"));
        ASSERT_NE(nullptr, w->layout());
        delete w;
    }

    // Assert：析构后新实例仍可正常构造并完成布局
    m_widget = new DriverIconWidget(QPixmap(10, 10), QStringLiteral("标题"),
                                    QStringLiteral("描述"));
    ASSERT_NE(nullptr, m_widget);
    EXPECT_NE(nullptr, m_widget->layout());
    // 图标 label + 标题 label + 描述 label 共 3 个 DLabel
    EXPECT_EQ(3, m_widget->findChildren<DLabel *>().count());
}
