// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method           | level | factors                                          | min | actual |
// |------------------|-------|--------------------------------------------------|-----|--------|
// | PageTableHeader  | mid   | in_degree:3                                      | 2   | 2      |
// | updateTable      | high  | complexity:7, lines:55, transitive_loop_depth:3  | 4   | 6      |// | initWidgets      | low   | -                                                | 1   | 1      |
// | paintEvent       | low   | -                                                | 1   | 1      |
// | setColumnAverage | low   | -                                                | 1   | 1      |
// | ~PageTableHeader | low   | destructor                                       | 1   | 1      |
// ─── actual >= min 达标 ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]
// 4. 同质 ≥ 3 组用 TEST_P: [x]
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（本类方法无显式 throw，异常路径为不适用）
// 8. 负面场景有专门用例: [x]
// 9. 负面用例验证强异常安全: [x]（early-return 用例验证表格操作计数保持 0）
// 10. stub_ext vs gMock 选择正确: [x]（Qt/项目内无虚注入类 → stub_ext；无接口类 → 不用 gMock）
//
// mock 深度分析（test-code-gen §4.0，来源：MCP get_code_snippet + trace_path）：
// - PageTableHeader 为 GUI 类（DWidget 子类），QApplication 由 tests/src/test_main.cpp 统一
//   以 QT_QPA_PLATFORM=offscreen 创建，本文件不重复创建 QApplication。
// - updateTable 的出向依赖全部落在 mp_Table（TableWidget，本项目 src/Widget/TableWidget.cpp，
//   已编入测试目标）与 this->setFixedHeight：均按 stub 清单以 stub_ext 运行时替换，
//   捕获调用参数/次数做副作用断言，不触真实表格插入逻辑。
// - TableWidget 方法签名经 MCP get_code_snippet 确认均无重载：
//   void clear(); void setHeaderLabels(const QStringList&); void setRowNum(int);
//   void setItem(int, int, DStandardItem*); void setColumnAverage();
// - QWidget::setFixedHeight(int) 为 Qt 类方法，按 Iron Law #13 用 static_cast 消歧，禁用 VADDR。
// - 无文件系统/网络/进程/环境变量依赖；DBus 调用已由 test_main.cpp 全局桩拦截。
//
// 分支清单（来源：MCP get_code_snippet → PageTableHeader::updateTable 完整方法体）
// B1: resizeTable == true            → configRowNum = ROW_NUM - step
// B2: lst.size() <= 1                → early return（不触碰表格）
// B3: row - 1 < configRowNum         → setRowNum(row-1)，高度=TREE_ROW_HEIGHT*row+HORSCROLL_WIDTH+WIDGET_MARGIN*2+BOTTOM_MARGIN
// B4: row - 1 >= configRowNum        → setRowNum(configRowNum)，高度=TREE_ROW_HEIGHT*(configRowNum+1)+其余同上
// B5: j == 0（每数据行首列）          → 内层循环按 lstMenuControl[i][index] 写 item->setData(UserRole+index)
// B6: 双层循环填充全部单元格          → setItem 共 (row-1)*column 次，末尾 setColumnAverage()
//
// 用例映射：
// - Ctor_ConstructWidgets_CreatesTableAndLayout        → 构造函数路径（mp_Table 创建 + 信号连接）
// - InitWidgets_SetLayout_ContainsTableWidget          → initWidgets 布局挂载
// - UpdateTable_EmptyList_EarlyReturnNoTableOps        → B2
// - UpdateTable_RowConfig_CapsRowNumAndHeight (TEST_P) → B1+B3+B4+B6（4 组参数：B1 真/假 × B3/B4）
// - UpdateTable_DataRows_FillsItemsAndMenuData         → B5+B6（含 item 数据/UserRole 断言）
// - SetColumnAverage_CallMethod_ForwardsToTable        → setColumnAverage 转发
// - PaintEvent_DelegatesBase_RendersWidgetContent      → paintEvent 基类委托
// - Destructor_DeleteObject_CleansTableChild           → 析构清理

#include <gtest/gtest.h>
#include <QPaintEvent>
#include <QPointer>

#include "stubext.h"
#include "PageTableHeader.h"
#include "TableWidget.h"
#include "MacroDefinition.h"

// ═══════════════════════════════════════════════════════════════
// 参数化数据结构（须置于 fixture 之前）
// ═══════════════════════════════════════════════════════════════
struct UpdateTableParam {
    int rows;          // lst 总行数（含表头行）
    bool resizeTable;  // 是否启用 step 调整
    int step;          // resizeTable 时的步长
};

class PageTableHeaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Arrange（公共前置）：构造被测 GUI 对象（offscreen 平台安全）
            obj = new PageTableHeader;
    }

    void TearDown() override {
        // 先卸载全部桩再析构被测对象：避免析构路径调用已打桩方法时
        // 触达指向测试函数栈帧（已销毁）的捕获引用
        stub.clear();
        delete obj;
        obj = nullptr;
    }

    stub_ext::StubExt stub;
    PageTableHeader *obj = nullptr;
};

class PageTableHeaderUpdateTableTest : public PageTableHeaderTest,
                                       public ::testing::WithParamInterface<UpdateTableParam> {
};

// ═══════════════════════════════════════════════════════════════
// 构造函数（mid, in_degree:3）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableHeaderTest, Ctor_ConstructWidgets_CreatesTableAndLayout)
{
    // Arrange
    ASSERT_NE(obj, nullptr);  // SetUp 已构造被测对象，构造路径已执行

    // Act：取内部表格指针供断言（构造路径已在 SetUp 执行）
    QWidget *table = obj->mp_Table;

    // Assert：内部表格被创建、以自身为父对象，且已挂入布局
    ASSERT_NE(table, nullptr);
    EXPECT_EQ(table->parentWidget(), obj);
    EXPECT_NE(obj->layout(), nullptr);
}

TEST_F(PageTableHeaderTest, Ctor_TwoInstances_HaveIndependentTables)
{
    // Arrange
    ASSERT_NE(obj, nullptr);

    // Act：再次构造第二个实例，验证实例间互不共享内部状态
    PageTableHeader obj2;

    // Assert：各实例持有独立的内部表格并各自挂载布局
    EXPECT_NE(obj2.mp_Table, obj->mp_Table);
    EXPECT_EQ(obj2.mp_Table->parentWidget(), &obj2);
    ASSERT_NE(obj2.layout(), nullptr);
    EXPECT_EQ(obj2.layout()->count(), 1);
}

TEST_F(PageTableHeaderTest, InitWidgets_SetLayout_ContainsTableWidget)
{
    // Arrange
    ASSERT_NE(obj->layout(), nullptr);

    // Act：取构造路径中 initWidgets 装配的布局
    QLayout *layout = obj->layout();

    // Assert：布局中恰好挂载了 mp_Table 一个控件
    ASSERT_NE(layout, nullptr);
    EXPECT_EQ(layout->count(), 1);
    EXPECT_EQ(layout->indexOf(obj->mp_Table), 0);
}

// ═══════════════════════════════════════════════════════════════
// updateTable（high, complexity:7, lines:55, transitive_loop_depth:3）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableHeaderTest, UpdateTable_EmptyList_EarlyReturnNoTableOps)
{
    // Arrange：lst 只有表头（size==1，命中 B2 边界）并安装全量计数桩
    QList<QStringList> lst;
    lst << QStringList{"Name", "Value", "Vendor"};
    int clearCount = 0, headerCount = 0, itemCallCount = 0, avgCount = 0;
    stub.set_lamda(&TableWidget::clear, [&](TableWidget *) { ++clearCount; });
    stub.set_lamda(&TableWidget::setHeaderLabels, [&](TableWidget *, const QStringList &) { ++headerCount; });
    stub.set_lamda(&TableWidget::setItem, [&](TableWidget *, int, int, DStandardItem *item) {
        ++itemCallCount;
        delete item;  // 桩接管所有权，防泄漏
    });
    stub.set_lamda(&TableWidget::setColumnAverage, [&](TableWidget *) { ++avgCount; });

    // Act
    obj->updateTable(lst, QList<QStringList>(), false, 0);

    // Assert：early-return 路径不触发任何表格操作（强异常安全：表格状态未被扰动）
    EXPECT_EQ(clearCount, 0);
    EXPECT_EQ(headerCount, 0);
    EXPECT_EQ(itemCallCount, 0);
    EXPECT_EQ(avgCount, 0);
}

TEST_P(PageTableHeaderUpdateTableTest, UpdateTable_RowConfig_CapsRowNumAndHeight)
{
    // Arrange：表头 3 列 + (rows-1) 数据行；按参数装桩捕获关键调用
    const UpdateTableParam &p = GetParam();
    QList<QStringList> lst;
    lst << QStringList{"Name", "Value", "Vendor"};
    for (int i = 1; i < p.rows; ++i)
        lst << QStringList{QString("dev%1").arg(i), "val", "ven"};
    QList<QStringList> lstMenuControl;
    for (int i = 0; i < p.rows - 1; ++i)
        lstMenuControl << QStringList{"m0", "m1"};

    int capturedRowNum = -1, capturedHeight = -1, avgCount = 0, itemCallCount = 0;
    stub.set_lamda(&TableWidget::setRowNum, [&](TableWidget *, int row) { capturedRowNum = row; });
    stub.set_lamda(static_cast<void (QWidget::*)(int)>(&QWidget::setFixedHeight),
                   [&](QWidget *, int h) { capturedHeight = h; });
    stub.set_lamda(&TableWidget::setColumnAverage, [&](TableWidget *) { ++avgCount; });
    stub.set_lamda(&TableWidget::setItem, [&](TableWidget *, int, int, DStandardItem *item) {
        ++itemCallCount;
        delete item;  // 桩接管所有权，防泄漏
    });

    // Act
    obj->updateTable(lst, lstMenuControl, p.resizeTable, p.step);

    // Assert：行号取 min(row-1, configRowNum)，高度公式随分支一致（覆盖 B1/B3/B4/B6）
    const int configRowNum = p.resizeTable ? ROW_NUM - p.step : ROW_NUM;  // B1
    const int expectedRowNum = qMin(p.rows - 1, configRowNum);
    const int expectedHeight = TREE_ROW_HEIGHT * (expectedRowNum + 1)
                               + HORSCROLL_WIDTH + WIDGET_MARGIN * 2 + BOTTOM_MARGIN;
    EXPECT_EQ(capturedRowNum, expectedRowNum);
    EXPECT_EQ(capturedHeight, expectedHeight);
    EXPECT_EQ(avgCount, 1);
    EXPECT_EQ(itemCallCount, (p.rows - 1) * 2);  // column = 3 - 1
}

INSTANTIATE_TEST_SUITE_P(
    UpdateTableRowConfig, PageTableHeaderUpdateTableTest,
    ::testing::Values(
        UpdateTableParam{2, false, 0},   // 单数据行：B3（row-1=1 < ROW_NUM），B1 假
        UpdateTableParam{2, true, 1},    // 单数据行 + resize：B3，B1 真（configRowNum=ROW_NUM-1）
        UpdateTableParam{10, false, 0},  // 多数据行：B4（9 >= ROW_NUM），B1 假
        UpdateTableParam{10, true, 2}    // 多数据行 + resize：B4，B1 真（configRowNum=ROW_NUM-2）
    ));

TEST_F(PageTableHeaderTest, UpdateTable_DataRows_FillsItemsAndMenuData)
{
    // Arrange：2 数据行 × 3 列；捕获各单元格文本与首列菜单数据（覆盖 B5/B6）
    QList<QStringList> lst;
    lst << QStringList{"Name", "Value", "Vendor"};
    lst << QStringList{"cpu0", "8C", "Intel"};
    lst << QStringList{"gpu0", "RTX", "NVIDIA"};
    QList<QStringList> lstMenuControl;
    lstMenuControl << QStringList{"m0", "m1"};
    lstMenuControl << QStringList{"n0", "n1"};

    QHash<QPair<int, int>, QString> texts;
    QString menu0, menu1, menu1Row1;
    stub.set_lamda(&TableWidget::setItem, [&](TableWidget *, int row, int col, DStandardItem *item) {
        texts.insert(qMakePair(row, col), item->text());
        if (row == 0 && col == 0) {
            menu0 = item->data(Qt::UserRole).toString();
            menu1 = item->data(Qt::UserRole + 1).toString();
        }
        if (row == 1 && col == 0)
            menu1Row1 = item->data(Qt::UserRole + 1).toString();
        delete item;  // 桩接管所有权，防泄漏
    });

    // Act
    obj->updateTable(lst, lstMenuControl, false, 0);

    // Assert：单元格全量填充 + 首列 UserRole 菜单数据精确匹配
    EXPECT_EQ(texts.size(), 4);  // (rows-1) * column = 2 * 2
    EXPECT_EQ(texts.value(qMakePair(0, 0)), QString("cpu0"));
    EXPECT_EQ(texts.value(qMakePair(1, 1)), QString("RTX"));
    EXPECT_EQ(menu0, QString("m0"));
    EXPECT_EQ(menu1, QString("m1"));
    EXPECT_EQ(menu1Row1, QString("n1"));
}

// ═══════════════════════════════════════════════════════════════
// setColumnAverage（low）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableHeaderTest, SetColumnAverage_CallMethod_ForwardsToTable)
{
    // Arrange
    ASSERT_NE(obj->mp_Table, nullptr);
    int avgCount = 0;
    stub.set_lamda(&TableWidget::setColumnAverage, [&](TableWidget *) { ++avgCount; });

    // Act
    obj->setColumnAverage();

    // Assert：转发恰好一次到内部表格；再调一次确认转发持续有效
    EXPECT_EQ(avgCount, 1);
    obj->setColumnAverage();
    EXPECT_EQ(avgCount, 2);
}

// ═══════════════════════════════════════════════════════════════
// paintEvent（low）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableHeaderTest, PaintEvent_DelegatesBase_RendersWidgetContent)
{
    // Arrange
    QPaintEvent paintEvent(obj->rect());

    // Act
    obj->paintEvent(&paintEvent);

    // Assert：基类委托后控件可正常离屏渲染，画布尺寸与控件一致
    const QPixmap grabbed = obj->grab();
    EXPECT_FALSE(grabbed.isNull());
    EXPECT_EQ(grabbed.size(), obj->size());
}

// ═══════════════════════════════════════════════════════════════
// ~PageTableHeader（low, destructor）
// ═══════════════════════════════════════════════════════════════
TEST_F(PageTableHeaderTest, Destructor_DeleteObject_CleansTableChild)
{
    // Arrange：独立堆对象 + QPointer 守卫观察析构效果
    PageTableHeader *local = new PageTableHeader;
    QPointer<PageTableHeader> headerGuard(local);
    QPointer<QWidget> tableGuard(local->mp_Table);
    ASSERT_FALSE(headerGuard.isNull());
    EXPECT_NE(local->mp_Table, static_cast<QWidget *>(nullptr));

    // Act
    delete local;
    local = nullptr;

    // Assert：析构删除内部表格并整体销毁
    EXPECT_TRUE(tableGuard.isNull());
    EXPECT_TRUE(headerGuard.isNull());
}
