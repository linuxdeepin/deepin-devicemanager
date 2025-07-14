// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverScanner.h"
#include "DeviceManager.h"
#include "HttpDriverInterface.h"
#include "commonfunction.h"
#include "DDLog.h"

#include <QRegularExpression>
#include <QLoggingCategory>
#include <QProcess>

#include <unistd.h>

using namespace DDLog;

DriverScanner::DriverScanner(QObject *parent)
    : QThread(parent)
    , m_IsStop(false)
{
    qCDebug(appLog) << "DriverScanner constructor";
}

void DriverScanner::run()
{
    qCDebug(appLog) << "DriverScanner thread started";
    HttpDriverInterface *hdi  = HttpDriverInterface::getInstance();
    connect(hdi, &HttpDriverInterface::sigRequestFinished, this, [ = ]() {
        m_IsStop = true;
    });

    qCDebug(appLog) << "Start scanning" << m_ListDriverInfo.size() << "drivers";
    foreach (DriverInfo *info, m_ListDriverInfo) {
        if (!m_IsStop) {
            qCDebug(appLog) << "Processing driver:" << info->name();
            hdi->getRequest(info);

            // 检测本地安装版本
            if (!info->packages().isEmpty()) {
                // qCDebug(appLog) << "Checking local version for package:" << info->packages();
                QString outInfo = Common::executeClientCmd("apt", QStringList() << "policy" << info->packages(), QString(), -1, false);
                if (!outInfo.isEmpty()) {
                    // qCDebug(appLog) << "apt policy output:" << outInfo;
                    QStringList infoList = outInfo.split("\n");
                    int index = 0;
                    for (int i = 0; i < infoList.size(); i++)
                    {
                        if (infoList[i].startsWith(info->packages())) {
                            index = i;
                            // qCDebug(appLog) << "Found package info at index:" << index;
                            break;
                        }
                    }
                    if (infoList.size() > (2 + index) && !infoList[1 + index].contains("（") && !infoList[1 + index].contains("(")) {
                        QRegularExpression rxlen("(\\d+\\S*)");
                        QRegularExpressionMatch match = rxlen.match(infoList[1 + index]);
                        QString curVersion;
                        if (match.hasMatch()) {
                            curVersion = match.captured(1);
                            // qCDebug(appLog) << "Found current version:" << curVersion;
                        }
                        info->m_Version = curVersion.trimmed();
                    }
                }
            }

            // qCDebug(appLog) << "Driver scan progress:" << info->name() << "progress:" << 100 / m_ListDriverInfo.size();
            emit scanInfo(info->name(), 100 / m_ListDriverInfo.size());
            sleep(1);
        } else {
            qCWarning(appLog) << "Scan interrupted for driver:" << info->name();
            emit scanFinished(SR_NETWORD_ERR);
        }
    }

    if (!m_IsStop) {
        // 扫描结束
        qCDebug(appLog) << "Driver scan completed successfully";
        usleep(10000);
        emit scanFinished(SR_SUCESS);
    } else {
        qCWarning(appLog) << "Driver scan interrupted by network error";
        emit scanFinished(SR_NETWORD_ERR);
    }
}

void DriverScanner::setDriverList(QList<DriverInfo *> lstInfo)
{
    qCDebug(appLog) << "Set driver list with" << lstInfo.size() << "items";
    m_ListDriverInfo = lstInfo;
    m_IsStop = false;
}

