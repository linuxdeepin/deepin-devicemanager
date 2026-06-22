// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceManager 的磁盘整理链(mergeDisk/checkDiskSize/orderDiskByType)、
// setStorageDeviceMediaType、PhysID(纯解析)、tomlDeviceSet(toml 注入链)。
// 注意: mergeDisk/tomlDeviceSet 拥有被删除设备的所有权,测试后用 clear() 统一回收。

#include "DeviceManager.h"
#include "GenerateDevicePool.h"
#include "DeviceStorage.h"
#include "ut_Head.h"

#include <QMap>
#include <QString>

#include <gtest/gtest.h>

class UT_DeviceManagerDisk : public UT_HEAD
{
public:
    void SetUp() override
    {
        DeviceManager::instance()->clear();
    }
    void TearDown() override
    {
        // clear() 会 delete 所有 m_ListDeviceStorage 中的指针(mergeDisk 后剩余的也由它回收)
        DeviceManager::instance()->clear();
    }
};

// mergeDisk: serialID 为空时不合并,但遍历调 unitConvertByDecimal
TEST_F(UT_DeviceManagerDisk, mergeDisk_noMerge)
{
    auto dm = DeviceManager::instance();
    dm->addStorageDeivce(new DeviceStorage);
    dm->addStorageDeivce(new DeviceStorage);
    EXPECT_NO_FATAL_FAILURE(dm->mergeDisk());
    EXPECT_EQ(dm->m_ListDeviceStorage.size(), 2);
}

// mergeDisk: 空列表不崩
TEST_F(UT_DeviceManagerDisk, mergeDisk_empty)
{
    auto dm = DeviceManager::instance();
    EXPECT_NO_FATAL_FAILURE(dm->mergeDisk());
    EXPECT_EQ(dm->m_ListDeviceStorage.size(), 0);
}

// checkDiskSize: 遍历调 device->checkDiskSize()
TEST_F(UT_DeviceManagerDisk, checkDiskSize)
{
    auto dm = DeviceManager::instance();
    dm->addStorageDeivce(new DeviceStorage);
    EXPECT_NO_FATAL_FAILURE(dm->checkDiskSize());
}

// orderDiskByType: 排序(多个磁盘触发 compareDevices 各分支)
TEST_F(UT_DeviceManagerDisk, orderDiskByType)
{
    auto dm = DeviceManager::instance();
    dm->addStorageDeivce(new DeviceStorage);
    dm->addStorageDeivce(new DeviceStorage);
    dm->addStorageDeivce(new DeviceStorage);
    EXPECT_NO_FATAL_FAILURE(dm->orderDiskByType());
}

// setStorageDeviceMediaType: 遍历设置
TEST_F(UT_DeviceManagerDisk, setStorageDeviceMediaType)
{
    auto dm = DeviceManager::instance();
    dm->addStorageDeivce(new DeviceStorage);
    bool r = dm->setStorageDeviceMediaType("sda", "SSD");
    EXPECT_NO_FATAL_FAILURE(dm->setStorageDeviceMediaType("nonexist", "HDD"));
    (void)r;
}

// PhysID: 纯解析(toml key 处理,去掉 _nouse)
TEST_F(UT_DeviceManagerDisk, PhysID)
{
    auto dm = DeviceManager::instance();
    QMap<QString, QString> m;
    m["Vendor_ID"] = "0x1234_nouse";
    QString r = dm->PhysID(m, "Vendor_ID");
    EXPECT_TRUE(r.contains("1234"));
    EXPECT_FALSE(r.contains("_nouse"));

    // key 不存在
    EXPECT_EQ(dm->PhysID(m, "NotExist"), QString(""));

    // 空 value
    m["EmptyKey"] = "";
    EXPECT_EQ(dm->PhysID(m, "EmptyKey"), QString(""));
}

// tomlDeviceSet: 注入 toml 数据(adjust) + 设备,触发 toml 解析链
TEST_F(UT_DeviceManagerDisk, tomlDeviceSet_adjust)
{
    auto dm = DeviceManager::instance();

    // 注入一个 storage 设备
    DeviceStorage *st = new DeviceStorage();
    st->m_Modalias = "pci:v00001234d00005678";
    dm->addStorageDeivce(st);

    // 注入 toml 数据
    QMap<QString, QString> tomlEntry;
    tomlEntry["tomlmatchkey"] = "Modalias";
    tomlEntry["tomlconfigdemanding"] = "adjust";
    tomlEntry["Modalias"] = "pci:v00001234d00005678";
    tomlEntry["Vendor_ID"] = "0x1234";
    tomlEntry["Product_ID"] = "0x5678";
    QMap<QString, QList<QMap<QString, QString> > > cmd;
    cmd["tomlStorage"].append(tomlEntry);
    dm->addCmdInfo(cmd);

    EXPECT_NO_FATAL_FAILURE(dm->tomlDeviceSet(DT_Storage));
}

// tomlDeviceSet: 无 toml 数据时安全完成
TEST_F(UT_DeviceManagerDisk, tomlDeviceSet_empty)
{
    auto dm = DeviceManager::instance();
    dm->addStorageDeivce(new DeviceStorage);
    EXPECT_NO_FATAL_FAILURE(dm->tomlDeviceSet(DT_Storage));
}
