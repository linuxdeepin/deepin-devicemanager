// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QResizeEvent>
#include <QApplication>

#include "ut_Head.h"
#include "stub.h"

#define private public
#include "PageDriverTableView.h"
#include "drivertableview.h"
#include "driveritem.h"
#include <DLabel>

DWIDGET_USE_NAMESPACE

class UT_PageDriverTableView : public UT_HEAD
{
public:
    void SetUp()
    {
        m_dPageDriverTableView = new PageDriverTableView;
    }
    void TearDown()
    {
        delete m_dPageDriverTableView;
    }
    PageDriverTableView *m_dPageDriverTableView;
    Stub stub;
};

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_ctor)
{
    // 构造后内部表格已创建,外层固定高度为表头高度 40,模型可用
    EXPECT_NE(nullptr, m_dPageDriverTableView->mp_View);
    EXPECT_NE(nullptr, m_dPageDriverTableView->model());
    EXPECT_EQ(40, m_dPageDriverTableView->minimumHeight());
    EXPECT_EQ(40, m_dPageDriverTableView->maximumHeight());
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_appendRowItems)
{
    // 追加一行 6 列 item:模型行数 +1,整体高度增长一个行高 64
    m_dPageDriverTableView->appendRowItems(6);
    EXPECT_EQ(1, m_dPageDriverTableView->mp_View->mp_Model->rowCount());
    EXPECT_EQ(6, m_dPageDriverTableView->mp_View->mp_Model->columnCount());
    EXPECT_EQ(40 + 64, m_dPageDriverTableView->height());

    m_dPageDriverTableView->appendRowItems(6);
    EXPECT_EQ(2, m_dPageDriverTableView->mp_View->mp_Model->rowCount());
    EXPECT_EQ(40 + 64 * 2, m_dPageDriverTableView->height());
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_clear)
{
    // clear 后行清空,整体高度恢复为表头高度 + 5
    m_dPageDriverTableView->appendRowItems(6);
    m_dPageDriverTableView->appendRowItems(6);
    ASSERT_EQ(2, m_dPageDriverTableView->mp_View->mp_Model->rowCount());

    m_dPageDriverTableView->clear();

    EXPECT_EQ(0, m_dPageDriverTableView->mp_View->mp_Model->rowCount());
    EXPECT_EQ(40 + 5, m_dPageDriverTableView->height());
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_initHeaderView)
{
    // 初始化表头:列数与表头文本一致;重复调用不崩溃
    QStringList headers;
    headers << "a" << "b" << "c" << "d" << "e" << "f";
    m_dPageDriverTableView->initHeaderView(headers, true);
    EXPECT_EQ(6, m_dPageDriverTableView->mp_View->mp_Model->columnCount());
    EXPECT_STREQ("a", m_dPageDriverTableView->mp_View->mp_Model->horizontalHeaderItem(0)->text().toStdString().c_str());
    EXPECT_STREQ("f", m_dPageDriverTableView->mp_View->mp_Model->horizontalHeaderItem(5)->text().toStdString().c_str());

    m_dPageDriverTableView->initHeaderView(headers, true);
    SUCCEED();
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_setWidget)
{
    // setWidget 后指定单元格挂载自定义控件
    m_dPageDriverTableView->appendRowItems(6);
    DLabel *label = new DLabel("content");
    m_dPageDriverTableView->setWidget(0, 1, label);
    EXPECT_EQ(label, m_dPageDriverTableView->mp_View->indexWidget(m_dPageDriverTableView->mp_View->mp_Model->index(0, 1)));
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_getCheckedDriverIndex)
{
    // 空表:无勾选索引
    QList<int> lst;
    m_dPageDriverTableView->getCheckedDriverIndex(lst);
    EXPECT_TRUE(lst.isEmpty());

    // 带勾选框与名称项的一行:勾选后返回该名称项的 index
    m_dPageDriverTableView->appendRowItems(6);
    DriverCheckItem *cb = new DriverCheckItem(nullptr, false);
    m_dPageDriverTableView->setWidget(0, 0, cb);
    DriverNameItem *name = new DriverNameItem(nullptr);
    name->setIndex(3);
    m_dPageDriverTableView->setWidget(0, 1, name);

    lst.clear();
    m_dPageDriverTableView->getCheckedDriverIndex(lst);
    EXPECT_TRUE(lst.isEmpty());

    QSignalSpy spy(m_dPageDriverTableView, &PageDriverTableView::itemChecked);
    cb->setChecked(true);
    ASSERT_EQ(1, spy.count());
    EXPECT_EQ(3, spy.first().at(0).toInt());
    EXPECT_TRUE(spy.first().at(1).toBool());

    lst.clear();
    m_dPageDriverTableView->getCheckedDriverIndex(lst);
    ASSERT_EQ(1, lst.size());
    EXPECT_EQ(3, lst.first());
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_setItemStatus)
{
    // 实际列布局:0 勾选框,1 名称,4 状态,5 操作
    m_dPageDriverTableView->appendRowItems(6);
    DriverCheckItem *cb = new DriverCheckItem(nullptr, false);
    m_dPageDriverTableView->setWidget(0, 0, cb);
    DriverNameItem *name = new DriverNameItem(nullptr);
    name->setIndex(0);
    m_dPageDriverTableView->setWidget(0, 1, name);
    DriverStatusItem *status = new DriverStatusItem(nullptr);
    m_dPageDriverTableView->setWidget(0, 4, status);
    DriverOperationItem *opera = new DriverOperationItem(nullptr);
    m_dPageDriverTableView->setWidget(0, 5, opera);

    // 安装成功:状态刷新,勾选框取消选中且置灰
    m_dPageDriverTableView->setItemStatus(0, ST_SUCESS);
    EXPECT_EQ(ST_SUCESS, status->getStatus());
    QList<int> lst;
    m_dPageDriverTableView->getCheckedDriverIndex(lst);
    EXPECT_TRUE(lst.isEmpty());

    // 错误消息委托,调用不崩溃
    m_dPageDriverTableView->setErrorMsg(0, "error msg");
    SUCCEED();
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_setHeaderCb)
{
    // 表头勾选状态/置灰委托(需先 initHeaderView 创建表头勾选框)
    QStringList headers;
    headers << "a" << "b" << "c" << "d" << "e" << "f";
    m_dPageDriverTableView->initHeaderView(headers, true);
    m_dPageDriverTableView->setHeaderCbStatus(true);
    m_dPageDriverTableView->setHeaderCbStatus(false);
    m_dPageDriverTableView->setHeaderCbEnable(false);
    m_dPageDriverTableView->setHeaderCbEnable(true);

    // 安装过程中勾选置灰:已勾选行的勾选框与操作置灰,状态置为等待
    m_dPageDriverTableView->appendRowItems(6);
    DriverCheckItem *cb = new DriverCheckItem(nullptr, false);
    m_dPageDriverTableView->setWidget(0, 0, cb);
    DriverStatusItem *status = new DriverStatusItem(nullptr);
    m_dPageDriverTableView->setWidget(0, 4, status);
    DriverOperationItem *opera = new DriverOperationItem(nullptr);
    m_dPageDriverTableView->setWidget(0, 5, opera);
    cb->setChecked(true);

    m_dPageDriverTableView->setCheckedCBDisnable();
    EXPECT_EQ(ST_WAITING, status->getStatus());
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_setItemOperationEnable)
{
    // 还原页布局(名称在 0 列,操作在 3 列)下的置灰委托,调用不崩溃
    m_dPageDriverTableView->appendRowItems(6);
    DriverNameItem *name = new DriverNameItem(nullptr);
    name->setIndex(0);
    m_dPageDriverTableView->setWidget(0, 0, name);
    DriverOperationItem *opera = new DriverOperationItem(nullptr);
    m_dPageDriverTableView->setWidget(0, 3, opera);

    m_dPageDriverTableView->setItemOperationEnable(0, false);
    m_dPageDriverTableView->setItemOperationEnable(0, true);
    EXPECT_FALSE(m_dPageDriverTableView->hasItemDisabled());
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_hasItemDisabled)
{
    // 安装页布局:勾选框所在行整体禁用时判定为存在禁用项
    m_dPageDriverTableView->appendRowItems(6);
    DriverCheckItem *cb = new DriverCheckItem(nullptr, false);
    m_dPageDriverTableView->setWidget(0, 0, cb);
    EXPECT_FALSE(m_dPageDriverTableView->hasItemDisabled());

    cb->setEnabled(false);
    EXPECT_TRUE(m_dPageDriverTableView->hasItemDisabled());
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_removeItemAndWidget)
{
    m_dPageDriverTableView->appendRowItems(6);
    DLabel *label = new DLabel("content");
    m_dPageDriverTableView->setWidget(0, 1, label);

    m_dPageDriverTableView->removeItemAndWidget(0, 1);
    EXPECT_EQ(nullptr, m_dPageDriverTableView->mp_View->indexWidget(m_dPageDriverTableView->mp_View->mp_Model->index(0, 1)));
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_resizeEvent)
{
    // 隐藏的顶层窗体 resize() 不派发事件:先 setGeometry 同步实际宽度,再发送 QResizeEvent
    // 先建 2 列(第 5 列仍越界为 0,命中“新表”分支),否则 setColumnWidth 无处生效
    m_dPageDriverTableView->appendRowItems(2);

    // 初始宽度不足 680:仅记录宽度,不调整列
    m_dPageDriverTableView->setGeometry(0, 0, 500, 40);
    m_dPageDriverTableView->m_PreWidth = 0;
    QResizeEvent e0(QSize(500, 40), QSize(0, 40));
    QApplication::sendEvent(m_dPageDriverTableView, &e0);
    EXPECT_EQ(500, m_dPageDriverTableView->m_PreWidth);

    // 新表(columnWidth(5)==0)预置第一列 200;宽度仍不足 680 时跳过调整
    m_dPageDriverTableView->setGeometry(0, 0, 760, 40);
    m_dPageDriverTableView->mp_View->setColumnWidth(0, 200);
    QResizeEvent e1(QSize(760, 40), QSize(500, 40));
    QApplication::sendEvent(m_dPageDriverTableView, &e1);
    EXPECT_EQ(760, m_dPageDriverTableView->m_PreWidth);
    EXPECT_EQ(200, m_dPageDriverTableView->mp_View->columnWidth(0));

    // 放大:第一列加 detal(200 + 40)
    m_dPageDriverTableView->setGeometry(0, 0, 800, 40);
    QResizeEvent e2(QSize(800, 40), QSize(760, 40));
    QApplication::sendEvent(m_dPageDriverTableView, &e2);
    EXPECT_EQ(240, m_dPageDriverTableView->mp_View->columnWidth(0));

    // 缩小但仍大于 120 下限
    m_dPageDriverTableView->setGeometry(0, 0, 760, 40);
    QResizeEvent e3(QSize(760, 40), QSize(800, 40));
    QApplication::sendEvent(m_dPageDriverTableView, &e3);
    EXPECT_EQ(200, m_dPageDriverTableView->mp_View->columnWidth(0));

    // 缩小到下限以内:不再缩窄(200 - 90 = 110 ≤ 120)
    m_dPageDriverTableView->setGeometry(0, 0, 670, 40);
    QResizeEvent e4(QSize(670, 40), QSize(760, 40));
    QApplication::sendEvent(m_dPageDriverTableView, &e4);
    EXPECT_EQ(200, m_dPageDriverTableView->mp_View->columnWidth(0));
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_paintEvent)
{
    // 离屏渲染触发边框绘制,不崩溃
    m_dPageDriverTableView->resize(680, 200);
    QPixmap pix = m_dPageDriverTableView->grab();
    EXPECT_FALSE(pix.isNull());
}

TEST_F(UT_PageDriverTableView, UT_PageDriverTableView_setColumnWidth)
{
    // 列宽委托:内部表格对应列宽度变化
    m_dPageDriverTableView->appendRowItems(6);
    m_dPageDriverTableView->setColumnWidth(0, 100);
    EXPECT_EQ(100, m_dPageDriverTableView->mp_View->columnWidth(0));
}
