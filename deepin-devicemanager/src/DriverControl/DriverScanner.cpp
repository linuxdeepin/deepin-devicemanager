// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverScanner.h"
#include "DeviceManager.h"
#include "HttpDriverInterface.h"
#include "commonfunction.h"

#include <QRegularExpression>
#include <QLoggingCategory>
#include <QProcess>

#include <unistd.h>


DriverScanner::DriverScanner(QObject *parent)
    : QThread(parent)
    , m_IsStop(false)
{

}

void DriverScanner::run()
{
    HttpDriverInterface *hdi  = HttpDriverInterface::getInstance();
    connect(hdi, &HttpDriverInterface::sigRequestFinished, this, [ = ]() {
        m_IsStop = true;
    });

    foreach (DriverInfo *info, m_ListDriverInfo) {
        if (!m_IsStop) {
            hdi->getRequest(info);

            // 检测本地安装版本
            if (!info->packages().isEmpty()) {
                QString outInfo = Common::executeClientCmd("apt", QStringList() << "policy" << info->packages(), QString(), -1, false);
                if (!outInfo.isEmpty()) {
                    QStringList infoList = outInfo.split("\n");
                    int index = 0;
                    for (int i = 0; i < infoList.size(); i++)
                    {
                        if (infoList[i].startsWith(info->packages())) {
                            index = i;
                            break;
                        }
                    }
                    if (infoList.size() > (2 + index) && !infoList[1 + index].contains("（") && !infoList[1 + index].contains("(")) {
                        QRegularExpression rxlen("(\\d+\\S*)");
                        QRegularExpressionMatch match = rxlen.match(infoList[1 + index]);
                        QString curVersion;
                        if (match.hasMatch()) {
                            curVersion = match.captured(1);
                        }
                        info->m_Version = curVersion.trimmed();
                    }
                }
            }

            emit scanInfo(info->name(), 100 / m_ListDriverInfo.size());
            sleep(1);
        } else {
            emit scanFinished(SR_NETWORD_ERR);
        }
    }

    if (!m_IsStop) {
        // 扫描结束
        usleep(10000);
        emit scanFinished(SR_SUCESS);
    } else {
        emit scanFinished(SR_NETWORD_ERR);
    }
}

void DriverScanner::setDriverList(QList<DriverInfo *> lstInfo)
{
    m_ListDriverInfo = lstInfo;
    m_IsStop = false;
}

