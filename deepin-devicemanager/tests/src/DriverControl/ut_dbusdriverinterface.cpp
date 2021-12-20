/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "DBusDriverInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QDBusInterface>
#include <QDBusAbstractInterface>
#include <QDBusPendingCall>
#include <QtDBus>
#include <QtDBus/qdbusreply.h>
#include <QtDBus/qdbuserror.h>

#include <gtest/gtest.h>

class UT_DBusDriverInterface : public UT_HEAD
{
public:
    void SetUp()
    {
        pDriver = DBusDriverInterface::getInstance();
    }
    void TearDown()
    {
    }
    DBusDriverInterface *pDriver = nullptr;
};

QDBusMessage ut_QDBusInterface_call(const QString &method,
                  const QVariant &arg1 = QVariant(),
                  const QVariant &arg2 = QVariant(),
                  const QVariant &arg3 = QVariant(),
                  const QVariant &arg4 = QVariant(),
                  const QVariant &arg5 = QVariant(),
                  const QVariant &arg6 = QVariant(),
                  const QVariant &arg7 = QVariant(),
                  const QVariant &arg8 = QVariant())
{
    QDBusMessage msg;
    //msg = msg.createErrorReply(QDBusError::ErrorType::NoError, "true");//ErrorType::NoError
    msg = msg.createReply(true);//ErrorType::NoError
    return msg;
}
QDBusPendingCall ut_QDBusInterface_asyncCall(const QString &method,
                           const QVariant &arg1 = QVariant(),
                           const QVariant &arg2 = QVariant(),
                           const QVariant &arg3 = QVariant(),
                           const QVariant &arg4 = QVariant(),
                           const QVariant &arg5 = QVariant(),
                           const QVariant &arg6 = QVariant(),
                           const QVariant &arg7 = QVariant(),
                           const QVariant &arg8 = QVariant())
{
    QDBusMessage msg;
    msg = msg.createReply(true);
    QDBusPendingCall asyn = QDBusPendingCall::fromCompletedCall(msg);
    return asyn;
}
int intCount = 0;
void ut_slotCallFinished(QDBusPendingCallWatcher* watcher)
{
    intCount = 1;
    return;
}

TEST_F(UT_DBusDriverInterface,UT_DBusDriverInterface_installDriver){
    Stub stub;
    QDBusMessage (QDBusInterface::*p)(const QString&, const QVariant&, const QVariant&, const QVariant&, const QVariant&
                      , const QVariant&, const QVariant&, const QVariant&, const QVariant&) = &QDBusInterface::call;
    stub.set(p, ut_QDBusInterface_call);
    bool isKo = pDriver->isDriverPackage("/home/uos/test.ko");
    EXPECT_TRUE(isKo);

    QDBusPendingCall (QDBusInterface::*pa)(const QString&, const QVariant&, const QVariant&, const QVariant&, const QVariant&
                      , const QVariant&, const QVariant&, const QVariant&, const QVariant&) = &QDBusInterface::asyncCall;
    stub.set(pa, ut_QDBusInterface_asyncCall);
    pDriver->installDriver("/home/uos/test.ko");
    QDBusPendingCallWatcher *cw = this->pDriver->findChild<QDBusPendingCallWatcher *>();
    emit cw->finished(cw);

    intCount = 0;
    stub.set(ADDR(DBusDriverInterface, slotCallFinished), ut_slotCallFinished);
    pDriver->uninstallDriver("/home/uos/test.ko");
    cw = this->pDriver->findChild<QDBusPendingCallWatcher *>();
    emit cw->finished(cw);
    EXPECT_TRUE(1 == intCount);
}

TEST_F(UT_DBusDriverInterface,UT_DBusDriverInterface_uninstallPrinter){
    Stub stub;

    intCount = 0;
    QDBusPendingCall (QDBusInterface::*pa)(const QString&, const QVariant&, const QVariant&, const QVariant&, const QVariant&
                      , const QVariant&, const QVariant&, const QVariant&, const QVariant&) = &QDBusInterface::asyncCall;
    stub.set(pa, ut_QDBusInterface_asyncCall);
    stub.set(ADDR(DBusDriverInterface, slotCallFinished), ut_slotCallFinished);
    pDriver->uninstallPrinter("Canon","iR-ADV C3720 22.21");
    QDBusPendingCallWatcher *cw = this->pDriver->findChild<QDBusPendingCallWatcher *>();
    emit cw->finished();
    EXPECT_TRUE(1 == intCount);
}
TEST_F(UT_DBusDriverInterface,UT_DBusDriverInterface_archMatch){
    Stub stub;
    QDBusMessage (QDBusInterface::*p)(const QString&, const QVariant&, const QVariant&, const QVariant&, const QVariant&
                      , const QVariant&, const QVariant&, const QVariant&, const QVariant&) = &QDBusInterface::call;
    stub.set(p, ut_QDBusInterface_call);
    bool isKo = pDriver->isArchMatched("/home/uos/e1000e.ko");
    EXPECT_TRUE(isKo);
    isKo = pDriver->isDebValid("/home/uos/e1000e.deb");
    EXPECT_TRUE(isKo);
}

