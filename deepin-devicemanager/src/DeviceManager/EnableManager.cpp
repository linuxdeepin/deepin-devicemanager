// 项目自身文件
#include "EnableManager.h"

// Qt库文件
#include <QDebug>
#include <QProcess>
#include <QTime>

// 其它头文件
#include <stdlib.h>
#include "ZmqOrder.h"

EnableManager *EnableManager::s_Instance = nullptr;

EnableManager::EnableManager()
{

}

EnableDeviceStatus EnableManager::enableDeviceByInput(const QString &name, bool enable, int index)
{
    int id = getDeviceID(name, enable, index);

    QString cmd = QString("xinput %1 %2").arg(enable ? "enable" : "disable").arg(id);
    QProcess process;
    int msecs = -1;
    process.start(cmd);
    process.waitForFinished(msecs);
    int exitCode = process.exitCode();
    QString output = process.readAllStandardOutput();
    if (exitCode == 0) {
        return EDS_Success;
    }
    return EDS_Faild;
}

bool EnableManager::isDeviceEnable(const QString &name)
{
    QString cmd = "xinput list-props \"" + name + "\" ";
    QProcess process;
    int msecs = -1;
    process.start(cmd);
    process.waitForFinished(msecs);
    QString output = process.readAllStandardOutput();
    QStringList listOutput = output.split("\n");
    foreach (const QString &str, listOutput) {
        if (!str.contains("Device Enabled")) {
            continue;
        }
        QStringList items = str.trimmed().split(":");
        if (items.size() != 2) {
            return true;
        }

        if (items[1].trimmed() == "1") {
            return true;
        } else {
            return false;
        }
    }
    return true;
}

bool EnableManager::isDeviceEnable(int id)
{
    QString cmd = QString("xinput list-props %1").arg(id);
    QProcess process;
    int msecs = -1;
    process.start(cmd);
    process.waitForFinished(msecs);
    QString output = process.readAllStandardOutput();
    QStringList listOutput = output.split("\n");
    foreach (const QString &str, listOutput) {
        if (!str.contains("Device Enabled")) {
            continue;
        }
        QStringList items = str.trimmed().split(":");
        if (items.size() != 2) {
            return true;
        }

        if (items[1].trimmed() == "1") {
            return true;
        } else {
            return false;
        }
    }
    return true;
}

EnableDeviceStatus EnableManager::enableDeviceByDriver(bool enable, const QString &driver)
{
    // 生成命令
    QString cmd;
    if (enable) {
        cmd = QString("insmod %1").arg(getDriverPath(driver));
    } else {
        cmd = QString("rmmod %1").arg(driver);
    }

    // 连接到后台
    ZmqOrder order;
    if (!order.connect()) {
        return  EDS_Faild;
    }

    // 通知后台执行禁用操作
    if (order.execDriverOrder(cmd)) {
        return EDS_Success;
    } else {
        return EDS_Faild;
    }
}

bool EnableManager::isDeviceEnableByDriver(const QString &driver)
{
    if (driver == "") {
        return false;
    }

    QString cmd = "lsmod";
    QProcess process;
    int msecs = -1;
    process.start(cmd);
    process.waitForFinished(msecs);
    QString output = process.readAllStandardOutput();
    QStringList drivers = output.split("\n");
    foreach (const QString &d, drivers) {
        if (d.startsWith(driver)) {
            return true;
        }
    }
    return false;
}

EnableDeviceStatus EnableManager::enablePrinter(const QString &name, bool enable)
{
    QString cmd;
    if (true == enable) {
        cmd = "cupsenable " + name;
    } else {
        cmd = "cupsdisable " + name;
    }

    QProcess process;
    int msecs = -1;
    process.start(cmd);
    process.waitForFinished(msecs);
    QString output = process.readAllStandardOutput();
    if (output == "") {
        return EDS_Success;
    } else {
        return EDS_Faild;
    }
}

EnableDeviceStatus EnableManager::enableNetworkByIfconfig(const QString &logicalName, bool enable)
{
    qInfo() << "enableNetworkByIfconfig **************** 001";
    // 生成命令
    QString cmd;
    if (enable) {
        cmd = QString("ifconfig %1 up").arg(logicalName);
    } else {
        cmd = QString("ifconfig %1 down").arg(logicalName);
    }
    qInfo() << "enableNetworkByIfconfig **************** 002";

    // 连接到后台
    ZmqOrder order;
    if (!order.connect()) {
        return  EDS_Faild;
    }

    qInfo() << "enableNetworkByIfconfig **************** 003";
    // 执行命令
    if (order.execIfconfigOrder(cmd)) {
        qInfo() << "enableNetworkByIfconfig **************** 004";
        return EDS_Success;
    } else {
        qInfo() << "enableNetworkByIfconfig **************** 004";
        return EDS_Faild;
    }

}

bool EnableManager::isNetworkEnableByIfconfig(const QString &logicalName)
{
    QString cmd = "ifconfig";
    QProcess process;
    int msecs = -1;
    process.start(cmd);
    process.waitForFinished(msecs);
    QString output = process.readAllStandardOutput();
    QStringList items = output.split("\n\n");
    foreach (const QString &item, items) {
        if (item.startsWith(logicalName)) {
            return true;
        }
    }
    return false;
}

int EnableManager::getDeviceID(const QString &name, bool enable, int index)
{
    int id = -1;
    int curIndex = -1;
    // 先判断有没有同名
    QString cmd = "xinput list";
    QProcess process;
    int msecs = -1;
    process.start(cmd);
    process.waitForFinished(msecs);
    QString output = process.readAllStandardOutput();
    QStringList items = output.split("\n");

    foreach (const QString &item, items) {
        QRegExp re = QRegExp(".*↳(.*)id=([0-9]{1,2}).*");
        if (re.exactMatch(item)) {
            QString n = re.cap(1).trimmed();
            int curId = re.cap(2).toInt();
            if (n == name && (!enable) == isDeviceEnable(curId)) {
                curIndex++;
                if (index == curIndex) {
                    id = curId;
                }
            }
        }
    }
    return id;
}

QString EnableManager::getDriverPath(const QString &driver)
{
    QString path;
    QString cmd = QString("modinfo %1").arg(driver);
    QProcess process;
    int msecs = -1;
    process.start(cmd);
    process.waitForFinished(msecs);
    QString output = process.readAllStandardOutput();
    QStringList lst = output.split("\n");
    foreach (const QString &item, lst) {
        if (item.startsWith("filename")) {
            QStringList kv = item.split(":");
            if (kv.size() == 2) {
                path = kv[1].trimmed();
            }
            break;
        }
    }
    return path;
}

