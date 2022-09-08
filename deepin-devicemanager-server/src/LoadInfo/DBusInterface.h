// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>
#include <QDBusContext>

class MainJob;
class DBusInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.devicemanager")
public:
    explicit DBusInterface(QObject *parent = nullptr);

signals:
    void update();

public slots:
    /**
     * @brief getInfo : Obtain hardware information through the DBus
     * @param key
     * @return : Hardware info
     */
    Q_SCRIPTABLE QString getInfo(const QString &key);

    /**
     * @brief refreshInfo
     * @return
     */
    Q_SCRIPTABLE void refreshInfo();
};

#endif // DBUSINTERFACE_H
