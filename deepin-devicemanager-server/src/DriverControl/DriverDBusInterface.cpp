#include "DriverDBusInterface.h"
#include "DriverManager.h"
#include "ModCore.h"
#include "Utils.h"
#include "DebInstaller.h"

#include <QDebug>
#include <QProcess>
#include <QDir>

#include <polkit-qt5-1/PolkitQt1/Authority>

#include <dirent.h>

using namespace PolkitQt1;

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
        char filepath[1024] = {0};
        sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);
        FILE *fp = fopen(filepath, "r");
        if (nullptr == fp)
            continue;

        char buf[1024] = {0};
        if (fgets(buf, 1023, fp) == nullptr) {
            fclose(fp);
            continue;
        }
        char cur_task_name[1024] = {0};
        sscanf(buf, "%s", cur_task_name);
        fclose(fp);
        if (!strcmp(taskName.toStdString().c_str(), cur_task_name)) {
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

bool DriverDBusInterface::getUserAuthorPasswd()
{
    int pid = getPidByName("deepin-devicemanager");
    if (pid >= 0) {
        Authority::Result result = Authority::instance()->checkAuthorizationSync("com.deepin.deepin-devicemanager.checkAuthentication",
                                                                                 UnixProcessSubject(pid),
                                                                                 Authority::AllowUserInteraction);
        return result == Authority::Yes;
    } else {
        return false;
    }
}

DriverDBusInterface::DriverDBusInterface(QObject *parent)
    : QObject(parent)
    , QDBusContext()
    , mp_drivermanager(new DriverManager(this))
    , pcore(new ModCore(this))
{
    initConnects();
}

void DriverDBusInterface::initConnects()
{
    connect(mp_drivermanager, &DriverManager::sigProgressDetail, this, &DriverDBusInterface::sigProgressDetail);
    connect(mp_drivermanager, &DriverManager::sigFinished, this, &DriverDBusInterface::sigFinished);

    connect(mp_drivermanager, &DriverManager::sigDownloadProgressChanged, this, &DriverDBusInterface::sigDownloadProgressChanged);
    connect(mp_drivermanager, &DriverManager::sigDownloadFinished, this, &DriverDBusInterface::sigDownloadFinished);
    connect(mp_drivermanager, &DriverManager::sigInstallProgressChanged, this, &DriverDBusInterface::sigInstallProgressChanged);
    connect(mp_drivermanager, &DriverManager::sigInstallProgressFinished, this, &DriverDBusInterface::sigInstallProgressFinished);
}

bool DriverDBusInterface::unInstallDriver(const QString &modulename)
{
    if (!getUserAuthorPasswd())
        return false;
    return  mp_drivermanager->unInstallDriver(modulename);
}

bool DriverDBusInterface::installDriver(const QString &filepath)
{
    if (!getUserAuthorPasswd())
        return false;
    return  mp_drivermanager->installDriver(filepath);
}

void DriverDBusInterface::installDriver(const QString &modulename, const QString &version)
{
    return  mp_drivermanager->installDriver(modulename, version);
}

void DriverDBusInterface::undoInstallDriver()
{
    return mp_drivermanager->undoInstallDriver();
}

QStringList DriverDBusInterface::checkModuleInUsed(const QString &modulename)
{
    return mp_drivermanager->checkModuleInUsed(modulename);
}

bool DriverDBusInterface::isDriverPackage(const QString &filepath)
{
    return  mp_drivermanager->isDriverPackage(filepath);
}

bool DriverDBusInterface::isBlackListed(const QString &modName)
{
    return mp_drivermanager->isBlackListed(modName);
}

bool DriverDBusInterface::isArchMatched(const QString &filePath)
{
    return mp_drivermanager->isArchMatched(filePath);
}

bool DriverDBusInterface::isDebValid(const QString &filePath)
{
    return mp_drivermanager->isDebValid(filePath);
}

bool DriverDBusInterface::unInstallPrinter(const QString &vendor, const QString &model)
{
    return mp_drivermanager->uninstallPrinter(vendor, model);
}
