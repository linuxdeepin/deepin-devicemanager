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
    const QString filePath = "/sys/kernel/debug/gc/total_mem";
    QString totalValue;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Error opening /sys/kernel/debug/gc/total_mem:" << file.errorString();
        return false;
    }

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    if (content.isEmpty()) {
        qCritical() << "Error: /sys/kernel/debug/gc/total_mem File is empty!";
        return false;
    }

    QRegularExpression regex(R"((\d+(?:\.\d+)?)\s*\(?(MB|GB|KB|B)\)?)",
                                    QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch memInfoMatch = regex.match(content);

    if (!memInfoMatch.hasMatch()) {
        qCritical() << "Error: Failed to find memory info";
        return false;
    }

    double value = memInfoMatch.captured(1).toDouble();
    QString unit = memInfoMatch.captured(2).toUpper();

    if (unit == "MB") {
        totalValue = QString("%1GB").arg(value / 1024.0, 0, 'f', 2);
    } else if (unit == "GB") {
        totalValue = QString("%1GB").arg(value, 0, 'f', 2);
    } else if (unit == "KB") {
        totalValue = QString("%1GB").arg(value / (1024.0 * 1024.0), 0, 'f', 2);
    } else if (unit == "B") {
        totalValue = QString("%1GB").arg(value / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }

    mapInfo.insert(kGraphicsMemory, totalValue);

    return true;
}

int main(int argc, char *argv[])
{
    QMap<QString, QString> mapInfo;
    if (getGpuMemInfoForFTDTM(mapInfo)) {
        for (auto it = mapInfo.begin(); it != mapInfo.end(); ++it)
            std::cout << it.key().toStdString() << ": " << it.value().toStdString() << std::endl;
        return 0;
    } else {
        return 1;
    }
}
