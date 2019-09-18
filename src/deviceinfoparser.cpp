#include "deviceinfoparser.h"
#include <QObject>
#include <sys/utsname.h>
#include <iostream>
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <DLog>

DCORE_USE_NAMESPACE

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
        if( dmidecodeOut[i] != '\n' && i != dmidecodeOut.size() -1 )
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
                if(dimdecodeDatabase_.contains(deviceType))
                {
                    QString insteadDeviceType;
                    endIndex = 1;
                    do
                    {
                        insteadDeviceType = deviceType;
                        insteadDeviceType += "_";
                        insteadDeviceType += QString::number(endIndex++);
                    }while(dimdecodeDatabase_.contains(insteadDeviceType));
                    deviceType = insteadDeviceType;
                }

                if(DeviceInfoMap.size() > 0)
                {
                    dimdecodeDatabase_[deviceType] = DeviceInfoMap;
                }
                deviceType = "";
            }

            continue;
        }

        if( lineNumber < 5 )
        {
            //# dmidecode 3.2
            QRegExp rx("^# dmidecode ([\\d]*).([\\d]*)$");
            if( rx.indexIn(line) > -1 )
            {
                DeviceInfoMap["version"] = rx.cap(1)+ "." + rx.cap(2);
                dimdecodeDatabase_["dmidecode"] = DeviceInfoMap;
                DeviceInfoMap.clear();
                continue;
            }

            //  SMBIOS 3.0.0 present.
            rx.setPattern("^SMBIOS ([\\d])*.([\\d])*.([\\d])* present.$");
            if( rx.indexIn(line) > -1 )
            {
                DeviceInfoMap["version"] = rx.cap(1)+ "." + rx.cap(2) + "." + rx.cap(1);
                dimdecodeDatabase_["SMBIOS"] = DeviceInfoMap;
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
                if(dimdecodeDatabase_.contains(deviceType))
                {
                    QString insteadDeviceType;
                    endIndex = 1;
                    do
                    {
                        insteadDeviceType = deviceType;
                        insteadDeviceType += "_";
                        insteadDeviceType += QString::number(endIndex++);
                    }while(dimdecodeDatabase_.contains(insteadDeviceType));
                    deviceType = insteadDeviceType;
                }

                if(DeviceInfoMap.size() > 0)
                {
                    dimdecodeDatabase_[deviceType] = DeviceInfoMap;
                }
            }

            deviceType = line.trimmed();
            DeviceInfoMap.clear();
            continue;
        }
    }

    return  true;
}

bool DeviceInfoParser::loadLshwDatabase()
{
    QFile lshwFile("//home//archermind//Desktop//lshw.txt");
    if( false == lshwFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    int startIndex = 0;
    int lineNumber = -1;
    QStringList deviceType;
    QMap<QString, QString> DeviceInfoMap;

    QString lshwOut = lshwFile.readAll();

    for( int i = 0; i < lshwOut.size(); ++i )
    {
        if( lshwOut[i] != '\n' && i != lshwOut.size() -1 )
        {
            continue;
        }

        ++lineNumber;

        QString line = lshwOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if( line.trimmed().isEmpty() )
        {
            dWarning("DeviceInfoParser::loadLshwDatabase lshw output contains empty line!");
            continue;
        }

        if(lineNumber == 0)
        {
            DeviceInfoMap[Devicetype_Name] = line.trimmed();
            deviceType.push_back(Devicetype_lshw_Class_Prefix+Deviceype_Computer);
            continue;
        }

        if(line.contains(Devicetype_lshw_Class_Prefix))
        {
            QString deviceTypeName;
            foreach(auto deviceType, deviceType)
            {
                if(deviceTypeName.isEmpty() == false)
                {
                    deviceTypeName += Devicetype_Stitching_Symbol;
                }
                deviceTypeName += deviceType.trimmed().remove(Devicetype_lshw_Class_Prefix);
            }

            lshwDatabase_[deviceTypeName] = DeviceInfoMap;

            DeviceInfoMap.clear();
            while( deviceType.size() > 0 )
            {
                if( deviceType.last().indexOf(Devicetype_lshw_Class_Prefix) >= line.indexOf(Devicetype_lshw_Class_Prefix) )
                {
                    deviceType.pop_back();
                }
                else
                {
                    break;
                }
            }

            if( line.contains(Devicetype_Separator) )
            {
                QStringList strList = line.split(Devicetype_Separator);
                DeviceInfoMap[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] = strList.last().trimmed();
            }
            deviceType.push_back(line);
            continue;
        }

        if( line.contains(Devicetype_Separator) )
        {
            QStringList strList = line.split(Devicetype_Separator);
            DeviceInfoMap[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] = strList.last().trimmed();
            continue;
        }
    }

    //last device
    {
        QString deviceTypeName;
        foreach(auto deviceType, deviceType)
        {
            if(deviceTypeName.isEmpty() == false)
            {
                deviceTypeName += Devicetype_Stitching_Symbol;
            }
            deviceTypeName += deviceType.trimmed().remove(Devicetype_lshw_Class_Prefix);
        }

        lshwDatabase_[deviceTypeName] = DeviceInfoMap;
    }

    return true;
}

bool DeviceInfoParser::loadLscpuDatabase()
{
    QFile lscpuFile("//home//archermind//Desktop//lscpu.txt");
    if( false == lscpuFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    int startIndex = 0;
    QString lscpuOut = lscpuFile.readAll();

    for( int i = 0; i < lscpuOut.size(); ++i )
    {
         if( lscpuOut[i] != '\n' && i != lscpuOut.size() -1 )
         {
             continue;
         }

         QString line = lscpuOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         if( line.contains(Devicetype_Separator) )
         {
             QStringList strList = line.split(Devicetype_Separator);
             lscpuDatabase_[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] = strList.last().trimmed();
         }
    }

    return true;
}

bool DeviceInfoParser::loadSmartctlDatabase()
{
    QFile smartctlFile("//home//archermind//Desktop//smartctl.txt");
    if( false == smartctlFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    int startIndex = 0;
    QString smartctlOut = smartctlFile.readAll();

    for( int i = 0; i < smartctlOut.size(); ++i )
    {
         if( smartctlOut[i] != '\n' && i != smartctlOut.size() -1 )
         {
             continue;
         }

         QString line = smartctlOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         if( line.contains(Devicetype_Separator) )
         {
             int index = line.indexOf(Devicetype_Separator);
             smartctlDatabase_[line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
             continue;
         }

         QRegExp rx("^[ ]*[0-9]+[ ]+([\\w-_]+)[ ]+0x[0-9a-fA-F-]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+([0-9\\/w-]+[ ]*[ 0-9\\/w-()]*)$");
         if( rx.indexIn(line) > -1 )
         {
             smartctlDatabase_[rx.cap(1)] = rx.cap(2);
         }
    }

    return true;
}

bool DeviceInfoParser::loadCatInputDatabase()
{
    QFile inputDeviceFile("//home//archermind//Desktop//input.txt");
    if( false == inputDeviceFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    QMap<QString, QString> DeviceInfoMap;
    int startIndex = 0;
    QString inputDeviceOut = inputDeviceFile.readAll();

    for( int i = 0; i < inputDeviceOut.size(); ++i )
    {
         if( inputDeviceOut[i] != '\n')
         {
             continue;
         }

         if( i != inputDeviceOut.size() -1 && inputDeviceOut[i+1] != '\n')
         {
             continue;
         }

         if( DeviceInfoMap.size() > 0 )
         {
            QString deviceSysfs = "InputDevice";
            if( DeviceInfoMap.contains(Devicetype_CatDeviceSysfs) )
            {
                deviceSysfs = DeviceInfoMap[Devicetype_CatDeviceSysfs];
            }

            static int deviceIndex;
            deviceIndex = 0;
            while( catInputDeviceDatabase_.contains(deviceSysfs) )
            {
                ++deviceIndex;
                deviceSysfs = "InputDevice" + QString::number(deviceIndex);
            }

            catInputDeviceDatabase_[deviceSysfs] = DeviceInfoMap;
         }

         DeviceInfoMap.clear();

         QString paragraph = inputDeviceOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         foreach( const QString& line,  paragraph.split('\n'))
         {
            QString cutLine = line.mid(line.indexOf(Devicetype_Separator)+1);
            int first_index = cutLine.indexOf(DeviceType_CatDevice_Separator);
            if( first_index < 0 )
            {
                continue;
            }

            if( cutLine.indexOf(DeviceType_CatDevice_Separator, first_index) < 0)
            {
                QStringList strList = cutLine.split(DeviceType_CatDevice_Separator);
                DeviceInfoMap[strList.first().trimmed()] = strList.last().trimmed();
            }

            foreach( const QString& typeStr, cutLine.split(' ') )
            {
                if( false == typeStr.contains(DeviceType_CatDevice_Separator) )
                {
                    continue;
                }

                QStringList strList = typeStr.split(DeviceType_CatDevice_Separator);
                DeviceInfoMap[strList.first().trimmed()] = strList.last().trimmed();
            }
         }
    }

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
