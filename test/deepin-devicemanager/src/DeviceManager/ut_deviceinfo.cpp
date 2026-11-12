// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DeviceBaseInfo 基础接口 (deepin-devicemanager/src/DeviceManager/DeviceInfo.cpp)
// 覆盖: getOtherAttribs/getBaseAttribs/getTableHeader/getTableData/subTitle/
//       isValueValid/setEnable/getOverviewInfo/getOtherMapInfo/
//       addBaseDeviceInfo/setAttribute/mapInfoToList。
// 载体: DeviceAudio(具体子类, 默认构造, 无外部依赖)。
//
// 用例计数声明（self-check-structural 验证此块）：
// | method               | level | factors | min | actual |
// |----------------------|-------|---------|-----|--------|
// | getOtherAttribs      | low   | -       | 1   | 1      |
// | getBaseAttribs       | low   | -       | 1   | 1      |
// | getTableHeader       | low   | -       | 1   | 1      |
// | getTableData         | low   | -       | 1   | 1      |
// | subTitle             | low   | -       | 1   | 1      |
// | isValueValid         | mid   | -       | 2   | 2      |
// | setEnable/canEnable  | low   | -       | 1   | 1      |
// | getOverviewInfo      | low   | -       | 1   | 1      |
// | getOtherMapInfo      | mid   | -       | 2   | 2      |
// | addBaseDeviceInfo    | low   | -       | 1   | 1      |
// | setAttribute         | mid   | -       | 2   | 3      |
// | mapInfoToList        | low   | -       | 1   | 1      |
// ============================================================

#include "DeviceInfo.h"
#include "DeviceAudio.h"

#include "ut_Head.h"

#include <QCoreApplication>

#include <gtest/gtest.h>

class UT_DeviceInfo : public ::testing::Test
{
public:
    void SetUp() override
    {
        ut_disable_system_dbus();  // 隔离真实系统服务, 避免运行期触发系统总线鉴权
        audio = new DeviceAudio;
    }
    void TearDown() override
    {
        delete audio;
        audio = nullptr;
    }
    DeviceBaseInfo *m_deviceBaseInfo = nullptr;
    DeviceAudio *audio = nullptr;
};

// getOtherAttribs: 默认设备无其他属性
TEST_F(UT_DeviceInfo, UT_DeviceInfo_getOtherAttribs)
{
    // Arrange: 默认构造的音频设备
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);

    // Act: 获取其他属性
    QList<QPair<QString, QString>> lst = m_deviceBaseInfo->getOtherAttribs();

    // Assert: 列表与内部数据均为空
    EXPECT_EQ(0, lst.size());
    EXPECT_EQ(0, m_deviceBaseInfo->m_LstOtherInfo.size());
}

// getBaseAttribs: 默认设备无基本属性
TEST_F(UT_DeviceInfo, UT_DeviceInfo_getBaseAttribs)
{
    // Arrange: 默认构造的音频设备
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);

    // Act: 获取基本属性
    QList<QPair<QString, QString>> lst = m_deviceBaseInfo->getBaseAttribs();

    // Assert: 列表与内部数据均为空
    EXPECT_EQ(0, lst.size());
    EXPECT_EQ(0, m_deviceBaseInfo->m_LstBaseInfo.size());
}

// getTableHeader: 返回内部表头
TEST_F(UT_DeviceInfo, UT_DeviceInfo_getTableHeader)
{
    // Arrange: 默认构造的音频设备
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);

    // Act: 获取表头
    QStringList lst = m_deviceBaseInfo->getTableHeader();

    // Assert: 表头与内部数据一致
    EXPECT_EQ(3, lst.size());
    EXPECT_EQ(3, m_deviceBaseInfo->m_TableHeader.size());
}

// getTableData: 返回内部表数据
TEST_F(UT_DeviceInfo, UT_DeviceInfo_getTableData)
{
    // Arrange: 默认构造的音频设备
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);

    // Act: 获取表数据
    QStringList lst = m_deviceBaseInfo->getTableData();

    // Assert: 表数据与内部数据一致
    EXPECT_EQ(2, lst.size());
    EXPECT_EQ(2, m_deviceBaseInfo->m_TableData.size());
}

// subTitle: 默认设备副标题为空
TEST_F(UT_DeviceInfo, UT_DeviceInfo_subTitle)
{
    // Arrange: 默认构造的音频设备
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);

    // Act: 获取副标题
    QString title = m_deviceBaseInfo->subTitle();

    // Assert: 未设置名称时副标题为空
    EXPECT_STREQ("", title.toStdString().c_str());
    EXPECT_TRUE(title.isEmpty());
}

// isValueValid: 空串与占位符均视为无效
TEST_F(UT_DeviceInfo, UT_DeviceInfo_isValueValid_invalid)
{
    // Arrange: 默认构造的音频设备
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);

    // Act: 执行被测调用
    // Assert: 空串与各占位符均无效
    EXPECT_FALSE(m_deviceBaseInfo->isValueValid(""));
    EXPECT_FALSE(m_deviceBaseInfo->isValueValid("Unknown"));
    EXPECT_FALSE(m_deviceBaseInfo->isValueValid("N/A"));
    EXPECT_FALSE(m_deviceBaseInfo->isValueValid("none"));
}

// isValueValid: 正常值有效
TEST_F(UT_DeviceInfo, UT_DeviceInfo_isValueValid_normal)
{
    // Arrange: 默认构造的音频设备
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);

    // Act: 执行被测调用
    // Assert: 普通值有效
    EXPECT_TRUE(m_deviceBaseInfo->isValueValid("Intel"));
    EXPECT_TRUE(m_deviceBaseInfo->isValueValid("snd_hda_intel"));
}

// setEnable/canEnable: 设置后硬件类别与使能开关生效
TEST_F(UT_DeviceInfo, UT_DeviceInfo_setEnable)
{
    // Arrange: 默认构造的音频设备(UniqueID 为空, DBus 不会被调用)
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);

    // Act: 使能开关与启用请求
    m_deviceBaseInfo->setCanEnable(true);
    m_deviceBaseInfo->setEnable(true);

    // Assert: canEnable 置位; setEnable 走空 UniqueID 失败路径但仍标记硬件类别
    EXPECT_TRUE(m_deviceBaseInfo->canEnable());
    EXPECT_STREQ("sound", audio->m_HardwareClass.toStdString().c_str());
}

// getOverviewInfo: 默认设备概览为空
TEST_F(UT_DeviceInfo, UT_DeviceInfo_getOverviewInfo)
{
    // Arrange: 默认构造的音频设备
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);

    // Act: 获取概览信息
    QString ret = m_deviceBaseInfo->getOverviewInfo();

    // Assert: 未注入数据时概览为空
    EXPECT_STREQ("", ret.toStdString().c_str());
    EXPECT_TRUE(ret.isEmpty());
}

// getOtherMapInfo: 过滤键外的属性被忽略
TEST_F(UT_DeviceInfo, UT_DeviceInfo_getOtherMapInfo_ignoredKey)
{
    // Arrange: 仅含非过滤键的映射
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "1@n");

    // Act: 提取其他属性
    m_deviceBaseInfo->getOtherMapInfo(mapinfo);

    // Assert: 非过滤键不进入其他属性表
    EXPECT_EQ(0, m_deviceBaseInfo->m_LstOtherInfo.size());
    EXPECT_EQ(0, m_deviceBaseInfo->m_MapOtherInfo.size());
}

// getOtherMapInfo: 过滤键 Version 的值被收录
TEST_F(UT_DeviceInfo, UT_DeviceInfo_getOtherMapInfo_keepsFilterKey)
{
    // Arrange: 含过滤键 Version 的映射
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Version", "1.0 ");

    // Act: 提取其他属性
    m_deviceBaseInfo->getOtherMapInfo(mapinfo);

    // Assert: 过滤键值已收录并去空白
    EXPECT_EQ(1, m_deviceBaseInfo->m_MapOtherInfo.size());
    EXPECT_STREQ("1.0", m_deviceBaseInfo->m_MapOtherInfo.value("Version").toStdString().c_str());
}

// addBaseDeviceInfo: 追加后基本属性表出现新键值
TEST_F(UT_DeviceInfo, UT_DeviceInfo_addBaseDeviceInfo)
{
    // Arrange: 默认构造的音频设备
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);

    // Act: 追加基本属性
    m_deviceBaseInfo->addBaseDeviceInfo("name", "abc@123");

    // Assert: 表长度为 1 且键值正确
    EXPECT_EQ(1, m_deviceBaseInfo->m_LstBaseInfo.size());
    EXPECT_STREQ("abc@123", m_deviceBaseInfo->m_LstBaseInfo.at(0).second.toStdString().c_str());
}

// setAttribute: overwrite=true 直接覆盖
TEST_F(UT_DeviceInfo, UT_DeviceInfo_setAttribute_overwrite)
{
    // Arrange: 含 name 键的映射
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "xyz ");
    QString value = "abc@123";

    // Act: 覆盖写入
    m_deviceBaseInfo->setAttribute(mapinfo, "name", value, true);

    // Assert: 值被覆盖并去空白; 空变量覆盖写入同样生效
    EXPECT_STREQ("xyz", value.toStdString().c_str());
    QString empty;
    m_deviceBaseInfo->setAttribute(mapinfo, "name", empty, true);
    EXPECT_STREQ("xyz", empty.toStdString().c_str());
}

// setAttribute: overwrite=false 时空值或 Unknown 才覆盖
TEST_F(UT_DeviceInfo, UT_DeviceInfo_setAttribute_keepIfSet)
{
    // Arrange: 已有值的变量与映射
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("name", "newval");
    QString value = "oldval";

    // Act: 执行被测调用
    // Assert: 已有值时 overwrite=false 不覆盖
    m_deviceBaseInfo->setAttribute(mapinfo, "name", value, false);
    EXPECT_STREQ("oldval", value.toStdString().c_str());

    // Act: 执行被测调用
    // Assert: Unknown 值时 overwrite=false 覆盖
    value = "Unknown";
    m_deviceBaseInfo->setAttribute(mapinfo, "name", value, false);
    EXPECT_STREQ("newval", value.toStdString().c_str());

    // Act: 执行被测调用
    // Assert: 键不存在时不变
    m_deviceBaseInfo->setAttribute(mapinfo, "missing", value, true);
    EXPECT_STREQ("newval", value.toStdString().c_str());
}

// mapInfoToList: 其他属性映射转换为键值列表
TEST_F(UT_DeviceInfo, UT_DeviceInfo_mapInfoToList)
{
    // Arrange: 填充其他属性映射
    m_deviceBaseInfo = dynamic_cast<DeviceBaseInfo *>(audio);
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Version", "1.0");
    m_deviceBaseInfo->m_MapOtherInfo = mapinfo;

    // Act: 转换为列表
    m_deviceBaseInfo->mapInfoToList();

    // Assert: 列表与映射一致
    EXPECT_EQ(1, m_deviceBaseInfo->m_LstOtherInfo.size());
    EXPECT_STREQ("1.0", m_deviceBaseInfo->m_LstOtherInfo.at(0).second.toStdString().c_str());
}
