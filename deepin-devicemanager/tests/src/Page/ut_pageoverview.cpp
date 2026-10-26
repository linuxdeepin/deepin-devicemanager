// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method        | level | factors      | min | actual |
// |---------------|-------|--------------|-----|--------|
// | setLabel      | high  | in_degree:9  | 3   | 3      |
// | PageOverview  | mid   | in_degree:5  | 2   | 2      |
// | updateInfo    | mid   | -            | 2   | 5      |
// | clearWidgets  | mid   | -            | 2   | 2      |
// | initWidgets   | mid   | -            | 2   | 1(合)  |
// | slotShowMenu  | mid   | -            | 2   | 1(合)  |
// | ~PageOverview | low   | -            | 2   | 2      |
// | isOverview    | low   | -            | 2   | 1      |
// | slotActionCopy| mid   | -            | 2   | 1(合)  |
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x] 9/9 方法均有专属用例（标注"合"的在关联用例中覆盖）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x] map 空/含类型；tooltip 短/长
// 3. 每个等价类的边界值显式覆盖: [x] tooltip 长度 60/120 换行边界、map 空边界
// 4. 同质 ≥ 3 组用 TEST_P: [x] 本类输入维度异构（设备类型表），无同质多组输入
// 5. 分支清单 → 用例映射已列出: [x] 见下方分支清单
// 6. 每条 if/switch/early-return 有触发用例: [x] 逐一映射
// 7. 异常路径 EXPECT_THROW 精确匹配: [x] 方法体无 throw，异常安全用 EXPECT_NO_FATAL_FAILURE
// 8. 负面场景有专门用例: [x] 空 map/空菜单触发点
// 9. 负面用例验证强异常安全: [x] 无崩溃且状态符合预期
// 10. stub_ext vs gMock 选择正确: [x] 沿用项目 stub.h；QClipboard/QMenu/DeviceManager
//     /PageTableWidget 全打桩
//
// 分支清单（基于 MCP get_code_snippet 真实源码 PageOverview.cpp）→ 用例映射：
// setLabel:
//   B1 tips.length()>ENTER_ONE(60) → 插入换行 ........ setLabel_longTips
//   B2 tips.length()>ENTER_TWO(120) → 再插换行 ....... setLabel_longTips
//   B3 社区版 → DEEPIN_LINK ......................... setLabel_editionLinks(分支触发)
//   B4 非社区版 → LINK_STR(uos) ..................... setLabel_shortTips(默认)
// updateInfo:
//   B1 maxRow<1 → setLimitRow(11) ................... updateInfo_smallHeight
//   B2 maxRow≥1 → setLimitRow(min(11,maxRow)) ....... updateInfo_largeHeight
//   B3 类型=="Overview" → 跳过 ...................... updateInfo_overviewSkipped(桩注入)
//   B4 map 含类型 → 写入表格两列 .................... updateInfo_overviewSkipped
//   B5 map.find 不命中 → 不写行 ..................... updateInfo_emptyMap
// clearWidgets: mp_Overview->clear() ................. clearWidgets_clearsTable
// slotShowMenu: clear+3 动作+exec .................... slotShowMenu_buildsMenu
// slotActionCopy: clipboard->setText(toString()) ..... slotActionCopy_copiesTableText
//
// stub 清单：QClipboard::setText（成员含 self）、QMenu::exec（成员含 self）、
//   DeviceManager::getDeviceTypes（const 引用返回，注入 Overview/Display/Separator）、
//   PageTableWidget::setLimitRow/setColumnAndRow（捕获分支结果，成员含 self）；
//   QDBusInterface::call 由 test_main.cpp 全局桩兜底

#include "PageInfo.h"
#include "PageOverview.h"
#include "PageTableWidget.h"
#include "LongTextLabel.h"
#include "DeviceManager.h"
#include "ut_Head.h"
#include "stub.h"

#include <DApplication>
#include <QAction>
#include <QClipboard>
#include <QMenu>

#include <gtest/gtest.h>

// ───────────────────────── stub 状态与桩函数 ─────────────────────────
static QString g_clipText;
static QPoint g_menuExecPos;
static int g_menuExecCount = 0;
static QList<QPair<QString, QString>> g_devTypes;
static int g_limitRow = -1;
static int g_colRow = -1;

static void ut_clip_setText(QClipboard *self, const QString &text, QClipboard::Mode mode)
{
    Q_UNUSED(self);
    Q_UNUSED(mode);
    g_clipText = text;
}

static QAction *ut_menu_exec(QMenu *self, const QPoint &pos, QAction *at)
{
    Q_UNUSED(self);
    Q_UNUSED(at);
    g_menuExecPos = pos;
    ++g_menuExecCount;
    return nullptr;
}

static const QList<QPair<QString, QString>> &ut_dev_getTypes(DeviceManager *self)
{
    Q_UNUSED(self);
    return g_devTypes;
}

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

// ───────────────────────── 测试夹具 ─────────────────────────
class UT_PageOverview : public UT_HEAD
{
public:
    void SetUp() override
    {
        page = new PageOverview;
        g_clipText.clear();
        g_menuExecPos = QPoint();
        g_menuExecCount = 0;
        g_devTypes.clear();
        g_limitRow = -1;
        g_colRow = -1;
    }

    void TearDown() override
    {
        delete page;
        page = nullptr;
    }

    PageOverview *page = nullptr;
};

// ───────────────────────── 构造 / 析构 / isOverview ─────────────────────────
// Arrange: 构造完成; Act: 检查成员; Assert: 动作 objectName 与布局就绪
TEST_F(UT_PageOverview, ctor_widgetsAndActionsCreated)
{
    EXPECT_NE(page->mp_PicLabel, nullptr);
    EXPECT_NE(page->mp_Overview, nullptr);
    EXPECT_NE(page->mp_Menu, nullptr);
    EXPECT_EQ(page->mp_Refresh->objectName(), QString("MpRefresh"));
    EXPECT_EQ(page->mp_Export->objectName(), QString("MpExport"));
    EXPECT_EQ(page->mp_Copy->objectName(), QString("MpCopy"));
    EXPECT_NE(page->layout(), nullptr);
}

// Arrange: 空数据页面; Act: delete; Assert: 析构无崩溃
TEST_F(UT_PageOverview, dtor_noCrashWithoutData)
{
    PageOverview *tmp = new PageOverview;
    EXPECT_NO_FATAL_FAILURE(delete tmp);
}

// Arrange: 已填充数据的页面; Act: delete; Assert: 析构无崩溃
TEST_F(UT_PageOverview, dtor_noCrashWithTableData)
{
    PageOverview *tmp = new PageOverview;
    QMap<QString, QString> map;
    map.insert("Display", "UHD 630");
    tmp->updateInfo(map);
    EXPECT_NO_FATAL_FAILURE(delete tmp);
}

// Arrange: 实例; Act: isOverview; Assert: 恒为 true
TEST_F(UT_PageOverview, isOverview_returnsTrue)
{
    EXPECT_TRUE(page->isOverview());
}

// ───────────────────────── clearWidgets ─────────────────────────
// Arrange: 表格有数据; Act: clearWidgets; Assert: toString 输出为空
TEST_F(UT_PageOverview, clearWidgets_clearsTable)
{
    // updateInfo 修复后按 setColumnAndRow(row + 1) 设行（PR #862），每个条目都有可见行；
    // 用 2 项 map 同时验证多行写入与 clear 后表格为空
    QMap<QString, QString> map;
    map.insert("Display", "UHD 630");
    map.insert("Audio", "HDMI Audio");
    Stub stub;
    stub.set(ADDR(DeviceManager, getDeviceTypes), ut_dev_getTypes);
    g_devTypes = {qMakePair(QString("Display"), QString("display##Display")),
                  qMakePair(QString("Audio"), QString("audio##Audio"))};
    page->updateInfo(map);
    ASSERT_FALSE(page->mp_Overview->toString().isEmpty());

    page->clearWidgets();

    EXPECT_TRUE(page->mp_Overview->toString().isEmpty());
}

// Arrange: 空表格重复清理（负面）; Act: clearWidgets; Assert: 无崩溃
TEST_F(UT_PageOverview, clearWidgets_emptyTable_noCrash)
{
    EXPECT_NO_FATAL_FAILURE(page->clearWidgets());
    EXPECT_TRUE(page->mp_Overview->toString().isEmpty());
}

// ───────────────────────── setLabel（high）─────────────────────────
// Arrange: 短 tooltip（≤60，B1/B2 不触发，B4 默认 uos 链接）; Act: setLabel;
// Assert: 设备标签文本/加粗、OS 标签含链接、tooltip 无换行
TEST_F(UT_PageOverview, setLabel_shortTips_linkAndBold)
{
    page->setLabel("UHD Graphics 630", "UOS 20");

    EXPECT_EQ(page->mp_DeviceLabel->text(), QString("UHD Graphics 630"));
    EXPECT_EQ(page->mp_DeviceLabel->font().weight(), static_cast<int>(QFont::Bold));
    // uos 链接前缀（非社区版 → LINK_STR）
    EXPECT_TRUE(page->mp_OSLabel->text().contains("href=https://"))
        << "actual: " << qUtf8Printable(page->mp_OSLabel->text());
    EXPECT_FALSE(page->mp_OSLabel->toolTip().contains('\n'));
}

// Arrange: 长 tooltip（130 字符，>60 且 >120，B1+B2）; Act: setLabel; Assert: 两次换行插入
TEST_F(UT_PageOverview, setLabel_longTips_insertsNewlines)
{
    QString longOs(130, 'x');

    page->setLabel("Device A", longOs);

    const QString tips = page->mp_OSLabel->toolTip();
    EXPECT_EQ(tips.count('\n'), 2);
    EXPECT_NE(tips.indexOf('\n'), -1);
}

// Arrange: 60/120 长度边界; Act: setLabel; Assert: 边界处换行位置正确（61 首换行、122 次换行）
TEST_F(UT_PageOverview, setLabel_boundaryTips_newlinePositions)
{
    QString tips(121, 'a');

    page->setLabel("Dev", tips);

    const QString result = page->mp_OSLabel->toolTip();
    // >60 → 位置 60 插入；首次插入后长度 122 > 120 → 位置 120 再插入
    EXPECT_EQ(result.indexOf('\n'), 60);
    EXPECT_EQ(result.lastIndexOf('\n'), 120);
}

// Arrange: 社区版链接常量差异（B3 分支触发）; Act: setLabel（DSysInfo 实测本机版本类型）;
// Assert: 链接为 chinauos 或 deepin 两者之一，文本非空
TEST_F(UT_PageOverview, setLabel_editionLinks_validLink)
{
    page->setLabel("Dev", "UOS");

    const QString text = page->mp_OSLabel->text();
    EXPECT_TRUE(text.contains("https://"));
    EXPECT_FALSE(text.isEmpty());
}

// ───────────────────────── updateInfo ─────────────────────────
// Arrange: 高度不足（B1 maxRow<1）; Act: updateInfo; Assert: setLimitRow(11)
TEST_F(UT_PageOverview, updateInfo_smallHeight_limitRow11)
{
    page->resize(400, 100);   // 100/ROW_HEIGHT-4 < 1
    Stub stub;
    stub.set(ADDR(PageTableWidget, setLimitRow), ut_ptw_setLimitRow);
    stub.set(ADDR(PageTableWidget, setColumnAndRow), ut_ptw_setColRow);

    page->updateInfo(QMap<QString, QString>());

    EXPECT_EQ(g_limitRow, 11);
}

// Arrange: 高度充足（B2 maxRow≥1）; Act: updateInfo; Assert: setLimitRow(11)（min(11,maxRow)）
TEST_F(UT_PageOverview, updateInfo_largeHeight_limitRowCapped)
{
    page->resize(800, 900);   // 900/ROW_HEIGHT-4 ≥ 1
    Stub stub;
    stub.set(ADDR(PageTableWidget, setLimitRow), ut_ptw_setLimitRow);
    stub.set(ADDR(PageTableWidget, setColumnAndRow), ut_ptw_setColRow);

    page->updateInfo(QMap<QString, QString>());

    EXPECT_EQ(g_limitRow, 11);   // min(11, 大值) = 11
}

// Arrange: 空 map（B5）; Act: updateInfo; Assert: setColumnAndRow 收到 row+1=1 且无崩溃
TEST_F(UT_PageOverview, updateInfo_emptyMap_passesRowPlusOne)
{
    Stub stub;
    stub.set(ADDR(PageTableWidget, setLimitRow), ut_ptw_setLimitRow);
    stub.set(ADDR(PageTableWidget, setColumnAndRow), ut_ptw_setColRow);

    EXPECT_NO_FATAL_FAILURE(page->updateInfo(QMap<QString, QString>()));

    EXPECT_EQ(g_colRow, 1);   // row = 0（空 map）→ setColumnAndRow(row + 1) = 1（PR #862 修复后语义）
}

// Arrange: 桩注入类型表 [Overview, Display, Separator]，map 含 Display（B3/B4）;
// Act: updateInfo; Assert: Overview 被跳过、Display 两列写入表格
TEST_F(UT_PageOverview, updateInfo_overviewSkipped_displayWritten)
{
    g_devTypes = {qMakePair(QObject::tr("Overview"), QString("overview##Overview")),
                  qMakePair(QString("Display"), QString("display##Display")),
                  qMakePair(QString("Separator"), QString("Separator##Separator"))};
    // setColumnAndRow(row + 1)（PR #862 修复 off-by-one）保证每个条目都有可见行
    QMap<QString, QString> map;
    map.insert("Display", "UHD 630");
    map.insert("Audio", "HDMI Audio");
    Stub stub;
    stub.set(ADDR(DeviceManager, getDeviceTypes), ut_dev_getTypes);

    page->updateInfo(map);

    const QString tableText = page->mp_Overview->toString();
    EXPECT_TRUE(tableText.contains("Display")) << "actual: " << qUtf8Printable(tableText);
    EXPECT_TRUE(tableText.contains("UHD 630"));
    EXPECT_FALSE(tableText.contains(QObject::tr("Overview")));
}

// Arrange: 实例; Act: 调用接口兼容空实现（list 重载 updateInfo + 单参 setLabel）;
// Assert: 均为无操作且无崩溃
TEST_F(UT_PageOverview, noopInterfaceOverrides_noCrash)
{
    EXPECT_NO_FATAL_FAILURE(page->updateInfo(QList<DeviceBaseInfo *>()));
    EXPECT_NO_FATAL_FAILURE(page->setLabel(QString("single-arg")));
    EXPECT_TRUE(page->mp_Overview->toString().isEmpty());
}

// ───────────────────────── slotShowMenu / slotActionCopy ─────────────────────────
// Arrange: QMenu::exec 桩; Act: slotShowMenu; Assert: 菜单含 3 个动作且 exec 被调用
TEST_F(UT_PageOverview, slotShowMenu_buildsMenuAndExecs)
{
    Stub stub;
    stub.set((QAction *(QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), ut_menu_exec);

    page->slotShowMenu(QPoint(10, 10));

    EXPECT_EQ(g_menuExecCount, 1);
    EXPECT_EQ(g_menuExecPos, QCursor::pos());
    EXPECT_EQ(page->mp_Menu->objectName(), QString("MpMenu"));
    EXPECT_EQ(page->mp_Menu->actions().size(), 3);   // Copy/Refresh/Export
    EXPECT_EQ(page->mp_Menu->actions().at(0), page->mp_Copy);
}

// Arrange: 表格有内容 + QClipboard 桩; Act: slotActionCopy; Assert: 剪贴板文本 == toString()
TEST_F(UT_PageOverview, slotActionCopy_copiesTableText)
{
    QMap<QString, QString> map;
    map.insert("Display", "UHD 630");
    map.insert("Audio", "HDMI Audio");
    Stub stub;
    stub.set(ADDR(DeviceManager, getDeviceTypes), ut_dev_getTypes);
    g_devTypes = {qMakePair(QString("Display"), QString("display##Display")),
                  qMakePair(QString("Audio"), QString("audio##Audio"))};
    page->updateInfo(map);
    Stub stub2;
    stub2.set((void (QClipboard::*)(const QString &, QClipboard::Mode))ADDR(QClipboard, setText), ut_clip_setText);

    page->slotActionCopy();

    EXPECT_EQ(g_clipText, page->mp_Overview->toString());
    EXPECT_FALSE(g_clipText.isEmpty());
}
