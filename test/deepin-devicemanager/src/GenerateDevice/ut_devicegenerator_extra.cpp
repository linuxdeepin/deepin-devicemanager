// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// DeviceGenerator.cpp 文件级自由函数测试套件
// 覆盖：isValidLogicalName / isValidMAC / sortCpuInfoByLogicalID /
//       mergeSortCpuInfoByLogicalID（DeviceGenerator.cpp @ ut-t08 99700372）
// 说明：这四个函数为 DeviceGenerator.cpp 内的外部链接自由函数，未在任何头文件
//       声明，测试文件以 extern 原型直接链接（-fno-access-control 下同二进制内
//       符号可见）。
// 分支清单：
//   isValidLogicalName: 空串 false / 含 p2p(大小写不敏感) false /
//                       /sys/class/net/<name> 存在 true / 不存在 false
//   isValidMAC: 含 00:00:00:00:00:00 false / 含 ff:ff:ff:ff:ff:ff false /
//               空串 false / 冒号分隔 true / 连字符分隔 true /
//               段数不足 false / 非法字符 false
//   sortCpuInfoByLogicalID(4 参): 双行交换 / 已序保持 / 单元素立即返回
//   mergeSortCpuInfoByLogicalID(4 参): 8 行乱序稳定合并 / 空表不崩溃
// 外部依赖：isValidLogicalName 查询真实 /sys/class/net —— 本机 lo 恒存在，
//           wlan7 恒不存在，无额外设备环境依赖。
// ============================================================================

#include "ut_Head.h"

#include <QDir>
#include <QList>
#include <QMap>
#include <QString>

#include <gtest/gtest.h>

// ---- 被测自由函数原型（定义于 src/GenerateDevice/DeviceGenerator.cpp，无头文件声明）----
bool isValidLogicalName(const QString &logicalName);
bool isValidMAC(const QString &macAddress);
void sortCpuInfoByLogicalID(QList<QMap<QString, QString> > &lsCpu,
                            QList<QMap<QString, QString> > &tmpLst, int begin, int end);
void mergeSortCpuInfoByLogicalID(QList<QMap<QString, QString> > &lsCpu,
                                 QList<QMap<QString, QString> > &tmpLst, int begin, int end);

// ============================ 数据构造辅助 ============================

static QMap<QString, QString> ut_dgff_row(std::initializer_list<std::pair<const char *, const char *>> pairs)
{
    QMap<QString, QString> map;
    for (const auto &p : pairs)
        map.insert(QString(p.first), QString(p.second));
    return map;
}

// ============================ Fixture ============================

class UT_DeviceGeneratorFreeFunc : public UT_HEAD
{
public:
    void SetUp() { }
    void TearDown() { }
};

// ============================ isValidLogicalName ============================

// 空字符串 → false
TEST_F(UT_DeviceGeneratorFreeFunc, isValidLogicalName_empty_false)
{
    // Arrange/Act
    const bool result = isValidLogicalName(QString());
    // Assert
    EXPECT_FALSE(result);
}

// 含 p2p（大小写不敏感，任意位置）→ false
TEST_F(UT_DeviceGeneratorFreeFunc, isValidLogicalName_p2pPrefix_false)
{
    // Arrange/Act
    const bool result = isValidLogicalName(QString("p2p-dev-wlan0"));
    // Assert
    EXPECT_FALSE(result);
}

// 真实存在的逻辑名（lo 恒在 /sys/class/net 下）→ true
TEST_F(UT_DeviceGeneratorFreeFunc, isValidLogicalName_realName_true)
{
    // Arrange: 本机必有回环网卡
    // Act
    const bool result = isValidLogicalName(QString("lo"));
    // Assert
    EXPECT_TRUE(result);
}

// 不存在的逻辑名（/sys/class/net/wlan7 不存在）→ false
TEST_F(UT_DeviceGeneratorFreeFunc, isValidLogicalName_missing_false)
{
    // Arrange/Act
    const bool result = isValidLogicalName(QString("wlan7"));
    // Assert
    EXPECT_FALSE(result);
}

// ============================ isValidMAC ============================

// 全零 MAC：源码显式排除 → false
TEST_F(UT_DeviceGeneratorFreeFunc, isValidMAC_allZero_false)
{
    // Act/Assert
    EXPECT_FALSE(isValidMAC(QString("00:00:00:00:00:00")));
}

// 全 FF MAC：源码显式排除 → false
TEST_F(UT_DeviceGeneratorFreeFunc, isValidMAC_allFF_false)
{
    // Act/Assert
    EXPECT_FALSE(isValidMAC(QString("FF:FF:FF:FF:FF:FF")));
}

// 冒号分隔合法 MAC → true
TEST_F(UT_DeviceGeneratorFreeFunc, isValidMAC_colon_true)
{
    // Act/Assert
    EXPECT_TRUE(isValidMAC(QString("f4:b5:20:24:5e:4f")));
}

// 连字符分隔合法 MAC → true
TEST_F(UT_DeviceGeneratorFreeFunc, isValidMAC_dash_true)
{
    // Act/Assert
    EXPECT_TRUE(isValidMAC(QString("F4-B5-20-24-5E-4F")));
}

// 段数不足（5 段）→ 正则不匹配 → false
TEST_F(UT_DeviceGeneratorFreeFunc, isValidMAC_tooShort_false)
{
    // Act/Assert
    EXPECT_FALSE(isValidMAC(QString("f4:b5:20:24:5e")));
}

// 非法字符（gg）→ 正则不匹配 → false
TEST_F(UT_DeviceGeneratorFreeFunc, isValidMAC_badChar_false)
{
    // Act/Assert
    EXPECT_FALSE(isValidMAC(QString("f4:b5:20:24:5e:gg")));
}

// 空串 → false
TEST_F(UT_DeviceGeneratorFreeFunc, isValidMAC_empty_false)
{
    // Act/Assert
    EXPECT_FALSE(isValidMAC(QString()));
}

// ============================ sortCpuInfoByLogicalID ============================

// 双行乱序 → 按 processor 升序交换（结果同时写入 srcLst 与 tmpLst）
TEST_F(UT_DeviceGeneratorFreeFunc, sortCpuInfoByLogicalID_swapTwoRows)
{
    // Arrange: 与 generatorCpuDevice 相同的调用方式
    QList<QMap<QString, QString> > srcLst;
    srcLst << ut_dgff_row({ {"processor", "2"}, {"model name", "CPU-A"} })
           << ut_dgff_row({ {"processor", "0"}, {"model name", "CPU-B"} });
    QList<QMap<QString, QString> > tmpLst = srcLst;
    // Act
    sortCpuInfoByLogicalID(srcLst, tmpLst, 0, srcLst.size() - 1);
    // Assert: 交换结果写入 srcLst（双元素分支直接在 lsCpu 上交换，不同步 tmpLst）
    ASSERT_EQ(srcLst.size(), 2);
    EXPECT_EQ(srcLst[0].value("processor"), QString("0"));
    EXPECT_EQ(srcLst[1].value("processor"), QString("2"));
}

// 已序输入 → 顺序不变
TEST_F(UT_DeviceGeneratorFreeFunc, sortCpuInfoByLogicalID_alreadySorted)
{
    // Arrange
    QList<QMap<QString, QString> > srcLst;
    srcLst << ut_dgff_row({ {"processor", "0"} })
           << ut_dgff_row({ {"processor", "1"} });
    QList<QMap<QString, QString> > tmpLst = srcLst;
    // Act
    sortCpuInfoByLogicalID(srcLst, tmpLst, 0, srcLst.size() - 1);
    // Assert
    ASSERT_EQ(srcLst.size(), 2);
    EXPECT_EQ(srcLst[0].value("processor"), QString("0"));
    EXPECT_EQ(srcLst[1].value("processor"), QString("1"));
}

// 单元素（end == begin）→ 立即返回
TEST_F(UT_DeviceGeneratorFreeFunc, sortCpuInfoByLogicalID_single)
{
    // Arrange
    QList<QMap<QString, QString> > srcLst;
    srcLst << ut_dgff_row({ {"processor", "5"} });
    QList<QMap<QString, QString> > tmpLst = srcLst;
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(sortCpuInfoByLogicalID(srcLst, tmpLst, 0, 0));
    EXPECT_EQ(srcLst[0].value("processor"), QString("5"));
}

// ============================ mergeSortCpuInfoByLogicalID ============================

// 8 行乱序 → 经 sortCpuInfoByLogicalID 递归后 processor 0..7 升序
//（mergeSort 要求两半已序，生产入口为 sortCpuInfoByLogicalID，与 generatorCpuDevice 同调用方式）
TEST_F(UT_DeviceGeneratorFreeFunc, mergeSortCpuInfoByLogicalID_eightRows)
{
    // Arrange
    QList<QMap<QString, QString> > srcLst;
    const int order[8] = { 7, 3, 5, 1, 6, 0, 4, 2 };
    for (int i = 0; i < 8; ++i) {
        QMap<QString, QString> row;
        row.insert("processor", QString::number(order[i]));
        srcLst << row;
    }
    QList<QMap<QString, QString> > tmpLst = srcLst;
    // Act
    sortCpuInfoByLogicalID(srcLst, tmpLst, 0, srcLst.size() - 1);
    // Assert: 最终结果回写 srcLst
    ASSERT_EQ(srcLst.size(), 8);
    for (int i = 0; i < 8; ++i)
        EXPECT_EQ(srcLst[i].value("processor"), QString::number(i)) << "index " << i;
}

// 空表（begin=0, end=-1）→ 合并区间为空，不崩溃
TEST_F(UT_DeviceGeneratorFreeFunc, mergeSortCpuInfoByLogicalID_empty)
{
    // Arrange
    QList<QMap<QString, QString> > srcLst;
    QList<QMap<QString, QString> > tmpLst;
    // Act/Assert
    EXPECT_NO_FATAL_FAILURE(mergeSortCpuInfoByLogicalID(srcLst, tmpLst, 0, -1));
    EXPECT_TRUE(srcLst.isEmpty());
}

// 双行 → 合并后升序
TEST_F(UT_DeviceGeneratorFreeFunc, mergeSortCpuInfoByLogicalID_twoRows)
{
    // Arrange
    QList<QMap<QString, QString> > srcLst;
    srcLst << ut_dgff_row({ {"processor", "9"} })
           << ut_dgff_row({ {"processor", "3"} });
    QList<QMap<QString, QString> > tmpLst = srcLst;
    // Act
    mergeSortCpuInfoByLogicalID(srcLst, tmpLst, 0, 1);
    // Assert
    ASSERT_EQ(tmpLst.size(), 2);
    EXPECT_EQ(tmpLst[0].value("processor"), QString("3"));
    EXPECT_EQ(tmpLst[1].value("processor"), QString("9"));
}
