// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 测试对象：deepin-devicemanager/src/Page/PageDriverRestoreInfo.cpp
// 依赖策略（dependency-tracer 决策矩阵）：出向依赖均为项目内部 Widget
// （DriverTableView/Driver*Item/DetectedStatusWidget），无外部 IO；
// 仅对 UI 头部件 setter（setRestoreDriverUI）做 stub（记录入参），
// 还原链路（slotOperatorClicked）走真实对象验证状态替换。

// ── 分支清单（来源：get_code_snippet，PageDriverRestoreInfo.cpp）──
// addDriverInfoToTableView(info, index)：
//   B1 !debBackupVersion().isEmpty() → mp_ViewBackable 4 列整行（RESTORE 操作）→ AddDriverInfoToTableView_WithBackup_RowAddedWithRestoreOperation
//   B2 else（备份版本为空）→ return，不落行                                   → AddDriverInfoToTableView_NoBackup_RowUnchanged
// showTables(backedLength)：
//   B1 backedLength == 0 → mp_StackWidget 第 0 页（无备份提示页）             → ShowTables_ZeroDrivers_StackIndexZero
//   B2 else → 第 1 页（还原列表页）                                           → ShowTables_WithDrivers_StackIndexOne
//   边界：负数长度（!= 0 为真）                                               → ShowTables_NegativeCount_StackIndexOne
// slotOperatorClicked(index, itemIndex, mode)：(itemIndex, 3) 状态项替换为 ST_DRIVER_RESTORING
//   → SlotOperatorClicked_RestoreMode_StatusSetToRestoring（经视图信号真实链路）
// setItemOperationEnable(index, enable)：委托视图按名称索引定位操作按钮       → SetItemOperationEnable_TrueFalse_BtnStateChanged
// clearAllData()：视图 clear + initTable                                     → ClearAllData_AfterAdd_RowResetToZero / _CalledTwice_...
// initUI()/initTable()：经构造函数与 clearAllData 触发                       → InitUI_ViaConstructor_* / InitTable_ViaConstructor_*
// 构造函数：成员初始化 + 信号转发连接 + gotoBackup 按钮连接                   → PageDriverRestoreInfo_DefaultConstruct_AllMembersReady
//                                                                            / HeadWidget_AfterInit_ReturnsHeadWidgetPointer
//                                                                            / GotoBackupSignal_ButtonClicked_SignalEmitted
//
// 用例计数声明（self-check-structural 验证此块，与 .ut/inventory.json 对齐）：
// | method                    | level | factors            | min | actual |
// |---------------------------|-------|--------------------|-----|--------|
// | PageDriverRestoreInfo     | low   | ctor               | 1   | 1      |
// | headWidget                | low   | -                  | 1   | 1      |
// | initUI                    | mid   | lines:91           | 2   | 2      |
// | initTable                 | mid   | in_degree:2        | 2   | 2      |
// | addDriverInfoToTableView  | low   | -                  | 1   | 2      |
// | showTables                | low   | -                  | 1   | 3      |
// | setItemOperationEnable    | low   | -                  | 1   | 1      |
// | slotOperatorClicked       | low   | -                  | 1   | 1      |
// | clearAllData              | mid   | name_pattern:clear | 2   | 2      |
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（备份版本 空/非空；backedLength 0/正/负）
// 3. 每个等价类的边界值显式覆盖: [x]（0/非 0 栈页切换、负数边界、空串）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（不适用：showTables 各计数断言目标不同，非同质输入）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（B2 early-return 已覆盖）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（不适用：无 throw 路径）
// 8. 负面场景有专门用例: [x]（无备份版本、零还原数、空表）
// 9. 负面用例验证强异常安全: [x]（负面用例断言行数与栈页不变）
// 10. stub 选择: [x]（沿用工程 tests/src/stub.h + 局部 Stub 对象，析构自动还原，不调用 stub.clear()）

#include <gtest/gtest.h>

#include <QSignalSpy>

#include "stub.h"
#include <DScrollArea>
#include <DStackedWidget>
#include "DetectedStatusWidget.h"

#define private public
#include "PageDriverRestoreInfo.h"
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

    void reset() { *this = UiCallRecord(); }
};

static UiCallRecord g_restoreRecord;  // DetectedStatusWidget::setRestoreDriverUI

static void resetAllRecords()
{
    g_restoreRecord.reset();
}

static void ut_setRestoreDriverUI(DetectedStatusWidget *self, int restorableSize)
{
    Q_UNUSED(self);
    ++g_restoreRecord.calls;
    g_restoreRecord.n1 = restorableSize;
}

}  // namespace

// ── 测试数据与视图访问辅助 ──
namespace {

static DriverInfo *makeDriverInfo(const QString &name, const QString &debBackupVersion)
{
    DriverInfo *info = new DriverInfo();
    info->m_Name = name;
    info->m_Type = DR_Camera;
    info->m_Version = "2.0.0";
    info->m_Size = "10MB";
    info->m_Status = ST_NOT_INSTALL;
    info->m_DebVersion = "1.0.0";
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

class UT_PageDriverRestoreInfo : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_page = new PageDriverRestoreInfo;
    }

    void TearDown() override
    {
        delete m_page;
    }

    PageDriverRestoreInfo *m_page = nullptr;
    Stub stub;
};

// 构造函数：成员初始化
TEST_F(UT_PageDriverRestoreInfo, PageDriverRestoreInfo_DefaultConstruct_AllMembersReady)
{
    // Arrange
    delete m_page;  // 释放 SetUp 实例，由 Act 在用例内构造

    // Act
    m_page = new PageDriverRestoreInfo;

    // Assert
    ASSERT_NE(nullptr, m_page);
    EXPECT_NE(nullptr, m_page->mp_StackWidget);
    EXPECT_NE(nullptr, m_page->mp_HeadWidget);
    EXPECT_NE(nullptr, m_page->mp_NotBackupWidget);
    EXPECT_NE(nullptr, m_page->mp_ViewBackable);
    EXPECT_NE(nullptr, m_page->mp_BackableDriverLabel);
    EXPECT_NE(nullptr, m_page->mp_NoRestoreDriverFrame);
    EXPECT_NE(nullptr, m_page->mp_GotoBackupSgButton);
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
}

// headWidget getter 返回内部头部件
TEST_F(UT_PageDriverRestoreInfo, HeadWidget_AfterInit_ReturnsHeadWidgetPointer)
{
    // Arrange
    ASSERT_NE(nullptr, m_page);

    // Act
    DetectedStatusWidget *head = m_page->headWidget();

    // Assert
    EXPECT_EQ(m_page->mp_HeadWidget, head);
    EXPECT_NE(nullptr, head);
}

// initUI 经构造函数装配双页栈
TEST_F(UT_PageDriverRestoreInfo, InitUI_ViaConstructor_StackReady)
{
    // Arrange
    ASSERT_NE(nullptr, m_page);

    // Act
    QLayout *layout = m_page->layout();
    int pageCount = m_page->mp_StackWidget->count();
    int noRestoreIndex = m_page->mp_StackWidget->indexOf(m_page->mp_NoRestoreDriverFrame);

    // Assert
    EXPECT_NE(nullptr, layout);
    EXPECT_EQ(2, pageCount);
    EXPECT_EQ(0, noRestoreIndex);
    EXPECT_EQ(QStringLiteral("Go to Backup Driver"), m_page->mp_GotoBackupSgButton->text());
    EXPECT_EQ(0, m_page->mp_StackWidget->currentIndex());
}

// initUI 装配后关键子部件挂到页面窗口树下
TEST_F(UT_PageDriverRestoreInfo, InitUI_ViaConstructor_WidgetsParented)
{
    // Arrange
    ASSERT_NE(nullptr, m_page);

    // Act
    QWidget *headWindow = m_page->mp_HeadWidget->window();
    QWidget *viewWindow = m_page->mp_ViewBackable->window();

    // Assert
    EXPECT_EQ(m_page, headWindow);
    EXPECT_EQ(m_page, viewWindow);
    EXPECT_EQ(m_page, m_page->mp_NoRestoreDriverFrame->window());
}

// initTable 初始化表头与列宽（还原视图无表头勾选框）
TEST_F(UT_PageDriverRestoreInfo, InitTable_ViaConstructor_ColumnWidthsSet)
{
    // Arrange
    ASSERT_NE(nullptr, m_page);

    // Act
    DriverTableView *backable = innerView(m_page->mp_ViewBackable);

    // Assert
    EXPECT_EQ(324, backable->columnWidth(0));
    EXPECT_EQ(158, backable->columnWidth(1));
    EXPECT_EQ(158, backable->columnWidth(2));
    EXPECT_EQ(QStringLiteral("Name"),
              backable->model()->headerData(0, Qt::Horizontal).toString());
    EXPECT_EQ(QStringLiteral("Action"),
              backable->model()->headerData(3, Qt::Horizontal).toString());
    EXPECT_EQ(nullptr, backable->mp_HeaderCb);
}

// initTable 在 clearAllData 后重新挂接，列宽恢复
TEST_F(UT_PageDriverRestoreInfo, InitTable_AfterClearAllData_WidthsRestored)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", "0.9.0");
    m_page->addDriverInfoToTableView(info, 0);
    ASSERT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());

    // Act
    m_page->clearAllData();

    // Assert
    DriverTableView *backable = innerView(m_page->mp_ViewBackable);
    EXPECT_EQ(324, backable->columnWidth(0));
    EXPECT_EQ(158, backable->columnWidth(1));
    EXPECT_EQ(nullptr, backable->mp_HeaderCb);
    delete info;
}

// clearAllData 清空还原视图
TEST_F(UT_PageDriverRestoreInfo, ClearAllData_AfterAdd_RowResetToZero)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", "0.9.0");
    m_page->addDriverInfoToTableView(info, 0);
    ASSERT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());

    // Act
    m_page->clearAllData();

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(nullptr, rowWidget(m_page->mp_ViewBackable, 0, 0));
    EXPECT_EQ(324, innerView(m_page->mp_ViewBackable)->columnWidth(0));
    delete info;
}

// clearAllData 重复调用保持空表且不崩溃
TEST_F(UT_PageDriverRestoreInfo, ClearAllData_CalledTwice_NoCrashAndEmpty)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", "0.9.0");
    m_page->addDriverInfoToTableView(info, 0);

    // Act
    m_page->clearAllData();
    m_page->clearAllData();

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(324, innerView(m_page->mp_ViewBackable)->columnWidth(0));
    delete info;
}

// showTables 零还原数：setRestoreDriverUI(0) 且栈页切到无备份提示页
TEST_F(UT_PageDriverRestoreInfo, ShowTables_ZeroDrivers_StackIndexZero)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DetectedStatusWidget, setRestoreDriverUI), ut_setRestoreDriverUI);

    // Act
    m_page->showTables(0);

    // Assert
    EXPECT_EQ(1, g_restoreRecord.calls);
    EXPECT_EQ(0, g_restoreRecord.n1);
    EXPECT_EQ(0, m_page->mp_StackWidget->currentIndex());
    EXPECT_EQ(QStringLiteral("Restorable Drivers"), m_page->mp_BackableDriverLabel->text());
}

// showTables 有还原数：栈页切到还原列表页
TEST_F(UT_PageDriverRestoreInfo, ShowTables_WithDrivers_StackIndexOne)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DetectedStatusWidget, setRestoreDriverUI), ut_setRestoreDriverUI);

    // Act
    m_page->showTables(5);

    // Assert
    EXPECT_EQ(1, g_restoreRecord.calls);
    EXPECT_EQ(5, g_restoreRecord.n1);
    EXPECT_EQ(1, m_page->mp_StackWidget->currentIndex());
}

// showTables 负数长度边界：!= 0 为真，栈页切到还原列表页
TEST_F(UT_PageDriverRestoreInfo, ShowTables_NegativeCount_StackIndexOne)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DetectedStatusWidget, setRestoreDriverUI), ut_setRestoreDriverUI);

    // Act
    m_page->showTables(-1);

    // Assert
    EXPECT_EQ(1, g_restoreRecord.calls);
    EXPECT_EQ(-1, g_restoreRecord.n1);
    EXPECT_EQ(1, m_page->mp_StackWidget->currentIndex());
}

// addDriverInfoToTableView B1：有备份版本 → 4 列整行 + RESTORE 操作
TEST_F(UT_PageDriverRestoreInfo, AddDriverInfoToTableView_WithBackup_RowAddedWithRestoreOperation)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", "0.9.0");

    // Act
    m_page->addDriverInfoToTableView(info, 5);

    // Assert
    ASSERT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());
    auto *name = dynamic_cast<DriverNameItem *>(rowWidget(m_page->mp_ViewBackable, 0, 0));
    auto *version = dynamic_cast<DriverLabelItem *>(rowWidget(m_page->mp_ViewBackable, 0, 1));
    auto *backupVersion = dynamic_cast<DriverLabelItem *>(rowWidget(m_page->mp_ViewBackable, 0, 2));
    auto *operate = dynamic_cast<DriverOperationItem *>(rowWidget(m_page->mp_ViewBackable, 0, 3));
    ASSERT_NE(nullptr, name);
    EXPECT_EQ(QStringLiteral("Cam"), name->mp_Name->text());
    EXPECT_EQ(5, name->index());
    EXPECT_NE(nullptr, version);
    EXPECT_NE(nullptr, backupVersion);
    ASSERT_NE(nullptr, operate);
    EXPECT_EQ(DriverOperationItem::RESTORE, operate->mode());
    delete info;
}

// addDriverInfoToTableView B2：备份版本为空 → early-return，不落行
TEST_F(UT_PageDriverRestoreInfo, AddDriverInfoToTableView_NoBackup_RowUnchanged)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", "");

    // Act
    m_page->addDriverInfoToTableView(info, 0);

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(nullptr, rowWidget(m_page->mp_ViewBackable, 0, 0));
    delete info;
}

// setItemOperationEnable：操作按钮可用状态随参数切换
TEST_F(UT_PageDriverRestoreInfo, SetItemOperationEnable_TrueFalse_BtnStateChanged)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", "0.9.0");
    m_page->addDriverInfoToTableView(info, 0);
    auto *operate = dynamic_cast<DriverOperationItem *>(rowWidget(m_page->mp_ViewBackable, 0, 3));
    ASSERT_NE(nullptr, operate);
    ASSERT_TRUE(operate->mp_Btn->isEnabled());
    EXPECT_EQ(1, m_page->mp_ViewBackable->model()->rowCount());
    EXPECT_EQ(nullptr, dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewBackable, 0, 3)));

    // Act
    m_page->setItemOperationEnable(0, false);

    // Assert
    EXPECT_FALSE(operate->mp_Btn->isEnabled());

    // Act: 恢复
    m_page->setItemOperationEnable(0, true);
    EXPECT_TRUE(operate->mp_Btn->isEnabled());
    delete info;
}

// slotOperatorClicked 经视图信号真实链路：操作项被替换为还原中状态项，页面转发信号
TEST_F(UT_PageDriverRestoreInfo, SlotOperatorClicked_RestoreMode_StatusSetToRestoring)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", "0.9.0");
    m_page->addDriverInfoToTableView(info, 0);
    ASSERT_NE(nullptr, dynamic_cast<DriverOperationItem *>(rowWidget(m_page->mp_ViewBackable, 0, 3)));
    QSignalSpy spy(m_page, &PageDriverRestoreInfo::operatorClicked);

    // Act
    emit m_page->mp_ViewBackable->operatorClicked(5, 0, DriverOperationItem::RESTORE);

    // Assert
    EXPECT_EQ(1, spy.count());
    ASSERT_EQ(3, spy.at(0).size());  // operatorClicked(int index, int itemIndex, Mode mode)
    EXPECT_EQ(5, spy.at(0).at(0).toInt());
    EXPECT_EQ(0, spy.at(0).at(1).toInt());
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewBackable, 0, 3));
    ASSERT_NE(nullptr, status);
    EXPECT_EQ(ST_DRIVER_RESTORING, status->getStatus());
    delete info;
}

// gotoBackup 信号：点击还原页按钮触发页面转发（构造函数连接路径）
TEST_F(UT_PageDriverRestoreInfo, GotoBackupSignal_ButtonClicked_SignalEmitted)
{
    // Arrange
    QSignalSpy spy(m_page, &PageDriverRestoreInfo::gotoBackup);
    ASSERT_NE(nullptr, m_page->mp_GotoBackupSgButton);
    ASSERT_TRUE(m_page->mp_GotoBackupSgButton->isEnabled());

    // Act
    m_page->mp_GotoBackupSgButton->click();

    // Assert
    EXPECT_EQ(1, spy.count());
    ASSERT_EQ(1, spy.size());
    EXPECT_EQ(0, spy.at(0).size());  // gotoBackup 无参信号
    EXPECT_EQ(QStringLiteral("Go to Backup Driver"), m_page->mp_GotoBackupSgButton->text());
}
