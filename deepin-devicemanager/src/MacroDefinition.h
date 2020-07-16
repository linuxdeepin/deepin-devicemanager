/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     liujun <liujuna@uniontech.com>
 *
 * Maintainer: liujun <liujuna@uniontech.com>
 * Maintainer: jixiaomei <jixiaomei@uniontech.com>
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
    /**添加每个设备的信息**/\
    foreach (auto device, deviceLst) {                                              \
        out << "\n";                                                                \
        \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size() > 1) {                                                 \
            device->getBaseAttribs();                                               \
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
    /**添加每个设备的信息**/                                                           \
    foreach (auto device, deviceLst) {                                              \
        device->getBaseAttribs();                                                   \
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
    /**添加每个设备的信息**/                                                           \
    foreach (auto device, deviceLst) {                                              \
        \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size() > 1) {                                                 \
            device->getBaseAttribs();                                               \
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
    htmlFile.write("<!DOCTYPE html>\n");                                            \
    htmlFile.write("<html>\n");                                                     \
    htmlFile.write("<body>\n");                                                     \
    \
    /**添加设备类型**/                                                                \
    htmlFile.write((QString("<h2>") + "[" + type + "]" + "</h2>").toUtf8());        \
    \
    /**无设备添加提示信息**/                                                           \
    if (deviceLst.size() < 1) {                                                     \
        htmlFile.write((QString("<h2>") + msg + "</h2>").toUtf8());                 \
    }                                                                               \
    \
    /**添加每个设备的信息**/                                                           \
    foreach (auto device, deviceLst) {                                              \
        QDomDocument doc;                                                           \
        \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size()) {                                                     \
            device->getBaseAttribs();                                               \
            device->subTitleToHTML(doc);                                            \
        }                                                                           \
        \
        /**添加设备的详细信息**/                                                       \
        device->toHtmlString(doc);\
        doc.appendChild(doc.createElement("br"));                                   \
        \
        htmlFile.write(doc.toString().toStdString().data());                        \
        \
        htmlFile.write("</body>\n");                                                \
        htmlFile.write("</html>\n");                                                \
    }                                                                               \
    \

