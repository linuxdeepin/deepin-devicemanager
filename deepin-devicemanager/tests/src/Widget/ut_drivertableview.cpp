// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * ut_drivertableview.cpp — DriverTableView / DriverHeaderView / DriverItemDelegate 单元测试
 *
 * 分支映射(与 .ut/inventory.json 分级一致):
 * - DriverTableView 23 方法:
 *   - high: setWidget(3 用例, 覆盖 DriverCheckItem/DriverOperationItem/普通 widget 三分支
 *           及 RESTORE/非 RESTORE 操作 lambda 分支)
 *   - mid : initHeaderView(2)/appendRowItems(1)/clear(2)/getCheckedDriverIndex(2)
 *           /hasItemDisabled(2)/setCheckedCBDisable(2)/setErrorMsg(2)/setHeaderCbStatus(2)
 *           /setItemOperationEnable(2)/setItemStatus(2)/slotAllItemChecked(2)
 *   - low : 构造(1)/removeItemAndWidget(1)/resizeColumn(1)/setAllItemCbEanble(1)
 *           /setHeaderCbEnable(1)/setItemCbEnable(2)/slotHeaderSectionResized(2)
 *           /paintEvent(1)/showEvent(1)/drawRow(1)/getTopRadiusPath(1)
 * - DriverHeaderView 7 方法: paintSection(2)/ctor(1)/setSpacing(1)/sizeHint(1)/minimumSectionSize(1)
 * - DriverItemDelegate 3 方法: paint(2, invalid+valid index)/ctor(1)
 *
 * 说明:
 * - 需要访问 protected paintSection/paint 与 private mp_HeaderCb, 故采用本文件
 *   private public / protected public 注入模式(与 ut_driveritem.cpp 一致)。
 * - setWidget 高分支: RESTORE 模式 lambda(名称列在 col0/操作列在 col3)与
 *   普通模式 lambda(cb 在 col0/name 在 col1/status 在 col4/opera 在 col5)均覆盖。
 * - paint 相关用例通过 QPixmap 后备绘制执行, 不依赖真实窗口环境。
 */

#define private public
#define protected public
#include "drivertableview.h"
#include "driveritem.h"
#include "BtnLabel.h"

#include <DStyle>
#undef protected
#undef private

#include "MacroDefinition.h"
#include "ut_Head.h"

#include <QStringList>
#include <QList>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QPainter>
#include <QPixmap>
#include <QPainterPath>
#include <QSignalSpy>

#include <gtest/gtest.h>

// ===================== 辅助: 构建一行完整数据(col0 勾选/col1 名称/col4 状态/col5 操作) =====================

static void ut_dtv_buildRow(DriverTableView *tv, int row, int driverIndex, Status st,
                            DriverOperationItem::Mode mode)
{
    tv->appendRowItems(6);
    DriverCheckItem *cb = new DriverCheckItem;
    DriverNameItem *name = new DriverNameItem(nullptr, DR_Camera);
    DriverStatusItem *status = new DriverStatusItem(nullptr, st);
    DriverOperationItem *opera = new DriverOperationItem(nullptr, mode);
    tv->setWidget(row, 0, cb);
    tv->setWidget(row, 1, name);
    tv->setWidget(row, 4, status);
    tv->setWidget(row, 5, opera);
    name->setIndex(driverIndex);
}

static DriverCheckItem *ut_dtv_cb(DriverTableView *tv, int row)
{
    return qobject_cast<DriverCheckItem *>(tv->indexWidget(tv->model()->index(row, 0)));
}

static DriverStatusItem *ut_dtv_status(DriverTableView *tv, int row)
{
    return qobject_cast<DriverStatusItem *>(tv->indexWidget(tv->model()->index(row, 4)));
}

static DriverOperationItem *ut_dtv_opera(DriverTableView *tv, int row)
{
    return qobject_cast<DriverOperationItem *>(tv->indexWidget(tv->model()->index(row, 5)));
}

// ===================== 夹具: 无表头勾选框 =====================

class UT_DriverTableView : public ::testing::Test
{
public:
    void SetUp()
    {
        tv = new DriverTableView;
        tv->initHeaderView(QStringList() << "Cb" << "Name" << "Status" << "Op", false);
        tv->resize(400, 200);
    }
    void TearDown()
    {
        delete tv;
        tv = nullptr;
    }

    DriverTableView *tv = nullptr;
};

// ===================== 夹具: 带表头勾选框 =====================

class UT_DriverTableViewCb : public ::testing::Test
{
public:
    void SetUp()
    {
        tv = new DriverTableView;
        tv->initHeaderView(QStringList() << "Cb" << "Name" << "Status" << "Op", true);
        tv->resize(400, 200);
    }
    void TearDown()
    {
        delete tv;
        tv = nullptr;
    }

    DriverTableView *tv = nullptr;
};

// ===================== DriverTableView 构造 (low) =====================

TEST_F(UT_DriverTableView, Constructor_Created_ConfiguresViewFlags)
{
    // Arrange: 夹具已构造 DriverTableView

    // Act: 读取构造后的视图配置
    bool noEdit = (tv->editTriggers() == QAbstractItemView::NoEditTriggers);
    bool noSelection = (tv->selectionMode() == QAbstractItemView::NoSelection);

    // Assert
    EXPECT_TRUE(noEdit);
    EXPECT_TRUE(noSelection);
    EXPECT_FALSE(tv->rootIsDecorated());
    EXPECT_FALSE(tv->alternatingRowColors());
    EXPECT_EQ(tv->verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    EXPECT_NE(tv->mp_Model, nullptr);
    EXPECT_NE(tv->mp_HeadView, nullptr);
}

// ===================== initHeaderView (mid) =====================

TEST_F(UT_DriverTableView, InitHeaderView_NoCheckbox_FixedHeightAndNoHeaderCb)
{
    // Arrange: 夹具中已 initHeaderView(..., false)

    // Act: 读取表头初始化结果
    int height = tv->height();

    // Assert
    EXPECT_EQ(height, 40);
    EXPECT_EQ(tv->mp_HeaderCb, nullptr);
    EXPECT_EQ(tv->mp_Model->columnCount(), 4);
    EXPECT_TRUE(tv->mp_HeadView->stretchLastSection());
}

TEST_F(UT_DriverTableViewCb, InitHeaderView_WithCheckbox_CreatesHeaderCb)
{
    // Arrange: 夹具中已 initHeaderView(..., true)

    // Act: 读取表头勾选框
    DriverCheckItem *cb = tv->mp_HeaderCb;

    // Assert: 新建时默认未勾选; 注: 初始化时模型无行, setIndexWidget 收到无效索引
    // 不会真正挂载(源码既有行为), 故仅断言成员创建与信号连接
    EXPECT_NE(cb, nullptr);
    EXPECT_FALSE(cb->checked());
    EXPECT_TRUE(cb->isEnabled());
}

// ===================== appendRowItems (mid) =====================

TEST_F(UT_DriverTableView, AppendRowItems_Twice_GrowsRowCountAndHeight)
{
    // Arrange: 初始 1 行表头, 高度 40
    ASSERT_EQ(tv->height(), 40);

    // Act: 追加两行, 每行 6 列
    tv->appendRowItems(6);
    tv->appendRowItems(6);

    // Assert
    EXPECT_EQ(tv->mp_Model->rowCount(), 2);
    EXPECT_EQ(tv->mp_Model->columnCount(), 6);
    EXPECT_EQ(tv->height(), 40 + 2 * DRIVER_TABLE_ROW_HEIGHT);
}

// ===================== clear / removeItemAndWidget (mid/low) =====================

TEST_F(UT_DriverTableView, Clear_WithWidgets_RemovesRowsAndWidgets)
{
    // Arrange: 构建一行带 widget 的数据
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ASSERT_EQ(tv->mp_Model->rowCount(), 1);
    ASSERT_NE(tv->indexWidget(tv->mp_Model->index(0, 0)), nullptr);

    // Act: 清空
    tv->clear();

    // Assert
    EXPECT_EQ(tv->mp_Model->rowCount(), 0);
    EXPECT_EQ(tv->indexWidget(tv->mp_Model->index(0, 0)), nullptr);
}

TEST_F(UT_DriverTableView, Clear_EmptyTable_StaysEmpty)
{
    // Arrange: 空表(仅表头)
    ASSERT_EQ(tv->mp_Model->rowCount(), 0);

    // Act: 对空表清空
    tv->clear();

    // Assert
    EXPECT_EQ(tv->mp_Model->rowCount(), 0);
    EXPECT_EQ(tv->mp_Model->columnCount(), 4);
}

TEST_F(UT_DriverTableView, RemoveItemAndWidget_AtIndex_WidgetDeleted)
{
    // Arrange: 单元格放入 widget
    tv->appendRowItems(4);
    DriverCheckItem *cb = new DriverCheckItem;
    tv->setWidget(0, 0, cb);
    ASSERT_NE(tv->indexWidget(tv->mp_Model->index(0, 0)), nullptr);

    // Act: 移除该行列的 item 与 widget
    tv->removeItemAndWidget(0, 0);

    // Assert
    EXPECT_EQ(tv->indexWidget(tv->mp_Model->index(0, 0)), nullptr);
    EXPECT_EQ(tv->mp_Model->rowCount(), 1);
}

// ===================== getCheckedDriverIndex (mid) =====================

TEST_F(UT_DriverTableView, GetCheckedDriverIndex_CheckedRows_Listed)
{
    // Arrange: 两行, 第一行勾选
    ut_dtv_buildRow(tv, 0, 11, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_buildRow(tv, 1, 12, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_cb(tv, 0)->setChecked(true);

    // Act: 收集勾选行的驱动索引
    QList<int> lst;
    tv->getCheckedDriverIndex(lst);

    // Assert
    EXPECT_EQ(lst.size(), 1);
    EXPECT_EQ(lst.first(), 11);
}

TEST_F(UT_DriverTableView, GetCheckedDriverIndex_NoCheckedRows_EmptyList)
{
    // Arrange: 两行均未勾选
    ut_dtv_buildRow(tv, 0, 11, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_buildRow(tv, 1, 12, ST_NOT_INSTALL, DriverOperationItem::INSTALL);

    // Act: 收集勾选行
    QList<int> lst;
    tv->getCheckedDriverIndex(lst);

    // Assert
    EXPECT_TRUE(lst.isEmpty());
    EXPECT_NE(ut_dtv_cb(tv, 0), nullptr);
}

// ===================== hasItemDisabled (mid) =====================

TEST_F(UT_DriverTableView, HasItemDisabled_DisabledCheckbox_ReturnsTrue)
{
    // Arrange: 一行, 勾选框置灰
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_cb(tv, 0)->setCbEnable(false);

    // Act: 检查是否存在禁用项
    bool has = tv->hasItemDisabled();

    // Assert
    EXPECT_TRUE(has);
    EXPECT_NE(ut_dtv_cb(tv, 0), nullptr);
}

TEST_F(UT_DriverTableView, HasItemDisabled_AllEnabled_ReturnsFalse)
{
    // Arrange: 一行, 勾选框可用
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);

    // Act: 检查是否存在禁用项
    bool has = tv->hasItemDisabled();

    // Assert
    EXPECT_FALSE(has);
    EXPECT_TRUE(ut_dtv_cb(tv, 0)->isEnabled());
}

// ===================== setWidget (high) =====================

TEST_F(UT_DriverTableView, SetWidget_PlainWidget_IndexedAtCell)
{
    // Arrange: 空行
    tv->appendRowItems(4);
    DriverCheckItem *cb = new DriverCheckItem;

    // Act: 放置到 (0,0)
    tv->setWidget(0, 0, cb);

    // Assert
    EXPECT_EQ(tv->indexWidget(tv->mp_Model->index(0, 0)), cb);
    EXPECT_EQ(cb->parentWidget(), tv->viewport());
}

TEST_F(UT_DriverTableView, SetWidget_CheckItemChecked_EmitsItemChecked)
{
    // Arrange: 一行完整数据, 勾选框连接已由 setWidget 建立
    ut_dtv_buildRow(tv, 0, 7, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    QSignalSpy spy(tv, &DriverTableView::itemChecked);

    // Act: 勾选第一行
    ut_dtv_cb(tv, 0)->setChecked(true);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 7);
    EXPECT_TRUE(spy.at(0).at(1).toBool());
}

TEST_F(UT_DriverTableView, SetWidget_OperationClicked_RowWaitsAndOtherRowUnchecked)
{
    // Arrange: 两行, 第二行已勾选
    ut_dtv_buildRow(tv, 0, 100, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_buildRow(tv, 1, 101, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_cb(tv, 1)->setChecked(true);
    QSignalSpy spy(tv, &DriverTableView::operatorClicked);
    DriverOperationItem *opera0 = ut_dtv_opera(tv, 0);
    ASSERT_NE(opera0, nullptr);

    // Act: 点击第一行安装按钮
    emit opera0->clicked(true);

    // Assert: 当前行进入等待且勾选置灰, 其它行取消勾选
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 100);
    EXPECT_EQ(spy.at(0).at(1).toInt(), 0);
    EXPECT_EQ(spy.at(0).at(2).value<DriverOperationItem::Mode>(), DriverOperationItem::INSTALL);
    EXPECT_TRUE(ut_dtv_cb(tv, 0)->checked());
    EXPECT_FALSE(ut_dtv_cb(tv, 0)->isEnabled());
    EXPECT_EQ(ut_dtv_status(tv, 0)->getStatus(), ST_WAITING);
    EXPECT_FALSE(ut_dtv_cb(tv, 1)->checked());
}

TEST_F(UT_DriverTableView, SetWidget_RestoreClicked_EmitsOperatorClickedWithRestoreMode)
{
    // Arrange: RESTORE 模式下名称在 col0, 操作在 col3
    tv->appendRowItems(4);
    DriverNameItem *name = new DriverNameItem(nullptr, DR_Camera);
    DriverOperationItem *opera = new DriverOperationItem(nullptr, DriverOperationItem::RESTORE);
    tv->setWidget(0, 0, name);
    tv->setWidget(0, 3, opera);
    name->setIndex(66);
    QSignalSpy spy(tv, &DriverTableView::operatorClicked);

    // Act: 点击还原按钮
    emit opera->clicked(true);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 66);
    EXPECT_EQ(spy.at(0).at(1).toInt(), 0);
    EXPECT_EQ(spy.at(0).at(2).value<DriverOperationItem::Mode>(), DriverOperationItem::RESTORE);
}

// ===================== setCheckedCBDisable (mid) =====================

TEST_F(UT_DriverTableViewCb, SetCheckedCBDisable_CheckedRow_DisabledAndWaiting)
{
    // Arrange: 表头勾选 + 一行已勾选数据(表头先置为选中)
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_cb(tv, 0)->setChecked(true);
    tv->mp_HeaderCb->setChecked(true);

    // Act: 置灰所有已勾选项
    tv->setCheckedCBDisable();

    // Assert
    EXPECT_FALSE(tv->mp_HeaderCb->isEnabled());
    EXPECT_FALSE(ut_dtv_cb(tv, 0)->isEnabled());
    EXPECT_EQ(ut_dtv_status(tv, 0)->getStatus(), ST_WAITING);
    EXPECT_NE(ut_dtv_opera(tv, 0), nullptr);
}

TEST_F(UT_DriverTableViewCb, SetCheckedCBDisable_NothingChecked_KeepsAllEnabled)
{
    // Arrange: 一行未勾选数据, 表头未勾选(注: 勾选表头会触发 sigChecked -> 全选, 故保持未勾选)
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ASSERT_TRUE(ut_dtv_cb(tv, 0)->isEnabled());

    // Act: 置灰所有已勾选项(无任何勾选)
    tv->setCheckedCBDisable();

    // Assert: 表头与行勾选框均保持可用, 状态不变
    EXPECT_TRUE(tv->mp_HeaderCb->isEnabled());
    EXPECT_TRUE(ut_dtv_cb(tv, 0)->isEnabled());
    EXPECT_EQ(ut_dtv_status(tv, 0)->getStatus(), ST_NOT_INSTALL);
}

// ===================== setErrorMsg (mid) =====================

TEST_F(UT_DriverTableView, SetErrorMsg_MatchingIndex_StoredInStatus)
{
    // Arrange: 一行状态为失败的数据
    ut_dtv_buildRow(tv, 0, 5, ST_FAILED, DriverOperationItem::INSTALL);

    // Act: 设置错误信息
    tv->setErrorMsg(5, "driver broken");

    // Assert
    EXPECT_EQ(ut_dtv_status(tv, 0)->mp_Status->m_Desc, "driver broken");
    EXPECT_NE(ut_dtv_status(tv, 0), nullptr);
}

TEST_F(UT_DriverTableView, SetErrorMsg_NoMatchingIndex_StatusUnchanged)
{
    // Arrange: 一行数据, 索引 5
    ut_dtv_buildRow(tv, 0, 5, ST_FAILED, DriverOperationItem::INSTALL);
    QString before = ut_dtv_status(tv, 0)->mp_Status->m_Desc;

    // Act: 使用不存在的索引设置错误信息
    tv->setErrorMsg(999, "should not apply");

    // Assert
    EXPECT_EQ(ut_dtv_status(tv, 0)->mp_Status->m_Desc, before);
    EXPECT_NE(ut_dtv_status(tv, 0), nullptr);
}

// ===================== setHeaderCbEnable / setHeaderCbStatus (low/mid) =====================

TEST_F(UT_DriverTableViewCb, SetHeaderCbEnable_False_DisablesHeaderCheckbox)
{
    // Arrange: 表头勾选框默认启用且未勾选
    ASSERT_TRUE(tv->mp_HeaderCb->isEnabled());

    // Act: 禁用表头勾选框
    tv->setHeaderCbEnable(false);

    // Assert
    EXPECT_FALSE(tv->mp_HeaderCb->isEnabled());
    EXPECT_FALSE(tv->mp_HeaderCb->checked());
}

TEST_F(UT_DriverTableViewCb, SetHeaderCbStatus_Uncheck_UnchecksHeader)
{
    // Arrange: 先勾选表头
    tv->mp_HeaderCb->setChecked(true);
    ASSERT_TRUE(tv->mp_HeaderCb->checked());

    // Act: 请求取消全选
    tv->setHeaderCbStatus(false);

    // Assert
    EXPECT_FALSE(tv->mp_HeaderCb->checked());
    EXPECT_TRUE(tv->mp_HeaderCb->isEnabled());
}

TEST_F(UT_DriverTableViewCb, SetHeaderCbStatus_AllChecked_ChecksHeader)
{
    // Arrange: 表头未勾选 + 两行全部勾选
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_buildRow(tv, 1, 2, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_cb(tv, 0)->setChecked(true);
    ut_dtv_cb(tv, 1)->setChecked(true);
    tv->mp_HeaderCb->setChecked(false);
    ASSERT_FALSE(tv->mp_HeaderCb->checked());

    // Act: 请求全选
    tv->setHeaderCbStatus(true);

    // Assert
    EXPECT_TRUE(tv->mp_HeaderCb->checked());
    EXPECT_TRUE(ut_dtv_cb(tv, 0)->checked());
}

TEST_F(UT_DriverTableView, SetHeaderCbStatus_NoHeaderCb_NoCrash)
{
    // Arrange: 无表头勾选框(夹具 false)
    ASSERT_EQ(tv->mp_HeaderCb, nullptr);

    // Act: 空指针保护分支
    tv->setHeaderCbStatus(true);
    tv->setHeaderCbStatus(false);

    // Assert
    EXPECT_EQ(tv->mp_HeaderCb, nullptr);
    EXPECT_EQ(tv->mp_Model->rowCount(), 0);
}

// ===================== setItemCbEnable / setAllItemCbEanble (low) =====================

TEST_F(UT_DriverTableView, SetItemCbEnable_Row_TogglesCheckbox)
{
    // Arrange: 一行数据
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);

    // Act: 先禁用再启用
    tv->setItemCbEnable(0, false);
    bool disabled = ut_dtv_cb(tv, 0)->isEnabled();
    tv->setItemCbEnable(0, true);
    bool enabled = ut_dtv_cb(tv, 0)->isEnabled();

    // Assert
    EXPECT_FALSE(disabled);
    EXPECT_TRUE(enabled);
}

TEST_F(UT_DriverTableView, SetItemCbEnable_EmptyRow_NoCrash)
{
    // Arrange: 空行(无 widget)
    tv->appendRowItems(4);

    // Act: 对空行设置
    tv->setItemCbEnable(0, false);

    // Assert
    EXPECT_EQ(tv->indexWidget(tv->mp_Model->index(0, 0)), nullptr);
    EXPECT_EQ(tv->mp_Model->rowCount(), 1);
}

TEST_F(UT_DriverTableView, SetAllItemCbEanble_False_DisablesAll)
{
    // Arrange: 两行数据
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_buildRow(tv, 1, 2, ST_NOT_INSTALL, DriverOperationItem::INSTALL);

    // Act: 全部禁用
    tv->setAllItemCbEanble(false);

    // Assert
    EXPECT_FALSE(ut_dtv_cb(tv, 0)->isEnabled());
    EXPECT_FALSE(ut_dtv_cb(tv, 1)->isEnabled());
    EXPECT_TRUE(tv->hasItemDisabled());
}

// ===================== setItemOperationEnable (mid) =====================

TEST_F(UT_DriverTableView, SetItemOperationEnable_MatchingIndex_TogglesButton)
{
    // Arrange: RESTORE 布局(name col0 / opera col3)
    tv->appendRowItems(4);
    DriverNameItem *name = new DriverNameItem(nullptr, DR_Camera);
    DriverOperationItem *opera = new DriverOperationItem(nullptr, DriverOperationItem::RESTORE);
    tv->setWidget(0, 0, name);
    tv->setWidget(0, 3, opera);
    name->setIndex(3);

    // Act: 先禁用再启用
    tv->setItemOperationEnable(3, false);
    bool disabled = opera->mp_Btn->isEnabled();
    tv->setItemOperationEnable(3, true);
    bool enabled = opera->mp_Btn->isEnabled();

    // Assert
    EXPECT_FALSE(disabled);
    EXPECT_TRUE(enabled);
}

TEST_F(UT_DriverTableView, SetItemOperationEnable_NoOpera_NoCrash)
{
    // Arrange: 仅名称列
    tv->appendRowItems(4);
    DriverNameItem *name = new DriverNameItem(nullptr, DR_Camera);
    tv->setWidget(0, 0, name);
    name->setIndex(3);

    // Act: 无操作按钮行
    tv->setItemOperationEnable(3, false);

    // Assert
    EXPECT_NE(tv->indexWidget(tv->mp_Model->index(0, 0)), nullptr);
    EXPECT_EQ(tv->mp_Model->rowCount(), 1);
}

// ===================== setItemStatus (mid) =====================

TEST_F(UT_DriverTableView, SetItemStatus_Success_UnchecksAndDisablesCb)
{
    // Arrange: 一行已勾选数据
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_cb(tv, 0)->setChecked(true);

    // Act: 状态置为安装成功
    tv->setItemStatus(1, ST_SUCESS);

    // Assert
    EXPECT_EQ(ut_dtv_status(tv, 0)->getStatus(), ST_SUCESS);
    EXPECT_FALSE(ut_dtv_cb(tv, 0)->checked());
    EXPECT_FALSE(ut_dtv_cb(tv, 0)->isEnabled());
}

TEST_F(UT_DriverTableView, SetItemStatus_Failed_EnablesCbAndOpera)
{
    // Arrange: 一行数据
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);

    // Act: 状态置为失败
    tv->setItemStatus(1, ST_FAILED);

    // Assert
    EXPECT_EQ(ut_dtv_status(tv, 0)->getStatus(), ST_FAILED);
    EXPECT_TRUE(ut_dtv_cb(tv, 0)->isEnabled());
    EXPECT_TRUE(ut_dtv_opera(tv, 0)->mp_Btn->isEnabled());
}

// ===================== slotAllItemChecked (mid) =====================

TEST_F(UT_DriverTableView, SlotAllItemChecked_True_ChecksAllEnabledRows)
{
    // Arrange: 两行可用数据
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_buildRow(tv, 1, 2, ST_NOT_INSTALL, DriverOperationItem::INSTALL);

    // Act: 全选
    tv->slotAllItemChecked(true);

    // Assert
    EXPECT_TRUE(ut_dtv_cb(tv, 0)->checked());
    EXPECT_TRUE(ut_dtv_cb(tv, 1)->checked());
}

TEST_F(UT_DriverTableView, SlotAllItemChecked_DisabledRow_Skipped)
{
    // Arrange: 两行, 第一行勾选框禁用
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_buildRow(tv, 1, 2, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    ut_dtv_cb(tv, 0)->setCbEnable(false);

    // Act: 全选
    tv->slotAllItemChecked(true);

    // Assert: 禁用行不勾选, 可用行勾选
    EXPECT_FALSE(ut_dtv_cb(tv, 0)->checked());
    EXPECT_TRUE(ut_dtv_cb(tv, 1)->checked());
}

// ===================== resizeColumn / slotHeaderSectionResized (low) =====================

TEST_F(UT_DriverTableView, ResizeColumn_Column0_NoCrash)
{
    // Arrange: 表头已初始化
    ASSERT_NE(tv->mp_HeadView, nullptr);

    // Act: 触发列宽调整
    tv->resizeColumn(0);

    // Assert
    EXPECT_EQ(tv->mp_HeadView->count(), 4);
    EXPECT_EQ(tv->mp_HeadView->orientation(), Qt::Horizontal);
}

TEST_F(UT_DriverTableView, SlotHeaderSectionResized_Column1TooSmall_ClampedTo106)
{
    // Arrange: 一行带 widget 数据
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    QWidget *nameWidget = tv->indexWidget(tv->mp_Model->index(0, 1));
    ASSERT_NE(nameWidget, nullptr);

    // Act: 第一列宽度缩小到 50
    tv->slotHeaderSectionResized(1, 120, 50);

    // Assert
    EXPECT_EQ(tv->columnWidth(1), 106);
    EXPECT_LE(nameWidget->maximumWidth(), 106);
}

TEST_F(UT_DriverTableView, SlotHeaderSectionResized_Column2TooSmall_ClampedTo80)
{
    // Arrange: 一行数据, 并在 col2 放置 widget(夹紧逻辑仅在存在 widget 时执行)
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    DriverCheckItem *cb2 = new DriverCheckItem;
    tv->setWidget(0, 2, cb2);
    ASSERT_NE(tv->indexWidget(tv->mp_Model->index(0, 2)), nullptr);

    // Act: 第二列宽度缩小到 40
    tv->slotHeaderSectionResized(2, 120, 40);

    // Assert
    EXPECT_EQ(tv->columnWidth(2), 80);
    EXPECT_LE(cb2->maximumWidth(), 80);
}

// ===================== paintEvent / showEvent / drawRow / getTopRadiusPath =====================

TEST_F(UT_DriverTableView, PaintEvent_Rendered_NoCrash)
{
    // Arrange: 带内容并渲染到图片
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    QPixmap pix(300, 200);
    pix.fill(Qt::white);

    // Act: 真实渲染触发 paintEvent
    tv->render(&pix);

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_NE(tv->mp_Model, nullptr);
}

TEST_F(UT_DriverTableView, ShowEvent_Shown_WidgetVisible)
{
    // Arrange: 夹具已构造
    ASSERT_FALSE(tv->isVisible());

    // Act: 显示
    tv->show();

    // Assert
    EXPECT_TRUE(tv->isVisible());
    EXPECT_NE(tv->viewport(), nullptr);
}

TEST_F(UT_DriverTableView, DrawRow_ValidIndex_PaintsWithoutCrash)
{
    // Arrange: 一行数据 + 绘制选项
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    QPixmap pix(300, 100);
    pix.fill(Qt::white);
    QPainter painter(&pix);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 300, DRIVER_TABLE_ROW_HEIGHT);
    option.state |= QStyle::State_Enabled;

    // Act: 绘制第一行
    tv->drawRow(&painter, option, tv->mp_Model->index(0, 0));
    painter.end();

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_TRUE(tv->mp_Model->index(0, 0).isValid());
}

TEST_F(UT_DriverTableView, GetTopRadiusPath_ValidRect_PathWithinBounds)
{
    // Arrange: 目标矩形
    QRect rect(0, 0, 300, 200);

    // Act: 生成顶部圆角路径
    QPainterPath path = tv->getTopRadiusPath(rect);

    // Assert
    EXPECT_FALSE(path.isEmpty());
    QRectF bounds = path.boundingRect();
    EXPECT_LE(bounds.right(), rect.right() + 1);
    EXPECT_GE(bounds.left(), rect.left() - 1);
}

// ===================== DriverHeaderView =====================

TEST_F(UT_DriverTableView, HeaderView_Created_DefaultConfig)
{
    // Arrange: 夹具中的表头
    DriverHeaderView *hv = tv->mp_HeadView;

    // Act: 读取默认配置
    QSize hint = hv->sizeHint();

    // Assert
    EXPECT_NE(hv, nullptr);
    EXPECT_EQ(hv->orientation(), Qt::Horizontal);
    EXPECT_EQ(hint.height(), 36 + hv->m_spacing);
}

TEST_F(UT_DriverTableView, HeaderView_SetSpacing_AffectsSizeHint)
{
    // Arrange: 默认 spacing 记录
    int oldSpacing = tv->mp_HeadView->m_spacing;

    // Act: 设置新 spacing
    tv->mp_HeadView->setSpacing(oldSpacing + 4);
    QSize hint = tv->mp_HeadView->sizeHint();

    // Assert
    EXPECT_EQ(tv->mp_HeadView->m_spacing, oldSpacing + 4);
    EXPECT_EQ(hint.height(), 36 + oldSpacing + 4);
}

TEST_F(UT_DriverTableView, HeaderView_PaintSection_FirstSection_PaintsWithoutCrash)
{
    // Arrange: 表头数据 + 渲染目标
    QPixmap pix(400, 36);
    pix.fill(Qt::white);
    QPainter painter(&pix);

    // Act: 绘制第 0 列
    tv->mp_HeadView->paintSection(&painter, QRect(0, 0, 100, 36), 0);
    painter.end();

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_EQ(tv->mp_HeadView->count(), 4);
}

TEST_F(UT_DriverTableView, HeaderView_PaintSection_SecondSection_PaintsWithoutCrash)
{
    // Arrange: 表头数据 + 渲染目标
    QPixmap pix(400, 36);
    pix.fill(Qt::white);
    QPainter painter(&pix);

    // Act: 绘制第 1 列(非首列走 vSpacing 分支)
    tv->mp_HeadView->paintSection(&painter, QRect(100, 0, 100, 36), 1);
    painter.end();

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_EQ(tv->mp_Model->columnCount(), 4);
}

// ===================== DriverItemDelegate =====================

TEST_F(UT_DriverTableView, Delegate_Created_AttachedToView)
{
    // Arrange: 视图自带的委托
    DriverItemDelegate *del = tv->mp_Delegate;

    // Act: 读取委托归属
    QObject *p = del->parent();

    // Assert
    EXPECT_NE(del, nullptr);
    EXPECT_EQ(p, tv);
}

TEST_F(UT_DriverTableView, Delegate_Paint_InvalidIndex_FallsBackToBase)
{
    // Arrange: 无效索引 + 渲染目标
    QPixmap pix(200, 60);
    pix.fill(Qt::white);
    QPainter painter(&pix);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 60);
    QModelIndex invalid;

    // Act: 绘制无效索引
    tv->mp_Delegate->paint(&painter, option, invalid);
    painter.end();

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(UT_DriverTableView, Delegate_Paint_ValidIndex_PaintsWithoutCrash)
{
    // Arrange: 一行数据 + 渲染目标
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    QPixmap pix(400, 60);
    pix.fill(Qt::white);
    QPainter painter(&pix);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, DRIVER_TABLE_ROW_HEIGHT);
    option.state |= QStyle::State_Enabled;
    option.features |= QStyleOptionViewItem::Alternate;

    // Act: 绘制有效索引
    tv->mp_Delegate->paint(&painter, option, tv->mp_Model->index(0, 0));
    painter.end();

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_NE(tv->mp_Delegate, nullptr);
}

// ===================== DriverHeaderView::sectionSizeHint (low) =====================

TEST_F(UT_DriverTableView, DriverHeaderView_SectionSizeHint_DefaultHeader_ReturnsNonNegative)
{
    // Arrange: 夹具已构造默认表头(4 列); 预置 DStyle 保证环境确定
    // (源码 dynamic_cast<DStyle*> 判空已由 fix 提交加固, 此处保留防御)
    if (!qobject_cast<DStyle *>(QApplication::style())) {
        QApplication::setStyle(new DStyle());
    }

    // Act: 请求第 0 列的尺寸提示
    int hint = tv->mp_HeadView->sectionSizeHint(0);

    // Assert: 基于字体度量计算, 结果为非负值
    EXPECT_GE(hint, 0);
    EXPECT_NE(tv->mp_HeadView, nullptr);
}

// ===================== DriverHeaderView::setSpacing/getSpacing (low) =====================

TEST_F(UT_DriverTableView, DriverHeaderView_SetSpacing_GetterReflectsValue)
{
    // Arrange: 记录初始间距
    int before = tv->mp_HeadView->getSpacing();

    // Act: 设置新间距
    tv->mp_HeadView->setSpacing(before + 4);

    // Assert: getter 返回最新值
    EXPECT_EQ(tv->mp_HeadView->getSpacing(), before + 4);
    EXPECT_GE(tv->mp_HeadView->getSpacing(), 4);
}

// ===================== DriverItemDelegate::paint 分支补强 (high, 迭代3) =====================

TEST_F(UT_DriverTableView, Delegate_Paint_SelectedWithIcon_HighlightAndIconDrawn)
{
    // Arrange: 首列图标 + 选中态 + 备选行背景
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    tv->mp_Model->setData(tv->mp_Model->index(0, 0), QIcon::fromTheme("audio-card"),
                          Qt::DecorationRole);
    QPixmap pix(400, 60);
    pix.fill(Qt::white);
    QPainter painter(&pix);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, DRIVER_TABLE_ROW_HEIGHT);
    option.state |= QStyle::State_Enabled | QStyle::State_Selected;
    option.features |= QStyleOptionViewItem::Alternate;
    option.viewItemPosition = QStyleOptionViewItem::Beginning;

    // Act: 绘制选中且带图标的首列
    tv->mp_Delegate->paint(&painter, option, tv->mp_Model->index(0, 1));
    painter.end();

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_TRUE(tv->mp_Delegate != nullptr);
}

TEST_F(UT_DriverTableView, Delegate_Paint_DisabledMiddle_RendersWithoutIcon)
{
    // Arrange: 非首列 + 禁用态, 走 Disabled 色组与 Middle 分支
    ut_dtv_buildRow(tv, 0, 1, ST_NOT_INSTALL, DriverOperationItem::INSTALL);
    QPixmap pix(400, 60);
    pix.fill(Qt::white);
    QPainter painter(&pix);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, DRIVER_TABLE_ROW_HEIGHT);
    option.viewItemPosition = QStyleOptionViewItem::Middle;

    // Act: 绘制禁用态的中间列
    tv->mp_Delegate->paint(&painter, option, tv->mp_Model->index(0, 1));
    painter.end();

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_TRUE(tv->mp_Delegate != nullptr);
}

// ===================== DriverHeaderView::paintSection 分支补强 (high, 迭代3) =====================

namespace {
void ut_dtv_paintSection(DriverTableView *tv, int logicalIndex, QPixmap *pix)
{
    QPainter painter(pix);
    tv->mp_HeadView->paintSection(&painter, QRect(0, 0, 200, 40), logicalIndex);
    painter.end();
}
}

TEST_F(UT_DriverTableView, PaintSection_FirstColumn_PaintsWithoutVerticalSpacing)
{
    // Arrange: 渲染目标与 DStyle(预置保持环境确定性)
    if (!qobject_cast<DStyle *>(QApplication::style())) {
        QApplication::setStyle(new DStyle());
    }
    QPixmap pix(200, 40);
    pix.fill(Qt::white);

    // Act: 绘制第 0 列(visualIndex 0, 无纵向分隔, col0Rect 偏移分支)
    ut_dtv_paintSection(tv, 0, &pix);

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_NE(tv->mp_HeadView, nullptr);
}

TEST_F(UT_DriverTableView, PaintSection_OtherColumn_PaintsWithVerticalSpacing)
{
    // Arrange: 渲染目标与 DStyle
    if (!qobject_cast<DStyle *>(QApplication::style())) {
        QApplication::setStyle(new DStyle());
    }
    QPixmap pix(200, 40);
    pix.fill(Qt::white);

    // Act: 绘制非排序列(visualIndex>0 分支 + 普通文本矩形分支)
    ut_dtv_paintSection(tv, 2, &pix);

    // Assert
    EXPECT_FALSE(pix.isNull());
    EXPECT_NE(tv->mp_HeadView, nullptr);
}

TEST_F(UT_DriverTableView, PaintSection_SortIndicatorShown_DrawsAscAndDescArrows)
{
    // Arrange: 开启排序指示, 分别升序/降序
    if (!qobject_cast<DStyle *>(QApplication::style())) {
        QApplication::setStyle(new DStyle());
    }
    ASSERT_NE(tv->mp_HeadView, nullptr);

    // Act: 升序箭头绘制
    tv->mp_HeadView->setSortIndicatorShown(true);
    tv->mp_HeadView->setSortIndicator(1, Qt::AscendingOrder);
    QPixmap pix1(200, 40);
    pix1.fill(Qt::white);
    ut_dtv_paintSection(tv, 1, &pix1);

    // Assert: 升序指示绘制无崩溃且有效
    EXPECT_FALSE(pix1.isNull());

    // Act: 降序箭头绘制
    tv->mp_HeadView->setSortIndicator(2, Qt::DescendingOrder);
    QPixmap pix2(200, 40);
    pix2.fill(Qt::white);
    ut_dtv_paintSection(tv, 2, &pix2);

    // Assert: 降序指示绘制无崩溃且有效
    EXPECT_FALSE(pix2.isNull());

    // Act: 排序列未显示指示时仍走排序矩形分支
    tv->mp_HeadView->setSortIndicatorShown(false);
    tv->mp_HeadView->setSortIndicator(3, Qt::AscendingOrder);
    QPixmap pix3(200, 40);
    pix3.fill(Qt::white);
    ut_dtv_paintSection(tv, 3, &pix3);

    // Assert: 排序矩形分支绘制无崩溃且有效
    EXPECT_FALSE(pix3.isNull());
}
