#pragma once

#include <QMap>
#include <QString>

class DeviceInfoParser
{
public:
    DeviceInfoParser();


    bool getOSInfo(QString& osInfo);

// dmidecode pars
    bool loadDemicodeDatabase();
    bool getSystemInfo();

public:
    bool executeProcess(const QString& cmd);
    QString standOutput_;

    QMap<QString, QMap<QString, QString>> democodeDatabase_;
};
