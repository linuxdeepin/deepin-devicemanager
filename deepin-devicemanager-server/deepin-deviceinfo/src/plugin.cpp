// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainjob.h"
#include "DDLog.h"

#include <QLoggingCategory>

using namespace DDLog;

static MainJob *mainJob = nullptr;

extern "C" int DSMRegister(const char *name, void *data)
{
    qCDebug(appLog) << "Enter DSMRegister, name:" << name;
    (void)data;
    mainJob = new MainJob(name);
    return 0;
}

extern "C" int DSMUnRegister(const char *name, void *data)
{
    qCDebug(appLog) << "Enter DSMUnRegister, name:" << name;
    (void)name;
    (void)data;
    mainJob->deleteLater();
    mainJob = nullptr;
    return 0;
}
