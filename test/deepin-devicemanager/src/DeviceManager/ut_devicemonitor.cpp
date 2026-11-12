// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ===================== 用例计数声明（总用例数: 49） =====================
// 设备: DeviceMonitor（显示器设备）
//
// | method | level | factors | min | actual |
// |---|---|---|---|---|
// | setInfoFromXradr | high | 等价类/分支 | 4 | 5 |
// | getMonitorResolutionMap | high | 等价类/分支 | 3 | 4 |
// | setMainInfoFromXrandr | high | 等价类/分支 | 3 | 4 |
// | caculateScreenSize | high | 等价类/边界/分支 | 4 | 6 |
// | getMonitorNameFromEdid | high | 等价类 | 2 | 2 |
// | parseMonitorSize | mid | 等价类/边界 | 2 | 3 |
// | setInfoFromHwinfo | mid | 等价类 | 1 | 2 |
// | setInfoFromEdid | mid | 等价类 | 1 | 2 |
// | transWeekToDate | mid | 边界 | 2 | 3 |
// | caculateScreenRatio | mid | 等价类/分支 | 2 | 3 |
// | gcd | low | 等价类 | 2 | 4 |
// | findAspectRatio | low | 分支 | 2 | 2 |
// | 加载接口与getter | low | 冒烟 | 5 | 7 |
//
// 环境限制: setInfoFromXradr/setMainInfoFromXrandr 依赖 qApp 全局应用实例,
// 运行于 QT_QPA_PLATFORM=offscreen。

#include "DeviceMonitor.h"
#include "EDIDParser.h"
#include "stub.h"

#include <DApplication>

#include <QCoreApplication>
#include <QMap>
#include <QSize>
#include <QString>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

// ===================== 数据构造辅助 =====================
static const char *ut_monitor_edid = "00ffffffffffff005a63384001010101\n0d1e010380351d782ece65a657519f27\n";

static const char *ut_monitor_xrandrMain =
    "HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";

static void ut_monitor_sethwinfomap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Model", "VIEWSONIC VA2430-FHD");
    mapinfo.insert("Vendor", "VIEWSONIC");
    mapinfo.insert("Size", "527x296 mm");
    mapinfo.insert("Resolution", "720x400@70Hz 640x480@60Hz 640x480@67Hz");
    mapinfo.insert("Serial ID", "VSQ201321294");
    mapinfo.insert("Year of Manufacture", "2020");
    mapinfo.insert("Week of Manufacture", "13");
}

static void ut_monitor_setedidmap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Vendor", "Vendor");
    mapinfo.insert("Size", "Size");
    mapinfo.insert("Date", "2020-04");
}

// isDXcbPlatform 桩: offscreen 平台下强制走 XCB 分支, 覆盖主屏判定逻辑
static bool ut_monitor_isDXcbPlatform()
{
    return true;
}

// EDIDParser::height 桩: 返回无效高度, 覆盖 height<=0 分支
static int ut_monitor_invalidHeight()
{
    return -1;
}

// ===================== 测试夹具 =====================
class UT_DeviceMonitor : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_deviceMonitor = new DeviceMonitor;
    }
    void TearDown() override
    {
        delete m_deviceMonitor;
        m_deviceMonitor = nullptr;
    }
    DeviceMonitor *m_deviceMonitor = nullptr;
    Stub m_stub;
};

// ===================== parseMonitorSize（B1-B3） =====================
// 用例: "527x296 mm" 格式解析出宽高与英寸数
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_ParseMonitorSize_SpaceFormat_ReturnsInch)
{
    // Arrange
    QString sizeDescription = "527x296 mm";
    double inch = 0;
    QSize size = QSize(0, 0);
    // Act
    QString ret = m_deviceMonitor->parseMonitorSize(sizeDescription, inch, size);
    // Assert
    EXPECT_STREQ("23.8 inch (527x296 mm)", ret.toStdString().c_str());
    EXPECT_EQ(527, m_deviceMonitor->m_Width);
    EXPECT_EQ(296, m_deviceMonitor->m_Height);
    EXPECT_EQ(QSize(527, 296), size);
}

// 用例: "527mm x 296mm" 格式解析出宽高与英寸数
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_ParseMonitorSize_MmXFormat_ReturnsInch)
{
    // Arrange
    QString sizeDescription = "527mm x 296mm";
    double inch = 0;
    QSize size = QSize(0, 0);
    // Act
    QString ret = m_deviceMonitor->parseMonitorSize(sizeDescription, inch, size);
    // Assert
    EXPECT_STREQ("23.8 inch (527mm x 296mm)", ret.toStdString().c_str());
    EXPECT_EQ(527, m_deviceMonitor->m_Width);
    EXPECT_EQ(296, m_deviceMonitor->m_Height);
    EXPECT_STREQ("23.8", QString::number(inch, 10, 1).toStdString().c_str());
}

// 用例: 无效描述原样返回且 inch 为 0（无效输入等价类）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_ParseMonitorSize_InvalidText_ReturnsOriginal)
{
    // Arrange
    QString sizeDescription = "unknown size";
    double inch = -1;
    QSize size = QSize(0, 0);
    // Act
    QString ret = m_deviceMonitor->parseMonitorSize(sizeDescription, inch, size);
    // Assert
    EXPECT_EQ(ret, QString("unknown size"));
    EXPECT_EQ(0.0, inch);
    EXPECT_EQ(QSize(0, 0), size);
}

// ===================== setInfoFromHwinfo =====================
// 用例: 完整 hwinfo map 解析型号/厂商/分辨率/尺寸/日期
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetInfoFromHwinfo_FullMap_ParsesFields)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_monitor_sethwinfomap(mapinfo);
    // Act
    m_deviceMonitor->setInfoFromHwinfo(mapinfo);
    // Assert
    EXPECT_STREQ("VIEWSONIC VA2430-FHD", m_deviceMonitor->m_Name.toStdString().c_str());
    EXPECT_STREQ("VIEWSONIC", m_deviceMonitor->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("720x400@70Hz, 640x480@60Hz, 640x480@67Hz", m_deviceMonitor->m_SupportResolution.toStdString().c_str());
    EXPECT_STREQ("23.8 inch (527x296 mm)", m_deviceMonitor->m_ScreenSize.toStdString().c_str());
    EXPECT_STREQ("VSQ201321294", m_deviceMonitor->m_SerialNumber.toStdString().c_str());
    EXPECT_STREQ("2020-03", m_deviceMonitor->m_ProductionWeek.toStdString().c_str());
}

// 用例: 空 map 不崩溃且字段保持为空（无效输入等价类）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetInfoFromHwinfo_EmptyMap_NoFields)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    // Act
    m_deviceMonitor->setInfoFromHwinfo(mapinfo);
    // Assert
    EXPECT_TRUE(m_deviceMonitor->m_Name.isEmpty());
    EXPECT_TRUE(m_deviceMonitor->m_SerialNumber.isEmpty());
    EXPECT_EQ(m_deviceMonitor->name(), QString(""));
}

// ===================== setInfoFromEdid（B1-B2） =====================
// 用例: EDID map 解析名称/厂商/尺寸
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetInfoFromEdid_Map_ParsesNameVendorSize)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_monitor_setedidmap(mapinfo);
    // Act
    m_deviceMonitor->setInfoFromEdid(mapinfo);
    // Assert
    EXPECT_STREQ("Monitor Vendor", m_deviceMonitor->m_Name.toStdString().c_str());
    EXPECT_STREQ("Size", m_deviceMonitor->m_ScreenSize.toStdString().c_str());
    EXPECT_STREQ("Vendor", m_deviceMonitor->m_Vendor.toStdString().c_str());
}

// 用例: EDID map 中 Date 键设置生产日期
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetInfoFromEdid_DateKey_SetsProductionDate)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_monitor_setedidmap(mapinfo);
    // Act
    m_deviceMonitor->setInfoFromEdid(mapinfo);
    // Assert
    EXPECT_STREQ("2020-04", m_deviceMonitor->m_ProductionWeek.toStdString().c_str());
    EXPECT_FALSE(m_deviceMonitor->m_ProductionWeek.isEmpty());
}

// ===================== transWeekToDate（B1-B3） =====================
// 用例: 第 13 周转换为 2020-03
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_TransWeekToDate_Week13_ReturnsMarch)
{
    // Arrange
    const QString year = "2020";
    const QString week = "13";
    // Act
    QString date = m_deviceMonitor->transWeekToDate(year, week);
    // Assert
    EXPECT_STREQ("2020-03", date.toStdString().c_str());
    EXPECT_TRUE(date.startsWith("2020-"));
}

// 用例: 第 1 周转换落在 1 月（边界值）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_TransWeekToDate_WeekOne_ReturnsJanuary)
{
    // Arrange
    const QString year = "2021";
    const QString week = "1";
    // Act
    QString date = m_deviceMonitor->transWeekToDate(year, week);
    // Assert
    EXPECT_STREQ("2021-01", date.toStdString().c_str());
    EXPECT_TRUE(date.contains("01"));
}

// 用例: 第 52 周转换落在 12 月（边界值）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_TransWeekToDate_Week52_ReturnsDecember)
{
    // Arrange
    const QString year = "2019";
    const QString week = "52";
    // Act
    QString date = m_deviceMonitor->transWeekToDate(year, week);
    // Assert
    EXPECT_STREQ("2019-12", date.toStdString().c_str());
    EXPECT_TRUE(date.startsWith("2019-"));
}

// ===================== setInfoFromXradr（B1-B5） =====================
// 用例: 主信息无 connected 关键字返回 false（B1 无效输入等价类）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetInfoFromXradr_NoConnected_ReturnsFalse)
{
    // Arrange
    m_deviceMonitor->m_Interface = "VGA";
    // Act
    bool result = m_deviceMonitor->setInfoFromXradr("/", "/", "/", "/");
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceMonitor->m_Interface, QString("VGA"));
}

// 用例: disconnected 设备返回 false（B2）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetInfoFromXradr_Disconnected_ReturnsFalse)
{
    // Arrange
    m_deviceMonitor->m_Interface = "";
    // Act
    bool result = m_deviceMonitor->setInfoFromXradr("disconnected", "/", "/", "/");
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceMonitor->m_Interface, QString(""));
}

// 用例: XCB 平台主屏但 EDID 为空返回 false 且标记主屏（B3）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetInfoFromXradr_PrimaryNoEdid_ReturnsFalse)
{
    // Arrange
    m_deviceMonitor->m_Interface = "";
    QString main = "HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";
    QString edid = "";
    m_stub.set(ADDR(DApplication, isDXcbPlatform), ut_monitor_isDXcbPlatform);
    // Act
    bool result = m_deviceMonitor->setInfoFromXradr(main, edid, "/", "/");
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceMonitor->m_CurrentResolution, QString(""));
}

// 用例: 生产日期与 EDID 不一致返回 false（B4 分支）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetInfoFromXradr_ProductionMismatch_ReturnsFalse)
{
    // Arrange
    m_deviceMonitor->m_Interface = "";
    m_deviceMonitor->m_ProductionWeek = "2020-04";
    QString main = "HDMI-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";
    QString edid = ut_monitor_edid;
    m_stub.set(ADDR(DApplication, isDXcbPlatform), ut_monitor_isDXcbPlatform);
    // Act
    bool result = m_deviceMonitor->setInfoFromXradr(main, edid, "/", "/");
    // Assert
    EXPECT_FALSE(result);
    EXPECT_STREQ("2020-04", m_deviceMonitor->m_ProductionWeek.toStdString().c_str());
}

// 用例: 生产日期一致时解析成功（B5）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetInfoFromXradr_MatchingProduction_ReturnsTrue)
{
    // Arrange
    m_deviceMonitor->m_Interface = "";
    m_deviceMonitor->m_ProductionWeek = "2020-03";
    QString main = ut_monitor_xrandrMain;
    QString edid = ut_monitor_edid;
    QString rate = "60.00Hz";
    // Act
    bool result = m_deviceMonitor->setInfoFromXradr(main, edid, rate, "");
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("HDMI", m_deviceMonitor->m_Interface.toStdString().c_str());
    EXPECT_STREQ("1920x1080@60.00Hz", m_deviceMonitor->m_CurrentResolution.toStdString().c_str());
    EXPECT_STREQ("2020-03", m_deviceMonitor->m_ProductionWeek.toStdString().c_str());
}

// ===================== setMainInfoFromXrandr（B1-B4） =====================
// 用例: 主屏信息带刷新率解析当前分辨率与接口
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetMainInfoFromXrandr_PrimaryWithRate_SetsResolution)
{
    // Arrange
    QString main = ut_monitor_xrandrMain;
    QString rate = "60.00Hz";
    // Act
    bool result = m_deviceMonitor->setMainInfoFromXrandr(main, rate);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("1920x1080@60.00Hz", m_deviceMonitor->m_CurrentResolution.toStdString().c_str());
    EXPECT_STREQ("HDMI", m_deviceMonitor->m_Interface.toStdString().c_str());
    EXPECT_STREQ("HDMI-1", m_deviceMonitor->m_RawInterface.toStdString().c_str());
}

// 用例: 无刷新率时仅设置分辨率
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetMainInfoFromXrandr_NoRate_SetsResolutionOnly)
{
    // Arrange
    QString main = "HDMI-1 connected 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm";
    QString rate = "";
    // Act
    bool result = m_deviceMonitor->setMainInfoFromXrandr(main, rate);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("1920x1080", m_deviceMonitor->m_CurrentResolution.toStdString().c_str());
    EXPECT_FALSE(m_deviceMonitor->m_CurrentResolution.contains("@"));
}

// 用例: XCB 平台且含 primary 标记时主屏为 Yes（B3）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetMainInfoFromXrandr_PrimaryOnXcb_SetsYes)
{
    // Arrange
    QString main = ut_monitor_xrandrMain;
    QString rate = "60.00Hz";
    m_stub.set(ADDR(DApplication, isDXcbPlatform), ut_monitor_isDXcbPlatform);
    // Act
    bool result = m_deviceMonitor->setMainInfoFromXrandr(main, rate);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("Yes", m_deviceMonitor->m_MainScreen.toStdString().c_str());
    EXPECT_STREQ("1920x1080@60.00Hz", m_deviceMonitor->m_CurrentResolution.toStdString().c_str());
}

// 用例: XCB 平台无 primary 标记时主屏为 NO（B4 分支）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SetMainInfoFromXrandr_NonPrimaryOnXcb_SetsNo)
{
    // Arrange
    QString main = "HDMI-2 connected 1280x1024+0+0 (normal left inverted right x axis y axis) 338mm x 270mm";
    QString rate = "75.02Hz";
    m_stub.set(ADDR(DApplication, isDXcbPlatform), ut_monitor_isDXcbPlatform);
    // Act
    bool result = m_deviceMonitor->setMainInfoFromXrandr(main, rate);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("NO", m_deviceMonitor->m_MainScreen.toStdString().c_str());
    EXPECT_STREQ("1280x1024@75.02Hz", m_deviceMonitor->m_CurrentResolution.toStdString().c_str());
}

// ===================== initFilterKey / getter =====================
// 用例: 初始化过滤键后包含 1 个键
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_InitFilterKey_FillsOne)
{
    // Arrange
    int keyCount = 0;
    // Act
    m_deviceMonitor->initFilterKey();
    keyCount = m_deviceMonitor->m_FilterKey.size();
    // Assert
    EXPECT_EQ(1, keyCount);
    EXPECT_FALSE(m_deviceMonitor->m_FilterKey.isEmpty());
}

// 用例: name 返回设备名称
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_Name_ReturnsDeviceName)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_monitor_sethwinfomap(mapinfo);
    m_deviceMonitor->setInfoFromHwinfo(mapinfo);
    // Act
    QString name = m_deviceMonitor->name();
    // Assert
    EXPECT_STREQ("VIEWSONIC VA2430-FHD", name.toStdString().c_str());
    EXPECT_EQ(name, m_deviceMonitor->m_Name);
}

// 用例: driver 默认为空
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_Driver_ReturnsEmpty)
{
    // Arrange
    QString driver = QString();
    // Act
    driver = m_deviceMonitor->driver();
    // Assert
    EXPECT_STREQ("", driver.toStdString().c_str());
    EXPECT_TRUE(driver.isEmpty());
}

// 用例: subTitle 返回设备名称
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_SubTitle_ReturnsName)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_monitor_sethwinfomap(mapinfo);
    m_deviceMonitor->setInfoFromHwinfo(mapinfo);
    // Act
    QString title = m_deviceMonitor->subTitle();
    // Assert
    EXPECT_STREQ("VIEWSONIC VA2430-FHD", title.toStdString().c_str());
    EXPECT_EQ(title, m_deviceMonitor->name());
}

// 用例: 概况为名称加屏幕尺寸
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_GetOverviewInfo_NamePlusScreenSize)
{
    // Arrange
    QMap<QString, QString> mapinfo;
    ut_monitor_sethwinfomap(mapinfo);
    m_deviceMonitor->setInfoFromHwinfo(mapinfo);
    // Act
    QString overview = m_deviceMonitor->getOverviewInfo();
    // Assert
    EXPECT_STREQ("VIEWSONIC VA2430-FHD(23.8 inch (527x296 mm))", overview.toStdString().c_str());
    EXPECT_TRUE(overview.contains("23.8"));
}

// ===================== caculateScreenRatio（B1-B3） =====================
// 用例: 1920x1080 计算为 16 : 9
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_CaculateScreenRatio_1920x1080_Sets16x9)
{
    // Arrange
    m_deviceMonitor->m_CurrentResolution = "1920x1080";
    // Act
    m_deviceMonitor->caculateScreenRatio();
    // Assert
    EXPECT_STREQ("16 : 9", m_deviceMonitor->m_AspectRatio.toStdString().c_str());
    EXPECT_TRUE(m_deviceMonitor->m_AspectRatio.contains(":"));
}

// 用例: 宽高比大于 21 时走 findAspectRatio 近似（B2 分支, 2560x1080→19:8）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_CaculateScreenRatio_Ultrawide_UsesFindAspectRatio)
{
    // Arrange
    m_deviceMonitor->m_CurrentResolution = "2560x1080";
    // Act
    m_deviceMonitor->caculateScreenRatio();
    // Assert
    EXPECT_STREQ("19 : 8", m_deviceMonitor->m_AspectRatio.toStdString().c_str());
    EXPECT_FALSE(m_deviceMonitor->m_AspectRatio.isEmpty());
}

// 用例: 非法分辨率不设置宽高比（无效输入等价类）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_CaculateScreenRatio_InvalidResolution_KeepsEmpty)
{
    // Arrange
    m_deviceMonitor->m_CurrentResolution = "not-a-resolution";
    // Act
    m_deviceMonitor->caculateScreenRatio();
    // Assert
    EXPECT_TRUE(m_deviceMonitor->m_AspectRatio.isEmpty());
    EXPECT_EQ(m_deviceMonitor->m_AspectRatio, QString(""));
}

// ===================== gcd（B1-B4） =====================
// 用例: 互质数最大公约数为 1
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_Gcd_Coprime_ReturnsOne)
{
    // Arrange
    int a = 10;
    int b = 11;
    // Act
    int result = m_deviceMonitor->gcd(a, b);
    // Assert
    EXPECT_EQ(1, result);
    EXPECT_EQ(result, 1);
}

// 用例: 相等数最大公约数为自身
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_Gcd_Equal_ReturnsValue)
{
    // Arrange
    int a = 10;
    int b = 10;
    // Act
    int result = m_deviceMonitor->gcd(a, b);
    // Assert
    EXPECT_EQ(10, result);
    EXPECT_NE(result, 1);
}

// 用例: 1920x1080 最大公约数为 120
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_Gcd_Common_ReturnsGcd)
{
    // Arrange
    int a = 1920;
    int b = 1080;
    // Act
    int result = m_deviceMonitor->gcd(a, b);
    // Assert
    EXPECT_EQ(120, result);
    EXPECT_TRUE(1920 % result == 0);
}

// 用例: a < b 时交换后仍正确（B 交换分支）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_Gcd_ReversedArgs_ReturnsGcd)
{
    // Arrange
    int a = 36;
    int b = 48;
    // Act
    int result = m_deviceMonitor->gcd(a, b);
    // Assert
    EXPECT_EQ(12, result);
    EXPECT_TRUE(48 % result == 0);
}

// ===================== findAspectRatio（B1-B2） =====================
// 用例: 16:9 精确匹配（B1）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_FindAspectRatio_16x9_ReturnsExact)
{
    // Arrange
    int arw = 0;
    int arh = 0;
    // Act
    bool result = m_deviceMonitor->findAspectRatio(1920, 1080, arw, arh);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(16, arw);
    EXPECT_EQ(9, arh);
}

// 用例: 2560x1080 在 1% 容差内近似为 19:8（B2）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_FindAspectRatio_Ultrawide_ReturnsApprox)
{
    // Arrange
    int arw = 0;
    int arh = 0;
    // Act
    bool result = m_deviceMonitor->findAspectRatio(2560, 1080, arw, arh);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(19, arw);
    EXPECT_EQ(8, arh);
}

// ===================== caculateScreenSize（B1-B6） =====================
// 用例: 有效 EDID 计算屏幕尺寸（B1）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_CaculateScreenSize_ValidEdid_SetsSize)
{
    // Arrange
    QString edid = ut_monitor_edid;
    m_deviceMonitor->m_ProductionWeek = "2020-03";
    // Act
    bool result = m_deviceMonitor->caculateScreenSize(edid);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("23.8 inch(530mm×290mm)", m_deviceMonitor->m_ScreenSize.toStdString().c_str());
    EXPECT_TRUE(m_deviceMonitor->m_ScreenSize.contains("23.8"));
}

// 用例: 空 EDID 返回 false（无效输入等价类）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_CaculateScreenSize_EmptyEdid_ReturnsFalse)
{
    // Arrange
    QString edid = "";
    m_deviceMonitor->m_ProductionWeek = "2020-03";
    // Act
    bool result = m_deviceMonitor->caculateScreenSize(edid);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceMonitor->m_ScreenSize, QString(""));
}

// 用例: EDID 尺寸与 hwinfo 差距大时使用 EDID 尺寸覆盖（B 偏差分支）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_CaculateScreenSize_LargeDeviation_UsesEdid)
{
    // Arrange
    m_deviceMonitor->m_Width = 527;
    m_deviceMonitor->m_Height = 296;
    m_deviceMonitor->m_ProductionWeek = "2020-03";
    QString edid = ut_monitor_edid;
    // Act
    bool result = m_deviceMonitor->caculateScreenSize(edid);
    // Assert
    EXPECT_TRUE(result);
    EXPECT_STREQ("23.8 inch(530mm×290mm)", m_deviceMonitor->m_ScreenSize.toStdString().c_str());
    EXPECT_TRUE(m_deviceMonitor->m_ScreenSize.contains("290"));
}

// 用例: EDID 高度无效返回 false（B3, height<=0 分支）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_CaculateScreenSize_HeightInvalid_ReturnsFalse)
{
    // Arrange
    m_deviceMonitor->m_Width = 527;
    m_deviceMonitor->m_Height = 296;
    m_deviceMonitor->m_ProductionWeek = "2020-03";
    QString edid = ut_monitor_edid;
    m_stub.set(ADDR(EDIDParser, height), ut_monitor_invalidHeight);
    // Act
    bool result = m_deviceMonitor->caculateScreenSize(edid);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(m_deviceMonitor->m_ScreenSize, QString(""));
}

// 用例: 无参重载从 m_ScreenSize 文本解析宽高（B5）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_CaculateScreenSize_NoArgOverload_ParsesText)
{
    // Arrange
    m_deviceMonitor->m_ScreenSize = "527x296 mm";
    // Act
    m_deviceMonitor->caculateScreenSize();
    // Assert
    EXPECT_STREQ("23.8 inch(527mm×296mm)", m_deviceMonitor->m_ScreenSize.toStdString().c_str());
    EXPECT_EQ(527, m_deviceMonitor->m_Width);
    EXPECT_EQ(296, m_deviceMonitor->m_Height);
}

// 用例: 生产日期与 EDID 不一致返回 false（B6 分支）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_CaculateScreenSize_ProductionMismatch_ReturnsFalse)
{
    // Arrange
    m_deviceMonitor->m_ProductionWeek = "2021-01";
    QString edid = ut_monitor_edid;
    // Act
    bool result = m_deviceMonitor->caculateScreenSize(edid);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_STREQ("2021-01", m_deviceMonitor->m_ProductionWeek.toStdString().c_str());
}

// ===================== getMonitorNameFromEdid（B1-B2） =====================
// 用例: 有效 EDID 无名称描述符时返回 Unknown Monitor 默认值
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_GetMonitorNameFromEdid_NoNameDescriptor_ReturnsUnknownMonitor)
{
    // Arrange
    QString edid = ut_monitor_edid;
    // Act
    QString name = m_deviceMonitor->getMonitorNameFromEdid(edid);
    // Assert
    EXPECT_EQ(name, QString("Unknown Monitor"));
    EXPECT_TRUE(name.contains("Monitor"));
}

// 用例: 无效 EDID 返回空串（无效输入等价类）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_GetMonitorNameFromEdid_InvalidEdid_ReturnsEmpty)
{
    // Arrange
    QString edid = "";
    // Act
    QString name = m_deviceMonitor->getMonitorNameFromEdid(edid);
    // Assert
    EXPECT_EQ(name, QString(""));
    EXPECT_FALSE(name.contains("Monitor"));
}

// ===================== getMonitorResolutionMap（B1-B4） =====================
// 用例: 解析 xrandr 原始文本构建分辨率-刷新率映射
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_GetMonitorResolutionMap_RawText_BuildsMap)
{
    // Arrange
    QString rawText =
        "HDMI-1 connected primary 1920x1080+0+0 527mm x 296mm\n"
        "   1920x1080     60.00*+  59.94    50.00\n"
        "   1280x720      60.00    59.94\n";
    QString key = "HDMI-1";
    // Act
    QMap<QString, QStringList> map = m_deviceMonitor->getMonitorResolutionMap(rawText, key, false);
    // Assert
    EXPECT_TRUE(map.contains(key));
    EXPECT_EQ(5, map.value(key).size());
    EXPECT_EQ(QString("1920x1080@60.00Hz"), map.value(key).at(0));
    EXPECT_EQ(QString("1280x720@59.94Hz"), map.value(key).at(4));
}

// 用例: round=true 时刷新率四舍五入格式化（B round 分支）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_GetMonitorResolutionMap_RoundTrue_FormatsRate)
{
    // Arrange
    QString rawText =
        "HDMI-1 connected primary 1920x1080+0+0 527mm x 296mm\n"
        "   1920x1080     60.00*+  164.84\n";
    QString key = "HDMI-1";
    // Act
    QMap<QString, QStringList> map = m_deviceMonitor->getMonitorResolutionMap(rawText, key, true);
    // Assert
    EXPECT_TRUE(map.contains(key));
    EXPECT_EQ(2, map.value(key).size());
    EXPECT_EQ(QString("1920x1080@60Hz"), map.value(key).at(0));
    EXPECT_EQ(QString("1920x1080@165Hz"), map.value(key).at(1));
}

// 用例: disconnected 行被跳过, 未知 key 返回空映射（B2/B miss 分支）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_GetMonitorResolutionMap_DisconnectedSkipped)
{
    // Arrange
    QString rawText =
        "HDMI-1 disconnected\n"
        "HDMI-2 connected 1280x1024+0+0 338mm x 270mm\n"
        "   1280x1024     75.02    60.02\n";
    QString key = "HDMI-1";
    // Act
    QMap<QString, QStringList> map = m_deviceMonitor->getMonitorResolutionMap(rawText, key, false);
    // Assert
    EXPECT_TRUE(map.isEmpty());
    EXPECT_EQ(0, map.size());
}

// 用例: 多显示器时按 key 返回单显示器子映射（B key 命中分支）
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_GetMonitorResolutionMap_KeyMatchReturnsSubset)
{
    // Arrange
    QString rawText =
        "HDMI-1 connected primary 1920x1080+0+0 527mm x 296mm\n"
        "   1920x1080     60.00*+\n"
        "VGA-1 connected 1280x1024+1920+0 338mm x 270mm\n"
        "   1280x1024     75.02    60.02\n";
    QString key = "VGA-1";
    // Act
    QMap<QString, QStringList> map = m_deviceMonitor->getMonitorResolutionMap(rawText, key, false);
    // Assert
    EXPECT_EQ(1, map.size());
    EXPECT_TRUE(map.contains("VGA-1"));
    EXPECT_EQ(QString("1280x1024@75.02Hz"), map.value("VGA-1").at(0));
}

// ===================== 加载接口 =====================
// 用例: 无基础信息时基础信息列表为空
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_LoadBaseDeviceInfo_EmptyList)
{
    // Arrange
    int baseCount = -1;
    // Act
    m_deviceMonitor->loadBaseDeviceInfo();
    baseCount = m_deviceMonitor->m_LstBaseInfo.size();
    // Assert
    EXPECT_EQ(0, baseCount);
    EXPECT_TRUE(m_deviceMonitor->m_LstBaseInfo.isEmpty());
}

// 用例: 设置当前分辨率后其他信息列表为 1 项
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_LoadOtherDeviceInfo_CurrentResolution_OneRow)
{
    // Arrange
    m_deviceMonitor->m_CurrentResolution = "1920*1080@60Hz";
    // Act
    m_deviceMonitor->loadOtherDeviceInfo();
    // Assert
    EXPECT_EQ(1, m_deviceMonitor->m_LstOtherInfo.size());
    EXPECT_FALSE(m_deviceMonitor->m_LstOtherInfo.isEmpty());
}

// 用例: 表数据为 3 行
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_LoadTableData_ThreeRows)
{
    // Arrange
    int dataCount = 0;
    // Act
    m_deviceMonitor->loadTableData();
    dataCount = m_deviceMonitor->m_TableData.size();
    // Assert
    EXPECT_EQ(3, dataCount);
    EXPECT_EQ(3, m_deviceMonitor->m_TableData.size());
}

// 用例: 显示器设备 available 恒为 true
TEST_F(UT_DeviceMonitor, UT_DeviceMonitor_Available_ReturnsTrue)
{
    // Arrange
    bool available = false;
    // Act
    available = m_deviceMonitor->available();
    // Assert
    EXPECT_TRUE(available);
    EXPECT_EQ(available, true);
}
