// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_getdrivernamewidget.cpp
 * @brief GetDriverNameWidget 单元测试（Mode 2 重写，逐类闭环）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method               | level | factors                          | min | actual |
 * |----------------------|-------|----------------------------------|-----|--------|
 * | loadAllDrivers       | low   | stack-switch,model-alloc         | 1   | 1      |
 * | selectName           | low   | row:-1,valid,out-of-range        | 1   | 3      |
 * | updateTipLabelText   | mid   | text-set,tooltip-set             | 2   | 2      |
 * | stopLoadingDrivers   | low   | stop-flag                        | 1   | 1      |
 * | reloadDriversListPages| low  | empty,non-empty                  | 1   | 2      |
 * | slotSelectedDriver   | low   | valid-select,reselect,invalid    | 1   | 2      |
 * | slotFinishLoadDrivers| low   | stack-switch,listview-model      | 1   | 1      |
 * | onUpdateTheme        | low   | palette-window-base              | 1   | 1      |
 *
 * 分支/等价类说明（源自 MCP get_code_snippet 真实源码）：
 *   - loadAllDrivers：WaitingWidget->start + 栈切到 0 + 新建 mp_model + emit startLoadDrivers；
 *   - selectName：row==-1 短路返回空；row 越界返回空；合法行返回 item(row)->data(UserRole)；
 *   - reloadDriversListPages：rowCount<=0 → frame/tip 隐藏 + 文案"No drivers found in this
 *     folder" + selectedRow 置 -1 + emit signalDriversCount；否则显示并恢复文案；
 *   - slotSelectedDriver：先清 tip、emit signalItemClicked，item(row,1) 为空提前返回；
 *     上一个选中项取消勾选；Unchecked → Checked；记录 mp_selectedRow；
 *   - slotFinishLoadDrivers：reload + ListView 挂模型 + 栈切 1 + WaitingWidget->stop；
 *   - onUpdateTheme（Qt6）：titleLabel 调色板 Window 色设为 applicationPalette 的 Base 色。
 * 打桩策略（外部依赖全部 stub，不依赖真实设备环境）：
 *   - GetDriverNameModel::startLoadDrivers（跨线程槽，会遍历文件系统/DBus）→ 空实现，
 *     避免测试触发真实磁盘遍历；其余对象走真实 offscreen 行为。
 */

#include "GetDriverNameWidget.h"
#include "GetDriverNameModel.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStackedWidget>
#include <DLabel>
#include <DFrame>
#include <DGuiApplicationHelper>

#include <QDir>
#include <QFileIconProvider>

#include <gtest/gtest.h>

// GetDriverNameModel::startLoadDrivers 空桩：阻断真实文件系统遍历与 DBus 调用
static void ut_startLoadDriversStub(GetDriverNameModel *self, QStandardItemModel *model, bool includeSub, const QString &path)
{
    Q_UNUSED(self);
    Q_UNUSED(model);
    Q_UNUSED(includeSub);
    Q_UNUSED(path);
}

// GetDriverNameModel::stopLoadingDrivers 计数桩：验证 stopLoadingDrivers 的透传行为
static int g_utStopCalls = 0;
static void ut_stopLoadingDriversStub(GetDriverNameModel *self)
{
    Q_UNUSED(self);
    ++g_utStopCalls;
}

class UT_GetDriverNameWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_GetDriverNameWidget = new GetDriverNameWidget;
    }
    void TearDown()
    {
        delete m_GetDriverNameWidget;
    }

    GetDriverNameWidget *m_GetDriverNameWidget;
};

// loadAllDrivers：等待页 + 栈切第 0 页 + 新建数据模型
TEST_F(UT_GetDriverNameWidget, loadAllDrivers_SwitchesStack_WaitingPageShown)
{
    Stub stub;
    // Arrange
    stub.set(ADDR(GetDriverNameModel, startLoadDrivers), ut_startLoadDriversStub);

    // Act
    m_GetDriverNameWidget->loadAllDrivers(true, "/tmp");

    // Assert
    EXPECT_EQ(0, m_GetDriverNameWidget->mp_StackWidget->currentIndex());   // 栈切到等待页
    EXPECT_NE(nullptr, m_GetDriverNameWidget->mp_model);                   // 数据模型已创建
    EXPECT_EQ(0, m_GetDriverNameWidget->mp_model->rowCount());
}

// selectName：未选中（row == -1）短路返回空串，不触碰模型
TEST_F(UT_GetDriverNameWidget, selectName_NoSelection_ReturnsEmpty)
{
    // Arrange
    m_GetDriverNameWidget->mp_selectedRow = -1;

    // Act
    const QString ret = m_GetDriverNameWidget->selectName();

    // Assert
    EXPECT_TRUE(ret.isEmpty());
    EXPECT_STREQ("", ret.toStdString().c_str());
}

// selectName：合法行返回 item(row, 0)？——实际取 item(mp_selectedRow)->data(UserRole)
TEST_F(UT_GetDriverNameWidget, selectName_ValidRow_ReturnsUserRolePath)
{
    // Arrange
    const QString path = QDir::tempPath() + QDir::separator() + "deepin-devicemanager_5.6.12-1_amd64.deb";
    QStandardItemModel *model = new QStandardItemModel();
    QStandardItem *iconItem = new QStandardItem;
    iconItem->setData(QVariant::fromValue(path), Qt::UserRole);
    model->setItem(0, 0, iconItem);
    model->setItem(0, 1, new QStandardItem("deepin-devicemanager_5.6.12-1_amd64.deb"));
    m_GetDriverNameWidget->mp_model = model;
    m_GetDriverNameWidget->mp_selectedRow = 0;

    // Act
    const QString ret = m_GetDriverNameWidget->selectName();

    // Assert
    EXPECT_EQ(path, ret);   // 返回 UserRole 数据
    EXPECT_FALSE(ret.isEmpty());
    delete model;
}

// selectName：选中行越界（row >= rowCount）返回空串
TEST_F(UT_GetDriverNameWidget, selectName_RowOutOfRange_ReturnsEmpty)
{
    // Arrange
    QStandardItemModel *model = new QStandardItemModel();
    model->setItem(0, 0, new QStandardItem("a.ko"));
    m_GetDriverNameWidget->mp_model = model;
    m_GetDriverNameWidget->mp_selectedRow = 5;   // 越界行

    // Act
    const QString ret = m_GetDriverNameWidget->selectName();

    // Assert
    EXPECT_TRUE(ret.isEmpty());
    EXPECT_NE(5, m_GetDriverNameWidget->mp_model->rowCount());   // 模型行数与选中行无关
    delete model;
}

// updateTipLabelText：非空文案同时写入 text 与 toolTip
TEST_F(UT_GetDriverNameWidget, updateTipLabelText_NonEmptyText_SyncsTextAndTooltip)
{
    // Arrange
    const QString text = "/home";

    // Act
    m_GetDriverNameWidget->updateTipLabelText(text);

    // Assert
    EXPECT_STREQ("/home", m_GetDriverNameWidget->mp_tipLabel->text().toStdString().c_str());
    EXPECT_STREQ("/home", m_GetDriverNameWidget->mp_tipLabel->toolTip().toStdString().c_str());
}

// updateTipLabelText：空文案清空显示
TEST_F(UT_GetDriverNameWidget, updateTipLabelText_EmptyText_ClearsDisplay)
{
    // Arrange
    m_GetDriverNameWidget->updateTipLabelText("previous");

    // Act
    m_GetDriverNameWidget->updateTipLabelText("");

    // Assert
    EXPECT_STREQ("", m_GetDriverNameWidget->mp_tipLabel->text().toStdString().c_str());
    EXPECT_STREQ("", m_GetDriverNameWidget->mp_tipLabel->toolTip().toStdString().c_str());
}

// stopLoadingDrivers：透传到模型的 stopLoadingDrivers（每次调用都委托一次）
TEST_F(UT_GetDriverNameWidget, stopLoadingDrivers_EachCall_DelegatesToModel)
{
    Stub stub;
    // Arrange
    g_utStopCalls = 0;
    stub.set(ADDR(GetDriverNameModel, stopLoadingDrivers), ut_stopLoadingDriversStub);

    // Act
    m_GetDriverNameWidget->stopLoadingDrivers();

    // Assert
    EXPECT_EQ(1, g_utStopCalls);   // 首次调用透传一次

    // Act 2 / Assert 2: 重复调用安全且继续透传
    m_GetDriverNameWidget->stopLoadingDrivers();
    EXPECT_EQ(2, g_utStopCalls);
}

// reloadDriversListPages：无驱动 → frame/tip 隐藏 + 文案提示 + 选中行复位 -1
TEST_F(UT_GetDriverNameWidget, reloadDriversListPages_NoDrivers_HidesFrameAndSetsTip)
{
    // Arrange
    QStandardItemModel *model = new QStandardItemModel();   // 0 行
    m_GetDriverNameWidget->mp_model = model;
    m_GetDriverNameWidget->mp_selectedRow = 0;

    // Act
    m_GetDriverNameWidget->reloadDriversListPages();

    // Assert
    DFrame *frame = m_GetDriverNameWidget->findChild<DFrame *>();
    DLabel *label = m_GetDriverNameWidget->findChild<DLabel *>();
    ASSERT_NE(nullptr, frame);
    ASSERT_NE(nullptr, label);
    EXPECT_TRUE(frame->isHidden());
    EXPECT_TRUE(m_GetDriverNameWidget->mp_tipLabel->isHidden());
    EXPECT_EQ(-1, m_GetDriverNameWidget->mp_selectedRow);
    EXPECT_STREQ("No drivers found in this folder", label->text().toStdString().c_str());
    delete model;
}

// reloadDriversListPages：有驱动 → frame/tip 显示 + 文案恢复选择提示
TEST_F(UT_GetDriverNameWidget, reloadDriversListPages_HasDrivers_ShowsFrame)
{
    // Arrange
    QStandardItemModel *model = new QStandardItemModel();
    model->setItem(0, 0, new QStandardItem("a.ko"));
    m_GetDriverNameWidget->mp_model = model;
    m_GetDriverNameWidget->mp_selectedRow = -1;

    // Act
    m_GetDriverNameWidget->reloadDriversListPages();

    // Assert
    DFrame *frame = m_GetDriverNameWidget->findChild<DFrame *>();
    DLabel *label = m_GetDriverNameWidget->findChild<DLabel *>();
    ASSERT_NE(nullptr, frame);
    ASSERT_NE(nullptr, label);
    EXPECT_FALSE(frame->isHidden());
    EXPECT_FALSE(m_GetDriverNameWidget->mp_tipLabel->isHidden());
    EXPECT_STREQ("Select a driver for update", label->text().toStdString().c_str());
    delete model;
}

// slotSelectedDriver：合法索引 → 勾选该行 + 记录选中行；再次选择 → 旧行取消勾选
TEST_F(UT_GetDriverNameWidget, slotSelectedDriver_ValidIndex_ChecksAndStoresRow)
{
    // Arrange
    QStandardItemModel *model = new QStandardItemModel();
    for (int r = 0; r < 2; ++r) {
        QStandardItem *iconItem = new QStandardItem;
        QStandardItem *textItem = new QStandardItem(QString("driver%1.ko").arg(r));
        model->setItem(r, 0, iconItem);
        model->setItem(r, 1, textItem);
    }
    m_GetDriverNameWidget->mp_model = model;

    // Act
    m_GetDriverNameWidget->slotSelectedDriver(model->index(0, 0));

    // Assert
    EXPECT_EQ(Qt::Checked, model->item(0, 1)->checkState());
    EXPECT_EQ(0, m_GetDriverNameWidget->mp_selectedRow);

    // Act 2: 切换选中第 1 行
    m_GetDriverNameWidget->slotSelectedDriver(model->index(1, 0));

    // Assert 2: 旧行取消勾选，新行勾选
    EXPECT_EQ(Qt::Unchecked, model->item(0, 1)->checkState());
    EXPECT_EQ(Qt::Checked, model->item(1, 1)->checkState());
    EXPECT_EQ(1, m_GetDriverNameWidget->mp_selectedRow);
    delete model;
}

// slotSelectedDriver：无效索引（item(row,1) 为空）→ 提前返回，选中行不变
TEST_F(UT_GetDriverNameWidget, slotSelectedDriver_InvalidIndex_KeepsRowUnchanged)
{
    // Arrange
    QStandardItemModel *model = new QStandardItemModel();
    model->setItem(0, 0, new QStandardItem("only-icon-column"));
    m_GetDriverNameWidget->mp_model = model;
    m_GetDriverNameWidget->mp_selectedRow = 0;
    const QModelIndex invalidIndex;   // 无效索引 row = -1

    // Act
    m_GetDriverNameWidget->slotSelectedDriver(invalidIndex);

    // Assert
    EXPECT_EQ(0, m_GetDriverNameWidget->mp_selectedRow);   // 未被修改
    EXPECT_TRUE(m_GetDriverNameWidget->mp_tipLabel->text().isEmpty());   // tip 已清空
    delete model;
}

// slotFinishLoadDrivers：列表挂模型 + 栈切第 1 页 + 默认选中首行
TEST_F(UT_GetDriverNameWidget, slotFinishLoadDrivers_LoadFinished_ShowsListSelectsFirst)
{
    // Arrange
    QStandardItemModel *model = new QStandardItemModel();
    model->setItem(0, 0, new QStandardItem("first.ko"));
    model->setItem(1, 0, new QStandardItem("second.ko"));
    m_GetDriverNameWidget->mp_model = model;

    // Act
    m_GetDriverNameWidget->slotFinishLoadDrivers();

    // Assert
    EXPECT_EQ(1, m_GetDriverNameWidget->mp_StackWidget->currentIndex());
    EXPECT_EQ(model, m_GetDriverNameWidget->mp_ListView->model());
    EXPECT_TRUE(m_GetDriverNameWidget->mp_ListView->currentIndex().isValid());
    EXPECT_EQ(0, m_GetDriverNameWidget->mp_ListView->currentIndex().row());
    delete model;
}

// onUpdateTheme（Qt6）：titleLabel 调色板 Window 色同步为 applicationPalette 的 Base 色
TEST_F(UT_GetDriverNameWidget, onUpdateTheme_ThemeChanged_WindowColorFollowsAppBase)
{
    // Arrange
    const QColor baseColor = DGuiApplicationHelper::instance()->applicationPalette().color(QPalette::Base);
    QPalette original = m_GetDriverNameWidget->mp_titleLabel->palette();
    original.setColor(QPalette::Window, QColor(Qt::red));   // 先设置一个可区分的旧色
    m_GetDriverNameWidget->mp_titleLabel->setPalette(original);

    // Act
    m_GetDriverNameWidget->onUpdateTheme();

    // Assert
    EXPECT_EQ(baseColor, m_GetDriverNameWidget->mp_titleLabel->palette().color(QPalette::Window));
    EXPECT_NE(QColor(Qt::red), m_GetDriverNameWidget->mp_titleLabel->palette().color(QPalette::Window));
}
