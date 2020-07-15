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

#define GET_BASE_INFO(out, pDevice) \
    foreach (auto item, pDevice->getBaseAttribs()) {\
        out << "\n";\
        out << "";\
    }\


#define GET_OTHER_INFO(pDevice, name, value) \
    \


#define EXPORT_TO_TXT(out, deviceLst, type) \
    out <<  "[" << type << "]\n-------------------------------------------------"; \
    foreach (auto device, deviceLst) {\
        out << "\n";\
        out << device->subTitle();\
        \
    }\

