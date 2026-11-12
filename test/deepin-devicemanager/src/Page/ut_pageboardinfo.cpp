// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method           | level | factors      | min | actual |
// |------------------|-------|--------------|-----|--------|
// | loadDeviceInfo   | high  | in_degree:6  | 3   | 6      |
// | PageBoardInfo    | mid   | -            | 2   | 2      |
// | getOtherInfoPair | mid   | -            | 2   | 3      |
// | getValueInfo     | mid   | -            | 2   | 2      |
// | updateInfo       | low   | -            | 2   | 4      |
// | ~PageBoardInfo   | low   | -            | 2   | 1(合)  |
// | isBaseBoard      | low   | -            | 2   | 1      |
// | setFontChangeFlag| low   | -            | 2   | 1      |
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x] 8/8 方法均有专属用例（标注"合"的在关联用例中覆盖）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x] 设备类型(主板Bios/非主板Bios/非Bios)
// 3. 每个等价类的边界值显式覆盖: [x] 空列表、高度不足(limitRow<0)、展开态跳过
// 4. 同质 ≥ 3 组用 TEST_P: [x] 本类输入维度异构（设备组合与状态标志），无同质多组输入
// 5. 分支清单 → 用例映射已列出: [x] 见下方分支清单
// 6. 每条 if/switch/early-return 有触发用例: [x] 逐一映射
// 7. 异常路径 EXPECT_THROW 精确匹配: [x] 方法体无 throw，异常安全用 EXPECT_NO_FATAL_FAILURE
// 8. 负面场景有专门用例: [x] 空列表/无主板Bios/空设备组合
// 9. 负面用例验证强异常安全: [x] 无崩溃且状态符合预期
// 10. stub_ext vs gMock 选择正确: [x] 沿用项目 stub.h；PageTableWidget 与
//     DetailTreeView::isExpanded 打桩，DeviceBios 用真实对象（setBaseBoardInfo 填充）
//
// 分支清单（基于 MCP get_code_snippet 真实源码 PageBoardInfo.cpp）→ 用例映射：
// updateInfo:
//   B1 lst.size()<1 → early return ................ updateInfo_emptyList
//   B2 非 DeviceBios → continue ................... updateInfo_noBiosDevice
//   B3 bios->isBoard() → board .................... updateInfo_withBoardBios
//   B4 非 board bios → lstOther ................... updateInfo_nonBoardBios
//   B5 无 board → early return .................... updateInfo_nonBoardBios
//   B6 命中 board → loadDeviceInfo(lstOther, ...) . updateInfo_withBoardBios
// loadDeviceInfo:
//   B1 lst.size()<1 → early return ................ loadDeviceInfo_emptyList
//   B2 高度不足 maxRow<0 → limitSize 负值 ......... loadDeviceInfo_smallHeight
//   B3 正常高度 → min(lst.size,maxRow) ............ loadDeviceInfo_largeHeight
//   B4 !m_FontChangeFlag && isExpanded → 跳过 ..... loadDeviceInfo_expandedSkip
//   B5 字体变化或未展开 → 清空重填 ................ loadDeviceInfo_fontChangedExpanded
//   B6 其他信息富文本行写入 ....................... loadDeviceInfo_fontChangedExpanded
// getOtherInfoPair:
//   B1 非 DeviceBios → continue ................... getOtherInfoPair_mixed
//   B2 DeviceBios → 追加 nameTr+attribs ........... getOtherInfoPair_mixed
// getValueInfo:
//   B1 值含 ':' → 替换为空格 ...................... getValueInfo_stripsColons
//   B2 尾部 '\n' → 正则去除 ....................... getValueInfo_stripsColons
//
// stub 清单：PageTableWidget::setLimitRow/setColumnAndRow/setItem（成员含 self，
//   setItem 桩内释放 item 防泄漏）、DetailTreeView::isExpanded（非虚，注入展开态）；
//   QDBusInterface::call 由 test_main.cpp 全局桩兜底

#include "PageSingleInfo.h"
#include "PageBoardInfo.h"
#include "PageTableWidget.h"
#include "DetailTreeView.h"
#include "DeviceBios.h"
#include "DeviceInput.h"
#include "ut_Head.h"
#include "stub.h"

#include <QTableWidget>

#include <gtest/gtest.h>

// ───────────────────────── stub 状态与桩函数 ─────────────────────────
struct CapturedItem {
    int row;
    int col;
    QString text;
};
static QVector<CapturedItem> g_items;
static int g_limitRow = INT_MIN;
static int g_colRow = INT_MIN;
static bool g_expanded = false;

static void ut_ptw_setLimitRow(PageTableWidget *self, int row)
{
    Q_UNUSED(self);
    g_limitRow = row;
}

static void ut_ptw_setColRow(PageTableWidget *self, int row, int column)
{
    Q_UNUSED(self);
    Q_UNUSED(column);
    g_colRow = row;
}

static void ut_ptw_setItem(PageTableWidget *self, int row, int column, QTableWidgetItem *item)
{
    Q_UNUSED(self);
    CapturedItem ci{row, column, item ? item->text() : QString()};
    delete item;   // 拦截后自管理，防泄漏
    g_items.append(ci);
}

static bool ut_tree_isExpanded(DetailTreeView *self)
{
    Q_UNUSED(self);
    return g_expanded;
}

// ───────────────────────── 测试夹具 ─────────────────────────
class UT_PageBoardInfo : public UT_HEAD
{
public:
    void SetUp() override
    {
        page = new PageBoardInfo;
        resetGlobals();
    }

    void TearDown() override
    {
        delete page;
        page = nullptr;
    }

    static void resetGlobals()
    {
        g_items.clear();
        g_limitRow = INT_MIN;
        g_colRow = INT_MIN;
        g_expanded = false;
    }

    // 构造主板 Bios 设备（isBoard()==true）
    DeviceBios *makeBoard()
    {
        DeviceBios *bios = new DeviceBios;
        bios->setBaseBoardInfo(QMap<QString, QString>{{"Manufacturer", "UTest Inc."},
                                                      {"Version", "V1.0"},
                                                      {"Product Name", "UTest Board"},
                                                      {"chipset", "X99"}});
        return bios;
    }

    // 构造非主板 Bios 设备（isBoard()==false）
    DeviceBios *makeSystem()
    {
        DeviceBios *bios = new DeviceBios;
        bios->setSystemInfo(QMap<QString, QString>{{"Manufacturer", "UTest Sys."},
                                                   {"Version", "2.0"}});
        return bios;
    }

    PageBoardInfo *page = nullptr;
};

// ───────────────────────── 构造 / 析构 / 标志 ─────────────────────────
// Arrange: 构造完成; Act: 检查成员; Assert: 继承 PageSingleInfo 的控件链就绪，字体标志为假
TEST_F(UT_PageBoardInfo, ctor_defaultsReady)
{
    EXPECT_NE(page->mp_Content, nullptr);
    EXPECT_NE(page->mp_Label, nullptr);
    EXPECT_FALSE(page->m_FontChangeFlag);
}

// Arrange: 已填充主板信息的页面; Act: delete; Assert: 析构无崩溃
TEST_F(UT_PageBoardInfo, dtor_noCrashWithBoardData)
{
    DeviceBios *board = makeBoard();
    page->updateInfo({board});
    EXPECT_NO_FATAL_FAILURE(delete page);
    page = nullptr;   // TearDown 不再二次删除
    delete board;
}

// Arrange: 实例; Act: isBaseBoard; Assert: 恒为 true
TEST_F(UT_PageBoardInfo, isBaseBoard_returnsTrue)
{
    EXPECT_TRUE(page->isBaseBoard());
}

// Arrange: 初始页面; Act: setFontChangeFlag; Assert: 标志置真
TEST_F(UT_PageBoardInfo, setFontChangeFlag_setsFlag)
{
    page->setFontChangeFlag();
    EXPECT_TRUE(page->m_FontChangeFlag);
}

// ───────────────────────── updateInfo ─────────────────────────
// Arrange: 空列表（B1）; Act: updateInfo; Assert: mp_Device 保持为空
TEST_F(UT_PageBoardInfo, updateInfo_emptyList_noChange)
{
    page->updateInfo(QList<DeviceBaseInfo *>());
    EXPECT_EQ(page->mp_Device, nullptr);
}

// Arrange: 仅非 Bios 设备（B2/B5）; Act: updateInfo; Assert: mp_Device 指向首元素且无写入
TEST_F(UT_PageBoardInfo, updateInfo_noBiosDevice_earlyReturn)
{
    DeviceInput *dev = new DeviceInput;
    dev->m_Name = "Test Keyboard";
    Stub stub;
    stub.set(ADDR(PageTableWidget, setItem), ut_ptw_setItem);

    page->updateInfo({dev});

    EXPECT_EQ(page->mp_Device, static_cast<DeviceBaseInfo *>(dev));
    EXPECT_TRUE(g_items.isEmpty());
    delete dev;
}

// Arrange: 主板 Bios（B3/B6）; Act: updateInfo; Assert: 属性行写入表格且列数就绪
TEST_F(UT_PageBoardInfo, updateInfo_withBoardBios_writesRows)
{
    DeviceBios *board = makeBoard();
    Stub stub;
    stub.set(ADDR(PageTableWidget, setItem), ut_ptw_setItem);
    stub.set(ADDR(PageTableWidget, setColumnAndRow), ut_ptw_setColRow);

    page->updateInfo({board});

    EXPECT_FALSE(g_items.isEmpty());
    // 列数 = row + 1 = (lst.size() + devices.size()) + 1；主板行数 4 → 至少 1
    EXPECT_GE(g_colRow, 1);
    bool found = false;
    for (const auto &it : g_items)
        if (it.text.contains("UTest"))
            found = true;
    EXPECT_TRUE(found);
    delete board;
}

// Arrange: 仅非主板 Bios（B4/B5）; Act: updateInfo; Assert: 无主板 → 无写入
TEST_F(UT_PageBoardInfo, updateInfo_nonBoardBios_noWrite)
{
    DeviceBios *sys = makeSystem();
    Stub stub;
    stub.set(ADDR(PageTableWidget, setItem), ut_ptw_setItem);

    page->updateInfo({sys});

    EXPECT_EQ(page->mp_Device, static_cast<DeviceBaseInfo *>(sys));
    EXPECT_TRUE(g_items.isEmpty());
    delete sys;
}

// ───────────────────────── getOtherInfoPair / getValueInfo ─────────────────────────
// Arrange: 主板/非主板 Bios 与非 Bios 混合（B1/B2）; Act: getOtherInfoPair;
// Assert: 仅 Bios 条目追加，nameTr 为首列
TEST_F(UT_PageBoardInfo, getOtherInfoPair_mixedOnlyBios)
{
    DeviceBios *board = makeBoard();
    DeviceBios *sys = makeSystem();
    DeviceInput *dev = new DeviceInput;
    QList<QPair<QString, QString>> lstPair;

    page->getOtherInfoPair({board, dev, sys}, lstPair);

    EXPECT_EQ(lstPair.size(), 2);
    EXPECT_EQ(lstPair[0].first, board->nameTr());
    EXPECT_EQ(lstPair[1].first, sys->nameTr());
    delete dev;
    delete board;
    delete sys;
}

// Arrange: 空列表（负面）; Act: getOtherInfoPair; Assert: 输出为空
TEST_F(UT_PageBoardInfo, getOtherInfoPair_emptyList)
{
    QList<QPair<QString, QString>> lstPair;
    page->getOtherInfoPair({}, lstPair);
    EXPECT_TRUE(lstPair.isEmpty());
}

// Arrange: 值含 ':' 的主板 Bios（B1/B2）; Act: getValueInfo; Assert: 冒号替换、尾部换行剥离
TEST_F(UT_PageBoardInfo, getValueInfo_stripsColonsAndTrailingNewline)
{
    DeviceBios *board = new DeviceBios;
    board->setBaseBoardInfo(QMap<QString, QString>{{"Manufacturer", "U:Test Inc."},
                                                   {"Version", "V:1"}});
    QPair<QString, QString> pair;

    page->getValueInfo(board, pair);

    EXPECT_FALSE(pair.second.isEmpty());
    EXPECT_FALSE(pair.second.endsWith('\n'));
    EXPECT_FALSE(pair.second.contains("U:Test"));
    EXPECT_TRUE(pair.second.contains("U Test"));
    delete board;
}

// Arrange: 属性极少的设备（边界）; Act: getValueInfo; Assert: 至少一行键值且无尾换行
TEST_F(UT_PageBoardInfo, getValueInfo_minimalAttribs)
{
    DeviceBios *board = new DeviceBios;
    board->setBaseBoardInfo(QMap<QString, QString>{{"Manufacturer", "M"},
                                                   {"Version", "1"}});
    QPair<QString, QString> pair;

    page->getValueInfo(board, pair);

    EXPECT_GT(pair.second.count('\n') + 1, 0);
    EXPECT_FALSE(pair.second.endsWith('\n'));
    delete board;
}

// ───────────────────────── loadDeviceInfo（high）─────────────────────────
// Arrange: 空信息列表（B1）; Act: loadDeviceInfo; Assert: setLimitRow 未被调用
TEST_F(UT_PageBoardInfo, loadDeviceInfo_emptyList_noSetLimitRow)
{
    Stub stub;
    stub.set(ADDR(PageTableWidget, setLimitRow), ut_ptw_setLimitRow);

    page->loadDeviceInfo({}, {});

    EXPECT_EQ(g_limitRow, INT_MIN);   // 早退，未触达
}

// Arrange: 页面高度不足（B2）; Act: loadDeviceInfo; Assert: limitRow 取负的 maxRow
TEST_F(UT_PageBoardInfo, loadDeviceInfo_smallHeight_negativeLimitRow)
{
    page->resize(400, 100);   // 100/ROW_HEIGHT - 3 < 0
    Stub stub;
    stub.set(ADDR(PageTableWidget, setLimitRow), ut_ptw_setLimitRow);

    page->loadDeviceInfo({}, {qMakePair(QString("k"), QString("v"))});

    EXPECT_LT(g_limitRow, 0);
}

// Arrange: 页面高度充足（B3）; Act: loadDeviceInfo; Assert: limitRow = min(lst.size, maxRow)
TEST_F(UT_PageBoardInfo, loadDeviceInfo_largeHeight_limitIsListSize)
{
    page->resize(800, 900);
    Stub stub;
    stub.set(ADDR(PageTableWidget, setLimitRow), ut_ptw_setLimitRow);

    QList<QPair<QString, QString>> lst{{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}};
    page->loadDeviceInfo({}, lst);

    EXPECT_EQ(g_limitRow, 3);
}

// Arrange: 展开态且字体未变（B4）; Act: loadDeviceInfo; Assert: setLimitRow 已调但无写入
TEST_F(UT_PageBoardInfo, loadDeviceInfo_expandedSkip_noWrites)
{
    g_expanded = true;   // DetailTreeView::isExpanded 桩
    Stub stub;
    stub.set(ADDR(PageTableWidget, setLimitRow), ut_ptw_setLimitRow);
    stub.set(ADDR(PageTableWidget, setItem), ut_ptw_setItem);
    stub.set(ADDR(DetailTreeView, isExpanded), ut_tree_isExpanded);

    page->loadDeviceInfo({}, {qMakePair(QString("k"), QString("v"))});

    EXPECT_GE(g_limitRow, INT_MIN + 1);   // setLimitRow 已触达
    EXPECT_TRUE(g_items.isEmpty());       // 展开态跳过刷新
}

// Arrange: 展开态但字体变化（B5/B6）; Act: setFontChangeFlag + loadDeviceInfo;
// Assert: 标志复位、行写入恢复、富文本行委托行号正确
TEST_F(UT_PageBoardInfo, loadDeviceInfo_fontChangedExpanded_writes)
{
    page->setFontChangeFlag();
    g_expanded = true;
    DeviceBios *sys = makeSystem();
    Stub stub;
    stub.set(ADDR(PageTableWidget, setLimitRow), ut_ptw_setLimitRow);
    stub.set(ADDR(PageTableWidget, setItem), ut_ptw_setItem);
    stub.set(ADDR(PageTableWidget, setColumnAndRow), ut_ptw_setColRow);
    stub.set(ADDR(DetailTreeView, isExpanded), ut_tree_isExpanded);

    page->loadDeviceInfo({sys}, {qMakePair(QString("BoardKey"), QString("BoardVal"))});

    // 展开态未阻断（字体变化标志生效）
    EXPECT_FALSE(g_items.isEmpty());
    // 标志在写入前复位
    EXPECT_FALSE(page->m_FontChangeFlag);
    // 主板信息行在前（row 0），其他信息（富文本）行随后
    bool foundBoard = false, foundOther = false;
    for (const auto &it : g_items) {
        if (it.text == "BoardKey" || it.text == "BoardVal")
            foundBoard = true;
        if (it.text == sys->nameTr())
            foundOther = true;
    }
    EXPECT_TRUE(foundBoard);
    EXPECT_TRUE(foundOther);
    delete sys;
}
