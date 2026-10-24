// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// TableWidget 补强批次：数据方法状态断言 + slotShowMenu 全分支白盒覆盖 + slotAction* 信号验证。
// 已有测试审查结论（Mode 6）：本文件原用例均为 EXPECT_NO_FATAL_FAILURE 空洞断言，整文件重写。

// 分支清单（来源：MCP get_code_snippet — TableWidget::slotShowMenu, src/Widget/TableWidget.cpp:216-371）
// B1: !currentRowAvailable()          → mp_Enable/mp_removeDriver 置灰
// B2: currentRowEnable()==true        → mp_Enable 文案 "Disable"
// B3: currentRowEnable()==false       → mp_Enable 文案 "Enable"，update/remove 置灰
// B4: PageDriverControl::isRunning()  → update/remove/Enable 全置灰
// B5: row < 0                         → 提前 return（菜单不加项、不 exec）
// B6: isPrinter && !isInstalled       → mp_updateDriver 置灰
// B7: item(row,0) 非空                → UserRole/UserRole+1 解析 canUninstall/canEnable
// B8: canEnable && selected>0         → 菜单添加 mp_Enable
// B9: !canEnable                      → mp_Enable 置灰
// B10: canUninstall && selected>0     → 菜单添加分隔线+update+remove
// B11: data(UserRole+2) 有效          → 唤醒分支：
//   B11a: 值非 "true"/"false"（网络唤醒）→ isNetworkWakeup：3→勾选 4→不勾 其他→置灰
//   B11b: 值=="true"（键鼠唤醒）→ 打开 data(UserRole+3) 路径文件：
//         路径含 /proc/acpi/wakeup → isInputWakeupMachine；否则按文件内容 disabled 判定
//   B11c: 值=="false"                → mp_WakeupMachine 置灰
//   B11d: !currentRowEnable()        → mp_WakeupMachine 置灰
// B12: data(UserRole+3)=="false"      → 菜单移除 mp_Enable
//
// 用例映射：
// - SlotShowMenu_NoCurrentRow_DisablesActionsAndReturns      → B1+B3+B5
// - SlotShowMenu_DriverPageRunning_DisablesDriverActions     → B4+B2
// - SlotShowMenu_PrinterNotInstalled_DisablesUpdate          → B6+B9
// - SlotShowMenu_CapableDevice_AddsEnableAndDriverActions    → B7+B8+B10+B2
// - SlotShowMenu_WakeupFileEnabled_ChecksWakeup              → B11b（普通路径 enabled）
// - SlotShowMenu_WakeupFileDisabled_UnchecksWakeup           → B11b（内容 disabled）
// - SlotShowMenu_WakeupAcpiPath_QueriesInputWakeup           → B11b（/proc/acpi/wakeup）
// - SlotShowMenu_NetworkWakeup_ChecksState (TEST_P)          → B11a（3/4/其他 三态）
// - SlotShowMenu_WakeupFalse_DisablesWakeupAndRemovesEnable  → B11c+B12
// - SlotShowMenu_DisabledRow_DisablesWakeup                  → B11d
// - SetHeaderLabels_* / SetItem_* / Clear_* / SetRowNum_* / UpdateCurItemEnable_* / SetColumnAverage_*
//   → 数据方法（setHeaderLabels 丢尾元素、clear 清空模型、updateCurItemEnable 前缀增删）
// - SlotActionRefresh_/Export_/Enable_*/UpdateDriver_*/RemoveDriver_/WakeupMachine_*
//   → 信号槽（QSignalSpy 验证发射与参数）
//
// 最小清单：[x]1 公开方法每方法≥1用例 [x]2 等价类划分 [x]3 边界值显式覆盖
//          [x]4 同质多组输入 TEST_P（网络唤醒三态） [x]5 分支清单映射 [x]6 每条分支有触发用例
//          [x]7 异常路径（本类无 throw） [x]8 负面场景（无行/空模型/false 数据）
//          [x]9 负面用例验证状态（early-return 后菜单为空） [x]10 Qt 类用 stub_ext（项目 Stub）

#include "TableWidget.h"
#include "logtreeview.h"
#include "PageDriverControl.h"
#include "DBusWakeupInterface.h"
#include "ut_Head.h"

#include <QAction>
#include <DStandardItem>
#include <DMenu>

#include <QFile>
#include <QIODevice>
#include <QMenu>
#include <QSignalSpy>
#include <QItemSelectionModel>
#include <QTest>

#include <gtest/gtest.h>
#include "stub.h"

// ---- stub 状态（文件内静态，用例间互不影响：Stub 析构自动还原函数入口） ----
static int g_menuExecCount = 0;        // mp_Menu->exec 调用计数
static bool g_pageRunning = false;     // PageDriverControl::isRunning 返回值
static int g_netWakeupRet = 0;         // isNetworkWakeup 返回值
static int g_netWakeupCalls = 0;
static bool g_inputWakeupRet = false;   // isInputWakeupMachine 返回值
static int g_inputWakeupCalls = 0;

static QAction *ut_menuExec(void *, const QPoint &, QAction *)
{
    ++g_menuExecCount;
    return nullptr;
}

static bool ut_isRunning()
{
    return g_pageRunning;
}

static int ut_isNetworkWakeup(void *, const QString &)
{
    ++g_netWakeupCalls;
    return g_netWakeupRet;
}

static bool ut_isInputWakeupMachine(void *, const QString &, const QString &, const QString &, const QString &)
{
    ++g_inputWakeupCalls;
    return g_inputWakeupRet;
}

// 唤醒状态探测走真实临时文件（QIODevice::readAll 为虚函数，PMF 地址不可 patch）
static QString makeWakeupFile(const QByteArray &content, const QString &name)
{
    QString path = "/tmp/" + name;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(content);
        f.close();
    }
    return path;
}

class UT_TableWidgetExtra : public UT_HEAD
{
public:
    void SetUp() override
    {
        t = new TableWidget;
        // 3 个表头 → 实际列数 = 3-1 = 2（setHeaderLabels 丢弃最后一个标签）
        t->setHeaderLabels(QStringList() << "A" << "B" << "C");
        resetGlobals();
    }
    void TearDown() override
    {
        delete t;
        t = nullptr;
    }

    TableWidget *t = nullptr;

    static void resetGlobals()
    {
        g_menuExecCount = 0;
        g_pageRunning = false;
        g_netWakeupRet = 0;
        g_netWakeupCalls = 0;
        g_inputWakeupRet = false;
        g_inputWakeupCalls = 0;
    }

    // 准备一行已选中的当前行，item 文本 text，携带 data 映射（Qt::UserRole 偏移 → 值）
    void prepareRow(const QString &text = "dev", const QMap<int, QString> &data = QMap<int, QString>())
    {
        t->mp_Table->mp_Model->insertRow(0);
        QStandardItem *item = new DStandardItem(text);
        for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
            item->setData(it.value(), Qt::UserRole + it.key());
        }
        t->mp_Table->mp_Model->setItem(0, 0, item);
        t->mp_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
        QModelIndex idx = t->mp_Table->mp_Model->index(0, 0);
        t->mp_Table->setCurrentIndex(idx);
        t->mp_Table->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    // slotShowMenu 用例统一安装 exec 桩（真实 exec 会模态阻塞）
    Stub *installMenuStub()
    {
        stub.set(static_cast<QAction *(QMenu::*)(const QPoint &, QAction *)>(&QMenu::exec), ut_menuExec);
        return &stub;
    }

    Stub stub;
};

// ---------------- 构造/析构语义（high 级：所有用例均经由，此处显式断言） ----------------

TEST_F(UT_TableWidgetExtra, TableWidget_DefaultCtor_BuildsTreeActionsAndMenu)
{
    // Act：SetUp 中已完成构造（每个用例均执行）
    // Assert：对象树、动作与命名均已建立
    EXPECT_NE(t->mp_Table, nullptr);
    EXPECT_EQ(t->mp_Table->objectName(), QString("MpTable"));
    EXPECT_NE(t->mp_Enable, nullptr);
    EXPECT_NE(t->mp_Refresh, nullptr);
    EXPECT_NE(t->mp_Export, nullptr);
    EXPECT_NE(t->mp_updateDriver, nullptr);
    EXPECT_NE(t->mp_removeDriver, nullptr);
    EXPECT_NE(t->mp_WakeupMachine, nullptr);
    EXPECT_NE(t->mp_Menu, nullptr);
    EXPECT_EQ(t->mp_Enable->objectName(), QString("MpEnable"));
    // 默认状态：初始文案 Disable，LogTreeView 默认行数 4
    EXPECT_EQ(t->mp_Enable->text(), TableWidget::tr("Disable"));
    EXPECT_EQ(t->mp_Table->RowNum(), 4);
}

// ---------------- 数据方法（状态断言） ----------------

TEST_F(UT_TableWidgetExtra, SetHeaderLabels_MultipleLabels_DropsLastLabel)
{
    // SetUp 已用 3 个标签 → 2 列
    // Assert：列数 = N-1，首列文案正确
    EXPECT_EQ(t->mp_Table->mp_Model->columnCount(), 2);   // branch: 循环 i < lst.size()-1
    EXPECT_EQ(t->mp_Table->mp_Model->horizontalHeaderItem(0)->text(), QString("A"));
    EXPECT_EQ(t->mp_Table->mp_Model->horizontalHeaderItem(1)->text(), QString("B"));
}

TEST_F(UT_TableWidgetExtra, SetHeaderLabels_SingleLabel_ForwardsNothing)
{
    // 边界：单标签 → mid(0,0) 为空列表，模型不建列（0 列）
    TableWidget fresh;
    fresh.setHeaderLabels(QStringList() << "only");
    EXPECT_EQ(fresh.mp_Table->mp_Model->columnCount(), 0);   // 边界：无标签转发

    // 对比：三标签 → 转发前 2 个，模型扩展为 2 列
    TableWidget fresh2;
    fresh2.setHeaderLabels(QStringList() << "x" << "y" << "z");
    EXPECT_EQ(fresh2.mp_Table->mp_Model->columnCount(), 2);   // 末尾标签被丢弃
}

TEST_F(UT_TableWidgetExtra, SetItem_ValidCell_StoresText)
{
    t->setRowNum(1);
    DStandardItem *item = new DStandardItem("cell");
    t->setItem(0, 0, item);   // Act

    EXPECT_NE(t->mp_Table->mp_Model->item(0, 0), nullptr);       // 状态：单元格存在
    EXPECT_EQ(t->mp_Table->mp_Model->item(0, 0)->text(), QString("cell"));
}

TEST_F(UT_TableWidgetExtra, Clear_AfterItemSet_EmptiesModel)
{
    t->setRowNum(1);
    DStandardItem *item = new DStandardItem("cell");
    t->setItem(0, 0, item);
    ASSERT_EQ(t->mp_Table->mp_Model->rowCount(), 1);

    t->clear();   // Act：模型接管 item，clear 时统一析构

    EXPECT_EQ(t->mp_Table->mp_Model->rowCount(), 0);      // 状态：行清空
    EXPECT_EQ(t->mp_Table->mp_Model->columnCount(), 0);   // 状态：QStandardItemModel::clear 连表头一并清空
}

TEST_F(UT_TableWidgetExtra, SetRowNum_Positive_UpdatesRowCount)
{
    t->setRowNum(3);   // Act
    EXPECT_EQ(t->mp_Table->m_RowCount, 3);
    EXPECT_EQ(t->mp_Table->RowNum(), 3);

    t->setRowNum(0);   // 边界：0
    EXPECT_EQ(t->mp_Table->RowNum(), 0);
}

TEST_F(UT_TableWidgetExtra, UpdateCurItemEnable_Toggle_AddsAndRemovesDisablePrefix)
{
    t->setRowNum(1);
    DStandardItem *item = new DStandardItem("dev");
    t->setItem(0, 0, item);

    t->updateCurItemEnable(0, false);   // Act：禁用 → 前缀
    EXPECT_EQ(t->mp_Table->mp_Model->item(0, 0)->text(), QString("(Disable)dev"));

    t->updateCurItemEnable(0, true);    // Act：启用 → 去前缀
    EXPECT_EQ(t->mp_Table->mp_Model->item(0, 0)->text(), QString("dev"));
}

TEST_F(UT_TableWidgetExtra, SetColumnAverage_TwoColumns_UsesAverageWidth)
{
    // Act
    t->setColumnAverage();

    // Assert：两列等宽 = 视口宽度 / 列数
    int expected = t->mp_Table->width() / 2;
    EXPECT_EQ(t->mp_Table->columnWidth(0), expected);
    EXPECT_EQ(t->mp_Table->columnWidth(1), expected);
}

// ---------------- 信号槽（QSignalSpy） ----------------

TEST_F(UT_TableWidgetExtra, SlotActionRefresh_Triggers_EmitsRefreshInfo)
{
    QSignalSpy spy(t, &TableWidget::refreshInfo);
    t->slotActionRefresh();   // Act
    EXPECT_EQ(spy.count(), 1);   // 信号发射
}

TEST_F(UT_TableWidgetExtra, SlotActionExport_Triggers_EmitsExportInfo)
{
    QSignalSpy spy(t, &TableWidget::exportInfo);
    t->slotActionExport();   // Act
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_TableWidgetExtra, SlotActionEnable_EnableText_EmitsEnableTrue)
{
    prepareRow();
    t->mp_Enable->setText(TableWidget::tr("Enable"));
    QSignalSpy spy(t, &TableWidget::enableDevice);

    t->slotActionEnable();   // Act

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 0);   // 参数：当前行号
    EXPECT_EQ(spy.at(0).at(1).toBool(), true);   // branch: text == tr("Enable")
}

TEST_F(UT_TableWidgetExtra, SlotActionEnable_DisableText_EmitsEnableFalse)
{
    prepareRow();   // mp_Enable 默认文案 "Disable"
    QSignalSpy spy(t, &TableWidget::enableDevice);

    t->slotActionEnable();   // Act

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 0);
    EXPECT_EQ(spy.at(0).at(1).toBool(), false);   // branch: text != tr("Enable")
}

TEST_F(UT_TableWidgetExtra, SlotActionUpdateDriver_WithCurrentRow_EmitsRow)
{
    prepareRow();
    QSignalSpy spy(t, &TableWidget::installDriver);

    t->slotActionUpdateDriver();   // Act
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 0);   // 参数：mp_Table->currentRow()
}

TEST_F(UT_TableWidgetExtra, SlotActionUpdateDriver_NoCurrentRow_EmitsNegativeRow)
{
    QSignalSpy spy(t, &TableWidget::installDriver);
    t->slotActionUpdateDriver();   // Act：无当前行
    EXPECT_EQ(spy.at(0).at(0).toInt(), -1);   // 边界：currentRow() == -1
}

TEST_F(UT_TableWidgetExtra, SlotActionRemoveDriver_WithCurrentRow_EmitsRow)
{
    prepareRow();
    QSignalSpy spy(t, &TableWidget::uninstallDriver);

    t->slotActionRemoveDriver();   // Act
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 0);
}

TEST_F(UT_TableWidgetExtra, SlotWakeupMachine_Checked_EmitsRowAndState)
{
    prepareRow();
    t->mp_WakeupMachine->setCheckable(true);   // 右键菜单流程中由 slotShowMenu 置 checkable，直调槽需补上
    t->mp_WakeupMachine->setChecked(true);
    QSignalSpy spy(t, &TableWidget::wakeupMachine);

    t->slotWakeupMachine();   // Act
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 0);
    EXPECT_EQ(spy.at(0).at(1).toBool(), true);
}

TEST_F(UT_TableWidgetExtra, SlotWakeupMachine_Unchecked_EmitsFalse)
{
    prepareRow();
    t->mp_WakeupMachine->setChecked(false);
    QSignalSpy spy(t, &TableWidget::wakeupMachine);

    t->slotWakeupMachine();   // Act
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(1).toBool(), false);
}

// ---------------- slotShowMenu 分支覆盖 ----------------

TEST_F(UT_TableWidgetExtra, SlotShowMenu_NoCurrentRow_DisablesActionsAndReturns)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    // Act：无当前行（模型空）
    t->slotShowMenu(QPoint(0, 0));

    // Assert：B1 Enable/卸载置灰，但 B3 else 分支重新置亮 Enable；B5 提前返回 → 菜单无动作且未 exec
    EXPECT_TRUE(t->mp_Enable->isEnabled());   // B3 else：置回 true 并改文案
    EXPECT_EQ(t->mp_Enable->text(), TableWidget::tr("Enable"));
    EXPECT_FALSE(t->mp_removeDriver->isEnabled());   // B1 + B3 else 双重置灰
    EXPECT_FALSE(t->mp_updateDriver->isEnabled());   // B3 else
    EXPECT_EQ(t->mp_Menu->actions().count(), 0);   // 提前 return，未添加任何动作
    EXPECT_EQ(g_menuExecCount, 0);                 // 未弹出菜单
}

TEST_F(UT_TableWidgetExtra, SlotShowMenu_DriverPageRunning_DisablesDriverActions)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    g_pageRunning = true;   // B4
    stub.set(ADDR(PageDriverControl, isRunning), ut_isRunning);
    prepareRow();

    t->slotShowMenu(QPoint(0, 0));   // Act

    EXPECT_FALSE(t->mp_updateDriver->isEnabled());
    EXPECT_FALSE(t->mp_removeDriver->isEnabled());
    EXPECT_FALSE(t->mp_Enable->isEnabled());
    EXPECT_EQ(t->mp_Enable->text(), TableWidget::tr("Disable"));   // B2
    EXPECT_EQ(g_menuExecCount, 1);
}

TEST_F(UT_TableWidgetExtra, SlotShowMenu_PrinterNotInstalled_DisablesUpdate)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    prepareRow();
    // 模拟 signalCheckPrinterStatus 回报：是打印机且 dde-printer 未安装 → B6
    QObject::connect(t, &TableWidget::signalCheckPrinterStatus, t,
                     [](int, bool &isPrinter, bool &isInstalled) {
                         isPrinter = true;
                         isInstalled = false;
                     });

    t->slotShowMenu(QPoint(0, 0));   // Act

    EXPECT_FALSE(t->mp_updateDriver->isEnabled());     // B6
    EXPECT_EQ(t->mp_Menu->actions().contains(t->mp_Enable), false);   // B9：canEnable 默认 false
    EXPECT_EQ(t->mp_Menu->actions().contains(t->mp_updateDriver), false);   // B10：canUninstall 默认 false
}

TEST_F(UT_TableWidgetExtra, SlotShowMenu_CapableDevice_AddsEnableAndDriverActions)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    prepareRow("dev", {{0, "true"}, {1, "true"}});   // B7：UserRole="true" 可卸载，UserRole+1="true" 可启用
    QObject::connect(t, &TableWidget::signalCheckPrinterStatus, t,
                     [](int, bool &isPrinter, bool &isInstalled) {
                         isPrinter = false;
                         isInstalled = true;
                     });

    t->slotShowMenu(QPoint(0, 0));   // Act

    // Assert：B8 启用动作入菜单；B10 更新/卸载入菜单；文案 Disable
    EXPECT_EQ(t->mp_Menu->actions().contains(t->mp_Enable), true);
    EXPECT_EQ(t->mp_Menu->actions().contains(t->mp_updateDriver), true);
    EXPECT_EQ(t->mp_Menu->actions().contains(t->mp_removeDriver), true);
    EXPECT_TRUE(t->mp_updateDriver->isEnabled());
    EXPECT_EQ(t->mp_Enable->text(), TableWidget::tr("Disable"));   // B2
    EXPECT_EQ(g_menuExecCount, 1);
}

TEST_F(UT_TableWidgetExtra, SlotShowMenu_WakeupFileEnabled_ChecksWakeup)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    // B11b：UserRole+2="true" 键鼠唤醒，UserRole+3 普通路径文件
    // B11b：UserRole+2="true" 键鼠唤醒，UserRole+3 指向真实临时文件（内容 enabled）
    QString path = makeWakeupFile("enabled\n", "ut_t19_wakeup_enabled.txt");
    prepareRow("dev", {{2, "true"}, {3, path}});

    t->slotShowMenu(QPoint(0, 0));   // Act

    // Assert：内容不含 "disabled" → isWakeup = true
    EXPECT_TRUE(t->mp_WakeupMachine->isChecked());
    EXPECT_TRUE(t->mp_WakeupMachine->isEnabled());
    EXPECT_EQ(t->mp_Menu->actions().contains(t->mp_WakeupMachine), true);
    QFile::remove(path);
}

TEST_F(UT_TableWidgetExtra, SlotShowMenu_WakeupFileDisabled_UnchecksWakeup)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    QString path = makeWakeupFile("disabled\n", "ut_t19_wakeup_disabled.txt");
    prepareRow("dev", {{2, "true"}, {3, path}});

    t->slotShowMenu(QPoint(0, 0));   // Act

    EXPECT_FALSE(t->mp_WakeupMachine->isChecked());   // branch: info.contains("disabled")
    EXPECT_TRUE(t->mp_WakeupMachine->isEnabled());
    QFile::remove(path);
}

TEST_F(UT_TableWidgetExtra, SlotShowMenu_WakeupFileMissing_UnchecksWakeup)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    // 边界：文件打开失败 → isWakeup 保持 false
    prepareRow("dev", {{2, "true"}, {3, "/tmp/ut_t19_wakeup_no_such_file"}});

    t->slotShowMenu(QPoint(0, 0));   // Act

    EXPECT_FALSE(t->mp_WakeupMachine->isChecked());   // branch: !file.open
    EXPECT_TRUE(t->mp_WakeupMachine->isEnabled());
}

TEST_F(UT_TableWidgetExtra, SlotShowMenu_WakeupAcpiPath_QueriesInputWakeup)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    // /proc/acpi/wakeup 真实存在且可读，仅隔离 DBus 查询
    prepareRow("dev", {{2, "true"}, {3, "/proc/acpi/wakeup"}});
    g_inputWakeupRet = true;
    stub.set(ADDR(DBusWakeupInterface, isInputWakeupMachine), ut_isInputWakeupMachine);

    t->slotShowMenu(QPoint(0, 0));   // Act

    // Assert：/proc/acpi/wakeup 路径走 isInputWakeupMachine 查询
    EXPECT_EQ(g_inputWakeupCalls, 1);
    EXPECT_TRUE(t->mp_WakeupMachine->isChecked());
}

// 网络唤醒三态（B11a）：isNetworkWakeup 返回 3→勾选 4→不勾 其他→置灰
// 宏值来源：TableWidget.cpp:31-32（#define WAKEUP_OPEN 3 / #define WAKEUP_CLOSE 4）
struct NetworkWakeupCase {
    int ret;
    bool expectChecked;
    bool expectEnabled;
};

class UT_TableWidgetExtra_NetworkWakeup : public UT_TableWidgetExtra,
                                          public ::testing::WithParamInterface<NetworkWakeupCase>
{
};

TEST_P(UT_TableWidgetExtra_NetworkWakeup, SlotShowMenu_NetworkWakeup_ChecksState)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    const NetworkWakeupCase &c = GetParam();
    prepareRow("dev", {{2, "eth0"}});   // 非 "true"/"false" → 网络唤醒分支
    g_netWakeupRet = c.ret;
    stub.set(ADDR(DBusWakeupInterface, isNetworkWakeup), ut_isNetworkWakeup);

    t->slotShowMenu(QPoint(0, 0));   // Act

    EXPECT_EQ(g_netWakeupCalls, 1);
    EXPECT_EQ(t->mp_WakeupMachine->isChecked(), c.expectChecked);
    EXPECT_EQ(t->mp_WakeupMachine->isEnabled(), c.expectEnabled);
    EXPECT_EQ(t->mp_Menu->actions().contains(t->mp_WakeupMachine), true);
}

INSTANTIATE_TEST_SUITE_P(NetworkWakeupStates, UT_TableWidgetExtra_NetworkWakeup,
                         ::testing::Values(NetworkWakeupCase{3, true, true},    // WAKEUP_OPEN
                                           NetworkWakeupCase{4, false, true},   // WAKEUP_CLOSE
                                           NetworkWakeupCase{0, false, false}   // 未知状态 → 置灰
                                           ));

TEST_F(UT_TableWidgetExtra, SlotShowMenu_WakeupFalse_DisablesWakeupAndRemovesEnable)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    // B11c："false" → 唤醒置灰；B12：UserRole+3=="false" → 移除启用动作
    prepareRow("dev", {{0, "true"}, {1, "true"}, {2, "false"}, {3, "false"}});

    t->slotShowMenu(QPoint(0, 0));   // Act

    EXPECT_FALSE(t->mp_WakeupMachine->isEnabled());
    EXPECT_EQ(t->mp_Menu->actions().contains(t->mp_WakeupMachine), true);   // 动作仍添加（置灰）
    EXPECT_EQ(t->mp_Menu->actions().contains(t->mp_Enable), false);         // B12 removeAction
}

TEST_F(UT_TableWidgetExtra, SlotShowMenu_DisabledRow_DisablesWakeup)
{
    Stub *s = installMenuStub();
    Q_UNUSED(s);
    // B11d：当前行禁用（文本 "(Disable)" 前缀）→ 唤醒置灰
    QString path = makeWakeupFile("enabled\n", "ut_t19_wakeup_disabled_row.txt");
    prepareRow("(Disable)dev", {{2, "true"}, {3, path}});

    t->slotShowMenu(QPoint(0, 0));   // Act

    EXPECT_FALSE(t->mp_WakeupMachine->isEnabled());   // B11d
    EXPECT_EQ(t->mp_Menu->actions().contains(t->mp_WakeupMachine), true);
    QFile::remove(path);
}
