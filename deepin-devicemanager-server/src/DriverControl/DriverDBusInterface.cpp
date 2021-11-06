#include "DriverDBusInterface.h"
#include "DriverManager.h"
#include "ModCore.h"
#include "Utils.h"
#include "DebInstaller.h"

#include <QDebug>

DriverDBusInterface::DriverDBusInterface(QObject *parent)
    : QObject(parent)
    , mp_drivermanager(new DriverManager(this))
    , pcore(new ModCore(this))
{
    initConnects();
}

void DriverDBusInterface::initConnects()
{
    connect(mp_drivermanager, &DriverManager::sigProgressDetail, this, &DriverDBusInterface::sigProgressDetail);
    connect(mp_drivermanager, &DriverManager::sigFinished, this, &DriverDBusInterface::sigFinished);
}

bool DriverDBusInterface::unInstallDriver(const QString &modulename)
{
    return  mp_drivermanager->unInstallDriver(modulename);
}

bool DriverDBusInterface::installDriver(const QString &filepath)
{
    return  mp_drivermanager->installDriver(filepath);
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


