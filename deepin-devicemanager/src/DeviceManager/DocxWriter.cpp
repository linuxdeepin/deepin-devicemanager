// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DocxWriter.h"
#include "ZipWriter.h"

#include <QSaveFile>

// ============================================================
// 工具函数
// ============================================================
namespace {

QString headingStyle(int level)
{
    // 级别 1/2 映射 Heading1/Heading2，其余级别按 2 处理（与旧实现的 1/2 两级用法对齐）
    return level <= 1 ? QStringLiteral("Heading1") : QStringLiteral("Heading2");
}

QByteArray borderElement(const char *name)
{
    // 单线边框，sz=4（0.5pt），颜色 auto —— 与旧实现的表格边框一致
    return QByteArray("<w:") + name
           + " w:val=\"single\" w:color=\"auto\" w:sz=\"4\" w:space=\"0\"/>";
}

}   // namespace

// ============================================================
// DocxWriter
// ============================================================
DocxWriter::DocxWriter() = default;

DocxWriter::~DocxWriter()
{
    qDeleteAll(m_tables);
}

void DocxWriter::addHeading(const QString &text, int level)
{
    Block block;
    block.text = text;
    block.style = headingStyle(level);
    m_blocks.append(block);
}

void DocxWriter::addParagraph(const QString &text)
{
    Block block;
    block.text = text;
    block.style = QStringLiteral("Normal");
    m_blocks.append(block);
}

DocxTable *DocxWriter::addTable(int rows, int cols)
{
    DocxTable *table = new DocxTable(rows, cols);
    m_tables.append(table);

    Block block;
    block.isTable = true;
    block.table = table;
    m_blocks.append(block);

    return table;
}

QByteArray DocxWriter::buildContentTypes() const
{
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        "<Override PartName=\"/word/document.xml\""
        " ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
        "<Override PartName=\"/word/styles.xml\""
        " ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>"
        "<Override PartName=\"/docProps/core.xml\""
        " ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/>"
        "<Override PartName=\"/docProps/app.xml\""
        " ContentType=\"application/vnd.openxmlformats-officedocument.extended-properties+xml\"/>"
        "</Types>");
}

QByteArray DocxWriter::buildRootRels() const
{
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\""
        " Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\""
        " Target=\"word/document.xml\"/>"
        "<Relationship Id=\"rId2\""
        " Type=\"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties\""
        " Target=\"docProps/core.xml\"/>"
        "<Relationship Id=\"rId3\""
        " Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties\""
        " Target=\"docProps/app.xml\"/>"
        "</Relationships>");
}

QByteArray DocxWriter::buildDocumentRels() const
{
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\""
        " Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\""
        " Target=\"styles.xml\"/>"
        "</Relationships>");
}

QByteArray DocxWriter::buildCore() const
{
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<cp:coreProperties"
        " xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\""
        " xmlns:dc=\"http://purl.org/dc/elements/1.1/\""
        " xmlns:dcterms=\"http://purl.org/dc/terms/\""
        " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
        "<dc:creator>deepin-devicemanager</dc:creator>"
        "<cp:lastModifiedBy>deepin-devicemanager</cp:lastModifiedBy>"
        "</cp:coreProperties>");
}

QByteArray DocxWriter::buildApp() const
{
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\""
        " xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\">"
        "<Application>deepin-devicemanager</Application>"
        "</Properties>");
}

QByteArray DocxWriter::buildDocument() const
{
    QByteArray xml(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
        "<w:body>");

    for (const Block &block : m_blocks) {
        if (block.isTable) {
            xml += block.table->toXml();
            // 表格之后必须跟一个段落（OOXML 要求表格不能是相邻表格/正文末尾的紧邻元素）
            xml += "<w:p/>";
            continue;
        }

        // 段落：pPr 必须位于 run 之前（OOXML schema 顺序）
        xml += "<w:p>";
        if (block.style != QLatin1String("Normal"))
            xml += QByteArray("<w:pPr><w:pStyle w:val=\"") + block.style.toUtf8() + "\"/></w:pPr>";
        if (!block.text.isEmpty()) {
            xml += "<w:r><w:t xml:space=\"preserve\">"
                   + ZipDetail::escapeXml(block.text).toUtf8() + "</w:t></w:r>";
        }
        xml += "</w:p>";
    }

    // 页面设置沿用原模板：A4，上下边距 1440 缇，左右 1800 缇
    xml += "<w:sectPr>"
           "<w:pgSz w:w=\"11906\" w:h=\"16838\"/>"
           "<w:pgMar w:top=\"1440\" w:right=\"1800\" w:bottom=\"1440\" w:left=\"1800\""
           " w:header=\"0\" w:footer=\"0\" w:gutter=\"0\"/>"
           "<w:docGrid w:type=\"lines\" w:linePitch=\"312\"/>"
           "</w:sectPr>"
           "</w:body></w:document>";
    return xml;
}

QByteArray DocxWriter::buildStyles() const
{
    // 字体沿用原模板：正文 Calibri/宋体 10.5pt，标题 Liberation Sans/Noto Sans CJK SC
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
        "<w:docDefaults>"
        "<w:rPrDefault><w:rPr>"
        "<w:rFonts w:ascii=\"Calibri\" w:hAnsi=\"Calibri\" w:eastAsia=\"宋体\" w:cs=\"Times New Roman\"/>"
        "<w:sz w:val=\"21\"/><w:szCs w:val=\"21\"/>"
        "<w:lang w:val=\"en-US\" w:eastAsia=\"zh-CN\" w:bidi=\"ar-SA\"/>"
        "</w:rPr></w:rPrDefault>"
        "<w:pPrDefault/>"
        "</w:docDefaults>"
        "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\">"
        "<w:name w:val=\"Normal\"/><w:qFormat/>"
        "</w:style>"
        "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\">"
        "<w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/>"
        "<w:qFormat/>"
        "<w:pPr><w:keepNext/><w:spacing w:before=\"240\" w:after=\"120\"/>"
        "<w:outlineLvl w:val=\"0\"/></w:pPr>"
        "<w:rPr>"
        "<w:rFonts w:ascii=\"Liberation Sans\" w:hAnsi=\"Liberation Sans\""
        " w:eastAsia=\"Noto Sans CJK SC\" w:cs=\"Noto Sans CJK SC\"/>"
        "<w:b/><w:sz w:val=\"28\"/><w:szCs w:val=\"28\"/>"
        "</w:rPr>"
        "</w:style>"
        "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\">"
        "<w:name w:val=\"heading 2\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/>"
        "<w:qFormat/>"
        "<w:pPr><w:keepNext/><w:spacing w:before=\"240\" w:after=\"120\"/>"
        "<w:outlineLvl w:val=\"1\"/></w:pPr>"
        "<w:rPr>"
        "<w:rFonts w:ascii=\"Liberation Sans\" w:hAnsi=\"Liberation Sans\""
        " w:eastAsia=\"Noto Sans CJK SC\" w:cs=\"Noto Sans CJK SC\"/>"
        "<w:b/><w:sz w:val=\"24\"/><w:szCs w:val=\"24\"/>"
        "</w:rPr>"
        "</w:style>"
        "</w:styles>");
}

bool DocxWriter::save(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    ZipWriter zip;
    zip.addFile(QLatin1String("[Content_Types].xml"), buildContentTypes());
    zip.addFile(QLatin1String("_rels/.rels"), buildRootRels());
    zip.addFile(QLatin1String("docProps/core.xml"), buildCore());
    zip.addFile(QLatin1String("docProps/app.xml"), buildApp());
    zip.addFile(QLatin1String("word/document.xml"), buildDocument());
    zip.addFile(QLatin1String("word/_rels/document.xml.rels"), buildDocumentRels());
    zip.addFile(QLatin1String("word/styles.xml"), buildStyles());
    return zip.saveAs(filePath);
}

// ============================================================
// DocxCell
// ============================================================
void DocxCell::addText(const QString &text)
{
    if (m_table)
        m_table->setCellText(m_row, m_col, text);
}

// ============================================================
// DocxTable
// ============================================================
DocxTable::DocxTable(int rows, int cols)
    : m_cols(qMax(0, cols))
{
    const int initRows = qMax(0, rows);
    for (int i = 0; i < initRows; ++i)
        m_cells.append(QVector<QString>(m_cols));
}

void DocxTable::addColumn()
{
    ++m_cols;
    for (QVector<QString> &row : m_cells)
        row.append(QString());
}

void DocxTable::addRow()
{
    m_cells.append(QVector<QString>(m_cols));
}

DocxCell *DocxTable::cell(int row, int col)
{
    if (row < 0 || row >= m_cells.size() || col < 0 || col >= m_cols)
        return nullptr;

    const QPair<int, int> key(row, col);
    if (!m_cellPool.contains(key))
        m_cellPool.insert(key, DocxCell(this, row, col));
    return &m_cellPool[key];
}

void DocxTable::setCellText(int row, int col, const QString &text)
{
    if (row < 0 || row >= m_cells.size() || col < 0 || col >= m_cols)
        return;
    m_cells[row][col] = text;
}

QByteArray DocxTable::toXml() const
{
    QByteArray xml(
        "<w:tbl>"
        "<w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/>"
        "<w:tblBorders>");
    xml += borderElement("top");
    xml += borderElement("left");
    xml += borderElement("bottom");
    xml += borderElement("right");
    xml += borderElement("insideH");
    xml += borderElement("insideV");
    xml += "</w:tblBorders></w:tblPr>";

    xml += QByteArray("<w:tblGrid>");
    for (int i = 0; i < m_cols; ++i)
        xml += "<w:gridCol/>";
    xml += "</w:tblGrid>";

    for (const QVector<QString> &row : m_cells) {
        xml += "<w:tr>";
        for (const QString &text : row) {
            xml += "<w:tc><w:tcPr><w:tcW w:w=\"0\" w:type=\"auto\"/></w:tcPr><w:p>";
            if (!text.isEmpty()) {
                xml += "<w:r><w:t xml:space=\"preserve\">"
                       + ZipDetail::escapeXml(text).toUtf8() + "</w:t></w:r>";
            }
            xml += "</w:p></w:tc>";
        }
        xml += "</w:tr>";
    }

    xml += "</w:tbl>";
    return xml;
}
