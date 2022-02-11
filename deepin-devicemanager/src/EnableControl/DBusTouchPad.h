/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Libang.Liu <liulibang@uniontech.com>
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

#ifndef DBUSTOUCHPAD_H
#define DBUSTOUCHPAD_H

#include <com_deepin_daemon_inputdevice_mouse.h>
#include <com_deepin_daemon_inputdevice_touchpad.h>
#include <com_deepin_daemon_inputdevices.h>
#include <QObject>

using Mouse = com::deepin::daemon::inputdevice::Mouse;
using TouchPad = com::deepin::daemon::inputdevice::TouchPad;
using InputDevices = com::deepin::daemon::InputDevices;

class DBusTouchPad : public QObject
{
    Q_OBJECT
public:
    static DBusTouchPad *instance()
    {
        if (!sInstance) {
            sInstance = new DBusTouchPad;
        }
        return sInstance;
    }
    /**
    * @brief 是否存在触摸板
    * @return 布尔值，true:存在；false:不存在
    */
    bool isExists();
    /**
    * @brief 设置触摸板状态
    * @bool 布尔值，设置触摸板状态，true:使能状态；false:去使能状态
    */
    void setEnable(bool);
    /**
    * @brief 获取触摸板状态
    * @return 布尔值，返回触摸板当前状态，true:使能状态；false:去使能状态
    */
    bool getEnable();
protected:
    DBusTouchPad();
    ~DBusTouchPad();
private:
    static DBusTouchPad    *sInstance;
    TouchPad     *m_dbusTouchPad;   //mouse dbus对象
};

#endif // DBUSTOUCHPAD_H
