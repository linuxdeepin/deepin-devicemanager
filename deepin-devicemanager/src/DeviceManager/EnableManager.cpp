#include "EnableManager.h"
#include <QDebug>
#include <QProcess>
#include <QTime>
#include <stdlib.h>

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
    if (!getPasswd()) {
        return EDS_Cancle;
    }

    QDateTime dt = QDateTime::currentDateTime();
    QString dtStr = dt.toString("yyyy:MM:dd:hh:mm:ss");
    QString dtInt = QString::number(dt.toMSecsSinceEpoch());
    QString key = getPKStr(dtStr, dtInt);

    QString cmd;
    if (enable) {
        cmd = QString("pkexec deepin-devicemanager-authenticateProxy \"insmod %1\" %2").arg(getDriverPath(driver)).arg(key);
    } else {
        cmd = QString("pkexec deepin-devicemanager-authenticateProxy \"rmmod %1\" %2").arg(driver).arg(key);
    }

    QProcess process;
    int msecs = -1;
    process.start(cmd);
    process.waitForFinished(msecs);
    int exitcode = process.exitCode();
    if (exitcode == 127 || exitcode == 126) {
        return EDS_Cancle;
    }
    QString output = process.readAllStandardOutput();
    if (output == "") {
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
    if (!getPasswd()) {
        return EDS_Cancle;
    }

    QDateTime dt = QDateTime::currentDateTime();
    QString dtStr = dt.toString("yyyy:MM:dd:hh:mm:ss");
    QString dtInt = QString::number(dt.toMSecsSinceEpoch());
    QString key = getPKStr(dtStr, dtInt);

    QString cmd;
    if (enable) {
        cmd = QString("pkexec deepin-devicemanager-authenticateProxy \"ifconfig %1 up\" %2").arg(logicalName).arg(key);
    } else {
        cmd = QString("pkexec deepin-devicemanager-authenticateProxy \"ifconfig %1 down\" %2").arg(logicalName).arg(key);
    }

    QProcess process;
    int msecs = -1;
    process.start(cmd);
    process.waitForFinished(msecs);

    int exitCode = process.exitCode();
    if (exitCode == 126 || exitCode == 127) {
        return EDS_Cancle;
    }

    QString output = process.readAllStandardOutput();
    if (output == "") {
        return EDS_Success;
    } else {
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

QString EnableManager::getPKStr(const QString &dtStr, const QString &dtInt)
{
    QString res = "";
    QString str = dtStr;
    str.replace(":", "");

    int year = str.mid(0, 4).toInt() - 253;
    int month = str.mid(4, 2).toInt() * 7;
    int day = str.mid(6, 2).toInt() * 3;
    int hour = str.mid(8, 2).toInt() * 4;
    int minus = str.mid(10, 2).toInt();
    int second = str.mid(12, 2).toInt();

    QString yearStr = QString("%1").arg(year, 4, 10, QLatin1Char('0'));
    QString monthStr = QString("%1").arg(month, 2, 10, QLatin1Char('0'));
    QString dayStr = QString("%1").arg(day, 2, 10, QLatin1Char('0'));
    QString hourStr = QString("%1").arg(hour, 2, 10, QLatin1Char('0'));
    QString minusStr = QString("%1").arg(minus, 2, 10, QLatin1Char('0'));
    QString secondStr = QString("%1").arg(second, 2, 10, QLatin1Char('0'));

    str = dtInt;
    QString value1 = str.mid(0, 1);
    QString value2 = str.mid(1, 2);
    QString value3 = str.mid(3, 3);
    QString value4 = str.mid(6, 4);
    QString value5 = str.mid(10);

    QTime time = QTime::currentTime();
    qsrand(uint(time.msec()) + uint(time.second()) * 1000);
    int random = (qrand() % 10000 + 10000) * 3;  //产生随机数
    QString randomS = QString::number(random);

    QString newDtStr = QString("%1%2%3%4%5%6%7%8%9%10%11%12").arg(value4).arg(dayStr).arg(value2).arg(secondStr).arg(value1).arg(hourStr).arg(value3).arg(monthStr).arg(yearStr).arg(minusStr).arg(value5).arg(randomS);

    return newDtStr;
}
void EnableManager::getPKStr(QString &dtStr, QString &dtInt, const QString &cStr)
{
    QString value4 = cStr.mid(0, 4);
    QString dayStr = QString("%1").arg(cStr.mid(4, 2).toInt() / 3, 2, 10, QLatin1Char('0'));
    QString value2 = cStr.mid(6, 2);
    QString secondStr = QString("%1").arg(cStr.mid(8, 2).toInt(), 2, 10, QLatin1Char('0'));
    QString value1 = cStr.mid(10, 1);
    QString hourStr = QString("%1").arg(cStr.mid(11, 2).toInt() / 4, 2, 10, QLatin1Char('0'));
    QString value3 = cStr.mid(13, 3);
    QString monthStr = QString("%1").arg(cStr.mid(16, 2).toInt() / 7, 2, 10, QLatin1Char('0'));
    QString yearStr = QString("%1").arg(cStr.mid(18, 4).toInt() + 253, 4, 10, QLatin1Char('0'));
    QString minuStr = cStr.mid(22, 2);

    QString value5 = cStr.mid(24);
    QString extraS = value5.right(5);
    value5.replace(extraS, "");

    int extraInt = extraS.toInt();
    if (extraInt % 3 != 0) {
        dtStr = "111";
        return;
    }

    dtStr = QString("%1:%2:%3:%4:%5:%6").arg(yearStr, monthStr, dayStr, hourStr, minuStr, secondStr);
    dtInt = QString("%1%2%3%4%5").arg(value1).arg(value2).arg(value3).arg(value4).arg(value5);
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

bool EnableManager::getPasswd()
{
    QString cmd = QString("pkexec deepin-devicemanager-authenticateProxy \"whoami\"");
    QProcess process;
    int msecs = -1;
    process.start(cmd);
    bool res = process.waitForFinished(msecs);
    int exitcode = process.exitCode();
    if (exitcode == 127 || exitcode == 126) {
        return false;
    }
    return res;
}

