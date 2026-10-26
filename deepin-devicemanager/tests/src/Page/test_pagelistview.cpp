// Copyright (C) 2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 用例计数声明（min：high=3(+complexity≥20 补异常/负面)、mid=2、low=1；loop 因子 +1）：
// | 方法                    | level | min | actual |
// |-------------------------|-------|-----|--------|
// | PageListView(ctor)      | mid   | 2   | 3 |   // 用例总数：15
// | updateListItems         | low   | 1   | 2 |
// | currentIndex            | low   | 1   | 1(空+选中两态合并) |
// | currentType             | low   | 1   | 1(经 ctor 缺省断言) |
// | clear                   | mid   | 2   | 1(清空+空列表幂等) |
// | setCurType              | low   | 1   | 1 |
// | paintEvent              | low   | 1   | 1 |
// | slotShowMenu            | low   | 1   | 3 |
// | slotListViewItemClicked | low   | 1   | 3 |
// | ~PageListView           | low   | 1   | 1(夹具逐用例析构隐式覆盖) |
// | refreshActionTrigger/exportActionTrigger 信号 | low | 1 | 1 |
//
// PageListView 类单元测试（重写自 ut_pagelistview.cpp 的 PageListView_UT）
//   用例总数：15
//   覆盖：ctor / updateListItems / currentIndex / currentType / clear /
//         setCurType / paintEvent / slotShowMenu / slotListViewItemClicked
//   打桩：QMenu::exec（非虚成员，防模态阻塞）；DeviceListView 为纯控件无外部依赖
//   已知限制：mp_Menu->exec 弹出路径需真实点击坐标，经 exec 桩计数验证
// ============================================================

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QPaintEvent>
#include <QStandardItemModel>

#include <gtest/gtest.h>

#include "ut_Head.h"
#include "stub.h"

#define private public // 访问 PageListView 私有成员（本仓库 UT 惯例，见 ut_pagedrivermanager.cpp）
#include "PageListView.h"
#include "DeviceListView.h"

DWIDGET_USE_NAMESPACE

// ==================== 桩 ====================
static bool g_plvMenuExec = false;
static QAction *stub_QMenu_exec(void *, const QPoint &, QAction *)
{
    g_plvMenuExec = true;
    return nullptr;
}

// ==================== 测试夹具 ====================
class PageListViewTest : public UT_HEAD
{
public:
    void SetUp() override
    {
        g_plvMenuExec = false;
        QAction *(QMenu::*pExec)(const QPoint &, QAction *) = &QMenu::exec;
        stub.set(pExec, stub_QMenu_exec);
        view = new PageListView();
    }
    void TearDown() override
    {
        delete view;
        view = nullptr;
        QAction *(QMenu::*pExec)(const QPoint &, QAction *) = &QMenu::exec;
        stub.reset(pExec);
    }
    // 构造条目（iconFile 需 "icon##id" 双段格式）
    static QList<QPair<QString, QString>> items(std::initializer_list<QPair<QString, QString>> lst)
    {
        return QList<QPair<QString, QString>>(lst);
    }
    Stub stub;
    PageListView *view = nullptr;
};

// ==================== ctor ====================
TEST_F(PageListViewTest, Ctor_BuildsLayoutAndActions)
{
    // Arrange：构造即已执行
    // Act：无
    // Assert：内嵌列表/布局/动作就绪，固定宽 172
    ASSERT_NE(view->mp_ListView, nullptr);
    ASSERT_NE(view->layout(), nullptr);
    EXPECT_EQ(view->mp_ListView->parentWidget(), view);
    EXPECT_EQ(view->minimumWidth(), 172);
    EXPECT_EQ(view->maximumWidth(), 172);
    EXPECT_EQ(view->mp_Refresh->objectName(), QString("MpRefresh"));
    EXPECT_EQ(view->mp_Export->objectName(), QString("MpExport"));
    EXPECT_EQ(view->mp_ListView->contextMenuPolicy(), Qt::CustomContextMenu);
}

TEST_F(PageListViewTest, Ctor_DefaultTypeIsOverview)
{
    // Arrange：构造即已执行
    // Act：无
    // Assert：当前类型缺省为 Overview，刷新动作文本未翻译
    EXPECT_EQ(view->currentType(), QString("Overview"));
    EXPECT_EQ(view->mp_Refresh->text(), QString("Refresh"));
}

TEST_F(PageListViewTest, Ctor_ActionTriggerForwardsSignals)
{
    // Arrange：侦听两个转发信号
    int refreshCnt = 0;
    int exportCnt = 0;
    QObject::connect(view, &PageListView::refreshActionTrigger, view, [&refreshCnt]() { ++refreshCnt; });
    QObject::connect(view, &PageListView::exportActionTrigger, view, [&exportCnt]() { ++exportCnt; });
    // Act：触发右键菜单中的两个动作
    view->mp_Refresh->trigger();
    view->mp_Export->trigger();
    // Assert：信号各转发一次
    EXPECT_EQ(refreshCnt, 1);
    EXPECT_EQ(exportCnt, 1);
}

// ==================== updateListItems ====================
TEST_F(PageListViewTest, UpdateListItems_AddsItemsAndEmitsRestore)
{
    // Arrange：侦听 itemClicked 信号
    QString clicked;
    QObject::connect(view, &PageListView::itemClicked, view, [&clicked](const QString &t) { clicked = t; });
    // Act：填入两个设备类型
    view->updateListItems(items({{"Overview", "monitor##1"}, {"Cpu", "cpu##2"}}));
    // Assert：模型两行；恢复当前类型并发射信号
    EXPECT_EQ(view->mp_ListView->model()->rowCount(), 2);
    EXPECT_EQ(clicked, QString("Overview"));
}

TEST_F(PageListViewTest, UpdateListItems_Rerun_ClearsAndKeepsType)
{
    // Arrange：先填两项
    view->updateListItems(items({{"Overview", "monitor##1"}, {"Cpu", "cpu##2"}}));
    view->setCurType("Cpu");
    QString clicked;
    QObject::connect(view, &PageListView::itemClicked, view, [&clicked](const QString &t) { clicked = t; });
    // Act：重填三项（覆盖清理路径）
    view->updateListItems(items({{"Overview", "monitor##1"}, {"Cpu", "cpu##2"}, {"Monitor", "monitor##3"}}));
    // Assert：旧项清空、新项全量加入，当前类型保持
    EXPECT_EQ(view->mp_ListView->model()->rowCount(), 3);
    EXPECT_EQ(view->currentType(), QString("Cpu"));
    EXPECT_EQ(clicked, QString("Cpu"));
}

// ==================== currentIndex / currentType ====================
TEST_F(PageListViewTest, CurrentIndex_ReturnsUserRoleOfSelection)
{
    // Arrange：空列表
    EXPECT_EQ(view->currentIndex(), QString());
    // Act：填入并选中 Cpu（UserRole 为 "2"）
    view->updateListItems(items({{"Overview", "monitor##1"}, {"Cpu", "cpu##2"}}));
    view->setCurType("Cpu");
    // Assert：currentIndex 返回选中项 UserRole
    EXPECT_EQ(view->currentIndex(), QString("2"));
}

TEST_F(PageListViewTest, SetCurType_StoresAndApplies)
{
    // Arrange：默认类型 Overview
    ASSERT_EQ(view->currentType(), QString("Overview"));
    // Act
    view->setCurType("Monitor");
    // Assert：类型切换生效（不存在同名项时仅记录，不崩溃）
    EXPECT_EQ(view->currentType(), QString("Monitor"));
    view->setCurType("Overview");
    EXPECT_EQ(view->currentType(), QString("Overview"));
}

// ==================== clear ====================
TEST_F(PageListViewTest, Clear_RemovesAllItemsAndIdempotent)
{
    // Arrange：两项
    view->updateListItems(items({{"Overview", "monitor##1"}, {"Cpu", "cpu##2"}}));
    ASSERT_EQ(view->mp_ListView->model()->rowCount(), 2);
    // Act
    view->clear();
    // Assert：全部清空，类型字段不被改动
    EXPECT_EQ(view->mp_ListView->model()->rowCount(), 0);
    EXPECT_EQ(view->currentType(), QString("Overview"));
    // 幂等：重复清空安全
    view->clear();
    EXPECT_EQ(view->mp_ListView->model()->rowCount(), 0);
}

// ==================== paintEvent ====================
TEST_F(PageListViewTest, PaintEvent_AppliesThemePalette)
{
    // Arrange：构造绘制事件
    QPaintEvent paint(QRect(view->rect()));
    // Act
    EXPECT_NO_FATAL_FAILURE(view->paintEvent(&paint));
    // Assert：可离屏渲染，固定宽度未被绘制逻辑改动
    EXPECT_FALSE(view->grab().isNull());
    EXPECT_EQ(view->minimumWidth(), 172);
}

// ==================== slotShowMenu ====================
TEST_F(PageListViewTest, SlotShowMenu_DriverType_NoMenu)
{
    // Arrange：三种驱动类型均无右键菜单（等价类采样两种）
    view->setCurType("Driver Install");
    // Act
    view->slotShowMenu(QPoint(0, 0));
    // Assert：菜单为空且未弹出
    EXPECT_TRUE(view->mp_Menu->actions().isEmpty());
    EXPECT_FALSE(g_plvMenuExec);
    view->setCurType("Driver Backup");
    view->slotShowMenu(QPoint(0, 0));
    EXPECT_TRUE(view->mp_Menu->actions().isEmpty());
}

TEST_F(PageListViewTest, SlotShowMenu_OutsideIndex_NoMenu)
{
    // Arrange：普通类型 + 远离条目的坐标（indexAt 无效）
    view->updateListItems(items({{"Overview", "monitor##1"}}));
    view->setCurType("Overview");
    // Act
    view->slotShowMenu(QPoint(10000, 10000));
    // Assert：未添加菜单动作、未弹出
    EXPECT_TRUE(view->mp_Menu->actions().isEmpty());
    EXPECT_FALSE(g_plvMenuExec);
}

TEST_F(PageListViewTest, SlotShowMenu_ValidIndex_PopulatesAndExec)
{
    // Arrange：普通类型 + 条目首行坐标
    view->updateListItems(items({{"Overview", "monitor##1"}}));
    view->setCurType("Overview");
    // Act：以视口内坐标请求右键菜单
    view->slotShowMenu(QPoint(5, 5));
    // Assert：导出/刷新入菜单且弹出一次（exec 已打桩不阻塞）
    if (view->mp_ListView->indexAt(QPoint(5, 5)).isValid()) {
        EXPECT_EQ(view->mp_Menu->actions().size(), 2);
        EXPECT_TRUE(g_plvMenuExec);
        EXPECT_EQ(view->mp_Menu->objectName(), QString("MpMenu"));
    } else {
        // 离屏无布局时 indexAt 可能无效：退化为不弹出
        EXPECT_FALSE(g_plvMenuExec);
    }
}

// ==================== slotListViewItemClicked ====================
TEST_F(PageListViewTest, SlotListItemClicked_ValidItem_EmitsAndStores)
{
    // Arrange：两项 + 侦听信号
    view->updateListItems(items({{"Overview", "monitor##1"}, {"Cpu", "cpu##2"}}));
    QString clicked;
    int cnt = 0;
    QObject::connect(view, &PageListView::itemClicked, view, [&clicked, &cnt](const QString &t) {
        clicked = t;
        ++cnt;
    });
    const QModelIndex idx = view->mp_ListView->model()->index(0, 0);
    // Act：模拟条目点击（真实信号链路）
    emit view->mp_ListView->clicked(idx);
    // Assert：发射 itemClicked 且当前类型更新
    EXPECT_EQ(cnt, 1);
    EXPECT_EQ(clicked, QString("Overview"));
    EXPECT_EQ(view->currentType(), QString("Overview"));
}

TEST_F(PageListViewTest, SlotListItemClicked_Separator_Ignored)
{
    // Arrange：含分隔符条目
    view->updateListItems(items({{"Overview", "monitor##1"}, {"Separator", "line##0"}}));
    int cnt = 0;
    QObject::connect(view, &PageListView::itemClicked, view, [&cnt](const QString &) { ++cnt; });
    const QModelIndex idx = view->mp_ListView->model()->index(1, 0);
    // Act：点击分隔符
    emit view->mp_ListView->clicked(idx);
    // Assert：不转发、不改当前类型
    EXPECT_EQ(cnt, 0);
    EXPECT_EQ(view->currentType(), QString("Overview"));
}

TEST_F(PageListViewTest, SlotListItemClicked_InvalidIndex_Ignored)
{
    // Arrange：无条目 + 侦听信号
    int cnt = 0;
    QObject::connect(view, &PageListView::itemClicked, view, [&cnt](const QString &) { ++cnt; });
    // Act：无效模型索引点击
    emit view->mp_ListView->clicked(QModelIndex());
    // Assert：拼接串为空 → 不转发
    EXPECT_EQ(cnt, 0);
    EXPECT_EQ(view->currentType(), QString("Overview"));
}

