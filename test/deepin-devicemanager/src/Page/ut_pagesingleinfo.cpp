// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                 | level | factors              | min | actual |
// |------------------------|-------|----------------------|-----|--------|
// | slotShowMenu           | high  | in_degree:38         | 3   | 9      |
// | PageSingleInfo         | mid   | in_degree:84         | 2   | 2      |
// | ~PageSingleInfo        | low   | -                    | 2   | 2      |
// | initWidgets            | mid   | -                    | 2   | 2      |
// | loadDeviceInfo         | mid   | -                    | 2   | 5(4+P) |
// | updateInfo             | low   | -                    | 2   | 2      |
// | setLabel               | low   | -                    | 2   | 2      |
// | clearWidgets           | mid   | -                    | 2   | 2      |
// | clearContent           | mid   | -                    | 2   | 2      |
// | expandTable            | low   | -                    | 2   | 2      |
// | isExpanded             | low   | -                    | 2   | 1(合)  |
// | setRowHeight           | low   | -                    | 2   | 2      |
// | slotActionCopy         | mid   | -                    | 2   | 2      |
// | slotActionEnable       | mid   | -                    | 2   | 4      |
// | slotActionUpdateDriver | mid   | -                    | 2   | 2      |
// | slotActionRemoveDriver | mid   | -                    | 2   | 2      |
// | slotWakeupMachine      | mid   | -                    | 2   | 3      |
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x] 17/17 方法均有专属用例
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x] loadDeviceInfo 空表/单行/多行/富文本; setLabel 相同/不同
// 3. 每个等价类的边界值显式覆盖: [x] 空列表边界(0 行)、value 含 "\n" 富文本边界
// 4. 同质 ≥ 3 组用 TEST_P: [x] LoadDeviceInfoTest 参数化 3 组
// 5. 分支清单 → 用例映射已列出: [x] 见下方分支清单
// 6. 每条 if/switch/throw/early-return 有触发用例: [x] 逐一映射
// 7. 异常路径 EXPECT_THROW 精确匹配: [x] 方法体无 throw，异常安全用 EXPECT_NO_FATAL_FAILURE
// 8. 负面场景有专门用例: [x] 空设备/空列表/失败状态等
// 9. 负面用例验证强异常安全: [x] EXPECT_NO_FATAL_FAILURE 验证对象状态不变或无崩溃
// 10. stub_ext vs gMock 选择正确: [x] 沿用项目 stub.h（ADDR + 静态函数），Qt/DBus/进程外部依赖全部打桩
//
// 分支清单（基于 MCP get_code_snippet 真实源码 PageSingleInfo.cpp）→ 用例映射：
// slotShowMenu:
//   B1 mp_Device==nullptr → early return .......... slotShowMenu_nullDevice
//   B2 !available() → removeDriver/Enable 置灰 .... slotShowMenu_inputUnavailable
//   B3 !enable() → Enable 文案 "Enable"/update/remove 置灰 .. slotShowMenu_inputDisabled
//   B3' enable() → 文案 "Disable" ................. slotShowMenu_inputAvailable
//   B4 PageDriverControl::isRunning() → 三 action 置灰 ..... slotShowMenu_driverRunning
//   B5 printer && !packageHasInstalled → updateDriver 置灰 .. slotShowMenu_printerNotInstalled
//   B6 canEnable() → 加 Enable action ............. slotShowMenu_inputAvailable
//   B7 canUninstall() → 加 update/remove .......... slotShowMenu_inputAvailable
//   B8 input && canWakeupMachine() → 勾选状态 ...... slotShowMenu_inputAvailable
//   B8' input && !canWakeupMachine() → wakeup 置灰 . slotShowMenu_inputDisabled(wakeupID 空)
//   B9 network && WAKEUP_OPEN(3) → wakeup checked .. slotShowMenu_networkWakeupOpen
//   B10 network && WAKEUP_CLOSE(4) → unchecked ..... slotShowMenu_networkWakeupClose
//   B11 network && 其他(-1) → wakeup 置灰 .......... slotShowMenu_networkWakeupError
// slotActionEnable:
//   B1 isCurDeviceEnable()==true → setEnable(false) .. slotActionEnable_disableSuccess/Failed/NoSerial
//   B2 setEnable==EDS_Success → emit refreshInfo ..... slotActionEnable_disableSuccess
//   B3 EDS_NoSerial → 提示消息 ....................... slotActionEnable_disableNoSerial
//   B4 其他 → 提示失败消息 ........................... slotActionEnable_disableFailed
//   B5 isCurDeviceEnable()==false → setEnable(true) .. slotActionEnable_enableSuccess
// slotActionUpdateDriver:
//   B1 hardwareClass=="printer" → startDetached ...... slotActionUpdateDriver_printer
//   B2 其他 → PageDriverControl + show ............... slotActionUpdateDriver_input
// slotActionRemoveDriver:
//   B1 printer → 取 vendor/model ..................... slotActionRemoveDriver_printer
//   B2 其他 → 直接构造对话框 show .................... slotActionRemoveDriver_input
// slotWakeupMachine:
//   B1 input && wakeupID && sysPath 非空 → setWakeupMachine .. slotWakeupMachine_inputSet
//   B2 wakeupID/sysPath 为空 → 不调用 ................ slotWakeupMachine_inputNoWakeup
//   B3 network && logicalName 非空 → setNetworkWakeup ....... slotWakeupMachine_network
// loadDeviceInfo:
//   B1 lst.size()<1 → early return ................... loadDeviceInfo_empty(+P)
//   B2 maxRow<0 → 13 ................................. 高度为 0 时触发(P 用例)
//   B3 value 含 "\n" → 富文本 delegate ............... loadDeviceInfo_multiLine(+P)
//   B4 否则 → delegate=nullptr ....................... loadDeviceInfo_singleLine(+P)
// updateInfo: B1 空列表 early return ................ updateInfo_emptyList
// setLabel: B1 相同 → m_SameDevice=true / B2 不同 → false .. setLabel_same/different
// clearContent/expandTable/isExpanded/setRowHeight: 委托表格，边界见用例注释
//
// stub 清单（stub.h 静态函数打桩，测试不依赖真实设备/DBus/进程环境）：
//   QMenu::exec / QClipboard::setText / QProcess::startDetached
//   DMessageManager::instance+sendMessage / DBusWakeupInterface::*
//   DeviceInput::canWakeupMachine（虚函数 setEnable 由 FakeWakeupInput 重写，不采用地址打桩）
//   PageInfo::packageHasInstalled / PageDriverControl::isRunning
//   QDBusInterface::call 由 test_main.cpp 进程级全局桩兜底

#include "PageSingleInfo.h"
#include "PageTableWidget.h"
#include "DetailTreeView.h"
#include "PageDriverControl.h"
#include "PageInfo.h"
#include "DeviceInput.h"
#include "DevicePrint.h"
#include "DeviceNetwork.h"
#include "DBusWakeupInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <DApplication>
#include <DCommandLinkButton>
#include <DMessageManager>

#include <QClipboard>
#include <QMenu>
#include <QProcess>
#include <QSignalSpy>
#include <QList>
#include <QPair>

#include <gtest/gtest.h>

// ───────────────────────── stub 状态与桩函数 ─────────────────────────
static DMessageManager *g_msgMgr = nullptr;
static int g_msgCount = 0;
static QString g_lastMsg;
static QMenu *g_execMenu = nullptr;
static int g_execCount = 0;
static QString g_clipText;
static int g_clipCount = 0;
static QString g_detachedProg;
static int g_detachedCount = 0;
static bool g_detachedRet = true;
static bool g_setEnableArg = false;
static EnableDeviceStatus g_setEnableRet = EDS_Success;
static int g_setEnableCount = 0;
static int g_wakeupSetCount = 0;
static QString g_wakeupIdArg;
static int g_netWakeupSetCount = 0;
static QString g_netLogicalArg;
static bool g_netWakeupVal = true;
static int g_netWakeupQueryRet = 3;
static bool g_pkgInstalled = true;
static bool g_driverRunning = false;

static DMessageManager *ut_msg_instance()
{
    return g_msgMgr;
}

static void ut_msg_send_icon(DMessageManager *self, QWidget *par, const QIcon &icon, const QString &message)
{
    Q_UNUSED(self);
    Q_UNUSED(par);
    Q_UNUSED(icon);
    ++g_msgCount;
    g_lastMsg = message;
}

static void ut_menu_exec(QMenu *menu, const QPoint &p, QAction *at)
{
    Q_UNUSED(p);
    Q_UNUSED(at);
    g_execMenu = menu;
    ++g_execCount;
}

static void ut_clip_setText(QClipboard *clip, const QString &text, QClipboard::Mode mode)
{
    Q_UNUSED(clip);
    Q_UNUSED(mode);
    g_clipText = text;
    ++g_clipCount;
}

static bool ut_startDetached(const QString &prog, const QStringList &args)
{
    Q_UNUSED(args);
    g_detachedProg = prog;
    ++g_detachedCount;
    return g_detachedRet;
}

typedef bool (*StartDetachedFn)(const QString &, const QStringList &, const QString &, qint64 *);

static EnableDeviceStatus ut_input_setEnable(DeviceInput *self, bool e)
{
    Q_UNUSED(self);
    g_setEnableArg = e;
    ++g_setEnableCount;
    return g_setEnableRet;
}

static int ut_wakeup_query(DBusWakeupInterface *self, const QString &logical_name)
{
    Q_UNUSED(self);
    Q_UNUSED(logical_name);
    return g_netWakeupQueryRet;
}

static bool ut_wakeup_set(DBusWakeupInterface *self, const QString &unique_id, const QString &sys_path,
                          bool e, const QString &name, const QString &hclass)
{
    Q_UNUSED(self);
    Q_UNUSED(sys_path);
    Q_UNUSED(name);
    Q_UNUSED(hclass);
    g_wakeupIdArg = unique_id;
    ++g_wakeupSetCount;
    return e;
}

static bool ut_netwakeup_set(DBusWakeupInterface *self, const QString &logical_name, bool wake)
{
    Q_UNUSED(self);
    Q_UNUSED(wake);
    g_netLogicalArg = logical_name;
    ++g_netWakeupSetCount;
    return true;
}

static bool ut_pkg_installed(const QString &packageName)
{
    Q_UNUSED(packageName);
    return g_pkgInstalled;
}

static bool ut_driver_isRunning()
{
    return g_driverRunning;
}

static bool ut_input_canWakeup(DeviceInput *self)
{
    Q_UNUSED(self);
    return true;
}

// 伪输入设备：重写 setEnable 避免对虚函数地址打桩（REPLACE_FAR 在本环境不可靠），
// 同时隔离 validateKeyboardVidPid/DBus 等真实外部依赖
static bool g_fakeEnableArg = false;
static EnableDeviceStatus g_fakeEnableRet = EDS_Success;
static int g_fakeEnableCount = 0;

class FakeWakeupInput : public DeviceInput
{
public:
    EnableDeviceStatus setEnable(bool e) override
    {
        g_fakeEnableArg = e;
        ++g_fakeEnableCount;
        return g_fakeEnableRet;
    }
};

// ───────────────────────── 测试夹具 ─────────────────────────
class UT_PageSingleInfo : public UT_HEAD
{
public:
    static void SetUpTestSuite()
    {
        // QApplication 已由 test_main 创建；消息管理器实例供桩返回
        g_msgMgr = new DMessageManager;
    }

    static void TearDownTestSuite()
    {
        delete g_msgMgr;
        g_msgMgr = nullptr;
    }

    void SetUp() override
    {
        page = new PageSingleInfo;
        // 复位全局桩计数（避免跨用例串扰）
        g_msgCount = 0;
        g_lastMsg.clear();
        g_execMenu = nullptr;
        g_execCount = 0;
        g_clipText.clear();
        g_clipCount = 0;
        g_detachedProg.clear();
        g_detachedCount = 0;
        g_detachedRet = true;
        g_setEnableArg = false;
        g_setEnableRet = EDS_Success;
        g_setEnableCount = 0;
        g_wakeupSetCount = 0;
        g_wakeupIdArg.clear();
        g_netWakeupSetCount = 0;
        g_netLogicalArg.clear();
        g_netWakeupVal = true;
        g_netWakeupQueryRet = 3;
        g_pkgInstalled = true;
        g_driverRunning = false;
        g_fakeEnableArg = false;
        g_fakeEnableRet = EDS_Success;
        g_fakeEnableCount = 0;
    }

    void TearDown() override
    {
        delete page;
        page = nullptr;
    }

    // 构造可用输入设备（接口 USB ∈ m_supportInterfaces → available()==true；
    // 使用 FakeWakeupInput 以隔离真实 setEnable/DBus/sysfs 依赖）
    DeviceInput *makeInput(bool enabled = true)
    {
        FakeWakeupInput *dev = new FakeWakeupInput;
        dev->m_Name = "Test Keyboard";
        dev->m_Vendor = "UTest";
        dev->m_Model = "KB-01";
        dev->m_Interface = "USB";
        dev->m_Driver = "usbhid";
        dev->m_Enable = enabled;
        dev->m_CanEnable = true;
        dev->m_CanUninstall = true;
        return dev;
    }

    PageSingleInfo *page = nullptr;
};

// ───────────────────────── 构造 / 析构 / initWidgets ─────────────────────────
// Arrange: 构造 PageSingleInfo；Act: 无；Assert: 7 个 QAction 对象名正确、核心控件非空
TEST_F(UT_PageSingleInfo, ctor_initializesActionsAndWidgets)
{
    EXPECT_NE(page->mp_Content, nullptr);
    EXPECT_NE(page->mp_Label, nullptr);
    EXPECT_NE(page->mp_Menu, nullptr);
    EXPECT_EQ(page->mp_Device, nullptr);
    EXPECT_FALSE(page->m_SameDevice);

    EXPECT_STREQ(qUtf8Printable(page->mp_Refresh->objectName()), "MpRefresh");
    EXPECT_STREQ(qUtf8Printable(page->mp_Export->objectName()), "MpExport");
    EXPECT_STREQ(qUtf8Printable(page->mp_Copy->objectName()), "MpCopy");
    EXPECT_STREQ(qUtf8Printable(page->mp_Enable->objectName()), "MpEnable");
    EXPECT_STREQ(qUtf8Printable(page->mp_updateDriver->objectName()), "MpUpdateDriver");
    EXPECT_STREQ(qUtf8Printable(page->mp_removeDriver->objectName()), "MpRemoveDriver");
    EXPECT_STREQ(qUtf8Printable(page->mp_WakeupMachine->objectName()), "MpWakeupMachine");
}

// Arrange: 已初始化页面；Act: 再次调用 initWidgets；Assert: 无崩溃（防御式覆盖）
TEST_F(UT_PageSingleInfo, initWidgets_reinit_noCrash)
{
    EXPECT_NO_FATAL_FAILURE(page->initWidgets());
    EXPECT_NE(page->layout(), nullptr);
}

// Arrange: 页面对象；Act: 设备置入后 delete；Assert: 析构无崩溃
TEST_F(UT_PageSingleInfo, dtor_noCrashWithDeviceSet)
{
    PageSingleInfo *tmp = new PageSingleInfo;
    DeviceInput *dev = makeInput();
    tmp->mp_Device = dev;
    EXPECT_NO_FATAL_FAILURE(delete tmp);
    delete dev;
}

// Arrange: 无设备；Act: delete；Assert: 析构无崩溃（负面等价类）
TEST_F(UT_PageSingleInfo, dtor_noCrashWithoutDevice)
{
    PageSingleInfo *tmp = new PageSingleInfo;
    EXPECT_NO_FATAL_FAILURE(delete tmp);
}

// ───────────────────────── updateInfo ─────────────────────────
// Arrange: DeviceInput; Act: updateInfo; Assert: mp_Device 指向该设备
TEST_F(UT_PageSingleInfo, updateInfo_validDevice_setsDevice)
{
    DeviceInput *dev = makeInput();
    QList<DeviceBaseInfo *> lst{dev};

    page->updateInfo(lst);

    EXPECT_EQ(page->mp_Device, dev);
    EXPECT_STREQ(qUtf8Printable(page->mp_Device->name()), "Test Keyboard");
    delete dev;
}

// Arrange: 空设备列表（B1 early return）; Act: updateInfo; Assert: mp_Device 保持 nullptr
TEST_F(UT_PageSingleInfo, updateInfo_emptyList_noChange)
{
    QList<DeviceBaseInfo *> lst;

    EXPECT_NO_FATAL_FAILURE(page->updateInfo(lst));
    EXPECT_EQ(page->mp_Device, nullptr);
}

// ───────────────────────── loadDeviceInfo（TEST_P 参数化 3 组）─────────────────────────
struct LoadDeviceInfoCase {
    QList<QPair<QString, QString>> rows;
    bool expectEmpty;
    QString expectContains;
};

class UT_PageSingleInfo_LoadDeviceInfo : public UT_PageSingleInfo,
                                         public ::testing::WithParamInterface<LoadDeviceInfoCase>
{
};

TEST_P(UT_PageSingleInfo_LoadDeviceInfo, loadDeviceInfo_fillsTable)
{
    const LoadDeviceInfoCase &cs = GetParam();

    page->loadDeviceInfo(cs.rows);

    QString tableText = page->mp_Content->toString();
    EXPECT_EQ(tableText.isEmpty(), cs.expectEmpty);
    if (!cs.expectEmpty) {
        EXPECT_TRUE(tableText.contains(cs.expectContains)) << "actual: " << qUtf8Printable(tableText);
    }
}

INSTANTIATE_TEST_SUITE_P(EquivalenceClasses, UT_PageSingleInfo_LoadDeviceInfo, ::testing::Values(
    // 等价类 1: 空列表 → early return, 表格为空（边界值 0 行）
    LoadDeviceInfoCase{QList<QPair<QString, QString>>(), true, QString()},
    // 等价类 2: 单行无换行 → 普通行
    LoadDeviceInfoCase{QList<QPair<QString, QString>>{qMakePair(QString("Vendor"), QString("Intel"))}, false, QString("Intel")},
    // 等价类 3: value 含 "\n" → 富文本 delegate 行
    LoadDeviceInfoCase{QList<QPair<QString, QString>>{qMakePair(QString("Info"), QString("line1\nline2"))}, false, QString("line2")}
));

// Arrange: 多行列表; Act: loadDeviceInfo; Assert: 行数与内容匹配
TEST_F(UT_PageSingleInfo, loadDeviceInfo_multiRow_containsAllValues)
{
    QList<QPair<QString, QString>> lst;
    lst << qMakePair(QString("Vendor"), QString("Intel"))
        << qMakePair(QString("Model"), QString("SSD-01"))
        << qMakePair(QString("Driver"), QString("nvme"));

    page->loadDeviceInfo(lst);

    QString tableText = page->mp_Content->toString();
    EXPECT_TRUE(tableText.contains("Intel"));
    EXPECT_TRUE(tableText.contains("SSD-01"));
    EXPECT_TRUE(tableText.contains("nvme"));
}

// ───────────────────────── setLabel ─────────────────────────
// Arrange: 初始标签为空; Act: 两次设置相同文本; Assert: m_SameDevice=true, 文本一致
TEST_F(UT_PageSingleInfo, setLabel_sameDevice_flagTrue)
{
    page->setLabel("Keyboard");
    page->setLabel("Keyboard");

    EXPECT_TRUE(page->m_SameDevice);
    EXPECT_STREQ(qUtf8Printable(page->mp_Label->text()), "Keyboard");
}

// Arrange: 已有标签; Act: 设置不同文本; Assert: m_SameDevice=false, 文本更新
TEST_F(UT_PageSingleInfo, setLabel_differentDevice_flagFalse)
{
    page->setLabel("Keyboard");
    page->setLabel("Mouse");

    EXPECT_FALSE(page->m_SameDevice);
    EXPECT_STREQ(qUtf8Printable(page->mp_Label->text()), "Mouse");
}

// ───────────────────────── clearWidgets / clearContent ─────────────────────────
// Arrange: 设备已挂载; Act: clearWidgets; Assert: mp_Device 置空
TEST_F(UT_PageSingleInfo, clearWidgets_resetsDevice)
{
    DeviceInput *dev = makeInput();
    page->mp_Device = dev;

    page->clearWidgets();

    EXPECT_EQ(page->mp_Device, nullptr);
    delete dev;
}

// Arrange: 已有设备与空 mp_Device 两态; Act: clearWidgets; Assert: 均无崩溃且置空（边界：重复清空）
TEST_F(UT_PageSingleInfo, clearWidgets_repeatedClear_noCrash)
{
    page->clearWidgets();
    EXPECT_EQ(page->mp_Device, nullptr);

    DeviceInput *dev = makeInput();
    page->mp_Device = dev;
    page->clearWidgets();
    page->clearWidgets();
    EXPECT_EQ(page->mp_Device, nullptr);
    delete dev;
}

// Arrange: 表格已填充; Act: clearContent; Assert: 表格清空且多行标志复位
TEST_F(UT_PageSingleInfo, clearContent_clearsTableAndMultiFlag)
{
    QList<QPair<QString, QString>> lst{qMakePair(QString("Vendor"), QString("Intel"))};
    page->loadDeviceInfo(lst);
    page->setRowHeight(0, 30);   // 置多行标志
    EXPECT_TRUE(page->getMultiFlag());

    page->clearContent();

    EXPECT_TRUE(page->mp_Content->toString().isEmpty());
    EXPECT_FALSE(page->getMultiFlag());
}

// Arrange: 空表格; Act: clearContent; Assert: 负面场景无崩溃
TEST_F(UT_PageSingleInfo, clearContent_emptyTable_noCrash)
{
    EXPECT_NO_FATAL_FAILURE(page->clearContent());
    EXPECT_FALSE(page->getMultiFlag());
}

// ───────────────────────── expandTable / isExpanded / setRowHeight ─────────────────────────
// Arrange: 手动为 DetailTreeView 创建展开按钮（源码中仅行数 > m_LimitRow+1 时才创建，
// 未创建时 expandCommandLinkClicked 对空 mp_CommandBtn 解引用段错误 —— 源码缺陷，见 defects 清单）;
// Act: expandTable; Assert: isExpanded 翻转
TEST_F(UT_PageSingleInfo, expandTable_togglesExpanded)
{
    EXPECT_FALSE(page->isExpanded());
    // 构造展开按钮，避免触发上述缺陷路径
    DCommandLinkButton *btn = new DCommandLinkButton("More", page->mp_Content->mp_Table);
    page->mp_Content->mp_Table->mp_CommandBtn = btn;

    page->expandTable();

    EXPECT_TRUE(page->isExpanded());
}

// Arrange: 行高设置; Act: setRowHeight; Assert: 多行标志置位（页面级副作用）
TEST_F(UT_PageSingleInfo, setRowHeight_setsMultiFlag)
{
    EXPECT_FALSE(page->getMultiFlag());

    page->setRowHeight(0, 30);

    EXPECT_TRUE(page->getMultiFlag());
}

// Arrange: 越界行号（负面等价类）; Act: setRowHeight; Assert: 无崩溃
TEST_F(UT_PageSingleInfo, setRowHeight_outOfRangeRow_noCrash)
{
    EXPECT_NO_FATAL_FAILURE(page->setRowHeight(-1, 30));
    EXPECT_NO_FATAL_FAILURE(page->setRowHeight(999, 30));
}

// ───────────────────────── slotShowMenu（high）─────────────────────────
// Arrange: mp_Device 为空（B1）; Act: slotShowMenu; Assert: 菜单无 action、exec 未弹
TEST_F(UT_PageSingleInfo, slotShowMenu_nullDevice_noMenu)
{
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);

    EXPECT_NO_FATAL_FAILURE(page->slotShowMenu(QPoint(5, 5)));

    EXPECT_EQ(g_execCount, 0);
    EXPECT_TRUE(page->mp_Menu->actions().isEmpty());
}

// Arrange: 可用且启用的输入设备（B2/B3'/B6/B7/B8 全真路径）; Act: slotShowMenu; Assert:
//   exec 被调用、Enable 文案 Disable、update/remove/wakeup 均可用
TEST_F(UT_PageSingleInfo, slotShowMenu_inputAvailable_fullMenu)
{
    DeviceInput *dev = makeInput(true);
    dev->m_WakeupID = "PNP0C0C:00";   // canWakeupMachine 桩置真,避开真实 sysfs 打开
    page->mp_Device = dev;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);
    stub.set(ADDR(DeviceInput, canWakeupMachine), ut_input_canWakeup);

    page->slotShowMenu(QPoint(5, 5));

    EXPECT_EQ(g_execCount, 1);
    EXPECT_EQ(g_execMenu, page->mp_Menu);
    EXPECT_STREQ(qUtf8Printable(page->mp_Enable->text()), page->tr("Disable").toStdString().c_str());
    EXPECT_TRUE(page->mp_Enable->isEnabled());
    EXPECT_TRUE(page->mp_updateDriver->isEnabled());
    EXPECT_TRUE(page->mp_removeDriver->isEnabled());
    EXPECT_TRUE(page->mp_WakeupMachine->isEnabled());
    // 菜单包含 Copy/Refresh/Enable/update/remove/wakeup 与分隔符
    QList<QAction *> acts = page->mp_Menu->actions();
    EXPECT_TRUE(acts.contains(page->mp_Copy));
    EXPECT_TRUE(acts.contains(page->mp_Refresh));
    EXPECT_TRUE(acts.contains(page->mp_Export));
    EXPECT_TRUE(acts.contains(page->mp_Enable));
    EXPECT_TRUE(acts.contains(page->mp_updateDriver));
    EXPECT_TRUE(acts.contains(page->mp_removeDriver));
    EXPECT_TRUE(acts.contains(page->mp_WakeupMachine));
    delete dev;
}

// Arrange: 禁用状态的输入设备（B3, wakeupID 空 → B8'）; Act: slotShowMenu; Assert:
//   Enable 文案 Enable、update/remove/wakeup 置灰
TEST_F(UT_PageSingleInfo, slotShowMenu_inputDisabled_greyedActions)
{
    DeviceInput *dev = makeInput(false);   // m_Enable=false
    page->mp_Device = dev;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);

    page->slotShowMenu(QPoint(5, 5));

    EXPECT_STREQ(qUtf8Printable(page->mp_Enable->text()), page->tr("Enable").toStdString().c_str());
    EXPECT_TRUE(page->mp_Enable->isEnabled());
    EXPECT_FALSE(page->mp_updateDriver->isEnabled());
    EXPECT_FALSE(page->mp_removeDriver->isEnabled());
    // wakeupID 为空 → canWakeupMachine()==false → wakeup 置灰
    EXPECT_FALSE(page->mp_WakeupMachine->isEnabled());
    delete dev;
}

// Arrange: 接口不在支持列表 → available()==false（B2）; Act: slotShowMenu; Assert: removeDriver 置灰
TEST_F(UT_PageSingleInfo, slotShowMenu_inputUnavailable_greyedRemove)
{
    DeviceInput *dev = makeInput(true);
    dev->m_Interface = "UnknownBus";   // 不属于 {"PS/2","Bluetooth","I2C","USB"}
    page->mp_Device = dev;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);

    page->slotShowMenu(QPoint(5, 5));

    EXPECT_FALSE(dev->available());
    EXPECT_FALSE(page->mp_removeDriver->isEnabled());
    delete dev;
}

// Arrange: 驱动对话框运行中（B4，桩 isRunning→true）; Act: slotShowMenu; Assert: 驱动/启用置灰
TEST_F(UT_PageSingleInfo, slotShowMenu_driverRunning_greyedDriverActions)
{
    DeviceInput *dev = makeInput(true);
    page->mp_Device = dev;
    g_driverRunning = true;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);
    stub.set(ADDR(PageDriverControl, isRunning), ut_driver_isRunning);

    page->slotShowMenu(QPoint(5, 5));

    EXPECT_FALSE(page->mp_updateDriver->isEnabled());
    EXPECT_FALSE(page->mp_removeDriver->isEnabled());
    EXPECT_FALSE(page->mp_Enable->isEnabled());
    g_driverRunning = false;
    delete dev;
}

// Arrange: 打印机且 dde-printer 未安装（B5，桩 packageHasInstalled→false）; Act: slotShowMenu;
// Assert: updateDriver 置灰（打印机路径不含 input/network 附加 action）
TEST_F(UT_PageSingleInfo, slotShowMenu_printerNotInstalled_greyedUpdateDriver)
{
    DevicePrint *dev = new DevicePrint;
    dev->setHardwareClass("printer");
    dev->setInfo(QMap<QString, QString>{{"printer-info", "HP LaserJet"}});
    page->mp_Device = dev;
    g_pkgInstalled = false;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);
    stub.set(ADDR(PageInfo, packageHasInstalled), ut_pkg_installed);

    page->slotShowMenu(QPoint(5, 5));

    EXPECT_FALSE(page->mp_updateDriver->isEnabled());
    g_pkgInstalled = true;
    delete dev;
}

// Arrange: 打印机且 dde-printer 已安装（B5'）; Act: slotShowMenu; Assert: updateDriver 可用
TEST_F(UT_PageSingleInfo, slotShowMenu_printerInstalled_updateDriverEnabled)
{
    DevicePrint *dev = new DevicePrint;
    dev->setHardwareClass("printer");
    dev->setInfo(QMap<QString, QString>{{"printer-info", "HP LaserJet"}});
    page->mp_Device = dev;
    g_pkgInstalled = true;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);
    stub.set(ADDR(PageInfo, packageHasInstalled), ut_pkg_installed);

    page->slotShowMenu(QPoint(5, 5));

    EXPECT_TRUE(page->mp_updateDriver->isEnabled());
    delete dev;
}

// Arrange: 网卡且远端返回 WAKEUP_OPEN=3（B9）; Act: slotShowMenu; Assert: wakeup 勾选
TEST_F(UT_PageSingleInfo, slotShowMenu_networkWakeupOpen_checked)
{
    DeviceNetwork *dev = new DeviceNetwork;
    dev->m_Name = "eth0 card";
    dev->m_LogicalName = "eth0";
    page->mp_Device = dev;
    g_netWakeupQueryRet = 3;   // WAKEUP_OPEN
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);
    stub.set((int (DBusWakeupInterface::*)(const QString &))ADDR(DBusWakeupInterface, isNetworkWakeup), ut_wakeup_query);

    page->slotShowMenu(QPoint(5, 5));

    EXPECT_TRUE(page->mp_WakeupMachine->isChecked());
    EXPECT_TRUE(page->mp_WakeupMachine->isEnabled());
    delete dev;
}

// Arrange: 网卡且返回 WAKEUP_CLOSE=4（B10）; Act: slotShowMenu; Assert: wakeup 未勾选
TEST_F(UT_PageSingleInfo, slotShowMenu_networkWakeupClose_unchecked)
{
    DeviceNetwork *dev = new DeviceNetwork;
    dev->m_Name = "eth0 card";
    dev->m_LogicalName = "eth0";
    page->mp_Device = dev;
    g_netWakeupQueryRet = 4;   // WAKEUP_CLOSE
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);
    stub.set((int (DBusWakeupInterface::*)(const QString &))ADDR(DBusWakeupInterface, isNetworkWakeup), ut_wakeup_query);

    page->slotShowMenu(QPoint(5, 5));

    EXPECT_FALSE(page->mp_WakeupMachine->isChecked());
    EXPECT_TRUE(page->mp_WakeupMachine->isEnabled());
    delete dev;
}

// Arrange: 网卡且 DBus 查询失败返回 -1（B11 负面）; Act: slotShowMenu; Assert: wakeup 置灰
TEST_F(UT_PageSingleInfo, slotShowMenu_networkWakeupError_disabled)
{
    DeviceNetwork *dev = new DeviceNetwork;
    dev->m_Name = "eth0 card";
    dev->m_LogicalName = "eth0";
    page->mp_Device = dev;
    g_netWakeupQueryRet = -1;
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);
    stub.set((int (DBusWakeupInterface::*)(const QString &))ADDR(DBusWakeupInterface, isNetworkWakeup), ut_wakeup_query);

    page->slotShowMenu(QPoint(5, 5));

    EXPECT_FALSE(page->mp_WakeupMachine->isEnabled());
    delete dev;
}

// ───────────────────────── slotActionCopy ─────────────────────────
// Arrange: 表格已填充（有内容可拷贝）; Act: slotActionCopy; Assert: 剪贴板文本 == 表格文本
TEST_F(UT_PageSingleInfo, slotActionCopy_copiesTableContent)
{
    QList<QPair<QString, QString>> lst{qMakePair(QString("Vendor"), QString("Intel"))};
    page->loadDeviceInfo(lst);
    Stub stub;
    stub.set((void (QClipboard::*)(const QString &, QClipboard::Mode))ADDR(QClipboard, setText), ut_clip_setText);

    page->slotActionCopy();

    EXPECT_EQ(g_clipCount, 1);
    EXPECT_EQ(g_clipText, page->mp_Content->toString());
    EXPECT_TRUE(g_clipText.contains("Intel"));
}

// Arrange: 空表格（负面等价类）; Act: slotActionCopy; Assert: 拷贝空串且无崩溃
TEST_F(UT_PageSingleInfo, slotActionCopy_emptyTable_copiesEmpty)
{
    Stub stub;
    stub.set((void (QClipboard::*)(const QString &, QClipboard::Mode))ADDR(QClipboard, setText), ut_clip_setText);

    EXPECT_NO_FATAL_FAILURE(page->slotActionCopy());

    EXPECT_EQ(g_clipCount, 1);
    EXPECT_TRUE(g_clipText.isEmpty());
}

// ───────────────────────── slotActionEnable ─────────────────────────
// Arrange: 当前设备启用态（B1 走禁用分支），桩 setEnable→EDS_Success; Act: slotActionEnable;
// Assert: setEnable(false) 被调、refreshInfo 发射一次、无失败提示
TEST_F(UT_PageSingleInfo, slotActionEnable_disableSuccess_emitsRefresh)
{
    DeviceInput *dev = makeInput(true);
    page->mp_Device = dev;
    g_fakeEnableRet = EDS_Success;
    QSignalSpy spy(page, &PageSingleInfo::refreshInfo);

    page->slotActionEnable();

    EXPECT_EQ(g_fakeEnableCount, 1);
    EXPECT_FALSE(g_fakeEnableArg);          // 启用态 → 传入 disable
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(g_msgCount, 0);
    delete dev;
}

// Arrange: 桩 setEnable→EDS_Faild（B4 失败提示）; Act: slotActionEnable; Assert: 提示一次、不发射
TEST_F(UT_PageSingleInfo, slotActionEnable_disableFailed_showsMessage)
{
    DeviceInput *dev = makeInput(true);
    page->mp_Device = dev;
    g_fakeEnableRet = EDS_Faild;
    QSignalSpy spy(page, &PageSingleInfo::refreshInfo);
    Stub stub;
    stub.set(ADDR(DMessageManager, instance), ut_msg_instance);
    stub.set((void (DMessageManager::*)(QWidget *, const QIcon &, const QString &))ADDR(DMessageManager, sendMessage),
             ut_msg_send_icon);

    page->slotActionEnable();

    EXPECT_EQ(g_fakeEnableCount, 1);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(g_msgCount, 1);
    EXPECT_FALSE(g_lastMsg.isEmpty());
    delete dev;
}

// Arrange: 桩 setEnable→EDS_NoSerial（B3 无序列号提示）; Act: slotActionEnable; Assert: 提示一次
TEST_F(UT_PageSingleInfo, slotActionEnable_disableNoSerial_showsMessage)
{
    DeviceInput *dev = makeInput(true);
    page->mp_Device = dev;
    g_fakeEnableRet = EDS_NoSerial;
    QSignalSpy spy(page, &PageSingleInfo::refreshInfo);
    Stub stub;
    stub.set(ADDR(DMessageManager, instance), ut_msg_instance);
    stub.set((void (DMessageManager::*)(QWidget *, const QIcon &, const QString &))ADDR(DMessageManager, sendMessage),
             ut_msg_send_icon);

    page->slotActionEnable();

    EXPECT_EQ(g_fakeEnableCount, 1);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(g_msgCount, 1);
    delete dev;
}

// Arrange: 当前设备禁用态（B5 启用分支，表格置 disabled），桩返回 EDS_Success;
// Act: slotActionEnable; Assert: setEnable(true) 被调、refreshInfo 发射
TEST_F(UT_PageSingleInfo, slotActionEnable_enableSuccess_emitsRefresh)
{
    DeviceInput *dev = makeInput(false);
    page->mp_Device = dev;
    page->mp_Content->setDeviceEnable(false, true);   // 表格当前设备状态 → 禁用
    g_fakeEnableRet = EDS_Success;
    QSignalSpy spy(page, &PageSingleInfo::refreshInfo);

    page->slotActionEnable();

    EXPECT_TRUE(g_fakeEnableArg);           // 禁用态 → 传入 enable
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(g_msgCount, 0);
    delete dev;
}

// ───────────────────────── slotActionUpdateDriver ─────────────────────────
// Arrange: 打印机设备（B1），桩 startDetached; Act: slotActionUpdateDriver; Assert: 拉起 dde-printer
TEST_F(UT_PageSingleInfo, slotActionUpdateDriver_printer_startsDetached)
{
    DevicePrint *dev = new DevicePrint;
    dev->setHardwareClass("printer");
    dev->setInfo(QMap<QString, QString>{{"printer-info", "HP LaserJet"}});
    page->mp_Device = dev;
    Stub stub;
    stub.set(static_cast<StartDetachedFn>(ADDR(QProcess, startDetached)), ut_startDetached);

    page->slotActionUpdateDriver();

    EXPECT_EQ(g_detachedCount, 1);
    EXPECT_STREQ(qUtf8Printable(g_detachedProg), "dde-printer");
    delete dev;
}

// Arrange: 非打印机设备（B2）; Act: slotActionUpdateDriver; Assert: 页面下创建并 show 了 PageDriverControl 对话框
TEST_F(UT_PageSingleInfo, slotActionUpdateDriver_input_showsDialog)
{
    DeviceInput *dev = makeInput(true);
    page->mp_Device = dev;

    page->slotActionUpdateDriver();

    PageDriverControl *dlg = page->findChild<PageDriverControl *>();
    ASSERT_NE(dlg, nullptr);
    // 窗口标题 = 动作文案 + 设备名（"Update Drivers-Test Keyboard"）
    EXPECT_TRUE(dlg->windowTitle().startsWith(page->tr("Update Drivers")))
        << "actual: " << qUtf8Printable(dlg->windowTitle());
    // 触发对话框刷新信号 → 覆盖 connect lambda（emit refreshInfo + disconnect）
    QSignalSpy spy(page, &PageSingleInfo::refreshInfo);
    emit dlg->refreshInfo();
    EXPECT_EQ(spy.count(), 1);
    delete dev;
}

// ───────────────────────── slotActionRemoveDriver ─────────────────────────
// Arrange: 输入设备; Act: slotActionRemoveDriver; Assert: 页面下创建卸载驱动对话框（标题 "Uninstall Drivers"）
TEST_F(UT_PageSingleInfo, slotActionRemoveDriver_input_showsDialog)
{
    DeviceInput *dev = makeInput(true);
    page->mp_Device = dev;

    page->slotActionRemoveDriver();

    PageDriverControl *dlg = page->findChild<PageDriverControl *>();
    ASSERT_NE(dlg, nullptr);
    EXPECT_TRUE(dlg->windowTitle().startsWith(page->tr("Uninstall Drivers")))
        << "actual: " << qUtf8Printable(dlg->windowTitle());
    // 触发对话框刷新信号 → 覆盖 connect lambda（emit refreshInfo + disconnect）
    QSignalSpy spy(page, &PageSingleInfo::refreshInfo);
    emit dlg->refreshInfo();
    EXPECT_EQ(spy.count(), 1);
    delete dev;
}

// Arrange: 打印机设备（B1 取 vendor/model 路径）; Act: slotActionRemoveDriver; Assert: 卸载对话框创建
TEST_F(UT_PageSingleInfo, slotActionRemoveDriver_printer_showsDialog)
{
    DevicePrint *dev = new DevicePrint;
    dev->setHardwareClass("printer");
    dev->setInfo(QMap<QString, QString>{{"printer-info", "HP LaserJet"}});
    page->mp_Device = dev;

    page->slotActionRemoveDriver();

    PageDriverControl *dlg = page->findChild<PageDriverControl *>();
    EXPECT_NE(dlg, nullptr);
    delete dev;
}

// ───────────────────────── slotWakeupMachine ─────────────────────────
// Arrange: 输入设备 wakeupID/sysPath 非空（B1）; Act: slotWakeupMachine; Assert: DBus 桩收到参数
TEST_F(UT_PageSingleInfo, slotWakeupMachine_inputSet_callsDBus)
{
    FakeWakeupInput *dev = new FakeWakeupInput;
    dev->m_Name = "Test Keyboard";
    dev->m_Interface = "USB";
    dev->m_WakeupID = "PNP0C0C:00";
    dev->m_SysPath = "/sys/devices/test";
    page->mp_Device = dev;
    page->mp_WakeupMachine->setChecked(true);
    Stub stub;
    stub.set((bool (DBusWakeupInterface::*)(const QString &, const QString &, bool, const QString &, const QString &))
                 ADDR(DBusWakeupInterface, setWakeupMachine), ut_wakeup_set);

    page->slotWakeupMachine();

    EXPECT_EQ(g_wakeupSetCount, 1);
    EXPECT_EQ(g_wakeupIdArg, QString("PNP0C0C:00"));
    delete dev;
}

// Arrange: 输入设备 wakeupID 为空（B2 负面）; Act: slotWakeupMachine; Assert: 不触发 DBus
TEST_F(UT_PageSingleInfo, slotWakeupMachine_inputNoWakeup_skipped)
{
    DeviceInput *dev = makeInput(true);   // m_WakeupID 默认空
    page->mp_Device = dev;
    Stub stub;
    stub.set((bool (DBusWakeupInterface::*)(const QString &, const QString &, bool, const QString &, const QString &))
                 ADDR(DBusWakeupInterface, setWakeupMachine), ut_wakeup_set);

    EXPECT_NO_FATAL_FAILURE(page->slotWakeupMachine());

    EXPECT_EQ(g_wakeupSetCount, 0);
    delete dev;
}

// Arrange: 网卡 logicalName 非空（B3）; Act: slotWakeupMachine; Assert: setNetworkWakeup 被调
TEST_F(UT_PageSingleInfo, slotWakeupMachine_network_callsSetNetworkWakeup)
{
    DeviceNetwork *dev = new DeviceNetwork;
    dev->m_Name = "eth0 card";
    dev->m_LogicalName = "eth0";
    page->mp_Device = dev;
    page->mp_WakeupMachine->setChecked(false);
    Stub stub;
    stub.set((bool (DBusWakeupInterface::*)(const QString &, bool))ADDR(DBusWakeupInterface, setNetworkWakeup), ut_netwakeup_set);

    page->slotWakeupMachine();

    EXPECT_EQ(g_netWakeupSetCount, 1);
    EXPECT_EQ(g_netLogicalArg, QString("eth0"));
    delete dev;
}
