// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusTouchPad.h"
#include "MacroDefinition.h"
#include "DDLog.h"
#include <DSysInfo>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>
#include <QDBusInterface>

using namespace DDLog;

const QString Service_V23 = "org.deepin.dde.InputDevices1";
const QString Path_V23 = "/org/deepin/dde/InputDevice1/TouchPad";
const QString Interface_V23 = "org.deepin.dde.InputDevice1.TouchPad";

const QString Service_V20 = "com.deepin.daemon.InputDevices";
const QString Path_V20 = "/com/deepin/daemon/InputDevice/TouchPad";
const QString Interface_V20 = "com.deepin.daemon.InputDevice.TouchPad";

inline bool isV20() { return Dtk::Core::DSysInfo::majorVersion() == "20"; }
const QString Service = isV20() ? Service_V20 : Service_V23;
const QString Path = isV20() ? Path_V20 : Path_V23;
const QString Interface = isV20() ? Interface_V20 : Interface_V23;

DBusTouchPad *DBusTouchPad::sInstance = nullptr;
DBusTouchPad::DBusTouchPad()
    : QObject(nullptr)
    , m_dbusTouchPad(new QDBusInterface(Service, Path, Interface, QDBusConnection::sessionBus(), this))
{
    qCDebug(appLog) << "DBusTouchPad constructor";
}

DBusTouchPad::~DBusTouchPad()
{
    qCDebug(appLog) << "DBusTouchPad destructor";
    DELETE_PTR(m_dbusTouchPad);
}
/**
* @brief 是否存在触摸板
* @return 布尔值，true:存在；false:不存在
*/
bool DBusTouchPad::isExists()
{
    qCDebug(appLog) << "Check touchpad existence";
    if (m_dbusTouchPad->isValid()) {
        return m_dbusTouchPad->property("Exist").toBool();
    }
    qCWarning(appLog) << "Invalid DBus interface when checking touchpad existence";
    return false;
}

/**
* @brief 设置触摸板状态
* @bool 布尔值，设置触摸板状态，true:使能状态；false:去使能状态
*/
void DBusTouchPad::setEnable(bool state)
{
    qCDebug(appLog) << "Set touchpad enable state to:" << state;
    if (m_dbusTouchPad->isValid()) {
        m_dbusTouchPad->setProperty("TPadEnable", state);
        qCInfo(appLog) << "Touchpad state changed to:" << state;
    } else {
        qCWarning(appLog) << "Invalid DBus interface when setting touchpad state";
    }
}

/**
* @brief 获取触摸板状态
* @return 布尔值，返回触摸板当前状态，true:使能状态；false:去使能状态
*/
bool DBusTouchPad::getEnable()
{
    qCDebug(appLog) << "Get touchpad enable state";
    if (m_dbusTouchPad->isValid()) {
        return m_dbusTouchPad->property("TPadEnable").toBool();
    }
    qCWarning(appLog) << "Invalid DBus interface when getting touchpad state";
    return false;
}
