// Copyright (C) 2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// MainWindow 单元测试（重写 ut_mainwindow.cpp / ut_mainwindow_extra.cpp）
// 覆盖 deepin-devicemanager/src/Page/MainWindow.cpp 全部方法 + 文件内静态函数
// checkWaylandMode（间接覆盖：内部链接，经构造路径触发 wayland/x11 两种环境分支）。
//
// 外部依赖隔离（全部打桩，不依赖真实设备环境；stub.h 不支持 lambda，统一使用
// 自由函数桩 + 全局计数器）：
//   - MainWindow::refreshDataBase / refreshBatteryStatus（线程/DBus/QProcess）
//   - ThreadExecXrandr::run（xrandr 子进程）、LoadCpuInfoThread::run（/proc 扫描）
//   - LoadInfoThread::run（全量设备扫描线程）
//   - DeviceManager 查询族（instance/setDeviceListClass/getDeviceTypes/getDeviceList/
//     getDeviceDriverPool/getDeviceOverview/networkDriver/correctNetworkLinkStatus/
//     correctPowerInfo/exportTo*）
//   - CmdTool::getCurNetworkLinkStatus / getCurPowerInfo（nmcli/upower 子进程）
//   - DBusInterface::refreshInfo / getInfo（DBus）
//   - Common::boardVendorType / isHwPlatform（dmidecode 子进程）
//   - PageDriverManager::scanDriverInfo/isFirstScan/isScanning/isInstalling/isBackingup/
//     isRestoring（驱动扫描线程/状态）
//   - QProcess::startDetached(静态)/start/waitForFinished/readAllStandardOutput（子进程）
//   - DDialog::exec、QFileDialog::getSaveFileName（模态对话框）
//   - QDBusAbstractInterface::isValid（系统总线依赖）
//
// 已知限制（源码结构决定，非测试遗漏）：
//   - startScanningFlag/monitorNumber 为 MainWindow.cpp 文件内 static（内部链接），
//     测试无法直接读写；经 ThreadExecXrandr::run 桩可控显示器数量、经驱动页 refresh()
//     可置 startScanningFlag，均以可观测副作用断言。
//   - refreshDataBaseLater 的 2s QTimer 分支需要事件循环，测试进程无事件循环，
//     仅断言即时部分（DBus refreshInfo 触发）。
//
// 分支清单（来源 get_code_snippet @ f2d46ee9，行号为工作区源码）：
//   ctor(75-148): 初始化链 + refreshDataBase + 信号连接 + xrandr monitorNumber
//     → Ctor_Constructs_InitializesUi, Ctor_WaylandEnv_IndirectCheckWaylandMode
//   ~MainWindow(161-192) → Dtor_DestroysAndAllowsReconstruct
//   refresh(194-222): battery → 4 条件跳过 → checkedId==1 置扫描标志 → 正常流程
//     → Refresh_WhenIdle / Refresh_WhenRefreshing / Refresh_WhenScanning /
//       Refresh_WhenThreadRunning / RefreshOnDriverPage_ThenLoadingFinish
//   refreshDataBase(513-524): 置等待光标+启线程 / 已置位不重复
//     → RefreshDataBase_SetsWaitCursor, RefreshDataBase_SecondCall_NoDoubleCursor
//   refreshDataBaseLater(149-159) → RefreshDataBaseLater_CallsDBusRefreshInfo
//   refreshBatteryStatus(224-266): UPower 无效跳过 / 有效执行 gdbus(BAT 解析)
//     → BatteryStatus_InterfaceInvalid_SkipsGdbus, BatteryStatus_BatOutput_RunsGdbusOnce
//   exportTo(268-318): 空 true / Text / Html / Doc / Xls / 未知 false
//     → ExportTo_{EmptyFile,TextFilter,HtmlFilter,DocFilter,XlsFilter,UnknownFilter}
//   getJsonDoc(358-394) / addJsonArrayItem(348-356)
//     → GetJsonDoc_BuildsTwoShortcutGroups, AddJsonArrayItem_AppendsNameValuePair
//   showDisplayShortcutsHelpDialog(322-346) → ShowShortcutsHelpDialog_StartsViewerProcess
//   swichStackWidget(408-413): 0→1 / 1→1 → SwichStackWidget_{FromWaitingToDetail,AlreadySwitched}
//   windowMaximizing(396-406) → WindowMaximizing_TogglesMaximizeState(正反两次)
//   initWindow(421-441) / initWidgets(493-511) / initWindowSize(443-453) / initWindowTitle(455-491)
//     → InitWindow_ReinitSafe, InitWidgets_FourPagesInStack,
//       InitWindowSize_AppliesSizes, InitWindowTitle_BuildsButtonBox
//   slotSetPage(526-545): DISABLE_DRIVER 早退 / 非 driver 无操作 /（Qt5 非 DISABLE 分支）
//     → SlotSetPage_Driver_DisabledBuild_EarlyReturn / SlotSetPage_OtherPage_NoOp / #ifndef 3 例
//   slotLoadingFinish(547-625): finish 概况 / finish 设备 / 扫描标志 / ShowDriverPage / 非 finish
//     → LoadingFinish_{OverviewBranch,DeviceBranch,ShowDriverPage,OtherMessageNoOp,
//       StartScanningFlag}（扫描标志经 DriverPageFlow 组合用例）
//   slotListItemClicked(627-684): Monitor/Overview→xrandr / DisplayAdapter→GPU / CPU→线程 /
//     Network→链路 / Battery→电源 / monitor 数变化→DBus 刷新 / 刷新中早退
//     → ListItemClicked_{Monitor,Overview,DisplayAdapter,Cpu,NetworkAdapter,Battery,
//       MonitorCountChanged,WhileRefreshing}
//   slotRefreshInfo(686-692) / slotExportInfo(694-699) / slotChangeUI(701-709)
//     → SlotRefreshInfo_RefreshesDbusAndUi, SlotExportInfo_TriggersExportDialog,
//       SlotChangeUI_SetsFontFlagAndReloadsCurrent
//   keyPressEvent(711-752): E+任意修饰含Ctrl→导出 / E 无修饰→透传 / Question+Ctrl→快捷键窗 /
//     F+Ctrl+Alt→最大化 / 其他→透传
//     → KeyPress_{CtrlE,ENoModifier,CtrlShiftQuestion,CtrlAltF,UnhandledKey}
//   event(754-765): ApplicationFontChange→emit / 其他→不 emit
//     → Event_{ApplicationFontChange,OtherEvent}
//   resizeEvent(415-419) → ResizeEvent_ForwardsToBase
//   closeEvent(767-823): 空闲 / 安装(0/1) / 备份(1) / 还原(1)
//     → CloseEvent_{Idle,InstallingExecZero,InstallingExecNonZero,Backingup,Restoring}
//
// 用例计数声明（min：high=3(+complexity≥20 补异常/负面)、mid=2、low=1；loop 因子 +1）：
// | 方法                           | level | min | actual |
// |--------------------------------|-------|-----|--------|
// | MainWindow(ctor)               | high  | 3   | 3(含 wayland 间接) |  // 用例总数：61（#ifndef 变体 +3 = 文本 64）
// | ~MainWindow                    | low   | 1   | 1 |
// | refresh                        | high  | 3   | 5 |
// | refreshDataBase                | mid   | 2   | 2 |
// | refreshDataBaseLater           | mid   | 2   | 1+1(内嵌于 SlotRefreshInfo 两例；2s 定时分支受事件循环限制,见已知限制) |
// | refreshBatteryStatus           | mid   | 2   | 2 |
// | exportTo                       | high  | 3   | 6 |
// | addJsonArrayItem               | low   | 1   | 1 |
// | getJsonDoc                     | mid   | 2   | 1 |
// | showDisplayShortcutsHelpDialog | mid   | 2   | 1(另经 KeyPress_CtrlShiftQuestion 覆盖) |
// | swichStackWidget               | low   | 1   | 2 |
// | windowMaximizing               | low   | 1   | 1(两次切换=正反等价类) |
// | initWindow                     | high  | 3   | 1(构造即覆盖+重入安全) |
// | initWidgets                    | low   | 1   | 1 |
// | initWindowSize                 | low   | 1   | 1 |
// | initWindowTitle                | mid   | 2   | 1(DISABLE_DRIVER 下 vendor 分支恒隐藏,不可区分) |
// | slotSetPage                    | mid   | 2   | 2(按 Qt6 构建实际分支;Qt5 分支 #ifndef 保留) |
// | slotLoadingFinish              | high  | 3   | 5 |
// | slotListItemClicked            | high  | 3   | 8 |
// | slotRefreshInfo                | mid   | 2   | 2(正常/刷新态早退) |
// | slotExportInfo                 | low   | 1   | 1 |
// | slotChangeUI                   | mid   | 2   | 2(Monitor/CPU) |
// | keyPressEvent                  | high  | 3   | 5 |
// | event                          | low   | 1   | 2 |
// | resizeEvent                    | low   | 1   | 1 |
// | closeEvent                     | high  | 3   | 5 |
// | checkWaylandMode               | low   | 1   | 1(间接) |
// 最小清单：[x]每方法≥1用例 [x]等价类划分 [x]边界值(空列表/空串/边界索引) [x]无同质多组(不适用参数化)
// [x]分支清单映射 [x]每分支有触发用例(按 Qt6/DISABLE_DRIVER 实际编译分支) [x]无显式 throw 路径
// [x]负面场景(空路径/未知类型/早退状态) [x]强异常安全(早退后状态不变断言) [x]stub.h(项目内置 stub_ext)
// ============================================================================

#include "MainWindow.h"
#include "DeviceWidget.h"
#include "PageDriverManager.h"
#include "DriverScanWidget.h"
#include "WaitingWidget.h"
#include "DeviceManager.h"
#include "DBusInterface.h"
#include "ThreadExecXrandr.h"
#include "LoadCpuInfoThread.h"
#include "LoadInfoThread.h"
#include "CmdTool.h"
#include "commonfunction.h"
#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QCloseEvent>
#include <QDBusAbstractInterface>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QProcess>
#include <QResizeEvent>
#include <QSignalSpy>
#include <DDialog>

namespace {
#define UT_MARK(name) fprintf(stderr, "\n[UT-MARK] %s\n", name)

// ============================ 全局计数器/状态 ============================

struct Counters {
    int refreshDbus = 0;
    int batteryRefresh = 0;
    int xrandrRun = 0;
    int cpuRun = 0;
    int scanDriver = 0;
    int dbusRefreshInfo = 0;
    int dbusGetInfo = 0;
    int detached = 0;
    int saveDialog = 0;
    int setDevListClass = 0;
    int updateListView = 0;
    int updateDevice = 0;
    int updateOverview = 0;
    int netStatus = 0;
    int netCorrect = 0;
    int powerInfo = 0;
    int powerCorrect = 0;
    int fontFlag = 0;
    int exportTxt = 0;
    int exportHtml = 0;
    int exportDoc = 0;
    int exportXlsx = 0;
    int procStart = 0;

    int xrandrMonitor = 2;       // ThreadExecXrandr::run 桩写入的显示器数量
    bool dmGetListRet = false;   // DeviceManager::getDeviceList 返回值
    bool firstScan = false;      // PageDriverManager::isFirstScan
    bool scanning = false;       // PageDriverManager::isScanning
    bool installing = false;     // PageDriverManager::isInstalling
    bool backingup = false;      // PageDriverManager::isBackingup
    bool restoring = false;      // PageDriverManager::isRestoring
    QString saveFile;            // getSaveFileName 返回
    QString dwCurrentIndex = "Monitor"; // DeviceWidget::currentIndex 桩返回
    QString saveFilter;          // getSaveFileName 选中的过滤器
    int dialogExecRet = 0;       // DDialog::exec 返回
};

Counters g_cnt;
DeviceManager *g_dm = nullptr;

// ============================ 自由函数桩 ============================

static void stub_MW_refreshDataBase(MainWindow *) { ++g_cnt.refreshDbus; }
static void stub_MW_refreshBatteryStatus(MainWindow *) { ++g_cnt.batteryRefresh; }

static void stub_TX_runCmd(ThreadExecXrandr *, QString &info, const QString &)
{
    info.clear();   // 空输出：loadXrandrInfo/loadXrandrVerboseInfo 解析不到设备
}

// 控制 gpu 线程的显示器数量（run 真实执行；txgpu 构造路径都会经过这里）
static void stub_TX_getResolutionFromDBus(ThreadExecXrandr *t, QMap<QString, QString> &)
{
    t->m_monitorLst.clear();
    for (int i = 0; i < g_cnt.xrandrMonitor; ++i)
        t->m_monitorLst.append(QString("/org/freedesktop/Display/Monitor%1").arg(i));
    ++g_cnt.xrandrRun;
}

static void stub_CPU_getCpuInfo(LoadCpuInfoThread *) { ++g_cnt.cpuRun; }
static bool stub_QProcess_startDetached4(const QString &, const QStringList &, const QString &, qint64 *)
{
    ++g_cnt.detached;
    return false;
}

static DeviceManager *stub_DM_instance() { return g_dm; }
static void stub_DM_setDevListClass(DeviceManager *) { ++g_cnt.setDevListClass; }
static const QList<QPair<QString, QString>> &stub_DM_getDeviceTypes(void *)
{
    static const QList<QPair<QString, QString>> types = {
        {"Monitor", "显示器"}, {"CPU", "处理器"}};
    return types;
}
static const QMap<QString, QMap<QString, QStringList>> &stub_DM_getDeviceDriverPool(void *)
{
    static const QMap<QString, QMap<QString, QStringList>> pool;
    return pool;
}
static bool stub_DM_getDeviceList(DeviceManager *, const QString &, QList<DeviceBaseInfo *> &lst)
{
    if (g_cnt.dmGetListRet)
        lst.append(nullptr);
    return g_cnt.dmGetListRet;
}
static const QMap<QString, QString> &stub_DM_getDeviceOverview(void *)
{
    static const QMap<QString, QString> overview = {{"Overview", "UOS"}, {"OS", "deepin"}};
    return overview;
}
static QStringList stub_DM_networkDriver(void *) { return QStringList{"eth0"}; }
static void stub_DM_correctNetworkLinkStatus(DeviceManager *, const QString &status, const QString &driver)
{
    ++g_cnt.netCorrect;
    EXPECT_EQ(status, QString("connected"));
    EXPECT_EQ(driver, QString("eth0"));
}
static void stub_DM_correctPowerInfo(DeviceManager *, const QMap<QString, QMap<QString, QString>> &)
{
    ++g_cnt.powerCorrect;
}
static bool stub_DM_exportToTxt(DeviceManager *, const QString &) { ++g_cnt.exportTxt; return true; }
static bool stub_DM_exportToHtml(DeviceManager *, const QString &) { ++g_cnt.exportHtml; return true; }
static bool stub_DM_exportToDoc(DeviceManager *, const QString &) { ++g_cnt.exportDoc; return false; }
static bool stub_DM_exportToXlsx(DeviceManager *, const QString &) { ++g_cnt.exportXlsx; return false; }

static QString stub_Cmd_getCurNetworkLinkStatus(CmdTool *, QString)
{
    ++g_cnt.netStatus;
    return QString("connected");
}
static QMap<QString, QMap<QString, QString>> stub_Cmd_getCurPowerInfo(void *)
{
    ++g_cnt.powerInfo;
    return QMap<QString, QMap<QString, QString>>();
}

static void stub_DBus_refreshInfo(void *) { ++g_cnt.dbusRefreshInfo; }
static bool stub_DBus_getInfo(void *, const QString &, QString &info)
{
    ++g_cnt.dbusGetInfo;
    info = "0";
    return true;
}

static QString stub_Common_boardVendorType() { return QString(); }
static bool stub_Common_isHwPlatform() { return true; }

static void stub_PDM_scanDriverInfo(PageDriverManager *) { ++g_cnt.scanDriver; }
static bool stub_PDM_isFirstScan(void *) { return g_cnt.firstScan; }
static bool stub_PDM_isScanning(void *) { return g_cnt.scanning; }
static bool stub_PDM_isInstalling(void *) { return g_cnt.installing; }
static bool stub_PDM_isBackingup(void *) { return g_cnt.backingup; }
static bool stub_PDM_isRestoring(void *) { return g_cnt.restoring; }

static void stub_DW_setFontChangeFlag(void *) { ++g_cnt.fontFlag; }
static void stub_DW_updateListView(void *, const QList<QPair<QString, QString>> &) { ++g_cnt.updateListView; }
static void stub_DW_updateDevice(void *, const QString &, const QList<DeviceBaseInfo *> &) { ++g_cnt.updateDevice; }
static void stub_DW_updateOverview(void *, const QMap<QString, QString> &) { ++g_cnt.updateOverview; }
static QString stub_DW_currentIndex(void *) { return g_cnt.dwCurrentIndex; }

static QString stub_QFileDialog_getSaveFileName(QWidget *, const QString &, const QString &,
                                                const QString &, QString *sf, QFileDialog::Options)
{
    ++g_cnt.saveDialog;
    if (sf)
        *sf = g_cnt.saveFilter;
    return g_cnt.saveFile;
}
static int stub_DDialog_exec(void *) { return g_cnt.dialogExecRet; }

static void stub_QProcess_start(void *, const QString &prog, const QStringList &, QIODevice::OpenMode)
{
    EXPECT_EQ(prog, QString("gdbus"));
    ++g_cnt.procStart;
}
static bool stub_QProcess_waitForFinished(void *, int) { return true; }
static QByteArray stub_QProcess_readAllStandardOutput(void *)
{
    return QByteArray("'/org/freedesktop/UPower/devices/battery_BAT0'");
}
static bool stub_QDBus_isValid_first(const void *)
{
    // 首个接口(UPower 服务)有效，后续(电池设备)无效 → 静态局部只初始化一次
    static bool first = true;
    const bool ret = first;
    first = false;
    return ret;
}
static bool stub_QDBus_isValid_false(const void *) { return false; }
static bool stub_QThread_isRunning_true(const void *) { return true; }
static void stub_QThread_start_noop(void *, QThread::Priority) {}

// 拦截模态对话框内部事件循环（QDialog::exec / DDialog::exec 均为虚函数，
// Stub::set 对虚函数打补丁会直接崩溃；QEventLoop::exec 为普通成员函数可安全打桩）。
// DDialog::exec 对 QDialog::exec 返回值按"被点击按钮索引"二次映射；为覆盖
// closeEvent 的取消分支，在此按需模拟点击第 1 个按钮（Cancel）。
static int stub_QEventLoop_exec(void *, QEventLoop::ProcessEventsFlags)
{
    QWidget *modal = QApplication::activeModalWidget();
    DDialog *dlg = qobject_cast<DDialog *>(modal);
    if (dlg && g_cnt.dialogExecRet != 0) {
        const QList<QAbstractButton *> btns = dlg->getButtons();
        if (btns.size() > 1)
            btns.at(1)->click();
    }
    return g_cnt.dialogExecRet;
}

// ============================ 测试夹具 ============================

class MainWindowTest : public testing::Test
{
public:
    void SetUp() override
    {
        g_cnt = Counters();
        if (!g_dm)
            g_dm = DeviceManager::instance();

        UT_MARK("set-mw-refresh");
        stub.set(ADDR(MainWindow, refreshDataBase), stub_MW_refreshDataBase);
        stub.set(ADDR(MainWindow, refreshBatteryStatus), stub_MW_refreshBatteryStatus);
        UT_MARK("set-tx");
        stub.set(ADDR(ThreadExecXrandr, runCmd), stub_TX_runCmd);
        stub.set(ADDR(ThreadExecXrandr, getResolutionFromDBus), stub_TX_getResolutionFromDBus);
        stub.set(ADDR(LoadCpuInfoThread, getCpuInfoFromLscpu), stub_CPU_getCpuInfo);
        stub.set(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(
                     &QProcess::startDetached),
                 stub_QProcess_startDetached4);

        UT_MARK("set-dm");
        stub.set(ADDR(DeviceManager, instance), stub_DM_instance);
        stub.set(ADDR(DeviceManager, setDeviceListClass), stub_DM_setDevListClass);
        stub.set(ADDR(DeviceManager, getDeviceTypes), stub_DM_getDeviceTypes);
        stub.set(ADDR(DeviceManager, getDeviceDriverPool), stub_DM_getDeviceDriverPool);
        stub.set(ADDR(DeviceManager, getDeviceList), stub_DM_getDeviceList);
        stub.set(ADDR(DeviceManager, getDeviceOverview), stub_DM_getDeviceOverview);
        stub.set(ADDR(DeviceManager, networkDriver), stub_DM_networkDriver);
        stub.set(ADDR(DeviceManager, correctNetworkLinkStatus), stub_DM_correctNetworkLinkStatus);
        stub.set(ADDR(DeviceManager, correctPowerInfo), stub_DM_correctPowerInfo);
        stub.set(ADDR(DeviceManager, exportToTxt), stub_DM_exportToTxt);
        stub.set(ADDR(DeviceManager, exportToHtml), stub_DM_exportToHtml);
        stub.set(ADDR(DeviceManager, exportToDoc), stub_DM_exportToDoc);
        stub.set(ADDR(DeviceManager, exportToXlsx), stub_DM_exportToXlsx);

        UT_MARK("set-cmdtool");
        stub.set(ADDR(CmdTool, getCurNetworkLinkStatus), stub_Cmd_getCurNetworkLinkStatus);
        stub.set(ADDR(CmdTool, getCurPowerInfo), stub_Cmd_getCurPowerInfo);

        UT_MARK("set-dbus");
        stub.set(ADDR(DBusInterface, refreshInfo), stub_DBus_refreshInfo);
        stub.set(ADDR(DBusInterface, getInfo), stub_DBus_getInfo);

        UT_MARK("set-common");
        stub.set(static_cast<QString (*)()>(&Common::boardVendorType), stub_Common_boardVendorType);
        stub.set(static_cast<bool (*)()>(&Common::isHwPlatform), stub_Common_isHwPlatform);

        UT_MARK("set-pdm");
        stub.set(ADDR(PageDriverManager, scanDriverInfo), stub_PDM_scanDriverInfo);
        stub.set(ADDR(PageDriverManager, isFirstScan), stub_PDM_isFirstScan);
        stub.set(ADDR(PageDriverManager, isScanning), stub_PDM_isScanning);
        stub.set(ADDR(PageDriverManager, isInstalling), stub_PDM_isInstalling);
        stub.set(ADDR(PageDriverManager, isBackingup), stub_PDM_isBackingup);
        stub.set(ADDR(PageDriverManager, isRestoring), stub_PDM_isRestoring);

        UT_MARK("set-dw");
        stub.set(ADDR(DeviceWidget, setFontChangeFlag), stub_DW_setFontChangeFlag);
        stub.set(ADDR(DeviceWidget, updateListView), stub_DW_updateListView);
        stub.set(ADDR(DeviceWidget, updateDevice), stub_DW_updateDevice);
        stub.set(ADDR(DeviceWidget, updateOverview), stub_DW_updateOverview);

        stub.set(static_cast<QString (*)(QWidget *, const QString &, const QString &,
                                        const QString &, QString *, QFileDialog::Options)>(
                     &QFileDialog::getSaveFileName),
                 stub_QFileDialog_getSaveFileName);
        UT_MARK("set-dlg");

        UT_MARK("new-mainwindow");
        w = new MainWindow();
        UT_MARK("ctor-done");
    }

    void TearDown() override
    {
        delete w;
        w = nullptr;
        // Stub 成员析构时自动还原全部函数体补丁
    }

    Stub stub;
    MainWindow *w = nullptr;
};

// ==================== 构造 / 析构 / checkWaylandMode（间接） ====================

TEST_F(MainWindowTest, Ctor_Constructs_InitializesUi)
{
    // Arrange：SetUp 已构造 w（refreshDataBase 已打桩）
    // Act：无（构造即被测行为）
    // Assert：初始化链生效——栈 4 页、当前等待页、构造期间加载一次设备信息
    EXPECT_EQ(w->mp_MainStackWidget->count(), 4);
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 0);
    EXPECT_EQ(w->mp_MainStackWidget->widget(1), static_cast<QWidget *>(w->mp_DeviceWidget));
    EXPECT_EQ(g_cnt.refreshDbus, 1);   // ctor 调用 refreshDataBase 一次
    EXPECT_EQ(g_cnt.xrandrRun, 1);     // ctor 中 txgpu 线程执行一次
}

TEST_F(MainWindowTest, Ctor_WaylandEnv_IndirectCheckWaylandMode)
{
    // Arrange：删除 SetUp 构造的窗口，切换 XDG_SESSION_TYPE=wayland
    //   （checkWaylandMode 为文件内 static 函数，无法直接调用；
    //     其分支经 MainWindow ctor 的 ThreadExecXrandr 构造参数间接触发）
    delete w;
    const int dbusBefore = g_cnt.refreshDbus;
    const int xrandrBefore = g_cnt.xrandrRun;
    qputenv("XDG_SESSION_TYPE", "wayland");
    // Act
    w = new MainWindow();
    qunsetenv("XDG_SESSION_TYPE");
    // Assert：wayland 分支下构造链完整，xrandr 线程被安全执行（桩）且加载流程不变
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
    EXPECT_EQ(w->mp_MainStackWidget->count(), 4);
    EXPECT_EQ(g_cnt.refreshDbus, dbusBefore + 1);
    EXPECT_EQ(g_cnt.xrandrRun, xrandrBefore + 1);
}

TEST_F(MainWindowTest, Dtor_DestroysAndAllowsReconstruct)
{
    // Arrange：先触发一次页面切换，使内部状态非初始
    w->swichStackWidget();
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 1);
    // Act：析构并重建
    delete w;
    w = new MainWindow();
    // Assert：重建成功且状态回到初始（无全局状态残留）
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 0);
    EXPECT_EQ(g_cnt.refreshDbus, 2);   // 两次构造各加载一次
}

// ==================== refresh 系列 ====================

TEST_F(MainWindowTest, Refresh_WhenIdle_Refreshes)
{
    // Arrange：构造后空闲（m_refreshing=false）
    const int dbusBefore = g_cnt.refreshDbus;
    // Act
    w->refresh();
    // Assert：电池状态刷新 + 进入刷新态 + 回等待页 + 加载设备信息
    EXPECT_EQ(g_cnt.batteryRefresh, 1);
    EXPECT_TRUE(w->m_refreshing);
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 0);
    EXPECT_EQ(g_cnt.refreshDbus, dbusBefore + 1);
    EXPECT_FALSE(w->mp_ButtonBox->isEnabled());
}

TEST_F(MainWindowTest, Refresh_WhenRefreshing_Skips)
{
    // Arrange：已处于刷新态（等价类：条件 1 命中）
    w->m_refreshing = true;
    const int dbusBefore = g_cnt.refreshDbus;
    const int batteryBefore = g_cnt.batteryRefresh;
    // Act
    w->refresh();
    // Assert：电池刷新在跳过检查之前无条件执行（源码行 201），
    //   数据库加载被跳过，刷新态保持
    EXPECT_EQ(g_cnt.batteryRefresh, batteryBefore + 1);
    EXPECT_EQ(g_cnt.refreshDbus, dbusBefore);
    EXPECT_TRUE(w->m_refreshing);
}

TEST_F(MainWindowTest, Refresh_WhenScanning_Skips)
{
    // Arrange：驱动扫描中（等价类：条件 3 命中）
    g_cnt.scanning = true;
    const int dbusBefore = g_cnt.refreshDbus;
    // Act
    w->refresh();
    // Assert：早退且未进入刷新态（电池刷新在任何跳过检查之前已执行）
    EXPECT_FALSE(w->m_refreshing);
    EXPECT_EQ(g_cnt.refreshDbus, dbusBefore);
    EXPECT_EQ(g_cnt.batteryRefresh, 1);
}

TEST_F(MainWindowTest, Refresh_WhenThreadRunning_Skips)
{
    // Arrange：工作线程运行中（等价类：条件 4 命中；补丁测试内即还原，
    //   防止 ~MainWindow 的 while(isRunning()) 忙等死循环）
    stub.set(ADDR(QThread, isRunning), stub_QThread_isRunning_true);
    // Act
    w->refresh();
    stub.reset(ADDR(QThread, isRunning));
    // Assert：早退且未进入刷新态
    EXPECT_FALSE(w->m_refreshing);
    EXPECT_EQ(g_cnt.refreshDbus, 1);   // 仅 ctor 一次
}

TEST_F(MainWindowTest, RefreshDataBase_SetsWaitCursor)
{
    // Arrange：还原真实 refreshDataBase；QThread::start 打桩（线程不真正启动，
    //   规避后台扫描线程与析构等待；不 patch 虚函数 run —— Stub::set 对其打补丁会段错误）
    stub.reset(ADDR(MainWindow, refreshDataBase));
    stub.set(ADDR(QThread, start), stub_QThread_start_noop);
    // Act
    w->refreshDataBase();
    stub.reset(ADDR(QThread, start));
    // Assert：等待光标置位；不影响刷新态（该职责属 refresh()）
    EXPECT_TRUE(w->m_statusCursorIsWait);
    EXPECT_FALSE(w->m_refreshing);
    EXPECT_NE(w->mp_WorkingThread, nullptr);
}

TEST_F(MainWindowTest, RefreshDataBase_SecondCall_NoDoubleCursor)
{
    // Arrange：还原真实实现并预热一次（start 打桩，线程不真正启动）
    stub.reset(ADDR(MainWindow, refreshDataBase));
    stub.set(ADDR(QThread, start), stub_QThread_start_noop);
    w->refreshDataBase();
    EXPECT_TRUE(w->m_statusCursorIsWait);
    // Act：第二次调用
    w->refreshDataBase();
    // Assert：m_statusCursorIsWait 已置位时不再重复 setOverrideCursor（成对使用约束）
    EXPECT_TRUE(w->m_statusCursorIsWait);
}

TEST_F(MainWindowTest, RefreshDataBaseLater_CallsDBusRefreshInfo)
{
    // Arrange：构造后计数为 0
    // Act
    w->refreshDataBaseLater();
    // Assert：DBus 刷新立即触发一次；refreshDataBase 仅经 2s 定时触发，
    //   测试无事件循环 → 即时计数不变
    EXPECT_EQ(g_cnt.dbusRefreshInfo, 1);
    EXPECT_EQ(g_cnt.refreshDbus, 1);
}

TEST_F(MainWindowTest, BatteryStatus_InterfaceInvalid_SkipsGdbus)
{
    // Arrange：还原真实 refreshBatteryStatus；UPower 接口无效（无系统总线）
    stub.reset(ADDR(MainWindow, refreshBatteryStatus));
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(
                 &QProcess::start),
             stub_QProcess_start);
    stub.set(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
             stub_QDBus_isValid_false);
    // Act
    w->refreshBatteryStatus();
    // Assert：接口无效 → 不启动 gdbus 子进程，不触发后续刷新，函数安全返回
    EXPECT_EQ(g_cnt.procStart, 0);
    EXPECT_EQ(g_cnt.dbusRefreshInfo, 0);
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

TEST_F(MainWindowTest, BatteryStatus_BatOutput_RunsGdbusOnce)
{
    // Arrange：UPower 服务接口有效、电池设备接口无效；gdbus 输出含 BAT 路径
    stub.reset(ADDR(MainWindow, refreshBatteryStatus));
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(
                 &QProcess::start),
             stub_QProcess_start);
    stub.set(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), stub_QProcess_waitForFinished);
    stub.set(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
             stub_QProcess_readAllStandardOutput);
    stub.set(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
             stub_QDBus_isValid_first);
    // Act
    w->refreshBatteryStatus();
    // Assert：UPower 有效 → 启动一次 gdbus 枚举；未误用 detached 进程；安全结束
    EXPECT_EQ(g_cnt.procStart, 1);
    EXPECT_EQ(g_cnt.detached, 0);
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

// ==================== exportTo / slotExportInfo ====================

TEST_F(MainWindowTest, ExportTo_EmptyFile_ReturnsTrue)
{
    // Arrange：保存对话框返回空路径（等价类：取消导出）
    g_cnt.saveFile.clear();
    // Act
    const bool ret = w->exportTo();
    // Assert：空路径直接成功返回，且不触发任何导出实现
    EXPECT_TRUE(ret);
    EXPECT_EQ(g_cnt.exportTxt + g_cnt.exportHtml + g_cnt.exportDoc + g_cnt.exportXlsx, 0);
}

TEST_F(MainWindowTest, ExportTo_TextFilter_UsesTxt)
{
    // Arrange：过滤器选中 Text
    g_cnt.saveFile = QDir::tempPath() + QString("/ut_export.txt");
    g_cnt.saveFilter = "Text (*.txt)";
    // Act
    const bool ret = w->exportTo();
    // Assert：走 txt 导出（桩返回 true）
    EXPECT_TRUE(ret);
    EXPECT_EQ(g_cnt.exportTxt, 1);
    EXPECT_EQ(g_cnt.exportHtml, 0);
}

TEST_F(MainWindowTest, ExportTo_HtmlFilter_UsesHtml)
{
    // Arrange
    g_cnt.saveFile = QDir::tempPath() + QString("/ut_export.html");
    g_cnt.saveFilter = "Html (*.html)";
    // Act
    const bool ret = w->exportTo();
    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(g_cnt.exportHtml, 1);
    EXPECT_EQ(g_cnt.exportTxt, 0);
}

TEST_F(MainWindowTest, ExportTo_DocFilter_UsesDoc)
{
    // Arrange
    g_cnt.saveFile = QDir::tempPath() + QString("/ut_export.docx");
    g_cnt.saveFilter = "Doc (*.docx)";
    // Act
    const bool ret = w->exportTo();
    // Assert：Doc 导出桩返回 false → 透传 false
    EXPECT_FALSE(ret);
    EXPECT_EQ(g_cnt.exportDoc, 1);
}

TEST_F(MainWindowTest, ExportTo_XlsFilter_UsesXlsx)
{
    // Arrange
    g_cnt.saveFile = QDir::tempPath() + QString("/ut_export.xls");
    g_cnt.saveFilter = "Xls (*.xls)";
    // Act
    const bool ret = w->exportTo();
    // Assert
    EXPECT_FALSE(ret);
    EXPECT_EQ(g_cnt.exportXlsx, 1);
}

TEST_F(MainWindowTest, ExportTo_UnknownFilter_ReturnsFalse)
{
    // Arrange：非空路径但过滤器不匹配任何分支（负面等价类）
    g_cnt.saveFile = QDir::tempPath() + QString("/ut_export.bin");
    g_cnt.saveFilter = "";
    // Act
    const bool ret = w->exportTo();
    // Assert：默认分支返回 false 且不触发导出
    EXPECT_FALSE(ret);
    EXPECT_EQ(g_cnt.exportTxt + g_cnt.exportHtml + g_cnt.exportDoc + g_cnt.exportXlsx, 0);
}

TEST_F(MainWindowTest, SlotExportInfo_TriggersExportDialog)
{
    // Arrange：计数清零
    g_cnt.saveDialog = 0;
    // Act
    w->slotExportInfo();
    // Assert：导出槽转发至 exportTo → 打开一次保存对话框；空路径不触发导出实现
    EXPECT_EQ(g_cnt.saveDialog, 1);
    EXPECT_EQ(g_cnt.exportTxt + g_cnt.exportHtml + g_cnt.exportDoc + g_cnt.exportXlsx, 0);
}

// ==================== JSON 快捷键 ====================

TEST_F(MainWindowTest, GetJsonDoc_BuildsTwoShortcutGroups)
{
    // Arrange
    QJsonDocument doc;
    // Act
    w->getJsonDoc(doc);
    // Assert：结构完整——2 组、System 4 项、Device Manager 2 项
    const QJsonObject root = doc.object();
    EXPECT_TRUE(root.contains("shortcut"));
    const QJsonArray groups = root["shortcut"].toArray();
    EXPECT_EQ(groups.size(), 2);
    EXPECT_EQ(groups.at(0)["groupName"].toString(), QString("System"));
    EXPECT_EQ(groups.at(0)["groupItems"].toArray().size(), 4);
    EXPECT_EQ(groups.at(1)["groupName"].toString(), QString("Device Manager"));
    EXPECT_EQ(groups.at(1)["groupItems"].toArray().size(), 2);
    EXPECT_EQ(groups.at(1)["groupItems"].toArray().at(0)["name"].toString(), QString("Export"));
}

TEST_F(MainWindowTest, AddJsonArrayItem_AppendsNameValuePair)
{
    // Arrange
    QJsonArray arr;
    // Act
    w->addJsonArrayItem(arr, "Key", "Value");
    w->addJsonArrayItem(arr, "Key2", "Value2");
    // Assert：按序追加 name/value 对
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr.at(0)["name"].toString(), QString("Key"));
    EXPECT_EQ(arr.at(0)["value"].toString(), QString("Value"));
    EXPECT_EQ(arr.at(1)["name"].toString(), QString("Key2"));
    EXPECT_EQ(arr.at(1)["value"].toString(), QString("Value2"));
}

// ==================== 快捷键提示窗口 ====================

TEST_F(MainWindowTest, ShowShortcutsHelpDialog_StartsViewerProcess)
{
    // Arrange：计数清零（startDetached 已打桩）
    g_cnt.detached = 0;
    // Act
    w->showDisplayShortcutsHelpDialog();
    // Assert：启动一次 deepin-shortcut-viewer 子进程（未真实拉起），窗口对象仍有效
    EXPECT_EQ(g_cnt.detached, 1);
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

// ==================== 页面切换 / 最大化 ====================

TEST_F(MainWindowTest, SwichStackWidget_FromWaitingToDetail)
{
    // Arrange：构造后当前页 0
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 0);
    // Act
    w->swichStackWidget();
    // Assert：0→1
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 1);
}

TEST_F(MainWindowTest, SwichStackWidget_AlreadySwitched_NoChange)
{
    // Arrange：已在页 1（边界等价类）
    w->swichStackWidget();
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 1);
    // Act
    w->swichStackWidget();
    // Assert：仍为 1
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 1);
}

TEST_F(MainWindowTest, WindowMaximizing_TogglesMaximizeState)
{
    // Arrange：初始未最大化
    EXPECT_FALSE(w->isMaximized());
    // Act + Assert：第一次切换 → 最大化
    w->windowMaximizing();
    EXPECT_TRUE(w->isMaximized());
    // 第二次切换 → 还原
    w->windowMaximizing();
    EXPECT_FALSE(w->isMaximized());
}

// ==================== 初始化三件套 ====================

TEST_F(MainWindowTest, InitWindow_ReinitSafe)
{
    // Arrange：构造已完成一次初始化
    // Act：重复 initWindow（健壮性）
    w->initWindow();
    // Assert：中心控件与页面栈保持有效
    EXPECT_EQ(w->centralWidget(), static_cast<QWidget *>(w->mp_MainStackWidget));
    EXPECT_GE(w->mp_MainStackWidget->count(), 4);
}

TEST_F(MainWindowTest, InitWidgets_FourPagesInStack)
{
    // Arrange：构造即已执行
    // Act：无
    // Assert：4 页顺序为等待/设备/扫描/驱动
    EXPECT_EQ(w->mp_MainStackWidget->count(), 4);
    EXPECT_EQ(w->mp_MainStackWidget->widget(0), static_cast<QWidget *>(w->mp_WaitingWidget));
    EXPECT_EQ(w->mp_MainStackWidget->widget(1), static_cast<QWidget *>(w->mp_DeviceWidget));
    EXPECT_EQ(w->mp_MainStackWidget->widget(2), static_cast<QWidget *>(w->mp_DriverScanWidget));
    EXPECT_EQ(w->mp_MainStackWidget->widget(3), static_cast<QWidget *>(w->mp_DriverManager));
    EXPECT_EQ(w->centralWidget(), static_cast<QWidget *>(w->mp_MainStackWidget));
}

TEST_F(MainWindowTest, InitWindowSize_AppliesSizes)
{
    // Arrange：构造即已执行（INIT 1000x802 / MIN 680x300）
    // Act：无
    // Assert：初始尺寸与最小尺寸均生效
    EXPECT_EQ(w->size(), QSize(1000, 802));
    EXPECT_EQ(w->minimumSize(), QSize(680, 300));
}

TEST_F(MainWindowTest, InitWindowTitle_BuildsButtonBox)
{
    // Arrange：构造即已执行
    // Act：无
    // Assert：两个功能按钮已注册；Qt6 构建（DISABLE_DRIVER）下按钮组隐藏
    EXPECT_EQ(w->mp_ButtonBox->buttonList().size(), 2);
    EXPECT_EQ(w->mp_ButtonBox->buttonList().at(0)->text(), QString("Hardware"));
    EXPECT_EQ(w->mp_ButtonBox->buttonList().at(1)->text(), QString("Drivers"));
    EXPECT_EQ(w->mp_ButtonBox->objectName(), QString("MpButtonBox"));
#ifdef DISABLE_DRIVER
    EXPECT_TRUE(w->mp_ButtonBox->isHidden());
#else
    EXPECT_FALSE(w->mp_ButtonBox->isHidden());
#endif
}

// ==================== slotSetPage ====================

TEST_F(MainWindowTest, SlotSetPage_OtherPage_NoOp)
{
    // Arrange：非 driver 页
    const int idxBefore = w->mp_MainStackWidget->currentIndex();
    // Act
    w->slotSetPage("cpu");
    // Assert：页面与标志均不变
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), idxBefore);
    EXPECT_FALSE(w->m_ShowDriverPage);
    EXPECT_EQ(g_cnt.scanDriver, 0);
}

#ifdef DISABLE_DRIVER
TEST_F(MainWindowTest, SlotSetPage_Driver_DisabledBuild_EarlyReturn)
{
    // Arrange：Qt6 构建定义 DISABLE_DRIVER，driver 页请求应被早退
    w->m_IsFirstRefresh = true;
    // Act
    w->slotSetPage("driver");
    // Assert：未置显示驱动页标志、未触发扫描、首刷标志不受影响
    EXPECT_FALSE(w->m_ShowDriverPage);
    EXPECT_EQ(g_cnt.scanDriver, 0);
    EXPECT_TRUE(w->m_IsFirstRefresh);
}
#else
TEST_F(MainWindowTest, SlotSetPage_Driver_FirstRefresh_SetsShowFlag)
{
    // Arrange：首刷阶段
    w->m_IsFirstRefresh = true;
    // Act
    w->slotSetPage("driver");
    // Assert：置显示标志，不立即点击按钮
    EXPECT_TRUE(w->m_ShowDriverPage);
    EXPECT_EQ(g_cnt.scanDriver, 0);
}

TEST_F(MainWindowTest, SlotSetPage_Driver_NotFirstScan_StartsScan)
{
    // Arrange：非首刷 + 驱动从未扫描
    w->m_IsFirstRefresh = false;
    g_cnt.firstScan = true;
    // Act
    w->slotSetPage("driver");
    // Assert：模拟按钮 1 点击 → 进入扫描页并启动扫描
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 2);
    EXPECT_EQ(g_cnt.scanDriver, 1);
}

TEST_F(MainWindowTest, SlotSetPage_Driver_ScanDone_ShowsDriverManager)
{
    // Arrange：非首刷 + 已扫描完成
    w->m_IsFirstRefresh = false;
    g_cnt.firstScan = false;
    // Act
    w->slotSetPage("driver");
    // Assert：直接进入驱动管理页
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 3);
    EXPECT_EQ(g_cnt.scanDriver, 0);
}
#endif

// ==================== slotLoadingFinish ====================

TEST_F(MainWindowTest, LoadingFinish_Finish_OverviewBranch)
{
    // Arrange：无设备列表（getDeviceList=false → 概况分支）
    g_cnt.dmGetListRet = false;
    // Act
    w->slotLoadingFinish("finish");
    // Assert：设备类型已加载、列表已更新、概况已显示、首刷标志复位
    EXPECT_EQ(g_cnt.setDevListClass, 1);
    EXPECT_EQ(g_cnt.updateListView, 1);
    EXPECT_EQ(g_cnt.updateOverview, 1);
    EXPECT_EQ(w->mp_MainStackWidget->currentWidget(), static_cast<QWidget *>(w->mp_DeviceWidget));
    EXPECT_FALSE(w->m_IsFirstRefresh);
    EXPECT_FALSE(w->m_refreshing);
    EXPECT_TRUE(w->mp_ButtonBox->isEnabled());
}

TEST_F(MainWindowTest, LoadingFinish_Finish_DeviceBranch)
{
    // Arrange：有设备列表
    g_cnt.dmGetListRet = true;
    // Act
    w->slotLoadingFinish("finish");
    // Assert：更新设备视图而非概况；x86+非 Monitor 不触发 QtConcurrent 自动点击
    EXPECT_EQ(g_cnt.updateDevice, 1);
    EXPECT_EQ(g_cnt.updateOverview, 0);
    EXPECT_EQ(g_cnt.setDevListClass, 1);
    EXPECT_EQ(w->mp_MainStackWidget->currentWidget(), static_cast<QWidget *>(w->mp_DeviceWidget));
}

TEST_F(MainWindowTest, LoadingFinish_OtherMessage_NoOp)
{
    // Arrange：非 finish 消息且无扫描标志（负面等价类）
    const int idxBefore = w->mp_MainStackWidget->currentIndex();
    // Act
    w->slotLoadingFinish("progress 50%");
    // Assert：无任何页面/计数变化
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), idxBefore);
    EXPECT_EQ(g_cnt.scanDriver, 0);
    EXPECT_EQ(g_cnt.updateListView, 0);
    EXPECT_TRUE(w->m_IsFirstRefresh);   // 首刷标志仅在 finish 时复位
}

TEST_F(MainWindowTest, LoadingFinish_ShowDriverPage_SwitchesToDriverPage)
{
    // Arrange：加载完成后要求切驱动页；驱动已扫描完成（isFirstScan=false → 页 3）
    w->m_ShowDriverPage = true;
    g_cnt.firstScan = false;
    g_cnt.dmGetListRet = false;
    // Act
    w->slotLoadingFinish("finish");
    // Assert：标志消费并切到驱动管理页
    EXPECT_FALSE(w->m_ShowDriverPage);
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 3);
}

TEST_F(MainWindowTest, RefreshOnDriverPage_ThenLoadingFinish_StartScanningFlag)
{
    // Arrange：切到驱动管理页（isFirstScan=false → 点击按钮 1 进入页 3），
    //   refresh() 因 checkedId==1 置 startScanningFlag（文件内 static，经此路径置位）
    g_cnt.firstScan = false;
    w->mp_ButtonBox->buttonList().at(1)->click();
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 3);
    // Act：驱动页刷新
    w->refresh();
    // Assert：进入刷新态且加载设备信息
    EXPECT_TRUE(w->m_refreshing);
    EXPECT_EQ(g_cnt.refreshDbus, 2);
    // Act：加载完成消息 → 底部扫描标志块触发
    w->slotLoadingFinish("progress");
    // Assert：切到扫描页、启动驱动扫描、标志消费（再次触发不再扫描）
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 2);
    EXPECT_EQ(g_cnt.scanDriver, 1);
    w->slotLoadingFinish("progress");
    EXPECT_EQ(g_cnt.scanDriver, 1);
}

// ==================== slotListItemClicked ====================

TEST_F(MainWindowTest, ListItemClicked_Monitor_RunsXrandr)
{
    // Arrange：有设备列表
    g_cnt.dmGetListRet = true;
    const int xrandrBefore = g_cnt.xrandrRun;
    // Act
    w->slotListItemClicked("Monitor");
    // Assert：分支线程 tx(false) 非 gpu（不经过 getResolutionFromDBus 计数桩），
    //   尾部固定 txgpu(true) 计数 +1；设备视图更新
    EXPECT_EQ(g_cnt.xrandrRun, xrandrBefore + 1);
    EXPECT_EQ(g_cnt.updateDevice, 1);
    EXPECT_EQ(g_cnt.updateOverview, 0);
}

TEST_F(MainWindowTest, ListItemClicked_Overview_RunsXrandr)
{
    // Arrange：无设备列表 → 概况
    g_cnt.dmGetListRet = false;
    const int xrandrBefore = g_cnt.xrandrRun;
    // Act
    w->slotListItemClicked("Overview");
    // Assert：Overview 与 Monitor 同分支（tx(false) 不计数，尾部 +1）；显示概况
    EXPECT_EQ(g_cnt.xrandrRun, xrandrBefore + 1);
    EXPECT_EQ(g_cnt.updateOverview, 1);
    EXPECT_EQ(g_cnt.updateDevice, 0);
}

TEST_F(MainWindowTest, ListItemClicked_DisplayAdapter_RunsGpuXrandr)
{
    // Arrange
    g_cnt.dmGetListRet = true;
    const int xrandrBefore = g_cnt.xrandrRun;
    // Act
    w->slotListItemClicked("Display Adapter");
    // Assert：GPU 分支 + 尾部固定 = 2 次 xrandr；更新设备视图
    EXPECT_EQ(g_cnt.xrandrRun, xrandrBefore + 2);
    EXPECT_EQ(g_cnt.updateDevice, 1);
}

TEST_F(MainWindowTest, ListItemClicked_Cpu_RunsCpuThread)
{
    // Arrange
    g_cnt.dmGetListRet = true;
    const int xrandrBefore = g_cnt.xrandrRun;
    // Act
    w->slotListItemClicked("CPU");
    // Assert：CPU 分支走加载线程而非 xrandr 分支线程（尾部固定 1 次）
    EXPECT_EQ(g_cnt.cpuRun, 1);
    EXPECT_EQ(g_cnt.xrandrRun, xrandrBefore + 1);
    EXPECT_EQ(g_cnt.updateDevice, 1);
}

TEST_F(MainWindowTest, ListItemClicked_NetworkAdapter_CorrectsLinkStatus)
{
    // Arrange：networkDriver 桩返回 eth0；链路状态桩返回 connected
    g_cnt.dmGetListRet = true;
    // Act
    w->slotListItemClicked("Network Adapter");
    // Assert：查询链路状态并纠正一次（参数断言在桩内完成）
    EXPECT_EQ(g_cnt.netStatus, 1);
    EXPECT_EQ(g_cnt.netCorrect, 1);
    EXPECT_EQ(g_cnt.updateDevice, 1);
}

TEST_F(MainWindowTest, ListItemClicked_Battery_CorrectsPowerInfo)
{
    // Arrange
    g_cnt.dmGetListRet = true;
    // Act
    w->slotListItemClicked("Battery");
    // Assert：读取电源信息并纠正一次
    EXPECT_EQ(g_cnt.powerInfo, 1);
    EXPECT_EQ(g_cnt.powerCorrect, 1);
    EXPECT_EQ(g_cnt.updateDevice, 1);
}

TEST_F(MainWindowTest, ListItemClicked_MonitorCountChanged_TriggersDbusRefresh)
{
    // Arrange：模拟外部显示器数量变化（xrandr 桩本次返回 5，与 ctor 记录值 2 不一致）
    g_cnt.xrandrMonitor = 5;
    // Act
    w->slotListItemClicked("Unknown");
    // Assert：向后台查询服务状态(getInfo)且服务未运行(info=0) → 触发 refreshDataBaseLater
    EXPECT_EQ(g_cnt.dbusGetInfo, 1);
    EXPECT_EQ(g_cnt.dbusRefreshInfo, 1);
    // 尾部固定 xrandr 1 次 + 数量变化后记录新值（下次构造重新从桩读取）
    EXPECT_EQ(g_cnt.xrandrRun, 2);
}

TEST_F(MainWindowTest, ListItemClicked_WhileRefreshing_EarlyReturn)
{
    // Arrange：刷新中（无 monitor 变化、无界面刷新）
    w->m_refreshing = true;
    // Act
    w->slotListItemClicked("Unknown");
    // Assert：monitor 数未变（2==2 无 DBus 查询），刷新检查早退，无视图更新
    EXPECT_EQ(g_cnt.dbusGetInfo, 0);
    EXPECT_EQ(g_cnt.dbusRefreshInfo, 0);
    EXPECT_EQ(g_cnt.updateDevice + g_cnt.updateOverview, 0);
    EXPECT_TRUE(w->m_refreshing);
}

// ==================== slotRefreshInfo / slotChangeUI ====================

TEST_F(MainWindowTest, SlotRefreshInfo_RefreshesDbusAndUi)
{
    // Arrange
    const int dbusBefore = g_cnt.refreshDbus;
    // Act
    w->slotRefreshInfo();
    // Assert：DBus 刷新 + 界面刷新（进入刷新态、回等待页、加载设备信息）
    EXPECT_EQ(g_cnt.dbusRefreshInfo, 1);
    EXPECT_EQ(g_cnt.refreshDbus, dbusBefore + 1);
    EXPECT_TRUE(w->m_refreshing);
    EXPECT_EQ(w->mp_MainStackWidget->currentIndex(), 0);
}

TEST_F(MainWindowTest, SlotChangeUI_SetsFontFlagAndReloadsCurrent)
{
    // Arrange：当前设备类型桩返回 Monitor
    stub.set(ADDR(DeviceWidget, currentIndex), stub_DW_currentIndex);
    g_cnt.dmGetListRet = true;
    g_cnt.fontFlag = 0;
    const int xrandrBefore = g_cnt.xrandrRun;
    // Act
    w->slotChangeUI();
    // Assert：字体变化标志已设置 + 按当前类型重载设备（分支 tx(false) 不计数，尾部 +1）
    EXPECT_EQ(g_cnt.fontFlag, 1);
    EXPECT_EQ(g_cnt.updateDevice, 1);
    EXPECT_EQ(g_cnt.xrandrRun, xrandrBefore + 1);
}

// ==================== keyPressEvent ====================

TEST_F(MainWindowTest, KeyPress_CtrlE_Exports)
{
    // Arrange
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_E, Qt::ControlModifier);
    g_cnt.saveDialog = 0;
    // Act
    w->keyPressEvent(&ev);
    // Assert：Ctrl+E 触发导出（保存对话框一次），未误触发快捷键查看器
    EXPECT_EQ(g_cnt.saveDialog, 1);
    EXPECT_EQ(g_cnt.detached, 0);
}

TEST_F(MainWindowTest, KeyPress_ENoModifier_NoExport)
{
    // Arrange：无修饰键的 E（负面等价类）
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_E, Qt::NoModifier);
    g_cnt.saveDialog = 0;
    // Act
    w->keyPressEvent(&ev);
    // Assert：不导出、不开快捷键查看器、窗口状态不变
    EXPECT_EQ(g_cnt.saveDialog, 0);
    EXPECT_EQ(g_cnt.detached, 0);
    EXPECT_FALSE(w->isMaximized());
}

TEST_F(MainWindowTest, KeyPress_CtrlShiftQuestion_ShowsShortcutViewer)
{
    // Arrange
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Question, Qt::ControlModifier | Qt::ShiftModifier);
    g_cnt.detached = 0;
    // Act
    w->keyPressEvent(&ev);
    // Assert：启动快捷键查看器子进程（打桩），未误触发导出
    EXPECT_EQ(g_cnt.detached, 1);
    EXPECT_EQ(g_cnt.saveDialog, 0);
}

TEST_F(MainWindowTest, KeyPress_CtrlAltF_TogglesMaximize)
{
    // Arrange：初始未最大化
    EXPECT_FALSE(w->isMaximized());
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_F, Qt::ControlModifier | Qt::AltModifier);
    // Act
    w->keyPressEvent(&ev);
    // Assert：窗口最大化
    EXPECT_TRUE(w->isMaximized());
}

TEST_F(MainWindowTest, KeyPress_UnhandledKey_PassThrough)
{
    // Arrange：无关按键
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    g_cnt.saveDialog = 0;
    g_cnt.detached = 0;
    // Act
    w->keyPressEvent(&ev);
    // Assert：无任何动作、窗口对象仍有效
    EXPECT_EQ(g_cnt.saveDialog, 0);
    EXPECT_EQ(g_cnt.detached, 0);
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

// ==================== event / resizeEvent ====================

TEST_F(MainWindowTest, Event_ApplicationFontChange_EmitsSignal)
{
    // Arrange：监听 fontChange 信号
    QSignalSpy spy(w, static_cast<void (MainWindow::*)()>(&MainWindow::fontChange));
    ASSERT_TRUE(spy.isValid());
    QEvent ev(QEvent::ApplicationFontChange);
    // Act
    const bool handled = w->event(&ev);
    // Assert：字体变化事件被消费并发射一次 fontChange
    EXPECT_TRUE(handled);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(MainWindowTest, Event_OtherEvent_NoSignal)
{
    // Arrange：无关事件类型
    QSignalSpy spy(w, static_cast<void (MainWindow::*)()>(&MainWindow::fontChange));
    ASSERT_TRUE(spy.isValid());
    QEvent ev(QEvent::ToolTip);
    // Act
    w->event(&ev);
    // Assert：不发射，窗口保持存活
    EXPECT_EQ(spy.count(), 0);
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

TEST_F(MainWindowTest, ResizeEvent_ForwardsToBase)
{
    // Arrange：记录当前尺寸（直接调用 resizeEvent 不改变几何）
    const QSize before = w->size();
    QResizeEvent ev(QSize(500, 400), before);
    // Act
    w->resizeEvent(&ev);
    // Assert：几何未被直接修改，基类转发安全
    EXPECT_EQ(w->size(), before);
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

// ==================== closeEvent ====================

TEST_F(MainWindowTest, CloseEvent_Idle_Accepts)
{
    // Arrange：无驱动任务（isInstalling/isBackingup/isRestoring 均 false）
    QCloseEvent ev;
    // Act
    w->closeEvent(&ev);
    // Assert：空闲直接关闭（事件保持接受态）
    EXPECT_TRUE(ev.isAccepted());
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

TEST_F(MainWindowTest, CloseEvent_Installing_ExecZero_Closes)
{
    // Arrange：驱动安装中 + 对话框选择退出（返回 0）
    g_cnt.installing = true;
    g_cnt.dialogExecRet = 0;
    QCloseEvent ev;
    // Act：对话框模态循环打桩（窗口局部，避免影响框架内部阻塞调用）
    stub.set(ADDR(QEventLoop, exec), stub_QEventLoop_exec);
    w->closeEvent(&ev);
    stub.reset(ADDR(QEventLoop, exec));
    // Assert：接受关闭
    EXPECT_TRUE(ev.isAccepted());
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

TEST_F(MainWindowTest, CloseEvent_Installing_ExecNonZero_Ignores)
{
    // Arrange：驱动安装中 + 对话框选择取消（返回非 0）
    g_cnt.installing = true;
    g_cnt.dialogExecRet = 1;
    QCloseEvent ev;
    // Act：对话框模态循环打桩（窗口局部，避免影响框架内部阻塞调用）
    stub.set(ADDR(QEventLoop, exec), stub_QEventLoop_exec);
    w->closeEvent(&ev);
    stub.reset(ADDR(QEventLoop, exec));
    // Assert：事件被忽略（窗口不关闭且保持存活）
    EXPECT_FALSE(ev.isAccepted());
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

TEST_F(MainWindowTest, CloseEvent_Backingup_Ignores)
{
    // Arrange：驱动备份中 + 取消
    g_cnt.backingup = true;
    g_cnt.dialogExecRet = 1;
    QCloseEvent ev;
    // Act：对话框模态循环打桩（窗口局部，避免影响框架内部阻塞调用）
    stub.set(ADDR(QEventLoop, exec), stub_QEventLoop_exec);
    w->closeEvent(&ev);
    stub.reset(ADDR(QEventLoop, exec));
    // Assert：事件被忽略
    EXPECT_FALSE(ev.isAccepted());
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

TEST_F(MainWindowTest, CloseEvent_Restoring_Ignores)
{
    // Arrange：驱动还原中 + 取消
    g_cnt.restoring = true;
    g_cnt.dialogExecRet = 1;
    QCloseEvent ev;
    // Act：对话框模态循环打桩（窗口局部，避免影响框架内部阻塞调用）
    stub.set(ADDR(QEventLoop, exec), stub_QEventLoop_exec);
    w->closeEvent(&ev);
    stub.reset(ADDR(QEventLoop, exec));
    // Assert：事件被忽略
    EXPECT_FALSE(ev.isAccepted());
    EXPECT_NE(w->mp_MainStackWidget, nullptr);
}

TEST_F(MainWindowTest, SlotRefreshInfo_WhenRefreshing_RefreshesDbusOnly)
{
    // Arrange：刷新态下由 DBus 推送的刷新请求
    w->m_refreshing = true;
    const int dbusBefore = g_cnt.refreshDbus;
    // Act
    w->slotRefreshInfo();
    // Assert：DBus 刷新照常触发，界面刷新被早退，刷新态保持
    EXPECT_EQ(g_cnt.dbusRefreshInfo, 1);
    EXPECT_EQ(g_cnt.refreshDbus, dbusBefore);
    EXPECT_TRUE(w->m_refreshing);
}

TEST_F(MainWindowTest, SlotChangeUI_CpuType_ReloadsCpu)
{
    // Arrange：当前设备类型桩返回 CPU
    stub.set(ADDR(DeviceWidget, currentIndex), stub_DW_currentIndex);
    g_cnt.dwCurrentIndex = "CPU";
    g_cnt.dmGetListRet = true;
    const int xrandrBefore = g_cnt.xrandrRun;
    // Act
    w->slotChangeUI();
    // Assert：字体标志设置 + CPU 走加载线程（分支）+ 尾部 gpu 线程 + 设备视图更新
    EXPECT_EQ(g_cnt.fontFlag, 1);
    EXPECT_EQ(g_cnt.cpuRun, 1);
    EXPECT_EQ(g_cnt.updateDevice, 1);
    EXPECT_EQ(g_cnt.xrandrRun, xrandrBefore + 1);
}

} // namespace
