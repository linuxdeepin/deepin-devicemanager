// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZipWriter.h"

#include <QDate>
#include <QSaveFile>
#include <QTime>

#include <algorithm>

// ============================================================
// 内部工具：CRC-32 / XML 转义（独立成自由函数便于单测直测）
// ============================================================
namespace ZipDetail {

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

}   // namespace ZipDetail

// ============================================================
// ZipWriter
// ============================================================
void ZipWriter::addFile(const QString &name, const QByteArray &data)
{
    const auto it = std::find_if(m_entries.begin(), m_entries.end(),
                                 [&name](const Entry &entry) { return entry.name == name; });
    if (it != m_entries.end()) {
        it->data = data;
        return;
    }
    m_entries.append(Entry { name, data });
}

bool ZipWriter::saveAs(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    const int entryCount = m_entries.size();

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
        const QByteArray name = m_entries[i].name.toUtf8();
        const QByteArray &data = m_entries[i].data;
        const quint32 crc = ZipDetail::crc32(data);
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
