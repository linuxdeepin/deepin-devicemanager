// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "XlsxWriter.h"
#include "ZipWriter.h"

#include <QDateTime>

// ============================================================
// 内部工具：列名（xlsx 专用，独立成自由函数便于单测直测）
// ============================================================
namespace XlsxDetail {

QString columnName(int column)   // 1 基列号 -> A/.../Z/AA/... 样式
{
    QString name;
    int col = qMax(1, column);
    while (col > 0) {
        const int rem = (col - 1) % 26;
        name.prepend(QChar(uchar('A' + rem)));
        col = (col - 1) / 26;
    }
    return name;
}

}   // namespace XlsxDetail

// ============================================================
// XlsxWriter
// ============================================================
bool XlsxWriter::write(int row, int column, const QString &value, const XlsxFormat &format)
{
    m_cells[qMax(1, row)][qMax(1, column)] = Cell { value, format };
    return true;
}

QByteArray XlsxWriter::buildContentTypes() const
{
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        "<Override PartName=\"/xl/workbook.xml\""
        " ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
        "<Override PartName=\"/xl/worksheets/sheet1.xml\""
        " ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
        "<Override PartName=\"/xl/styles.xml\""
        " ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
        "<Override PartName=\"/docProps/app.xml\""
        " ContentType=\"application/vnd.openxmlformats-officedocument.extended-properties+xml\"/>"
        "</Types>");
}

QByteArray XlsxWriter::buildRootRels() const
{
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\""
        " Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\""
        " Target=\"xl/workbook.xml\"/>"
        "</Relationships>");
}

QByteArray XlsxWriter::buildWorkbook() const
{
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\""
        " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
        "<sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/></sheets>"
        "</workbook>");
}

QByteArray XlsxWriter::buildWorkbookRels() const
{
    return QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\""
        " Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\""
        " Target=\"worksheets/sheet1.xml\"/>"
        "<Relationship Id=\"rId2\""
        " Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\""
        " Target=\"styles.xml\"/>"
        "</Relationships>");
}

QByteArray XlsxWriter::buildStyles() const
{
    // 收集实际用到的格式组合（默认格式之外），动态生成 fonts/cellXfs
    QList<XlsxFormat> used;
    for (auto rowIt = m_cells.constBegin(); rowIt != m_cells.constEnd(); ++rowIt) {
        for (auto colIt = rowIt->constBegin(); colIt != rowIt->constEnd(); ++colIt) {
            const XlsxFormat &fmt = colIt->format;
            if (fmt == XlsxFormat())
                continue;
            if (!used.contains(fmt))
                used.append(fmt);
        }
    }

    QByteArray xml(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">");

    // fonts: 索引 0 为默认字体（Calibri 11），其后按 used 顺序
    xml += QByteArray("<fonts count=\"") + QByteArray::number(used.size() + 1) + "\">";
    xml += "<font><name val=\"Calibri\"/><sz val=\"11\"/></font>";
    for (const XlsxFormat &fmt : used) {
        xml += "<font><name val=\"Calibri\"/>";
        if (fmt.fontBold())
            xml += "<b/>";
        if (fmt.fontSize() > 0)
            xml += QByteArray("<sz val=\"") + QByteArray::number(fmt.fontSize()) + "\"/>";
        xml += "</font>";
    }
    xml += "</fonts>";

    // fills/borders: OOXML 要求的最小集合（fill[1] 的 gray125 为规范惯例）
    xml += "<fills count=\"2\">"
           "<fill><patternFill patternType=\"none\"/></fill>"
           "<fill><patternFill patternType=\"gray125\"/></fill>"
           "</fills>"
           "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
           "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>";

    // cellXfs: 索引 0 为默认样式，其后与 used 对齐（fontId = 样式序号）
    xml += QByteArray("<cellXfs count=\"") + QByteArray::number(used.size() + 1) + "\">";
    xml += "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>";
    for (int i = 0; i < used.size(); ++i) {
        xml += QByteArray("<xf numFmtId=\"0\" fontId=\"") + QByteArray::number(i + 1)
               + "\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/>";
    }
    xml += "</cellXfs>";

    xml += "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>"
           "</styleSheet>";
    return xml;
}

QByteArray XlsxWriter::buildSheet() const
{
    // 先收集所有用到的格式，建立格式 -> cellXf 索引（与 buildStyles 同序）
    QList<XlsxFormat> used;
    for (auto rowIt = m_cells.constBegin(); rowIt != m_cells.constEnd(); ++rowIt) {
        for (auto colIt = rowIt->constBegin(); colIt != rowIt->constEnd(); ++colIt) {
            const XlsxFormat &fmt = colIt->format;
            if (fmt == XlsxFormat())
                continue;
            if (!used.contains(fmt))
                used.append(fmt);
        }
    }

    QByteArray xml(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">");

    if (!m_cells.isEmpty()) {
        const int maxRow = m_cells.lastKey();
        int maxCol = 1;
        for (auto rowIt = m_cells.constBegin(); rowIt != m_cells.constEnd(); ++rowIt)
            maxCol = qMax(maxCol, rowIt->lastKey());
        xml += QByteArray("<dimension ref=\"A1:") + XlsxDetail::columnName(maxCol).toLatin1()
               + QByteArray::number(maxRow) + "\"/>";
    }

    QByteArray rowsXml;
    for (auto rowIt = m_cells.constBegin(); rowIt != m_cells.constEnd(); ++rowIt) {
        rowsXml += QByteArray("<row r=\"") + QByteArray::number(rowIt.key()) + "\">";
        for (auto colIt = rowIt->constBegin(); colIt != rowIt->constEnd(); ++colIt) {
            const Cell &cell = colIt.value();
            rowsXml += QByteArray("<c r=\"") + XlsxDetail::columnName(colIt.key()).toLatin1()
                   + QByteArray::number(rowIt.key()) + "\"";
            // 非默认格式对应 cellXfs 索引 = used 序号 + 1（0 号保留给默认样式）
            const int styleIdx = used.indexOf(cell.format);
            if (styleIdx >= 0)
                rowsXml += QByteArray(" s=\"") + QByteArray::number(styleIdx + 1) + "\"";
            rowsXml += " t=\"inlineStr\"><is><t xml:space=\"preserve\">"
                   + ZipDetail::escapeXml(cell.value).toUtf8() + "</t></is></c>";
        }
        rowsXml += "</row>";
    }

    if (rowsXml.isEmpty())
        xml += "<sheetData/>";
    else
        xml += "<sheetData>" + rowsXml + "</sheetData>";
    xml += "</worksheet>";
    return xml;
}

bool XlsxWriter::saveAs(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    ZipWriter zip;
    zip.addFile(QLatin1String("[Content_Types].xml"), buildContentTypes());
    zip.addFile(QLatin1String("_rels/.rels"), buildRootRels());
    zip.addFile(QLatin1String("docProps/app.xml"), QByteArray(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/"
        "extended-properties\" xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/"
        "2006/docPropsVTypes\"><Application>deepin-devicemanager</Application></Properties>"));
    zip.addFile(QLatin1String("xl/workbook.xml"), buildWorkbook());
    zip.addFile(QLatin1String("xl/_rels/workbook.xml.rels"), buildWorkbookRels());
    zip.addFile(QLatin1String("xl/styles.xml"), buildStyles());
    zip.addFile(QLatin1String("xl/worksheets/sheet1.xml"), buildSheet());
    return zip.saveAs(filePath);
}
