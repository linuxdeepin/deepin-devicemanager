#include "commontools.h"

#include <QDebug>
#include <QDateTime>

#define ICON_SIZE_WIDTH 36
#define ICON_SIZE_HEIGHT 36

QMap<DriverType, QString> CommonTools::m_MapDriverIcon = {
    {DR_Bluetooth, QString(":/icons/deepin/builtin/icons/bluetooth.svg")}
    , {DR_Camera, QString(":/icons/deepin/builtin/icons/camera.svg")}
    , {DR_Gpu, QString(":/icons/deepin/builtin/icons/GPU.svg")}
    , {DR_Keyboard, QString(":/icons/deepin/builtin/icons/keyboard.svg")}
    , {DR_Sound, QString(":/icons/deepin/builtin/icons/mc.svg")}
    , {DR_Mouse, QString(":/icons/deepin/builtin/icons/mouse.svg")}
    , {DR_Network, QString(":/icons/deepin/builtin/icons/network.svg")}
    , {DR_Printer, QString(":/icons/deepin/builtin/icons/printer.svg")}
    , {DR_Scaner, QString(":/icons/deepin/builtin/icons/scaner.svg")}
    , {DR_Tablet, QString(":/icons/deepin/builtin/icons/tablet.svg")}
    , {DR_WiFi, QString(":/icons/deepin/builtin/icons/Wi-Fi.svg")}
};

QMap<DriverType, QString> CommonTools::m_MapDriverType = {
    {DR_Bluetooth, tr("Bluetooth adapter")}
    , {DR_Camera, tr("Imaging device")}
    , {DR_Gpu, tr("Display adapter")}
    , {DR_Keyboard, tr("Keyboard")}
    , {DR_Sound, tr("Sound card")}
    , {DR_Mouse, tr("Mouse")}
    , {DR_Network, tr("Network adapter")}
    , {DR_Printer, tr("Printer")}
    , {DR_Scaner, tr("Imaging device")}
    , {DR_Tablet, tr("Other Devices")}
    , {DR_WiFi, tr("Wireless network adapter")}
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
    {ST_SUCESS, tr("Installation successful")}
    , {ST_FAILED, tr("Installation failed")}
    , {ST_DOWNLOADING, tr("Downloading")}
    , {ST_INSTALL, tr("Installing")}
    , {ST_NOT_INSTALL, tr("Not installed")}
    , {ST_CAN_UPDATE, tr("Out-of-date")}
    , {ST_WAITING, tr("Waiting")}
};

QMap<int, QString> CommonTools::m_MapErrorString = {
    {EC_NULL, tr("Unknown error")}
    , {EC_NETWORK, tr("Network error")}
    , {EC_CANCEL, tr("Canceled")}
    , {EC_3, tr("EC_3")}
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
