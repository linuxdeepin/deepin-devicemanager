// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DriverWaitingWidget 的 ctor/init/setValue/setText/onUpdateTheme。
//
// 用例计数声明表：
//
// | method | level | factors | min | actual |
// | --- | --- | --- | --- | --- |
// | DriverWaitingWidget::DriverWaitingWidget | middle | - | 1 | 1 |
// | DriverWaitingWidget::init | low | - | 1 | 1 |
// | DriverWaitingWidget::setValue | low | 边界0/50/100 | 3 | 3 |
// | DriverWaitingWidget::setText | low | 正常/空串 | 2 | 2 |
// | DriverWaitingWidget::onUpdateTheme | low | 直调/信号 | 2 | 2 |
//
// 通过 stub DWaterProgress::start 隔离动画计时器（不依赖真实定时器/设备环境），
// 通过 stub QWidget::repaint 捕获进度重绘调用，通过 DGuiApplicationHelper
// 主题切换驱动 onUpdateTheme 信号路径。QApplication 由 tests/src/test_main.cpp
// 提供（offscreen 全局实例），此处不再重建。

#include "DriverWaitingWidget.h"

#include <stubext.h>

#include <DWaterProgress>
#include <DGuiApplicationHelper>

#include <QLabel>
#include <QLayout>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace {

class DriverWaitingWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        start_count = 0;
        repaint_count = 0;
        // stub 动画启动：init 会 start() 计时器，测试环境不得依赖真实定时器
        stub.set_lamda(static_cast<void (DWaterProgress::*)()>(&DWaterProgress::start),
                       [this](DWaterProgress *self) {
                           __DBG_STUB_INVOKE__
                           ++start_count;
                       });
        // Act: 构造被测对象（ctor → init 也是被测方法）
        obj = new DriverWaitingWidget(QStringLiteral("loading"));
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    DriverWaitingWidget *obj = nullptr;
    int start_count = 0;
    int repaint_count = 0;
};

// setValue 多边界值断言结构一致 → 参数化
class DriverWaitingWidgetSetValueTest : public DriverWaitingWidgetTest,
                                        public ::testing::WithParamInterface<int>
{
};

}  // namespace

// ---- Constructor -----------------------------------------------------------
TEST_F(DriverWaitingWidgetTest, DriverWaitingWidget_Ctor_CreatesChildrenWithStatus)
{
    // Arrange: 打桩在 fixture 完成，初始文案与进度由 init 决定
    QScopedPointer<DriverWaitingWidget> subject;

    // Act: 构造被测对象（ctor 经 init 装配子部件）
    subject.reset(new DriverWaitingWidget(QStringLiteral("loading")));

    // Assert: 构造后状态文本为传入文案，进度初值为 init 设置的 50
    EXPECT_EQ(subject->mp_Label->text(), QStringLiteral("loading"));
    EXPECT_EQ(subject->mp_Progress->value(), 50);
}

// ---- init: 由 ctor 调用 -----------------------------------------------------
TEST_F(DriverWaitingWidgetTest, Init_OnCtor_BuildsLayoutAndStartsProgress)
{
    // Arrange: fixture 构造已产生一次启动计数，以此为基线
    const int started_after_ctor = start_count;

    // Act: 再构造一个实例，其 ctor 应恰好再触发一次 init 内的启动
    QScopedPointer<DriverWaitingWidget> subject(new DriverWaitingWidget(QStringLiteral("init")));

    // Assert: init 装配了布局，且启动次数恰好增加一次
    EXPECT_NE(subject->layout(), nullptr);
    EXPECT_EQ(start_count, started_after_ctor + 1);
}

// ---- setValue: 合法区间 0/50/100 边界 ---------------------------------------
TEST_P(DriverWaitingWidgetSetValueTest, SetValue_ValidRange_ProgressReflectsValue)
{
    // Arrange: 捕获 setValue 触发的重绘调用
    stub.set_lamda(static_cast<void (QWidget::*)()>(&QWidget::repaint),
                   [this](QWidget *self) {
                       __DBG_STUB_INVOKE__
                       ++repaint_count;
                   });
    const int value = GetParam();

    // Act
    obj->setValue(value);

    // Assert: 进度值与重绘都被同步更新
    EXPECT_EQ(obj->mp_Progress->value(), value);
    EXPECT_EQ(repaint_count, 1);
}

INSTANTIATE_TEST_SUITE_P(Boundaries, DriverWaitingWidgetSetValueTest,
                         ::testing::Values(0, 50, 100));

// ---- setText: 新文案 --------------------------------------------------------
TEST_F(DriverWaitingWidgetTest, SetText_NewText_LabelShowsText)
{
    // Arrange: 初始文案为 "loading"
    EXPECT_EQ(obj->mp_Label->text(), QStringLiteral("loading"));
    const QString initial = obj->mp_Label->text();

    // Act
    obj->setText(QStringLiteral("done"));

    // Assert: 标签显示新文案
    EXPECT_EQ(obj->mp_Label->text(), QStringLiteral("done"));
    EXPECT_NE(obj->mp_Label->text(), QStringLiteral("loading"));
}

// ---- setText: 空串边界 ------------------------------------------------------
TEST_F(DriverWaitingWidgetTest, SetText_EmptyString_ClearsLabel)
{
    // Arrange: 初始文案为 "loading"
    const QString initial = obj->mp_Label->text();
    EXPECT_EQ(initial, QStringLiteral("loading"));

    // Act
    obj->setText(QString());

    // Assert: 空串同样透传到标签
    EXPECT_TRUE(obj->mp_Label->text().isEmpty());
    EXPECT_NE(obj->mp_Label->text(), QStringLiteral("loading"));
}

// ---- onUpdateTheme: 直调 ----------------------------------------------------
TEST_F(DriverWaitingWidgetTest, OnUpdateTheme_DirectCall_LabelPaletteMatchesBase)
{
    // Arrange: 取应用调色板 Base 色作为期望值
    const QColor base = DGuiApplicationHelper::instance()->applicationPalette().color(QPalette::Base);

    // Act
    obj->onUpdateTheme();

    // Assert: 标签调色板 Window 颜色跟随应用 Base 色；重复调用结果稳定
    EXPECT_EQ(obj->mp_Label->palette().color(QPalette::Window), base);
    obj->onUpdateTheme();
    EXPECT_EQ(obj->mp_Label->palette().color(QPalette::Window), base);
}

// ---- onUpdateTheme: themeTypeChanged 信号路径 -------------------------------
TEST_F(DriverWaitingWidgetTest, OnUpdateTheme_ThemeTypeChanged_LabelPaletteFollows)
{
    // Arrange: stub 应用调色板为可辨识的 magenta 基色；直接发射 helper 的
    // themeTypeChanged 信号（DTK6 中该信号由配置监听驱动），驱动真实连接链路
    DGuiApplicationHelper *helper = DGuiApplicationHelper::instance();
    DPalette themed;
    themed.setColor(QPalette::Base, QColor(Qt::magenta));
    stub.set_lamda(static_cast<DPalette (DGuiApplicationHelper::*)() const>(&DGuiApplicationHelper::applicationPalette),
                   [themed](const DGuiApplicationHelper *self) -> DPalette {
                       __DBG_STUB_INVOKE__
                       return themed;
                   });
    const QColor before = obj->mp_Label->palette().color(QPalette::Window);

    // Act: 发射信号，经 connect 触发 onUpdateTheme
    const bool delivered = QMetaObject::invokeMethod(helper,
                                                     "themeTypeChanged",
                                                     Qt::DirectConnection,
                                                     Q_ARG(DGuiApplicationHelper::ColorType,
                                                           DGuiApplicationHelper::DarkType));

    // Assert: 信号送达，标签 Window 色被 onUpdateTheme 更新为桩调色板 Base 色
    EXPECT_TRUE(delivered);
    EXPECT_NE(obj->mp_Label->palette().color(QPalette::Window), before);
    EXPECT_EQ(obj->mp_Label->palette().color(QPalette::Window), QColor(Qt::magenta));
}
