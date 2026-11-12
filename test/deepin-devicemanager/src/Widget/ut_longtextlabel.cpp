// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 LongTextLabel 的构造函数与 paintEvent。
//
// 用例计数声明表：
//
// | method | level | factors | min | actual |
// | --- | --- | --- | --- | --- |
// | LongTextLabel::LongTextLabel | middle | - | 1 | 1 |
// | LongTextLabel::paintEvent | low | B1(锚点),B2(内嵌),B3(省略) | 3 | 4 |
//
// paintEvent 通过 stub QTextDocument::setHtml 捕获重装结果、stub
// QFontMetrics::elidedText 固定省略输出，隔离真实绘制与字体度量，
// 逐分支验证 HTML <a> 链接提取/重组逻辑。QApplication 由
// tests/src/test_main.cpp 提供（offscreen 全局实例），此处不再重建。

#include "LongTextLabel.h"

#include <stubext.h>

#include <QTextDocument>
#include <QPainter>
#include <QPixmap>

#include <gtest/gtest.h>

// ---- 方法分支清单（源自 MCP get_code_snippet LongTextLabel::paintEvent 32-78 行）----
// B1: reg.match(text).hasMatch()            → 用例3/4/5 走 true；用例2 走 false
// B2: contentMatch.hasMatch()               → 用例3/4 走 true；用例5 走 false
// B3: !html.isEmpty() && !OS.isEmpty()      → 用例3/4 走 true；用例2/5 走 false
// B5: !html.isEmpty() && !OS.isEmpty()      → 用例3/4 走 true；用例2/5 走 false
// （B4 为 elidedText 调用点，非分支；方法复杂度 < 10，无循环/异常分支）

namespace {

class LongTextLabelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        captured_dst.clear();
        paint_count = 0;
        elided_return = QStringLiteral("EL");
        stub.set_lamda(static_cast<void (QTextDocument::*)(const QString &)>(&QTextDocument::setHtml),
                       [this](QTextDocument *self, const QString &html) {
                           __DBG_STUB_INVOKE__
                           captured_dst = html;
                           ++paint_count;
                       });
        stub.set_lamda(static_cast<QString (QFontMetrics::*)(const QString &, Qt::TextElideMode, int, int) const>(&QFontMetrics::elidedText),
                       [this](QFontMetrics *self, const QString &text, Qt::TextElideMode mode, int width, int flags) -> QString {
                           __DBG_STUB_INVOKE__
                           return elided_return;
                       });
        // Act: 构造被测对象（ctor 也是被测方法）
        obj = new LongTextLabel;
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    // 渲染一次以触发 paintEvent，返回像素图供断言
    QPixmap renderOnce(int w, int h)
    {
        obj->resize(w, h);
        QPixmap pix(w, h);
        obj->render(&pix);
        return pix;
    }

    stub_ext::StubExt stub;
    LongTextLabel *obj = nullptr;
    QString captured_dst;
    QString elided_return;
    int paint_count = 0;
};

}  // namespace

// ---- Constructor -----------------------------------------------------------
TEST_F(LongTextLabelTest, LongTextLabel_Ctor_AlignsCenterAndStoresWidth)
{
    // Arrange: fixture 打桩已完成，构造参数为默认无参形式
    QScopedPointer<LongTextLabel> subject;

    // Act: 构造被测对象（ctor 是被测方法）
    subject.reset(new LongTextLabel);

    // Assert: 构造后垂直居中对齐，且 m_width 记录了构造时宽度
    EXPECT_EQ(subject->alignment(), Qt::AlignVCenter);
    EXPECT_EQ(subject->m_width, subject->width());
}

// ---- paintEvent: 纯文本（B1=false 路径） ------------------------------------
TEST_F(LongTextLabelTest, PaintEvent_PlainText_NoHtmlPassthrough)
{
    // Arrange: 无 <a> 链接的普通文本；省略桩返回固定串
    obj->setText(QStringLiteral("plain text no link"));
    elided_return = QStringLiteral("EL");

    // Act: 触发一次绘制
    QPixmap pix = renderOnce(300, 30);

    // Assert: 无 HTML 命中时 dst 即省略后的文本；paintEvent 已执行
    EXPECT_EQ(captured_dst, QStringLiteral("EL"));
    EXPECT_GE(paint_count, 1);
    EXPECT_FALSE(pix.isNull());
}

// ---- paintEvent: 含完整 <a> 结构，B2/B3/B5=true 路径 ------------------------
TEST_F(LongTextLabelTest, PaintEvent_AnchorWithContent_RecomposesHtmlAndText)
{
    // Arrange: 带 <a href> 的链接文本；省略桩按无省略返回原始可见文本 "UOS"
    obj->setText(QStringLiteral("<a href='x'>UOS</a>"));
    elided_return = QStringLiteral("UOS");

    // Act: 触发一次绘制
    renderOnce(300, 30);

    // Assert: html 提取成功，OS="UOS" 被重新省略后拼回 html 前缀，即恢复原文
    EXPECT_EQ(captured_dst, QStringLiteral("<a href='x'>UOS</a>"));
    EXPECT_GE(paint_count, 1);
}

// ---- paintEvent: 含 <a> 但宽度不足，可见文本被省略 --------------------------
TEST_F(LongTextLabelTest, PaintEvent_AnchorElided_KeepsHtmlPrefixWithElidedText)
{
    // Arrange: 链接文本；省略桩返回 "U…" 模拟宽度不足
    obj->setText(QStringLiteral("<a href='x'>UOS</a>"));
    elided_return = QStringLiteral("U…");

    // Act: 触发一次绘制
    renderOnce(20, 30);

    // Assert: dst = html 前缀 + 省略后的可见文本
    EXPECT_EQ(captured_dst, QStringLiteral("<a href='x'>UOS</a>U…"));
    EXPECT_GE(paint_count, 1);
}

// ---- paintEvent: 有 "</a>" 但无内部 <...> 结构，B2=false 路径 ---------------
TEST_F(LongTextLabelTest, PaintEvent_AnchorWithoutInnerTag_NoOsExtracted)
{
    // Arrange: 仅含 "</a>" 结尾、无 "<...>...</a>" 完整结构的文本
    obj->setText(QStringLiteral("see docs</a>here"));
    elided_return = QStringLiteral("EL");

    // Act: 触发一次绘制
    renderOnce(300, 30);

    // Assert: B1 命中但 B2 不命中，不进入 OS 提取分支，dst 即省略文本
    EXPECT_EQ(captured_dst, QStringLiteral("EL"));
    EXPECT_GE(paint_count, 1);
}
