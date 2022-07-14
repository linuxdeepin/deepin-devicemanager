#include "DriverDBusInterface.h"
#include "DriverManager.h"
#include "ModCore.h"
#include "Utils.h"
#include "DebInstaller.h"

#include <QDebug>
#include <QProcess>
#include <QDir>

bool DriverDBusInterface::getUserAuthorPasswd()
{

    // 2. 执行命令获取用户
    QString  username;
    QString info;
    QProcess process;
    process.start("whoami");
    process.waitForFinished(-1);
    info = process.readAllStandardOutput();
    if(!info.contains("root"))
        return true;

    process.start("lslogins -u");
    process.waitForFinished(-1);
    info = process.readAllStandardOutput();
    QStringList infolist = info.split("\n", QString::SkipEmptyParts);
    for (int i = 0; i < infolist.size(); i++) {
        QStringList names = infolist[i].split(" ", QString::SkipEmptyParts);

        if(names.size() < 2 || names[1].contains("root") || names[1].contains("USER"))
            continue;
         else if(! names[1].isEmpty()){
              username = names[1];
              break;
        }
    }
    process.start("pkexec --user deep pkexec sudo");
    process.waitForFinished(-1);
    int exitCode = process.exitCode();
    if (exitCode == 127 || exitCode == 126) {
        return false;
    }
    return true;
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
    if(!getUserAuthorPasswd())
        return false;
    return  mp_drivermanager->unInstallDriver(modulename);
}

bool DriverDBusInterface::installDriver(const QString &filepath)
{
    if(!getUserAuthorPasswd())
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
