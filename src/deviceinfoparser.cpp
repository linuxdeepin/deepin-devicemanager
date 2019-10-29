#include "deviceinfoparser.h"
#include <QObject>
#include <sys/utsname.h>
#include <iostream>
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <DLog>
#include <com_deepin_daemon_power.h>
#include "hwinfohandler.h"
#include "deviceattributedefine.h"
#include <DApplication>
#include "logpasswordauth.h"
#include "DMessageBox"
#include <cups.h>

DWIDGET_USE_NAMESPACE

using PowerInter = com::deepin::daemon::Power;

DCORE_USE_NAMESPACE

static QProcess process_;

bool DeviceInfoParser::isToolSuccess(const QString& toolname)
{
    if(false == toolDatabase_.contains(toolname))
    {
        return false;
    }

    foreach( auto map, toolDatabase_[toolname])
    {
        if( map.size() > 0 )
        {
            return true;
        }
    }

    return false;
}

bool DeviceInfoParser::fuzzeyQueryKey(const QString& toolname, const QString& fuzzeyKey, QString& key)
{
    if(false == toolDatabase_.contains(toolname))
    {
        return false;
    }

    foreach(auto toolKey, toolDatabase_[toolname].keys() )
    {
        if( toolKey.contains(fuzzeyKey) )
        {
            key = toolKey;
            return true;
        }
    }

    return false;
}

const QString& DeviceInfoParser::queryData(const QString& toolname, const QString& firstKey, const QString& secondKey)
{
    static QString result = DApplication::translate("Main", "Unknown");
    if(false == toolDatabase_.contains(toolname))
    {
        return result;
    }

    if(false == toolDatabase_[toolname].contains(firstKey))
    {
        return result;
    }

    if(false == toolDatabase_[toolname][firstKey].contains(secondKey))
    {
        return result;
    }

    return toolDatabase_[toolname][firstKey][secondKey];
}

const QString& DeviceInfoParser::fuzzyQueryData(const QString& toolname, const QString& firstKey, const QString& secondKey)
{
    static QString result = DApplication::translate("Main", "Unknown");
    if(false == toolDatabase_.contains(toolname))
    {
        return result;
    }

    foreach(const QString& fk, toolDatabase_[toolname].uniqueKeys() )
    {
        if( fk.contains(firstKey) )
        {
            foreach(const QString& sk, toolDatabase_[toolname][fk].uniqueKeys() )
            {
                if( sk.contains(secondKey) )
                {
                    return toolDatabase_[toolname][fk][sk];
                }
            }
        }
    }

    return result;
}

bool DeviceInfoParser::queryDeviceInfo(const QString& toolname, const QString& deviceName, QList<ArticleStruct>& articles)
{
    if(false == toolDatabase_.contains(toolname))
    {
        return false;
    }

    if(false == toolDatabase_[toolname].contains(deviceName))
    {
        return false;
    }

    foreach( auto key, toolDatabase_[toolname][deviceName].keys())
    {
        ArticleStruct article;
        article.name = key;
        article.value = toolDatabase_[toolname][deviceName][key];
        articles.push_back(article);
    }

    return true;
}

bool DeviceInfoParser::queryRemainderDeviceInfo(const QString& toolname, const QString& deviceName, QList<ArticleStruct>& articles, const QSet<QString>& existArticles)
{
    if(false == toolDatabase_.contains(toolname))
    {
        return false;
    }

    if(false == toolDatabase_[toolname].contains(deviceName))
    {
        return false;
    }

    foreach( auto key, toolDatabase_[toolname][deviceName].keys())
    {
        if(existArticles.contains(key))
        {
            continue;
        }

        ArticleStruct article(key, true);
        article.value = toolDatabase_[toolname][deviceName][key];
        articles.push_back(article);
    }

    return true;
}

QStringList DeviceInfoParser::getCpuList()
{
    QStringList catcpuList;

    if(false == toolDatabase_.contains("catcpu"))
    {
        return catcpuList;
    }

    foreach(const QString& fk, toolDatabase_["catcpu"].keys() )
    {
        catcpuList.push_back(fk);
    }

    return catcpuList;
}

QStringList DeviceInfoParser::getMemorynameList()
{
    QStringList memList;

    if(false == toolDatabase_.contains("dmidecode"))
    {
        return memList;
    }

    foreach(const QString& fk, toolDatabase_["dmidecode"].uniqueKeys() )
    {
        if( fk == "Memory Device" || fk.contains("Memory Device_"))
        {
            memList.push_back(fk);
        }
    }

    std::sort( memList.begin(), memList.end(),
                [this](const QString& m1, const QString& m2)
                {
                    return toolDatabase_["dmidecode"][m1]["Locator"] < toolDatabase_["dmidecode"][m2]["Locator"] ;
                }
                );

    return memList;
}

QStringList DeviceInfoParser::getDisknameList()
{
    QStringList diskList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return diskList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        int index = fk.lastIndexOf("disk");
        if( index > 0 && fk.size() - index < 7 )
        {
            diskList.push_back(fk);
        }
    }

    return diskList;
}

QStringList DeviceInfoParser::getDiaplayadapterList()
{
    QStringList displayadapterList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return displayadapterList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if( fk.contains("display") )
        {
            displayadapterList.push_back(fk);
        }
    }

    return displayadapterList;
}

QStringList DeviceInfoParser::getScreenName()
{
    QStringList screenList;

    if(false == toolDatabase_.contains("xrandr"))
    {
        return screenList;
    }

    foreach(const QString& fk, toolDatabase_["xrandr"].uniqueKeys() )
    {
        if( fk.startsWith("Screen", Qt::CaseInsensitive) )
        {
            screenList.push_back(fk);;
        }
    }

    return screenList;
}

QStringList DeviceInfoParser::getDisplayInterfaceList()
{
    QStringList interfaceList;

    if(false == toolDatabase_.contains("xrandr"))
    {
        return interfaceList;
    }

    foreach(const QString& fk, toolDatabase_["xrandr"].uniqueKeys() )
    {
        int index = fk.indexOf(Devicetype_Xrandr_Disconnected);
        if(index < 1)
        {
            index = fk.indexOf(Devicetype_Xrandr_Connected);
        }

        if( index < 0 )
        {
            continue;
        }

        QString interface = fk.mid(0, index).trimmed();
        index = interface.indexOf('-');
        if( index > 0 )
        {
            interface = interface.mid(0, index);
        }
        if( false == interfaceList.contains(interface) )
        {
            interfaceList.push_back(interface);
        }
    }

    return interfaceList;
}

QStringList DeviceInfoParser::getHwinfoMonitorList()
{
    QStringList monitorList;

    if(false == toolDatabase_.contains("hwinfo"))
    {
        return monitorList;
    }

    foreach(const QString& fk, toolDatabase_["hwinfo"].uniqueKeys() )
    {
        monitorList.push_back(fk);
    }

    return monitorList;
}

QStringList DeviceInfoParser::getXrandrMonitorList()
{
    QStringList connectedMonitorList;

    if(false == toolDatabase_.contains("xrandr"))
    {
        return connectedMonitorList;
    }

    foreach(const QString& fk, toolDatabase_["xrandr"].uniqueKeys() )
    {
        int index = fk.indexOf(Devicetype_Xrandr_Connected);
        if( index < 0 )
        {
            continue;
        }

//        QString interface = fk.mid(0, index).trimmed();
//        index = interface.indexOf('-');
//        if( index > 0 )
//        {
//            interface = interface.mid(0, index);
//        }

        connectedMonitorList.push_back(fk);
    }

    return connectedMonitorList;
}

QStringList DeviceInfoParser::getMultimediaList()
{
    QStringList multimediaList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return multimediaList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].keys() )
    {
        if( fk.contains("multimedia") )
        {
            multimediaList.push_back(fk);
        }
    }

    return multimediaList;
}

QStringList DeviceInfoParser::getInputAudioDeviceList()
{
    QStringList inputdeviceList;

    if(false == toolDatabase_.contains("catinput"))
    {
        return inputdeviceList;
    }

    foreach(const QString& fk, toolDatabase_["catinput"].uniqueKeys() )
    {
        if(false ==toolDatabase_["catinput"][fk].contains("Name"))
        {
            continue;
        }

        QString name = toolDatabase_["catinput"][fk]["Name"];
        if(     false == name.contains("Speaker", Qt::CaseInsensitive) \
            &&  false == name.contains("Headphone", Qt::CaseInsensitive) \
            &&  false == name.contains("Mic", Qt::CaseInsensitive) )
        {
            continue;
        }

        inputdeviceList.push_back(fk);
    }

    return inputdeviceList;
}

QStringList DeviceInfoParser::getInputdeviceList()
{
    QStringList inputdeviceList;

    if(false == toolDatabase_.contains("catinput"))
    {
        return inputdeviceList;
    }

    foreach(const QString& fk, toolDatabase_["catinput"].uniqueKeys() )
    {
        inputdeviceList.push_back(fk);
    }

    return inputdeviceList;
}

QStringList DeviceInfoParser::getNetworkadapterList()
{
    QStringList networkadapterList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return networkadapterList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if( fk.contains("network") )
        {
            networkadapterList.push_back(fk);
        }
    }

    return networkadapterList;
}

QStringList DeviceInfoParser::getBluetoothList()
{
    QStringList bluetoothList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return bluetoothList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].keys() )
    {
        if( toolDatabase_["lshw"][fk].contains("description") == false )
        {
            continue;
        }

        if( toolDatabase_["lshw"][fk]["description"].contains("Bluetooth", Qt::CaseInsensitive) )
        {
            bluetoothList.push_back(fk);
        }
    }

    return bluetoothList;
}

QStringList DeviceInfoParser::getHciconfigBluetoothList()
{
    QStringList bluetoothList;

    if(false == toolDatabase_.contains("hciconfig"))
    {
        return bluetoothList;
    }

    foreach(const QString& fk, toolDatabase_["hciconfig"].keys() )
    {
        bluetoothList.push_back(fk);
    }

    return bluetoothList;
}

QStringList DeviceInfoParser::getCameraList()
{
    QStringList cameraList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return cameraList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if(true == toolDatabase_["lshw"][fk].contains("description"))
        {
            if( toolDatabase_["lshw"][fk]["description"] == "Video" )
            {
                cameraList.push_back(fk);
                continue;
            }
        }

        if(true == toolDatabase_["lshw"][fk].contains("product"))
        {
            if( toolDatabase_["lshw"][fk]["product"].contains("Camera", Qt::CaseInsensitive) )
            {
                cameraList.push_back(fk);
                continue;
            }
        }
    }

    return cameraList;
}

QStringList DeviceInfoParser::getOtherUsbdeviceList()
{
    QStringList usbdeviceList;

    //QStringList mouseList = getMouseList();
    //QStringList keyboardList = getKeyboardList();
    //QStringList cameraList = getCameraList();

    if(false == toolDatabase_.contains("lshw"))
    {
        return usbdeviceList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        QRegExp rx("^[\\s\\S]*usb[:0-9]*$");
        if( rx.exactMatch(fk) ==false )
        {
            continue;
        }

//        int index = fk.indexOf( "usb:", -1 );
//        if( index < 0 )
//        {
//            index = fk.indexOf("usb", -1 );
//        }

//        if( index < 0 )
//        {
//            continue;
//        }

//        if(fk.indexOf("usb"), -1)
//        {

//        }

        if( fk.contains("usb", Qt::CaseInsensitive) )
        {
            if( fk.contains("multimedia") )
            {
                continue;
            }

            if(true == toolDatabase_["lshw"][fk].contains("description"))
            {
                if( toolDatabase_["lshw"][fk]["description"].contains("USB controller", Qt::CaseInsensitive) )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["description"].contains("Host Controller", Qt::CaseInsensitive) )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["description"].contains("USB hub", Qt::CaseInsensitive) )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["description"].contains("mouse", Qt::CaseInsensitive) )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["description"].contains("keyboard", Qt::CaseInsensitive) )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["description"] == "Video" )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["description"].contains("Bluetooth", Qt::CaseInsensitive) )
                {
                    continue;
                }
            }

            if(true == toolDatabase_["lshw"][fk].contains("product"))
            {
                if( toolDatabase_["lshw"][fk]["product"].contains("USB controller", Qt::CaseInsensitive) )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["product"].contains("Host Controller", Qt::CaseInsensitive) )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["product"].contains("USB hub", Qt::CaseInsensitive) )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["product"].contains("mouse", Qt::CaseInsensitive) )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["product"].contains("keyboard", Qt::CaseInsensitive) )
                {
                    continue;
                }

                if( toolDatabase_["lshw"][fk]["product"].contains("Camera", Qt::CaseInsensitive) )
                {
                    continue;
                }
            }

            usbdeviceList.push_back(fk);
        }
    }

    return usbdeviceList;
}

QStringList DeviceInfoParser::getPS_2MouseInputdeviceList()
{
    QStringList inputdeviceList;

    if(false == toolDatabase_.contains("catinput"))
    {
        return inputdeviceList;
    }

    foreach(const QString& fk, toolDatabase_["catinput"].keys() )
    {
        if( false == toolDatabase_["catinput"][fk].contains("Name") )
        {
            continue;
        }

        if( false == toolDatabase_["catinput"][fk]["Name"].contains("mouse", Qt::CaseInsensitive))
        {
            continue;
        }

        if( toolDatabase_["catinput"][fk].contains("Phys") && toolDatabase_["catinput"][fk]["Phys"].contains("usb", Qt::CaseInsensitive) )// remove dumplicate usb mouse
        {
            continue;
        }

//        if( toolDatabase_["catinput"][fk]["Name"].contains("PS/2", Qt::CaseInsensitive) )
//        {
            inputdeviceList.push_back(fk);
//        }
    }

    return inputdeviceList;
}

QStringList DeviceInfoParser::getMouseList()
{
    QStringList mouseList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return mouseList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].keys() )
    {
        if(true == toolDatabase_["lshw"][fk].contains("description"))
        {
            if( toolDatabase_["lshw"][fk]["description"].contains("mouse", Qt::CaseInsensitive) )
            {
                mouseList.push_back(fk);
                continue;
            }
        }

        if(true == toolDatabase_["lshw"][fk].contains("product"))
        {
            if( toolDatabase_["lshw"][fk]["product"].contains("mouse", Qt::CaseInsensitive) )
            {
                mouseList.push_back(fk);
                continue;
            }
        }
    }

    return mouseList;
}

QStringList DeviceInfoParser::getPS_2KeyboardInputdeviceList()
{
    QStringList inputdeviceList;

    if(false == toolDatabase_.contains("catinput"))
    {
        return inputdeviceList;
    }

    foreach(const QString& fk, toolDatabase_["catinput"].keys() )
    {
        if( false == toolDatabase_["catinput"][fk].contains("Name") )
        {
            continue;
        }

        if( false == toolDatabase_["catinput"][fk]["Name"].contains("keyboard", Qt::CaseInsensitive) )
        {
            continue;
        }     

        if( toolDatabase_["catinput"][fk].contains("Phys") && toolDatabase_["catinput"][fk]["Phys"].contains("usb", Qt::CaseInsensitive) )// remove dumplicate usb keyboard
        {
            continue;
        }

//        if( toolDatabase_["catinput"][fk]["Name"].contains("PS/2", Qt::CaseInsensitive))
//        {
            inputdeviceList.push_back(fk);
//        }
    }

    return inputdeviceList;
}

QStringList DeviceInfoParser::getKeyboardList()
{
    QStringList keyboardList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return keyboardList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if(true == toolDatabase_["lshw"][fk].contains("description"))
        {
            if( toolDatabase_["lshw"][fk]["description"].contains("keyboard", Qt::CaseInsensitive) )
            {
                keyboardList.push_back(fk);
                continue;
            }
        }

        if(true == toolDatabase_["lshw"][fk].contains("product"))
        {
            if( toolDatabase_["lshw"][fk]["product"].contains("keyboard", Qt::CaseInsensitive) )
            {
                keyboardList.push_back(fk);
                continue;
            }
        }
    }

    return keyboardList;
}

QStringList DeviceInfoParser::getSwitchingpowerList()
{
    QStringList switchingpowerList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return switchingpowerList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if(fk.contains("power", Qt::CaseInsensitive))
        {
            switchingpowerList.push_back(fk);
        }
    }

    return switchingpowerList;
}

QStringList DeviceInfoParser::getSwitchingUpowerList()
{
    QStringList switchingpowerList;

    if(false == toolDatabase_.contains("upower"))
    {
        return switchingpowerList;
    }

    foreach(const QString& fk, toolDatabase_["upower"].uniqueKeys() )
    {
        if(fk.contains("line_power", Qt::CaseInsensitive))
        {
            switchingpowerList.push_back(fk);
        }
    }

    return switchingpowerList;
}

QStringList DeviceInfoParser::getDemidecodeSwitchingpowerList()
{
    QStringList switchingpowerList;

    if(false == toolDatabase_.contains("dmidecode"))
    {
        return switchingpowerList;
    }

    foreach(const QString& fk, toolDatabase_["dmidecode"].uniqueKeys() )
    {
        if(fk.contains("System Power Supply", Qt::CaseInsensitive))
        {
            switchingpowerList.push_back(fk);
        }
    }

    return switchingpowerList;
}

QStringList DeviceInfoParser::getBatteryList()
{
    QStringList batteryList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return batteryList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if(fk.contains("battery", Qt::CaseInsensitive))
        {
            batteryList.push_back(fk);
        }
    }

    return batteryList;
}

QStringList DeviceInfoParser::getDemidecodeBatteryList()
{
    QStringList batteryList;

    if(false == toolDatabase_.contains("dmidecode"))
    {
        return batteryList;
    }

    foreach(const QString& fk, toolDatabase_["dmidecode"].uniqueKeys() )
    {
        if(fk.contains("Battery", Qt::CaseInsensitive))
        {
            batteryList.push_back(fk);
        }
    }

    return batteryList;
}

QStringList DeviceInfoParser::getBatteryUpowerList()
{
    QStringList batteryList;

    if(false == toolDatabase_.contains("upower"))
    {
        return batteryList;
    }

    foreach(const QString& fk, toolDatabase_["upower"].uniqueKeys() )
    {
        if(fk.contains("battery", Qt::CaseInsensitive))
        {
            batteryList.push_back(fk);
        }
    }

    return batteryList;
}

QStringList DeviceInfoParser::getOtherInputdeviceList()
{
    QStringList otherInputdeviceList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return otherInputdeviceList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        QString product =toolDatabase_["lshw"][fk]["product"];
        QString description =toolDatabase_["lshw"][fk]["description"];

        if( product.contains("touchpad", Qt::CaseInsensitive) || description.contains("touchpad", Qt::CaseInsensitive) || \
            product.contains("scanner", Qt::CaseInsensitive) || description.contains("scanner", Qt::CaseInsensitive) || \
            product.contains("Joystick", Qt::CaseInsensitive) || description.contains("Joystick", Qt::CaseInsensitive) ||
            product.contains("Handwriting", Qt::CaseInsensitive) || description.contains("Handwriting", Qt::CaseInsensitive) ||
            product.contains("Voice input", Qt::CaseInsensitive) || description.contains("Voice input", Qt::CaseInsensitive)  )
        {
            otherInputdeviceList.push_back(fk);
        }
    }

    return otherInputdeviceList;
}

QStringList DeviceInfoParser::getOtherPciDeviceList()
{
    QStringList otherPcideviceList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return otherPcideviceList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        QRegExp re("^[\\s\\S]*_pci_[\\S\\s]*pci:[\\d]?_[\\S\\s]*$");

        if( re.exactMatch(fk) )
        {
            if( fk.contains("display", Qt::CaseInsensitive) || fk.contains("network", Qt::CaseInsensitive) )
            {
                continue;
            }

            otherPcideviceList.push_back(fk);
        }
    }

    return otherPcideviceList;
}

QStringList DeviceInfoParser::getPrinterList()
{
    QStringList printerList;

    if(false == toolDatabase_.contains("Cups"))
    {
        return printerList;
    }

    foreach(const QString& fk, toolDatabase_["Cups"].uniqueKeys() )
    {
        printerList.push_back(fk);
    }

    return printerList;
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
        osInfo = DApplication::translate("Main", "Unknown");
        return false;
    }

    QString str = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    executeProcess("tar zxvf /home/archermind/Desktop/deviceInfo.tar.gz");

    QFile osinfoFile("./deviceInfo/osinfo.txt");
    if( false == osinfoFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    str = osinfoFile.readAll();
    osinfoFile.close();
#endif

     QString linuxCoreVerson;
     QString releaseVersion;

     QRegExp rx("^([\\s\\S]*)\\([\\w!#$%&'*+/=?^_`{|}~-]+(?:\\.[\\w!#$%&'*+/=?^_`{|}~-]+)*@(?:[\\w](?:[\\w-]*[\\w])?\\.)+[\\w](?:[\\w-]*[\\w])?\\)([\\s\\S]*)$");
     if( rx.exactMatch(str) )
     {
        linuxCoreVerson = rx.cap(1).trimmed();
        releaseVersion = rx.cap(2).trimmed();

        rx.setPattern("^(\\(gcc version [\\d-.]*)[\\s\\S]*$");
        if( rx.exactMatch( releaseVersion ) )
        {
            releaseVersion.remove(rx.cap(1));
            int index = releaseVersion.indexOf(")");
            releaseVersion.remove( index, 1 );
        }

        osInfo = linuxCoreVerson + releaseVersion;
     }
     else
     {
        osInfo = standOutput_;
     }

    osInfo.remove("version");
    QString lsbRelease = queryData("catOsrelease", "catOsrelease", "PRETTY_NAME");
    lsbRelease.remove("\"");
    QString homeUrl = queryData("catOsrelease", "catOsrelease", "HOME_URL");
    homeUrl.remove("\"");
    if(lsbRelease.isEmpty() || lsbRelease ==  DApplication::translate("Main", "Unknown"))
    {
        lsbRelease = queryData("lsb_release", "lsb_release", "Description");
    }

    if(lsbRelease.isEmpty() || lsbRelease ==  DApplication::translate("Main", "Unknown"))
    {
        return true;
    }

    if( homeUrl.isEmpty() == false && homeUrl !=  DApplication::translate("Main", "Unknown"))
    {
        osInfo =  "<style> a {text-decoration: none} </style><a href=\"" + homeUrl + "\">" + lsbRelease + " </a> \t" + osInfo;
    }
    else
    {
        osInfo = lsbRelease + " / " + osInfo;
    }

    return true;
}

bool DeviceInfoParser::loadCatosrelelease()
{
    if( false == executeProcess("cat /etc/os-release"))
    {
        return false;
    }

    QString CatosreleleaseOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lsb_releaseFile("./deviceInfo/os-release.txt");
    if( false == lsb_releaseFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    CatosreleleaseOut = lsb_releaseFile.readAll();
    lsb_releaseFile.close();
#endif

    // lscpu
    QMap<QString, QString> osreleaseDatabase_;

    int startIndex = 0;

    for( int i = 0; i < CatosreleleaseOut.size(); ++i )
    {
         if( CatosreleleaseOut[i] != '\n' && i != CatosreleleaseOut.size() -1 )
         {
             continue;
         }

         QString line = CatosreleleaseOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         if( line.contains(DeviceType_CatDevice_Separator) )
         {
             QStringList strList = line.split(DeviceType_CatDevice_Separator);
             osreleaseDatabase_[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] = strList.last().trimmed();
         }
    }

    DatabaseMap rootlscuDb;
    rootlscuDb["catOsrelease"] = osreleaseDatabase_;
    toolDatabase_["catOsrelease"] = rootlscuDb;
    return true;
}

bool DeviceInfoParser::loadlsb_release()
{
    if( false == executeProcess("lsb_release -a"))
    {
        return false;
    }

    QString lsb_releaseOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lsb_releaseFile("./deviceInfo/lsb_release.txt");
    if( false == lsb_releaseFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    lsb_releaseOut = lsb_releaseFile.readAll();
    lsb_releaseFile.close();
#endif

    // lscpu
    QMap<QString, QString> lsb_releaseDatabase_;

    int startIndex = 0;

    for( int i = 0; i < lsb_releaseOut.size(); ++i )
    {
         if( lsb_releaseOut[i] != '\n' && i != lsb_releaseOut.size() -1 )
         {
             continue;
         }

         QString line = lsb_releaseOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         if( line.contains(Devicetype_Separator) )
         {
             QStringList strList = line.split(Devicetype_Separator);
             lsb_releaseDatabase_[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] = strList.last().trimmed();
         }
    }

    DatabaseMap rootlscuDb;
    rootlscuDb["lsb_release"] = lsb_releaseDatabase_;
    toolDatabase_["lsb_release"] = rootlscuDb;
    return true;
}

bool DeviceInfoParser::loadDemicodeDatabase()
{
    if( false == executeProcess("sudo dmidecode"))
    {
        return false;
    }

    QString dmidecodeOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile dmidecodeFile("./deviceInfo/dmidecode.txt");
    if( false == dmidecodeFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    dmidecodeOut = dmidecodeFile.readAll();
    dmidecodeFile.close();
#endif

    // dimdecode
    DatabaseMap dimdecodeDatabase_;

    int startIndex = 0;
    int lineNumber = 0;
    QString deviceType;
    QString childDeviceType;
    QString childDeviceContent;

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
            QRegExp rx("^# dmidecode ([\\d]*.[\\d]*)$");
            if( rx.indexIn(line) > -1 )
            {
                DeviceInfoMap["version"] = rx.cap(1);
                dimdecodeDatabase_["dmidecode"] = DeviceInfoMap;
                DeviceInfoMap.clear();
                continue;
            }

            //  SMBIOS 3.0.0 present.
            rx.setPattern("^SMBIOS ([\\d]*.[\\d]*.[\\d])+ present.$");
            if( rx.indexIn(line) > -1 )
            {
                DeviceInfoMap["version"] = rx.cap(1);
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

        if( line.startsWith("\t\t") && childDeviceType.isEmpty() == false )
        {
            if(childDeviceContent.isEmpty() ==false)
            {
                childDeviceContent += ", ";
            }
            childDeviceContent += line.trimmed();
            continue;
        }

        if( line.startsWith('\t') )
        {
            if(childDeviceType.isEmpty() == false)
            {
                DeviceInfoMap[childDeviceType] = childDeviceContent;
                childDeviceType.clear();
                childDeviceContent.clear();
            }

            if( line.contains(':') )
            {
                QStringList strList = line.split(':');
                if(strList.last().trimmed().isEmpty() == true)
                {
                    childDeviceType = strList[0].trimmed();
                }
                else if(strList.size() == 2)
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

    toolDatabase_["dmidecode"] = dimdecodeDatabase_;
    return  true;
}

bool DeviceInfoParser::loadLshwDatabase()
{
    if( false == executeProcess("sudo lshw"))
    {
        return false;
    }

    QString lshwOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lshwFile("./deviceInfo/lshw.txt");
    if( false == lshwFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    lshwOut = lshwFile.readAll();
    lshwFile.close();
#endif
    // lshw
    DatabaseMap lshwDatabase_;

    int startIndex = 0;
    int lineNumber = -1;
    QStringList deviceType;
    QMap<QString, QString> DeviceInfoMap;

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
            foreach(auto dt, deviceType)
            {
                if(deviceTypeName.isEmpty() == false)
                {
                    deviceTypeName += Devicetype_Stitching_Symbol;
                }
                deviceTypeName += dt.trimmed().remove(Devicetype_lshw_Class_Prefix);
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

        int index = line.indexOf(Devicetype_Separator);
        if( index > 0 )
        {
            QString name = line.mid(0,index).trimmed().remove(Devicetype_lshw_Class_Prefix);
            if(name == "configuration" || name == "resources" )
            {
                QChar splitChar = name == "configuration" ? '=': ':';

                QStringList lst = line.mid(index+1).trimmed().split(splitChar);
                if(lst.size() < 2)
                {
                    DeviceInfoMap[name] = line.mid(index+1).trimmed();
                }
                else
                {
                    for(int ind = 0; ind < lst.size() - 1; ++ind)
                    {
                        QString tempName = lst[ind].split(" ").last();
                        int spaceIndex = lst[ind+1].lastIndexOf(" ");
                        if(spaceIndex < 0)
                        {
                            if(DeviceInfoMap.contains(tempName))
                            {
                                DeviceInfoMap[tempName] += ",";
                                DeviceInfoMap[tempName] += lst[ind+1];
                            }
                            else {
                                DeviceInfoMap[tempName] = lst[ind+1];
                            }
                        }
                        else
                        {
                            if(DeviceInfoMap.contains(tempName))
                            {
                                DeviceInfoMap[tempName] += ",";
                                DeviceInfoMap[tempName] += lst[ind+1].mid(0, spaceIndex);
                            }
                            else {
                                DeviceInfoMap[tempName] = lst[ind+1].mid(0, spaceIndex);
                            }
                        }
                    }
                }
            }
            else
                DeviceInfoMap[name] = line.mid(index+1).trimmed();
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

    toolDatabase_["lshw"] = lshwDatabase_;
    return true;
}

bool DeviceInfoParser::loadLscpuDatabase()
{
    if( false == executeProcess("sudo lscpu"))
    {
        return false;
    }

    QString lscpuOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lscpuFile("./deviceInfo/lscpu.txt");
    if( false == lscpuFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    lscpuOut = lscpuFile.readAll();
    lscpuFile.close();
#endif

    // lscpu
    QMap<QString, QString> lscpuDatabase_;

    int startIndex = 0;

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

    DatabaseMap rootlscuDb;
    rootlscuDb["lscpu"] = lscpuDatabase_;
    toolDatabase_["lscpu"] = rootlscuDb;
    return true;
}

bool DeviceInfoParser::loadCatcpuDatabase()
{
    if( false == executeProcess("cat /proc/cpuinfo"))
    {
        return false;
    }

    QString catCpuOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile inputDeviceFile("./deviceInfo/catcpu.txt");
    if( false == inputDeviceFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    catCpuOut = inputDeviceFile.readAll();
    inputDeviceFile.close();
#endif

    // cat /proc/bus/input/devices
    DatabaseMap catcpuDatabase_;

    QMap<QString, QString> DeviceInfoMap;
    int startIndex = 0;
    QString cpuName;

    for( int i = 0; i < catCpuOut.size(); ++i )
    {
         if( catCpuOut[i] != '\n')
         {
             continue;
         }

         QString line = catCpuOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         if( line.trimmed().isEmpty() && false == cpuName.isEmpty() )
         {
             catcpuDatabase_[cpuName] = DeviceInfoMap;
             DeviceInfoMap.clear();
             cpuName = "";
             continue;
         }

         int index = line.indexOf(":");
         if(index > 0)
         {
            QString name = line.mid(0, index).trimmed();
            DeviceInfoMap[name] = line.mid(index+1).trimmed();
            if(name == "processor")
            {
                cpuName = line.mid(index+1).trimmed();
            }
         }
    }

    if( false == cpuName.isEmpty() )
    {
        catcpuDatabase_[cpuName] = DeviceInfoMap;
    }
    toolDatabase_["catcpu"] = catcpuDatabase_;
    return true;
}

bool DeviceInfoParser::loadSmartctlDatabase(const QString& diskLogical)
{
#ifndef TEST_DATA_FROM_FILE
    if( false == executeProcess("sudo smartctl --all " + diskLogical))
    {
        if( false == executeProcess("sudo chmod +x smartctl") )
        {
            return false;
        }

        if( false == executeProcess("sudo ./smartctl --all " + diskLogical))
        {
            return false;
        }
    }
#endif
    QString smartctlOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile smartctlFile("./deviceInfo/smartctl.txt");
    if( false == smartctlFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    smartctlOut = smartctlFile.readAll();
    smartctlFile.close();
#endif

    // smartctl
    QMap<QString, QString> smartctlDatabase_;
    int startIndex = 0;

    QRegExp reg("^[\\s\\S]*[\\d]:[\\d][\\s\\S]*$");

    for( int i = 0; i < smartctlOut.size(); ++i )
    {
         if( smartctlOut[i] != '\n' && i != smartctlOut.size() -1 )
         {
             continue;
         }

         QString line = smartctlOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;


         if( line.contains(Devicetype_Separator) && reg.exactMatch(line) == false )
         {
             int index = line.indexOf(Devicetype_Separator);
             smartctlDatabase_[line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
             continue;
         }

         QRegExp rx("^[ ]*[0-9]+[ ]+([\\w-_]+)[ ]+0x[0-9a-fA-F-]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+([0-9\\/w-]+[ ]*[ 0-9\\/w-()]*)$");
         if( rx.indexIn(line) > -1 )
         {
             smartctlDatabase_[rx.cap(1)] = rx.cap(2);
             continue;
         }

         if(line.contains("Power_On_Hours"))
         {
             smartctlDatabase_["Power_On_Hours"] = line.split(" ").last();
             continue;
         }

         if(line.contains("Power_Cycle_Count"))
         {
             smartctlDatabase_["Power_Cycle_Count"] = line.split(" ").last();
             continue;
         }
    }

    DatabaseMap rootsmartctlDb;
    rootsmartctlDb[diskLogical] = smartctlDatabase_;
    toolDatabase_["smartctl"] = rootsmartctlDb;

    return true;
}

bool DeviceInfoParser::loadCatInputDatabase()
{
    if( false == executeProcess("cat /proc/bus/input/devices"))
    {
        return false;
    }

    QString inputDeviceOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile inputDeviceFile("./deviceInfo/input.txt");
    if( false == inputDeviceFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    inputDeviceOut = inputDeviceFile.readAll();
    inputDeviceFile.close();
#endif

    // cat /proc/bus/input/devices
    DatabaseMap catInputDeviceDatabase_;

    QMap<QString, QString> DeviceInfoMap;
    int startIndex = 0;

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

            if( cutLine.indexOf(DeviceType_CatDevice_Separator, first_index+1) < 0)
            {
                QStringList strList = cutLine.split(DeviceType_CatDevice_Separator);
                DeviceInfoMap[strList.first().trimmed()] = strList.last().trimmed();
                continue;
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

    toolDatabase_["catinput"] = catInputDeviceDatabase_;
    return true;
}

bool DeviceInfoParser::loadXrandrDatabase()
{
    if( false == executeProcess("sudo xrandr --verbose"))
    {
        return false;
    }

    QString xrandrOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile xrandrFile("./deviceInfo/xrandr.txt");
    if( false == xrandrFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    xrandrOut = xrandrFile.readAll();
    xrandrFile.close();
#endif

    // xrandr --verbose
    DatabaseMap xrandrDatabase_;
    QMap<QString, QString> DeviceInfoMap;

    int startIndex = 0;
    QString title;
    QString deviceType;
    QString content;

    for( int i = 0; i < xrandrOut.size(); ++i )
    {
        if( xrandrOut[i] != '\n' && i != xrandrOut.size() -1)
        {
            continue;
        }

        QString line = xrandrOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if( i == xrandrOut.size() -1)
        {
            content += line;
            DeviceInfoMap[deviceType] = content.trimmed();
            if(title.isEmpty() == false && DeviceInfoMap.size() > 0 )
            {
                xrandrDatabase_[title] = DeviceInfoMap;
            }
            break;
        }

        if( line.startsWith(Devicetype_Xrandr_Screen) && line.contains(Devicetype_Separator) )
        {
            QStringList strList = line.split(Devicetype_Separator);

            DeviceInfoMap.clear();
            foreach(const QString& screenStr, strList.last().split(Devicetype_Xrandr_Screen_Separator) )
            {
                QString contentStr = screenStr.trimmed();
                int index = contentStr.indexOf(Devicetype_Xrandr_Space);
                if( index > 0 )
                {
                    DeviceInfoMap[contentStr.mid(0, index).trimmed()] = contentStr.mid(index+1).trimmed();
                }
            }

            xrandrDatabase_[strList.first().trimmed()] = DeviceInfoMap;
            continue;
        }

        if( line.startsWith(Devicetype_Xrandr_Twotab) || line.startsWith(Devicetype_Xrandr_Twospace) || line.startsWith(Devicetype_Xrandr_TabAndspace))
        {
            content += line;
            continue;
        }

        if( line.startsWith(Devicetype_Xrandr_Tab) )
        {
            if(deviceType.isEmpty() == false)
            {
                DeviceInfoMap[deviceType] = content.trimmed();
            }
            int index = line.indexOf(Devicetype_Separator);
            if(index > 0)
            {
                deviceType = line.mid(0, index).trimmed();
            }

            content = line.mid(index+1);
            continue;
        }

        if( line.contains(Devicetype_Xrandr_Connected) || line.contains(Devicetype_Xrandr_Disconnected) )
        {
            if(deviceType.isEmpty() == false)
            {
                DeviceInfoMap[deviceType] = content;
            }

            if(title.isEmpty() == false && DeviceInfoMap.size() > 0 )
            {
                xrandrDatabase_[title] = DeviceInfoMap;
            }

            DeviceInfoMap.clear();

            title = line.trimmed();
            continue;
        }
    }

    toolDatabase_["xrandr"] = xrandrDatabase_;
    return true;
}

bool DeviceInfoParser::loadPowerSettings()
{
    PowerInter power("com.deepin.daemon.Power", "/com/deepin/daemon/Power", QDBusConnection::sessionBus(), nullptr);
    // switchingpower
    switchingpowerScreenSuspendDelay_ = power.linePowerScreenBlackDelay();     //screen suspend delay seconds
    switchingpowerComputerSuspendDelay_ = power.linePowerSleepDelay();   //computer suspend delay seconds
    switchingpowerAutoLockScreenDelay_ = power.linePowerLockDelay();        //auto lock screen delay seconds
    // Battery
    batteryScreenSuspendDelay_ = power.batteryScreenBlackDelay();
    batteryComputerSuspendDelay_ = power.batterySleepDelay();
    batteryAutoLockScreenDelay_ = power.batteryLockDelay();
    return true;
}

bool DeviceInfoParser::loadUpowerDatabase()
{
    if( false == executeProcess("upower --dump"))
    {
        return false;
    }

    QString upowerOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile hciconfigFile("./deviceInfo/upower.txt");
    if( false == hciconfigFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    upowerOut = hciconfigFile.readAll();
    hciconfigFile.close();
#endif

    // hciconfig
    DatabaseMap upowerDatabase_;
    QMap<QString, QString> DeviceInfoMap;
    QString deviceName;
    int startIndex = 0;

    for( int i = 0; i < upowerOut.size(); ++i )
    {
        if( upowerOut[i] != '\n' && i != upowerOut.size() -1)
        {
            continue;
        }

        QString line = upowerOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        int index = line.indexOf("Device:")+1;
        if( line.startsWith("Device:") || line.trimmed().isEmpty() || i == upowerOut.size() )
        {
            if( deviceName.isEmpty() == false )
            {
                upowerDatabase_[deviceName] = DeviceInfoMap;
            }

            DeviceInfoMap.clear();
            int index2 = line.lastIndexOf("/")+1;
            deviceName = line.mid(index2>index? index2:index);

            continue;
        }

        index = line.indexOf(Devicetype_Separator);
        if( index > 0 )
        {
            DeviceInfoMap[line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
        }
        else
        {
            DeviceInfoMap[line.trimmed()] = "";
        }
    }

    toolDatabase_["upower"] = upowerDatabase_;
    return true;
}

bool DeviceInfoParser::loadLspciDatabase()
{
    if( false == executeProcess("sudo lspci -v"))
    {
        return false;
    }
    QString lspciOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lspciFile("./deviceInfo/lspci.txt");
    if( false == lspciFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    lspciOut = lspciFile.readAll();
    lspciFile.close();
#endif
    // lspci --verbose
    DatabaseMap lspciDatabase_;
    QMap<QString, QString> DeviceInfoMap;
    QString deviceName;

    int startIndex = 0;

    for( int i = 0; i < lspciOut.size(); ++i )
    {

        if( lspciOut[i] != '\n' && i != lspciOut.size() -1)
        {
            continue;
        }

        QString line = lspciOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if(line.contains(Devicetype_Lspci_Tab))
        {
            int index = line.indexOf(Devicetype_Lspci_Seperator);
            if( index > 0 )
            {
                DeviceInfoMap[line.mid(0,index).trimmed()] = line.mid(index+1).trimmed();
            }
            else if(line.contains(Devicetype_Lspci_Memory) )
            {
                if( line.contains(Devicetype_Lspci_non_prefetchable) )
                {
                    QRegExp rx("^[\\s\\S]*\\[size=([\\d]*M)\\]$");
                    if( rx.exactMatch(line) )
                    {
                        DeviceInfoMap[Devicetype_Lspci_Memory] = QString(rx.cap(1)).trimmed();
                    }
                }
                else if( line.contains(Devicetype_Lspci_prefetchable) )
                {
                    QRegExp rx("^[\\s\\S]*\\[size=([\\d]*M)\\]$");
                    if( rx.exactMatch(line) )
                    {
                        DeviceInfoMap[Devicetype_Lspci_Memory] = QString(rx.cap(1)).trimmed();
                    }
                }
            }
            else
            {
                DeviceInfoMap[line.trimmed()] = "";
            }
            continue;
        }

        if(line.trimmed().isEmpty())
        {
            lspciDatabase_[deviceName] = DeviceInfoMap;
            DeviceInfoMap.clear();
            deviceName = "";
            continue;
        }

        int index = line.indexOf(Devicetype_Lspci_Seperator);
        if( index > 0 )
        {
            if(deviceName.isEmpty() == false)
            {
                lspciDatabase_[deviceName] = DeviceInfoMap;
                DeviceInfoMap.clear();
                deviceName = "";
            }
            DeviceInfoMap[Devicetype_Name] = line.mid(index+1).trimmed();
            deviceName = line.mid(0,index);
            continue;
        }

        if( i == lspciOut.size() -1 )
        {
            if(deviceName.isEmpty() == false)
            {
                lspciDatabase_[deviceName] = DeviceInfoMap;
            }
        }
    }

    toolDatabase_["lspci"] = lspciDatabase_;
    return true;
}

bool DeviceInfoParser::loadHciconfigDatabase()
{
    if( false == executeProcess("sudo hciconfig -a"))
    {
        return false;
    }

    QString hciconfigOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile hciconfigFile("./deviceInfo/hciconfig.txt");
    if( false == hciconfigFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    hciconfigOut = hciconfigFile.readAll();
    hciconfigFile.close();
#endif

    // hciconfig
    DatabaseMap hciconfigDatabase_;
    QMap<QString, QString> DeviceInfoMap;
    QString deviceName;
    int startIndex = 0;

    for( int i = 0; i < hciconfigOut.size(); ++i )
    {
        if( hciconfigOut[i] != '\n' && i != hciconfigOut.size() -1)
        {
            continue;
        }

        QString line = hciconfigOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if(line.startsWith(Devicetype_Hciconfig_Hci))
        {
            if(deviceName.isEmpty() == false)
            {
                hciconfigDatabase_[deviceName] = DeviceInfoMap;
            }
            DeviceInfoMap.clear();
            deviceName = line.mid(0, line.indexOf(Devicetype_Separator));
            continue;
        }

        if( line.startsWith(Devicetype_Hciconfig_Multispace) || line.startsWith(Devicetype_Hciconfig_Tab) )
        {
            int index = line.indexOf(Devicetype_Separator);
            if( index > 0 )
            {
                DeviceInfoMap[line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
            }
            else
            {
                DeviceInfoMap[line.trimmed()] = "";
            }
        }

        if( i == hciconfigOut.size() -1 || line.trimmed().isEmpty() )
        {
            if(deviceName.isEmpty() == false)
            {
                hciconfigDatabase_[deviceName] = DeviceInfoMap;
            }
            DeviceInfoMap.clear();
            deviceName = "";
            continue;
        }
    }

    toolDatabase_["hciconfig"] = hciconfigDatabase_;
    return true;
}

bool DeviceInfoParser::loadLsusbDatabase()
{
    // lsusb
    DatabaseMap lsusbDatabase_;
    return true;
}

bool DeviceInfoParser::loadHwinfoDatabase()
{
    if( false == executeProcess("sudo hwinfo --monitor"))
    {
        return false;
    }

    QString hwOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile hwinfoFile("./deviceInfo/hwinfo.txt");
    if( false == hwinfoFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    hwOut = hwinfoFile.readAll();
    hwinfoFile.close();
#endif

    //QString hwOut = getHwMonitorString();
    if( hwOut.size() < 1 )
    {
        return false;
    }

    // hciconfig
    DatabaseMap hwinfoDatabase_;
    QMap<QString, QString> DeviceInfoMap;
    QString deviceName;
    int startIndex = 0;

    for( int i = 0; i < hwOut.size(); ++i )
    {
        if( hwOut[i] != '\n' && i != hwOut.size() -1)
        {
            continue;
        }

        QString line = hwOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if( i == hwOut.size() -1 || line.trimmed().isEmpty() )
        {
            if(deviceName.isEmpty() == false)
            {
                hwinfoDatabase_[deviceName] = DeviceInfoMap;
            }

            DeviceInfoMap.clear();
            deviceName = "";
            continue;
        }

        if(line.startsWith(Devicetype_HwInfo_Fourspace))
        {
            int index = line.indexOf(": ");
            if(index > 0)
            {
                if( line.trimmed().contains(Devicetype_HwInfo_Resolution) )
                {
                    if(DeviceInfoMap.contains(Devicetype_HwInfo_Resolution))
                    {
                        DeviceInfoMap[Devicetype_HwInfo_Currentresolution] += ",";
                        DeviceInfoMap[Devicetype_HwInfo_Currentresolution] +=line.mid(index+1).trimmed();
                    }
                    else
                    {
                        DeviceInfoMap[Devicetype_HwInfo_Currentresolution] = line.mid(index+1).trimmed();
                    }

                    continue;
                }

                DeviceInfoMap[ line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
            }
            continue;
        }

        if(line.startsWith(Devicetype_HwInfo_Twospace))
        {
            int index = line.indexOf(": ");
            if(index > 0)
            {
                if( line.contains(Devicetype_HwInfo_Resolution) )
                {
                    if(DeviceInfoMap.contains(Devicetype_HwInfo_ResolutionList))
                    {
                        DeviceInfoMap[Devicetype_HwInfo_ResolutionList] += ",";
                        DeviceInfoMap[Devicetype_HwInfo_ResolutionList] +=line.mid(index+1).trimmed();
                    }
                    else
                    {
                        DeviceInfoMap[Devicetype_HwInfo_ResolutionList] = line.mid(index+1).trimmed();
                    }

                    continue;
                }

                DeviceInfoMap[ line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
            }
            continue;
        }

        deviceName = line.trimmed();
    }

    toolDatabase_["hwinfo"] = hwinfoDatabase_;
    return true;
}

bool DeviceInfoParser::loadLpstatDatabase()
{
    // lpstat -l -p
    if( false == executeProcess("sudo lpstat -l -p") )
    {
        return false;
    }

    QString lpstatOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lpstatFile("./deviceInfo/lpstat.txt");
    if( false == lpstatFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    lpstatOut = lpstatFile.readAll();
    lpstatFile.close();
#endif

    DatabaseMap lpstatDatabase_;
    QMap<QString, QString> DeviceInfoMap;
    QString deviceName;
    int startIndex = 0;

    for( int i = 0; i < lpstatOut.size(); ++i )
    {
        if( lpstatOut[i] != '\n' && i != lpstatOut.size() -1)
        {
            continue;
        }

        QString line = lpstatOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if( i == lpstatOut.size() -1 || line.trimmed().isEmpty() )
        {
            if(deviceName.isEmpty() == false)
            {
                lpstatDatabase_[deviceName] = DeviceInfoMap;
            }

            DeviceInfoMap.clear();
            deviceName = "";
            continue;
        }

        if( line.startsWith(Devicetype_lpstat_4Space) || line.startsWith(Devicetype_lpstat_Tab) )
        {

        }
    }

    toolDatabase_["lpstat"] = lpstatDatabase_;
    return true;
}

void showDetailedInfo(cups_dest_t *dest, const char* option, QMap<QString, QString>& DeviceInfoMap)
{
    cups_dinfo_t* info = cupsCopyDestInfo(CUPS_HTTP_DEFAULT, dest);

    if ( true == cupsCheckDestSupported(CUPS_HTTP_DEFAULT, dest, info, option, nullptr) )
    {
          ipp_attribute_t *finishings = cupsFindDestSupported(CUPS_HTTP_DEFAULT, dest, info, option);
          int i, count = ippGetCount(finishings);

          //std::cout << option << " support" << std::endl;
          for (i = 0; i < count; i ++)
          {
              if(option == CUPS_FINISHINGS || option == CUPS_COPIES /*|| option == CUPS_MEDIA */|| \
                      option == CUPS_NUMBER_UP || option == CUPS_ORIENTATION || option == CUPS_PRINT_QUALITY)
              {
                  if(DeviceInfoMap.contains(option))
                  {
                      DeviceInfoMap[option] += ", ";
                      DeviceInfoMap[option] += QString::number(ippGetInteger(finishings, i));
                  }
                  else
                  {
                      DeviceInfoMap[option] = QString::number(ippGetInteger(finishings, i));
                  }

                  //std::cout << option<<" : "<<ippGetInteger(finishings, i)  << std::endl;
              }
              else
              {
                  if(DeviceInfoMap.contains(option))
                  {
                      DeviceInfoMap[option] += ", ";
                      DeviceInfoMap[option] += ippGetString(finishings, i, nullptr);
                  }
                  else
                  {
                      DeviceInfoMap[option] = ippGetString(finishings, i, nullptr);
                  }

                  //std::cout << option<<" : " << ippGetString(finishings, i, nullptr) << std::endl;
              }
          }
    }
    else
    {
        //std::cout << option << " not supported." << std::endl;
    }
}

int getDestInfo(void *user_data, unsigned /*flags*/, cups_dest_t *dest)
{
    DatabaseMap& cupsDatabase = *(reinterpret_cast<DatabaseMap*>(user_data));

    if(dest == nullptr)
    {
        return -1;
    }

    QMap<QString, QString> DeviceInfoMap;

    showDetailedInfo( dest, CUPS_FINISHINGS, DeviceInfoMap );
    showDetailedInfo( dest, CUPS_COPIES, DeviceInfoMap);
    showDetailedInfo( dest, CUPS_MEDIA, DeviceInfoMap );
    showDetailedInfo( dest, CUPS_MEDIA_SOURCE, DeviceInfoMap);
    showDetailedInfo( dest, CUPS_MEDIA_TYPE, DeviceInfoMap);
    showDetailedInfo( dest, CUPS_NUMBER_UP, DeviceInfoMap );
    showDetailedInfo( dest, CUPS_ORIENTATION, DeviceInfoMap );
    showDetailedInfo( dest, CUPS_PRINT_COLOR_MODE, DeviceInfoMap );
    showDetailedInfo( dest, CUPS_PRINT_QUALITY, DeviceInfoMap );
    showDetailedInfo( dest, CUPS_SIDES, DeviceInfoMap);


    //cups_dinfo_t* info = cupsCopyDestInfo(CUPS_HTTP_DEFAULT, dest);

    for(int i = 0; i < dest->num_options; ++i)
    {
        DeviceInfoMap[(dest->options + i)->name] =  (dest->options + i)->value;
        //std::cout << (dest->options + i)->name <<" : " << (dest->options + i)->value << std::endl;
    }

    cupsDatabase[dest->name] = DeviceInfoMap;
    return (1);
}


bool DeviceInfoParser::loadCupsDatabase()
{
    DatabaseMap cupsDatabase;

    cupsEnumDests(CUPS_DEST_FLAGS_NONE, 1000, nullptr, 0, 0, getDestInfo, &cupsDatabase);

    toolDatabase_["Cups"] = cupsDatabase;
    return true;
}

bool DeviceInfoParser::getRootPassword()
{
    bool res = runCmd("id -un");  // file path is fixed. So write cmd direct
    if( res == true && standOutput_.trimmed() == "root" )
    {
        return true;
    }

#ifdef TEST_DATA_FROM_FILE
    return true;
#endif
    if( autoDialog == nullptr )
    {
        autoDialog = new LogPasswordAuth;
    }

    if( autoDialog->getPasswd().isEmpty() )
    {
        if( -1 == autoDialog->exec())
        {
            exit(-1);
        }
    }

    QStringList arg;
    arg << "-c" << "echo " + autoDialog->getPasswd() + " | sudo -S whoami";
    res = runCmd(arg);  // file path is fixed. So write cmd direct
    if( res == false || standOutput_.trimmed() != "root" )
    {
        autoDialog->clearPasswd();
        autoDialog->showMessage(DApplication::translate("Main", "Password Error!"));
        //DMessageBox::warning(nullptr, "", DApplication::translate("Main", "Password Error!"));
        exit(-1);
    }

    return true;
}

bool DeviceInfoParser::executeProcess(const QString& cmd)
{
#ifdef TEST_DATA_FROM_FILE
    return true;
#endif
    if( false == cmd.startsWith("sudo") )
    {
        return runCmd(cmd);
    }

    runCmd("id -un");
    if(standOutput_.trimmed() == "root")
    {
        return runCmd(cmd);
    }

    QStringList arg;
    arg.clear();
    QString newCmd = cmd;

    arg << "-c" << "echo " + autoDialog->getPasswd() + " | sudo -S" + newCmd.remove("sudo");
    return runCmd(arg);
}

bool DeviceInfoParser::runCmd(const QString& cmd)
{
    process_.start(cmd);
    standOutput_ = process_.readAllStandardOutput();
    bool res = process_.waitForFinished();
    standOutput_ = process_.readAllStandardOutput();
    QString errorOut = process_.readAllStandardError();
    return res;
}

bool DeviceInfoParser::runCmd(const QStringList& cmdList)
{
    process_.start("/bin/bash",cmdList);
    bool res = process_.waitForFinished();
    standOutput_ = process_.readAllStandardOutput();
    return res;
}
