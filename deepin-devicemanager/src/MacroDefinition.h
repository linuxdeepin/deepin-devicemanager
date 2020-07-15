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

#define TXT_BASE_INFO(out, device) \
    foreach (auto item, device->getBaseAttribs()) {\
        QString value = item.second;\
        if (device->isValueValid(value) == false) {\
            continue;\
        }\
        \
        out.setFieldWidth(21); \
        out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft); \
        out << item.first + ": "; \
        out.setFieldWidth(0); \
        out << item.second; \
        out << "\n"; \
    }\


#define TXT_OTHER_INFO(out, device) \
    QMap<QString,QString>::const_iterator iter = device->getOtherAttribs().begin(); \
    for (;iter != device->getOtherAttribs().end(); ++iter) { \
        QString str = iter.value(); \
        if (device->isValueValid(str) == false) { \
            continue; \
        } \
        \
        out.setFieldWidth(21); \
        out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft); \
        out << iter.key() + ": "; \
        out.setFieldWidth(0); \
        out << iter.value(); \
        out << "\n"; \
    } \

#define EXPORT_TO_TXT(out, deviceLst, type, msg) \
    out <<  "[" << type << "]\n-------------------------------------------------"; \
    if (deviceLst.size() < 1) { \
        out << "\n"; \
        out << msg; \
    }\
    \
    foreach (auto device, deviceLst) { \
        out << "\n"; \
        \
        if (deviceLst.size() > 1) { \
            device->getBaseAttribs(); \
            out << device->subTitle(); \
            out << "\n";\
        }\
        \
        TXT_BASE_INFO(out,device); \
        TXT_OTHER_INFO(out,device); \
    }\
    \
    out << "\n"; \

#define XLS_BASE_INFO(xlsx, device) \
    QXlsx::Format boldFont;\
    boldFont.setFontBold(true);\
    boldFont.setFontSize(10);\
    \
    foreach (auto item, device->getBaseAttribs()) {\
        QString value = item.second;\
        if (device->isValueValid(value) == false) {\
            continue;\
        }\
        \
        xlsx.write(m_CurrentXlsRow, 1, item.first, setFontBold);\
        xlsx.write(m_CurrentXlsRow++, 2, item.second, setFontBold);\
    }\

#define XLS_OTHER_INFO(xlsx, device) \
    QXlsx::Format boldFont;\
    boldFont.setFontBold(true);\
    boldFont.setFontSize(10);\
    \
    QMap<QString,QString>::const_iterator iter = device->getOtherAttribs().begin(); \
    for (;iter != device->getOtherAttribs().end(); ++iter) { \
        QString str = iter.value(); \
        if (device->isValueValid(str) == false) { \
            continue; \
        } \
        \
        xlsx.write(m_CurrentXlsRow, 1, iter.key(), setFontBold);\
        xlsx.write(m_CurrentXlsRow++, 2, iter.value(), setFontBold);\
    } \


#define EXPORT_TO_XLSX(xlsx, deviceLst, type, msg) \
    QXlsx::Format boldFont;\
    boldFont.setFontBold(true);\
    xlsx.write(m_CurrentXlsRow++, 1, "[" << type << "]", setFontBold); \
    \
    if (deviceLst.size() < 1) { \
        xlsx.write(m_CurrentXlsRow++, 1, msg, setFontBold); \
    }\
    \
    foreach (auto device, deviceLst) { \
        \
        if (deviceLst.size() > 1) { \
            boldFont.setFontSize(10);\
            device->getBaseAttribs(); \
            xlsx.write(m_CurrentXlsRow++, 1, device->subTitle(), setFontBold);\
        }\
        \
        XLS_BASE_INFO(out,device); \
        XLS_OTHER_INFO(out,device); \
    }\
    \


