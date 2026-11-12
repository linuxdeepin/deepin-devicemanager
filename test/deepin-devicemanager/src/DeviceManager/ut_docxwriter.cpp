// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DocxWriter 零依赖 docx 写入器 (deepin-devicemanager/src/DeviceManager/DocxWriter.cpp)
// 覆盖: addHeading/addParagraph/addTable/cell/addText/save、ZIP 结构、
//       XML 转义、越界行为、空文档。
// 断言模式: DocxWriter 无读回 API, 统一 save 落盘到 QTemporaryDir,
//           用 utExtractStoredEntry 提取 ZIP 条目校验 XML 内容。
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                          | level | factors | min | actual |
// |---------------------------------|-------|---------|-----|--------|
// | addHeading                      | mid   | -       | 2   | 2      |
// | addParagraph                    | mid   | -       | 2   | 2      |
// | addTable/Table::addRow/addColumn| mid   | -       | 3   | 4      |
// | Table::cell/Cell::addText       | mid   | -       | 2   | 3      |
// | save                            | mid   | -       | 2   | 3      |
// | DocxWriter::DocxWriter          | low   | -       | 1   | 1      |
// ============================================================

#include "DocxWriter.h"
#include "minizip_read_helper.h"

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QTemporaryDir>

#include <gtest/gtest.h>

// 提取落盘 docx 中的 ZIP 条目内容
static QByteArray utDocxEntryOf(DocxWriter &writer, const QTemporaryDir &dir,
                                const QString &name)
{
    const QString path = dir.filePath("ut_docx.docx");
    if (!writer.save(path))
        return QByteArray();
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    return utExtractStoredEntry(f.readAll(), name);
}

// ---------- addHeading ----------
// addHeading: 默认级别为 Heading1
TEST(UT_DocxWriter, UT_DocxWriter_addHeading_defaultLevel1)
{
    // Arrange: 写入器与临时目录
    DocxWriter writer;
    QTemporaryDir tmpDir;

    // Act: 添加默认级别标题并落盘
    writer.addHeading("概况");
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");

    // Assert: pPr 在 run 之前, 样式为 Heading1, 文本存在
    const QString xml = QString::fromUtf8(doc);
    EXPECT_TRUE(xml.contains("<w:pStyle w:val=\"Heading1\"/>"));
    EXPECT_TRUE(xml.contains("概况"));
    const int pPrPos = xml.indexOf("<w:pPr>");
    const int runPos = xml.indexOf("<w:r>");
    EXPECT_GE(pPrPos, 0);
    EXPECT_GT(runPos, pPrPos);
}

// addHeading: 级别 2 映射 Heading2
TEST(UT_DocxWriter, UT_DocxWriter_addHeading_level2)
{
    // Arrange: 写入器与临时目录
    DocxWriter writer;
    QTemporaryDir tmpDir;

    // Act: 添加二级标题并落盘
    writer.addHeading("CPU", 2);
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");

    // Assert: 样式为 Heading2
    EXPECT_TRUE(QString::fromUtf8(doc).contains("<w:pStyle w:val=\"Heading2\"/>"));
}

// ---------- addParagraph ----------
// addParagraph: 普通段落带文本
TEST(UT_DocxWriter, UT_DocxWriter_addParagraph_withText)
{
    // Arrange: 写入器与临时目录
    DocxWriter writer;
    QTemporaryDir tmpDir;

    // Act: 添加段落并落盘
    writer.addParagraph("设备:  UOS-PC");
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");

    // Assert: 文本存在且不附加样式（Normal 为默认样式, 无 pStyle）
    const QString xml = QString::fromUtf8(doc);
    EXPECT_TRUE(xml.contains("设备:  UOS-PC"));
    EXPECT_FALSE(xml.contains("<w:pStyle"));
}

// addParagraph: 特殊字符转义
TEST(UT_DocxWriter, UT_DocxWriter_addParagraph_escapesXml)
{
    // Arrange: 含 XML 特殊字符的文本
    DocxWriter writer;
    QTemporaryDir tmpDir;

    // Act: 添加段落并落盘
    writer.addParagraph("a<b>c&d\"e'f\t制表\n换行");
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");

    // Assert: 转义后内容正确
    const QString xml = QString::fromUtf8(doc);
    EXPECT_TRUE(xml.contains("a&lt;b&gt;c&amp;d&quot;e&apos;f\t制表\n换行"));
}

// ---------- addTable / addRow / addColumn ----------
// addTable: 初始行列规模正确
TEST(UT_DocxWriter, UT_DocxWriter_addTable_initialSize)
{
    // Arrange: 写入器
    DocxWriter writer;

    // Act: 创建 2x3 表格
    DocxTable *tab = writer.addTable(2, 3);

    // Assert: 行列数符合预期
    EXPECT_EQ(tab->rowCount(), 2);
    EXPECT_EQ(tab->columnCount(), 3);
}

// addTable: 负数行列按 0 处理
TEST(UT_DocxWriter, UT_DocxWriter_addTable_negativeClampsToZero)
{
    // Arrange: 写入器
    DocxWriter writer;

    // Act: 创建负数规模表格
    DocxTable *tab = writer.addTable(-1, -2);

    // Assert: 行列数为 0
    EXPECT_EQ(tab->rowCount(), 0);
    EXPECT_EQ(tab->columnCount(), 0);
}

// Table::addRow/addColumn: 动态扩展并保持已有内容
TEST(UT_DocxWriter, UT_DocxWriter_addRowColumn_grows)
{
    // Arrange: 从 1x0 起步（与 EXPORT_TO_DOC 用法一致）
    DocxWriter writer;
    DocxTable *tab = writer.addTable(1, 0);

    // Act: 扩列加行, 写入内容
    tab->addColumn();
    tab->addColumn();
    tab->cell(0, 0)->addText("制造商");
    tab->addRow();
    tab->cell(1, 1)->addText("型号");

    // Assert: 规模正确, 单元格可读回（通过再次 save 校验 XML）
    QTemporaryDir tmpDir;
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");
    const QString xml = QString::fromUtf8(doc);
    EXPECT_EQ(tab->rowCount(), 2);
    EXPECT_EQ(tab->columnCount(), 2);
    EXPECT_TRUE(xml.contains("制造商"));
    EXPECT_TRUE(xml.contains("型号"));
}

// addTable: 空表格输出自闭合 tblGrid 且无行
TEST(UT_DocxWriter, UT_DocxWriter_addTable_emptyTableXml)
{
    // Arrange: 0x0 表格
    DocxWriter writer;
    writer.addTable(0, 0);
    QTemporaryDir tmpDir;

    // Act: 落盘并提取文档
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");

    // Assert: 有 tbl 与 tblGrid, 无 w:tr
    const QString xml = QString::fromUtf8(doc);
    EXPECT_TRUE(xml.contains("<w:tbl>"));
    EXPECT_TRUE(xml.contains("<w:tblGrid></w:tblGrid>"));
    EXPECT_FALSE(xml.contains("<w:tr>"));
}

// ---------- cell / addText ----------
// cell: 越界返回 nullptr
TEST(UT_DocxWriter, UT_DocxWriter_cell_outOfRange)
{
    // Arrange: 1x1 表格
    DocxWriter writer;
    DocxTable *tab = writer.addTable(1, 1);

    // Act & Assert: 越界访问返回空指针
    EXPECT_EQ(tab->cell(1, 0), nullptr);
    EXPECT_EQ(tab->cell(0, 1), nullptr);
    EXPECT_EQ(tab->cell(-1, 0), nullptr);
    EXPECT_NE(tab->cell(0, 0), nullptr);
}

// cell: 重复取同一格返回同一指针, 后写覆盖先写
TEST(UT_DocxWriter, UT_DocxWriter_cell_samePointerOverwrite)
{
    // Arrange: 1x1 表格
    DocxWriter writer;
    DocxTable *tab = writer.addTable(1, 1);

    // Act: 两次取同一格并分别写入
    DocxCell *first = tab->cell(0, 0);
    DocxCell *second = tab->cell(0, 0);
    first->addText("旧值");
    second->addText("新值");
    QTemporaryDir tmpDir;
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");

    // Assert: 指针一致, 仅保留新值
    EXPECT_EQ(first, second);
    EXPECT_TRUE(QString::fromUtf8(doc).contains("新值"));
    EXPECT_FALSE(QString::fromUtf8(doc).contains("旧值"));
}

// cell: addText 转义特殊字符
TEST(UT_DocxWriter, UT_DocxWriter_cell_escapesXml)
{
    // Arrange: 1x1 表格
    DocxWriter writer;
    DocxTable *tab = writer.addTable(1, 1);
    QTemporaryDir tmpDir;

    // Act: 写入特殊字符并落盘
    tab->cell(0, 0)->addText("<&>");
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");

    // Assert: 转义正确
    EXPECT_TRUE(QString::fromUtf8(doc).contains("&lt;&amp;&gt;"));
}

// ---------- save ----------
// save: 完整 ZIP 结构与 OOXML 部件齐全
TEST(UT_DocxWriter, UT_DocxWriter_save_zipStructure)
{
    // Arrange: 带内容的写入器与临时目录
    DocxWriter writer;
    writer.addHeading("概况");
    writer.addParagraph("文本");
    DocxTable *tab = writer.addTable(1, 1);
    tab->cell(0, 0)->addText("单元格");
    QTemporaryDir tmpDir;
    QString path = tmpDir.filePath("ut_docx_full.docx");

    // Act: 落盘
    EXPECT_TRUE(writer.save(path));

    // Assert: 7 个必需部件齐全
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    const QByteArray zip = f.readAll();
    const char *parts[] = {
        "[Content_Types].xml", "_rels/.rels", "docProps/core.xml", "docProps/app.xml",
        "word/document.xml", "word/_rels/document.xml.rels", "word/styles.xml"
    };
    for (const char *part : parts)
        EXPECT_FALSE(utExtractStoredEntry(zip, QString::fromLatin1(part)).isEmpty()) << part;
}

// save: 表格带边框定义
TEST(UT_DocxWriter, UT_DocxWriter_save_tableBorders)
{
    // Arrange: 带表格的写入器
    DocxWriter writer;
    writer.addTable(1, 1);
    QTemporaryDir tmpDir;

    // Act: 落盘并提取文档
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");

    // Assert: tblBorders 六向边框齐全（与旧实现一致）
    const QString xml = QString::fromUtf8(doc);
    EXPECT_TRUE(xml.contains("<w:tblBorders>"));
    EXPECT_EQ(xml.count("w:val=\"single\""), 6);
}

// save: 空文档也能生成有效包
TEST(UT_DocxWriter, UT_DocxWriter_save_emptyDocument)
{
    // Arrange: 空写入器
    DocxWriter writer;
    QTemporaryDir tmpDir;

    // Act: 落盘并提取文档
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");

    // Assert: 含 body 与 sectPr, 无段落
    const QString xml = QString::fromUtf8(doc);
    EXPECT_TRUE(xml.contains("<w:body>"));
    EXPECT_TRUE(xml.contains("<w:sectPr>"));
    EXPECT_FALSE(xml.contains("<w:pStyle"));
}

// save: 不可写路径返回 false
TEST(UT_DocxWriter, UT_DocxWriter_save_badPath)
{
    // Arrange: 空写入器与不存在目录下的路径
    DocxWriter writer;

    // Act & Assert: 保存失败
    EXPECT_FALSE(writer.save("/nonexistent-dir-ut/doc.docx"));
    EXPECT_FALSE(writer.save(QString()));
}

// ---------- styles ----------
// save: styles.xml 内置 Heading1/Heading2 样式（修复旧实现引用不存在样式的问题）
TEST(UT_DocxWriter, UT_DocxWriter_save_stylesDefined)
{
    // Arrange: 写入器与临时目录
    DocxWriter writer;
    writer.addHeading("标题", 1);
    writer.addHeading("小节", 2);
    QTemporaryDir tmpDir;

    // Act: 落盘并提取样式部件
    const QByteArray styles = utDocxEntryOf(writer, tmpDir, "word/styles.xml");

    // Assert: Heading1/Heading2 样式存在且带大纲级别
    const QString xml = QString::fromUtf8(styles);
    EXPECT_TRUE(xml.contains("w:styleId=\"Heading1\""));
    EXPECT_TRUE(xml.contains("w:styleId=\"Heading2\""));
    EXPECT_TRUE(xml.contains("<w:outlineLvl w:val=\"0\"/>"));
    EXPECT_TRUE(xml.contains("<w:outlineLvl w:val=\"1\"/>"));
}

// ---------- DocxWriter 生命周期 ----------
// setCellText: 越界安全返回（测试构建 -fno-access-control 可直调私有方法）
TEST(UT_DocxWriter, UT_DocxWriter_setCellText_outOfRangeSafe)
{
    // Arrange: 1x1 表格
    DocxWriter writer;
    DocxTable *tab = writer.addTable(1, 1);

    // Act: 越界写入不应崩溃
    tab->setCellText(5, 5, "x");
    tab->setCellText(-1, 0, "x");
    tab->setCellText(0, -1, "x");

    // Assert: 正常路径仍可写入
    tab->setCellText(0, 0, "ok");
    QTemporaryDir tmpDir;
    const QByteArray doc = utDocxEntryOf(writer, tmpDir, "word/document.xml");
    EXPECT_TRUE(QString::fromUtf8(doc).contains("ok"));
}

// addTable: 表格由 DocxWriter 释放, 不泄漏（ASAN 环境下泄漏会报错）
TEST(UT_DocxWriter, UT_DocxWriter_destructor_ownsTables)
{
    // Arrange & Act: 作用域内创建带表格的写入器
    {
        DocxWriter writer;
        DocxTable *tab = writer.addTable(1, 1);
        tab->cell(0, 0)->addText("内容");
    }

    // Assert: 正常退出作用域即通过（无崩溃/泄漏）
    SUCCEED();
}
