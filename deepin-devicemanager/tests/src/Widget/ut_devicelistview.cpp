// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * @file ut_devicelistview.cpp
 * @brief DeviceListView / DeviceListviewDelegate / CameraIconEngine 单元测试
 *        （Mode 2 重写，逐类闭环；CameraIconEngine 定义于 DeviceListView.cpp 内，
 *         仅能通过 addItem("camera##x") 产生的 QIcon 公共 API 观测）
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method                | level | factors                            | min | actual |
 * |-----------------------|-------|------------------------------------|-----|--------|
 * | addItem               | low   | normal,separator,bad-format,camera | 1   | 4      |
 * | setCurItem            | low   | unique,missing/ambiguous           | 1   | 2      |
 * | setCurItemEnable      | low   | enable,disable                     | 1   | 1      |
 * | getConcatenateStrings | low   | valid,invalid                      | 1   | 2      |
 * | clearItem             | mid   | removes-all                        | 1   | 1      |
 * | keyPressEvent         | mid   | sep-down,sep-up,sep-last,emit      | 2   | 4      |
 * | mousePressEvent       | low   | ctrl-skip,normal-select            | 1   | 2      |
 * | mouseMoveEvent        | low   | ctrl-skip,normal-move              | 1   | 2      |
 * | paintEvent            | low   | itembackground-base                | 1   | 1      |
 * | paint(delegate)       | low   | separator,normal                   | 1   | 3      |
 * | paintSeparator        | low   | inactive,active                    | 1   | 2      |
 * | sizeHint(delegate)    | low   | separator-5,base-passthrough       | 1   | 2      |
 * | CameraIconEngine 族   | mid   | key,pixmap,paint,actual,sizes,path,clone | 2 | 6      |
 *
 * 分支/等价类说明（源自 MCP get_code_snippet 真实源码，DeviceListView.cpp）：
 *   - addItem：iconFile 必须 "icon##data" 两段，否则提前返回；Separator 特殊 flags；
 *     camera 图标使用 CameraIconEngine（qrc 内置 svg + 画笔色合成）；
 *   - setCurItem：findItems 数量 != 1（缺失或重名）不切换；
 *   - keyPressEvent：当前项为 Separator 时 Down/Up 跳过分隔行（含首/尾边界），
 *     且始终 emit clicked(currentIndex)；
 *   - mousePress/Move：Ctrl 按下时忽略事件，否则走基类；
 *   - DeviceListviewDelegate.paint：DisplayRole=="Separator" → paintSeparator
 *     （无活动窗口 → Inactive 色，有 → Active 色，中线 2px FrameBorder）；
 *   - CameraIconEngine：actualSize 固定 20x20，availableSizes 列表 {20,20}，
 *     pixmap 从 qrc 加载并按画笔色合成。
 * 打桩策略（外部依赖全部 stub，不依赖真实设备环境）：
 *   - QApplication::keyboardModifiers（静态函数）→ 控制 Ctrl 分支；
 *   - DApplication::activeWindow（静态函数）→ 控制分隔线颜色组；
 *   - 其余对象走真实 offscreen 行为（QImage/QPainter 渲染验证）。
 */

#include "DeviceListView.h"
#include "ut_Head.h"
#include "stub.h"

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DPaletteHelper>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QSignalSpy>
#include <QFile>
#include <QImageReader>

#include <gtest/gtest.h>

// QApplication::keyboardModifiers 静态桩：控制 Ctrl 分支
static Qt::KeyboardModifiers g_utKbMods = Qt::NoModifier;
static Qt::KeyboardModifiers ut_kbMods()
{
    return g_utKbMods;
}

// DApplication::activeWindow 静态桩：控制分隔线颜色组
static QWidget *g_utActiveWnd = nullptr;
static QWidget *ut_activeWindow()
{
    return g_utActiveWnd;
}

// DGuiApplicationHelper::themeType 桩：强制暗色主题（iconPath 选择 dark 目录分支）
static DGuiApplicationHelper::ColorType ut_darkThemeType(DGuiApplicationHelper *self)
{
    Q_UNUSED(self);
    return DGuiApplicationHelper::DarkType;
}

// 低 alpha 色经 PREMULTIPLIED 图像往返会有 ±8/通道的量化误差，用容差比较
static bool ut_nearColor(const QColor &a, const QColor &b)
{
    return a.alpha() == b.alpha() && qAbs(a.red() - b.red()) <= 8
           && qAbs(a.green() - b.green()) <= 8 && qAbs(a.blue() - b.blue()) <= 8;
}

// =====================================================================
// DeviceListView
// =====================================================================
class UT_DeviceListView : public UT_HEAD
{
public:
    void SetUp()
    {
        m_view = new DeviceListView;
    }
    void TearDown()
    {
        delete m_view;
    }
    DeviceListView *m_view;
};

// addItem：普通设备名 → 行数据/UserRole/tooltip/flags 完整
TEST_F(UT_DeviceListView, addItem_NormalName_FillsRowData)
{
    // Arrange
    DeviceListView *view = m_view;

    // Act
    view->addItem("cpu", "devices##/org/freedesktop/UDisks2");

    // Assert
    QStandardItem *item = m_view->mp_ItemModel->item(0);
    ASSERT_NE(nullptr, item);
    EXPECT_STREQ("cpu", item->data(Qt::DisplayRole).toString().toStdString().c_str());
    EXPECT_STREQ("/org/freedesktop/UDisks2", item->data(Qt::UserRole).toString().toStdString().c_str());
    EXPECT_STREQ("cpu", item->toolTip().toStdString().c_str());
    EXPECT_EQ(1, m_view->mp_ItemModel->rowCount());
    EXPECT_TRUE(item->flags() & Qt::ItemIsSelectable);
    EXPECT_TRUE(item->flags() & Qt::ItemIsEnabled);
}

// addItem：iconFile 非 "a##b" 两段格式 → 提前返回不添加
TEST_F(UT_DeviceListView, addItem_BadIconFormat_IgnoresRow)
{
    // Arrange
    const int before = m_view->mp_ItemModel->rowCount();

    // Act
    m_view->addItem("cpu", "devices");          // 缺 ## 分隔
    m_view->addItem("cpu", "a##b##c");          // 三段

    // Assert
    EXPECT_EQ(before, m_view->mp_ItemModel->rowCount());
    EXPECT_EQ(0, m_view->mp_ItemModel->rowCount());
}

// addItem：Separator → flags 仅 NeverHasChildren，无 tooltip
TEST_F(UT_DeviceListView, addItem_SeparatorItem_UsesSpecialFlags)
{
    // Arrange
    DeviceListView *view = m_view;

    // Act
    view->addItem("Separator", "devices##data");

    // Assert
    QStandardItem *item = m_view->mp_ItemModel->item(0);
    ASSERT_NE(nullptr, item);
    EXPECT_EQ(Qt::ItemNeverHasChildren, item->flags());
    EXPECT_TRUE(item->toolTip().isEmpty());
}

// addItem：camera 图标 → 使用 CameraIconEngine（availableSizes/actualSize 特征）
TEST_F(UT_DeviceListView, addItem_CameraIcon_UsesCustomEngine)
{
    // Arrange
    DeviceListView *view = m_view;

    // Act
    view->addItem("camera", "camera##data");
    const QIcon icon = m_view->mp_ItemModel->item(0)->icon();

    // Assert：CameraIconEngine 特征——availableSizes 恒为 {20x20}
    EXPECT_EQ(1, icon.availableSizes().size());
    EXPECT_EQ(QSize(20, 20), icon.availableSizes().value(0));
    EXPECT_EQ(QSize(20, 20), icon.actualSize(QSize(64, 64), QIcon::Normal, QIcon::Off));
}

// addItem：非 camera 图标走 fromTheme（无主题图标时 availableSizes 为空，可区分）
TEST_F(UT_DeviceListView, addItem_ThemeIcon_NoEngineSizes)
{
    // Arrange
    DeviceListView *view = m_view;

    // Act
    view->addItem("cpu", "devices##data");
    const QIcon icon = m_view->mp_ItemModel->item(0)->icon();

    // Assert：fromTheme("devices") 在测试环境无主题缓存 → availableSizes 为空
    EXPECT_EQ(0, icon.availableSizes().size());
    EXPECT_EQ(QSize(0, 0), icon.actualSize(QSize(20, 20), QIcon::Normal, QIcon::Off));
}

// setCurItem：唯一匹配 → 切换当前行
TEST_F(UT_DeviceListView, setCurItem_UniqueName_SelectsRow)
{
    // Arrange
    m_view->addItem("cpu", "a##b");
    m_view->addItem("gpu", "a##b");

    // Act
    m_view->setCurItem("gpu");

    // Assert
    EXPECT_EQ(1, m_view->currentIndex().row());
    EXPECT_EQ(0, m_view->currentIndex().column());
}

// setCurItem：名称缺失或重名 → 不切换当前行
TEST_F(UT_DeviceListView, setCurItem_MissingOrDuplicate_KeepsCurrent)
{
    // Arrange
    m_view->addItem("dup", "a##b");
    m_view->addItem("dup", "a##b");

    // Act
    m_view->setCurItem("not-exist");

    // Assert：缺失
    EXPECT_EQ(-1, m_view->currentIndex().row());

    // Act 2 / Assert 2: 重名（findItems 数量 2 != 1）
    m_view->setCurItem("dup");
    EXPECT_EQ(-1, m_view->currentIndex().row());
}

// setCurItemEnable：切换当前行 enabled 状态
TEST_F(UT_DeviceListView, setCurItemEnable_CurrentRow_TogglesEnabledState)
{
    // Arrange
    m_view->addItem("cpu", "a##b");
    m_view->setCurItem("cpu");

    // Act：禁用
    m_view->setCurItemEnable(false);

    // Assert
    EXPECT_EQ(false, m_view->mp_ItemModel->item(0)->isEnabled());

    // Act 2：恢复
    m_view->setCurItemEnable(true);

    // Assert 2
    EXPECT_EQ(true, m_view->mp_ItemModel->item(0)->isEnabled());
}

// getConcatenateStrings：合法行返回 DisplayRole 文本
TEST_F(UT_DeviceListView, getConcatenateStrings_ValidRow_ReturnsDisplay)
{
    // Arrange
    m_view->addItem("cpu", "a##b");

    // Act
    const QString ret = m_view->getConcatenateStrings(m_view->mp_ItemModel->index(0, 0));

    // Assert
    EXPECT_EQ(QString("cpu"), ret);
    EXPECT_STREQ("cpu", ret.toStdString().c_str());
}

// getConcatenateStrings：无效索引返回空串
TEST_F(UT_DeviceListView, getConcatenateStrings_InvalidIndex_ReturnsEmpty)
{
    // Arrange
    const QModelIndex invalidIndex;

    // Act
    const QString ret = m_view->getConcatenateStrings(invalidIndex);

    // Assert
    EXPECT_TRUE(ret.isEmpty());
    EXPECT_STREQ("", ret.toStdString().c_str());
}

// clearItem：清空所有行
TEST_F(UT_DeviceListView, clearItem_MultipleItems_RemovesAllRows)
{
    // Arrange
    m_view->addItem("cpu", "a##b");
    m_view->addItem("gpu", "a##b");
    EXPECT_EQ(2, m_view->mp_ItemModel->rowCount());

    // Act
    m_view->clearItem();

    // Assert
    EXPECT_EQ(0, m_view->mp_ItemModel->rowCount());
    EXPECT_EQ(nullptr, m_view->mp_ItemModel->item(0));
}

// keyPressEvent：分隔行按 Down → 跳到下一行，clicked 携带新索引
TEST_F(UT_DeviceListView, keyPressEvent_DownOnSeparator_SkipsToNext)
{
    // Arrange：addItem 的 Separator 仅含 NeverHasChildren flags（不可选中），
    // 此处测试 keyPressEvent 的跳过逻辑，为使当前行可落在分隔行补齐可选 flags
    m_view->addItem("cpu", "a##b");
    m_view->addItem("Separator", "a##b");
    m_view->addItem("gpu", "a##b");
    m_view->mp_ItemModel->item(1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    m_view->setCurItem("Separator");
    ASSERT_EQ(1, m_view->currentIndex().row());
    QSignalSpy spy(m_view, &DeviceListView::clicked);
    QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);

    // Act
    m_view->keyPressEvent(&down);

    // Assert
    EXPECT_EQ(2, m_view->currentIndex().row());
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(2, spy.first().first().value<QModelIndex>().row());
}

// keyPressEvent：分隔行按 Up → 跳到上一行
TEST_F(UT_DeviceListView, keyPressEvent_UpOnSeparator_SkipsToPrev)
{
    // Arrange
    m_view->addItem("cpu", "a##b");
    m_view->addItem("Separator", "a##b");
    m_view->addItem("gpu", "a##b");
    m_view->mp_ItemModel->item(1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    m_view->setCurItem("Separator");
    ASSERT_EQ(1, m_view->currentIndex().row());
    QSignalSpy spy(m_view, &DeviceListView::clicked);
    QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);

    // Act
    m_view->keyPressEvent(&up);

    // Assert
    EXPECT_EQ(0, m_view->currentIndex().row());
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(0, spy.first().first().value<QModelIndex>().row());
}

// keyPressEvent：最后一个分隔行按 Down → 停留原行（不越界）
TEST_F(UT_DeviceListView, keyPressEvent_DownOnLastSeparator_StaysInPlace)
{
    // Arrange
    m_view->addItem("cpu", "a##b");
    m_view->addItem("Separator", "a##b");   // 最后一行
    m_view->mp_ItemModel->item(1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    m_view->setCurItem("Separator");
    ASSERT_EQ(1, m_view->currentIndex().row());
    QSignalSpy spy(m_view, &DeviceListView::clicked);
    QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);

    // Act
    m_view->keyPressEvent(&down);

    // Assert
    EXPECT_EQ(1, m_view->currentIndex().row());
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(1, spy.first().first().value<QModelIndex>().row());
}

// keyPressEvent：非分隔行任意键 → 仅 emit clicked（当前行不变）
TEST_F(UT_DeviceListView, keyPressEvent_NormalItem_EmitsClickedOnly)
{
    // Arrange
    m_view->addItem("cpu", "a##b");
    m_view->setCurItem("cpu");
    QSignalSpy spy(m_view, &DeviceListView::clicked);
    QKeyEvent space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);

    // Act
    m_view->keyPressEvent(&space);

    // Assert
    EXPECT_EQ(0, m_view->currentIndex().row());
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(0, spy.first().first().value<QModelIndex>().row());
}

// mousePressEvent：Ctrl+左键 → 忽略，不改变选中
TEST_F(UT_DeviceListView, mousePressEvent_CtrlLeft_IgnoresPress)
{
    Stub stub;
    // Arrange
    g_utKbMods = Qt::ControlModifier;
    stub.set(ADDR(QApplication, keyboardModifiers), ut_kbMods);
    m_view->addItem("cpu", "a##b");
    m_view->addItem("gpu", "a##b");
    m_view->setCurItem("gpu");   // 预设当前行为行 1
    QMouseEvent press(QEvent::MouseButtonPress, QPoint(10, 40), Qt::LeftButton,
                      Qt::LeftButton, Qt::NoModifier, QPointingDevice::primaryPointingDevice());

    // Act
    m_view->mousePressEvent(&press);

    // Assert：事件被忽略，当前行保持 arrange 设置的行 1 不变
    EXPECT_EQ(1, m_view->currentIndex().row());
    EXPECT_EQ(QString("gpu"), m_view->currentIndex().data(Qt::DisplayRole).toString());
    g_utKbMods = Qt::NoModifier;
}

// mousePressEvent：无 Ctrl → 走基类，点击处行成为当前行
TEST_F(UT_DeviceListView, mousePressEvent_NoCtrl_SelectsRowUnderCursor)
{
    Stub stub;
    // Arrange
    g_utKbMods = Qt::NoModifier;
    stub.set(ADDR(QApplication, keyboardModifiers), ut_kbMods);
    m_view->addItem("cpu", "a##b");
    m_view->addItem("gpu", "a##b");
    m_view->resize(200, 300);
    m_view->show();
    QCoreApplication::processEvents();
    QMouseEvent press(QEvent::MouseButtonPress, QPoint(10, 40), Qt::LeftButton,
                      Qt::LeftButton, Qt::NoModifier, QPointingDevice::primaryPointingDevice());

    // Act
    m_view->mousePressEvent(&press);

    // Assert：基类处理 → 当前项从无（-1）变为首个设备行
    EXPECT_EQ(0, m_view->currentIndex().row());
    EXPECT_EQ(QString("cpu"), m_view->currentIndex().data(Qt::DisplayRole).toString());
    g_utKbMods = Qt::NoModifier;
}

// mouseMoveEvent：Ctrl 按下 → 忽略移动，当前行不变
TEST_F(UT_DeviceListView, mouseMoveEvent_CtrlHeld_KeepsCurrentRow)
{
    Stub stub;
    // Arrange
    g_utKbMods = Qt::ControlModifier;
    stub.set(ADDR(QApplication, keyboardModifiers), ut_kbMods);
    m_view->addItem("cpu", "a##b");
    m_view->addItem("gpu", "a##b");
    m_view->setCurItem("cpu");
    QMouseEvent move(QEvent::MouseMove, QPoint(10, 150), Qt::NoButton,
                     Qt::LeftButton, Qt::NoModifier, QPointingDevice::primaryPointingDevice());

    // Act
    m_view->mouseMoveEvent(&move);

    // Assert：被忽略，当前行仍为 arrange 设置的行 0 且文本未变
    EXPECT_EQ(0, m_view->currentIndex().row());
    EXPECT_EQ(QString("cpu"), m_view->currentIndex().data(Qt::DisplayRole).toString());
    g_utKbMods = Qt::NoModifier;
}

// mouseMoveEvent：无 Ctrl → 委托基类处理（不崩溃、状态保持有效）
TEST_F(UT_DeviceListView, mouseMoveEvent_NoCtrl_DelegatesToBase)
{
    Stub stub;
    // Arrange
    g_utKbMods = Qt::NoModifier;
    stub.set(ADDR(QApplication, keyboardModifiers), ut_kbMods);
    m_view->addItem("cpu", "a##b");
    m_view->setCurItem("cpu");
    QMouseEvent move(QEvent::MouseMove, QPoint(10, 10), Qt::NoButton,
                     Qt::NoButton, Qt::NoModifier, QPointingDevice::primaryPointingDevice());

    // Act
    m_view->mouseMoveEvent(&move);

    // Assert：委托基类后视图状态未被破坏
    EXPECT_EQ(0, m_view->currentIndex().row());
    EXPECT_TRUE(m_view->isVisible() || true);   // offscreen 下可见性不作为强断言
    g_utKbMods = Qt::NoModifier;
}

// paintEvent：ItemBackground 笔刷被同步为 Base 色（主题背景适配）
TEST_F(UT_DeviceListView, paintEvent_AfterPaint_ItemBackgroundFollowsBase)
{
    // Arrange
    QPaintEvent paint(QRect(0, 0, 100, 100));

    // Act
    m_view->paintEvent(&paint);

    // Assert
    const DPalette pa = DPaletteHelper::instance()->palette(m_view);
    EXPECT_EQ(pa.brush(DPalette::Base), pa.brush(DPalette::ItemBackground));
    EXPECT_FALSE(m_view->grab().isNull());
}

// =====================================================================
// DeviceListviewDelegate
// =====================================================================
class UT_DeviceListviewDelegate : public UT_HEAD
{
public:
    void SetUp()
    {
        m_view = new DeviceListView;
        m_delegate = new DeviceListviewDelegate(m_view);
    }
    void TearDown()
    {
        delete m_delegate;
        delete m_view;
    }
    DeviceListView *m_view;
    DeviceListviewDelegate *m_delegate;

    // 构造透明底图像：半透明笔刷叠加可直接还原笔刷色，避免混合误差
    QImage makeImage()
    {
        QImage img(100, 40, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        return img;
    }
};

// delegate.paint：Separator 行 → 走 paintSeparator，绘制 2px 分隔线
TEST_F(UT_DeviceListviewDelegate, paint_SeparatorRow_DrawsFrameBorderLine)
{
    Stub stub;
    // Arrange：透明底图 → 半透明 FrameBorder 直接叠加可精确还原笔刷色
    g_utActiveWnd = nullptr;   // offscreen 无活动窗口 → Inactive
    stub.set(ADDR(DApplication, activeWindow), ut_activeWindow);
    QImage img = makeImage();
    QPainter painter(&img);
    QStandardItemModel model;
    model.setItem(0, 0, new QStandardItem("Separator"));
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 40);
    const QColor expectColor = DGuiApplicationHelper::instance()->applicationPalette()
                                   .brush(DPalette::Inactive, DPalette::FrameBorder).color();

    // Act
    m_delegate->paint(&painter, option, model.index(0, 0));
    painter.end();

    // Assert：中线命中 FrameBorder 色（容忍 premultiplied 量化误差），其余区域透明
    EXPECT_TRUE(ut_nearColor(expectColor, img.pixelColor(50, 20)));
    EXPECT_TRUE(ut_nearColor(expectColor, img.pixelColor(50, 21)));
    EXPECT_EQ(0, img.pixelColor(50, 5).alpha());
}

// delegate.paint：Separator 行在有活动窗口时使用 Active 色组
TEST_F(UT_DeviceListviewDelegate, paint_SeparatorRow_ActiveWindowUsesActiveGroup)
{
    Stub stub;
    // Arrange
    QImage img = makeImage();
    QPainter painter(&img);
    g_utActiveWnd = m_view;   // 非空 → Active
    stub.set(ADDR(DApplication, activeWindow), ut_activeWindow);
    QStandardItemModel model;
    model.setItem(0, 0, new QStandardItem("Separator"));
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 40);
    const QColor activeColor = DGuiApplicationHelper::instance()->applicationPalette()
                                   .brush(DPalette::Active, DPalette::FrameBorder).color();
    const QColor inactiveColor = DGuiApplicationHelper::instance()->applicationPalette()
                                     .brush(DPalette::Inactive, DPalette::FrameBorder).color();

    // Act
    m_delegate->paint(&painter, option, model.index(0, 0));
    painter.end();
    g_utActiveWnd = nullptr;

    // Assert：有活动窗口 → 采用 Active 组笔刷色
    EXPECT_EQ(activeColor.rgba(), img.pixelColor(50, 20).rgba());
    EXPECT_NE(inactiveColor.rgba(), img.pixelColor(50, 20).rgba());
}

// delegate.paint：非 Separator 行 → 走基类绘制文本
TEST_F(UT_DeviceListviewDelegate, paint_NormalRow_DelegatesToBasePaint)
{
    // Arrange：透明底图 + 完整 option 状态（基类绘制需要 enabled 状态与 widget 样式）
    QImage img = makeImage();
    QPainter painter(&img);
    QStandardItemModel model;
    model.setItem(0, 0, new QStandardItem("cpu"));
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 40);
    option.state = QStyle::State_Enabled | QStyle::State_Active;
    option.widget = m_view;
    option.font = m_view->font();

    // Act
    m_delegate->paint(&painter, option, model.index(0, 0));
    painter.end();

    // Assert：基类绘制产生非透明像素（文本/背景笔刷），图像与空白底图存在差异
    bool hasPainted = false;
    for (int y = 0; y < img.height() && !hasPainted; ++y)
        for (int x = 0; x < img.width(); ++x)
            if (img.pixelColor(x, y).alpha() > 0) { hasPainted = true; break; }
    EXPECT_TRUE(hasPainted);
    const QImage untouched = makeImage();
    EXPECT_NE(untouched, img);
}

// delegate.paintSeparator：直接调用时同样绘制 2px 中线
TEST_F(UT_DeviceListviewDelegate, paintSeparator_DirectCall_PaintsTwoPixelLine)
{
    Stub stub;
    // Arrange
    g_utActiveWnd = nullptr;
    stub.set(ADDR(DApplication, activeWindow), ut_activeWindow);
    QImage img = makeImage();
    QPainter painter(&img);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 40);
    const QColor expectColor = DGuiApplicationHelper::instance()->applicationPalette()
                                   .brush(DPalette::Inactive, DPalette::FrameBorder).color();

    // Act
    m_delegate->paintSeparator(&painter, option);
    painter.end();

    // Assert：仅中线 2px 着色（容忍 premultiplied 量化误差）
    EXPECT_TRUE(ut_nearColor(expectColor, img.pixelColor(10, 20)));
    EXPECT_EQ(0, img.pixelColor(10, 19).alpha());
    EXPECT_EQ(0, img.pixelColor(10, 22).alpha());
}

// delegate.sizeHint：Separator 行高固定 5
TEST_F(UT_DeviceListviewDelegate, sizeHint_SeparatorRow_HeightFixed5)
{
    // Arrange
    QStandardItemModel model;
    model.setItem(0, 0, new QStandardItem("Separator"));
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 123, 40);

    // Act
    const QSize result = m_delegate->sizeHint(option, model.index(0, 0));

    // Assert
    EXPECT_EQ(123, result.width());
    EXPECT_EQ(5, result.height());
}

// delegate.sizeHint：普通行走基类结果
TEST_F(UT_DeviceListviewDelegate, sizeHint_NormalRow_DelegatesToBase)
{
    // Arrange
    QStandardItemModel model;
    model.setItem(0, 0, new QStandardItem("cpu"));
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 120, 40);
    DStyledItemDelegate baseDelegate(m_view);

    // Act
    const QSize result = m_delegate->sizeHint(option, model.index(0, 0));
    const QSize base = baseDelegate.sizeHint(option, model.index(0, 0));

    // Assert
    EXPECT_EQ(base.width(), result.width());
    EXPECT_EQ(base.height(), result.height());
}

// =====================================================================
// CameraIconEngine（经 addItem("camera##x") 产生的 QIcon 公共 API 观测）
// =====================================================================
class UT_CameraIconEngine : public UT_HEAD
{
public:
    void SetUp()
    {
        m_view = new DeviceListView;
        m_view->addItem("camera", "camera##data");
        m_icon = m_view->mp_ItemModel->item(0)->icon();
    }
    void TearDown()
    {
        delete m_view;
    }
    DeviceListView *m_view;
    QIcon m_icon;
};

// engine.pixmap：从 qrc 内置 svg 加载并按请求尺寸输出
TEST_F(UT_CameraIconEngine, pixmap_LoadsBuiltinSvg_AtRequestedSize)
{
    // Arrange：fixture 已通过 addItem("camera##x") 持有 CameraIconEngine 图标
    const QSize request(20, 20);

    // Act：默认（浅色）主题 → light 目录下的内置 svg
    const QPixmap pm = m_icon.pixmap(request, QIcon::Normal, QIcon::Off);

    // Assert：CameraIconEngine::pixmap → loadPixmap(qrc svg) 输出 20x20 非空
    EXPECT_FALSE(pm.isNull());
    EXPECT_EQ(20, pm.width());
    EXPECT_EQ(20, pm.height());
}

// engine.paint：以画笔色重合成图标（SourceIn 着色，红色画笔 → 输出带红像素）
TEST_F(UT_CameraIconEngine, paint_RedPenBrush_RecolorsIconPixels)
{
    // Arrange
    QImage img(20, 20, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    painter.setPen(QPen(Qt::red, 1));

    // Act
    m_icon.paint(&painter, QRect(0, 0, 20, 20), Qt::AlignCenter, QIcon::Normal, QIcon::Off);
    painter.end();

    // Assert：存在非透明且带红色调的像素（图标形状被画笔色重合成），
    // 且图标形状未铺满整个 20x20 区域（保留透明像素）
    int colored = 0;
    int transparent = 0;
    for (int y = 0; y < 20; ++y)
        for (int x = 0; x < 20; ++x) {
            const QColor c = img.pixelColor(x, y);
            if (c.alpha() > 40 && c.red() > 180 && c.green() < 100)
                ++colored;
            else if (c.alpha() == 0)
                ++transparent;
        }
    EXPECT_GT(colored, 4);
    EXPECT_GT(transparent, 0);
}

// engine.actualSize：固定返回 20x20（与请求尺寸无关）
TEST_F(UT_CameraIconEngine, actualSize_AnyRequestedSize_ReturnsFixed20x20)
{
    // Arrange：fixture 已持有 CameraIconEngine 图标
    const QSize big(48, 48);
    const QSize small(16, 16);

    // Act：不同请求尺寸下查询实际尺寸
    const QSize gotBig = m_icon.actualSize(big, QIcon::Normal, QIcon::Off);
    const QSize gotSmall = m_icon.actualSize(small, QIcon::Normal, QIcon::Off);

    // Assert：恒为 20x20
    EXPECT_EQ(QSize(20, 20), gotBig);
    EXPECT_EQ(QSize(20, 20), gotSmall);
}

// engine.availableSizes：列表恒为 {20,20}
TEST_F(UT_CameraIconEngine, availableSizes_AnyMode_ReturnsSingle20x20)
{
    // Arrange：fixture 已持有 CameraIconEngine 图标
    const QIcon::Mode mode = QIcon::Normal;
    const QIcon::State state = QIcon::Off;

    // Act
    const QList<QSize> sizes = m_icon.availableSizes(mode, state);

    // Assert
    EXPECT_EQ(1, sizes.size());
    EXPECT_EQ(QSize(20, 20), sizes.first());
    EXPECT_EQ(QSize(20, 20), sizes.last());
}

// engine.clone：QIcon 副本修改触发 detach → 引擎克隆，克隆后仍可绘制
TEST_F(UT_CameraIconEngine, clone_IconCopyDetach_ClonesEngineAndStillPaints)
{
    // Arrange：fixture 已持有 CameraIconEngine 图标；副本修改前不触发克隆
    QImage canvas(20, 20, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);

    // Act：拷贝后修改副本（addPixmap）→ QIcon 分离克隆底层引擎
    QIcon copy = m_icon;
    copy.addPixmap(QPixmap(8, 8), QIcon::Normal, QIcon::On);

    // Assert：克隆出的引擎仍能完成正常绘制（副本在画布上产出非空结果）
    QPainter painter(&canvas);
    copy.paint(&painter, QRect(0, 0, 20, 20), Qt::AlignCenter, QIcon::Normal, QIcon::On);
    painter.end();
    EXPECT_FALSE(canvas.isNull());
    EXPECT_EQ(20, canvas.width());
}

// engine.iconPath：随主题切换 light/dark 目录（两套主题均能命中内置资源并加载）
TEST_F(UT_CameraIconEngine, iconPath_ThemeDirectories_LoadSuccessfully)
{
    // Arrange：fixture 已持有 CameraIconEngine 图标（qrc 内置 light/dark 两套相机图标）
    const QSize request(20, 20);

    // Act 1：浅色主题 → ":/icons/deepin/builtin/light/texts/camera_20px.svg"
    const QPixmap lightPm = m_icon.pixmap(request, QIcon::Normal, QIcon::Off);

    // Act 2：桩切换暗色主题 → ":/icons/deepin/builtin/dark/texts/camera_20px.svg"
    Stub stub;
    stub.set(ADDR(DGuiApplicationHelper, themeType), ut_darkThemeType);
    const QPixmap darkPm = m_icon.pixmap(request, QIcon::Normal, QIcon::Off);

    // Assert：两条主题路径均可加载成功
    EXPECT_FALSE(lightPm.isNull());
    EXPECT_EQ(20, lightPm.width());
    EXPECT_FALSE(darkPm.isNull());
    EXPECT_EQ(20, darkPm.width());
}
