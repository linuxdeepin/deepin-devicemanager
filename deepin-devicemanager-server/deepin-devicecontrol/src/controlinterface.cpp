// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controlinterface.h"
#include "drivermanager.h"
#include "modcore.h"
#include "utils.h"
#include "debinstaller.h"
#include "enablesqlmanager.h"
#include "enableutils.h"
#include "wakeuputils.h"
#include "DDLog.h"

#include <QLoggingCategory>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <polkit-qt5-1/PolkitQt1/Authority>
#else
#include <polkit-qt6-1/PolkitQt1/Authority>
#endif

// 系统库文件
#include <dirent.h>
#include <cups.h>
#include <string>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#else
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#endif

using namespace PolkitQt1;
using namespace DDLog;
static int getPidByName(const QString &taskName)
{
    DIR *dir = opendir("/proc");
    int pid = -1;
    if (nullptr == dir)
        return pid;

    struct dirent *ptr = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0) || DT_DIR != ptr->d_type)
            continue;
        char filepath[1024] = { 0 };
        sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);
        FILE *fp = fopen(filepath, "r");
        if (nullptr == fp)
            continue;

        char buf[1024] = { 0 };
        if (fgets(buf, 1023, fp) == nullptr) {
            fclose(fp);
            continue;
        }
        char cur_task_name[1024] = { 0 };
        sscanf(buf, "%s", cur_task_name);
        fclose(fp);
        if (QString(cur_task_name).endsWith(taskName)) {
            bool ok = false;
            int curPid = QString(ptr->d_name).toInt(&ok);
            if (ok) {
                pid = curPid;
                break;
            }
        }
    }
    closedir(dir);

    return pid;
}


bool ControlInterface::getUserAuthorPasswd()
{
    qCDebug(appLog) << "Checking user authorization...";
#ifdef DISABLE_POLKIT
    qCDebug(appLog) << "Polkit disabled, authorization granted";
    return true;
#endif
    if (connection().interface()->serviceUid(message().service()).value() == 0) {
        return true;
    }

    Authority::Result result = Authority::instance()->checkAuthorizationSync("com.deepin.deepin-devicemanager.checkAuthentication",
                                                                             SystemBusNameSubject(message().service()),
                                                                             Authority::AllowUserInteraction);
    return result == Authority::Yes;
}
ControlInterface::ControlInterface(QObject *parent)
    : QDBusService(parent)
#ifndef DISABLE_DRIVER
      ,
      mp_drivermanager(new DriverManager(this)),
      pcore(new ModCore(this))
#endif
{
    qCDebug(appLog) << "ControlInterface initializing...";
    initPolicy(QDBusConnection::SystemBus, QString(SERVICE_CONFIG_DIR) + "other/deepin-devicecontrol.json");
    initConnects();
}

void ControlInterface::initConnects()
{
#ifndef DISABLE_DRIVER
    connect(mp_drivermanager, &DriverManager::sigProgressDetail, this, &ControlInterface::sigProgressDetail);
    connect(mp_drivermanager, &DriverManager::sigDownloadProgressChanged, this, &ControlInterface::sigDownloadProgressChanged);
    connect(mp_drivermanager, &DriverManager::sigDownloadFinished, this, &ControlInterface::sigDownloadFinished);
    connect(mp_drivermanager, &DriverManager::sigInstallProgressChanged, this, &ControlInterface::sigInstallProgressChanged);
    connect(mp_drivermanager, &DriverManager::sigFinished, this, [=](bool bsuccess, QString msg) {
        lockTimer(false);
        emit sigFinished(bsuccess, msg);
    });
    connect(mp_drivermanager, &DriverManager::sigInstallProgressFinished, this, [=](bool bsuccess, int err) {
        lockTimer(false);
        emit sigInstallProgressFinished(bsuccess, err);
    });
#endif
}

QString ControlInterface::getRemoveInfo()
{
    if (!getUserAuthorPasswd())
        return {};
    return EnableSqlManager::getInstance()->removedInfo();
}

QString ControlInterface::getAuthorizedInfo()
{
    if (!getUserAuthorPasswd())
        return {};
    return EnableSqlManager::getInstance()->authorizedInfo();
}

bool ControlInterface::enable(const QString &hclass, const QString &name, const QString &path, const QString &value, bool enable_device, const QString strDriver)
{
    qCDebug(appLog) << "Enable device request:" << hclass << name << path << "enable:" << enable_device;
    if (!getUserAuthorPasswd()) {
        qCWarning(appLog) << "Authorization failed for enable operation";
        return {};
    }
    // 网卡通过ioctl禁用
    // 先判断是否是网卡
    QRegularExpression reg("^[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}$");
    if (reg.match(value).hasMatch()) {
        return ioctlEnableNetwork(hclass, name, path, value, enable_device, strDriver);
    }

    // 先从数据库中查找路径，防止设备更换usb接口
    QString sPath = EnableSqlManager::getInstance()->authorizedPath(value);
    sPath = path;

    // 判断是内置设备，还是外设，内置设备通过remove文件禁用，外设通过authorized文件禁用
    bool res = false;
    if (QFile::exists("/sys" + sPath + QString("/authorized"))) {
        modifyPath(sPath);
        res = authorizedEnable(hclass, name, sPath, value, enable_device, strDriver);
    } else { /* if(QFile::exists("/sys" + sPath + QString("/remove")))*/
        res = removeEnable(hclass, name, path, value, enable_device, strDriver);
    }
    emit sigUpdate();
    return res;
}

bool ControlInterface::enablePrinter(const QString &hclass, const QString &name, const QString &path, bool enable_device)
{
    if (!getUserAuthorPasswd())
        return {};
    ipp_op_t op = enable_device ? IPP_OP_RESUME_PRINTER : IPP_OP_PAUSE_PRINTER;
    char uri[HTTP_MAX_URI];
    ipp_t *request = nullptr;
    ipp_t *answer = nullptr;
    request = ippNewRequest(op);
    construct_uri(uri, sizeof(uri), "ipp://localhost/printers/", name.toStdString().c_str());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
    const char *host = cupsServer();
    if (!host) {
        return false;
    }
    int port = ippPort();
    int encrption = (http_encryption_t)cupsEncryption();
    int cancel = 0;
    http_t *http = httpConnect2(host, port, nullptr, AF_UNSPEC, (http_encryption_t)encrption, 1, 30000, &cancel);
    if (!http) {
        return false;
    }
    answer = cupsDoRequest(http, request, "/admin/");
    ippDelete(answer);

    // 禁用成功后将信息保存到数据库
    if (enable_device) {
        EnableSqlManager::getInstance()->removeDataFromPrinterTable(name);
    } else {
        EnableSqlManager::getInstance()->insertDataToPrinterTable(hclass, name, path);
    }
    emit sigUpdate();
    return true;
}

bool ControlInterface::enableKeyboard(const QString& vid, const QString& pid, const QString &hclass, const QString &name, const QString &sPath, const QString &value, bool enable_device, const QString strDriver)
{
    qCInfo(appLog) << "Enable keyboard request:" << hclass << name << sPath << value << "enable:" << enable_device;
    
    if (!getUserAuthorPasswd()) {
        qCWarning(appLog) << "Authorization failed for keyboard enable operation";
        return false;
    }
    
    // Use EnableUtils's validation function
    QString safeVid, safePid;
    if (!EnableUtils::validateAndNormalizeVidPid(vid, pid, safeVid, safePid)) {
        qCWarning(appLog) << "VID or PID validation failed. VID:" << vid << "PID:" << pid;
        return false;
    }

    QString rulesFile = QString(UDEV_RULES_PATH_LOCAL "/99-keyboard-device-control-%1-%2.rules").arg(vid).arg(pid);
    
    QString ruleContent;
    if (enable_device) {
        ruleContent = QString("# enable keyboard - VID:%1 PID:%2\n"
                              "ACTION==\"add|change\", SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"%1\", ATTRS{idProduct}==\"%2\", "
                              "ATTR{authorized}=\"1\"").arg(safeVid, safePid);
        qCInfo(appLog) << "Creating rule to enable keyboard";
    } else {
        ruleContent = QString("# disable keyboard - VID:%1 PID:%2\n"
                              "ACTION==\"add|change\", SUBSYSTEM==\"usb\", ATTRS{idVendor}==\"%1\", ATTRS{idProduct}==\"%2\", "
                              "ATTR{authorized}=\"0\"").arg(safeVid, safePid);
        qCInfo(appLog) << "Creating rule to disable keyboard";
    }

    QFileInfo fileInfo(rulesFile);
    QDir rulesDir = fileInfo.absoluteDir();
    if (!rulesDir.exists()) {
        qCInfo(appLog) << "Udev rules directory does not exist, creating:" << rulesDir.absolutePath();
        if (!rulesDir.mkpath(".")) {
            qCWarning(appLog) << "Failed to create udev rules directory:" << rulesDir.absolutePath();
            return false;
        }
        qCInfo(appLog) << "Successfully created udev rules directory:" << rulesDir.absolutePath();
    }

    QFile file(rulesFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(appLog) << "Failed to open udev rules file for writing:" << rulesFile;
        qCWarning(appLog) << "Error:" << file.errorString();
        return false;
    }
    
    QTextStream out(&file);
    out << ruleContent;
    file.close();
    
    if (file.error() != QFile::NoError) {
        qCWarning(appLog) << "Failed to write udev rule. Error:" << file.errorString();
        return false;
    }

    QProcess reloadProcess;
    qCInfo(appLog) << "Reloading udev rules";
    reloadProcess.start("udevadm", QStringList() << "control" << "--reload-rules");
    reloadProcess.waitForFinished(10000);
    
    if (reloadProcess.exitCode() != 0) {
        qCWarning(appLog) << "Failed to reload udev rules. Error:" << reloadProcess.readAllStandardError();
        return false;
    }
    
    QProcess triggerProcess;
    qCInfo(appLog) << "Triggering udev rules";
    triggerProcess.start("udevadm", QStringList() << "trigger");
    triggerProcess.waitForFinished(10000);
    
    if (triggerProcess.exitCode() != 0) {
        qCWarning(appLog) << "Failed to trigger udev rules. Error:" << triggerProcess.readAllStandardError();
        return false;
    }

    if (!enable_device) {
        EnableSqlManager::getInstance()->insertDataToAuthorizedTable(hclass, name, sPath, value, true, strDriver);
        qCInfo(appLog) << "insertDataToAuthorizedTable";
    } else {
        EnableSqlManager::getInstance()->removeDataFromAuthorizedTable(value);
        qCInfo(appLog) << "removeDataFromAuthorizedTable";
    }
    
    qCInfo(appLog) << "Keyboard udev rule created and applied successfully";
    emit sigUpdate();
    return true;
}

void ControlInterface::disableInDevice()
{
    if (!getUserAuthorPasswd())
        return;
    EnableUtils::disableInDevice();
}

void ControlInterface::disableOutDevice(const QString &devInfo)
{
    if (!getUserAuthorPasswd())
        return;
    EnableUtils::disableOutDevice(devInfo);
}

bool ControlInterface::isDeviceEnabled(const QString &unique_id)
{
    if (!getUserAuthorPasswd())
        return {};
    return EnableSqlManager::getInstance()->isUniqueIdEnabled(unique_id);
}

bool ControlInterface::setWakeupMachine(const QString &unique_id, const QString &path, bool wakeup)
{
    if (!getUserAuthorPasswd())
        return {};
    // 读写wake文件
    QString tpath;
    if (!WakeupUtils::wakeupPath(path, tpath))
        return false;
    if (!WakeupUtils::writeWakeupFile(tpath, wakeup))
        return false;

    // 将数据写到数据库或者从数据库删除数据
    saveWakeupInfo(unique_id, path, wakeup);
    return true;
}

bool ControlInterface::setNetworkWake(const QString &logicalName, bool wakeup)
{
    if (!getUserAuthorPasswd())
        return {};
    bool res = WakeupUtils::setWakeOnLan(logicalName, wakeup);
    if (res) {
        // 将数据保存到数据库
        EnableSqlManager::getInstance()->insertNetworkWakeup(logicalName, wakeup);
    }
    return res;
}

void ControlInterface::updateWakeup(const QString &devInfo)
{
    if (!getUserAuthorPasswd())
        return;
    WakeupUtils::updateWakeupDeviceInfo(devInfo);
}

int ControlInterface::isNetworkWakeup(const QString &logicalName)
{
    if (!getUserAuthorPasswd())
        return {};
    return WakeupUtils::wakeOnLanIsOpen(logicalName);
}

void ControlInterface::setMonitorWorkingDBFlag(bool flag)
{
    if (!getUserAuthorPasswd())
        return;
    EnableSqlManager::getInstance()->setMonitorWorkingFlag(flag);
}

bool ControlInterface::monitorWorkingDBFlag()
{
    if (!getUserAuthorPasswd())
        return {};
    return EnableSqlManager::getInstance()->monitorWorkingFlag();
}

#ifndef DISABLE_DRIVER
bool ControlInterface::unInstallDriver(const QString &modulename)
{
    if (!getUserAuthorPasswd())
        return false;
    lockTimer(false);
    return mp_drivermanager->unInstallDriver(modulename);
}

bool ControlInterface::installDriver(const QString &filepath)
{
    if (!getUserAuthorPasswd()) {
        emit sigFinished(false, "Cancel");
        return false;
    }
    lockTimer(true);
    return mp_drivermanager->installDriver(filepath);
}

void ControlInterface::installDriver(const QString &modulename, const QString &version)
{
    if (!getUserAuthorPasswd()) {
        emit mp_drivermanager->sigInstallProgressFinished(false, EC_CANCEL);
        return;
    }
    lockTimer(true);
    return mp_drivermanager->installDriver(modulename, version);
}

void ControlInterface::undoInstallDriver()
{
    if (!getUserAuthorPasswd())
        return;
    lockTimer(false);
    return mp_drivermanager->undoInstallDriver();
}

QStringList ControlInterface::checkModuleInUsed(const QString &modulename)
{
    if (!getUserAuthorPasswd())
        return {};
    return mp_drivermanager->checkModuleInUsed(modulename);
}

bool ControlInterface::isDriverPackage(const QString &filepath)
{
    if (!getUserAuthorPasswd())
        return false;
    else
        return mp_drivermanager->isDriverPackage(filepath);
}

bool ControlInterface::isBlackListed(const QString &modName)
{
    if (!getUserAuthorPasswd())
        return {};
    return mp_drivermanager->isBlackListed(modName);
}

bool ControlInterface::isArchMatched(const QString &filePath)
{
    if (!getUserAuthorPasswd())
        return {};
    return mp_drivermanager->isArchMatched(filePath);
}

bool ControlInterface::isDebValid(const QString &filePath)
{
    if (!getUserAuthorPasswd())
        return {};
    return mp_drivermanager->isDebValid(filePath);
}

bool ControlInterface::unInstallPrinter(const QString &vendor, const QString &model)
{
    if (!getUserAuthorPasswd()) {
        emit mp_drivermanager->sigFinished(false, "Cancel");
        return false;
    }
    return mp_drivermanager->uninstallPrinter(vendor, model);
}

bool ControlInterface::backupDeb(const QString &debpath)
{
    if (!getUserAuthorPasswd()) {
        emit sigBackupProgressFinished(false);
        return false;
    }
    bool ret = mp_drivermanager->backupDeb(debpath);
    emit sigBackupProgressFinished(ret);
    return ret;
}

bool ControlInterface::delDeb(const QString &debname)
{
    if (!getUserAuthorPasswd()) {
        return false;
    }
    return mp_drivermanager->delDeb(debname);
}

bool ControlInterface::aptUpdate()
{
    if (!getUserAuthorPasswd()) {
        return false;
    }
    return mp_drivermanager->aptUpdate();
}

#endif
bool ControlInterface::authorizedEnable(const QString &hclass, const QString &name, const QString &path, const QString &unique_id, bool enable_device, const QString strDriver)
{
    qCDebug(appLog) << "Authorized enable operation:" << hclass << name << path << "enable:" << enable_device;
    // 通过authorized文件启用禁用设备
    // 0:表示禁用 ，1:表示启用
    QFile file("/sys" + path + QString("/authorized"));
    if (!file.open(QIODevice::ReadWrite)) {
        return false;
    }
    if (enable_device) {
        /*
         启用的流程为：以 /devices/pci0000:00/0000:00:14.0/usb1/1-5/1-5:1.0 为例
         第一步: 向 /sys/devices/pci0000:00/0000:00:14.0/usb1/1-5/1-5:1.0/authorized 文件中写 1
         第二步: 向 /sys/devices/pci0000:00/0000:00:14.0/usb1/1-5/authorized 文件中写 0
         第三步: 向 /sys/devices/pci0000:00/0000:00:14.0/usb1/1-5/authorized 文件中写 1
         */
        // 第一步
        file.write("1");
        file.close();

        // 第二步
        QFileInfo fi(path);
        QString pop = fi.path();
        QFile fpop("/sys" + pop + QString("/authorized"));
        if (!fpop.open(QIODevice::ReadWrite))
            return false;
        fpop.write("0");
        fpop.close();

        // 第三步
        if (!fpop.open(QIODevice::ReadWrite))
            return false;
        fpop.write("1");
        fpop.close();

        EnableSqlManager::getInstance()->removeDataFromAuthorizedTable(unique_id);
    } else {
        file.write("0");
        file.close();
        EnableSqlManager::getInstance()->insertDataToAuthorizedTable(hclass, name, path, unique_id, true, strDriver);
    }
    return true;
}

bool ControlInterface::removeEnable(const QString &hclass, const QString &name, const QString &path, const QString &unique_id, bool enable, const QString strDriver)
{
    qCDebug(appLog) << "Remove enable operation:" << hclass << name << path << "enable:" << enable;
    if (enable) {
        // 1. 先rescan 向rescan写入1,则重新加载
        QFile file("/sys/bus/pci/rescan");
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }
        file.write("1");
        file.close();

        // platform disable with reset
        if (path.contains("platform")) {
            QFile filep("/sys" + path + QString("/reset"));
            if (!filep.open(QIODevice::WriteOnly)) {
                return false;
            }
            filep.write("1");
            filep.close();
        }
        EnableSqlManager::getInstance()->removeDateFromRemoveTable(path);

        // 由于rescan会将所有的remove数据都回复，因此需要重新禁用其它设备
        QStringList rpList;
        EnableSqlManager::getInstance()->removePathList(rpList);
        foreach (const QString &path, rpList) {
            QFile filerp("/sys" + path + QString("/remove"));
            if (filerp.open(QIODevice::WriteOnly)) {
                filerp.write("1");
                filerp.close();
            }
        }
    } else {
        // 1. 直接remove写入
        // 通过remove文件禁用
        // 1:表示禁用 ，0:表示启用
        qCInfo(appLog) << ""
                       << "/sys" + path + QString("/remove");
        //        return false;
        QFile file("/sys" + path + QString("/remove"));
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }
        file.write("1");
        file.close();

        // 2. 持久化保存
        EnableSqlManager::getInstance()->insertDataToRemoveTable(hclass, name, path, unique_id, strDriver);
    }
    return true;
}

bool ControlInterface::ioctlEnableNetwork(const QString &hclass, const QString &name, const QString &logical_name, const QString &unique_id, bool enable, const QString strDriver)
{
    // 1. 通过ioctl禁用
    if (!EnableUtils::ioctlOperateNetworkLogicalName(logical_name, enable))
        return false;
    // 2. 持久化保存
    if (enable) {
        EnableSqlManager::getInstance()->removeDataFromAuthorizedTable(unique_id);
    } else {
        EnableSqlManager::getInstance()->insertDataToAuthorizedTable(hclass, name, logical_name, unique_id, enable, strDriver);
    }
    return true;
}

void ControlInterface::construct_uri(char *buffer, size_t buflen, const char *base, const char *value)
{
    char *d = buffer;
    const unsigned char *s = (const unsigned char *)value;
    if (strlen(base) < buflen) {
        strcpy(buffer, base);
        d += strlen(base);
    } else {
        strncpy(buffer, base, buflen);
        d += buflen;
    }

    while (*s && d < buffer + buflen) {
        if (isalpha(*s) || isdigit(*s) || *s == '-')
            *d++ = *s++;
        else if (*s == ' ') {
            *d++ = '+';
            s++;
        } else {
            if (d + 2 < buffer + buflen) {
                *d++ = '%';
                *d++ = "0123456789ABCDEF"[((*s) & 0xf0) >> 4];
                *d++ = "0123456789ABCDEF"[((*s) & 0x0f)];
                s++;
            } else {
                break;
            }
        }
    }

    if (d < buffer + buflen)
        *d = '\0';
}

void ControlInterface::modifyPath(QString &path)
{
    path.replace(QRegularExpression("[1-9]$"), "0");
}

void ControlInterface::saveWakeupInfo(const QString &unique_id, const QString &path, bool wakeup)
{
    if (EnableSqlManager::getInstance()->isWakeupUniqueIdExisted(unique_id)) {
        EnableSqlManager::getInstance()->updateWakeData(unique_id, path, wakeup);
    } else {
        EnableSqlManager::getInstance()->insertWakeupData(unique_id, path, wakeup);
    }
}
