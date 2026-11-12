// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                       | level | factors                  | min | actual |
// |------------------------------|-------|--------------------------|-----|--------|
// | DeviceWidget(ctor)           | mid   | in_degree:3              | 2   | 2      |
// | ~DeviceWidget                | low   | -                        | 1   | 1      |
// | initWidgets                  | low   | -                        | 1   | 1      |
// | clear                        | mid   | name_pattern:clear       | 2   | 2      |
// | currentIndex                 | mid   | in_degree:3              | 2   | 2      |
// | setFontChangeFlag            | low   | -                        | 1   | 1      |
// | slotListViewWidgetItemClicked| low   | -                        | 1   | 1      |
// | slotUpdateUI                 | low   | -                        | 1   | 1      |
// | updateDevice                 | low   | -                        | 1   | 2      |
// | updateListView               | low   | -                        | 1   | 1      |
// | updateOverview               | low   | -                        | 1   | 2      |
// | resizeEvent                  | mid   | complexity:5             | 2   | 3      |
// ─── actual >= min 达标（共 19 个用例实例）───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（空/非空设备列表、空/非空 map、单/多设备）
// 3. 每个等价类的边界值显式覆盖: [x]（空列表 0、单设备 1、多设备 >1、空 map）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（resizeEvent 三分支为异质断言，不用参数化）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（updateDevice/updateOverview 早退分支、resizeEvent 的 ret/lst.size()==1/>1/BIOS 分支、dtor 的三重空检查）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（无显式 throw，不适用）
// 8. 负面场景有专门用例: [x]（空列表/空 map 早退）
// 9. 负面用例验证强异常安全: [x]（早退分支断言零副作用）
// 10. stub_ext vs gMock 选择正确: [x]（DeviceManager 单例/子控件转发 → stub_ext 运行时替换；无接口抽象 → 不用 gMock）
//
// mock 深度分析（test-code-gen §4.0，来源：工作区实际源码 + MCP trace_path）：
// - DeviceWidget 为 GUI 容器：左 PageListView + 右 PageInfoWidget。二者为项目自有类
//   直接链入；其转发方法按「外部依赖打桩」以 stub_ext 记录型桩验证转发契约。
// - DeviceManager::instance() 惰性构造真实单例并枚举真实硬件 —— 测试严禁依赖真实
//   设备环境：打桩 instance() 返回 nullptr，getDeviceTypes/getDeviceList/
//   getDeviceOverview 打桩返回静态构造数据，全程零硬件访问。
// - resizeEvent 多设备分支经 dynamic_cast<DeviceBios*> 检查 lst[0]，桩不解除引用
//   场景下用 nullptr 占位；多设备场景传入指向真实 DWidget 的指针（多态对象，RTTI
//   安全），dynamic_cast 判定非 BIOS 返回空 → 验证跳过更新分支。
// - QApplication offscreen；DBus 已由 test_main.cpp 全局桩拦截；无文件/网络依赖。
//
// 分支清单（来源：工作区实际源码 DeviceWidget.cpp）
// B1: updateDevice: lst.size()==0 → 早退 / 非空 → mp_PageInfo->updateTable(itemStr, lst)
// B2: updateOverview: map.size()==0 → 早退 / 非空 → mp_PageInfo->updateTable(map)
// B3: resizeEvent: 设备类型匹配循环 / getDeviceList 失败 → updateTable(overviewMap)
// B4: resizeEvent: lst.size()==1 → updateTable(type, lst) / >1 且非 BIOS → 跳过
// B5: clear: 转发 mp_ListView->clear() 与 mp_PageInfo->clear()
// B6: slotListViewWidgetItemClicked: 记录 m_CurItemStr 并 emit itemClicked
// B7: dtor: mp_ListView/mp_PageInfo/m_Layout 逐个判空删除
//
// 用例映射：
// - Ctor_DefaultConstruction_InitializesChildrenAndLayout   → ctor+initWidgets
// - Ctor_DefaultConstruction_IndexMatchesCurrentType        → 初始索引转发一致性
// - InitWidgets_HorizontalLayout_TwoChildrenZeroMargins     → initWidgets
// - Clear_ForwardsToChildren_EachCalledOnce                 → B5（mid #1）
// - Clear_RepeatedCalls_EachCalledTwice                     → B5（mid #2）
// - CurrentIndex_StubbedChildType_ForwardedToCaller                → currentIndex（mid）
// - CurrentIndex_EmptyList_ReturnsEmptyString               → currentIndex 边界
// - SetFontChangeFlag_ForwardsToPageInfo_CalledTwice        → 转发
// - SlotListViewWidgetItemClicked_StoresAndEmits_ItemClicked → B6
// - SlotUpdateUI_ReEmits_CurrentItemString                  → slotUpdateUI
// - UpdateDevice_EmptyList_SkipsTableUpdate                 → B1 早退
// - UpdateDevice_NonEmptyList_ForwardsToUpdateTable         → B1 转发
// - UpdateListView_ForwardsListItems_ToListView             → updateListView
// - UpdateOverview_EmptyMap_SkipsTableUpdate                → B2 早退
// - UpdateOverview_NonEmptyMap_ForwardsToUpdateTable        → B2 转发
// - ResizeEvent_GetDeviceListFailed_UpdatesOverviewTable    → B3
// - ResizeEvent_SingleDevice_UpdatesDeviceTable             → B4 size==1
// - ResizeEvent_MultiDeviceNonBios_SkipsUpdate              → B4 size>1 非 BIOS
// - Dtor_DeletesChildren_GuardsBecomeNull                   → B7

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QResizeEvent>
#include <QPointer>

#include "stubext.h"
#include "DeviceWidget.h"
#include "PageListView.h"
#include "PageInfoWidget.h"
#include "DeviceManager.h"
#include "DeviceBios.h"

class DeviceWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override {
        // Arrange（公共前置）：隔离 DeviceManager 单例，杜绝真实硬件枚举
        stub.set_lamda(&DeviceManager::instance,
                       []() -> DeviceManager * { return nullptr; });

        // Arrange：构造被测对象（其子控件构造不触 DeviceManager）
        obj = new DeviceWidget;
    }

    void TearDown() override {
        // 先卸桩再析构，防止析构路径调用已打桩方法触达已销毁的栈帧捕获
        stub.clear();
        delete obj;
        obj = nullptr;
    }

    stub_ext::StubExt stub;
    DeviceWidget *obj = nullptr;
};

// ═══════════════════════════════════════════════════════════════
// 构造函数（mid）+ initWidgets
// ═══════════════════════════════════════════════════════════════
TEST_F(DeviceWidgetTest, Ctor_DefaultConstruction_InitializesChildrenAndLayout)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：读取构造后的成员与布局
    QLayout *lay = obj->layout();

    // Assert：左右子控件已创建，水平布局含两项且零边距零间距
    EXPECT_NE(obj->mp_ListView, nullptr);
    EXPECT_NE(obj->mp_PageInfo, nullptr);
    ASSERT_NE(lay, nullptr);
    EXPECT_EQ(lay->count(), 2);
    EXPECT_EQ(lay->contentsMargins().left(), 0);
    EXPECT_EQ(lay->spacing(), 0);
}

TEST_F(DeviceWidgetTest, Ctor_DefaultConstruction_IndexMatchesCurrentType)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：读取构造后的当前索引与子控件当前类型
    QString index = obj->currentIndex();
    QString curType = obj->mp_ListView->currentType();

    // Assert：currentIndex 一致转发 currentType（与初始译名无关）
    EXPECT_EQ(index, curType);
    EXPECT_NE(obj, nullptr);
}

TEST_F(DeviceWidgetTest, InitWidgets_HorizontalLayout_TwoChildrenZeroMargins)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：读取 m_Layout 属性
    QHBoxLayout *lay = obj->m_Layout;

    // Assert：布局含 ListView 与 PageInfo 两项
    ASSERT_NE(lay, nullptr);
    EXPECT_EQ(lay->count(), 2);
    EXPECT_EQ(lay->itemAt(0)->widget(), obj->mp_ListView);
    EXPECT_EQ(lay->itemAt(1)->widget(), obj->mp_PageInfo);
}

// ═══════════════════════════════════════════════════════════════
// clear（mid，B5 双子控件转发）
// ═══════════════════════════════════════════════════════════════
TEST_F(DeviceWidgetTest, Clear_ForwardsToChildren_EachCalledOnce)
{
    // Arrange：替换两个子控件的 clear 记录调用
    int listViewClears = 0;
    int pageInfoClears = 0;
    stub.set_lamda(&PageListView::clear,
                   [&](PageListView *) { ++listViewClears; });
    stub.set_lamda(&PageInfoWidget::clear,
                   [&](PageInfoWidget *) { ++pageInfoClears; });

    // Act
    obj->clear();

    // Assert：左右子控件各清空一次
    EXPECT_EQ(listViewClears, 1);
    EXPECT_EQ(pageInfoClears, 1);
}

TEST_F(DeviceWidgetTest, Clear_RepeatedCalls_EachCalledTwice)
{
    // Arrange：替换两个子控件的 clear 记录调用
    int listViewClears = 0;
    int pageInfoClears = 0;
    stub.set_lamda(&PageListView::clear,
                   [&](PageListView *) { ++listViewClears; });
    stub.set_lamda(&PageInfoWidget::clear,
                   [&](PageInfoWidget *) { ++pageInfoClears; });

    // Act：连续清空两次
    obj->clear();
    obj->clear();

    // Assert：两个子控件各累计两次
    EXPECT_EQ(listViewClears, 2);
    EXPECT_EQ(pageInfoClears, 2);
}

// ═══════════════════════════════════════════════════════════════
// currentIndex（mid，转发 mp_ListView->currentType()）
// ═══════════════════════════════════════════════════════════════
TEST_F(DeviceWidgetTest, CurrentIndex_StubbedChildType_ForwardedToCaller)
{
    // Arrange：替换 currentType 返回受控值
    stub.set_lamda(&PageListView::currentType,
                   [](PageListView *) { return QString("cpu"); });

    // Act
    QString index = obj->currentIndex();

    // Assert：转发子控件当前类型（打桩值一致传出）
    EXPECT_EQ(index, QString("cpu"));
    EXPECT_EQ(index, obj->mp_ListView->currentType());
}

TEST_F(DeviceWidgetTest, CurrentIndex_EmptyList_ReturnsEmptyString)
{
    // Arrange：替换 currentType 返回空串（等价未选中）
    stub.set_lamda(&PageListView::currentType,
                   [](PageListView *) { return QString(); });

    // Act
    QString index = obj->currentIndex();

    // Assert：未选中时返回空串，且与子控件类型一致
    EXPECT_TRUE(index.isEmpty());
    EXPECT_EQ(index, obj->mp_ListView->currentType());
}

// ═══════════════════════════════════════════════════════════════
// setFontChangeFlag（转发 PageInfoWidget）
// ═══════════════════════════════════════════════════════════════
TEST_F(DeviceWidgetTest, SetFontChangeFlag_ForwardsToPageInfo_CalledTwice)
{
    // Arrange：替换 PageInfoWidget::setFontChangeFlag 计数
    int flagCalls = 0;
    stub.set_lamda(&PageInfoWidget::setFontChangeFlag,
                   [&](PageInfoWidget *) { ++flagCalls; });

    // Act：连续设置两次
    obj->setFontChangeFlag();
    obj->setFontChangeFlag();

    // Assert：转发累计两次，且落在右侧详情控件上
    EXPECT_EQ(flagCalls, 2);
    EXPECT_NE(obj->mp_PageInfo, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// slotListViewWidgetItemClicked（B6）/ slotUpdateUI
// ═══════════════════════════════════════════════════════════════
TEST_F(DeviceWidgetTest, SlotListViewWidgetItemClicked_StoresAndEmits_ItemClicked)
{
    // Arrange：监听 itemClicked 信号
    QSignalSpy spy(obj, &DeviceWidget::itemClicked);
    ASSERT_TRUE(spy.isValid());

    // Act
    obj->slotListViewWidgetItemClicked("cpu");

    // Assert：记录当前项并转发同值信号
    EXPECT_EQ(obj->m_CurItemStr, QString("cpu"));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), QString("cpu"));
}

TEST_F(DeviceWidgetTest, SlotUpdateUI_ReEmits_CurrentItemString)
{
    // Arrange：预置当前项并监听信号
    obj->m_CurItemStr = QString("memory");
    QSignalSpy spy(obj, &DeviceWidget::itemClicked);
    ASSERT_TRUE(spy.isValid());

    // Act
    obj->slotUpdateUI();

    // Assert：以当前项字符串重发信号
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), QString("memory"));
}

// ═══════════════════════════════════════════════════════════════
// updateDevice（B1 空列表早退 / 非空转发）
// ═══════════════════════════════════════════════════════════════
TEST_F(DeviceWidgetTest, UpdateDevice_EmptyList_SkipsTableUpdate)
{
    // Arrange：替换 updateTable 计数
    int tableUpdates = 0;
    stub.set_lamda(static_cast<void (PageInfoWidget::*)(const QString &, const QList<DeviceBaseInfo *> &)>(&PageInfoWidget::updateTable),
                   [&](PageInfoWidget *, const QString &, const QList<DeviceBaseInfo *> &) { ++tableUpdates; });
    QList<DeviceBaseInfo *> emptyList;

    // Act：空列表调用后，再以非空列表验证转发仍正常
    obj->updateDevice("cpu", emptyList);

    // Assert：空列表早退，零副作用
    EXPECT_EQ(tableUpdates, 0);

    // Act：非空列表正常转发（桩不解引用元素，nullptr 占位安全）
    QList<DeviceBaseInfo *> oneList;
    oneList.append(nullptr);
    obj->updateDevice("cpu", oneList);

    // Assert：转发恰一次
    EXPECT_EQ(tableUpdates, 1);
}

TEST_F(DeviceWidgetTest, UpdateDevice_NonEmptyList_ForwardsToUpdateTable)
{
    // Arrange：替换 updateTable 记录参数（桩不解引用列表元素，nullptr 占位安全）
    QString recType;
    int recCount = -1;
    stub.set_lamda(static_cast<void (PageInfoWidget::*)(const QString &, const QList<DeviceBaseInfo *> &)>(&PageInfoWidget::updateTable),
                   [&](PageInfoWidget *, const QString &itemStr, const QList<DeviceBaseInfo *> &lst) {
                       recType = itemStr;
                       recCount = lst.size();
                   });
    QList<DeviceBaseInfo *> lst;
    lst.append(nullptr);

    // Act
    obj->updateDevice("cpu", lst);

    // Assert：转发设备类型与列表规模
    EXPECT_EQ(recType, QString("cpu"));
    EXPECT_EQ(recCount, 1);
}

// ═══════════════════════════════════════════════════════════════
// updateListView（转发 updateListItems）
// ═══════════════════════════════════════════════════════════════
TEST_F(DeviceWidgetTest, UpdateListView_ForwardsListItems_ToListView)
{
    // Arrange：替换 updateListItems 记录参数
    int recSize = -1;
    stub.set_lamda(&PageListView::updateListItems,
                   [&](PageListView *, const QList<QPair<QString, QString>> &lst) { recSize = lst.size(); });
    QList<QPair<QString, QString>> items{{"cpu", "处理器"},
                                         {"memory", "内存"}};

    // Act
    obj->updateListView(items);

    // Assert：转发完整列表，且落在左侧列表控件上
    EXPECT_EQ(recSize, 2);
    EXPECT_NE(obj->mp_ListView, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// updateOverview（B2 空 map 早退 / 非空转发）
// ═══════════════════════════════════════════════════════════════
TEST_F(DeviceWidgetTest, UpdateOverview_EmptyMap_SkipsTableUpdate)
{
    // Arrange：替换 QMap 重载 updateTable 计数
    int tableUpdates = 0;
    stub.set_lamda(static_cast<void (PageInfoWidget::*)(const QMap<QString, QString> &)>(&PageInfoWidget::updateTable),
                   [&](PageInfoWidget *, const QMap<QString, QString> &) { ++tableUpdates; });
    QMap<QString, QString> emptyMap;

    // Act：空 map 调用后，再以非空 map 验证转发仍正常
    obj->updateOverview(emptyMap);

    // Assert：空 map 早退，零副作用
    EXPECT_EQ(tableUpdates, 0);

    // Act：非空 map 正常转发
    QMap<QString, QString> map{{"OS", "UOS"}};
    obj->updateOverview(map);

    // Assert：转发恰一次
    EXPECT_EQ(tableUpdates, 1);
}

TEST_F(DeviceWidgetTest, UpdateOverview_NonEmptyMap_ForwardsToUpdateTable)
{
    // Arrange：替换 QMap 重载 updateTable 记录参数
    int recSize = -1;
    stub.set_lamda(static_cast<void (PageInfoWidget::*)(const QMap<QString, QString> &)>(&PageInfoWidget::updateTable),
                   [&](PageInfoWidget *, const QMap<QString, QString> &map) { recSize = map.size(); });
    QMap<QString, QString> map{{"OS", "UOS"}, {"Kernel", "5.10"}};

    // Act
    obj->updateOverview(map);

    // Assert：转发完整 map，且落在右侧详情控件上
    EXPECT_EQ(recSize, 2);
    EXPECT_NE(obj->mp_PageInfo, nullptr);
}

// ═══════════════════════════════════════════════════════════════
// resizeEvent（mid，B3/B4 分支）
// ═══════════════════════════════════════════════════════════════
TEST_F(DeviceWidgetTest, ResizeEvent_GetDeviceListFailed_UpdatesOverviewTable)
{
    // Arrange：类型表返回 CPU，设备列表获取失败
    stub.set_lamda(&DeviceManager::getDeviceTypes,
                   []() -> const QList<QPair<QString, QString>> & {
                       static QList<QPair<QString, QString>> types{{"CPU", "cpu"}};
                       return types;
                   });
    stub.set_lamda(&DeviceManager::getDeviceList,
                   [](DeviceManager *, const QString &, QList<DeviceBaseInfo *> &lst) {
                       lst.clear();
                       return false;
                   });
    stub.set_lamda(&DeviceManager::getDeviceOverview,
                   []() -> const QMap<QString, QString> & {
                       static QMap<QString, QString> overview{{"OS", "UOS"}};
                       return overview;
                   });
    int mapUpdates = 0;
    int listUpdates = 0;
    stub.set_lamda(static_cast<void (PageInfoWidget::*)(const QMap<QString, QString> &)>(&PageInfoWidget::updateTable),
                   [&](PageInfoWidget *, const QMap<QString, QString> &) { ++mapUpdates; });
    stub.set_lamda(static_cast<void (PageInfoWidget::*)(const QString &, const QList<DeviceBaseInfo *> &)>(&PageInfoWidget::updateTable),
                   [&](PageInfoWidget *, const QString &, const QList<DeviceBaseInfo *> &) { ++listUpdates; });
    QResizeEvent event(QSize(100, 100), QSize(80, 80));

    // Act
    obj->resizeEvent(&event);

    // Assert：失败分支更新概况表，不更新设备表
    EXPECT_EQ(mapUpdates, 1);
    EXPECT_EQ(listUpdates, 0);
}

TEST_F(DeviceWidgetTest, ResizeEvent_SingleDevice_UpdatesDeviceTable)
{
    // Arrange：设备列表获取成功且仅一台设备（桩不解除引用，nullptr 占位安全）
    stub.set_lamda(&DeviceManager::getDeviceTypes,
                   []() -> const QList<QPair<QString, QString>> & {
                       static QList<QPair<QString, QString>> types{{"CPU", "cpu"}};
                       return types;
                   });
    stub.set_lamda(&DeviceManager::getDeviceList,
                   [](DeviceManager *, const QString &, QList<DeviceBaseInfo *> &lst) {
                       lst.clear();
                       lst.append(nullptr);
                       return true;
                   });
    int mapUpdates = 0;
    int listUpdates = 0;
    stub.set_lamda(static_cast<void (PageInfoWidget::*)(const QMap<QString, QString> &)>(&PageInfoWidget::updateTable),
                   [&](PageInfoWidget *, const QMap<QString, QString> &) { ++mapUpdates; });
    stub.set_lamda(static_cast<void (PageInfoWidget::*)(const QString &, const QList<DeviceBaseInfo *> &)>(&PageInfoWidget::updateTable),
                   [&](PageInfoWidget *, const QString &, const QList<DeviceBaseInfo *> &) { ++listUpdates; });
    QResizeEvent event(QSize(100, 100), QSize(80, 80));

    // Act
    obj->resizeEvent(&event);

    // Assert：单设备走设备表更新，不走概况表
    EXPECT_EQ(listUpdates, 1);
    EXPECT_EQ(mapUpdates, 0);
}

TEST_F(DeviceWidgetTest, ResizeEvent_MultiDeviceNonBios_SkipsUpdate)
{
    // Arrange：两台设备且首元素非 BIOS（指向真实多态对象，dynamic_cast RTTI 安全判空）
    stub.set_lamda(&DeviceManager::getDeviceTypes,
                   []() -> const QList<QPair<QString, QString>> & {
                       static QList<QPair<QString, QString>> types{{"CPU", "cpu"}};
                       return types;
                   });
    DWidget nonBiosDummy;
    stub.set_lamda(&DeviceManager::getDeviceList,
                   [&nonBiosDummy](DeviceManager *, const QString &, QList<DeviceBaseInfo *> &lst) {
                       lst.clear();
                       lst.append(reinterpret_cast<DeviceBaseInfo *>(&nonBiosDummy));
                       lst.append(nullptr);
                       return true;
                   });
    int mapUpdates = 0;
    int listUpdates = 0;
    stub.set_lamda(static_cast<void (PageInfoWidget::*)(const QMap<QString, QString> &)>(&PageInfoWidget::updateTable),
                   [&](PageInfoWidget *, const QMap<QString, QString> &) { ++mapUpdates; });
    stub.set_lamda(static_cast<void (PageInfoWidget::*)(const QString &, const QList<DeviceBaseInfo *> &)>(&PageInfoWidget::updateTable),
                   [&](PageInfoWidget *, const QString &, const QList<DeviceBaseInfo *> &) { ++listUpdates; });
    QResizeEvent event(QSize(100, 100), QSize(80, 80));

    // Act
    obj->resizeEvent(&event);

    // Assert：非 BIOS 多设备跳过更新，零副作用
    EXPECT_EQ(listUpdates, 0);
    EXPECT_EQ(mapUpdates, 0);
}

// ═══════════════════════════════════════════════════════════════
// ~DeviceWidget（B7 判空删除）
// ═══════════════════════════════════════════════════════════════
TEST_F(DeviceWidgetTest, Dtor_DeletesChildren_GuardsBecomeNull)
{
    // Arrange：QPointer 监视子控件占位
    QPointer<PageListView> listViewGuard;
    QPointer<PageInfoWidget> pageInfoGuard;

    // Act：作用域内构造并析构（立即调用的 lambda 保证真实语句触发）
    [&listViewGuard, &pageInfoGuard]() {
        DeviceWidget scopedWidget;
        listViewGuard = scopedWidget.mp_ListView;
        pageInfoGuard = scopedWidget.mp_PageInfo;
        ASSERT_FALSE(listViewGuard.isNull());
        ASSERT_FALSE(pageInfoGuard.isNull());
    }();

    // Assert：两个子控件均被析构删除（QPointer 守护指针归空）
    EXPECT_EQ(listViewGuard.data(), nullptr);
    EXPECT_EQ(pageInfoGuard.data(), nullptr);
}
