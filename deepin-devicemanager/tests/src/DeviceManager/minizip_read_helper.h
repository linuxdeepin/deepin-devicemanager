// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MINIZIP_READ_HELPER_H
#define MINIZIP_READ_HELPER_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>

// 测试专用：从 STORE（不压缩）ZIP 中按条目名提取内容。
// 仅适配 XlsxWriter 生成的 ZIP：无 extra 字段、无 data descriptor、无加密。
// 本辅助仅存在于测试目录，不参与主程序构建。
inline QByteArray utExtractStoredEntry(const QByteArray &zip, const QString &name)
{
    const QByteArray nameBytes = name.toUtf8();
    const int size = zip.size();
    int pos = 0;
    while (pos + 30 <= size) {
        if (zip.mid(pos, 4) != QByteArray("PK\x03\x04", 4))
            break;   // 到达 central directory 或数据损坏

        const quint16 nameLen = quint16(uchar(zip[pos + 26])) | (quint16(uchar(zip[pos + 27])) << 8);
        const quint16 extraLen = quint16(uchar(zip[pos + 28])) | (quint16(uchar(zip[pos + 29])) << 8);
        const quint32 dataLen = quint32(uchar(zip[pos + 18])) | (quint32(uchar(zip[pos + 19])) << 8)
                | (quint32(uchar(zip[pos + 20])) << 16) | (quint32(uchar(zip[pos + 21])) << 24);

        const QByteArray entryName = zip.mid(pos + 30, nameLen);
        const int dataPos = pos + 30 + nameLen + extraLen;
        if (entryName == nameBytes)
            return zip.mid(dataPos, int(dataLen));
        pos = dataPos + int(dataLen);
    }
    return QByteArray();
}

#endif   // MINIZIP_READ_HELPER_H
