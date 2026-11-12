// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_getdriverpathwidget.cpp
 * @brief GetDriverPathWidget 单元测试（Mode 2 重写，逐类闭环）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method               | level | factors                        | min | actual |
 * |----------------------|-------|--------------------------------|-----|--------|
 * | path                 | low   | default,forward                | 1   | 2      |
 * | includeSubdir        | low   | checked,unchecked,partial      | 1   | 3      |
 * | updateTipLabelText   | low   | set-text-tooltip               | 1   | 1      |
 * | clearTipLabelText    | low   | clear-after-update             | 1   | 1      |
 * | signalNotLocalFolder | low   | forward-false,forward-true     | 1   | 2      |
 * | onUpdateTheme        | low   | window-follows-base            | 1   | 1      |
 *
 * 分支/等价类说明（源自 MCP get_code_snippet 真实源码，GetDriverPathWidget.cpp）：
 *   - 构造：mp_includeCheckBox 默认 Qt::Checked，objectName/accessibleName
 *     均为 "MpIncludeCheckBox"；tipLabel 初始为空；
 *   - path()：转发 UrlChooserEdit::text()（构造默认 QDir::homePath()）；
 *   - includeSubdir()：checkState == Qt::Checked 才返回 true
 *     （PartiallyChecked/Unchecked 均为 false）；
 *   - updateTipLabelText：同时写入 text 与 toolTip；clearTipLabelText 置空两者；
 *   - 构造中 connect：UrlChooserEdit::signalNotLocalFolder(bool) → 转发本类
 *     同名信号并调用 clearTipLabelText；
 *   - onUpdateTheme（Qt6 分支）：titleLabel 的 Window 色刷跟随 Base 色刷。
 * 打桩策略（外部依赖全部 stub，不依赖真实设备环境）：
 *   - UrlChooserEdit::text（非虚成员函数）→ 验证 path() 转发链路；
 *   - 其余对象走真实 offscreen 行为（DWidget/DCheckBox/DLabel 正常构造）。
 */

#include "GetDriverPathWidget.h"
#include "UrlChooserEdit.h"
#include "ut_Head.h"
#include "stub.h"

#include <QSignalSpy>
#include <QPalette>

#include <gtest/gtest.h>

// ---- stub：UrlChooserEdit::text 非虚成员函数，替换为固定路径 ----
static QString g_utUrlText = QStringLiteral("dvm-stub://drivers");
static QString ut_urlText()
{
    return g_utUrlText;
}

class UT_GetDriverPathWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_w = new GetDriverPathWidget;
    }
    void TearDown()
    {
        delete m_w;
    }

    GetDriverPathWidget *m_w = nullptr;
};

// 构造：默认勾选子目录、tip 为空、无障碍名就绪
TEST_F(UT_GetDriverPathWidget, GetDriverPathWidget_DefaultState_CheckBoxCheckedTipEmpty)
{
    // Arrange：fixture 已构造默认 GetDriverPathWidget
    DCheckBox *cb = m_w->mp_includeCheckBox;

    // Act：读取构造后的默认状态
    const bool sub = m_w->includeSubdir();
    const Qt::CheckState state = cb->checkState();

    // Assert：默认勾选、objectName/accessibleName 就绪、tip 为空
    EXPECT_EQ(true, sub);
    EXPECT_EQ(Qt::Checked, state);
    EXPECT_EQ(QStringLiteral("MpIncludeCheckBox"), cb->objectName());
    EXPECT_EQ(QStringLiteral("MpIncludeCheckBox"), cb->accessibleName());
    EXPECT_EQ(QString(), m_w->mp_tipLabel->text());
}

// path：默认转发 UrlChooserEdit 内部输入框初始文本（homePath）
TEST_F(UT_GetDriverPathWidget, path_DefaultText_ReturnsHomePath)
{
    // Arrange：fixture 已构造默认 GetDriverPathWidget
    const QString home = QDir::homePath();

    // Act
    const QString ret = m_w->path();

    // Assert：默认文本即 homePath 且目录真实存在
    EXPECT_EQ(home, ret);
    EXPECT_EQ(true, QDir(ret).exists());
}

// path：转发 UrlChooserEdit::text()（打桩验证链路）
TEST_F(UT_GetDriverPathWidget, path_StubbedText_ForwardsUrlChooserEdit)
{
    // Arrange：替换 UrlChooserEdit::text 返回固定路径
    Stub stub;
    g_utUrlText = QStringLiteral("dvm-stub://drivers");
    stub.set(ADDR(UrlChooserEdit, text), ut_urlText);

    // Act
    const QString ret = m_w->path();

    // Assert：path 即打桩后的固定路径，且不等于默认 homePath
    EXPECT_EQ(QStringLiteral("dvm-stub://drivers"), ret);
    EXPECT_NE(QDir::homePath(), ret);
    g_utUrlText = QDir::homePath();
}

// includeSubdir：勾选 → true
TEST_F(UT_GetDriverPathWidget, includeSubdir_CheckedState_ReturnsTrue)
{
    // Arrange
    m_w->mp_includeCheckBox->setCheckState(Qt::Checked);

    // Act
    const bool sub = m_w->includeSubdir();

    // Assert：返回 true 且复选框保持勾选态
    EXPECT_EQ(true, sub);
    EXPECT_EQ(Qt::Checked, m_w->mp_includeCheckBox->checkState());
}

// includeSubdir：未勾选 → false
TEST_F(UT_GetDriverPathWidget, includeSubdir_UncheckedState_ReturnsFalse)
{
    // Arrange
    m_w->mp_includeCheckBox->setCheckState(Qt::Unchecked);

    // Act
    const bool sub = m_w->includeSubdir();

    // Assert：返回 false 且复选框保持未勾选态
    EXPECT_EQ(false, sub);
    EXPECT_EQ(Qt::Unchecked, m_w->mp_includeCheckBox->checkState());
}

// includeSubdir：半选 → false（仅 Qt::Checked 视为包含子目录）
TEST_F(UT_GetDriverPathWidget, includeSubdir_PartiallyCheckedState_ReturnsFalse)
{
    // Arrange
    m_w->mp_includeCheckBox->setCheckState(Qt::PartiallyChecked);

    // Act
    const bool sub = m_w->includeSubdir();

    // Assert：返回 false 且复选框保持半选态
    EXPECT_EQ(false, sub);
    EXPECT_EQ(Qt::PartiallyChecked, m_w->mp_includeCheckBox->checkState());
}

// updateTipLabelText：同步写入 text 与 toolTip
TEST_F(UT_GetDriverPathWidget, updateTipLabelText_NewText_UpdatesTextAndToolTip)
{
    // Arrange：初始 tip 为空
    EXPECT_EQ(QString(), m_w->mp_tipLabel->text());

    // Act
    m_w->updateTipLabelText(QStringLiteral("dvm-stub-dir"));

    // Assert
    EXPECT_EQ(QStringLiteral("dvm-stub-dir"), m_w->mp_tipLabel->text());
    EXPECT_EQ(QStringLiteral("dvm-stub-dir"), m_w->mp_tipLabel->toolTip());
}

// clearTipLabelText：更新后清空 → text/toolTip 均为空
TEST_F(UT_GetDriverPathWidget, clearTipLabelText_AfterUpdate_ClearsTextAndToolTip)
{
    // Arrange：先写入非空 tip
    m_w->updateTipLabelText(QStringLiteral("not local folder"));
    EXPECT_EQ(QStringLiteral("not local folder"), m_w->mp_tipLabel->text());

    // Act
    m_w->clearTipLabelText();

    // Assert
    EXPECT_EQ(QString(), m_w->mp_tipLabel->text());
    EXPECT_EQ(QString(), m_w->mp_tipLabel->toolTip());
}

// signalNotLocalFolder(false)：转发信号并清空 tip
TEST_F(UT_GetDriverPathWidget, signalNotLocalFolder_ForwardFalse_EmitsSignalAndClearsTip)
{
    // Arrange：预置非空 tip 并挂信号探针
    m_w->updateTipLabelText(QStringLiteral("tip"));
    QSignalSpy spy(m_w, &GetDriverPathWidget::signalNotLocalFolder);

    // Act：内部输入控件发出非本地目录信号（false = 非本地）
    emit m_w->mp_fileChooseEdit->signalNotLocalFolder(false);

    // Assert：转发 1 次且参数为 false，tip 被清空
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(QList<QVariant>{QVariant(false)}, spy.takeFirst());
    EXPECT_EQ(QString(), m_w->mp_tipLabel->text());
}

// signalNotLocalFolder(true)：转发信号并清空 tip
TEST_F(UT_GetDriverPathWidget, signalNotLocalFolder_ForwardTrue_EmitsSignalAndClearsTip)
{
    // Arrange：预置非空 tip 并挂信号探针
    m_w->updateTipLabelText(QStringLiteral("tip2"));
    QSignalSpy spy(m_w, &GetDriverPathWidget::signalNotLocalFolder);

    // Act：内部输入控件发出信号（true = 本地）
    emit m_w->mp_fileChooseEdit->signalNotLocalFolder(true);

    // Assert：转发 1 次且参数为 true，tip 被清空
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(QList<QVariant>{QVariant(true)}, spy.takeFirst());
    EXPECT_EQ(QString(), m_w->mp_tipLabel->text());
}

// onUpdateTheme：titleLabel 的 Window 色刷跟随 Base 色刷（Qt6 分支）
TEST_F(UT_GetDriverPathWidget, onUpdateTheme_BaseColor_WindowFollowsBase)
{
    // Arrange：预置 Window=红 / Base=绿 的调色板
    QPalette plt = m_w->mp_titleLabel->palette();
    plt.setColor(QPalette::Window, Qt::red);
    plt.setColor(QPalette::Base, Qt::green);
    m_w->mp_titleLabel->setPalette(plt);

    // Act
    m_w->onUpdateTheme();

    // Assert：Window 色变为 Base 色（绿），不再是 Arrange 时的红
    EXPECT_EQ(QColor(Qt::green).rgba(),
              m_w->mp_titleLabel->palette().color(QPalette::Window).rgba());
    EXPECT_NE(QColor(Qt::red).rgba(),
              m_w->mp_titleLabel->palette().color(QPalette::Window).rgba());
}
