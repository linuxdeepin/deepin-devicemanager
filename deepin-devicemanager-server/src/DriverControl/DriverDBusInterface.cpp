#include "DriverDBusInterface.h"
#include "DriverManager.h"
#include "ModCore.h"

DriverDBusInterface::DriverDBusInterface(QObject *parent)
    : QObject(parent)
    , mp_drivermanager(new DriverManager(this))
    , pcore(new ModCore(this))
{

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


