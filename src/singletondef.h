/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
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

/*  Best Singleton
    noncopyable
    use stack, not new from heap
    easy use
*/

#define Declare_Singleton(classname) \
public: \
static classname& Instance() \
{ \
    static classname _instance; \
    return _instance; \
} \
protected: \
    classname() = default; \
    ~classname() = default; \
                            \
    classname(const classname& rInstance); \
    classname& operator = (const classname& rInstance) \
\
