/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <QString>

/**@brief:定义一个删除指针的宏*/
#define DELETE_PTR(p) \
    if (p) { \
        delete p; \
        p = nullptr; \
    } \

/**
 * @brief EXPORT_TO_TXT:导出设备信息到txt
 * @param htmlFile:txt文件
 * @param deviceLst:设备列表
 * @param type:设备类型
 * @param msg:没有设备时的提示信息
 */
#define EXPORT_TO_TXT(out, deviceLst, type, msg) \
    /**添加设备类型**/                                                                \
    out <<  "[" << type << "]\n-------------------------------------------------";  \
    \
    /**无设备添加提示信息**/                                                           \
    if (deviceLst.size() < 1) {                                                     \
        out << "\n";                                                                \
        out << msg;                                                                 \
    }                                                                               \
    \
    /**添加Table信息**/                                                              \
    if (deviceLst.size() > 1) {                                                     \
        deviceLst[0]->tableHeaderToTxt(out);                                        \
        foreach (auto device, deviceLst) {                                          \
            device->tableInfoToTxt(out);                                            \
        }                                                                           \
    }                                                                               \
    \
    /**添加每个设备的信息**/\
    foreach (auto device, deviceLst) {                                              \
        out << "\n";                                                                \
        \
        device->getBaseAttribs();                                                   \
        device->getOtherAttribs();                                                  \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size() > 1) {                                                 \
            out << device->subTitle();                                              \
            out << "\n";                                                            \
        }                                                                           \
        \
        /**添加设备的详细信息**/                                                       \
        device->toTxtString(out);\
    }                                                                               \
    \
    out << "\n";                                                                    \


/**
 * @brief EXPORT_TO_XLSX:导出设备信息到xlsx
 * @param boldFont:字体格式
 * @param htmlFile:xlsx文件
 * @param deviceLst:设备列表
 * @param type:设备类型
 * @param msg:没有设备时的提示信息
 */
#define EXPORT_TO_XLSX(xlsx, boldFont, deviceLst, type, msg)                        \
    /**添加设备类型**/                                                                \
    boldFont.setFontBold(true);                                                     \
    xlsx.write(m_CurrentXlsRow++, 1, "[" + type + "]", boldFont);                   \
    \
    /**无设备添加提示信息**/                                                           \
    if (deviceLst.size() < 1) {                                                     \
        xlsx.write(m_CurrentXlsRow++, 1, msg, boldFont);                            \
    }                                                                               \
    \
    /**添加Table信息**/                                                              \
    if (deviceLst.size() > 1) {                                                     \
        deviceLst[0]->tableHeaderToXlsx(xlsx);                                      \
        foreach (auto device, deviceLst) {                                          \
            device->tableInfoToXlsx(xlsx);                                          \
        }                                                                           \
    }                                                                               \
    \
    /**添加每个设备的信息**/                                                           \
    foreach (auto device, deviceLst) {                                              \
        device->getBaseAttribs();                                                   \
        device->getOtherAttribs();                                                  \
        \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size() > 1) {                                                 \
            boldFont.setFontSize(10);                                               \
            boldFont.setFontBold(true);                                             \
            \
            xlsx.write(m_CurrentXlsRow++, 1, device->subTitle(), boldFont);         \
        }                                                                           \
        \
        /**添加设备的详细信息**/                                                       \
        device->toXlsxString(xlsx, boldFont);                                       \
        m_CurrentXlsRow++;\
    }                                                                               \
    \

/**
 * @brief EXPORT_TO_DOC:导出设备信息到doc
 * @param htmlFile:doc文件
 * @param deviceLst:设备列表
 * @param type:设备类型
 * @param msg:没有设备时的提示信息
 */
#define EXPORT_TO_DOC(doc, deviceLst, type, msg)                                    \
    /**添加设备类型**/                                                                \
    doc.addHeading("[" + type + "]", 2);                                            \
    doc.addParagraph("-------------------------------------------------");          \
    \
    /**无设备添加提示信息**/                                                           \
    if (deviceLst.size() < 1) {                                                     \
        doc.addParagraph(msg);                                                      \
    }                                                                               \
    \
    /**添加表格信息**/\
    if (deviceLst.size() > 1) {\
        Docx::Table *tab = doc.addTable(1,0);\
        deviceLst[0]->tableHeaderToDoc(tab);\
        for (int row = 1; row <= deviceLst.size(); ++row) {\
            tab->addRow();\
            deviceLst[row - 1]->tableInfoToDoc(tab, row);\
        }\
    }\
    /**添加每个设备的信息**/                                                           \
    foreach (auto device, deviceLst) {                                              \
        \
        device->getBaseAttribs();                                                   \
        device->getOtherAttribs();                                                  \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size() > 1) {                                                 \
            doc.addParagraph(device->subTitle());                                   \
        }                                                                           \
        \
        /**添加设备的详细信息**/                                                       \
        device->toDocString(doc);                                                   \
        \
        doc.addParagraph("\n");                                                     \
    }                                                                               \
    \

/**
 * @brief EXPORT_TO_HTML:导出设备信息到html
 * @param htmlFile:html文件
 * @param deviceLst:设备列表
 * @param type:设备类型
 * @param msg:没有设备时的提示信息
 */
#define EXPORT_TO_HTML(htmlFile, deviceLst, type, msg)                              \
    \
    /**添加设备类型**/                                                                \
    htmlFile.write((QString("<h2>") + "[" + type + "]" + "</h2>").toUtf8());        \
    \
    /**无设备添加提示信息**/                                                           \
    if (deviceLst.size() < 1) {                                                     \
        htmlFile.write((QString("<h2>") + msg + "</h2>").toUtf8());                 \
    }                                                                               \
    \
    /**添加表格信息**/\
    if(deviceLst.size() > 1) {\
        html.write("<table border=\"0\" white-space:pre>\n");\
        deviceLst[0]->tableHeaderToHtml(html);\
        foreach(auto device, deviceLst) {\
            device->tableInfoToHtml(html);\
        }\
        html.write("</table>\n");\
    }\
    \
    /**添加每个设备的信息**/                                                           \
    foreach (auto device, deviceLst) {                                              \
        QDomDocument doc;                                                           \
        \
        device->getBaseAttribs();                                                   \
        device->getOtherAttribs();                                                  \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size() > 1) {                                                     \
            device->subTitleToHTML(doc);                                            \
        }                                                                           \
        \
        /**添加设备的详细信息**/                                                       \
        device->toHtmlString(doc);\
        doc.appendChild(doc.createElement("br"));                                   \
        \
        htmlFile.write(doc.toString().toStdString().data());                        \
        \
    }                                                                               \
    \


//HeaderTableView的相关宏定义
#define   TABLE_HEIGHT       180   // Table的高度
#define   ROW_HEIGHT         40    // 每一行的高度
#define   LINE_WIDTH         1     // 表头的线宽
#define   ROW_NUM            4     // TreeWidget 默认行数
#define   TREE_ROW_HEIGHT    36    // TreeWidget 行高
#define   WIDGET_MARGIN      2     // 页边距
#define   BOTTOM_MARGIN      11    // 下边距
#define   LABEL_MARGIN       12    // 页面标签的上下边距
#define   HORSCROLL_WIDTH    11    // TreeWidget 横向滚动条高度
#define   MIN_NUM            5     // num

const QString LINK_STR = "<a style=\"text-decoration:none\" href=https://www.chinauos.com/home>";        // uos官网链接
const QString COMMUNITY_LINK_STR = "<a style=\"text-decoration:none\" href=https://www.deepin.org/zh>";  // community uos官网链接
const QString PROF_STR = "UnionTech OS Desktop 20 Professional";                                         // 桌面专业版系统名称
const QString HOME_STR = "UnionTech OS Desktop 20 Home";                                                 // 个人版系统名称
const QString COMMUNITY_STR = "Deepin 20.1";                                                        // 社区版系统名称
const QString ENTERPRISE_STR = "UnionTech OS Server 20 Enterprise";                                      // 服务器企业版
const QString ENTERPRISEC_STR = "UnionTech OS Server 20 Enterprise-C";                                   // 服务器行业版
const QString EULER_STR = "UnionTech OS Server 20 Euler";                                                // 服务器欧拉版
const QString DEFAULT_STR = "UnionTech OS";                                                              // 默认值
const QString END_STR = " </a>";                                                                         // end html
