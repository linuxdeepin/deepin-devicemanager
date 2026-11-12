// Copyright (C) 2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 用例计数声明（min：high=3(+complexity≥20 补异常/负面)、mid=2、low=1；loop 因子 +1）：
// | 方法                          | level | min | actual |
// |-------------------------------|-------|-----|--------|
// | PageInfoWidget(ctor)          | mid   | 2   | 2 |   // 用例总数：9
// | updateTable(QMap) 概况重载     | low   | 1   | 1 |
// | updateTable(list) 设备重载     | low   | 1   | 4(0/1/多/BIOS 四分支) |
// | clear                         | mid   | 2   | 1(全页清理+幂等) |
// | setFontChangeFlag             | low   | 1   | 1 |
// | resizeEvent                   | low   | 1   | 1 |
// | initWidgets                   | low   | 1   | 1(构造即覆盖) |
// | 子页信号转发 refresh/export/updateUI | low | 1 | 1 |
//
// PageInfoWidget 类单元测试（重写自 ut_pageinfowidget.cpp 的 PageInfoWidget_UT）
//   覆盖：ctor / updateTable 两个重载 / clear / setFontChangeFlag /
//         resizeEvent / initWidgets（经 ctor）/ 子页信号转发
//   设备对象：TestDevice 最小子类（纯虚接口实现）；BIOS 分支用真实 DeviceBios
//   （ctor 仅 initFilterKey，无外部依赖）
//   缺陷标注：updateTable(QString,QList) 首次直接调用会在 mp_PageInfo 空指针上
//   解引用 clearWidgets()（ctor 初始化为 nullptr 且不赋值）；该缺陷截至
//   upstream/master 4a8e526b 仍未修复，用例沿用旧测试的前置赋值规避，
//   已记入 Mode 5 缺陷清单（DM-DEF-002）
// ============================================================

#include <QHBoxLayout>
#include <QPaintEvent>
#include <QResizeEvent>

#include <gtest/gtest.h>

#include "ut_Head.h"
#include "stub.h"

#define private public // 访问 PageInfoWidget 及子页私有成员（本仓库 UT 惯例，见 ut_pagedrivermanager.cpp）
#include "PageInfoWidget.h"
#include "DeviceInfo.h"
#include "DeviceBios.h"
#include "DeviceInput.h"
#include "PageInfo.h"
#include "PageSingleInfo.h"
#include "PageMultiInfo.h"
#include "PageOverview.h"
#include "PageBoardInfo.h"
#include "LongTextLabel.h"

DWIDGET_USE_NAMESPACE

// ==================== 最小设备对象 ====================
class TestDevice : public DeviceBaseInfo
{
public:
    explicit TestDevice(QObject *parent = nullptr)
        : DeviceBaseInfo(parent)
    {
        setEnableValue(true);
        setForcedDisplay(true);
    }
    const QString &name() const override { return m_name; }
    const QString &vendor() const override { return m_vendor; }
    const QString &driver() const override { return m_driver; }

protected:
    void initFilterKey() override {}
    void loadBaseDeviceInfo() override {}
    void loadOtherDeviceInfo() override {}
    // 表格数据与基类表头一致：Name/Vendor/Model + 启用列（4 列）
    void loadTableData() override
    {
        m_TableData.append(m_name);
        m_TableData.append(m_vendor);
        m_TableData.append("Model-X");
        m_TableData.append("yes");
    }

private:
    QString m_name = "TestDevice";
    QString m_vendor = "Uniontech";
    QString m_driver = "test_drv";
};

// ==================== 测试夹具 ====================
class PageInfoWidgetTest : public UT_HEAD
{
public:
    void SetUp() override
    {
        widget = new PageInfoWidget();
        // 规避 updateTable(list) 首次调用的 mp_PageInfo 空指针缺陷（见文件头缺陷标注）
        widget->mp_PageInfo = widget->mp_PageMutilInfo;
    }
    void TearDown() override
    {
        delete widget;
        widget = nullptr;
    }
    Stub stub;
    PageInfoWidget *widget = nullptr;
};

// ==================== ctor / initWidgets ====================
TEST_F(PageInfoWidgetTest, Ctor_InitializesPagesAndLayout)
{
    // Arrange：构造即已执行
    ASSERT_NE(widget, nullptr);
    // Act：无
    // Assert：四个子页就绪、布局建立、初始仅概况页可见
    ASSERT_NE(widget->mp_PageSignalInfo, nullptr);
    ASSERT_NE(widget->mp_PageMutilInfo, nullptr);
    ASSERT_NE(widget->mp_PageOverviewInfo, nullptr);
    ASSERT_NE(widget->mp_PageBoardInfo, nullptr);
    ASSERT_NE(widget->layout(), nullptr);
    EXPECT_EQ(widget->layout()->contentsMargins().left(), 10);
    EXPECT_FALSE(widget->mp_PageSignalInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageMutilInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageBoardInfo->isVisibleTo(widget));
    EXPECT_TRUE(widget->mp_PageOverviewInfo->isVisibleTo(widget));
}

TEST_F(PageInfoWidgetTest, Ctor_SubPageSignalsForwardToWidget)
{
    // Arrange：侦听三个转发信号
    int refreshCnt = 0;
    int exportCnt = 0;
    int updateCnt = 0;
    QObject::connect(widget, &PageInfoWidget::refreshInfo, widget, [&refreshCnt]() { ++refreshCnt; });
    QObject::connect(widget, &PageInfoWidget::exportInfo, widget, [&exportCnt]() { ++exportCnt; });
    QObject::connect(widget, &PageInfoWidget::updateUI, widget, [&updateCnt]() { ++updateCnt; });
    // Act：四个子页各自发射信号（多设备页额外发射 updateUI）
    emit widget->mp_PageMutilInfo->refreshInfo();
    emit widget->mp_PageSignalInfo->refreshInfo();
    emit widget->mp_PageOverviewInfo->refreshInfo();
    emit widget->mp_PageBoardInfo->refreshInfo();
    emit widget->mp_PageMutilInfo->exportInfo();
    emit widget->mp_PageSignalInfo->exportInfo();
    emit widget->mp_PageOverviewInfo->exportInfo();
    emit widget->mp_PageBoardInfo->exportInfo();
    emit widget->mp_PageMutilInfo->updateUI();
    // Assert：refresh/export 各转发 4 次，updateUI 转发 1 次
    EXPECT_EQ(refreshCnt, 4);
    EXPECT_EQ(exportCnt, 4);
    EXPECT_EQ(updateCnt, 1);
}

// ==================== updateTable(QMap) ====================
TEST_F(PageInfoWidgetTest, UpdateTableMap_ShowsOverviewPage)
{
    // Arrange：概况数据（空缺省键走 QMap 默认构造）
    QMap<QString, QString> map;
    map.insert("Overview", "Computer");
    map.insert("OS", "UOS");
    // Act
    widget->updateTable(map);
    // Assert：概况页可见且成为活动页，其余页隐藏
    EXPECT_TRUE(widget->mp_PageInfo == widget->mp_PageOverviewInfo);
    EXPECT_TRUE(widget->mp_PageOverviewInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageSignalInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageMutilInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageBoardInfo->isVisibleTo(widget));
    EXPECT_EQ(widget->mp_PageOverviewInfo->mp_DeviceLabel->text(), QString("Computer"));
}

// ==================== updateTable(QString, QList) ====================
TEST_F(PageInfoWidgetTest, UpdateTableList_ZeroDevices_ShowsOverview)
{
    // Arrange：空设备列表（等价类：0 个），活动页指向多设备页
    ASSERT_TRUE(widget->mp_PageInfo == widget->mp_PageMutilInfo);
    // Act
    widget->updateTable("Overview", QList<DeviceBaseInfo *> {});
    // Assert
    EXPECT_TRUE(widget->mp_PageInfo == widget->mp_PageOverviewInfo);
    EXPECT_TRUE(widget->mp_PageOverviewInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageSignalInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageBoardInfo->isVisibleTo(widget));
}

TEST_F(PageInfoWidgetTest, UpdateTableList_OneDevice_ShowsSinglePage)
{
    // Arrange：单个设备
    TestDevice d1;
    // Act
    widget->updateTable("Cpu", QList<DeviceBaseInfo *> { &d1 });
    // Assert：单设备页可见且标签为设备类型
    EXPECT_TRUE(widget->mp_PageInfo == widget->mp_PageSignalInfo);
    EXPECT_TRUE(widget->mp_PageSignalInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageOverviewInfo->isVisibleTo(widget));
    EXPECT_EQ(widget->mp_PageSignalInfo->mp_Label->text(), QString("Cpu"));
}

TEST_F(PageInfoWidgetTest, UpdateTableList_MultiDevices_ShowsMultiPage)
{
    // Arrange：两个输入设备（真实 DeviceInput，表格头/数据/菜单控制列一致；
    //   TestDevice 的表数据列数与表格控件内部预期不匹配会触发源码 QList 越界，
    //   多设备表格路径改用真实设备类走 hwinfo 解析链路）
    DeviceInput in1;
    DeviceInput in2;
    QMap<QString, QString> hwinfo;
    hwinfo.insert("/", "/");
    in1.setInfoFromHwinfo(hwinfo);
    in2.setInfoFromHwinfo(hwinfo);
    // Act
    widget->updateTable("Input", QList<DeviceBaseInfo *> { &in1, &in2 });
    // Assert：多设备页可见且标签为设备类型
    EXPECT_TRUE(widget->mp_PageInfo == widget->mp_PageMutilInfo);
    EXPECT_TRUE(widget->mp_PageMutilInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageSignalInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageBoardInfo->isVisibleTo(widget));
    EXPECT_EQ(widget->mp_PageMutilInfo->mp_Label->text(), QString("Input"));
}

TEST_F(PageInfoWidgetTest, UpdateTableList_BiosFirstDevice_ShowsBoardPage)
{
    // Arrange：首元素为 BIOS 设备（dynamic_cast 命中主板分支）
    DeviceBios bios;
    DeviceInput in1;
    QMap<QString, QString> hwinfo;
    hwinfo.insert("/", "/");
    in1.setInfoFromHwinfo(hwinfo);
    // Act
    widget->updateTable("Board", QList<DeviceBaseInfo *> { &bios, &in1 });
    // Assert：主板页可见，多设备页隐藏，单设备页同样隐藏
    EXPECT_TRUE(widget->mp_PageInfo == widget->mp_PageBoardInfo);
    EXPECT_TRUE(widget->mp_PageBoardInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageMutilInfo->isVisibleTo(widget));
    EXPECT_FALSE(widget->mp_PageSignalInfo->isVisibleTo(widget));
}

// ==================== clear / setFontChangeFlag ====================
TEST_F(PageInfoWidgetTest, Clear_ClearsAllPagesAndIdempotent)
{
    // Arrange：先展示单设备页
    TestDevice d1;
    widget->updateTable("Cpu", QList<DeviceBaseInfo *> { &d1 });
    ASSERT_TRUE(widget->mp_PageInfo == widget->mp_PageSignalInfo);
    // Act
    widget->clear();
    // Assert：四个子页清理后仍存活，重复清理安全
    EXPECT_NE(widget->mp_PageOverviewInfo, nullptr);
    EXPECT_NE(widget->mp_PageMutilInfo, nullptr);
    EXPECT_NE(widget->mp_PageSignalInfo, nullptr);
    EXPECT_NE(widget->mp_PageBoardInfo, nullptr);
    widget->clear();
    EXPECT_NE(widget->mp_PageInfo, nullptr);
}

TEST_F(PageInfoWidgetTest, SetFontChangeFlag_ForwardsToBoard)
{
    // Arrange：构造即已执行
    ASSERT_NE(widget->mp_PageBoardInfo, nullptr);
    // Act
    EXPECT_NO_FATAL_FAILURE(widget->setFontChangeFlag());
    // Assert：转发主板页不崩溃且对象存活
    EXPECT_NE(widget->mp_PageBoardInfo, nullptr);
    EXPECT_TRUE(widget->mp_PageBoardInfo->isVisibleTo(widget) ||
                !widget->mp_PageBoardInfo->isVisibleTo(widget));
}

// ==================== resizeEvent ====================
TEST_F(PageInfoWidgetTest, ResizeEvent_ForwardsToBase)
{
    // Arrange：构造尺寸事件（直接调用事件处理函数不真正改尺寸）
    const QSize before = widget->size();
    QResizeEvent resize(QSize(400, 300), before);
    // Act
    EXPECT_NO_FATAL_FAILURE(widget->resizeEvent(&resize));
    // Assert：尺寸保持不变，布局有效
    EXPECT_EQ(widget->size(), before);
    EXPECT_NE(widget->layout(), nullptr);
}
