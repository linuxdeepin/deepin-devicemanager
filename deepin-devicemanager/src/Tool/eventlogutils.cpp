// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QLibrary>
#include "DDLog.h"
#include <QDir>
#include <QLibraryInfo>
#include <QJsonDocument>

#include "eventlogutils.h"

using namespace DDLog;

EventLogUtils *EventLogUtils::mInstance(nullptr);

EventLogUtils &EventLogUtils::get()
{
    if (mInstance == nullptr) {
        mInstance = new EventLogUtils;
    }
    return *mInstance;
}

EventLogUtils::EventLogUtils()
{
    qCDebug(appLog) << "Initializing EventLogUtils";

    QLibrary library("libdeepin-event-log.so");

    init =reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLog = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));

    if (init == nullptr) {
        qCWarning(appLog) << "Failed to resolve Initialize function";
        return;
    }

    init("deepin-devicemanager", true);
}

void EventLogUtils::writeLogs(QJsonObject &data)
{
    qCDebug(appLog) << "Writing event log";

    if (writeEventLog == nullptr)
        qCWarning(appLog) << "WriteEventLog function not resolved, skipping log write";
        return;

    //std::string str = QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString();
    writeEventLog(QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
}
