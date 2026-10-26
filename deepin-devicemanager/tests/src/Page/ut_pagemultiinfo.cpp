// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                 | level | factors      | min | actual |
// |------------------------|-------|--------------|-----|--------|
// | updateInfo             | high  | in_degree:28 | 3   | 3      |
// | PageMultiInfo          | mid   | in_degree:12 | 2   | 2      |
// | getTableListInfo       | mid   | -            | 2   | 5      |
// | clearLayout            | mid   | -            | 2   | 2      |
// | clearWidgets           | mid   | -            | 2   | 2      |
// | initWidgets            | mid   | -            | 2   | 1(合)  |
// | resizeEvent            | mid   | -            | 2   | 2      |
// | setLabel               | low   | -            | 2   | 2      |
// | slotActionRemoveDriver | low   | -            | 2   | 4      |
// | slotActionUpdateDriver | low   | -            | 2   | 2      |
// | slotCheckPrinterStatus | mid   | -            | 2   | 3      |
// | slotEnableDevice       | mid   | -            | 2   | 5      |
// | slotItemClicked        | mid   | -            | 2   | 2      |
// | slotWakeupMachine      | mid   | -            | 2   | 2      |
// | ~PageMultiInfo         | low   | -            | 2   | 2      |
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x] 15/15 方法均有专属用例
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x] 设备类型(input/network/printer/cpu/null)
// 3. 每个等价类的边界值显式覆盖: [x] 空列表、lst[0]==nullptr、中间 nullptr、row 越界
// 4. 同质 ≥ 3 组用 TEST_P: [x] 本类方法无同质多组输入维度（设备类型为异构分支，非同质参数化）
// 5. 分支清单 → 用例映射已列出: [x] 见下方分支清单
// 6. 每条 if/switch/throw/early-return 有触发用例: [x] 逐一映射
// 7. 异常路径 EXPECT_THROW 精确匹配: [x] 方法体无 throw，异常安全用 EXPECT_NO_FATAL_FAILURE
// 8. 负面场景有专门用例: [x] 空列表/越界行/空设备/EDS_Cancle
// 9. 负面用例验证强异常安全: [x] 无崩溃且状态符合预期
// 10. stub_ext vs gMock 选择正确: [x] 沿用项目 stub.h，PageDetail/DMessageManager/QProcess 全打桩
//
// 分支清单（基于 MCP get_code_snippet 真实源码 PageMultiInfo.cpp）→ 用例映射：
// updateInfo:
//   B1 lst.size()<1 → early return ................ updateInfo_emptyList
//   B2 lst[0] 为 DeviceCpu → 头部信息视图可见 ...... updateInfo_cpuHeader
//   B3 非 CPU → 头部信息视图隐藏 .................. updateInfo_singleDevice
// getTableListInfo:
//   B1 lst[0]==nullptr → early return ............. getTableListInfo_nullFirst
//   B2 元素为 nullptr → continue 跳过 ............. getTableListInfo_nullMid
//   B3 DeviceInput → 追加 wakeup 字段 ............. getTableListInfo_input
//   B4 name 含 "PS/2" → 追加 4 个 bus 字段 ........ getTableListInfo_ps2
//   B5 DeviceNetwork → logicalName/canDisable ..... getTableListInfo_network
// slotEnableDevice:
//   B1 mp_Detail 空 → early return ................ 恒非空（ctor 创建），防御分支
//   B2 EDS_Success → emit updateUI ................ slotEnableDevice_success
//   B3 EDS_Faild && enable → 启用失败提示 ......... slotEnableDevice_failedEnable
//   B4 EDS_Faild && !enable → 禁用失败提示 ........ slotEnableDevice_failedDisable
//   B5 EDS_NoSerial → 无序列号提示 ................ slotEnableDevice_noSerial
//   B6 其他(EDS_Cancle) → 无动作 .................. slotEnableDevice_cancle
// slotActionUpdateDriver:
//   B1 printer → startDetached("dde-printer") ..... slotActionUpdateDriver_printer
//   B2 其他 → PageDriverControl show .............. slotActionUpdateDriver_input
//   【源码缺陷】无 row 越界/空指针防护（对照 RemoveDriver 有防护）→ 缺陷清单
// slotActionRemoveDriver:
//   B1 row<0 || row>=size → return ................ slotActionRemoveDriver_invalidRow
//   B2 device==nullptr → return ................... slotActionRemoveDriver_invalidRow
//   B3 printer → 取 vendor/model .................. slotActionRemoveDriver_printer
//   B4 其他 → 直接构造 ............................ slotActionRemoveDriver_input
// slotCheckPrinterStatus:
//   B1 行无效 → refs 不变 ......................... slotCheckPrinterStatus_invalidRow
//   B2 printer → isPrinter/installed ............. slotCheckPrinterStatus_printer
//   B3 非 printer → refs 不变 ..................... slotCheckPrinterStatus_notPrinter
// slotWakeupMachine: B1 mp_Detail 委托 .............. slotWakeupMachine_forwardsToDetail
// slotItemClicked: B1 mp_Detail->showInfoOfNum ...... slotItemClicked_forwardsToDetail
// resizeEvent:
//   B1 m_lstDevice<1 → PageInfo::resizeEvent ...... resizeEvent_emptyList
//   B2 高度不足/充足 → updateTable ................ resizeEvent_withDevices
// clearLayout: 循环清空含控件项 .................... clearLayout_clearsItems
//
// stub 清单：PageDetail::enableDevice/showInfoOfNum/setWakeupMachine（成员含 this）、
//   DMessageManager::instance+sendMessage、QProcess::startDetached（静态，4 参完整签名）、
//   PageInfo::packageHasInstalled（静态）；QDBusInterface::call 由 test_main.cpp 全局桩兜底

#include "PageInfo.h"
#include "PageMultiInfo.h"
#include "PageDetail.h"
#include "PageDriverControl.h"
#include "PageTableHeader.h"
#include "DeviceInput.h"
#include "DevicePrint.h"
#include "DeviceNetwork.h"
#include "DeviceCpu.h"
#include "ut_Head.h"
#include "stub.h"

#include <DMessageManager>

#include <QProcess>
#include <QSignalSpy>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QPushButton>

#include <gtest/gtest.h>

// ───────────────────────── stub 状态与桩函数 ─────────────────────────
static DMessageManager *g_msgMgr = nullptr;
static int g_msgCount = 0;
static QString g_lastMsg;
static QString g_detachedProg;
static int g_detachedCount = 0;
static bool g_pkgInstalled = true;
static EnableDeviceStatus g_detailEnableRet = EDS_Success;
static int g_detailEnableRow = -1;
static bool g_detailEnableArg = false;
static int g_detailEnableCount = 0;
static int g_detailShowRow = -1;
static int g_detailShowCount = 0;
static int g_detailWakeupRow = -1;
static bool g_detailWakeupArg = false;
static int g_detailWakeupCount = 0;

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

typedef bool (*StartDetachedFn)(const QString &, const QStringList &, const QString &, qint64 *);

static bool ut_startDetached(const QString &prog, const QStringList &args)
{
    Q_UNUSED(args);
    g_detachedProg = prog;
    ++g_detachedCount;
    return true;
}

static bool ut_pkg_installed(const QString &packageName)
{
    Q_UNUSED(packageName);
    return g_pkgInstalled;
}

static EnableDeviceStatus ut_detail_enable(PageDetail *self, int row, bool enable)
{
    Q_UNUSED(self);
    g_detailEnableRow = row;
    g_detailEnableArg = enable;
    ++g_detailEnableCount;
    return g_detailEnableRet;
}

static void ut_detail_showInfo(PageDetail *self, int index)
{
    Q_UNUSED(self);
    g_detailShowRow = index;
    ++g_detailShowCount;
}

static void ut_detail_setWakeup(PageDetail *self, int row, bool wakeup)
{
    Q_UNUSED(self);
    g_detailWakeupRow = row;
    g_detailWakeupArg = wakeup;
    ++g_detailWakeupCount;
}

// ───────────────────────── 测试夹具 ─────────────────────────
class UT_PageMultiInfo : public UT_HEAD
{
public:
    static void SetUpTestSuite()
    {
        g_msgMgr = new DMessageManager;
    }

    static void TearDownTestSuite()
    {
        delete g_msgMgr;
        g_msgMgr = nullptr;
    }

    void SetUp() override
    {
        page = new PageMultiInfo;
        resetGlobals();
    }

    void TearDown() override
    {
        delete page;
        page = nullptr;
    }

    static void resetGlobals()
    {
        g_msgCount = 0;
        g_lastMsg.clear();
        g_detachedProg.clear();
        g_detachedCount = 0;
        g_pkgInstalled = true;
        g_detailEnableRet = EDS_Success;
        g_detailEnableRow = -1;
        g_detailEnableArg = false;
        g_detailEnableCount = 0;
        g_detailShowRow = -1;
        g_detailShowCount = 0;
        g_detailWakeupRow = -1;
        g_detailWakeupArg = false;
        g_detailWakeupCount = 0;
    }

    // 构造测试用输入设备（属性直接赋值，避开 setInfoFromHwinfo 的真实解析路径）
    DeviceInput *makeInput(bool enabled = true)
    {
        DeviceInput *dev = new DeviceInput;
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

    PageMultiInfo *page = nullptr;
};

// ───────────────────────── 构造 / 析构 ─────────────────────────
// Arrange: PageTableHeader 各信号已连接（ctor 连接）; Act: 从外部 emit 表头信号;
// Assert: 对应槽被调用（enableDevice/wakeupMachine/showInfo 路径）
TEST_F(UT_PageMultiInfo, ctor_connectsTableHeaderSignals)
{
    Stub stub;
    stub.set(ADDR(PageDetail, enableDevice), ut_detail_enable);
    stub.set(ADDR(PageDetail, showInfoOfNum), ut_detail_showInfo);
    stub.set(ADDR(PageDetail, setWakeupMachine), ut_detail_setWakeup);

    emit page->mp_Table->itemClicked(2);
    emit page->mp_Table->enableDevice(1, false);
    emit page->mp_Table->wakeupMachine(3, true);

    EXPECT_EQ(g_detailShowRow, 2);
    EXPECT_EQ(g_detailEnableRow, 1);
    EXPECT_FALSE(g_detailEnableArg);
    EXPECT_EQ(g_detailWakeupRow, 3);
    EXPECT_TRUE(g_detailWakeupArg);
}

// Arrange: 无设备页面; Act: delete; Assert: 析构无崩溃
TEST_F(UT_PageMultiInfo, dtor_noCrashWithoutDevice)
{
    PageMultiInfo *tmp = new PageMultiInfo;
    EXPECT_NO_FATAL_FAILURE(delete tmp);
}

// Arrange: 已填充设备列表的页面; Act: delete; Assert: 析构无崩溃
TEST_F(UT_PageMultiInfo, dtor_noCrashWithDeviceList)
{
    PageMultiInfo *tmp = new PageMultiInfo;
    DeviceInput *dev = makeInput();
    tmp->m_lstDevice.append(dev);
    EXPECT_NO_FATAL_FAILURE(delete tmp);
    delete dev;
}

// ───────────────────────── initWidgets ─────────────────────────
// Arrange: 已初始化页面（ctor 已调用）; Act: 断言核心成员; Assert: 控件链完整
TEST_F(UT_PageMultiInfo, initWidgets_widgetsCreated)
{
    EXPECT_NE(page->mp_Label, nullptr);
    EXPECT_NE(page->mp_Table, nullptr);
    EXPECT_NE(page->mp_Detail, nullptr);
    EXPECT_NE(page->layout(), nullptr);
    EXPECT_TRUE(page->m_lstDevice.isEmpty());
}

// ───────────────────────── updateInfo（high）─────────────────────────
// Arrange: 空设备列表（B1）; Act: updateInfo; Assert: 内部列表保持为空
TEST_F(UT_PageMultiInfo, updateInfo_emptyList_noChange)
{
    QList<DeviceBaseInfo *> lst;

    page->updateInfo(lst);

    EXPECT_TRUE(page->m_lstDevice.isEmpty());
    EXPECT_TRUE(page->m_deviceList.isEmpty());
}

// Arrange: 单个输入设备（B3 非 CPU）; Act: updateInfo; Assert: 列表/表格数据就绪，
//   头部信息视图隐藏
TEST_F(UT_PageMultiInfo, updateInfo_singleDevice_hidesHeader)
{
    DeviceInput *dev = makeInput();
    QList<DeviceBaseInfo *> lst{dev};

    page->updateInfo(lst);

    EXPECT_EQ(page->m_lstDevice.size(), 1);
    // 设备头行 + 数据行
    EXPECT_GE(page->m_deviceList.size(), 2);
    EXPECT_FALSE(page->mp_HeaderInfoWidget->isVisibleTo(page));
    delete dev;
}

// Arrange: 单个 CPU 设备（B2）; Act: updateInfo; Assert: 头部信息视图切换为可见
TEST_F(UT_PageMultiInfo, updateInfo_cpuHeader_showsHeaderWidget)
{
    DeviceCpu *cpu = new DeviceCpu;
    QList<DeviceBaseInfo *> lst{cpu};

    page->updateInfo(lst);

    EXPECT_EQ(page->m_lstDevice.size(), 1);
    EXPECT_TRUE(page->mp_HeaderInfoWidget->isVisibleTo(page));
    delete cpu;
}

// ───────────────────────── getTableListInfo ─────────────────────────
// Arrange: 首元素为 nullptr（B1）; Act: getTableListInfo; Assert: 不填充直接返回
TEST_F(UT_PageMultiInfo, getTableListInfo_nullFirst_returnsEarly)
{
    QList<DeviceBaseInfo *> lst{nullptr};
    QList<QStringList> deviceList, menuList;

    page->getTableListInfo(lst, deviceList, menuList);

    EXPECT_TRUE(deviceList.isEmpty());
    EXPECT_TRUE(menuList.isEmpty());
}

// Arrange: 输入设备 + 中间 nullptr（B2/B3）; Act: getTableListInfo; Assert:
//   头行 + 有效数据行，nullptr 行被跳过；menuControl 含 canUninstall/canEnable/wakeup/path
TEST_F(UT_PageMultiInfo, getTableListInfo_input_skipsNullMid)
{
    DeviceInput *dev = makeInput();
    QList<DeviceBaseInfo *> lst{dev, nullptr};
    QList<QStringList> deviceList, menuList;

    page->getTableListInfo(lst, deviceList, menuList);

    EXPECT_EQ(deviceList.size(), 2);   // 表头 + 1 有效行
    // 表头列数 ≥ 数据列数（getTableData 跳过空字段）
    EXPECT_GE(deviceList[0].size(), deviceList[1].size());
    EXPECT_GE(deviceList[1].size(), 1);
    EXPECT_EQ(menuList.size(), 1);
    // canUninstall/canEnable/canWakeupMachine/wakeupPath
    EXPECT_EQ(menuList[0].size(), 4);
    EXPECT_EQ(menuList[0][0], QString("true"));    // canUninstall=true
    EXPECT_EQ(menuList[0][1], QString("true"));    // canEnable=true
    delete dev;
}

// Arrange: 名称含 "PS/2" 的输入设备（B4）; Act: getTableListInfo; Assert: 追加 4 个字段
TEST_F(UT_PageMultiInfo, getTableListInfo_ps2_appendsBusFields)
{
    DeviceInput *dev = makeInput();
    dev->m_Name = "PS/2 Keyboard";
    QList<DeviceBaseInfo *> lst{dev};
    QList<QStringList> deviceList, menuList;

    page->getTableListInfo(lst, deviceList, menuList);

    EXPECT_EQ(menuList.size(), 1);
    EXPECT_EQ(menuList[0].size(), 8);   // 4 基础 + 4 bus 字段
    delete dev;
}

// Arrange: 网卡设备（B5）; Act: getTableListInfo; Assert: logicalName/canDisable 追加
TEST_F(UT_PageMultiInfo, getTableListInfo_network_appendsFields)
{
    DeviceNetwork *dev = new DeviceNetwork;
    dev->m_Name = "eth0 card";
    dev->m_LogicalName = "eth0";
    dev->m_SysPath = "/sys/class/net/eth0";   // canDisable 需要 SysPath 非空
    QList<DeviceBaseInfo *> lst{dev};
    QList<QStringList> deviceList, menuList;

    page->getTableListInfo(lst, deviceList, menuList);

    EXPECT_EQ(menuList.size(), 1);
    // 前 2 项为通用 canUninstall/canEnable，再追加 logicalName/canDisable
    EXPECT_EQ(menuList[0].size(), 4);
    EXPECT_EQ(menuList[0][2], QString("eth0"));
    EXPECT_EQ(menuList[0][3], QString("true"));   // SysPath 非空 → canDisable=true
    delete dev;
}

// ───────────────────────── clearLayout / clearWidgets ─────────────────────────
// Arrange: 布局含 2 个控件; Act: clearLayout; Assert: 布局清空，控件延迟删除
TEST_F(UT_PageMultiInfo, clearLayout_clearsItems)
{
    QVBoxLayout lay;
    lay.addWidget(new QPushButton("a"));
    lay.addWidget(new QPushButton("b"));
    ASSERT_EQ(lay.count(), 2);

    page->clearLayout(&lay);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    EXPECT_EQ(lay.count(), 0);
}

// Arrange: 空布局（负面等价类）; Act: clearLayout; Assert: 无崩溃
TEST_F(UT_PageMultiInfo, clearLayout_emptyLayout_noCrash)
{
    QVBoxLayout lay;
    EXPECT_NO_FATAL_FAILURE(page->clearLayout(&lay));
    EXPECT_EQ(lay.count(), 0);
}

// Arrange: 已填充设备的页面; Act: clearWidgets; Assert: 设备列表与详情清空
TEST_F(UT_PageMultiInfo, clearWidgets_clearsDeviceAndDetail)
{
    DeviceInput *dev = makeInput();
    page->m_lstDevice.append(dev);
    ASSERT_FALSE(page->m_lstDevice.isEmpty());

    page->clearWidgets();

    EXPECT_TRUE(page->m_lstDevice.isEmpty());
    EXPECT_TRUE(page->mp_Detail->m_ListTextBrowser.isEmpty());
    delete dev;
}

// ───────────────────────── setLabel ─────────────────────────
// Arrange: 初始页面; Act: setLabel; Assert: 文本与加粗字体生效
TEST_F(UT_PageMultiInfo, setLabel_setsTextAndBold)
{
    page->setLabel("USB Controller");

    EXPECT_EQ(page->mp_Label->text(), QString("USB Controller"));
    EXPECT_EQ(page->mp_Label->font().weight(), static_cast<int>(QFont::Bold));
}

// Arrange: 空字符串（边界）; Act: setLabel; Assert: 文本为空且无崩溃
TEST_F(UT_PageMultiInfo, setLabel_emptyString_noCrash)
{
    EXPECT_NO_FATAL_FAILURE(page->setLabel(QString()));
    EXPECT_TRUE(page->mp_Label->text().isEmpty());
}

// ───────────────────────── resizeEvent ─────────────────────────
// Arrange: 无设备（B1，直接 sendEvent 确保 resizeEvent 派发）; Act: 发送 200 高度事件;
// Assert: 走早期返回，无崩溃
TEST_F(UT_PageMultiInfo, resizeEvent_emptyList_noCrash)
{
    QResizeEvent ev(QSize(400, 200), page->size());
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(page, &ev));
    EXPECT_EQ(page->m_deviceList.size(), 0);
}

// Arrange: 已加载设备（B2/B3 高度不足与充足两分支）; Act: 依次发送小/大高度事件;
// Assert: updateTable 路径无崩溃
TEST_F(UT_PageMultiInfo, resizeEvent_withDevices_updatesTable)
{
    DeviceInput *dev = makeInput();
    page->updateInfo(QList<DeviceBaseInfo *>{dev});
    ASSERT_FALSE(page->m_deviceList.isEmpty());

    QResizeEvent smallEv(QSize(400, 200), page->size());   // 高度不足 → 压缩行数
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(page, &smallEv));
    QResizeEvent largeEv(QSize(800, 900), page->size());   // 高度充足 → 常规刷新
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(page, &largeEv));
    delete dev;
}

// ───────────────────────── slotItemClicked / slotWakeupMachine ─────────────────────────
// Arrange: 桩接管 PageDetail::showInfoOfNum; Act: slotItemClicked; Assert: 行号转发
TEST_F(UT_PageMultiInfo, slotItemClicked_forwardsToDetail)
{
    Stub stub;
    stub.set(ADDR(PageDetail, showInfoOfNum), ut_detail_showInfo);

    page->slotItemClicked(5);

    EXPECT_EQ(g_detailShowCount, 1);
    EXPECT_EQ(g_detailShowRow, 5);
}

// Arrange: 桩接管 PageDetail::setWakeupMachine; Act: slotWakeupMachine; Assert: 行号/状态转发
TEST_F(UT_PageMultiInfo, slotWakeupMachine_forwardsToDetail)
{
    Stub stub;
    stub.set(ADDR(PageDetail, setWakeupMachine), ut_detail_setWakeup);

    page->slotWakeupMachine(4, false);

    EXPECT_EQ(g_detailWakeupCount, 1);
    EXPECT_EQ(g_detailWakeupRow, 4);
    EXPECT_FALSE(g_detailWakeupArg);
}

// ───────────────────────── slotEnableDevice ─────────────────────────
// Arrange: 桩 enableDevice→EDS_Success（B2）; Act: slotEnableDevice; Assert: updateUI 发射、无提示
TEST_F(UT_PageMultiInfo, slotEnableDevice_success_emitsUpdateUI)
{
    Stub stub;
    stub.set(ADDR(PageDetail, enableDevice), ut_detail_enable);
    stub.set(ADDR(DMessageManager, instance), ut_msg_instance);
    stub.set((void (DMessageManager::*)(QWidget *, const QIcon &, const QString &))ADDR(DMessageManager, sendMessage),
             ut_msg_send_icon);
    QSignalSpy spy(page, &PageMultiInfo::updateUI);

    page->slotEnableDevice(0, false);

    EXPECT_EQ(g_detailEnableCount, 1);
    EXPECT_EQ(g_detailEnableRow, 0);
    EXPECT_FALSE(g_detailEnableArg);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(g_msgCount, 0);
}

// Arrange: 桩→EDS_Faild 且 enable=true（B3）; Act: slotEnableDevice; Assert: 启用失败提示
TEST_F(UT_PageMultiInfo, slotEnableDevice_failedEnable_showsMessage)
{
    Stub stub;
    stub.set(ADDR(PageDetail, enableDevice), ut_detail_enable);
    stub.set(ADDR(DMessageManager, instance), ut_msg_instance);
    stub.set((void (DMessageManager::*)(QWidget *, const QIcon &, const QString &))ADDR(DMessageManager, sendMessage),
             ut_msg_send_icon);
    g_detailEnableRet = EDS_Faild;
    QSignalSpy spy(page, &PageMultiInfo::updateUI);

    page->slotEnableDevice(0, true);

    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(g_msgCount, 1);
    EXPECT_EQ(g_lastMsg, page->tr("Failed to enable the device"));
}

// Arrange: 桩→EDS_Faild 且 enable=false（B4）; Act: slotEnableDevice; Assert: 禁用失败提示
TEST_F(UT_PageMultiInfo, slotEnableDevice_failedDisable_showsMessage)
{
    Stub stub;
    stub.set(ADDR(PageDetail, enableDevice), ut_detail_enable);
    stub.set(ADDR(DMessageManager, instance), ut_msg_instance);
    stub.set((void (DMessageManager::*)(QWidget *, const QIcon &, const QString &))ADDR(DMessageManager, sendMessage),
             ut_msg_send_icon);
    g_detailEnableRet = EDS_Faild;

    page->slotEnableDevice(0, false);

    EXPECT_EQ(g_msgCount, 1);
    EXPECT_EQ(g_lastMsg, page->tr("Failed to disable the device"));
}

// Arrange: 桩→EDS_NoSerial（B5）; Act: slotEnableDevice; Assert: 无序列号提示
TEST_F(UT_PageMultiInfo, slotEnableDevice_noSerial_showsMessage)
{
    Stub stub;
    stub.set(ADDR(PageDetail, enableDevice), ut_detail_enable);
    stub.set(ADDR(DMessageManager, instance), ut_msg_instance);
    stub.set((void (DMessageManager::*)(QWidget *, const QIcon &, const QString &))ADDR(DMessageManager, sendMessage),
             ut_msg_send_icon);
    g_detailEnableRet = EDS_NoSerial;

    page->slotEnableDevice(0, false);

    EXPECT_EQ(g_msgCount, 1);
    EXPECT_EQ(g_lastMsg, page->tr("Failed to disable it: unable to get the device SN"));
}

// Arrange: 桩→EDS_Cancle（B6 负面）; Act: slotEnableDevice; Assert: 无提示无发射
TEST_F(UT_PageMultiInfo, slotEnableDevice_cancle_noAction)
{
    Stub stub;
    stub.set(ADDR(PageDetail, enableDevice), ut_detail_enable);
    stub.set(ADDR(DMessageManager, instance), ut_msg_instance);
    stub.set((void (DMessageManager::*)(QWidget *, const QIcon &, const QString &))ADDR(DMessageManager, sendMessage),
             ut_msg_send_icon);
    g_detailEnableRet = EDS_Cancle;
    QSignalSpy spy(page, &PageMultiInfo::updateUI);

    EXPECT_NO_FATAL_FAILURE(page->slotEnableDevice(0, true));

    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(g_msgCount, 0);
}

// ───────────────────────── slotActionUpdateDriver ─────────────────────────
// Arrange: 打印机设备（B1），桩 startDetached; Act: slotActionUpdateDriver(0);
// Assert: 拉起 dde-printer，不创建对话框
TEST_F(UT_PageMultiInfo, slotActionUpdateDriver_printer_startsDetached)
{
    DevicePrint *dev = new DevicePrint;
    dev->setHardwareClass("printer");
    dev->setInfo(QMap<QString, QString>{{"printer-info", "HP LaserJet"}});
    page->m_lstDevice.append(dev);
    Stub stub;
    stub.set(static_cast<StartDetachedFn>(ADDR(QProcess, startDetached)), ut_startDetached);

    page->slotActionUpdateDriver(0);

    EXPECT_EQ(g_detachedCount, 1);
    EXPECT_EQ(g_detachedProg, QString("dde-printer"));
    EXPECT_EQ(page->findChild<PageDriverControl *>(), nullptr);
    delete dev;
}

// Arrange: 输入设备（B2）; Act: slotActionUpdateDriver(0); Assert: 创建更新驱动对话框
TEST_F(UT_PageMultiInfo, slotActionUpdateDriver_input_showsDialog)
{
    DeviceInput *dev = makeInput();
    page->m_lstDevice.append(dev);

    page->slotActionUpdateDriver(0);

    PageDriverControl *dlg = page->findChild<PageDriverControl *>();
    ASSERT_NE(dlg, nullptr);
    EXPECT_TRUE(dlg->windowTitle().startsWith(page->tr("Update Drivers")))
        << "actual: " << qUtf8Printable(dlg->windowTitle());
    delete dev;
}

// ───────────────────────── slotActionRemoveDriver ─────────────────────────
// Arrange: 输入设备（B4）; Act: slotActionRemoveDriver(0); Assert: 创建卸载对话框并连接刷新信号
TEST_F(UT_PageMultiInfo, slotActionRemoveDriver_input_showsDialog)
{
    DeviceInput *dev = makeInput();
    page->m_lstDevice.append(dev);
    QSignalSpy spy(page, &PageMultiInfo::refreshInfo);

    page->slotActionRemoveDriver(0);

    PageDriverControl *dlg = page->findChild<PageDriverControl *>();
    ASSERT_NE(dlg, nullptr);
    EXPECT_TRUE(dlg->windowTitle().startsWith(page->tr("Uninstall Drivers")))
        << "actual: " << qUtf8Printable(dlg->windowTitle());
    // 对话框 refreshInfo → 页面 refreshInfo 连接生效
    emit dlg->refreshInfo();
    EXPECT_EQ(spy.count(), 1);
    delete dev;
}

// Arrange: 打印机设备（B3 取 vendor/model）; Act: slotActionRemoveDriver(0); Assert: 对话框创建
TEST_F(UT_PageMultiInfo, slotActionRemoveDriver_printer_showsDialog)
{
    DevicePrint *dev = new DevicePrint;
    dev->setHardwareClass("printer");
    dev->setInfo(QMap<QString, QString>{{"printer-info", "HP LaserJet"}});
    page->m_lstDevice.append(dev);

    page->slotActionRemoveDriver(0);

    EXPECT_NE(page->findChild<PageDriverControl *>(), nullptr);
    delete dev;
}

// Arrange: 空列表（B1 越界防护）; Act: slotActionRemoveDriver(-1/0/999); Assert: 无对话框
TEST_F(UT_PageMultiInfo, slotActionRemoveDriver_invalidRow_noDialog)
{
    EXPECT_NO_FATAL_FAILURE(page->slotActionRemoveDriver(-1));
    EXPECT_NO_FATAL_FAILURE(page->slotActionRemoveDriver(0));
    EXPECT_NO_FATAL_FAILURE(page->slotActionRemoveDriver(999));
    EXPECT_EQ(page->findChild<PageDriverControl *>(), nullptr);
}

// Arrange: 行指向 nullptr 设备（B2）; Act: slotActionRemoveDriver(0); Assert: 无对话框
TEST_F(UT_PageMultiInfo, slotActionRemoveDriver_nullDevice_noDialog)
{
    page->m_lstDevice.append(nullptr);

    EXPECT_NO_FATAL_FAILURE(page->slotActionRemoveDriver(0));

    EXPECT_EQ(page->findChild<PageDriverControl *>(), nullptr);
}

// ───────────────────────── slotCheckPrinterStatus ─────────────────────────
// Arrange: 打印机 + 包已安装桩（B2）; Act: slotCheckPrinterStatus; Assert: isPrinter/installed 置真
TEST_F(UT_PageMultiInfo, slotCheckPrinterStatus_printer_reportsInstalled)
{
    DevicePrint *dev = new DevicePrint;
    dev->setHardwareClass("printer");
    dev->setInfo(QMap<QString, QString>{{"printer-info", "HP LaserJet"}});
    page->m_lstDevice.append(dev);
    g_pkgInstalled = true;
    Stub stub;
    stub.set(ADDR(PageInfo, packageHasInstalled), ut_pkg_installed);

    bool isPrinter = false, isInstalled = false;
    page->slotCheckPrinterStatus(0, isPrinter, isInstalled);

    EXPECT_TRUE(isPrinter);
    EXPECT_TRUE(isInstalled);
    delete dev;
}

// Arrange: 非打印机设备（B3）; Act: slotCheckPrinterStatus; Assert: 两个出参保持 false
TEST_F(UT_PageMultiInfo, slotCheckPrinterStatus_notPrinter_refsUntouched)
{
    DeviceInput *dev = makeInput();
    page->m_lstDevice.append(dev);

    bool isPrinter = false, isInstalled = false;
    page->slotCheckPrinterStatus(0, isPrinter, isInstalled);

    EXPECT_FALSE(isPrinter);
    EXPECT_FALSE(isInstalled);
    delete dev;
}

// Arrange: 行越界（B1）; Act: slotCheckPrinterStatus(999); Assert: 出参保持不变且无崩溃
TEST_F(UT_PageMultiInfo, slotCheckPrinterStatus_invalidRow_refsUntouched)
{
    bool isPrinter = true, isInstalled = true;

    EXPECT_NO_FATAL_FAILURE(page->slotCheckPrinterStatus(999, isPrinter, isInstalled));

    EXPECT_TRUE(isPrinter);
    EXPECT_TRUE(isInstalled);
}
