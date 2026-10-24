// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 测试对象：deepin-devicemanager/src/Page/PageDriverBackupInfo.cpp
// 依赖策略（dependency-tracer 决策矩阵）：出向依赖均为项目内部 Widget
// （DriverTableView/Driver*Item/DetectedStatusWidget），无外部 IO；
// 仅对 UI 头部件 setter 与 DriverStatusItem::setErrorMsg 做 stub（记录入参）。

// ── 分支清单（来源：get_code_snippet，PageDriverBackupInfo.cpp）──
// addDriverInfoToTableView(info, index)：
//   B1 debBackupVersion() != debVersion() → mp_ViewBackable 6 列整行
//      （勾选框按 ST_NOT_INSTALL、状态固定 ST_DRIVER_NOT_BACKUP、操作 BACKUP）
//                                                              → AddDriverInfoToTableView_Backable_RowAddedWithNotBackupStatus
//                                                              / AddDriverInfoToTableView_BothVersions_BothViewsGetRows
//   B2 !debBackupVersion().isEmpty() → mp_ViewBackedUp 3 列行   → AddDriverInfoToTableView_BackedUp_RowAddedToBackedUpView
//                                                              / AddDriverInfoToTableView_BothVersions_BothViewsGetRows
//   两分支皆不满足（双版本相等且备份版本为空）→ 无行落      → AddDriverInfoToTableView_NoBackupVersions_NoRowsAdded
// showTables(backableLength, backedupLength)：
//   B1 backedupLength != 0 → 备份完成区域可见                    → ShowTables_BackedUpOnly_NoBackupUICalled / _BothPresent_...
//   B2 backableLength != 0 → 可备份区域可见                      → ShowTables_BackableOnly_BackableUICalled
//   B3 backableLength == 0 → setNoBackupDriverUI(b, u)           → ShowTables_BothZero_NoBackupUICalled / _BackedUpOnly_...
//   B4 else → setBackableDriverUI(b, u)                          → ShowTables_BackableOnly_BackableUICalled / _BothPresent_...
//   边界：负数计数（!= 0 为真）                                  → ShowTables_NegativeCounts_WidgetsStillVisible
// updateItemStatus(index, status)：
//   B1 status == ST_DRIVER_BACKUP_FAILED → setErrorMsg("Backup Failed") → UpdateItemStatus_BackupFailed_SetsErrorMsg
//   B2 其他状态不写错误信息                                      → UpdateItemStatus_BackupSuccess_NoErrorMsg
// getCheckedDriverIndex(lst)：仅扫可备份视图                    → GetCheckedDriverIndex_EmptyTable_ReturnsEmptyList
//                                                              / GetCheckedDriverIndex_CheckedItems_ReturnsIndicesInOrder
// clearAllData()：双视图 clear + initTable                      → ClearAllData_AfterAdd_AllViewsResetToZero / _CalledTwice_...
// setCheckedCBDisnable()：委托可备份视图                        → SetCheckedCBDisnable_CheckedItem_DisabledAndWaiting
// setHeaderCbEnable(enable)：委托可备份视图表头勾选框           → SetHeaderCbEnable_TrueAndFalse_HeaderStateChanges
//   经 backupAll 信号触发同一逻辑                                → SetCheckedCBDisnable_ViaBackupAllSignal_CBsDisabled
// initUI()/initTable()：经构造函数与 clearAllData 触发          → InitUI_ViaConstructor_* / InitTable_ViaConstructor_*
// 构造函数：成员初始化 + 信号转发连接                            → PageDriverBackupInfo_DefaultConstruct_AllMembersReady
//                                                              / HeadWidget_AfterInit_ReturnsHeadWidgetPointer
//
// 用例计数声明（self-check-structural 验证此块，与 .ut/inventory.json 对齐）：
// | method                   | level | factors            | min | actual |
// |--------------------------|-------|--------------------|-----|--------|
// | PageDriverBackupInfo     | low   | ctor               | 1   | 1      |
// | headWidget               | low   | -                  | 1   | 1      |
// | initUI                   | mid   | lines:55           | 2   | 2      |
// | initTable                | mid   | in_degree:2        | 2   | 2      |
// | addDriverInfoToTableView | mid   | lines:70           | 2   | 4      |
// | clearAllData             | mid   | name_pattern:clear | 2   | 2      |
// | showTables               | low   | -                  | 1   | 5      |
// | updateItemStatus         | low   | -                  | 1   | 2      |
// | getCheckedDriverIndex    | low   | -                  | 1   | 2      |
// | setCheckedCBDisnable     | low   | -                  | 1   | 2      |
// | setHeaderCbEnable        | low   | -                  | 1   | 1      |
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（备份版本三态：空/等于当前/不同于当前）
// 3. 每个等价类的边界值显式覆盖: [x]（0/非 0、负数、空串边界）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（不适用：各版本组合断言目标不同，非同质输入）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（双 if 皆可绕过）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（不适用：无 throw 路径）
// 8. 负面场景有专门用例: [x]（无备份版本、空表勾选、双零计数）
// 9. 负面用例验证强异常安全: [x]（负面用例均断言行数/状态不变）
// 10. stub 选择: [x]（沿用工程 tests/src/stub.h + 局部 Stub 对象，析构自动还原，不调用 stub.clear()）

#include <gtest/gtest.h>

#include <QSignalSpy>

#include "stub.h"
#include <DScrollArea>
#include "DetectedStatusWidget.h"

#define private public
#include "PageDriverBackupInfo.h"
#include "PageDriverTableView.h"
#include "drivertableview.h"
#include "driveritem.h"
#include "BtnLabel.h"
#undef private

DWIDGET_USE_NAMESPACE

// ── stub 记录器与桩函数 ──
namespace {

struct UiCallRecord
{
    int calls = 0;
    int n1 = -1;
    int n2 = -1;
    QString str1;

    void reset() { *this = UiCallRecord(); }
};

static UiCallRecord g_noBackupRecord;  // DetectedStatusWidget::setNoBackupDriverUI
static UiCallRecord g_backableRecord;  // DetectedStatusWidget::setBackableDriverUI
static UiCallRecord g_errorRecord;     // DriverStatusItem::setErrorMsg

static void resetAllRecords()
{
    g_noBackupRecord.reset();
    g_backableRecord.reset();
    g_errorRecord.reset();
}

static void ut_setNoBackupDriverUI(DetectedStatusWidget *self, int backableSize, int backedupSize)
{
    Q_UNUSED(self);
    ++g_noBackupRecord.calls;
    g_noBackupRecord.n1 = backableSize;
    g_noBackupRecord.n2 = backedupSize;
}

static void ut_setBackableDriverUI(DetectedStatusWidget *self, int backableSize, int backedupSize)
{
    Q_UNUSED(self);
    ++g_backableRecord.calls;
    g_backableRecord.n1 = backableSize;
    g_backableRecord.n2 = backedupSize;
}

static void ut_statusSetErrorMsg(DriverStatusItem *self, const QString &msg)
{
    Q_UNUSED(self);
    ++g_errorRecord.calls;
    g_errorRecord.str1 = msg;
}

}  // namespace

// ── 测试数据与视图访问辅助 ──
namespace {

static DriverInfo *makeDriverInfo(const QString &name, Status st, const QString &debVersion,
                                  const QString &debBackupVersion)
{
    DriverInfo *info = new DriverInfo();
    info->m_Name = name;
    info->m_Type = DR_Camera;
    info->m_Version = "2.0.0";
    info->m_Size = "10MB";
    info->m_Status = st;
    info->m_DebVersion = debVersion;
    info->m_DebBackupVersion = debBackupVersion;
    return info;
}

static DriverTableView *innerView(PageDriverTableView *view)
{
    return view->mp_View;
}

static QWidget *rowWidget(PageDriverTableView *view, int row, int col)
{
    DriverTableView *dtv = innerView(view);
    return dtv->indexWidget(dtv->model()->index(row, col));
}

}  // namespace

class UT_PageDriverBackupInfo : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_page = new PageDriverBackupInfo;
    }

    void TearDown() override
    {
        delete m_page;
    }

    PageDriverBackupInfo *m_page = nullptr;
    Stub stub;
};

// 构造函数：成员初始化
TEST_F(UT_PageDriverBackupInfo, PageDriverBackupInfo_DefaultConstruct_AllMembersReady)
{
    // Arrange
    delete m_page;  // 释放 SetUp 实例，由 Act 在用例内构造

    // Act
    m_page = new PageDriverBackupInfo;

    // Assert
    ASSERT_NE(nullptr, m_page);
    EXPECT_NE(nullptr, m_page->mp_HeadWidget);
    EXPECT_NE(nullptr, m_page->mp_ViewBackable);
    EXPECT_NE(nullptr, m_page->mp_ViewBackedUp);
    EXPECT_NE(nullptr, m_page->mp_BackableDriverLabel);
    EXPECT_NE(nullptr, m_page->mp_BackedUpDriverLabel);
    EXPECT_NE(nullptr, m_page->mp_NotBackupWidget);
    EXPECT_NE(nullptr, m_page->mp_BackedUpWidget);
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewBackedUp->model()->rowCount());
}

// headWidget getter 返回内部头部件
TEST_F(UT_PageDriverBackupInfo, HeadWidget_AfterInit_ReturnsHeadWidgetPointer)
{
    // Arrange
    ASSERT_NE(nullptr, m_page);

    // Act
    DetectedStatusWidget *head = m_page->headWidget();

    // Assert
    EXPECT_EQ(m_page->mp_HeadWidget, head);
    EXPECT_NE(nullptr, head);
}

// initUI 经构造函数装配布局
TEST_F(UT_PageDriverBackupInfo, InitUI_ViaConstructor_LayoutInstalled)
{
    // Arrange
    ASSERT_NE(nullptr, m_page);

    // Act
    QLayout *layout = m_page->layout();

    // Assert
    EXPECT_NE(nullptr, layout);
    EXPECT_NE(nullptr, m_page->findChild<DScrollArea *>());
    EXPECT_EQ(0, m_page->lineWidth());
}

// initUI 装配后关键子部件挂到页面窗口树下
TEST_F(UT_PageDriverBackupInfo, InitUI_ViaConstructor_WidgetsParented)
{
    // Arrange
    ASSERT_NE(nullptr, m_page);

    // Act
    QWidget *headWindow = m_page->mp_HeadWidget->window();
    QWidget *viewWindow = m_page->mp_ViewBackable->window();

    // Assert
    EXPECT_EQ(m_page, headWindow);
    EXPECT_EQ(m_page, viewWindow);
    EXPECT_EQ(m_page, m_page->mp_NotBackupWidget->window());
}

// initTable 初始化表头、列宽与表头勾选框
TEST_F(UT_PageDriverBackupInfo, InitTable_ViaConstructor_ColumnWidthsSet)
{
    // Arrange
    ASSERT_NE(nullptr, m_page);

    // Act
    DriverTableView *backable = innerView(m_page->mp_ViewBackable);
    DriverTableView *backedUp = innerView(m_page->mp_ViewBackedUp);

    // Assert
    EXPECT_EQ(41, backable->columnWidth(0));
    EXPECT_EQ(290, backable->columnWidth(1));
    EXPECT_EQ(120, backable->columnWidth(2));
    EXPECT_EQ(120, backable->columnWidth(3));
    EXPECT_EQ(120, backable->columnWidth(4));
    EXPECT_EQ(80, backable->columnWidth(5));
    EXPECT_EQ(418, backedUp->columnWidth(0));
    EXPECT_EQ(185, backedUp->columnWidth(1));
    EXPECT_EQ(QStringLiteral("Name"),
              backable->model()->headerData(1, Qt::Horizontal).toString());
    EXPECT_EQ(QStringLiteral("Action"),
              backable->model()->headerData(5, Qt::Horizontal).toString());
    EXPECT_NE(nullptr, backable->mp_HeaderCb);
    EXPECT_EQ(nullptr, backedUp->mp_HeaderCb);
}

// initTable 在 clearAllData 后重新挂接，列宽恢复
TEST_F(UT_PageDriverBackupInfo, InitTable_AfterClearAllData_WidthsRestored)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL, "1.0.0", "0.9.0");
    m_page->addDriverInfoToTableView(info, 0);
    ASSERT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());

    // Act
    m_page->clearAllData();

    // Assert
    DriverTableView *backable = innerView(m_page->mp_ViewBackable);
    EXPECT_EQ(41, backable->columnWidth(0));
    EXPECT_EQ(290, backable->columnWidth(1));
    EXPECT_EQ(80, backable->columnWidth(5));
    EXPECT_NE(nullptr, backable->mp_HeaderCb);
    delete info;
}

// clearAllData 清空两个视图
TEST_F(UT_PageDriverBackupInfo, ClearAllData_AfterAdd_AllViewsResetToZero)
{
    // Arrange
    DriverInfo *both = makeDriverInfo("Cam", ST_NOT_INSTALL, "1.0.0", "0.9.0");
    m_page->addDriverInfoToTableView(both, 0);
    ASSERT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());
    ASSERT_EQ(1, m_page->mp_ViewBackedUp->model()->rowCount());

    // Act
    m_page->clearAllData();

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewBackedUp->model()->rowCount());
    delete both;
}

// clearAllData 重复调用保持空表且不崩溃
TEST_F(UT_PageDriverBackupInfo, ClearAllData_CalledTwice_NoCrashAndEmpty)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL, "1.0.0", "0.9.0");
    m_page->addDriverInfoToTableView(info, 0);

    // Act
    m_page->clearAllData();
    m_page->clearAllData();

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewBackedUp->model()->rowCount());
    EXPECT_EQ(41, innerView(m_page->mp_ViewBackable)->columnWidth(0));
    delete info;
}

// showTables 双零：setNoBackupDriverUI(0, 0) 且区域隐藏
TEST_F(UT_PageDriverBackupInfo, ShowTables_BothZero_NoBackupUICalled)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DetectedStatusWidget, setNoBackupDriverUI), ut_setNoBackupDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setBackableDriverUI), ut_setBackableDriverUI);

    // Act
    m_page->showTables(0, 0);

    // Assert
    EXPECT_EQ(1, g_noBackupRecord.calls);
    EXPECT_EQ(0, g_noBackupRecord.n1);
    EXPECT_EQ(0, g_noBackupRecord.n2);
    EXPECT_EQ(0, g_backableRecord.calls);
    EXPECT_FALSE(m_page->mp_BackableDriverLabel->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_NotBackupWidget->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_BackedUpDriverLabel->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_BackedUpWidget->isVisibleTo(m_page));
    EXPECT_EQ(QStringLiteral("Backupable Drivers"), m_page->mp_BackableDriverLabel->text());
}

// showTables 仅可备份：setBackableDriverUI 分支
TEST_F(UT_PageDriverBackupInfo, ShowTables_BackableOnly_BackableUICalled)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DetectedStatusWidget, setNoBackupDriverUI), ut_setNoBackupDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setBackableDriverUI), ut_setBackableDriverUI);

    // Act
    m_page->showTables(2, 0);

    // Assert
    EXPECT_EQ(1, g_backableRecord.calls);
    EXPECT_EQ(2, g_backableRecord.n1);
    EXPECT_EQ(0, g_backableRecord.n2);
    EXPECT_EQ(0, g_noBackupRecord.calls);
    EXPECT_TRUE(m_page->mp_BackableDriverLabel->isVisibleTo(m_page));
    EXPECT_TRUE(m_page->mp_NotBackupWidget->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_BackedUpWidget->isVisibleTo(m_page));
}

// showTables 仅已备份：setNoBackupDriverUI(0, n) 且备份区域可见
TEST_F(UT_PageDriverBackupInfo, ShowTables_BackedUpOnly_NoBackupUICalled)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DetectedStatusWidget, setNoBackupDriverUI), ut_setNoBackupDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setBackableDriverUI), ut_setBackableDriverUI);

    // Act
    m_page->showTables(0, 3);

    // Assert
    EXPECT_EQ(1, g_noBackupRecord.calls);
    EXPECT_EQ(0, g_noBackupRecord.n1);
    EXPECT_EQ(3, g_noBackupRecord.n2);
    EXPECT_EQ(0, g_backableRecord.calls);
    EXPECT_TRUE(m_page->mp_BackedUpDriverLabel->isVisibleTo(m_page));
    EXPECT_TRUE(m_page->mp_BackedUpWidget->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_NotBackupWidget->isVisibleTo(m_page));
}

// showTables 双区域共存：setBackableDriverUI(b, u) 且全部可见
TEST_F(UT_PageDriverBackupInfo, ShowTables_BothPresent_AllVisible)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DetectedStatusWidget, setNoBackupDriverUI), ut_setNoBackupDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setBackableDriverUI), ut_setBackableDriverUI);

    // Act
    m_page->showTables(2, 3);

    // Assert
    EXPECT_EQ(1, g_backableRecord.calls);
    EXPECT_EQ(2, g_backableRecord.n1);
    EXPECT_EQ(3, g_backableRecord.n2);
    EXPECT_EQ(0, g_noBackupRecord.calls);
    EXPECT_TRUE(m_page->mp_NotBackupWidget->isVisibleTo(m_page));
    EXPECT_TRUE(m_page->mp_BackedUpWidget->isVisibleTo(m_page));
    EXPECT_EQ(QStringLiteral("Backed up Drivers"), m_page->mp_BackedUpDriverLabel->text());
}

// showTables 负数计数边界：!= 0 为真，区域仍可见
TEST_F(UT_PageDriverBackupInfo, ShowTables_NegativeCounts_WidgetsStillVisible)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DetectedStatusWidget, setNoBackupDriverUI), ut_setNoBackupDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setBackableDriverUI), ut_setBackableDriverUI);

    // Act
    m_page->showTables(-1, -1);

    // Assert
    EXPECT_EQ(1, g_backableRecord.calls);
    EXPECT_EQ(-1, g_backableRecord.n1);
    EXPECT_EQ(-1, g_backableRecord.n2);
    EXPECT_TRUE(m_page->mp_NotBackupWidget->isVisibleTo(m_page));
    EXPECT_TRUE(m_page->mp_BackedUpWidget->isVisibleTo(m_page));
}

// addDriverInfoToTableView B1：备份版本 != 当前版本 → 可备份视图整行，状态固定 NOT_BACKUP
TEST_F(UT_PageDriverBackupInfo, AddDriverInfoToTableView_Backable_RowAddedWithNotBackupStatus)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL, "1.0.0", "");

    // Act
    m_page->addDriverInfoToTableView(info, 5);

    // Assert
    ASSERT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewBackedUp->model()->rowCount());
    auto *cb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewBackable, 0, 0));
    auto *name = dynamic_cast<DriverNameItem *>(rowWidget(m_page->mp_ViewBackable, 0, 1));
    auto *version = dynamic_cast<DriverLabelItem *>(rowWidget(m_page->mp_ViewBackable, 0, 2));
    auto *debVersion = dynamic_cast<DriverLabelItem *>(rowWidget(m_page->mp_ViewBackable, 0, 3));
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewBackable, 0, 4));
    auto *operate = dynamic_cast<DriverOperationItem *>(rowWidget(m_page->mp_ViewBackable, 0, 5));
    ASSERT_NE(nullptr, cb);
    EXPECT_TRUE(cb->checked());
    ASSERT_NE(nullptr, name);
    EXPECT_EQ(QStringLiteral("Cam"), name->mp_Name->text());
    EXPECT_EQ(5, name->index());
    EXPECT_NE(nullptr, version);
    EXPECT_NE(nullptr, debVersion);
    ASSERT_NE(nullptr, status);
    EXPECT_EQ(ST_DRIVER_NOT_BACKUP, status->getStatus());
    ASSERT_NE(nullptr, operate);
    EXPECT_EQ(DriverOperationItem::BACKUP, operate->mode());
    delete info;
}

// addDriverInfoToTableView B2：备份版本非空且等于当前版本 → 仅落已备份视图
TEST_F(UT_PageDriverBackupInfo, AddDriverInfoToTableView_BackedUp_RowAddedToBackedUpView)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL, "0.9.0", "0.9.0");

    // Act
    m_page->addDriverInfoToTableView(info, 2);

    // Assert
    ASSERT_EQ(1, m_page->mp_ViewBackedUp->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
    auto *name = dynamic_cast<DriverNameItem *>(rowWidget(m_page->mp_ViewBackedUp, 0, 0));
    auto *version = dynamic_cast<DriverLabelItem *>(rowWidget(m_page->mp_ViewBackedUp, 0, 1));
    auto *debVersion = dynamic_cast<DriverLabelItem *>(rowWidget(m_page->mp_ViewBackedUp, 0, 2));
    ASSERT_NE(nullptr, name);
    EXPECT_EQ(QStringLiteral("Cam"), name->mp_Name->text());
    EXPECT_EQ(2, name->index());
    EXPECT_NE(nullptr, version);
    EXPECT_NE(nullptr, debVersion);
    delete info;
}

// addDriverInfoToTableView B1+B2：备份版本非空且不同于当前版本 → 两个视图各落一行
TEST_F(UT_PageDriverBackupInfo, AddDriverInfoToTableView_BothVersions_BothViewsGetRows)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_CAN_UPDATE, "1.0.0", "0.9.0");

    // Act
    m_page->addDriverInfoToTableView(info, 0);

    // Assert
    ASSERT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());
    ASSERT_EQ(1, m_page->mp_ViewBackedUp->model()->rowCount());
    auto *cb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewBackable, 0, 0));
    ASSERT_NE(nullptr, cb);
    EXPECT_FALSE(cb->checked());  // 状态非 ST_NOT_INSTALL，勾选框不预选
    auto *name = dynamic_cast<DriverNameItem *>(rowWidget(m_page->mp_ViewBackedUp, 0, 0));
    ASSERT_NE(nullptr, name);
    EXPECT_EQ(QStringLiteral("Cam"), name->mp_Name->text());
    EXPECT_EQ(0, name->index());
    delete info;
}

// addDriverInfoToTableView 双版本相等且备份版本为空 → 两个视图均不落行
TEST_F(UT_PageDriverBackupInfo, AddDriverInfoToTableView_NoBackupVersions_NoRowsAdded)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL, "", "");

    // Act
    m_page->addDriverInfoToTableView(info, 0);

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewBackedUp->model()->rowCount());
    delete info;
}

// updateItemStatus：备份失败状态写入错误信息
TEST_F(UT_PageDriverBackupInfo, UpdateItemStatus_BackupFailed_SetsErrorMsg)
{
    // Arrange
    resetAllRecords();
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL, "1.0.0", "");
    m_page->addDriverInfoToTableView(info, 0);
    stub.set(ADDR(DriverStatusItem, setErrorMsg), ut_statusSetErrorMsg);

    // Act
    m_page->updateItemStatus(0, ST_DRIVER_BACKUP_FAILED);

    // Assert
    EXPECT_EQ(1, g_errorRecord.calls);
    EXPECT_EQ(QStringLiteral("Backup Failed"), g_errorRecord.str1);
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewBackable, 0, 4));
    ASSERT_NE(nullptr, status);
    EXPECT_EQ(ST_DRIVER_BACKUP_FAILED, status->getStatus());
    delete info;
}

// updateItemStatus：非失败状态不写错误信息
TEST_F(UT_PageDriverBackupInfo, UpdateItemStatus_BackupSuccess_NoErrorMsg)
{
    // Arrange
    resetAllRecords();
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL, "1.0.0", "");
    m_page->addDriverInfoToTableView(info, 0);
    stub.set(ADDR(DriverStatusItem, setErrorMsg), ut_statusSetErrorMsg);

    // Act
    m_page->updateItemStatus(0, ST_DRIVER_BACKUP_SUCCESS);

    // Assert
    EXPECT_EQ(0, g_errorRecord.calls);
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewBackable, 0, 4));
    ASSERT_NE(nullptr, status);
    EXPECT_EQ(ST_DRIVER_BACKUP_SUCCESS, status->getStatus());
    EXPECT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());
    delete info;
}

// setCheckedCBDisnable：已勾选行置灰、状态转等待
TEST_F(UT_PageDriverBackupInfo, SetCheckedCBDisnable_CheckedItem_DisabledAndWaiting)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL, "1.0.0", "");
    m_page->addDriverInfoToTableView(info, 0);
    auto *cb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewBackable, 0, 0));
    ASSERT_NE(nullptr, cb);
    ASSERT_TRUE(cb->checked());

    // Act
    m_page->setCheckedCBDisnable();

    // Assert
    EXPECT_FALSE(cb->isEnabled());
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewBackable, 0, 4));
    auto *operate = dynamic_cast<DriverOperationItem *>(rowWidget(m_page->mp_ViewBackable, 0, 5));
    ASSERT_NE(nullptr, status);
    ASSERT_NE(nullptr, operate);
    EXPECT_EQ(ST_WAITING, status->getStatus());
    EXPECT_FALSE(operate->mp_Btn->isEnabled());
    delete info;
}

// setHeaderCbEnable：表头勾选框可用状态随参数切换
TEST_F(UT_PageDriverBackupInfo, SetHeaderCbEnable_TrueAndFalse_HeaderStateChanges)
{
    // Arrange
    DriverTableView *backable = innerView(m_page->mp_ViewBackable);
    ASSERT_NE(nullptr, backable->mp_HeaderCb);
    ASSERT_TRUE(backable->mp_HeaderCb->isEnabled());

    // Act
    m_page->setHeaderCbEnable(false);

    // Assert
    EXPECT_FALSE(backable->mp_HeaderCb->isEnabled());
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());

    // Act: 恢复
    m_page->setHeaderCbEnable(true);

    // Assert
    EXPECT_TRUE(backable->mp_HeaderCb->isEnabled());
}

// getCheckedDriverIndex 空表：返回空列表
TEST_F(UT_PageDriverBackupInfo, GetCheckedDriverIndex_EmptyTable_ReturnsEmptyList)
{
    // Arrange
    QList<int> lstIndex;

    // Act
    m_page->getCheckedDriverIndex(lstIndex);

    // Assert
    EXPECT_TRUE(lstIndex.isEmpty());
    EXPECT_EQ(0, lstIndex.size());
}

// getCheckedDriverIndex：勾选状态决定收集结果与顺序
TEST_F(UT_PageDriverBackupInfo, GetCheckedDriverIndex_CheckedItems_ReturnsIndicesInOrder)
{
    // Arrange
    DriverInfo *info0 = makeDriverInfo("Cam0", ST_NOT_INSTALL, "1.0.0", "");
    DriverInfo *info1 = makeDriverInfo("Cam1", ST_NOT_INSTALL, "1.1.0", "");
    m_page->addDriverInfoToTableView(info0, 0);
    m_page->addDriverInfoToTableView(info1, 1);
    QList<int> lstIndex;

    // Act
    m_page->getCheckedDriverIndex(lstIndex);

    // Assert
    ASSERT_EQ(2, lstIndex.size());
    EXPECT_EQ(0, lstIndex.at(0));
    EXPECT_EQ(1, lstIndex.at(1));

    // Act: 取消首行勾选后不再收集
    auto *cb0 = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewBackable, 0, 0));
    ASSERT_NE(nullptr, cb0);
    cb0->setChecked(false);
    lstIndex.clear();
    m_page->getCheckedDriverIndex(lstIndex);
    ASSERT_EQ(1, lstIndex.size());
    EXPECT_EQ(1, lstIndex.at(0));
    delete info0;
    delete info1;
}

// backupAll 信号：页面转发 + 已勾选行置灰（构造函数连接路径）
TEST_F(UT_PageDriverBackupInfo, SetCheckedCBDisnable_ViaBackupAllSignal_CBsDisabled)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL, "1.0.0", "");
    m_page->addDriverInfoToTableView(info, 0);
    auto *cb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewBackable, 0, 0));
    ASSERT_NE(nullptr, cb);
    ASSERT_TRUE(cb->checked());
    QSignalSpy spy(m_page, &PageDriverBackupInfo::backupAll);

    // Act
    emit m_page->mp_HeadWidget->backupAll();

    // Assert
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(cb->isEnabled());
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewBackable, 0, 4));
    ASSERT_NE(nullptr, status);
    EXPECT_EQ(ST_WAITING, status->getStatus());
    delete info;
}
