// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "XlsxWriter.h"

#include <QDateTime>
#include <QSaveFile>

#include <limits>

// ============================================================
// 内部工具：CRC-32 / 列名 / XML 转义（独立成自由函数便于单测直测）
// ============================================================
namespace XlsxDetail {

quint32 crc32(const QByteArray &data)
{
    static quint32 table[256];
    static bool tableReady = false;
    if (!tableReady) {
        for (quint32 i = 0; i < 256; ++i) {
            quint32 c = i;
            for (int k = 0; k < 8; ++k)
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            table[i] = c;
        }
        tableReady = true;
    }

    quint32 crc = 0xFFFFFFFFu;
    const unsigned char *p = reinterpret_cast<const unsigned char *>(data.constData());
    for (int i = 0; i < data.size(); ++i)
        crc = table[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

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

QString escapeXml(const QString &text)
{
    QString out;
    out.reserve(text.size() + 8);
    for (const QChar &c : text) {
        switch (c.unicode()) {
        case u'&':  out += QLatin1String("&amp;");   break;
        case u'<':  out += QLatin1String("&lt;");    break;
        case u'>':  out += QLatin1String("&gt;");    break;
        case u'"':  out += QLatin1String("&quot;");  break;
        case u'\'': out += QLatin1String("&apos;");  break;
        default:
            // 丢弃 XML 1.0 非法控制字符（保留 \t \n \r）
            if (c.unicode() >= 0x20 || c.unicode() == 0x09 || c.unicode() == 0x0A || c.unicode() == 0x0D)
                out += c;
            break;
        }
    }
    return out;
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
                   + XlsxDetail::escapeXml(cell.value).toUtf8() + "</t></is></c>";
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

    struct Entry {
        const char *name;
        QByteArray data;
    };
    Entry entries[] = {
        { "[Content_Types].xml",        buildContentTypes() },
        { "_rels/.rels",                buildRootRels() },
        { "docProps/app.xml",           QByteArray(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/"
            "extended-properties\" xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/"
            "2006/docPropsVTypes\"><Application>deepin-devicemanager</Application></Properties>") },
        { "xl/workbook.xml",            buildWorkbook() },
        { "xl/_rels/workbook.xml.rels", buildWorkbookRels() },
        { "xl/styles.xml",              buildStyles() },
        { "xl/worksheets/sheet1.xml",   buildSheet() },
    };
    const int entryCount = sizeof(entries) / sizeof(entries[0]);

    // 时间戳（DOS 格式）
    const QDateTime now = QDateTime::currentDateTime();
    const QTime t = now.time();
    const QDate d = now.date();
    const quint16 dosTime = quint16((t.hour() << 11) | (t.minute() << 5) | (t.second() / 2));
    const quint16 dosDate = quint16(((qMax(1980, d.year()) - 1980) << 9) | (d.month() << 5) | d.day());

    QByteArray out;
    QByteArray central;

    auto appendLe16 = [](QByteArray &buf, quint16 v) {
        buf += char(v & 0xFF);
        buf += char((v >> 8) & 0xFF);
    };
    auto appendLe32 = [](QByteArray &buf, quint32 v) {
        buf += char(v & 0xFF);
        buf += char((v >> 8) & 0xFF);
        buf += char((v >> 16) & 0xFF);
        buf += char((v >> 24) & 0xFF);
    };

    for (int i = 0; i < entryCount; ++i) {
        const QByteArray name = QByteArray(entries[i].name);
        const QByteArray &data = entries[i].data;
        const quint32 crc = XlsxDetail::crc32(data);
        const quint32 offset = quint32(out.size());

        // local file header（STORE：method=0，压缩前后大小一致）
        out += QByteArray("PK\x03\x04", 4);
        appendLe16(out, 20);                  // version needed
        appendLe16(out, 0);                   // flags
        appendLe16(out, 0);                   // method: STORE
        appendLe16(out, dosTime);
        appendLe16(out, dosDate);
        appendLe32(out, crc);
        appendLe32(out, quint32(data.size()));   // compressed size
        appendLe32(out, quint32(data.size()));   // uncompressed size
        appendLe16(out, quint16(name.size()));
        appendLe16(out, 0);                   // extra len
        out += name;
        out += data;

        // central directory entry
        central += QByteArray("PK\x01\x02", 4);
        appendLe16(central, 20);              // version made by
        appendLe16(central, 20);              // version needed
        appendLe16(central, 0);               // flags
        appendLe16(central, 0);               // method: STORE
        appendLe16(central, dosTime);
        appendLe16(central, dosDate);
        appendLe32(central, crc);
        appendLe32(central, quint32(data.size()));
        appendLe32(central, quint32(data.size()));
        appendLe16(central, quint16(name.size()));
        appendLe16(central, 0);               // extra len
        appendLe16(central, 0);               // comment len
        appendLe16(central, 0);               // disk number start
        appendLe16(central, 0);               // internal attrs
        appendLe32(central, 0);               // external attrs
        appendLe32(central, offset);
        central += name;
    }

    // end of central directory
    const quint32 cdOffset = quint32(out.size());
    const quint32 cdSize = quint32(central.size());
    out += central;
    out += QByteArray("PK\x05\x06", 4);
    out += char(0); out += char(0);           // disk number
    out += char(0); out += char(0);           // disk with cd
    out += char(entryCount & 0xFF); out += char((entryCount >> 8) & 0xFF);
    out += char(entryCount & 0xFF); out += char((entryCount >> 8) & 0xFF);
    out += char(cdSize & 0xFF); out += char((cdSize >> 8) & 0xFF);
    out += char((cdSize >> 16) & 0xFF); out += char((cdSize >> 24) & 0xFF);
    out += char(cdOffset & 0xFF); out += char((cdOffset >> 8) & 0xFF);
    out += char((cdOffset >> 16) & 0xFF); out += char((cdOffset >> 24) & 0xFF);
    out += char(0); out += char(0);           // comment len

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    if (file.write(out) != out.size()) {
        file.cancelWriting();
        return false;
    }
    return file.commit();
}
