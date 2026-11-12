// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef XLSXWRITER_H
#define XLSXWRITER_H

#include <QMap>
#include <QString>

// 内部实现辅助：独立声明便于单元测试直接验证（不属于公开 API）
namespace XlsxDetail {
quint32 crc32(const QByteArray &data);        // 标准 CRC-32（IEEE 802.3，多项式 0xEDB88320）
QString columnName(int column);               // 1 基列号 -> A/.../Z/AA/... 样式
QString escapeXml(const QString &text);       // XML 1.0 转义，非法控制字符丢弃
}

/**
 * @brief XlsxFormat:单元格字体格式（对齐 QXlsx::Format 用到的最小子集）
 *        仅支持加粗与字号，字号 0 表示未指定（写入时省略 sz，遵循 Excel 默认值）
 */
class XlsxFormat
{
public:
    XlsxFormat() = default;

    void setFontBold(bool bold) { m_fontBold = bold; }
    void setFontSize(int size) { m_fontSize = size; }
    bool fontBold() const { return m_fontBold; }
    int fontSize() const { return m_fontSize; }

    bool operator==(const XlsxFormat &other) const
    {
        return m_fontBold == other.m_fontBold && m_fontSize == other.m_fontSize;
    }
    bool operator!=(const XlsxFormat &other) const { return !(*this == other); }

private:
    bool m_fontBold = false;
    int m_fontSize = 0;
};

/**
 * @brief XlsxWriter:零三方依赖的极简 xlsx（OOXML）写入器
 *
 * 仅依赖 QtCore 公开 API：
 *  - ZIP 容器采用 STORE（不压缩）条目，CRC-32 手写实现，无 zlib 依赖
 *  - XML 部件用 QString 拼装，字符串单元格使用 inlineStr，免去共享字符串表
 *  - 样式按实际使用的 (加粗, 字号) 组合动态生成，内联 Calibri 字体
 *
 * 生成的文件可被 Excel / WPS / LibreOffice 正常打开。
 */
class XlsxWriter
{
public:
    XlsxWriter() = default;

    /**
     * @brief write:写入一个字符串单元格（行列均为 1 基，越界时截断到 1）
     * @param row:行号（1 基）
     * @param column:列号（1 基）
     * @param value:单元格文本（自动做 XML 转义）
     * @param format:字体格式，缺省为默认格式
     * @return 总是返回 true（越界截断，覆盖同格旧值）
     */
    bool write(int row, int column, const QString &value, const XlsxFormat &format = XlsxFormat());

    /**
     * @brief saveAs:组装 OOXML 部件并以 STORE ZIP 打包落盘（QSaveFile 原子写）
     * @param filePath:目标文件路径
     * @return true:导出成功，false:文件无法写入
     */
    bool saveAs(const QString &filePath);

private:
    struct Cell {
        QString value;
        XlsxFormat format;
    };

    QMap<int, QMap<int, Cell>> m_cells;   // 行号 -> (列号 -> 单元格)，写入即覆盖

    QByteArray buildContentTypes() const;
    QByteArray buildRootRels() const;
    QByteArray buildWorkbook() const;
    QByteArray buildWorkbookRels() const;
    QByteArray buildStyles() const;
    QByteArray buildSheet() const;
};

#endif   // XLSXWRITER_H
