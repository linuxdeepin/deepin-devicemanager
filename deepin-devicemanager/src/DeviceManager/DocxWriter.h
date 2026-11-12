// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCXWRITER_H
#define DOCXWRITER_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QPair>
#include <QString>
#include <QVector>

class DocxTable;

/**
 * @brief DocxWriter:零三方依赖的极简 docx（OOXML WordprocessingML）写入器
 *
 * 替代 3rdparty/docx（python-docx 的 Qt 移植，依赖 Qt 私有 API
 * private/qzipreader_p.h / qzipwriter_p.h），仅覆盖本项目导出用到的 API 子集：
 *  - addHeading / addParagraph / addTable / Table::addColumn / Table::addRow /
 *    Table::cell / Cell::addText / save
 *  - 不再依赖模板文件，样式（Normal/Heading1/Heading2）内建于 word/styles.xml，
 *    页面设置沿用原模板（A4，上下边距 1440 缇，左右 1800 缇）
 *
 * 仅依赖 QtCore 公开 API：ZIP 容器复用 ZipWriter（STORE 条目 + 手写 CRC-32），
 * XML 部件用 QString 拼装。生成的文件可被 Word / WPS / LibreOffice 正常打开。
 */
class DocxWriter
{
public:
    DocxWriter();
    ~DocxWriter();

    DocxWriter(const DocxWriter &) = delete;
    DocxWriter &operator=(const DocxWriter &) = delete;

    /**
     * @brief addHeading:添加标题段落（内置 Heading1/Heading2 样式，带大纲级别）
     * @param text:标题文本
     * @param level:标题级别，1/2 映射 Heading1/Heading2，其余级别按 2 处理
     */
    void addHeading(const QString &text, int level = 1);

    /**
     * @brief addParagraph:添加正文段落（Normal 样式）
     * @param text:文本，自动做 XML 转义
     */
    void addParagraph(const QString &text);

    /**
     * @brief addTable:添加表格（行列为初始规模，可继续 addRow/addColumn 扩展）
     * @param rows:初始行数（负数按 0 处理）
     * @param cols:初始列数（负数按 0 处理）
     * @return 表格指针（生命周期由 DocxWriter 管理）
     */
    DocxTable *addTable(int rows, int cols);

    /**
     * @brief save:组装 OOXML 部件并以 STORE ZIP 打包落盘（QSaveFile 原子写）
     * @param filePath:目标文件路径
     * @return true:导出成功，false:文件无法写入
     */
    bool save(const QString &filePath);

private:
    struct Block {
        bool isTable = false;
        QString text;          // 段落文本
        QString style;         // 段落样式（Heading1/Heading2/Normal）
        DocxTable *table = nullptr;
    };

    QList<Block> m_blocks;
    QList<DocxTable *> m_tables;   // 拥有表格所有权

    QByteArray buildContentTypes() const;
    QByteArray buildRootRels() const;
    QByteArray buildDocumentRels() const;
    QByteArray buildCore() const;
    QByteArray buildApp() const;
    QByteArray buildDocument() const;
    QByteArray buildStyles() const;
};

/**
 * @brief DocxCell:单元格写入代理（句柄，写入转发给所属表格）
 */
class DocxCell
{
public:
    /**
     * @brief addText:设置单元格文本（自动做 XML 转义，覆盖旧值）
     */
    void addText(const QString &text);

    DocxCell() = default;

private:
    friend class DocxTable;

    explicit DocxCell(DocxTable *table, int row, int col)
        : m_table(table), m_row(row), m_col(col) {}

    DocxTable *m_table = nullptr;
    int m_row = 0;
    int m_col = 0;
};

/**
 * @brief DocxTable:docx 表格，单元格文本按 (行, 列) 寻址
 */
class DocxTable
{
public:
    /**
     * @brief addColumn:追加一列（所有已有行补齐空单元格）
     */
    void addColumn();

    /**
     * @brief addRow:追加一行（按当前列数补齐空单元格）
     */
    void addRow();

    /**
     * @brief cell:取得指定单元格（越界返回 nullptr；重复取同一格返回同一指针）
     * @param row:行号（0 基）
     * @param col:列号（0 基）
     */
    DocxCell *cell(int row, int col);

    /**
     * @brief rowCount/columnCount:当前行数/列数
     */
    int rowCount() const { return m_cells.size(); }
    int columnCount() const { return m_cols; }

private:
    friend class DocxWriter;
    friend class DocxCell;

    explicit DocxTable(int rows, int cols);

    void setCellText(int row, int col, const QString &text);
    QByteArray toXml() const;

    QVector<QVector<QString>> m_cells;   // 行 -> 列 -> 文本
    int m_cols = 0;
    QHash<QPair<int, int>, DocxCell> m_cellPool;   // 稳定地址的单元格代理
};

#endif   // DOCXWRITER_H
