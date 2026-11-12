// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZIPWRITER_H
#define ZIPWRITER_H

#include <QByteArray>
#include <QList>
#include <QString>

// 内部实现辅助：独立声明便于单元测试直接验证（不属于公开 API）
namespace ZipDetail {
quint32 crc32(const QByteArray &data);        // 标准 CRC-32（IEEE 802.3，多项式 0xEDB88320）
QString escapeXml(const QString &text);       // XML 1.0 转义，非法控制字符丢弃
}

/**
 * @brief ZipWriter:零三方依赖的极简 ZIP 容器写入器（OOXML 包共用）
 *
 * 仅依赖 QtCore 公开 API：
 *  - 条目采用 STORE（不压缩）方式，CRC-32 手写实现，无 zlib 依赖
 *  - XML/文本部件体积小，STORE 不压缩的体积开销可忽略
 *  - saveAs 使用 QSaveFile 原子写，失败不留半截文件
 *
 * 供 XlsxWriter / DocxWriter 共用。
 */
class ZipWriter
{
public:
    ZipWriter() = default;

    /**
     * @brief addFile:向容器添加一个条目（同名后添加者覆盖先添加者）
     * @param name:条目名（如 "word/document.xml"）
     * @param data:条目内容
     */
    void addFile(const QString &name, const QByteArray &data);

    /**
     * @brief saveAs:组装 ZIP 结构（local header + central directory + EOCD）并落盘
     * @param filePath:目标文件路径
     * @return true:写出成功，false:文件无法写入
     */
    bool saveAs(const QString &filePath);

private:
    struct Entry {
        QString name;
        QByteArray data;
    };

    QList<Entry> m_entries;
};

#endif   // ZIPWRITER_H
