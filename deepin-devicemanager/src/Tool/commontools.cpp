#include "commontools.h"

#include <QDebug>
#include <QDateTime>

DWIDGET_USE_NAMESPACE

#define ICON_SIZE_WIDTH 36
#define ICON_SIZE_HEIGHT 36

QMap<DriverType, QString> CommonTools::m_MapDriverIcon = {
    {DR_Bluetooth, QString(":/icons/deepin/builtin/icons/bluetooth.svg")}
    , {DR_Camera, QString(":/icons/deepin/builtin/icons/image.svg")}
    , {DR_Gpu, QString(":/icons/deepin/builtin/icons/GPU.svg")}
    , {DR_Keyboard, QString(":/icons/deepin/builtin/icons/keyboard.svg")}
    , {DR_Sound, QString(":/icons/deepin/builtin/icons/voice.svg")}
    , {DR_Mouse, QString(":/icons/deepin/builtin/icons/mouse.svg")}
    , {DR_Network, QString(":/icons/deepin/builtin/icons/network.svg")}
    , {DR_Printer, QString(":/icons/deepin/builtin/icons/printer.svg")}
    , {DR_Scaner, QString(":/icons/deepin/builtin/icons/scaner.svg")}
    , {DR_Tablet, QString(":/icons/deepin/builtin/icons/tablet.svg")}
    , {DR_WiFi, QString(":/icons/deepin/builtin/icons/Wi-Fi.svg")}
};

QMap<DriverType, QString> CommonTools::m_MapDriverType = {
    {DR_Bluetooth, QObject::tr("Bluetooth adapter")}
    , {DR_Camera, QObject::tr("Imaging device")}
    , {DR_Gpu, QObject::tr("Display adapter")}
    , {DR_Keyboard, QObject::tr("Keyboard")}
    , {DR_Sound, QObject::tr("Sound card")}
    , {DR_Mouse, QObject::tr("Mouse")}
    , {DR_Network, QObject::tr("Network adapter")}
    , {DR_Printer, QObject::tr("Printer")}
    , {DR_Scaner, QObject::tr("Imaging device")}
    , {DR_Tablet, QObject::tr("Other Devices")}
    , {DR_WiFi, QObject::tr("Wireless network adapter")}
};

QMap<Status, QString> CommonTools::m_MapStatusIcon = {
    {ST_SUCESS, QString(":/icons/deepin/builtin/icons/ok.svg")}
    , {ST_FAILED, QString(":/icons/deepin/builtin/icons/warning.svg")}
    , {ST_DOWNLOADING, QString("")}
    , {ST_INSTALL, QString("")}
    , {ST_NOT_INSTALL, QString(":/icons/deepin/builtin/icons/alert.svg")}
    , {ST_CAN_UPDATE, QString(":/icons/deepin/builtin/icons/alert.svg")}
    , {ST_WAITING, QString(":/icons/deepin/builtin/icons/waiting.svg")}
};

QMap<Status, QString> CommonTools::m_MapStatusType = {
    {ST_SUCESS, QObject::tr("Installation successful")}
    , {ST_FAILED, QObject::tr("Installation failed")}
    , {ST_DOWNLOADING, QObject::tr("Downloading")}
    , {ST_INSTALL, QObject::tr("Installing")}
    , {ST_NOT_INSTALL, QObject::tr("Not installed")}
    , {ST_CAN_UPDATE, QObject::tr("Out-of-date")}
    , {ST_WAITING, QObject::tr("Waiting")}
};

QMap<int, QString> CommonTools::m_MapErrorString = {
    {EC_NULL, QObject::tr("Unknown error")}
    , {EC_NETWORK, QObject::tr("Network error")}
    , {EC_CANCEL, QObject::tr("Canceled")}
    , {EC_NOTFOUND, QObject::tr("Failed to get driver files")}
    , {EC_4, tr("EC_4")}
    , {EC_5, tr("EC_5")}
    , {EC_6, tr("EC_6")}
};

CommonTools::CommonTools(QObject *parent)
    : QObject(parent)
{

}

QString CommonTools::getDriverType(DriverType type)
{
    return m_MapDriverType[type];
}

QString CommonTools::getDriverPixmap(DriverType type)
{
    return m_MapDriverIcon[type];
}

QString CommonTools::getStausType(Status type)
{
    return m_MapStatusType[type];
}

QString CommonTools::getStatusPixmap(Status type)
{
    return m_MapStatusIcon[type];
}

QString CommonTools::getSystemTime()
{
    QDateTime time = QDateTime::currentDateTime();
    QString date = time.toString("yyyy-M-d hh:mm");
    return date;
}

QString CommonTools::getErrorString(int err)
{
    return m_MapErrorString[err];
}
