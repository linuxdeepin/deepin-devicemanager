// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DeviceManager 磁盘链 (deepin-devicemanager/src/DeviceManager/DeviceManager.cpp)
// 覆盖: mergeDisk/checkDiskSize/orderDiskByType、setStorageDeviceMediaType、
//       PhysID(纯解析)、tomlDeviceSet(toml 注入链)。
// 注意: mergeDisk/tomlDeviceSet 拥有被删除设备的所有权, 测试后用 clear() 统一回收。
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                    | level | factors | min | actual |
// |---------------------------|-------|---------|-----|--------|
// | mergeDisk                 | mid   | -       | 2   | 2      |
// | checkDiskSize             | low   | -       | 1   | 1      |
// | orderDiskByType           | low   | -       | 1   | 1      |
// | setStorageDeviceMediaType | mid   | -       | 2   | 2      |
// | PhysID                    | low   | -       | 1   | 1      |
// | tomlDeviceSet             | mid   | -       | 2   | 2      |
// ============================================================

#include "DeviceManager.h"
#include "GenerateDevicePool.h"
#include "DeviceStorage.h"
#include "ut_Head.h"

#include <QMap>
#include <QString>

#include <gtest/gtest.h>

class UT_DeviceManagerDisk : public ::testing::Test
{
public:
    void SetUp() override
    {
        ut_disable_system_dbus();  // 隔离真实系统服务, 避免运行期触发系统总线鉴权
        DeviceManager::instance()->clear();
    }
    void TearDown() override
    {
        // clear() 会 delete 所有 m_ListDeviceStorage 中的指针(mergeDisk 后剩余的也由它回收)
        DeviceManager::instance()->clear();
    }
};

// mergeDisk: serialID 为空时不合并, 列表保持不变
TEST_F(UT_DeviceManagerDisk, UT_DeviceManagerDisk_mergeDisk_noMerge)
{
    // Arrange: 两个无 serialID 的存储设备
    auto dm = DeviceManager::instance();
    dm->addStorageDeivce(new DeviceStorage);
    dm->addStorageDeivce(new DeviceStorage);

    // Act: 执行磁盘合并
    dm->mergeDisk();

    // Assert: serialID 均为空不触发合并, 两个设备均保留
    EXPECT_EQ(dm->m_ListDeviceStorage.size(), 2);
    EXPECT_NE(dm->m_ListDeviceStorage.at(0), dm->m_ListDeviceStorage.at(1));
}

// mergeDisk: 空列表安全完成
TEST_F(UT_DeviceManagerDisk, UT_DeviceManagerDisk_mergeDisk_empty)
{
    // Arrange: 空存储设备列表
    auto dm = DeviceManager::instance();

    // Act: 执行磁盘合并
    dm->mergeDisk();

    // Assert: 空列表遍历安全, 长度仍为 0
    EXPECT_EQ(dm->m_ListDeviceStorage.size(), 0);
    EXPECT_TRUE(dm->m_ListDeviceStorage.isEmpty());
}

// checkDiskSize: 非 HW 平台直接返回, 默认设备容量字段保持不变
TEST_F(UT_DeviceManagerDisk, UT_DeviceManagerDisk_checkDiskSize)
{
    // Arrange: 加入一个默认构造的存储设备(容量未识别)
    auto dm = DeviceManager::instance();
    dm->addStorageDeivce(new DeviceStorage);

    // Act: 执行容量校正
    dm->checkDiskSize();

    // Assert: 无容量数据时不误改字段, 设备保留在列表
    EXPECT_EQ(dm->m_ListDeviceStorage.size(), 1);
    EXPECT_TRUE(static_cast<DeviceStorage *>(dm->m_ListDeviceStorage.at(0))->m_Size.isEmpty());
}

// orderDiskByType: 多磁盘排序安全(触发 compareDevices 各分支)
TEST_F(UT_DeviceManagerDisk, UT_DeviceManagerDisk_orderDiskByType)
{
    // Arrange: 三个默认存储设备
    auto dm = DeviceManager::instance();
    dm->addStorageDeivce(new DeviceStorage);
    dm->addStorageDeivce(new DeviceStorage);
    dm->addStorageDeivce(new DeviceStorage);

    // Act: 执行类型排序
    dm->orderDiskByType();

    // Assert: 排序不增删设备, 数量保持 3
    EXPECT_EQ(dm->m_ListDeviceStorage.size(), 3);
    EXPECT_FALSE(dm->m_ListDeviceStorage.isEmpty());
}

// setStorageDeviceMediaType: 命中设备设置介质类型; 未命中返回 false
TEST_F(UT_DeviceManagerDisk, UT_DeviceManagerDisk_setStorageDeviceMediaType_hit)
{
    // Arrange: 存储设备文件为 /dev/sda
    auto dm = DeviceManager::instance();
    DeviceStorage *st = new DeviceStorage;
    st->m_DeviceFile = "/dev/sda";
    dm->addStorageDeivce(st);

    // Act: 设置 sda 介质类型
    bool r = dm->setStorageDeviceMediaType("sda", "1");

    // Assert: 命中返回 true, 介质类型已写入
    EXPECT_TRUE(r);
    EXPECT_STREQ("HDD", st->m_MediaType.toStdString().c_str());
}

TEST_F(UT_DeviceManagerDisk, UT_DeviceManagerDisk_setStorageDeviceMediaType_miss)
{
    // Arrange: 存储设备文件为 /dev/sda
    auto dm = DeviceManager::instance();
    DeviceStorage *st = new DeviceStorage;
    st->m_DeviceFile = "/dev/sda";
    dm->addStorageDeivce(st);

    // Act: 查询不存在的设备名
    bool r = dm->setStorageDeviceMediaType("nonexist", "HDD");

    // Assert: 未命中返回 false, 原设备介质类型未被修改
    EXPECT_FALSE(r);
    EXPECT_TRUE(st->m_MediaType.isEmpty());
}

// PhysID: 纯解析(toml key 处理, 去掉 _nouse 后缀); key 不存在与空 value 均返回空串
TEST_F(UT_DeviceManagerDisk, UT_DeviceManagerDisk_PhysID)
{
    // Arrange: 构造含 _nouse 后缀与空值的映射
    auto dm = DeviceManager::instance();
    QMap<QString, QString> m;
    m["Vendor_ID"] = "0x1234_nouse";
    m["EmptyKey"] = "";

    // Act: 执行被测调用
    // Assert: 正常 key 解析出去除 _nouse 的值
    QString r = dm->PhysID(m, "Vendor_ID");
    EXPECT_TRUE(r.contains("1234"));
    EXPECT_FALSE(r.contains("_nouse"));

    // Assert: key 不存在或 value 为空均返回空串
    EXPECT_EQ(dm->PhysID(m, "NotExist"), QString(""));
    EXPECT_EQ(dm->PhysID(m, "EmptyKey"), QString(""));
}

// tomlDeviceSet: adjust 需求 + tomlmatchkey=Modalias 命中, 触发 toml 解析链且不删设备
TEST_F(UT_DeviceManagerDisk, UT_DeviceManagerDisk_tomlDeviceSet_adjust)
{
    // Arrange: 注入 storage 设备与 tomlStorage 命令缓存(adjust 需求)
    auto dm = DeviceManager::instance();
    DeviceStorage *st = new DeviceStorage();
    st->m_Modalias = "pci:v00001234d00005678";
    dm->addStorageDeivce(st);

    QMap<QString, QString> tomlEntry;
    tomlEntry["tomlmatchkey"] = "Modalias";
    tomlEntry["tomlconfigdemanding"] = "adjust";
    tomlEntry["Modalias"] = "pci:v00001234d00005678";
    tomlEntry["Vendor_ID"] = "0x1234";
    tomlEntry["Product_ID"] = "0x5678";
    QMap<QString, QList<QMap<QString, QString> > > cmd;
    cmd["tomlStorage"].append(tomlEntry);
    dm->addCmdInfo(cmd);

    // Act: 执行 toml 注入
    dm->tomlDeviceSet(DT_Storage);

    // Assert: adjust 不删除设备, 设备仍在列表且 modalias 保持
    EXPECT_EQ(dm->m_ListDeviceStorage.size(), 1);
    EXPECT_EQ(dm->m_ListDeviceStorage.at(0), st);
    EXPECT_STREQ("pci:v00001234d00005678", st->m_Modalias.toStdString().c_str());
}

// tomlDeviceSet: 无 toml 数据时安全完成, 设备不受影响
TEST_F(UT_DeviceManagerDisk, UT_DeviceManagerDisk_tomlDeviceSet_empty)
{
    // Arrange: 仅注入设备, 无 toml 缓存
    auto dm = DeviceManager::instance();
    dm->addStorageDeivce(new DeviceStorage);

    // Act: 执行 toml 注入
    dm->tomlDeviceSet(DT_Storage);

    // Assert: 无数据时遍历安全, 设备保留
    EXPECT_EQ(dm->m_ListDeviceStorage.size(), 1);
    EXPECT_NE(dm->m_ListDeviceStorage.at(0), nullptr);
}
