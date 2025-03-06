// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverScanner.h"
#include "DeviceManager.h"
#include "HttpDriverInterface.h"
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
                QProcess process;
                process.start("apt", QStringList()  << "policy" << info->packages());
                process.waitForFinished(-1);

                QString output = process.readAllStandardOutput();
                QStringList lines = output.split("\n");
                if(lines.size()>=2) {
                    QRegularExpression rxlen("(\\d+\\S*)");
                    QRegularExpressionMatch match = rxlen.match(lines[1]);
                    if (match.hasMatch() && info->version().isEmpty()) {
                        info->m_Version = match.captured(1);
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

