// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 测试对象：deepin-devicemanager/src/Page/PageDriverInstallInfo.cpp
// 依赖策略（dependency-tracer 决策矩阵）：全部出向依赖均为项目内部 Widget
// （DriverTableView/Driver*Item/DetectedStatusWidget）与 DeviceManager 惰性单例，
// 无 DBus/网络/文件系统/proc 隐式依赖，仅对 UI 头部件 setter 与
// DeviceManager::getDeviceOverview 做 stub（记录入参），其余走真实对象。

// ── 分支清单（来源：get_code_snippet，PageDriverInstallInfo.cpp）──
// addDriverInfoToTableView(info, index)：
//   B1 ST_NOT_INSTALL            → mp_ViewNotInstall appendRowItems(6)  → AddDriverInfoToTableView_NotInstall_RowAddedToNotInstallView
//   B2 ST_CAN_UPDATE             → mp_ViewCanUpdate  appendRowItems(6)  → AddDriverInfoToTableView_CanUpdate_RowAddedToCanUpdateView
//   B3 ST_DRIVER_IS_NEW          → mp_AllDriverIsNew appendRowItems(2)  → AddDriverInfoToTableView_DriverIsNew_RowAddedToNewView
//   B4 else（未知状态）          → return，不落行                       → AddDriverInfoToTableView_UnknownStatus_NoRowAdded
//   B5 view != mp_AllDriverIsNew → 6 列整行（勾选框/名称/版本/大小/状态/操作）→ B1/B2 用例断言行内容
//   B6 view == mp_AllDriverIsNew → 2 列简单行（名称/当前版本）           → AddDriverInfoToTableView_DriverIsNew_RowAddedToNewView
// showTables(installLength, updateLength, newLength)：
//   B1 installLength == 0 && updateLength == 0 → setNoUpdateDriverUI    → ShowTables_AllZeroCount_NoUpdateUICalled
//   B2 else → setDetectFinishUI(count, overview, installLength != 0)   → ShowTables_InstallOnly_DetectFinishUIHasInstall
//                                                             / ShowTables_UpdateOnly_DetectFinishUINoInstall
//   B3 newLength != 0  → 新驱动 Label+Table 可见                        → ShowTables_NewOnly_NewWidgetsVisible
//   B4 newLength == 0  → 隐藏                                           → ShowTables_InstallOnly_DetectFinishUIHasInstall
//   B5 installLength != 0 → InstallWidget 可见                          → ShowTables_InstallOnly_DetectFinishUIHasInstall
//   B6 updateLength != 0 → UpdateWidget 可见                            → ShowTables_UpdateOnly_DetectFinishUINoInstall
//   边界：负数长度（!= 0 为真）                                         → ShowTables_NegativeCounts_WidgetsStillVisible
// getCheckedDriverIndex(lst)：
//   L1 空表循环 0 次                                                    → GetCheckedDriverIndex_EmptyTable_ReturnsEmptyList
//   L2 两个视图各扫一遍，未勾选/取消勾选不收集                          → GetCheckedDriverIndex_CheckedItems_ReturnsIndicesInOrder
// updateItemStatus(index, status, errS)：双视图 setItemStatus + setErrorMsg
//   → UpdateItemStatus_AfterAdd_StatusAppliedToBothViews / UpdateItemStatus_WithErrorMsg_SetsErrorMsg
// slotDownloadFinished(index, status)：双视图 setItemStatus            → SlotDownloadFinished_FailedStatus_... / _SuccessStatus_...
// slotDownloadProgressChanged(type, size, msg)：setDownloadUI(type, msg[2], msg[1], size, msg[0].toInt())
//   → SlotDownloadProgressChanged_CameraType_ForwardsArgsToHeadWidget / _GpuType_ForwardsArgsToHeadWidget
// clearAllData()：三视图 clear + initTable 重挂表头列宽                → ClearAllData_AfterAdd_AllViewsResetToZero / _CalledTwice_...
// setCheckedCBDisnable()：委托两个视图（仅已勾选行受影响）             → SetCheckedCBDisnable_CheckedItem_DisabledAndWaiting
//   经 installAll 信号触发同一逻辑                                      → SetCheckedCBDisnable_ViaInstallAllSignal_CBsDisabled
// setHeaderCbEnable(enable)：委托两个视图表头勾选框                    → SetHeaderCbEnable_TrueAndFalse_HeaderStateChanges
// initUI()/initTable()：经构造函数与 clearAllData 触发                 → InitUI_ViaConstructor_* / InitTable_ViaConstructor_*
// 构造函数：成员初始化 + 信号转发连接                                   → PageDriverInstallInfo_DefaultConstruct_AllMembersReady
//                                                                      / HeadWidget_AfterInit_ReturnsHeadWidgetPointer
//
// 用例计数声明（self-check-structural 验证此块，与 .ut/inventory.json 对齐）：
// | method                      | level | factors           | min | actual |
// |-----------------------------|-------|-------------------|-----|--------|
// | PageDriverInstallInfo       | low   | ctor              | 1   | 1      |
// | headWidget                  | low   | -                 | 1   | 1      |
// | initUI                      | mid   | lines:64          | 2   | 2      |
// | initTable                   | mid   | in_degree:2       | 2   | 2      |
// | addDriverInfoToTableView    | mid   | lines:69          | 2   | 4      |
// | addCurDriverInfo            | low   | -                 | 1   | 1      |
// | showTables                  | low   | -                 | 1   | 5      |
// | getCheckedDriverIndex       | low   | -                 | 1   | 2      |
// | clearAllData                | mid   | name_pattern:clear| 2   | 2      |
// | updateItemStatus            | low   | -                 | 1   | 2      |
// | setCheckedCBDisnable        | low   | -                 | 1   | 2      |
// | setHeaderCbEnable           | low   | -                 | 1   | 1      |
// | slotDownloadProgressChanged | mid   | in_degree:2       | 2   | 2      |
// | slotDownloadFinished        | mid   | in_degree:2       | 2   | 2      |
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（含 ctor/headWidget/initUI/initTable 经构造路径）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（status 三态 + 未知态；showTables 三计数维度）
// 3. 每个等价类的边界值显式覆盖: [x]（0/非 0 边界、负数边界）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（不适用：showTables 各维度断言目标不同，非同质输入）
// 5. 分支清单 → 用例映射已列出: [x]（见文件顶部）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]（B4 early-return 已覆盖）
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（不适用：无 throw 路径）
// 8. 负面场景有专门用例: [x]（未知状态、空表勾选、负数计数）
// 9. 负面用例验证强异常安全: [x]（负面用例均断言原状态未损坏）
// 10. stub 选择: [x]（沿用工程 tests/src/stub.h + 局部 Stub 对象，析构自动还原；
//     不调用 stub.clear() 以免影响 test_main 的进程级 DBus 桩，与本仓 20+ 测试文件约定一致）

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QMap>

#include "stub.h"
#include <DScrollArea>
#include "DetectedStatusWidget.h"
#include "DeviceManager.h"

#define private public
#include "PageDriverInstallInfo.h"
#include "PageDriverTableView.h"
#include "drivertableview.h"
#include "driveritem.h"
#include "BtnLabel.h"
#undef private

DWIDGET_USE_NAMESPACE

// ── stub 记录器与桩函数（仅记录入参，不触及真实 UI 状态机）──
namespace {

struct UiCallRecord
{
    int calls = 0;
    QString str1;
    QString str2;
    QString str3;
    int n1 = -1;
    bool flag = false;
    DriverType type = DR_Null;

    void reset() { *this = UiCallRecord(); }
};

static UiCallRecord g_detectRecord;    // DetectedStatusWidget::setDetectFinishUI
static UiCallRecord g_noUpdateRecord;  // DetectedStatusWidget::setNoUpdateDriverUI
static UiCallRecord g_downloadRecord;  // DetectedStatusWidget::setDownloadUI
static UiCallRecord g_errorRecord;     // DriverStatusItem::setErrorMsg

static void resetAllRecords()
{
    g_detectRecord.reset();
    g_noUpdateRecord.reset();
    g_downloadRecord.reset();
    g_errorRecord.reset();
}

static void ut_setDetectFinishUI(DetectedStatusWidget *self, const QString &count, const QString &model, bool hasInstall)
{
    Q_UNUSED(self);
    ++g_detectRecord.calls;
    g_detectRecord.n1 = count.toInt();
    g_detectRecord.str1 = model;
    g_detectRecord.flag = hasInstall;
}

static void ut_setNoUpdateDriverUI(DetectedStatusWidget *self, const QString &model)
{
    Q_UNUSED(self);
    ++g_noUpdateRecord.calls;
    g_noUpdateRecord.str1 = model;
}

static void ut_setDownloadUI(DetectedStatusWidget *self, const DriverType &driverType, const QString &speed,
                             const QString &downloadSize, const QString &totalSize, int progressValue)
{
    Q_UNUSED(self);
    ++g_downloadRecord.calls;
    g_downloadRecord.type = driverType;
    g_downloadRecord.str1 = speed;
    g_downloadRecord.str2 = downloadSize;
    g_downloadRecord.str3 = totalSize;
    g_downloadRecord.n1 = progressValue;
}

static const QMap<QString, QString> &ut_getDeviceOverview(DeviceManager *self)
{
    Q_UNUSED(self);
    static QMap<QString, QString> overview = {{"Overview", "UT-Overview"}};
    return overview;
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

static DriverInfo *makeDriverInfo(const QString &name, Status st, const QString &debVersion = "1.0.0",
                                  const QString &version = "2.0.0", const QString &size = "10MB")
{
    DriverInfo *info = new DriverInfo();
    info->m_Name = name;
    info->m_Type = DR_Camera;
    info->m_Version = version;
    info->m_Size = size;
    info->m_Status = st;
    info->m_DebVersion = debVersion;
    info->m_DebBackupVersion = "";
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

class UT_PageDriverInstallInfo : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_page = new PageDriverInstallInfo;
    }

    void TearDown() override
    {
        delete m_page;
        // Stub 成员析构自动还原被桩函数，无需 clear()（见文件头说明）
    }

    PageDriverInstallInfo *m_page = nullptr;
    Stub stub;
};

// 构造函数：成员初始化与行数
TEST_F(UT_PageDriverInstallInfo, PageDriverInstallInfo_DefaultConstruct_AllMembersReady)
{
    // Arrange
    delete m_page;  // 释放 SetUp 实例，由 Act 在用例内构造

    // Act
    m_page = new PageDriverInstallInfo;

    // Assert
    ASSERT_NE(nullptr, m_page);
    EXPECT_NE(nullptr, m_page->mp_HeadWidget);
    EXPECT_NE(nullptr, m_page->mp_ViewNotInstall);
    EXPECT_NE(nullptr, m_page->mp_ViewCanUpdate);
    EXPECT_NE(nullptr, m_page->mp_AllDriverIsNew);
    EXPECT_NE(nullptr, m_page->mp_InstallLabel);
    EXPECT_NE(nullptr, m_page->mp_UpdateLabel);
    EXPECT_NE(nullptr, m_page->mp_LabelIsNew);
    EXPECT_NE(nullptr, m_page->mp_InstallWidget);
    EXPECT_NE(nullptr, m_page->mp_UpdateWidget);
    EXPECT_EQ(0, m_page->mp_ViewNotInstall->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewCanUpdate->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_AllDriverIsNew->model()->rowCount());
}

// headWidget getter 返回内部头部件
TEST_F(UT_PageDriverInstallInfo, HeadWidget_AfterInit_ReturnsHeadWidgetPointer)
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
TEST_F(UT_PageDriverInstallInfo, InitUI_ViaConstructor_LayoutInstalled)
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
TEST_F(UT_PageDriverInstallInfo, InitUI_ViaConstructor_WidgetsParented)
{
    // Arrange
    ASSERT_NE(nullptr, m_page);

    // Act
    QWidget *headWindow = m_page->mp_HeadWidget->window();
    QWidget *viewWindow = m_page->mp_ViewNotInstall->window();

    // Assert
    EXPECT_EQ(m_page, headWindow);
    EXPECT_EQ(m_page, viewWindow);
    EXPECT_EQ(m_page, m_page->mp_InstallWidget->window());
}

// initTable 初始化表头、列宽与表头勾选框
TEST_F(UT_PageDriverInstallInfo, InitTable_ViaConstructor_ColumnWidthsSet)
{
    // Arrange
    ASSERT_NE(nullptr, m_page);

    // Act
    DriverTableView *notInstall = innerView(m_page->mp_ViewNotInstall);
    DriverTableView *canUpdate = innerView(m_page->mp_ViewCanUpdate);
    DriverTableView *isNew = innerView(m_page->mp_AllDriverIsNew);

    // Assert
    EXPECT_EQ(41, notInstall->columnWidth(0));
    EXPECT_EQ(278, notInstall->columnWidth(1));
    EXPECT_EQ(150, notInstall->columnWidth(2));
    EXPECT_EQ(100, notInstall->columnWidth(3));
    EXPECT_EQ(140, notInstall->columnWidth(4));
    EXPECT_EQ(60, notInstall->columnWidth(5));
    EXPECT_EQ(41, canUpdate->columnWidth(0));
    EXPECT_EQ(418, isNew->columnWidth(0));
    EXPECT_EQ(QStringLiteral("Device Name"),
              notInstall->model()->headerData(1, Qt::Horizontal).toString());
    EXPECT_EQ(QStringLiteral("Action"),
              notInstall->model()->headerData(5, Qt::Horizontal).toString());
    EXPECT_NE(nullptr, notInstall->mp_HeaderCb);
    EXPECT_NE(nullptr, canUpdate->mp_HeaderCb);
    EXPECT_EQ(nullptr, isNew->mp_HeaderCb);
}

// initTable 在 clearAllData 后重新挂接，列宽恢复
TEST_F(UT_PageDriverInstallInfo, InitTable_AfterClearAllData_WidthsRestored)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL);
    m_page->addDriverInfoToTableView(info, 0);
    EXPECT_EQ(1, m_page->mp_ViewNotInstall->model()->rowCount());

    // Act
    m_page->clearAllData();

    // Assert
    DriverTableView *notInstall = innerView(m_page->mp_ViewNotInstall);
    EXPECT_EQ(41, notInstall->columnWidth(0));
    EXPECT_EQ(278, notInstall->columnWidth(1));
    EXPECT_EQ(140, notInstall->columnWidth(4));
    EXPECT_NE(nullptr, notInstall->mp_HeaderCb);
    delete info;
}

// clearAllData 清空三个视图并重建表头
TEST_F(UT_PageDriverInstallInfo, ClearAllData_AfterAdd_AllViewsResetToZero)
{
    // Arrange
    DriverInfo *notInstall = makeDriverInfo("Cam", ST_NOT_INSTALL);
    DriverInfo *canUpdate = makeDriverInfo("Gpu", ST_CAN_UPDATE);
    DriverInfo *isNew = makeDriverInfo("Nv", ST_DRIVER_IS_NEW);
    m_page->addDriverInfoToTableView(notInstall, 0);
    m_page->addDriverInfoToTableView(canUpdate, 1);
    m_page->addDriverInfoToTableView(isNew, 2);
    ASSERT_EQ(1, m_page->mp_ViewNotInstall->model()->rowCount());
    ASSERT_EQ(1, m_page->mp_ViewCanUpdate->model()->rowCount());
    ASSERT_EQ(1, m_page->mp_AllDriverIsNew->model()->rowCount());

    // Act
    m_page->clearAllData();

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewNotInstall->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewCanUpdate->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_AllDriverIsNew->model()->rowCount());
    delete notInstall;
    delete canUpdate;
    delete isNew;
}

// clearAllData 重复调用保持空表且不崩溃（强状态不损坏）
TEST_F(UT_PageDriverInstallInfo, ClearAllData_CalledTwice_NoCrashAndEmpty)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL);
    m_page->addDriverInfoToTableView(info, 0);

    // Act
    m_page->clearAllData();
    m_page->clearAllData();

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewNotInstall->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewCanUpdate->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_AllDriverIsNew->model()->rowCount());
    EXPECT_EQ(41, innerView(m_page->mp_ViewNotInstall)->columnWidth(0));
    delete info;
}

// showTables 全零：走 setNoUpdateDriverUI 分支
TEST_F(UT_PageDriverInstallInfo, ShowTables_AllZeroCount_NoUpdateUICalled)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DeviceManager, getDeviceOverview), ut_getDeviceOverview);
    stub.set(ADDR(DetectedStatusWidget, setNoUpdateDriverUI), ut_setNoUpdateDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setDetectFinishUI), ut_setDetectFinishUI);

    // Act
    m_page->showTables(0, 0, 0);

    // Assert
    EXPECT_EQ(1, g_noUpdateRecord.calls);
    EXPECT_EQ(QStringLiteral("UT-Overview"), g_noUpdateRecord.str1);
    EXPECT_EQ(0, g_detectRecord.calls);
    EXPECT_FALSE(m_page->mp_InstallWidget->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_UpdateWidget->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_LabelIsNew->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_AllDriverIsNew->isVisibleTo(m_page));
    EXPECT_EQ(QStringLiteral("Missing drivers (0)"), m_page->mp_InstallLabel->text());
}

// showTables 仅缺装驱动：setDetectFinishUI 且 hasInstall=true
TEST_F(UT_PageDriverInstallInfo, ShowTables_InstallOnly_DetectFinishUIHasInstall)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DeviceManager, getDeviceOverview), ut_getDeviceOverview);
    stub.set(ADDR(DetectedStatusWidget, setNoUpdateDriverUI), ut_setNoUpdateDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setDetectFinishUI), ut_setDetectFinishUI);

    // Act
    m_page->showTables(2, 0, 0);

    // Assert
    EXPECT_EQ(1, g_detectRecord.calls);
    EXPECT_EQ(2, g_detectRecord.n1);
    EXPECT_EQ(QStringLiteral("UT-Overview"), g_detectRecord.str1);
    EXPECT_TRUE(g_detectRecord.flag);
    EXPECT_EQ(0, g_noUpdateRecord.calls);
    EXPECT_TRUE(m_page->mp_InstallWidget->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_UpdateWidget->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_LabelIsNew->isVisibleTo(m_page));
}

// showTables 仅可升级驱动：setDetectFinishUI 且 hasInstall=false
TEST_F(UT_PageDriverInstallInfo, ShowTables_UpdateOnly_DetectFinishUINoInstall)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DeviceManager, getDeviceOverview), ut_getDeviceOverview);
    stub.set(ADDR(DetectedStatusWidget, setNoUpdateDriverUI), ut_setNoUpdateDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setDetectFinishUI), ut_setDetectFinishUI);

    // Act
    m_page->showTables(0, 3, 0);

    // Assert
    EXPECT_EQ(1, g_detectRecord.calls);
    EXPECT_EQ(3, g_detectRecord.n1);
    EXPECT_FALSE(g_detectRecord.flag);
    EXPECT_EQ(0, g_noUpdateRecord.calls);
    EXPECT_TRUE(m_page->mp_UpdateWidget->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_InstallWidget->isVisibleTo(m_page));
    EXPECT_EQ(QStringLiteral("Outdated drivers (3)"), m_page->mp_UpdateLabel->text());
}

// showTables 仅有最新驱动：无更新分支 + 新驱动区域可见
TEST_F(UT_PageDriverInstallInfo, ShowTables_NewOnly_NewWidgetsVisible)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DeviceManager, getDeviceOverview), ut_getDeviceOverview);
    stub.set(ADDR(DetectedStatusWidget, setNoUpdateDriverUI), ut_setNoUpdateDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setDetectFinishUI), ut_setDetectFinishUI);

    // Act
    m_page->showTables(0, 0, 5);

    // Assert
    EXPECT_EQ(1, g_noUpdateRecord.calls);
    EXPECT_EQ(0, g_detectRecord.calls);
    EXPECT_TRUE(m_page->mp_LabelIsNew->isVisibleTo(m_page));
    EXPECT_TRUE(m_page->mp_AllDriverIsNew->isVisibleTo(m_page));
    EXPECT_EQ(QStringLiteral("Up-to-date drivers (5)"), m_page->mp_LabelIsNew->text());
    EXPECT_FALSE(m_page->mp_InstallWidget->isVisibleTo(m_page));
    EXPECT_FALSE(m_page->mp_UpdateWidget->isVisibleTo(m_page));
}

// showTables 三区域共存：全部可见且计数正确
TEST_F(UT_PageDriverInstallInfo, ShowTables_AllPresent_AllWidgetsVisible)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DeviceManager, getDeviceOverview), ut_getDeviceOverview);
    stub.set(ADDR(DetectedStatusWidget, setNoUpdateDriverUI), ut_setNoUpdateDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setDetectFinishUI), ut_setDetectFinishUI);

    // Act
    m_page->showTables(1, 1, 5);

    // Assert
    EXPECT_EQ(1, g_detectRecord.calls);
    EXPECT_EQ(2, g_detectRecord.n1);
    EXPECT_TRUE(g_detectRecord.flag);
    EXPECT_TRUE(m_page->mp_InstallWidget->isVisibleTo(m_page));
    EXPECT_TRUE(m_page->mp_UpdateWidget->isVisibleTo(m_page));
    EXPECT_TRUE(m_page->mp_LabelIsNew->isVisibleTo(m_page));
    EXPECT_TRUE(m_page->mp_AllDriverIsNew->isVisibleTo(m_page));
    EXPECT_EQ(QStringLiteral("Missing drivers (1)"), m_page->mp_InstallLabel->text());
    EXPECT_EQ(QStringLiteral("Up-to-date drivers (5)"), m_page->mp_LabelIsNew->text());
}

// showTables 负数计数边界：!= 0 为真，区域仍可见
TEST_F(UT_PageDriverInstallInfo, ShowTables_NegativeCounts_WidgetsStillVisible)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DeviceManager, getDeviceOverview), ut_getDeviceOverview);
    stub.set(ADDR(DetectedStatusWidget, setNoUpdateDriverUI), ut_setNoUpdateDriverUI);
    stub.set(ADDR(DetectedStatusWidget, setDetectFinishUI), ut_setDetectFinishUI);

    // Act
    m_page->showTables(-1, 0, -1);

    // Assert
    EXPECT_TRUE(m_page->mp_InstallWidget->isVisibleTo(m_page));
    EXPECT_TRUE(m_page->mp_LabelIsNew->isVisibleTo(m_page));
    EXPECT_TRUE(m_page->mp_AllDriverIsNew->isVisibleTo(m_page));
    EXPECT_EQ(1, g_detectRecord.calls);
    EXPECT_EQ(-1, g_detectRecord.n1);  // count = installLength + updateLength
    EXPECT_TRUE(g_detectRecord.flag);
    EXPECT_EQ(0, g_noUpdateRecord.calls);
}

// addDriverInfoToTableView B1：ST_NOT_INSTALL 落未安装视图，6 列整行
TEST_F(UT_PageDriverInstallInfo, AddDriverInfoToTableView_NotInstall_RowAddedToNotInstallView)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL);

    // Act
    m_page->addDriverInfoToTableView(info, 5);

    // Assert
    ASSERT_EQ(1, m_page->mp_ViewNotInstall->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewCanUpdate->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_AllDriverIsNew->model()->rowCount());
    auto *cb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 0));
    auto *name = dynamic_cast<DriverNameItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 1));
    auto *version = dynamic_cast<DriverLabelItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 2));
    auto *size = dynamic_cast<DriverLabelItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 3));
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 4));
    auto *operate = dynamic_cast<DriverOperationItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 5));
    EXPECT_NE(nullptr, cb);
    EXPECT_TRUE(cb->checked());
    EXPECT_NE(nullptr, name);
    EXPECT_EQ(QStringLiteral("Cam"), name->mp_Name->text());
    EXPECT_EQ(5, name->index());
    EXPECT_NE(nullptr, version);
    EXPECT_NE(nullptr, size);
    ASSERT_NE(nullptr, status);
    EXPECT_EQ(ST_NOT_INSTALL, status->getStatus());
    ASSERT_NE(nullptr, operate);
    EXPECT_EQ(DriverOperationItem::INSTALL, operate->mode());
    delete info;
}

// addDriverInfoToTableView B2：ST_CAN_UPDATE 落可升级视图，勾选框未选中
TEST_F(UT_PageDriverInstallInfo, AddDriverInfoToTableView_CanUpdate_RowAddedToCanUpdateView)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Gpu", ST_CAN_UPDATE, "1.5.0", "3.0.0", "20MB");

    // Act
    m_page->addDriverInfoToTableView(info, 1);

    // Assert
    ASSERT_EQ(1, m_page->mp_ViewCanUpdate->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewNotInstall->model()->rowCount());
    auto *cb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewCanUpdate, 0, 0));
    auto *name = dynamic_cast<DriverNameItem *>(rowWidget(m_page->mp_ViewCanUpdate, 0, 1));
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewCanUpdate, 0, 4));
    auto *operate = dynamic_cast<DriverOperationItem *>(rowWidget(m_page->mp_ViewCanUpdate, 0, 5));
    ASSERT_NE(nullptr, cb);
    EXPECT_FALSE(cb->checked());
    ASSERT_NE(nullptr, name);
    EXPECT_EQ(1, name->index());
    ASSERT_NE(nullptr, status);
    EXPECT_EQ(ST_CAN_UPDATE, status->getStatus());
    ASSERT_NE(nullptr, operate);
    EXPECT_EQ(DriverOperationItem::UPDATE, operate->mode());
    delete info;
}

// addDriverInfoToTableView B3/B6：ST_DRIVER_IS_NEW 落最新驱动视图，仅 2 列无勾选框
TEST_F(UT_PageDriverInstallInfo, AddDriverInfoToTableView_DriverIsNew_RowAddedToNewView)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Nv", ST_DRIVER_IS_NEW, "1.0.0", "2.0.0");

    // Act
    m_page->addDriverInfoToTableView(info, 3);

    // Assert
    ASSERT_EQ(1, m_page->mp_AllDriverIsNew->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewNotInstall->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewCanUpdate->model()->rowCount());
    auto *name = dynamic_cast<DriverNameItem *>(rowWidget(m_page->mp_AllDriverIsNew, 0, 0));
    auto *version = dynamic_cast<DriverLabelItem *>(rowWidget(m_page->mp_AllDriverIsNew, 0, 1));
    auto *noCb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_AllDriverIsNew, 0, 0));
    ASSERT_NE(nullptr, name);
    EXPECT_EQ(QStringLiteral("Nv"), name->mp_Name->text());
    EXPECT_EQ(3, name->index());
    EXPECT_NE(nullptr, version);
    EXPECT_EQ(nullptr, noCb);
    delete info;
}

// addDriverInfoToTableView B4：未知状态 early-return，任何视图不落行
TEST_F(UT_PageDriverInstallInfo, AddDriverInfoToTableView_UnknownStatus_NoRowAdded)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Waiting", ST_WAITING);

    // Act
    m_page->addDriverInfoToTableView(info, 0);

    // Assert
    EXPECT_EQ(0, m_page->mp_ViewNotInstall->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_ViewCanUpdate->model()->rowCount());
    EXPECT_EQ(0, m_page->mp_AllDriverIsNew->model()->rowCount());
    delete info;
}

// addCurDriverInfo：最新驱动视图追加名称与当前版本
TEST_F(UT_PageDriverInstallInfo, AddCurDriverInfo_ValidInfo_RowAddedToNewView)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cur", ST_DRIVER_IS_NEW, "1.0.0", "2.0.0");

    // Act
    m_page->addCurDriverInfo(info);

    // Assert
    ASSERT_EQ(1, m_page->mp_AllDriverIsNew->model()->rowCount());
    auto *name = dynamic_cast<DriverNameItem *>(rowWidget(m_page->mp_AllDriverIsNew, 0, 0));
    auto *version = dynamic_cast<DriverLabelItem *>(rowWidget(m_page->mp_AllDriverIsNew, 0, 1));
    ASSERT_NE(nullptr, name);
    EXPECT_EQ(QStringLiteral("Cur"), name->mp_Name->text());
    EXPECT_NE(nullptr, version);
    delete info;
}

// getCheckedDriverIndex 空表：循环 0 次，返回空列表
TEST_F(UT_PageDriverInstallInfo, GetCheckedDriverIndex_EmptyTable_ReturnsEmptyList)
{
    // Arrange
    QList<int> lstIndex;

    // Act
    m_page->getCheckedDriverIndex(lstIndex);

    // Assert
    EXPECT_TRUE(lstIndex.isEmpty());
    EXPECT_EQ(0, lstIndex.size());
}

// getCheckedDriverIndex：两视图各扫一遍，勾选状态决定收集结果
TEST_F(UT_PageDriverInstallInfo, GetCheckedDriverIndex_CheckedItems_ReturnsIndicesInOrder)
{
    // Arrange
    DriverInfo *notInstall0 = makeDriverInfo("Cam0", ST_NOT_INSTALL);
    DriverInfo *notInstall1 = makeDriverInfo("Cam1", ST_NOT_INSTALL);
    DriverInfo *canUpdate = makeDriverInfo("Gpu", ST_CAN_UPDATE);
    m_page->addDriverInfoToTableView(notInstall0, 0);
    m_page->addDriverInfoToTableView(notInstall1, 1);
    m_page->addDriverInfoToTableView(canUpdate, 2);
    QList<int> lstIndex;

    // Act
    m_page->getCheckedDriverIndex(lstIndex);

    // Assert
    ASSERT_EQ(2, lstIndex.size());
    EXPECT_EQ(0, lstIndex.at(0));
    EXPECT_EQ(1, lstIndex.at(1));

    // Act: 勾选可升级行的勾选框后被收集
    auto *cb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewCanUpdate, 0, 0));
    ASSERT_NE(nullptr, cb);
    cb->setChecked(true);
    lstIndex.clear();
    m_page->getCheckedDriverIndex(lstIndex);
    ASSERT_EQ(3, lstIndex.size());
    EXPECT_EQ(2, lstIndex.at(2));

    // Act: 取消未安装首行勾选后不再收集
    auto *cb0 = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 0));
    ASSERT_NE(nullptr, cb0);
    cb0->setChecked(false);
    lstIndex.clear();
    m_page->getCheckedDriverIndex(lstIndex);
    ASSERT_EQ(2, lstIndex.size());
    EXPECT_EQ(1, lstIndex.at(0));
    EXPECT_EQ(2, lstIndex.at(1));
    delete notInstall0;
    delete notInstall1;
    delete canUpdate;
}

// updateItemStatus：双视图状态项同步更新
TEST_F(UT_PageDriverInstallInfo, UpdateItemStatus_AfterAdd_StatusAppliedToBothViews)
{
    // Arrange
    DriverInfo *notInstall = makeDriverInfo("Cam", ST_NOT_INSTALL);
    DriverInfo *canUpdate = makeDriverInfo("Gpu", ST_CAN_UPDATE);
    m_page->addDriverInfoToTableView(notInstall, 0);
    m_page->addDriverInfoToTableView(canUpdate, 0);

    // Act
    m_page->updateItemStatus(0, ST_FAILED, QStringLiteral("Net Error"));

    // Assert
    auto *notInstallStatus = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 4));
    auto *canUpdateStatus = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewCanUpdate, 0, 4));
    ASSERT_NE(nullptr, notInstallStatus);
    ASSERT_NE(nullptr, canUpdateStatus);
    EXPECT_EQ(ST_FAILED, notInstallStatus->getStatus());
    EXPECT_EQ(ST_FAILED, canUpdateStatus->getStatus());
    EXPECT_EQ(1, m_page->mp_ViewNotInstall->model()->rowCount());
    EXPECT_EQ(1, m_page->mp_ViewCanUpdate->model()->rowCount());
    delete notInstall;
    delete canUpdate;
}

// updateItemStatus：errS 经视图 setErrorMsg 传达（含错误信息路径）
TEST_F(UT_PageDriverInstallInfo, UpdateItemStatus_WithErrorMsg_SetsErrorMsg)
{
    // Arrange
    resetAllRecords();
    DriverInfo *notInstall = makeDriverInfo("Cam", ST_NOT_INSTALL);
    DriverInfo *canUpdate = makeDriverInfo("Gpu", ST_CAN_UPDATE);
    m_page->addDriverInfoToTableView(notInstall, 0);
    m_page->addDriverInfoToTableView(canUpdate, 0);
    stub.set(ADDR(DriverStatusItem, setErrorMsg), ut_statusSetErrorMsg);

    // Act
    m_page->updateItemStatus(0, ST_FAILED, QStringLiteral("Net Error"));

    // Assert
    EXPECT_EQ(2, g_errorRecord.calls);  // 两个视图各按行匹配一次
    EXPECT_EQ(QStringLiteral("Net Error"), g_errorRecord.str1);
    delete notInstall;
    delete canUpdate;
}

// setCheckedCBDisnable：仅已勾选行被置灰且状态转等待，未勾选行不受影响
TEST_F(UT_PageDriverInstallInfo, SetCheckedCBDisnable_CheckedItem_DisabledAndWaiting)
{
    // Arrange
    DriverInfo *notInstall = makeDriverInfo("Cam", ST_NOT_INSTALL);
    DriverInfo *canUpdate = makeDriverInfo("Gpu", ST_CAN_UPDATE);
    m_page->addDriverInfoToTableView(notInstall, 0);
    m_page->addDriverInfoToTableView(canUpdate, 0);
    auto *checkedCb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 0));
    auto *uncheckedCb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewCanUpdate, 0, 0));
    ASSERT_NE(nullptr, checkedCb);
    ASSERT_NE(nullptr, uncheckedCb);
    ASSERT_TRUE(checkedCb->checked());
    ASSERT_FALSE(uncheckedCb->checked());

    // Act
    m_page->setCheckedCBDisnable();

    // Assert
    EXPECT_FALSE(checkedCb->isEnabled());
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 4));
    auto *operate = dynamic_cast<DriverOperationItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 5));
    ASSERT_NE(nullptr, status);
    ASSERT_NE(nullptr, operate);
    EXPECT_EQ(ST_WAITING, status->getStatus());
    EXPECT_FALSE(operate->mp_Btn->isEnabled());
    EXPECT_TRUE(uncheckedCb->isEnabled());
    auto *canUpdateStatus = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewCanUpdate, 0, 4));
    ASSERT_NE(nullptr, canUpdateStatus);
    EXPECT_EQ(ST_CAN_UPDATE, canUpdateStatus->getStatus());
    delete notInstall;
    delete canUpdate;
}

// setCheckedCBDisnable 经 installAll 信号触发（构造函数连接的 lambda 路径）
TEST_F(UT_PageDriverInstallInfo, SetCheckedCBDisnable_ViaInstallAllSignal_CBsDisabled)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL);
    m_page->addDriverInfoToTableView(info, 0);
    auto *cb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 0));
    ASSERT_NE(nullptr, cb);
    ASSERT_TRUE(cb->checked());
    QSignalSpy spy(m_page, &PageDriverInstallInfo::installAll);

    // Act
    emit m_page->mp_HeadWidget->installAll();

    // Assert
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(cb->isEnabled());
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 4));
    ASSERT_NE(nullptr, status);
    EXPECT_EQ(ST_WAITING, status->getStatus());
    delete info;
}

// setHeaderCbEnable：表头勾选框可用状态随参数切换
TEST_F(UT_PageDriverInstallInfo, SetHeaderCbEnable_TrueAndFalse_HeaderStateChanges)
{
    // Arrange
    DriverTableView *notInstall = innerView(m_page->mp_ViewNotInstall);
    ASSERT_NE(nullptr, notInstall->mp_HeaderCb);
    ASSERT_TRUE(notInstall->mp_HeaderCb->isEnabled());

    // Act
    m_page->setHeaderCbEnable(false);

    // Assert
    EXPECT_FALSE(notInstall->mp_HeaderCb->isEnabled());
    EXPECT_EQ(0, m_page->mp_ViewNotInstall->model()->rowCount());

    // Act: 恢复
    m_page->setHeaderCbEnable(true);

    // Assert
    EXPECT_TRUE(notInstall->mp_HeaderCb->isEnabled());
}

// slotDownloadProgressChanged：参数按 (type, msg[2], msg[1], size, msg[0].toInt()) 转发
TEST_F(UT_PageDriverInstallInfo, SlotDownloadProgressChanged_CameraType_ForwardsArgsToHeadWidget)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DetectedStatusWidget, setDownloadUI), ut_setDownloadUI);
    QStringList msg{"30", "1.2MB", "55MB"};

    // Act
    m_page->slotDownloadProgressChanged(DR_Camera, QStringLiteral("55MB"), msg);

    // Assert
    EXPECT_EQ(1, g_downloadRecord.calls);
    EXPECT_EQ(DR_Camera, g_downloadRecord.type);
    EXPECT_EQ(QStringLiteral("55MB"), g_downloadRecord.str1);   // speed = msg[2]
    EXPECT_EQ(QStringLiteral("1.2MB"), g_downloadRecord.str2);  // downloadSize = msg[1]
    EXPECT_EQ(QStringLiteral("55MB"), g_downloadRecord.str3);   // totalSize = size
    EXPECT_EQ(30, g_downloadRecord.n1);                         // progress = msg[0].toInt()
}

// slotDownloadProgressChanged：另一驱动类型与参数组合仍正确转发
TEST_F(UT_PageDriverInstallInfo, SlotDownloadProgressChanged_GpuType_ForwardsArgsToHeadWidget)
{
    // Arrange
    resetAllRecords();
    stub.set(ADDR(DetectedStatusWidget, setDownloadUI), ut_setDownloadUI);
    QStringList msg{"75", "3.0MB", "4.0MB"};

    // Act
    m_page->slotDownloadProgressChanged(DR_Gpu, QStringLiteral("4.0MB"), msg);

    // Assert
    EXPECT_EQ(1, g_downloadRecord.calls);
    EXPECT_EQ(DR_Gpu, g_downloadRecord.type);
    EXPECT_EQ(QStringLiteral("4.0MB"), g_downloadRecord.str1);
    EXPECT_EQ(QStringLiteral("3.0MB"), g_downloadRecord.str2);
    EXPECT_EQ(75, g_downloadRecord.n1);
}

// slotDownloadFinished：失败状态同步到两个视图
TEST_F(UT_PageDriverInstallInfo, SlotDownloadFinished_FailedStatus_StatusAppliedToBothViews)
{
    // Arrange
    DriverInfo *notInstall = makeDriverInfo("Cam", ST_NOT_INSTALL);
    DriverInfo *canUpdate = makeDriverInfo("Gpu", ST_CAN_UPDATE);
    m_page->addDriverInfoToTableView(notInstall, 0);
    m_page->addDriverInfoToTableView(canUpdate, 0);

    // Act
    m_page->slotDownloadFinished(0, ST_FAILED);

    // Assert
    auto *notInstallStatus = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 4));
    auto *canUpdateStatus = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewCanUpdate, 0, 4));
    ASSERT_NE(nullptr, notInstallStatus);
    ASSERT_NE(nullptr, canUpdateStatus);
    EXPECT_EQ(ST_FAILED, notInstallStatus->getStatus());
    EXPECT_EQ(ST_FAILED, canUpdateStatus->getStatus());
    EXPECT_EQ(1, m_page->mp_ViewNotInstall->model()->rowCount());
    EXPECT_EQ(1, m_page->mp_ViewCanUpdate->model()->rowCount());
    delete notInstall;
    delete canUpdate;
}

// slotDownloadFinished：成功状态落地且勾选框被复位取消
TEST_F(UT_PageDriverInstallInfo, SlotDownloadFinished_SuccessStatus_CbUncheckedByReset)
{
    // Arrange
    DriverInfo *info = makeDriverInfo("Cam", ST_NOT_INSTALL);
    m_page->addDriverInfoToTableView(info, 0);
    auto *cb = dynamic_cast<DriverCheckItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 0));
    ASSERT_NE(nullptr, cb);
    ASSERT_TRUE(cb->checked());

    // Act
    m_page->slotDownloadFinished(0, ST_SUCESS);

    // Assert
    auto *status = dynamic_cast<DriverStatusItem *>(rowWidget(m_page->mp_ViewNotInstall, 0, 4));
    ASSERT_NE(nullptr, status);
    EXPECT_EQ(ST_SUCESS, status->getStatus());
    EXPECT_FALSE(cb->checked());
    delete info;
}
