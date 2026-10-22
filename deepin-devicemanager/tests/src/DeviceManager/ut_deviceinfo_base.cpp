// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DeviceBaseInfo 基类方法 (deepin-devicemanager/src/DeviceManager/DeviceInfo.cpp)
// 载体: DeviceCpu(具体子类)。
// 覆盖: isValueValid/displayWidth、set*/get* 简单对、VID/PID/Modalias、
//       FilterKey/OtherInfo 族、setAttribute、Attribs/Table 族、
//       HTML/Txt 输出、readDeviceInfoKeyValue、getDriverVersion。
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                      | level | factors | min | actual |
// |-----------------------------|-------|---------|-----|--------|
// | isValueValid                | high  | -       | 3   | 3      |
// | displayWidth                | mid   | -       | 2   | 2      |
// | hardwareClass               | low   | -       | 1   | 1      |
// | uniqueID                    | low   | -       | 1   | 1      |
// | sysPath                     | low   | -       | 1   | 1      |
// | canEnable                   | low   | -       | 1   | 1      |
// | canUninstall                | low   | -       | 1   | 1      |
// | setEnableValue              | low   | -       | 1   | 1      |
// | setForcedDisplay            | mid   | -       | 2   | 2      |
// | getVID/getPID/getVIDAndPID  | mid   | -       | 2   | 2      |
// | getModalias                 | low   | -       | 1   | 1      |
// | addFilterKey                | mid   | -       | 2   | 2      |
// | addOtherDeviceInfo          | mid   | -       | 2   | 2      |
// | setOtherDeviceInfo          | low   | -       | 1   | 1      |
// | getOtherMapInfo             | mid   | -       | 2   | 2      |
// | setAttribute                | mid   | -       | 2   | 3      |
// | nameTr                      | low   | -       | 1   | 1      |
// | getBaseAttribs              | mid   | -       | 2   | 2      |
// | getOtherAttribs             | mid   | -       | 2   | 2      |
// | getBaseTranslationAttribs   | mid   | -       | 2   | 2      |
// | getOtherTranslationAttribs  | mid   | -       | 2   | 2      |
// | getTableHeader              | mid   | -       | 2   | 2      |
// | getTableData                | mid   | -       | 2   | 2      |
// | toHtmlString                | low   | -       | 1   | 1      |
// | subTitleToHTML              | low   | -       | 1   | 1      |
// | readDeviceInfoKeyValue      | mid   | -       | 2   | 2      |
// | getDriverVersion            | low   | -       | 1   | 1      |
// | tableHeaderToTxt/tableInfoToTxt | low | -     | 1   | 1      |
// ============================================================

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QDomDocument>
#include <QTextStream>
#include <QStringList>

#define private public
#define protected public
#include "DeviceCpu.h"
#include "DeviceInfo.h"

// 用 DeviceCpu 作为 DeviceBaseInfo 的具体子类来补充测试基类方法
class UT_DeviceInfoBase : public ::testing::Test
{
public:
    void SetUp() override
    {
        ut_disable_system_dbus();  // 隔离真实系统服务, 避免运行期触发系统总线鉴权
        m_cpu = new DeviceCpu;
    }
    void TearDown() override
    {
        delete m_cpu;
        m_cpu = nullptr;
    }
    DeviceCpu *m_cpu = nullptr;
};

// ===================== isValueValid：覆盖所有无效分支 + 有效值 =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_isValueValid_invalid)
{
    // Arrange: 默认构造的 CPU 设备
    // Act: 执行被测调用
    // Assert: 空串与各占位符均判无效
    EXPECT_FALSE(m_cpu->isValueValid(""));
    EXPECT_FALSE(m_cpu->isValueValid("Unknown"));
    EXPECT_FALSE(m_cpu->isValueValid("N/A"));
    EXPECT_FALSE(m_cpu->isValueValid("n/a"));
    EXPECT_FALSE(m_cpu->isValueValid("Null"));
    EXPECT_FALSE(m_cpu->isValueValid("none"));
    EXPECT_FALSE(m_cpu->isValueValid("Not Provided"));
    EXPECT_FALSE(m_cpu->isValueValid("Not Specified"));
    EXPECT_FALSE(m_cpu->isValueValid("Default string"));
    EXPECT_FALSE(m_cpu->isValueValid("Unspecified"));
    EXPECT_FALSE(m_cpu->isValueValid("Not Present"));
    EXPECT_FALSE(m_cpu->isValueValid("<OUT OF SPEC>"));
    EXPECT_FALSE(m_cpu->isValueValid("Other"));
    EXPECT_FALSE(m_cpu->isValueValid("TBD"));
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_isValueValid_valid)
{
    // Arrange: 默认构造的 CPU 设备
    // Act: 执行被测调用
    // Assert: 普通值判有效
    EXPECT_TRUE(m_cpu->isValueValid("Intel Core i3"));
    EXPECT_TRUE(m_cpu->isValueValid("GenuineIntel"));
    EXPECT_TRUE(m_cpu->isValueValid("1024 MB"));
}

// ===================== displayWidth（静态） =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_displayWidth)
{
    // Arrange: 不同宽度特征的字符串
    // Act: 执行被测调用
    // Assert: 空=0, ASCII=1/字符, CJK=1.5/字符, 混合相加
    EXPECT_DOUBLE_EQ(0.0, DeviceBaseInfo::displayWidth(""));
    EXPECT_DOUBLE_EQ(3.0, DeviceBaseInfo::displayWidth("abc"));
    // 两个 CJK 字符，各占 1.5 => 3.0
    EXPECT_DOUBLE_EQ(3.0, DeviceBaseInfo::displayWidth(QString::fromUtf8("中文")));
    // 混合：1 ASCII(1) + 1 CJK(1.5) = 2.5
    EXPECT_DOUBLE_EQ(2.5, DeviceBaseInfo::displayWidth(QString::fromUtf8("a中")));
}

// ===================== 简单 set/get 对 =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_hardwareClass)
{
    // Arrange: 默认构造
    // Act: 执行被测调用
    // Assert: 默认为空, 设置后可读回
    EXPECT_TRUE(m_cpu->hardwareClass().isEmpty());
    m_cpu->setHardwareClass("cpu");
    EXPECT_STREQ("cpu", m_cpu->hardwareClass().toStdString().c_str());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_uniqueID)
{
    // Arrange: 默认构造
    // Act: 执行被测调用
    // Assert: 默认为空, 设置后可读回
    EXPECT_TRUE(m_cpu->uniqueID().isEmpty());
    m_cpu->setUniqueID("unique-123");
    EXPECT_STREQ("unique-123", m_cpu->uniqueID().toStdString().c_str());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_sysPath)
{
    // Arrange: 默认构造
    // Act: 执行被测调用
    // Assert: 默认为空, 设置后可读回
    EXPECT_TRUE(m_cpu->sysPath().isEmpty());
    m_cpu->setSysPath("/sys/devices/cpu");
    EXPECT_STREQ("/sys/devices/cpu", m_cpu->sysPath().toStdString().c_str());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_canEnable)
{
    // Arrange: 默认构造
    // Act: 执行被测调用
    // Assert: 默认不可使能, 置位后可
    EXPECT_FALSE(m_cpu->canEnable());
    m_cpu->setCanEnable(true);
    EXPECT_TRUE(m_cpu->canEnable());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_canUninstall)
{
    // Arrange: 默认构造
    // Act: 执行被测调用
    // Assert: 默认不可卸载, 置位后可
    EXPECT_FALSE(m_cpu->canUninstall());
    m_cpu->setCanUninstall(true);
    EXPECT_TRUE(m_cpu->canUninstall());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setEnableValue)
{
    // Arrange: 默认构造
    // Act: 执行被测调用
    // Assert: 写 false 后读 false, 写 true 后读 true
    m_cpu->setEnableValue(false);
    EXPECT_FALSE(m_cpu->m_Enable);
    m_cpu->setEnableValue(true);
    EXPECT_TRUE(m_cpu->m_Enable);
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setForcedDisplay)
{
    // Arrange: 默认构造
    // Act: 执行被测调用
    // Assert: 默认不强制显示, 置位后强制
    EXPECT_FALSE(m_cpu->m_forcedDisplay);
    m_cpu->setForcedDisplay(true);
    EXPECT_TRUE(m_cpu->m_forcedDisplay);
}

// ===================== VID / PID / VID_PID / Modalias =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_VID_PID_Modalias)
{
    // Arrange: 注入 VID/PID/VID_PID/Modalias
    m_cpu->m_VID = "8086";
    m_cpu->m_PID = "1234";
    m_cpu->m_VID_PID = "8086:1234";
    m_cpu->m_Modalias = "pci:v1234";

    // Act: 执行被测调用
    // Assert: 各 getter 原样返回
    EXPECT_STREQ("8086", m_cpu->getVID().toStdString().c_str());
    EXPECT_STREQ("1234", m_cpu->getPID().toStdString().c_str());
    EXPECT_STREQ("8086:1234", m_cpu->getVIDAndPID().toStdString().c_str());
    EXPECT_STREQ("pci:v1234", m_cpu->getModalias().toStdString().c_str());
}

// ===================== addFilterKey =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_addFilterKey)
{
    // Arrange: 清空预置 filter key
    m_cpu->m_FilterKey.clear();

    // Act: 追加键(含重复)
    m_cpu->addFilterKey("Name");
    m_cpu->addFilterKey("Vendor");
    m_cpu->addFilterKey("Name");  // 重复不增加

    // Assert: 去重后数量为 2
    EXPECT_EQ(2, m_cpu->m_FilterKey.size());
    EXPECT_TRUE(m_cpu->m_FilterKey.contains("Vendor"));
}

// ===================== addOtherDeviceInfo（头插） =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_addOtherDeviceInfo)
{
    // Arrange: 清空其他属性
    m_cpu->m_LstOtherInfo.clear();

    // Act: 追加三条(含空值)
    m_cpu->addOtherDeviceInfo("K1", "V1");
    m_cpu->addOtherDeviceInfo("K2", "");  // 空值不添加
    m_cpu->addOtherDeviceInfo("K3", "V3");  // 头插，排最前

    // Assert: 空值被过滤, 后插者排最前
    EXPECT_EQ(2, m_cpu->m_LstOtherInfo.size());
    EXPECT_STREQ("K3", m_cpu->m_LstOtherInfo.at(0).first.toStdString().c_str());
    EXPECT_STREQ("K1", m_cpu->m_LstOtherInfo.at(1).first.toStdString().c_str());
}

// ===================== setOtherDeviceInfo =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setOtherDeviceInfo)
{
    // Arrange: 默认映射
    // Act: 写入两个键
    m_cpu->setOtherDeviceInfo("Key1", "Value1");
    m_cpu->setOtherDeviceInfo("Key2", "Value2");

    // Assert: 两个键均可读回
    EXPECT_STREQ("Value1", m_cpu->m_MapOtherInfo["Key1"].toStdString().c_str());
    EXPECT_STREQ("Value2", m_cpu->m_MapOtherInfo["Key2"].toStdString().c_str());
}

// ===================== getOtherMapInfo =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getOtherMapInfo)
{
    // Arrange: 注册过滤键并构造映射
    m_cpu->addFilterKey("Speed");
    QMap<QString, QString> map;
    map.insert("Speed", "2667");
    map.insert("Junk", "xyz");  // 不在 FilterKey 中，忽略

    // Act: 提取其他属性
    m_cpu->getOtherMapInfo(map);

    // Assert: 过滤键收录, 其余忽略
    EXPECT_STREQ("2667", m_cpu->m_MapOtherInfo["Speed"].toStdString().c_str());
    EXPECT_FALSE(m_cpu->m_MapOtherInfo.contains("Junk"));
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getOtherMapInfo_nouse)
{
    // Arrange: 注册过滤键并构造含 nouse 的映射
    m_cpu->addFilterKey("Status");
    QMap<QString, QString> map;
    map.insert("Status", "this is NOUSE value");

    // Act: 提取其他属性
    m_cpu->getOtherMapInfo(map);

    // Assert: 含 "nouse"（忽略大小写）会被 remove
    EXPECT_FALSE(m_cpu->m_MapOtherInfo.contains("Status"));
    EXPECT_TRUE(m_cpu->m_MapOtherInfo.isEmpty());
}

// ===================== setAttribute =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setAttribute)
{
    // Arrange: 含 Name 键的映射
    QMap<QString, QString> map;
    map.insert("Name", "Intel CPU");
    QString var;

    // Act: 覆盖写入
    m_cpu->setAttribute(map, "Name", var);

    // Assert: 值写入; 二次写入保持一致
    EXPECT_STREQ("Intel CPU", var.toStdString().c_str());
    map["Name"] = "AMD CPU";
    m_cpu->setAttribute(map, "Name", var);
    EXPECT_STREQ("AMD CPU", var.toStdString().c_str());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setAttribute_noKey)
{
    // Arrange: 键缺失的映射
    QMap<QString, QString> map;
    QString var = "old";

    // Act: 执行被测调用
    // Assert: 键缺失不变; 未知键重复调用仍不变
    m_cpu->setAttribute(map, "Missing", var);
    EXPECT_STREQ("old", var.toStdString().c_str());
    m_cpu->setAttribute(map, "Missing2", var);
    EXPECT_STREQ("old", var.toStdString().c_str());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setAttribute_overwriteFalse)
{
    // Arrange: 含 Name 键的映射与已有值
    QMap<QString, QString> map;
    map.insert("Name", "NewValue");
    QString var = "Existing";

    // Act: 执行被测调用
    // Assert: overwrite=false 且 var 非空 => 不覆盖; var 为 Unknown 时仍被替换
    m_cpu->setAttribute(map, "Name", var, false);
    EXPECT_STREQ("Existing", var.toStdString().c_str());
    var = "Unknown";
    m_cpu->setAttribute(map, "Name", var, false);
    EXPECT_STREQ("NewValue", var.toStdString().c_str());
}

// ===================== nameTr =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_nameTr)
{
    // Arrange: 设置名称
    m_cpu->m_Name = "Intel";

    // Act: 执行被测调用
    // Assert: 无翻译文件时 tr 返回原文
    EXPECT_STREQ("Intel", m_cpu->nameTr().toStdString().c_str());
    m_cpu->m_Name = "AMD";
    EXPECT_STREQ("AMD", m_cpu->nameTr().toStdString().c_str());
}

// ===================== getBaseAttribs / getOtherAttribs（走 DeviceCpu 实现） =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getBaseAttribs)
{
    // Arrange: 默认构造
    // Act: 获取基本属性
    const auto &base = m_cpu->getBaseAttribs();

    // Assert: 空数据下为空列表
    EXPECT_TRUE(base.isEmpty());
    EXPECT_EQ(0, base.size());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getOtherAttribs)
{
    // Arrange: 默认构造
    // Act: 获取其他属性
    const auto &other = m_cpu->getOtherAttribs();

    // Assert: 空数据下为空列表
    EXPECT_TRUE(other.isEmpty());
    EXPECT_EQ(0, other.size());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getBaseTranslationAttribs)
{
    // Arrange: 默认构造
    // Act: 获取翻译属性
    // getBaseAttribs 会清空并按空数据加载 => 0 项，仍会执行翻译循环
    const auto &tr = m_cpu->getBaseTranslationAttribs();

    // Assert: 空数据下为空列表
    EXPECT_EQ(0, tr.size());
    EXPECT_TRUE(tr.isEmpty());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getOtherTranslationAttribs)
{
    // Arrange: 默认构造
    // Act: 获取翻译属性
    const auto &tr = m_cpu->getOtherTranslationAttribs();

    // Assert: 空数据下为空列表
    EXPECT_EQ(0, tr.size());
    EXPECT_TRUE(tr.isEmpty());
}

// ===================== getTableHeader / getTableData =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getTableHeader)
{
    // Arrange: 默认构造
    // Act: 获取表头
    const auto &header = m_cpu->getTableHeader();

    // Assert: 与内部表头一致且非空
    EXPECT_FALSE(header.isEmpty());
    EXPECT_EQ(m_cpu->m_TableHeader.size(), header.size());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getTableData)
{
    // Arrange: 默认构造
    // Act: 获取表数据
    const auto &data = m_cpu->getTableData();

    // Assert: 与内部表数据一致且非空
    EXPECT_FALSE(data.isEmpty());
    EXPECT_EQ(m_cpu->m_TableData.size(), data.size());
}

// ===================== toHtmlString / subTitleToHTML =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_toHtmlString)
{
    // Arrange: 填充基本属性
    m_cpu->addBaseDeviceInfo("Name", "Intel");
    m_cpu->getBaseAttribs();
    m_cpu->getBaseTranslationAttribs();

    // Act: 转 HTML
    QDomDocument doc;
    m_cpu->toHtmlString(doc);

    // Assert: 文档可用
    EXPECT_FALSE(doc.isNull());
    QDomDocument doc2;
    m_cpu->toHtmlString(doc2);
    EXPECT_FALSE(doc2.isNull());
}

TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_subTitleToHTML)
{
    // Arrange: 空文档(CPU 默认副标题非空)
    QDomDocument doc;

    // Act: 写入副标题
    m_cpu->subTitleToHTML(doc);

    // Assert: 文档可用且包含副标题内容
    EXPECT_FALSE(doc.isNull());
    EXPECT_TRUE(QString(doc.toByteArray()).contains(m_cpu->subTitle().trimmed()));
}

// ===================== readDeviceInfoKeyValue =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_readDeviceInfoKeyValue_emptyKey)
{
    // Arrange: 默认构造
    // Act: 执行被测调用
    // Assert: 空键与未设置键均返回空串
    EXPECT_STREQ("", m_cpu->readDeviceInfoKeyValue("").toStdString().c_str());
    EXPECT_TRUE(m_cpu->readDeviceInfoKeyValue("Vendor").isEmpty());
}

// ===================== getDriverVersion（调用 modinfo，空驱动返回空） =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getDriverVersion)
{
    // Arrange: 默认构造(驱动为空)
    // Act: 查询驱动版本
    // DeviceCpu::driver() 返回空 => modinfo 无输出 => 返回 ""
    QString v = m_cpu->getDriverVersion();

    // Assert: 空驱动返回空版本
    EXPECT_STREQ("", v.toStdString().c_str());
    EXPECT_TRUE(v.isEmpty());
}

// ===================== tableInfoToTxt / tableHeaderToTxt =====================
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_tableToTxt)
{
    // Arrange: 内存流与列宽
    QString out;
    QTextStream stream(&out);
    QList<double> widths;
    widths << 20 << 20;

    // Act: 写表头与表数据
    m_cpu->tableHeaderToTxt(stream, widths);
    m_cpu->tableInfoToTxt(stream, widths);
    stream.flush();

    // Assert: 输出非空
    EXPECT_FALSE(out.isEmpty());
    EXPECT_GT(out.size(), 0);
}

// ---------- 补充分级下限用例 ----------
// getVID/getPID/getVIDAndPID/getModalias: 新鲜设备默认空
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_VID_PID_defaults)
{
    // Arrange: 新鲜设备
    // Act: 执行被测调用
    // Assert: 四个 getter 均空
    EXPECT_TRUE(m_cpu->getVID().isEmpty());
    EXPECT_TRUE(m_cpu->getPID().isEmpty());
    EXPECT_TRUE(m_cpu->getVIDAndPID().isEmpty());
    EXPECT_TRUE(m_cpu->getModalias().isEmpty());
}

// isValueValid: 全部占位符无效
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_isValueValid_placeholders)
{
    // Arrange: 常见占位符列表
    QStringList ph = {"Unknown", "N/A", "Null", "none", "Not Provided", "Not Specified",
                      "Default string", "Unspecified", "Not Present", "<OUT OF SPEC>",
                      "Other", "TBD"};

    // Act: 执行被测调用
    // Assert: 全部无效
    for (const QString &p : ph)
        EXPECT_FALSE(m_cpu->isValueValid(p)) << p.toStdString();
    EXPECT_TRUE(m_cpu->isValueValid(" "));
}

// displayWidth: 空串为 0, CJK 宽于等长 ASCII
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_displayWidth_mixed)
{
    // Arrange: 新鲜设备
    // Act: 执行被测调用
    // Assert: 空串 0, CJK 1.5/字
    EXPECT_DOUBLE_EQ(0.0, DeviceBaseInfo::displayWidth(""));
    EXPECT_GT(DeviceBaseInfo::displayWidth("中"), DeviceBaseInfo::displayWidth("a"));
}

// setForcedDisplay: 置位后取消
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_setForcedDisplay_toggle)
{
    // Arrange: 新鲜设备
    // Act: 置位后取消
    m_cpu->setForcedDisplay(true);
    EXPECT_TRUE(m_cpu->m_forcedDisplay);
    m_cpu->setForcedDisplay(false);

    // Assert: 取消生效
    EXPECT_FALSE(m_cpu->m_forcedDisplay);
}

// addFilterKey: 注册后进入过滤表, 重复注册不重复
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_addFilterKey_dedupStable)
{
    // Arrange: 新鲜设备
    // Act: 重复注册同一键
    m_cpu->addFilterKey("Speed");
    int n1 = m_cpu->m_FilterKey.size();
    m_cpu->addFilterKey("Speed");

    // Assert: 已包含且不重复增长
    EXPECT_TRUE(m_cpu->m_FilterKey.contains("Speed"));
    EXPECT_EQ(n1, m_cpu->m_FilterKey.size());
}

// addOtherDeviceInfo: 头部插入且空值跳过
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_addOtherDeviceInfo_headInsert)
{
    // Arrange: 新鲜设备
    // Act: 先加有效项, 再加空值项
    m_cpu->addOtherDeviceInfo("OK", "OV");
    int afterFirst = m_cpu->m_LstOtherInfo.size();
    m_cpu->addOtherDeviceInfo("E", "");

    // Assert: 有效项在头部且空值被跳过
    EXPECT_STREQ("OK", m_cpu->m_LstOtherInfo.at(0).first.toStdString().c_str());
    EXPECT_EQ(afterFirst, m_cpu->m_LstOtherInfo.size());
}

// addBaseDeviceInfo: 两键均可查到
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_addBaseDeviceInfo_twoKeys)
{
    // Arrange: 记录初始长度
    int n = m_cpu->m_LstBaseInfo.size();

    // Act: 追加两键
    m_cpu->addBaseDeviceInfo("BK1", "BV1");
    m_cpu->addBaseDeviceInfo("BK2", "BV2");

    // Assert: 列表增长且两键均可查到
    EXPECT_EQ(n + 2, m_cpu->m_LstBaseInfo.size());
    bool found1 = false, found2 = false;
    for (const auto &p : m_cpu->m_LstBaseInfo) {
        if (p.first == "BK1") found1 = true;
        if (p.first == "BK2") found2 = true;
    }
    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);
}

// readDeviceInfoKeyValue: 新鲜设备返回空, 空键返回空
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_readDeviceInfoKeyValue_fresh)
{
    // Arrange: 新鲜设备
    // Act: 执行被测调用
    // Assert: 标准键/未知键/空键均空
    EXPECT_TRUE(m_cpu->readDeviceInfoKeyValue("Vendor").isEmpty());
    EXPECT_TRUE(m_cpu->readDeviceInfoKeyValue("NotExist_XYZ").isEmpty());
    EXPECT_TRUE(m_cpu->readDeviceInfoKeyValue("").isEmpty());
}

// getBaseAttribs: 新鲜设备空且两次采集一致
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getBaseAttribs_freshStable)
{
    // Arrange: 新鲜设备
    // Act: 两次采集
    auto l1 = m_cpu->getBaseAttribs();
    auto l2 = m_cpu->getBaseAttribs();

    // Assert: 空设备不生成基础属性且采集确定
    EXPECT_TRUE(l1.isEmpty());
    EXPECT_EQ(l1, l2);
}

// getOtherAttribs: 新鲜设备空且两次采集一致
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getOtherAttribs_freshStable)
{
    // Arrange: 新鲜设备
    // Act: 两次采集
    auto l1 = m_cpu->getOtherAttribs();
    auto l2 = m_cpu->getOtherAttribs();

    // Assert: 空设备不生成其他属性且采集确定
    EXPECT_TRUE(l1.isEmpty());
    EXPECT_EQ(l1, l2);
}

// getBaseTranslationAttribs: 新鲜设备空, 翻译列表镜像基础列表
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getBaseTranslationAttribs_freshStable)
{
    // Arrange: 新鲜设备
    // Act: 两次采集
    auto l1 = m_cpu->getBaseTranslationAttribs();
    auto l2 = m_cpu->getBaseTranslationAttribs();

    // Assert: 空设备翻译列表为空且采集确定
    EXPECT_TRUE(l1.isEmpty());
    EXPECT_EQ(l1, l2);
}

// getOtherTranslationAttribs: 新鲜设备空
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getOtherTranslationAttribs_freshStable)
{
    // Arrange: 新鲜设备
    // Act: 两次采集
    auto l1 = m_cpu->getOtherTranslationAttribs();
    auto l2 = m_cpu->getOtherTranslationAttribs();

    // Assert: 空设备翻译列表为空且采集确定
    EXPECT_TRUE(l1.isEmpty());
    EXPECT_EQ(l1, l2);
}

// getTableHeader: 首次调用加载表头且尾部为 yes/no 开关位
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getTableHeader_loadsAndAppendsSwitch)
{
    // Arrange: 新鲜设备
    // Act: 采集表头(两次)
    const QStringList &h1 = m_cpu->getTableHeader();
    const QStringList &h2 = m_cpu->getTableHeader();

    // Assert: 表头非空, 两次一致, 尾部为 yes/no
    EXPECT_FALSE(h1.isEmpty());
    EXPECT_EQ(h1, h2);
    EXPECT_TRUE(m_cpu->m_TableHeader.last() == "yes" || m_cpu->m_TableHeader.last() == "no");
}

// getTableData: 两次采集一致
TEST_F(UT_DeviceInfoBase, UT_DeviceInfoBase_getTableData_deterministic)
{
    // Arrange: 新鲜设备
    // Act: 两次采集
    const QStringList &d1 = m_cpu->getTableData();
    const QStringList &d2 = m_cpu->getTableData();

    // Assert: 采集确定
    EXPECT_EQ(d1, d2);
    EXPECT_EQ(d1.size(), m_cpu->m_TableData.size());
}
