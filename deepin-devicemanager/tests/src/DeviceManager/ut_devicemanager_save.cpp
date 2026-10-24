// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DeviceManager 文件写入链 (deepin-devicemanager/src/DeviceManager/DeviceManager.cpp)
// 覆盖: exportToTxt/exportToXlsx/exportToHtml、overviewToTxt/overviewToHtml/
//       overviewToXlsx、addOthersDeviceFromHwinfo。
// 环境隔离: 导出路径一律使用 QTemporaryDir/QTemporaryFile, 不写系统目录。
// 注: Doc 格式因 Docx::Document 默认构造 SEGV 暂跳过。
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                     | level | factors | min | actual |
// |----------------------------|-------|---------|-----|--------|
// | exportToTxt                | mid   | -       | 2   | 2      |
// | exportToXlsx               | low   | -       | 1   | 1      |
// | exportToHtml               | low   | -       | 1   | 1      |
// | overviewToTxt              | low   | -       | 1   | 1      |
// | overviewToHtml             | low   | -       | 1   | 1      |
// | overviewToXlsx             | low   | -       | 1   | 1      |
// | addOthersDeviceFromHwinfo  | low   | -       | 1   | 2      |
// ============================================================

#include "DeviceManager.h"
#include "DeviceOthers.h"
#include "xlsxdocument.h"
#include "ut_Head.h"

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include <gtest/gtest.h>

class UT_DeviceManagerSave : public ::testing::Test
{
public:
    void SetUp() override
    {
        ut_disable_system_dbus();  // 隔离真实系统服务, 避免运行期触发系统总线鉴权
        DeviceManager::instance()->clear();
        tmpDir = new QTemporaryDir;
    }
    void TearDown() override
    {
        DeviceManager::instance()->clear();
        delete tmpDir;
        tmpDir = nullptr;
    }
    QTemporaryDir *tmpDir = nullptr;
};

// exportToTxt: 空设备列表写入有效文件(含表头); 不可写路径返回 false
TEST_F(UT_DeviceManagerSave, UT_DeviceManagerSave_exportToTxt_valid)
{
    // Arrange: 临时目录中的输出路径
    QString path = tmpDir->filePath("ut_dm_out.txt");

    // Act: 导出 txt
    bool ok = DeviceManager::instance()->exportToTxt(path);

    // Assert: 导出成功且文件非空
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QFile(path).exists());
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    EXPECT_GT(f.size(), qint64(0));
}

TEST_F(UT_DeviceManagerSave, UT_DeviceManagerSave_exportToTxt_badPath)
{
    // Arrange: 指向不存在目录的路径
    // Act: 导出 txt 到不可写路径
    bool ok = DeviceManager::instance()->exportToTxt(tmpDir->filePath("no_such_dir/x.txt"));

    // Assert: 打开失败返回 false, 且未产生文件
    EXPECT_FALSE(ok);
    EXPECT_FALSE(QFile(tmpDir->filePath("no_such_dir/x.txt")).exists());
}

// exportToXlsx: 空设备列表生成有效 xlsx 文件
TEST_F(UT_DeviceManagerSave, UT_DeviceManagerSave_exportToXlsx)
{
    // Arrange: 临时目录中的输出路径
    QString path = tmpDir->filePath("ut_dm_out.xlsx");

    // Act: 导出 xlsx
    bool ok = DeviceManager::instance()->exportToXlsx(path);

    // Assert: 导出成功且文件存在
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QFile(path).exists());
}

// exportToHtml: 空设备列表生成非空 html 文件
TEST_F(UT_DeviceManagerSave, UT_DeviceManagerSave_exportToHtml)
{
    // Arrange: 临时目录中的输出路径
    QString path = tmpDir->filePath("ut_dm_out.html");

    // Act: 导出 html
    bool ok = DeviceManager::instance()->exportToHtml(path);

    // Assert: 导出成功且文件非空
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QFile(path).exists());
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    EXPECT_GT(f.size(), qint64(0));
}

// overviewToTxt: 概览写入流, 含 Overview 段头
TEST_F(UT_DeviceManagerSave, UT_DeviceManagerSave_overviewToTxt)
{
    // Arrange: 内存流
    QString out;
    QTextStream ts(&out);

    // Act: 写入概览
    DeviceManager::instance()->overviewToTxt(ts);
    ts.flush();

    // Assert: 流含 Overview 段头且非空
    EXPECT_TRUE(out.contains("Overview"));
    EXPECT_FALSE(out.isEmpty());
}

// overviewToHtml: 概览写入临时 html 文件
TEST_F(UT_DeviceManagerSave, UT_DeviceManagerSave_overviewToHtml)
{
    // Arrange: 临时文件
    QTemporaryFile tmp;
    tmp.open();

    // Act: 写入概览
    DeviceManager::instance()->overviewToHtml(tmp);
    tmp.flush();

    // Assert: 文件有内容且含 Overview 标题段
    EXPECT_GT(tmp.size(), qint64(0));
    tmp.close();
    QFile back(tmp.fileName());
    ASSERT_TRUE(back.open(QIODevice::ReadOnly));
    QString html = QString::fromUtf8(back.readAll());
    back.close();
    EXPECT_TRUE(html.contains("Overview"));
}

// overviewToXlsx: 概览写入 xlsx 文档对象
TEST_F(UT_DeviceManagerSave, UT_DeviceManagerSave_overviewToXlsx)
{
    // Arrange: xlsx 文档对象
    QXlsx::Document xlsx;
    QXlsx::Format fmt;

    // Act: 写入概览
    DeviceManager::instance()->overviewToXlsx(xlsx, fmt);

    // Assert: 工作表可用且有行写入
    EXPECT_TRUE(xlsx.selectSheet("Sheet1"));
    EXPECT_GT(xlsx.dimension().lastRow(), 0);
}

// addOthersDeviceFromHwinfo: busInfo 为空的新设备被追加; busInfo 相同的设备被去重
TEST_F(UT_DeviceManagerSave, UT_DeviceManagerSave_addOthersDeviceFromHwinfo_append)
{
    // Arrange: 空的其他设备列表与新设备
    auto dm = DeviceManager::instance();
    DeviceOthers *d = new DeviceOthers();

    // Act: 从 hwinfo 追加设备
    dm->addOthersDeviceFromHwinfo(d);

    // Assert: 设备被追加且唯一
    EXPECT_EQ(1, dm->m_ListDeviceOthers.size());
    EXPECT_EQ(d, dm->m_ListDeviceOthers.at(0));
}

TEST_F(UT_DeviceManagerSave, UT_DeviceManagerSave_addOthersDeviceFromHwinfo_dedup)
{
    // Arrange: 已存在相同 busInfo 的设备
    auto dm = DeviceManager::instance();
    DeviceOthers *first = new DeviceOthers;
    first->m_BusInfo = "usb@1:2";
    dm->m_ListDeviceOthers.append(first);
    DeviceOthers *dup = new DeviceOthers;
    dup->m_BusInfo = "usb@1:2";

    // Act: 追加重复 busInfo 设备
    dm->addOthersDeviceFromHwinfo(dup);

    // Assert: 列表仍只有原设备, 重复者未加入
    EXPECT_EQ(1, dm->m_ListDeviceOthers.size());
    EXPECT_EQ(first, dm->m_ListDeviceOthers.at(0));
    delete dup;
}
