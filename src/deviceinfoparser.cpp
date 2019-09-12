#include "deviceinfoparser.h"
#include <QObject>
#include <sys/utsname.h>
#include <iostream>
#include <QFile>
#include <QProcess>
#include <QRegExp>

static QProcess process_;

DeviceInfoParser::DeviceInfoParser()
{
}

bool DeviceInfoParser::getOSInfo(QString& osInfo)
{
    struct utsname kernel_info;
    int ret = uname(&kernel_info);
    if (ret == 0) {
        char kversion[100] = { 0 };
        strncpy(kversion, kernel_info.release, strlen(kernel_info.release));
    }

    if( false == executeProcess("cat /proc/version") )
    {
        osInfo = "Unknow";
        return false;
    }

    osInfo = standOutput_;
    return true;
}

bool DeviceInfoParser::loadDemicodeDatabase()
{
    QFile dmidecodeFile("//home//archermind//Desktop//dmidecode.txt");
    if( false == dmidecodeFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    QString dmidecodeOut = dmidecodeFile.readAll();

    int startIndex = 0;
    int lineNumber = 0;
    QString deviceType;
    QMap<QString, QString> DeviceInfoMap;

    for( int i = 0; i < dmidecodeOut.size(); ++i )
    {
        if( dmidecodeOut[i] != '\n' )
        {
            continue;
        }

        QString line = dmidecodeOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;
        ++lineNumber;

        if( line.trimmed().isEmpty() )
        {
            if(deviceType.isEmpty() == false)
            {
                static int endIndex = 1;
                if(democodeDatabase_.contains(deviceType))
                {
                    QString insteadDeviceType;
                    endIndex = 1;
                    do
                    {
                        insteadDeviceType = deviceType;
                        insteadDeviceType += "_";
                        insteadDeviceType += QString::number(endIndex++);
                    }while(democodeDatabase_.contains(insteadDeviceType));
                    deviceType = insteadDeviceType;
                }

                if(DeviceInfoMap.size() > 0)
                {
                    democodeDatabase_[deviceType] = DeviceInfoMap;
                }
                deviceType = "";
            }

            continue;
        }

        if( lineNumber < 5 )
        {
            //# dmidecode 3.2
            QRegExp rx("^# dmidecode ([\\d])*.([\\d])*$");
            if( rx.indexIn(line) > -1 )
            {
                DeviceInfoMap["version"] = rx.cap(1)+ "." + rx.cap(2);
                democodeDatabase_["dmidecode"] = DeviceInfoMap;
                DeviceInfoMap.clear();
                continue;
            }

            //  SMBIOS 3.0.0 present.
            rx.setPattern("^SMBIOS ([\\d])*.([\\d])*.([\\d])* present.$");
            if( rx.indexIn(line) > -1 )
            {
                DeviceInfoMap["version"] = rx.cap(1)+ "." + rx.cap(2) + "." + rx.cap(1);
                democodeDatabase_["SMBIOS"] = DeviceInfoMap;
                DeviceInfoMap.clear();
                continue;
            }
        }
        // Handle 0x0002, DMI type 2, 15 bytes
        QRegExp rx("^Handle 0x[0-9a-fA-F]{1,4}, DMI type [\\d]*, [\\d]* bytes$");
        if( rx.exactMatch(line) )
        {
            DeviceInfoMap.clear();
            continue;
        }

        if( line.startsWith('\t') )
        {
            if( line.contains(':') )
            {
                QStringList strList = line.split(':');
                if(strList.size() == 2)
                {
                    DeviceInfoMap[strList[0].trimmed()] = strList[1].trimmed();
                }
            }
            continue;
        }

        //BIOS Information
        if( false == line.endsWith('.') )
        {
            if(deviceType.isEmpty() == false)
            {
                static int endIndex = 1;
                if(democodeDatabase_.contains(deviceType))
                {
                    QString insteadDeviceType;
                    endIndex = 1;
                    do
                    {
                        insteadDeviceType = deviceType;
                        insteadDeviceType += "_";
                        insteadDeviceType += QString::number(endIndex++);
                    }while(democodeDatabase_.contains(insteadDeviceType));
                    deviceType = insteadDeviceType;
                }

                if(DeviceInfoMap.size() > 0)
                {
                    democodeDatabase_[deviceType] = DeviceInfoMap;
                }
            }

            deviceType = line.trimmed();
            DeviceInfoMap.clear();
            continue;
        }
    }

    return  true;
}

bool DeviceInfoParser::getSystemInfo()
{
//    if( false == executeProcess("dmidecode") )
//    {
//        //osInfo = "Unknow";
//        return false;
//    }

    loadDemicodeDatabase();

    //osInfo = standOutput_;
    return true;
}

bool DeviceInfoParser::executeProcess(const QString& cmd)
{

    /*int res = */process_.start(cmd);
    bool res = process_.waitForFinished();
    standOutput_ = process_.readAllStandardOutput();
    std::cout << standOutput_.toStdString() << std::endl;

    return res;
}
