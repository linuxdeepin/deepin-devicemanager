// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: 零三方依赖 xlsx 写入器 (deepin-devicemanager/src/DeviceManager/xlsxwriter.cpp)
// 覆盖: CRC-32、列名转换、XML 转义、单元格写入/覆盖/越界截断、
//       ZIP(STORE) 结构完整性、OOXML 部件内容、动态样式、异常路径。
// 注: ZIP 内容提取使用测试专用 STORE 读取辅助（minizip_read_helper.h）。
//
// 用例计数声明（self-check-structural 验证此块）：
// | function              | level | factors | min | actual |
// |-----------------------|-------|---------|-----|--------|
// | XlsxDetail::crc32     | low   | -       | 1   | 2      |
// | XlsxDetail::columnName| mid   | -       | 2   | 4      |
// | XlsxDetail::escapeXml | mid   | -       | 2   | 2      |
// | write                 | mid   | -       | 2   | 3      |
// | saveAs                | mid   | -       | 2   | 6      |
// ============================================================

#include "XlsxWriter.h"
#include "minizip_read_helper.h"

#include <QFile>
#include <QTemporaryDir>

#include <gtest/gtest.h>

// ------------------------------------------------------------
// XlsxDetail 自由函数直测
// ------------------------------------------------------------
TEST(XlsxDetail, crc32_knownVectors)
{
    // RFC 3720 / zip 标准校验向量
    EXPECT_EQ(XlsxDetail::crc32(QByteArray("")), 0x00000000u);
    EXPECT_EQ(XlsxDetail::crc32(QByteArray("123456789")), 0xCBF43926u);
    EXPECT_EQ(XlsxDetail::crc32(QByteArray("The quick brown fox jumps over the lazy dog")),
              0x414FA339u);
}

TEST(XlsxDetail, crc32_matchesZipHeader)
{
    // crc32 结果必须与写出的 ZIP local header 中 CRC 字段一致（首条目 [Content_Types].xml）
    XlsxWriter xlsx;
    xlsx.write(1, 1, QStringLiteral("crc-check"));
    QTemporaryDir dir;
    const QString path = dir.filePath("crc.xlsx");
    ASSERT_TRUE(xlsx.saveAs(path));

    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    const QByteArray zip = f.readAll();

    // local header: 签名(4) + version/flags/method/time/date(10) 后即 CRC 字段（偏移 14..17）
    const quint32 headerCrc = quint32(uchar(zip[14])) | (quint32(uchar(zip[15])) << 8)
            | (quint32(uchar(zip[16])) << 16) | (quint32(uchar(zip[17])) << 24);
    EXPECT_EQ(headerCrc, XlsxDetail::crc32(utExtractStoredEntry(zip, "[Content_Types].xml")));
}

TEST(XlsxDetail, columnName_basic)
{
    EXPECT_EQ(XlsxDetail::columnName(1), QStringLiteral("A"));
    EXPECT_EQ(XlsxDetail::columnName(26), QStringLiteral("Z"));
}

TEST(XlsxDetail, columnName_multiLetters)
{
    EXPECT_EQ(XlsxDetail::columnName(27), QStringLiteral("AA"));
    EXPECT_EQ(XlsxDetail::columnName(52), QStringLiteral("AZ"));
    EXPECT_EQ(XlsxDetail::columnName(703), QStringLiteral("AAA"));
}

TEST(XlsxDetail, columnName_clampsToValid)
{
    EXPECT_EQ(XlsxDetail::columnName(0), QStringLiteral("A"));
    EXPECT_EQ(XlsxDetail::columnName(-5), QStringLiteral("A"));
}

TEST(XlsxDetail, escapeXml_specialChars)
{
    EXPECT_EQ(XlsxDetail::escapeXml(QStringLiteral("a<b>&\"c\"'d'")),
              QStringLiteral("a&lt;b&gt;&amp;&quot;c&quot;&apos;d&apos;"));
}

TEST(XlsxDetail, escapeXml_dropsControlChars)
{
    // XML 1.0 非法控制字符被丢弃，\t \n \r 保留
    QString in = QStringLiteral("a");
    in.append(QChar(0x01));
    in.append(QChar(0x02));
    in.append(QChar(0x09));
    in.append(QChar(0x0A));
    in.append(QChar(0x0D));
    in.append(QChar('b'));
    EXPECT_EQ(XlsxDetail::escapeXml(in), QStringLiteral("a\t\n\rb"));
}

// ------------------------------------------------------------
// XlsxWriter::write
// ------------------------------------------------------------
TEST(XlsxWriterTest, write_overwriteSameCell_lastWins)
{
    XlsxWriter xlsx;
    xlsx.write(1, 1, QStringLiteral("first"));
    xlsx.write(1, 1, QStringLiteral("second"));

    QTemporaryDir dir;
    const QString path = dir.filePath("overwrite.xlsx");
    ASSERT_TRUE(xlsx.saveAs(path));
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    const QByteArray sheet = utExtractStoredEntry(f.readAll(), "xl/worksheets/sheet1.xml");
    EXPECT_FALSE(sheet.contains("first"));
    EXPECT_TRUE(sheet.contains("second"));
}

TEST(XlsxWriterTest, write_clampsInvalidRowCol)
{
    XlsxWriter xlsx;
    xlsx.write(0, 0, QStringLiteral("clamped"));   // 越界截断到 A1

    QTemporaryDir dir;
    const QString path = dir.filePath("clamp.xlsx");
    ASSERT_TRUE(xlsx.saveAs(path));
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    const QByteArray sheet = utExtractStoredEntry(f.readAll(), "xl/worksheets/sheet1.xml");
    EXPECT_TRUE(sheet.contains("r=\"A1\""));
}

TEST(XlsxWriterTest, write_chineseAndSpecialText)
{
    XlsxWriter xlsx;
    xlsx.write(1, 1, QStringLiteral("厂商<Core&I7> \"测试\""));

    QTemporaryDir dir;
    const QString path = dir.filePath("utf8.xlsx");
    ASSERT_TRUE(xlsx.saveAs(path));
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    const QByteArray sheet = f.readAll();
    EXPECT_TRUE(sheet.contains("厂商&lt;Core&amp;I7&gt; &quot;测试&quot;"));
}

// ------------------------------------------------------------
// XlsxWriter::saveAs — 结构与内容
// ------------------------------------------------------------
class UT_XlsxWriter : public ::testing::Test
{
public:
    void SetUp() override
    {
        dir = new QTemporaryDir;
    }
    void TearDown() override
    {
        delete dir;
    }
    QByteArray readAll(const QString &name) const
    {
        QFile f(dir->filePath(name));
        if (!f.open(QIODevice::ReadOnly))
            return QByteArray();
        return f.readAll();
    }
    QTemporaryDir *dir = nullptr;
};

TEST_F(UT_XlsxWriter, saveAs_emptyWriter_producesValidEmptySheet)
{
    XlsxWriter xlsx;
    const QString path = dir->filePath("empty.xlsx");
    EXPECT_TRUE(xlsx.saveAs(path));

    const QByteArray zip = readAll("empty.xlsx");
    EXPECT_TRUE(zip.startsWith(QByteArray("PK\x03\x04", 4)));
    const QByteArray sheet = utExtractStoredEntry(zip, "xl/worksheets/sheet1.xml");
    EXPECT_TRUE(sheet.contains("<sheetData/>"));
    EXPECT_FALSE(sheet.contains("<sheetData>"));   // 无行数据
}

TEST_F(UT_XlsxWriter, saveAs_containsAllOoxmlParts)
{
    XlsxWriter xlsx;
    xlsx.write(1, 1, QStringLiteral("data"));
    const QString path = dir->filePath("parts.xlsx");
    ASSERT_TRUE(xlsx.saveAs(path));

    const QByteArray zip = readAll("parts.xlsx");
    EXPECT_FALSE(utExtractStoredEntry(zip, "[Content_Types].xml").isEmpty());
    EXPECT_FALSE(utExtractStoredEntry(zip, "_rels/.rels").isEmpty());
    EXPECT_FALSE(utExtractStoredEntry(zip, "docProps/app.xml").isEmpty());
    EXPECT_FALSE(utExtractStoredEntry(zip, "xl/workbook.xml").isEmpty());
    EXPECT_FALSE(utExtractStoredEntry(zip, "xl/_rels/workbook.xml.rels").isEmpty());
    EXPECT_FALSE(utExtractStoredEntry(zip, "xl/styles.xml").isEmpty());
    const QByteArray sheet = utExtractStoredEntry(zip, "xl/worksheets/sheet1.xml");
    EXPECT_TRUE(sheet.contains("t=\"inlineStr\""));
    EXPECT_TRUE(sheet.contains(QString("data").toUtf8()));
}

TEST_F(UT_XlsxWriter, saveAs_rowColToCellReference)
{
    XlsxWriter xlsx;
    xlsx.write(2, 27, QStringLiteral("AA2"));   // 第 27 列 = AA

    const QString path = dir->filePath("ref.xlsx");
    ASSERT_TRUE(xlsx.saveAs(path));
    const QByteArray sheet = utExtractStoredEntry(readAll("ref.xlsx"), "xl/worksheets/sheet1.xml");
    EXPECT_TRUE(sheet.contains("r=\"AA2\""));
    EXPECT_TRUE(sheet.contains("<row r=\"2\">"));
}

TEST_F(UT_XlsxWriter, saveAs_defaultFormat_hasNoStyleIndex)
{
    XlsxWriter xlsx;
    xlsx.write(1, 1, QStringLiteral("plain"));   // 默认格式，s 属性应省略

    const QString path = dir->filePath("plain.xlsx");
    ASSERT_TRUE(xlsx.saveAs(path));
    const QByteArray sheet = utExtractStoredEntry(readAll("plain.xlsx"), "xl/worksheets/sheet1.xml");
    EXPECT_TRUE(sheet.contains("<c r=\"A1\" t=\"inlineStr\">"));
}

TEST_F(UT_XlsxWriter, saveAs_dynamicStyles_boldAndSize)
{
    XlsxWriter xlsx;
    XlsxFormat bold10;
    bold10.setFontBold(true);
    bold10.setFontSize(10);
    XlsxFormat plain10;
    plain10.setFontSize(10);

    xlsx.write(1, 1, QStringLiteral("head"), bold10);     // s=1
    xlsx.write(2, 1, QStringLiteral("body"), plain10);    // s=2

    const QString path = dir->filePath("styles.xlsx");
    ASSERT_TRUE(xlsx.saveAs(path));
    const QByteArray zip = readAll("styles.xlsx");

    const QByteArray styles = utExtractStoredEntry(zip, "xl/styles.xml");
    EXPECT_TRUE(styles.contains("<b/>"));
    EXPECT_TRUE(styles.contains("<sz val=\"10\"/>"));
    EXPECT_TRUE(styles.contains("<fonts count=\"3\">"));   // 默认 + 2 种组合
    EXPECT_TRUE(styles.contains("<cellXfs count=\"3\">"));

    const QByteArray sheet = utExtractStoredEntry(zip, "xl/worksheets/sheet1.xml");
    EXPECT_TRUE(sheet.contains("s=\"1\""));
    EXPECT_TRUE(sheet.contains("s=\"2\""));
}

TEST_F(UT_XlsxWriter, saveAs_invalidPaths_fails)
{
    XlsxWriter xlsx;
    xlsx.write(1, 1, QStringLiteral("x"));
    EXPECT_FALSE(xlsx.saveAs(QString()));   // 空路径

    // 不可写路径（目录不存在）
    EXPECT_FALSE(xlsx.saveAs(dir->filePath("no_such_dir/a/b.xlsx")));
}

TEST_F(UT_XlsxWriter, saveAs_zipStructure_eocdConsistent)
{
    XlsxWriter xlsx;
    xlsx.write(1, 1, QStringLiteral("a"));
    const QString path = dir->filePath("struct.xlsx");
    ASSERT_TRUE(xlsx.saveAs(path));

    const QByteArray zip = readAll("struct.xlsx");
    ASSERT_GT(zip.size(), 22);
    // EOCD 位于尾部，条目总数应为 7
    const int eocd = zip.size() - 22;
    EXPECT_EQ(zip.mid(eocd, 4), QByteArray("PK\x05\x06", 4));
    const quint16 entries = quint16(uchar(zip[eocd + 10])) | (quint16(uchar(zip[eocd + 11])) << 8);
    EXPECT_EQ(entries, 7);
    // central directory 签名存在
    const quint32 cdOffset = quint32(uchar(zip[eocd + 16])) | (quint32(uchar(zip[eocd + 17])) << 8)
            | (quint32(uchar(zip[eocd + 18])) << 16) | (quint32(uchar(zip[eocd + 19])) << 24);
    EXPECT_EQ(zip.mid(int(cdOffset), 4), QByteArray("PK\x01\x02", 4));
}
