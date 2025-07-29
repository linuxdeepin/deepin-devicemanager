// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CustomGenerator.h"
#include "DeviceGpu.h"
#include "DBusInterface.h"
#include "commontools.h"

#include <QProcess>
#include <QRegularExpression>
#include <QDir>
#include <QDBusInterface>
#include <QDBusReply>

CustomGenerator::CustomGenerator(QObject *parent)
    : DeviceGenerator(parent)
{
}

void CustomGenerator::generatorGpuDevice()
{
    QString cmd = CommonTools::getGpuInfoCommandFromDConfig();
    if (cmd.isEmpty()) {
        DeviceGenerator::generatorGpuDevice();
        return;
    }

    QStringList arguments;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString display = env.value("DISPLAY");
    QString xauthority = env.value("XAUTHORITY");
    if (display.isEmpty() || xauthority.isEmpty()) {
        qWarning() << "DISPLAY or XAUTHORITY is not set!";
    } else {
        arguments << display << xauthority;
    }

    QString tmpGpuInfo;
    DBusInterface::getInstance()->getGpuInfoByCustom(cmd, arguments, tmpGpuInfo);
    if (tmpGpuInfo.isEmpty()) {
        qCritical() << "Failed to get gpu info by commad " << cmd;
        return;
    }

    QList<QMap<QString, QString>> gpuInfo;
    QStringList mapBlocks = tmpGpuInfo.split("\n\n");
    for (const QString &block : mapBlocks) {
        QMap<QString, QString> infoMap;
        QStringList lines = block.split("\n");
        for (const QString &line : lines) {
            int colonIndex = line.indexOf(':');
            if (colonIndex != -1) {
                QString key = line.left(colonIndex).trimmed();
                QString value = line.mid(colonIndex + 1).trimmed();
                infoMap.insert(key, value);
            }
        }
        if (!infoMap.isEmpty())
            gpuInfo.push_back(infoMap);
    }

    for(int i = 0; i < gpuInfo.count(); ++i) {
        DeviceGpu *device = new DeviceGpu();
        device->setCanUninstall(false);
        device->setForcedDisplay(true);
        device->setGpuInfoByCustom(gpuInfo.at(i));
        DeviceManager::instance()->addGpuDevice(device);
    }
}

void CustomGenerator::generatorMonitorDevice()
{
    QString toDir = "/sys/class/drm";
    QDir toDir_(toDir);

    if (!toDir_.exists())
        return;

    QFileInfoList fileInfoList = toDir_.entryInfoList(QDir::NoFilter, QDir::Name);
    foreach(QFileInfo fileInfo, fileInfoList) {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == ".." || !fileInfo.fileName().startsWith("card"))
            continue;

        if (QFile::exists(fileInfo.filePath() + "/" + "edid")) {
            QStringList allEDIDS_all;
            allEDIDS_all.append(fileInfo.filePath() + "/" + "edid");
            QString interface = fileInfo.fileName().remove("card0-").remove("card1-").remove("card2-");
            CommonTools::parseEDID(allEDIDS_all, interface, false);
         }
    }
}
