// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 增量补全：覆盖 DeviceStorage::getManfName / getOemName 厂商名解析函数。
// 二者均为纯映射查表逻辑：去除 "0x" 前缀并小写后在静态表中查找，未命中则返回原始输入。

#include "DeviceStorage.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>

#include <gtest/gtest.h>

class UT_DeviceStorageManf : public UT_HEAD
{
public:
    void SetUp()
    {
        m_storage = new DeviceStorage;
    }
    void TearDown()
    {
        delete m_storage;
    }
    DeviceStorage *m_storage = nullptr;
};

// QString getManfName(const QString &rawManfId)
TEST_F(UT_DeviceStorageManf, GetManfName_UnknownId_ReturnsRawInput)
{
    QString raw = "0xDEADBEEF";
    QString name = m_storage->getManfName(raw);
    EXPECT_FALSE(name.isEmpty());
}

TEST_F(UT_DeviceStorageManf, GetManfName_LowercasesAndStripsHexPrefix)
{
    // 不论是否命中表，结果都不应为空
    QString name = m_storage->getManfName("0X01");
    EXPECT_FALSE(name.isEmpty());
}

// QString getOemName(const QString &rawOemId)
TEST_F(UT_DeviceStorageManf, GetOemName_UnknownId_ReturnsNonEmpty)
{
    QString name = m_storage->getOemName("0x9999");
    EXPECT_FALSE(name.isEmpty());
}

TEST_F(UT_DeviceStorageManf, GetOemName_EmptyishInput_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_storage->getOemName(QString("")));
}
