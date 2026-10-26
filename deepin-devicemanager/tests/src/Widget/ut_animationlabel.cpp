// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 AnimationLabel 的 ctor/setImageList/setcurrentIcon/currentIcon。
//
// 用例计数声明表：
//
// | method | level | factors | min | actual |
// | --- | --- | --- | --- | --- |
// | AnimationLabel::AnimationLabel | middle | - | 1 | 1 |
// | AnimationLabel::setImageList | low | 首次/累积/空表 | 3 | 3 |
// | AnimationLabel::setcurrentIcon | low | 合法/越界 | 4 | 4 |
// | AnimationLabel::currentIcon | low | - | 1 | 1 |
//
// 通过 stub QIcon::fromTheme 隔离主题图标查找（不依赖真实图标包环境），
// 验证列表累积语义与索引边界检查。QApplication 由 tests/src/test_main.cpp
// 提供（offscreen 全局实例），此处不再重建。

#include "AnimationLabel.h"

#include <stubext.h>

#include <QIcon>
#include <QPixmap>
#include <QStringList>

#include <gtest/gtest.h>

// ---- 方法分支清单（源自 MCP get_code_snippet AnimationLabel::setcurrentIcon 24-36 行）----
// B1: iconIndex < 0 || iconIndex >= m_ImageList.size() → 越界提前 return
//     用例6 参数 {-1, 2, 99} 均走 true（含下界与上界两侧越界）；
//     用例5/7 走 false（0 与 size-1 合法下标）。
// （ctor/setImageList/currentIcon 为直线逻辑，无分支）

namespace {

class AnimationLabelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        fromtheme_count = 0;
        last_theme_name.clear();
        fallback_pix = QPixmap(10, 10);
        fallback_pix.fill(Qt::red);
        // stub 主题图标查找：返回固定像素图并记录请求名
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme),
                       [this](const QString &name) -> QIcon {
                           __DBG_STUB_INVOKE__
                           ++fromtheme_count;
                           last_theme_name = name;
                           return QIcon(fallback_pix);
                       });
        // Act: 构造被测对象（ctor 也是被测方法）
        obj = new AnimationLabel;
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    AnimationLabel *obj = nullptr;
    QPixmap fallback_pix;
    int fromtheme_count = 0;
    QString last_theme_name;
};

// 越界参数断言结构一致 → 参数化（负下界、上界 size、远超上界）
class AnimationLabelOutOfRangeTest : public AnimationLabelTest,
                                     public ::testing::WithParamInterface<int>
{
};

}  // namespace

// ---- Constructor -----------------------------------------------------------
TEST_F(AnimationLabelTest, AnimationLabel_Ctor_IndexStartsAtMinusOne)
{
    // Arrange: 打桩在 fixture 完成，期望初始索引为 -1
    QScopedPointer<AnimationLabel> subject;

    // Act: 构造被测对象（ctor 是被测方法）
    subject.reset(new AnimationLabel);

    // Assert: 构造后索引为 -1，图片列表为空
    EXPECT_EQ(subject->currentIcon(), -1);
    EXPECT_TRUE(subject->m_ImageList.isEmpty());
}

// ---- setImageList: 首次设置 -------------------------------------------------
TEST_F(AnimationLabelTest, SetImageList_InitialCall_StoresItems)
{
    // Arrange: 期望的初始列表内容
    const QStringList expected = { QStringLiteral("a"), QStringLiteral("b") };

    // Act
    obj->setImageList(QStringList() << QStringLiteral("a") << QStringLiteral("b"));

    // Assert: 列表按序保存
    EXPECT_EQ(obj->m_ImageList.size(), 2);
    EXPECT_EQ(obj->m_ImageList.at(0), QStringLiteral("a"));
    EXPECT_EQ(obj->m_ImageList.at(1), QStringLiteral("b"));
}

// ---- setImageList: 二次调用为累积而非替换 -----------------------------------
TEST_F(AnimationLabelTest, SetImageList_SecondCall_AccumulatesInsteadOfReplace)
{
    // Arrange
    obj->setImageList(QStringList() << QStringLiteral("a") << QStringLiteral("b"));

    // Act
    obj->setImageList(QStringList() << QStringLiteral("c"));

    // Assert: "<<" 追加语义，列表共 3 项
    EXPECT_EQ(obj->m_ImageList.size(), 3);
    EXPECT_EQ(obj->m_ImageList.at(2), QStringLiteral("c"));
}

// ---- setImageList: 空列表边界 ----------------------------------------------
TEST_F(AnimationLabelTest, SetImageList_EmptyList_ListRemainsEmpty)
{
    // Arrange: 构造后列表为空，记录当前长度作为基线
    const int size_before = obj->m_ImageList.size();

    // Act
    obj->setImageList(QStringList());

    // Assert: 空列表不改变内容；随后仍可正常累积
    EXPECT_EQ(size_before, 0);
    EXPECT_EQ(obj->m_ImageList.size(), size_before);
    obj->setImageList(QStringList() << QStringLiteral("x"));
    EXPECT_EQ(obj->m_ImageList.size(), 1);
}

// ---- setcurrentIcon: 合法下标 ----------------------------------------------
TEST_F(AnimationLabelTest, SetcurrentIcon_ValidIndex_UpdatesPixmapAndIndex)
{
    // Arrange
    obj->setImageList(QStringList() << QStringLiteral("icon-a") << QStringLiteral("icon-b"));

    // Act
    obj->setcurrentIcon(1);

    // Assert: 索引更新、像素图来自桩图标、主题查找使用了对应表项
    EXPECT_EQ(obj->currentIcon(), 1);
    EXPECT_FALSE(obj->pixmap().isNull());
    EXPECT_EQ(last_theme_name, QStringLiteral("icon-b"));
    EXPECT_EQ(fromtheme_count, 1);
}

// ---- setcurrentIcon: 越界下标（B1=true） ------------------------------------
TEST_P(AnimationLabelOutOfRangeTest, SetcurrentIcon_OutOfRange_IndexUnchangedAndNoThemeLookup)
{
    // Arrange: 列表长度 2，索引尚未合法设置过（保持 -1）
    obj->setImageList(QStringList() << QStringLiteral("icon-a") << QStringLiteral("icon-b"));
    const int index = GetParam();

    // Act
    obj->setcurrentIcon(index);

    // Assert: 索引保持 -1，未触发主题图标查找（提前 return）
    EXPECT_EQ(obj->currentIcon(), -1);
    EXPECT_EQ(fromtheme_count, 0);
}

INSTANTIATE_TEST_SUITE_P(Boundaries, AnimationLabelOutOfRangeTest,
                         ::testing::Values(-1, 2, 99));

// ---- currentIcon: 返回最近一次合法设置 --------------------------------------
TEST_F(AnimationLabelTest, CurrentIcon_AfterSetCurrentIcon_ReturnsLatestIndex)
{
    // Arrange
    obj->setImageList(QStringList() << QStringLiteral("icon-a") << QStringLiteral("icon-b"));

    // Act: 依次设置合法下标 0、1
    obj->setcurrentIcon(0);
    EXPECT_EQ(obj->currentIcon(), 0);
    obj->setcurrentIcon(1);

    // Assert: 返回最新索引，两次合法设置各触发一次主题查找
    EXPECT_EQ(obj->currentIcon(), 1);
    EXPECT_EQ(fromtheme_count, 2);
}
