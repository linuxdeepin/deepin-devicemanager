// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QProcess>
#include <QMap>
#include <QRegularExpression>
#include <QFile>
#include <QDebug>

#include <iostream>

//  名称("Name") 厂商("Vendor") 型号("Model") 版本(Version) 显存("Graphics Memory")

constexpr char kName[] { "Name" };
constexpr char kVendor[] { "Vendor" };
constexpr char kModel[] { "Model" };
constexpr char kVersion[] { "Version" };
constexpr char kGraphicsMemory[] { "Graphics Memory" };

bool getGpuBaseInfo(QMap<QString, QString> &mapInfo)
{   
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    process.setProcessEnvironment(env);
    process.start("/usr/bin/glxinfo", QStringList() << "-B");
    if (!process.waitForFinished(3000)) {
        qCritical() << "Error executing glxinfo:" << process.errorString();
        return false;
    }

    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QStringList lines = output.split('\n');
    QRegularExpression regex("^([^:]+):\\s*(.+)$");
    for (const QString &line : lines) {
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            QString key = match.captured(1).trimmed();
            QString value = match.captured(2).trimmed();
            if (key == "OpenGL renderer string") {
                mapInfo.insert(kName, value);
                mapInfo.insert(kModel, value);
            } else if (key == "OpenGL vendor string") {
                mapInfo.insert(kVendor, value);
            }
        }
    }

    return true;
}

bool getGpuMemInfoForFTDTM(QMap<QString, QString> &mapInfo)
{
    const QString filePath = "/sys/kernel/debug/gc/meminfo";
    QString totalValue;
    bool foundTotal = false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Error opening /sys/kernel/debug/gc/meminfo:" << file.errorString();
        return false;
    }

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    if (content.isEmpty()) {
        qCritical() << "Error: /sys/kernel/debug/gc/meminfo File is empty!";
        return false;
    }

    QRegularExpression system0Regex(R"(POOL SYSTEM0:*(.*?)POOL VIRTUAL:)",
                                    QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch system0Match = system0Regex.match(content);

    if (!system0Match.hasMatch()) {
        qCritical() << "Error: Failed to find SYSTEM0 section";
        return false;
    }

    QString system0Content = system0Match.captured(1);
    QRegularExpression totalRegex(R"(Total\s*:\s*(\d+)\s+B)");
    QRegularExpressionMatch totalMatch = totalRegex.match(system0Content);
    if (totalMatch.hasMatch()) {
        totalValue = totalMatch.captured(1);
        foundTotal = true;
    }

    if (!foundTotal || totalValue.isEmpty()) {
        qCritical() << "Error: Failed to find Total value in SYSTEM0 content";
        return false;
    }

    bool ok;
    quint64 memSize = totalValue.trimmed().toULong(&ok, 10);
    if (ok && memSize >= 1048576) {
        memSize /= 1048576;
        auto curSize = memSize / 1024.0;
        if (curSize >= 1) {
            totalValue = QString::number(curSize) + "GB";
        } else {
            totalValue = QString::number(memSize) + "MB";
        }
    }

    mapInfo.insert(kGraphicsMemory, totalValue);

    return true;
}

int main(int argc, char *argv[])
{
    QMap<QString, QString> mapInfo;
    if (getGpuBaseInfo(mapInfo) && getGpuMemInfoForFTDTM(mapInfo)) {
        for (auto it = mapInfo.begin(); it != mapInfo.end(); ++it)
            std::cout << it.key().toStdString() << ": " << it.value().toStdString() << std::endl;
        return 0;
    } else {
        return 1;
    }
}
