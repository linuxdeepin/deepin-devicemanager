// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusTouchPad.h"
#include "MacroDefinition.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
const QString Service = "org.deepin.dde.InputDevices1";
DBusTouchPad *DBusTouchPad::sInstance = nullptr;
DBusTouchPad::DBusTouchPad()
    :QObject(nullptr)
, m_dbusTouchPad(new TouchPad(Service, "/org/deepin/dde/InputDevice1/TouchPad", QDBusConnection::sessionBus(), this))
{
}

DBusTouchPad::~DBusTouchPad()
{
    DELETE_PTR(m_dbusTouchPad);
}
/**
* @brief 是否存在触摸板
* @return 布尔值，true:存在；false:不存在
*/
bool DBusTouchPad::isExists()
{
    return m_dbusTouchPad->exist();
}

/**
* @brief 设置触摸板状态
* @bool 布尔值，设置触摸板状态，true:使能状态；false:去使能状态
*/
void DBusTouchPad::setEnable(bool state)
{
    m_dbusTouchPad->setTPadEnable(state);
}

/**
* @brief 获取触摸板状态
* @return 布尔值，返回触摸板当前状态，true:使能状态；false:去使能状态
*/
bool DBusTouchPad::getEnable()
{
    return m_dbusTouchPad->tPadEnable();
}
