/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "deviceinfoparser.h"
#include <QObject>
#include <sys/utsname.h>
#include <iostream>
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <DLog>
#include <com_deepin_daemon_power.h>
#include "commondefine.h"
#include "deviceattributedefine.h"
#include <DApplication>
//#include "logpasswordauth.h"
#include "DMessageBox"
#include <cups.h>
#include "Logger.h"

DWIDGET_USE_NAMESPACE

//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/deepin_uos_arm64_nvme_disk";
//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/deviceInfo_bug_12495";//读取不到内存的bug
//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/deviceInfo_12417_cpu_bug";//cpu主频为0
//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/deviceInfo_dmidecode_multiProcessor";//dmidecode 有多个Processor information
//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/longxin_3A400_notebook";//monitor size is in error
const QString DEVICEINFO_PATH = "/home/archermind/Desktop/deviceInfo_test";
//#define TEST_DATA_FROM_FILE

using PowerInter = com::deepin::daemon::Power;

DCORE_USE_NAMESPACE

DeviceInfoParser::DeviceInfoParser(): QObject()
{

}

DeviceInfoParser::~DeviceInfoParser()
{

}

void DeviceInfoParser::refreshDabase()
{
    osInfo_.clear();
    homeUrl_.clear();
    lsbRelease_.clear();
    orderedDevices.clear();
    toolDatabase_.clear();
    toolDatabaseSecondOrder_.clear();

    QString defaultLanguage = getenv("LANGUAGE");

    emit loadFinished(tr("Loading Operating System Info..."));

    setenv ("LANGUAGE", "en_US", 1);    //for aviod translate in lscpu...

    loadCatosrelelease();
    loadlsb_release();
    loadOSInfo();
    //DeviceInfoParser::Instance().getOSInfo(osInfo);

    emit loadFinished(tr("Loading SMBios Info..."));
    loadDemicodeDatabase();
    loadCatBoardinfoDatabase();

    emit loadFinished(tr("Loading Hardware Info..."));
    loadLshwDatabase();

    emit loadFinished(tr("Loading Storage Info..."));
    loadAllSmartctlDatabase();

    emit loadFinished(tr("Loading CPU Info..."));
    loadCatcpuDatabase();
    loadLscpuDatabase();

    emit loadFinished(tr("Loading Input Devices Info..."));
    loadCatInputDatabase();

    emit loadFinished(tr("Loading Power Settings..."));
    loadPowerSettings();
    loadUpowerDatabase();

    emit loadFinished(tr("Loading Monitor Info..."));
    loadXrandrDatabase();
    loadHwinfoDatabase();

    emit loadFinished(tr("Loading PCI Devices Info..."));
    loadLspciDatabase();

    emit loadFinished(tr("Loading Bluetooth Device Info..."));
    loadHciconfigDatabase();
    loadAllBluetoothctlControllerDatabase();
    loadAllBluethctlPairedDeviceDatabase();

    emit loadFinished(tr("Loading USB Devices Info..."));
    loadLsusbDatabase();

    setenv ("LANGUAGE", defaultLanguage.toStdString().c_str(), 1);

    emit loadFinished(tr("Loading Printer Info..."));
    //loadCupsDatabase();

    emit loadFinished("finish");
}

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

    foreach(auto toolKey, toolDatabase_[toolname].keys())
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
    static QString result = tr("Unknown");
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
    static QString result = tr("Unknown");
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
//when provide both context and disabiguation,trigger translations
bool DeviceInfoParser::queryRemainderDeviceInfo(const QString& toolname,
                                                const QString& deviceName,
                                                QList<ArticleStruct>& articles,
                                                const QSet<QString>& existArticles,
                                                 const char *context,const char *disambiguation)
{
    if(false == toolDatabase_.contains(toolname))
    {
        return false;
    }

    if(false == toolDatabase_[toolname].contains(deviceName))
    {
        return false;
    }
#if GenerateTsItem
    QFile file(QString("../../translate/%1.txt").arg(context));
    QTextStream *out = nullptr;
    if(file.exists())
        file.remove();
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        out = new QTextStream(&file);
        *out <<"<context>\n"
             <<"<name>"<<context<<"</name>\n";
    }
#endif
    foreach( auto key, toolDatabase_[toolname][deviceName].keys())
    {
        if(existArticles.contains(key))
        {
            continue;
        }
        QString articleName;
        if(context != nullptr && disambiguation != nullptr) {
            articleName = DApplication::translate(context,key.trimmed().toStdString().data(),disambiguation);
#if GenerateTsItem
            if(out != nullptr) {
                *out << "   <message><!-- manul track message -->\n"
                        "       <source>"  <<key.trimmed().toStdString().data() << "</source>\n"
                        "       <comment>" <<disambiguation                     << "</comment>\n"
                        "       <translation type =\"unfinished\"></translation>\n"
                        "   </message>\n";
            }
#endif
            }else {
            articleName = key;
            }
        ArticleStruct article(articleName, true);
        article.value = toolDatabase_[toolname][deviceName][key];
        articles.push_back(article);
      }
#if GenerateTsItem
    *out <<"</context>\n";
    file.close();
    delete out;
#endif

    return true;
}

QStringList DeviceInfoParser::getMatchToolDeviceList(const QString& toolName, checkValueFun_t* checkFunc )
{
    QStringList catcpuList;

    if(false == toolDatabaseSecondOrder_.contains(toolName))
    {
        return catcpuList;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_[toolName] )
    {
        if(checkFunc)
        {
            if((*checkFunc)(fk))
            {
                catcpuList.push_back(fk);
            }
        }
        else
        {
            catcpuList.push_back(fk);
        }
    }

    return catcpuList;
}

QStringList DeviceInfoParser::getCatcpuCpuList()
{
    return getMatchToolDeviceList("catcpu");
}

QStringList DeviceInfoParser::getlscpuCpuList()
{
    return getMatchToolDeviceList("lscpu");
}

QStringList DeviceInfoParser::getDmidecodePhysicMemory()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if(fk == "Physical Memory Array" || fk.startsWith("Physical Memory Array_"))
        {
            DeviceInfoParser::Instance().orderedDevices.insert(fk);
            return true;
        }

        return false;
    };

    return getMatchToolDeviceList("dmidecode", &func );
}

QStringList DeviceInfoParser::getDmidecodeMemoryList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if(fk == "Memory Device" || fk.contains("Memory Device_"))
        {
            DeviceInfoParser::Instance().orderedDevices.insert(fk);
            return true;
        }

        return false;
    };

    QStringList memList = getMatchToolDeviceList("dmidecode", &func );

    return memList;
}

QStringList DeviceInfoParser::getDmidecodeMemoryArrayMappedAddress()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if(fk == "Memory Array Mapped Address")
        {
            DeviceInfoParser::Instance().orderedDevices.insert(fk);
            return true;
        }
        return false;
    };

    QStringList memList = getMatchToolDeviceList("dmidecode", &func );

    return memList;
}

QStringList DeviceInfoParser::getLshwDisknameList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        int index = fk.lastIndexOf("disk");
        if((index > 0 && fk.size() - index < 7 )||fk.lastIndexOf("storage") > 0)    //avoid disk_volume:0
        {
//            int index = fk.lastIndexOf("_");
//            if(index > 0)
//            {
//                DeviceInfoParser::Instance().orderedDevices.insert(fk.left(index));
//            }
            DeviceInfoParser::Instance().orderedDevices.insert(fk);
            return true;
        }

        return false;
    };


    QStringList diskList = getMatchToolDeviceList("lshw", &func );

    return diskList;
}

QStringList DeviceInfoParser::getLshwDiaplayadapterList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if(fk.contains("display", Qt::CaseInsensitive))
        {
            DeviceInfoParser::Instance().orderedDevices.insert(fk);
            return true;
        }

        return false;
    };

    return getMatchToolDeviceList("lshw", &func );
}

QStringList DeviceInfoParser::getXrandrScreenName()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if(fk.contains("Screen", Qt::CaseInsensitive))
        {
            return true;
        }

        return false;
    };

    return getMatchToolDeviceList("xrandr", &func );
}

QStringList DeviceInfoParser::getXrandrDisplayInterfaceList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        int index = fk.indexOf(Devicetype_Xrandr_Disconnected);
        if(index < 1)
        {
            index = fk.indexOf(Devicetype_Xrandr_Connected);
        }

        if( index < 0 )
        {
            return false;
        }

        QString interface = fk.mid(0, index).trimmed();
        index = interface.indexOf('-');
        if( index > 0 )
        {
            interface = interface.mid(0, index);
        }

        return true;
    };

    QStringList interfaceList = getMatchToolDeviceList("xrandr", &func );
    interfaceList.removeDuplicates();

    return interfaceList;
}

QStringList DeviceInfoParser::getHwinfoMonitorList()
{
    return getMatchToolDeviceList("hwinfo" );
}

QStringList DeviceInfoParser::getXrandrMonitorList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        int index = fk.indexOf(Devicetype_Xrandr_Connected);
        if( index < 0 )
        {
            return false;
        }

        return true;
    };

    return getMatchToolDeviceList("xrandr", &func);
}

QString DeviceInfoParser::getEDID()
{
    QStringList monitorList = getXrandrMonitorList();
    foreach(auto firstKey, monitorList) {
        QString edid = queryData("xrandr",firstKey,"EDID__0");
        if(tr("Unknown") != edid  &&
                edid.count()%(QString("00ffffffffffff0030aed86100000000").count()+1) == 0) {
            return edid;
        }
    }
    return QString("");
}

QStringList DeviceInfoParser::getLshwMultimediaList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if( fk.contains("multimedia") )
        {
            DeviceInfoParser::Instance().orderedDevices.insert(fk);
            return true;
        }


        return false;
    };

    return getMatchToolDeviceList("lshw", &func);
}

QStringList DeviceInfoParser::getCatinputAudioDeviceList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if( DeviceInfoParser::Instance().toolDatabase_["catinput"][fk].contains("Sysfs") )
        {
            QString sysfs = DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Sysfs"];
            if( sysfs.contains("/sound/", Qt::CaseInsensitive) )
            {
                return true;
            }
        }

        return false;
    };

    return getMatchToolDeviceList("catinput", &func);
}

QStringList DeviceInfoParser::getCatinputInputdeviceList()
{
    return getMatchToolDeviceList("catinput");
}

QStringList DeviceInfoParser::getLshwNetworkadapterList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if( fk.contains("network") )
        {
            DeviceInfoParser::Instance().orderedDevices.insert(fk);
            return true;
        }

        return false;
    };

    return getMatchToolDeviceList("lshw", &func);
}

QStringList DeviceInfoParser::getLshwBluetoothList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk].contains("description") == false )
        {
            return false;
        }

        if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["description"].contains("Bluetooth", Qt::CaseInsensitive) )
        {
            DeviceInfoParser::Instance().orderedDevices.insert(fk);
            return true;
        }

        if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk].contains("driver") == false )
        {
            return false;
        }

        if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["driver"].contains("btusb", Qt::CaseInsensitive) )
        {
            DeviceInfoParser::Instance().orderedDevices.insert(fk);
            return true;
        }

        return false;
    };

    return getMatchToolDeviceList("lshw", &func);
}

QStringList DeviceInfoParser::getHciconfigBluetoothControllerList()
{
    return getMatchToolDeviceList("hciconfig");
}

QStringList DeviceInfoParser::getOtherBluetoothctlPairedAndConnectedDevicesList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if( DeviceInfoParser::Instance().toolDatabase_["paired-devices"][fk].contains("Connected") )
        {
            if(DeviceInfoParser::Instance().toolDatabase_["paired-devices"][fk]["Connected"].compare("Yes", Qt::CaseInsensitive) == 0)
            {
                return true;
            }

            return false;
        }

        if( DeviceInfoParser::Instance().orderedDevices.contains(fk) == false )
        {
            return true;
        }

        return false;
    };

    return getMatchToolDeviceList("paired-devices", &func);
}

QStringList DeviceInfoParser::getLshwCameraList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if(true == DeviceInfoParser::Instance().toolDatabase_["lshw"][fk].contains("description"))
        {
            if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["description"] == "Video" )
            {
                DeviceInfoParser::Instance().orderedDevices.insert(fk);
                return true;
            }
        }

        if(true == DeviceInfoParser::Instance().toolDatabase_["lshw"][fk].contains("product"))
        {
            if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["product"].contains("Camera", Qt::CaseInsensitive) )
            {
                DeviceInfoParser::Instance().orderedDevices.insert(fk);
                return true;
            }
        }

        if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk].contains("driver") == false )
        {
            return false;
        }

        if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["driver"].contains("uvcvideo", Qt::CaseInsensitive) )
        {
            DeviceInfoParser::Instance().orderedDevices.insert(fk);
            return true;
        }

        return false;
    };

    return getMatchToolDeviceList("lshw", &func);
}

QStringList DeviceInfoParser::getLshwOtherUsbdeviceList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        QRegExp rx("^[\\s\\S]*usb[:0-9]*$");
        if( rx.exactMatch(fk) ==false )
        {
            return false;
        }

        if( fk.contains("usb", Qt::CaseInsensitive) )
        {
            if(true == DeviceInfoParser::Instance().toolDatabase_["lshw"][fk].contains("description"))
            {
                if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["description"].contains("USB controller", Qt::CaseInsensitive) )
                {
                    return false;
                }

                if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["description"].contains("Host Controller", Qt::CaseInsensitive) )
                {
                    return false;
                }

                if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["description"].contains("USB hub", Qt::CaseInsensitive) )
                {
                    return false;
                }
                if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["description"].contains("Keyboard", Qt::CaseInsensitive) )
                {
                    return false;
                }
            }

            if(true == DeviceInfoParser::Instance().toolDatabase_["lshw"][fk].contains("product"))
            {
                if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["product"].contains("USB controller", Qt::CaseInsensitive) )
                {
                    return false;
                }

                if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["product"].contains("Host Controller", Qt::CaseInsensitive) )
                {
                    return false;
                }

                if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["product"].contains("USB hub", Qt::CaseInsensitive) )
                {
                    return false;
                }
            }

            if( DeviceInfoParser::Instance().orderedDevices.contains(fk) == false )
            {
                DeviceInfoParser::Instance().orderedDevices.insert(fk);
                return true;
            }
        }

        return false;
    };

    return getMatchToolDeviceList("lshw", &func);
}

QStringList DeviceInfoParser::getLshwUsbKeyboardDeviceList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        QRegExp rx("^[\\s\\S]*usb[:0-9]*$");
        if( rx.exactMatch(fk) ==false )
        {
            return false;
        }
        if( fk.contains("usb", Qt::CaseInsensitive) )
        {
            if(true == DeviceInfoParser::Instance().toolDatabase_["lshw"][fk].contains("description"))
            {
                if( DeviceInfoParser::Instance().toolDatabase_["lshw"][fk]["description"].contains("Keyboard", Qt::CaseInsensitive) )
                {
                    if( DeviceInfoParser::Instance().orderedDevices.contains(fk) == false )
                    {
                        DeviceInfoParser::Instance().orderedDevices.insert(fk);
                        return true;
                    }
                }
            }
        }
        return false;
    };

    return getMatchToolDeviceList("lshw", &func);
}

QStringList DeviceInfoParser::getInputdeviceMouseList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if( true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk].contains("Name") )
        {
            if( true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Name"].contains("mouse", Qt::CaseInsensitive) || \
                true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Name"].contains("mice", Qt::CaseInsensitive) || \
                true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Name"].contains("TouchPad", Qt::CaseInsensitive) ||\
                true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Name"].contains("TrackPoint", Qt::CaseInsensitive))
            {
                return true;
            }
        }

//        if( true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk].contains("Handlers") )
//        {
//            if( true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Handlers"].contains("mouse", Qt::CaseInsensitive) )
//            {
//                return true;
//            }
//        }


        return false;
    };

    return getMatchToolDeviceList("catinput", &func);
}

QString DeviceInfoParser::getCorrespondLshwMouse(const QString& inputMouse)
{
    QString mouse;

    if(false == toolDatabaseSecondOrder_.contains("lshw"))
    {
        return mouse;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["lshw"] )
    {
        if(true == toolDatabase_["lshw"][fk].contains("product"))
        {
            //if( toolDatabase_["lshw"][fk]["product"].contains("mouse", Qt::CaseInsensitive) || toolDatabase_["lshw"][fk]["product"].contains("TouchPad", Qt::CaseInsensitive))
            if(inputMouse.contains(toolDatabase_["lshw"][fk]["product"]) == true )
            {
                orderedDevices.insert(fk);
                return fk;
            }
        }
    }

    return mouse;
}

QString DeviceInfoParser::getCorrespondBluetoothMouse(const QString& inputMouse)
{
    if(false == toolDatabaseSecondOrder_.contains("paired-devices"))
    {
        return "";
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["paired-devices"] )
    {
        if(inputMouse.compare( fk, Qt::CaseInsensitive ) == 0)
        {
            orderedDevices.insert(fk);
            return fk;
        }
    }

    return "";
}

QStringList DeviceInfoParser::getInputdeviceKeyboardList()
{
    checkValueFun_t func = [](const QString& fk)->bool
    {
        if( true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk].contains("Name") )
        {
            QString t = DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Name"];
            if(     DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Name"].contains("System Control", Qt::CaseInsensitive) \
                 || DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Name"].contains("Consumer Control", Qt::CaseInsensitive)  )
            {
                return false;
            }

            if( DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Name"].contains("Keyboard", Qt::CaseInsensitive) )
            {
                return true;
            }
        }

//        if( true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk].contains("EV") )
//        {
//            if( true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["EV"].contains("120013", Qt::CaseInsensitive) )
//            {
//                return true;
//            }
//        }

//        if( true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk].contains("Handlers") )
//        {
//            if( true == DeviceInfoParser::Instance().toolDatabase_["catinput"][fk]["Handlers"].contains("kbd", Qt::CaseInsensitive) )
//            {
//                return true;
//            }
//        }

        return false;
    };

    return getMatchToolDeviceList("catinput", &func);
}

QString DeviceInfoParser::getCorrespondLshwKeyboard(const QString& inputKeyboard)
{
    if(false == toolDatabaseSecondOrder_.contains("lshw"))
    {
        return "";
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["lshw"] )
    {
        if(true == toolDatabase_["lshw"][fk].contains("product"))
        {
            //if( toolDatabase_["lshw"][fk]["product"].contains("mouse", Qt::CaseInsensitive) || toolDatabase_["lshw"][fk]["product"].contains("TouchPad", Qt::CaseInsensitive))
            if(inputKeyboard.contains(toolDatabase_["lshw"][fk]["product"]) == true )
            {
                orderedDevices.insert(fk);
                return fk;
            }
        }
    }

    return "";
}

QString DeviceInfoParser::getCorrespondBluetoothKeyboard(const QString& inputKeyboard)
{
    if(false == toolDatabaseSecondOrder_.contains("paired-devices"))
    {
        return "";
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["paired-devices"] )
    {
        if(inputKeyboard.compare( fk, Qt::CaseInsensitive ) == 0)
        {
            orderedDevices.insert(fk);
            return fk;
        }
    }

    return "";
}

QString DeviceInfoParser::getCorrespondUpower(const QString& bluetoothDevice)
{
    if(false == toolDatabaseSecondOrder_.contains("upower"))
    {
        return "";
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["upower"] )
    {
        if( toolDatabase_["upower"][fk].contains("serial") == false)
        {
            return "";
        }

        if(toolDatabase_["upower"][fk]["serial"].compare(bluetoothDevice) == 0)
        {
            return fk;
        }
    }

    return "";
}

QStringList DeviceInfoParser::getLshwSwitchingpowerList()
{
    QStringList switchingpowerList;

    if(false == toolDatabaseSecondOrder_.contains("lshw"))
    {
        return switchingpowerList;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["lshw"] )
    {
        if(fk.contains("power", Qt::CaseInsensitive))
        {
            switchingpowerList.push_back(fk);
            orderedDevices.insert(fk);
        }
    }

    return switchingpowerList;
}

QStringList DeviceInfoParser::getUpowerSwitchingList()
{
    QStringList switchingpowerList;

    if(false == toolDatabaseSecondOrder_.contains("upower"))
    {
        return switchingpowerList;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["upower"] )
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

    if(false == toolDatabaseSecondOrder_.contains("dmidecode"))
    {
        return switchingpowerList;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["dmidecode"] )
    {
        if(fk.contains("System Power Supply", Qt::CaseInsensitive))
        {
            switchingpowerList.push_back(fk);
        }
    }

    return switchingpowerList;
}

QStringList DeviceInfoParser::getLshwBatteryList()
{
    QStringList batteryList;

    if(false == toolDatabaseSecondOrder_.contains("lshw"))
    {
        return batteryList;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["lshw"] )
    {
        if(fk.contains("battery", Qt::CaseInsensitive))
        {
            batteryList.push_back(fk);
            orderedDevices.insert(fk);
        }
    }

    return batteryList;
}

QStringList DeviceInfoParser::getDemidecodeBatteryList()
{
    QStringList batteryList;

    if(false == toolDatabaseSecondOrder_.contains("dmidecode"))
    {
        return batteryList;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["dmidecode"] )
    {
        if(fk.contains("Battery", Qt::CaseInsensitive))
        {
            batteryList.push_back(fk);
        }
    }

    return batteryList;
}

QStringList DeviceInfoParser::getUpowerBatteryList()
{
    QStringList batteryList;

    if(false == toolDatabaseSecondOrder_.contains("upower"))
    {
        return batteryList;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["upower"] )
    {
        if(fk.startsWith("battery", Qt::CaseInsensitive))
        {
            batteryList.push_back(fk);
        }
    }

    return batteryList;
}

QStringList DeviceInfoParser::getLshwOtherDeviceList()
{
    QStringList otherDeviceList;

    if(false == toolDatabaseSecondOrder_.contains("lshw"))
    {
        return otherDeviceList;
    }

    int lshwSize = toolDatabaseSecondOrder_["lshw"].size();
    for(int i = 0; i < lshwSize; ++i )
    {
        const QString& fk = toolDatabaseSecondOrder_["lshw"][i];
        if( i < lshwSize - 1 && toolDatabaseSecondOrder_["lshw"][i+1].startsWith(fk) == true )
        {
            continue;
        }

        if( orderedDevices.contains(fk) )
        {
            continue;
        }

        if(fk.contains("_firmware") || fk.contains("_memory") || fk.contains("_cache") || fk.contains("_cpu") \
           || fk.contains("_volume")|| fk.contains("_usb") /*|| fk.contains("_pci")*/  ||
              fk.contains("_pnp") || fk.contains("_volume") || fk.contains("_volume") || fk.contains("_volume")  )
        {
            continue;
        }

        if( toolDatabase_["lshw"][fk].contains("description") )
        {
            if( toolDatabase_["lshw"][fk]["description"].compare("Signal processing controller", Qt::CaseInsensitive) == 0 \
            || toolDatabase_["lshw"][fk]["description"].compare("Communication controller", Qt::CaseInsensitive) == 0   \
            || toolDatabase_["lshw"][fk]["description"].compare("Serial controller", Qt::CaseInsensitive) == 0 \
            || toolDatabase_["lshw"][fk]["description"].compare("PCI bridge", Qt::CaseInsensitive) == 0 \
            || toolDatabase_["lshw"][fk]["description"].compare("ISA bridge", Qt::CaseInsensitive) == 0 \
            || toolDatabase_["lshw"][fk]["description"].compare("SMBus", Qt::CaseInsensitive) == 0 )
            {
                continue;
            }
        }

        otherDeviceList.push_back(fk);
    }

    return otherDeviceList;
}

QStringList DeviceInfoParser::getLshwOtherPciDeviceList()
{
    QStringList otherPcideviceList;

    if(false == toolDatabaseSecondOrder_.contains("lshw"))
    {
        return otherPcideviceList;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["lshw"] )
    {
        QRegExp re("^[\\s\\S]*_pci_[\\S\\s]*pci:[\\d]?_[\\S\\s]*$");

        if( re.exactMatch(fk) )
        {
            if( orderedDevices.contains(fk) == false )
            {
                otherPcideviceList.push_back(fk);
                orderedDevices.insert(fk);
            }
        }
    }

    return otherPcideviceList;
}

QStringList DeviceInfoParser::getLshwPrinterList()
{
    QStringList printerList;

    if(false == toolDatabaseSecondOrder_.contains("lshw"))
    {
        return printerList;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["lshw"] )
    {
        if( toolDatabase_["lshw"][fk].contains("description") == false )
        {
            continue;
        }

        if( toolDatabase_["lshw"][fk]["description"].contains("Printer", Qt::CaseInsensitive) )
        {
            printerList.push_back(fk);
            orderedDevices.insert(fk);
            continue;
        }

        if( toolDatabase_["lshw"][fk].contains("driver") == false )
        {
            continue;
        }

        if( toolDatabase_["lshw"][fk]["driver"].contains("usblp", Qt::CaseInsensitive) )
        {
            printerList.push_back(fk);
            orderedDevices.insert(fk);
            continue;
        }
    }

    return printerList;
}

QStringList DeviceInfoParser::getCupsPrinterList()
{
    QStringList printerList;

    if(false == toolDatabase_.contains("Cups"))
    {
        return printerList;
    }

    foreach(const QString& fk, toolDatabase_["Cups"].keys() )
    {
        printerList.push_back(fk);
    }

    return printerList;
}

QStringList DeviceInfoParser::getLshwCDRomList()
{
    QStringList cdromList;

    if(false == toolDatabaseSecondOrder_.contains("lshw"))
    {
        return cdromList;
    }

    foreach(const QString& fk, toolDatabaseSecondOrder_["lshw"] )
    {
        if(fk.contains("cdrom"))
        {
            cdromList.push_back(fk);
            orderedDevices.insert(fk);
            continue;
        }
    }

    return cdromList;
}

bool DeviceInfoParser::loadOSInfo()
{
    struct utsname kernel_info;
    int ret = uname(&kernel_info);
    if (ret == 0) {
        char kversion[100] = { 0 };
        strncpy(kversion, kernel_info.release, strlen(kernel_info.release));
    }

    if( false == executeProcess("cat /proc/version") )
    {
        osInfo_ = tr("Unknown");
        return false;
    }

    QString str = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    executeProcess("tar zxvf /home/archermind/Desktop/deviceInfo.tar.gz");

    QFile osinfoFile(DEVICEINFO_PATH + "/osinfo.txt");
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

        osInfo_ = linuxCoreVerson + releaseVersion;
     }
     else
     {
        osInfo_ = str;
     }

    osInfo_.remove("version");
    lsbRelease_ = queryData("catOsrelease", "catOsrelease", "PRETTY_NAME");
    lsbRelease_.remove("\"");
    homeUrl_ = queryData("catOsrelease", "catOsrelease", "HOME_URL");
    homeUrl_.remove("\"");
    if(lsbRelease_.isEmpty() || lsbRelease_ ==  tr("Unknown"))
    {
        lsbRelease_ = queryData("lsb_release", "lsb_release", "Description");
    }

    return true;
}

QString DeviceInfoParser::getOsInfo()
{
    return osInfo_.replace("deepin","",Qt::CaseInsensitive);
}

QString DeviceInfoParser::getLsbRelease()
{
    return lsbRelease_;
}

QString DeviceInfoParser::getHomeUrl()
{
    return homeUrl_;
}

bool DeviceInfoParser::loadCatosrelelease()
{
    if( false == executeProcess("cat /etc/os-release"))
    {
        return false;
    }

    QString CatosreleleaseOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lsb_releaseFile(DEVICEINFO_PATH + "/os-release.txt");
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
    QFile lsb_releaseFile(DEVICEINFO_PATH + "/lsb_release.txt");
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
    QFile dmidecodeFile(DEVICEINFO_PATH + "/dmidecode.txt");
    if( false == dmidecodeFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    dmidecodeOut = dmidecodeFile.readAll();
    dmidecodeFile.close();
#endif

    // dimdecode
    DatabaseMap dimdecodeDatabase_;
    QStringList secondOrder;

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
            if(childDeviceType.isEmpty() == false)
            {
                DeviceInfoMap[childDeviceType] = childDeviceContent;
                childDeviceType.clear();
                childDeviceContent.clear();
            }

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
                    secondOrder.push_back(deviceType);
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
                secondOrder.push_back("dmidecode");
                DeviceInfoMap.clear();
                continue;
            }

            //  SMBIOS 3.0.0 present.
            rx.setPattern("^SMBIOS ([\\d]*.[\\d]*.[\\d])+ present.$");
            if( rx.indexIn(line) > -1 )
            {
                DeviceInfoMap["version"] = rx.cap(1);
                dimdecodeDatabase_["SMBIOS"] = DeviceInfoMap;
                secondOrder.push_back("SMBIOS");
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
                    secondOrder.push_back(deviceType);
                }
            }

            deviceType = line.trimmed();
            DeviceInfoMap.clear();
            continue;
        }
    }

    toolDatabase_["dmidecode"] = dimdecodeDatabase_;
    secondOrder.removeDuplicates();
    toolDatabaseSecondOrder_["dmidecode"] = secondOrder;
    return  true;
}

bool DeviceInfoParser::loadCatBoardinfoDatabase()
{
    if( false == executeProcess("cat /proc/boardinfo"))
    {
        return false;
    }

    QString catbaseboardOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile catboardinfoFile(DEVICEINFO_PATH + "/boardinfo.txt");
    if( false == catboardinfoFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    catbaseboardOut = catboardinfoFile.readAll();
    catboardinfoFile.close();
#endif

    // lscpu
    DatabaseMap catboardinfoDb;
    QMap<QString, QString> catboardinfoDatabase_;
    QString deviceType;
    int startIndex = 0;

    for( int i = 0; i < catbaseboardOut.size(); ++i )
    {
         if( catbaseboardOut[i] != '\n' && i != catbaseboardOut.size() -1 )
         {
             continue;
         }

         QString line = catbaseboardOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         if( line.trimmed().isEmpty() )
         {
             catboardinfoDb[deviceType] = catboardinfoDatabase_;
             catboardinfoDatabase_.clear();
             deviceType.clear();
             continue;
         }

         if( deviceType.isEmpty() )
         {
            deviceType = line.trimmed();
            continue;
         }

         int index = line.indexOf(Devicetype_Separator);
         if( index > 0 )
         {
             catboardinfoDatabase_[line.left(index).trimmed()] = line.mid(index+1).trimmed();
         }
    }

    toolDatabase_["catbaseboard"] = catboardinfoDb;
    return true;
}

bool DeviceInfoParser::loadLshwDatabase()
{
    if( false == executeProcess("sudo lshw"))
    {
        return false;
    }

    QString lshwOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lshwFile(DEVICEINFO_PATH + "/lshw.txt");
    if( false == lshwFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    lshwOut = lshwFile.readAll();
    lshwFile.close();
#endif
    // lshw
    DatabaseMap lshwDatabase_;
    QStringList secondOrder;

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
                continue;
            }

            lshwDatabase_[deviceTypeName] = DeviceInfoMap;
            secondOrder.push_back(deviceTypeName);

            DeviceInfoMap.clear();

            QString typeStr =  line;
            DeviceInfoMap["Type"] = typeStr.remove(Devicetype_lshw_Class_Prefix).trimmed();

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
                if(DeviceInfoMap.contains(strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)))
                {
                    DeviceInfoMap[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] += ", ";
                    DeviceInfoMap[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] += strList.last().trimmed();
                }
                else {
                    DeviceInfoMap[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] = strList.last().trimmed();
                }
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
                    if(DeviceInfoMap.contains(name))
                    {
                        DeviceInfoMap[name] += ", ";
                        DeviceInfoMap[name] += line.mid(index+1).trimmed();
                    }
                    else {
                        DeviceInfoMap[name] = line.mid(index+1).trimmed();
                    }

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
                                DeviceInfoMap[tempName] += ", ";
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
                                DeviceInfoMap[tempName] += ", ";
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
            {
                if(DeviceInfoMap.contains(name))
                {
                    DeviceInfoMap[name] += ", ";
                    DeviceInfoMap[name] += line.mid(index+1).trimmed();
                }
                else {
                    DeviceInfoMap[name] = line.mid(index+1).trimmed();
                }
            }
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
        secondOrder.push_back(deviceTypeName);
    }

    toolDatabase_["lshw"] = lshwDatabase_;
    secondOrder.removeDuplicates();
    toolDatabaseSecondOrder_["lshw"] = secondOrder;

    //QString logicalName = DeviceInfoParser::Instance().queryData("lshw", disk, "logical name");

    //DeviceInfoParser::Instance().loadSmartctlDatabase(logicalName);

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
    QFile lscpuFile(DEVICEINFO_PATH + "/lscpu.txt");
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
    toolDatabaseSecondOrder_["lscpu"] = QStringList("lscpu");
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
    QFile inputDeviceFile(DEVICEINFO_PATH + "/catcpu.txt");
    if( false == inputDeviceFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    catCpuOut = inputDeviceFile.readAll();
    inputDeviceFile.close();
#endif

    // cat /proc/bus/input/devices
    DatabaseMap catcpuDatabase_;
    QStringList secondOrder;

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
             secondOrder.push_back(cpuName);
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
        secondOrder.push_back(cpuName);
    }
    toolDatabase_["catcpu"] = catcpuDatabase_;
    secondOrder.removeDuplicates();
    toolDatabaseSecondOrder_["catcpu"] = secondOrder;
    return true;
}

bool DeviceInfoParser::loadAllSmartctlDatabase()
{
    QStringList diskList = DeviceInfoParser::Instance().getLshwDisknameList();

    foreach(const QString& disk, diskList)
    {
        QString logicalName = DeviceInfoParser::Instance().queryData("lshw", disk, "logical name");
        DeviceInfoParser::Instance().loadSmartctlDatabase(logicalName);
    }

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
    QFile smartctlFile(DEVICEINFO_PATH + "/smartctl.txt");
    if( false == smartctlFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    smartctlOut = smartctlFile.readAll();
    smartctlFile.close();
#endif

    // smartctl
    QMap<QString, QString> smartctlDatabase_;
    QString indexName;
    int startIndex = 0;

    QRegExp reg("^[\\s\\S]*[\\d]:[\\d][\\s\\S]*$");//time 08:00

    for( int i = 0; i < smartctlOut.size(); ++i )
    {
         if( smartctlOut[i] != '\n' && i != smartctlOut.size() -1 )
         {
             continue;
         }

         QString line = smartctlOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;


         int index = line.indexOf(Devicetype_Separator);
         if( index > 0 && reg.exactMatch(line) == false && false == line.contains("Error") && false == line.contains("hh:mm:SS") )
         {
            if(line.indexOf("(") < index && line.indexOf(")") > index)
            {
                continue;
            }

            if(line.indexOf("[") < index && line.indexOf("]") > index)
            {
                continue;
            }

            indexName = line.mid(0, index).trimmed().remove(" is");
            if(smartctlDatabase_.contains(indexName) )
            {
                smartctlDatabase_[indexName] += ", ";
                smartctlDatabase_[indexName] += line.mid(index+1).trimmed();
            }
            else
            {
                smartctlDatabase_[indexName] = line.mid(index+1).trimmed();
            }
            continue;
         }

         if( indexName.isEmpty() == false && ( line.startsWith("\t\t") || line.startsWith("    ") ) && line.contains(":") == false )
         {
            if(smartctlDatabase_.contains(indexName))
            {
                smartctlDatabase_[indexName] += ", ";
                smartctlDatabase_[indexName] += line.trimmed();
            }
            else
            {
                smartctlDatabase_[indexName] = line.trimmed();
            }

            continue;
         }

         indexName = "";

         QRegExp rx("^[ ]*[0-9]+[ ]+([\\w-_]+)[ ]+0x[0-9a-fA-F-]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+([0-9\\/w-]+[ ]*[ 0-9\\/w-()]*)$");
         if( rx.indexIn(line) > -1 )
         {
             smartctlDatabase_[rx.cap(1)] = rx.cap(2);
             continue;
         }

         QStringList strList;

        if( line.endsWith(")") )
        {
            int leftBracket = line.indexOf("(");
            if(leftBracket > 0)
            {
                QString str = line.left(leftBracket).trimmed();
                strList = str.trimmed().split(" ");
                if(strList.size() > 2)
                {
                    strList.last() += line.mid(leftBracket);
                }
            }
        }
        else if(strList.size() == 0 )
        {
             strList = line.trimmed().split(" ");
        }

         if(strList.size() < 5)
         {
             continue;
         }

         if(line.contains("Power_On_Hours"))
         {
             smartctlDatabase_["Power_On_Hours"] = strList.last();
             continue;
         }

         if(line.contains("Power_Cycle_Count"))
         {
             smartctlDatabase_["Power_Cycle_Count"] = strList.last();
             continue;
         }

         if(line.contains("Raw_Read_Error_Rate"))
         {
             smartctlDatabase_["Raw_Read_Error_Rate"] = strList.last();
             continue;
         }

         if(line.contains("Spin_Up_Time"))
         {
             smartctlDatabase_["Spin_Up_Time"] = strList.last();
             continue;
         }

         if(line.contains("Start_Stop_Count"))
         {
             smartctlDatabase_["Start_Stop_Count"] = strList.last();
             continue;
         }

         if(line.contains("Reallocated_Sector_Ct"))
         {
             smartctlDatabase_["Reallocated_Sector_Ct"] = strList.last();
             continue;
         }

         if(line.contains("Seek_Error_Rate"))
         {
             smartctlDatabase_["Seek_Error_Rate"] = strList.last();
             continue;
         }

         if(line.contains("Spin_Retry_Count"))
         {
             smartctlDatabase_["Spin_Retry_Count"] = strList.last();
             continue;
         }
         if(line.contains("Calibration_Retry_Count"))
         {
             smartctlDatabase_["Calibration_Retry_Count"] = strList.last();
             continue;
         }
         if(line.contains("G-Sense_Error_Rate"))
         {
             smartctlDatabase_["G-Sense_Error_Rate"] = strList.last();
             continue;
         }
         if(line.contains("Power-Off_Retract_Count"))
         {
             smartctlDatabase_["Power-Off_Retract_Count"] = strList.last();
             continue;
         }
         if(line.contains("Load_Cycle_Count"))
         {
             smartctlDatabase_["Load_Cycle_Count"] = strList.last();
             continue;
         }
         if(line.contains("Temperature_Celsius"))
         {
             smartctlDatabase_["Temperature_Celsius"] = strList.last();
             continue;
         }
         if(line.contains("Reallocated_Event_Count"))
         {
             smartctlDatabase_["Reallocated_Event_Count"] = strList.last();
             continue;
         }
         if(line.contains("Current_Pending_Sector"))
         {
             smartctlDatabase_["Current_Pending_Sector"] = strList.last();
             continue;
         }
         if(line.contains("Offline_Uncorrectable"))
         {
             smartctlDatabase_["Offline_Uncorrectable"] = strList.last();
             continue;
         }
         if(line.contains("UDMA_CRC_Error_Count"))
         {
             smartctlDatabase_["UDMA_CRC_Error_Count"] = strList.last();
             continue;
         }
         if(line.contains("Multi_Zone_Error_Rate"))
         {
             smartctlDatabase_["Multi_Zone_Error_Rate"] = strList.last();
             continue;
         }
    }

    if(toolDatabase_.contains("smartctl") == false)
    {
        DatabaseMap rootsmartctlDb;
        rootsmartctlDb[diskLogical] = smartctlDatabase_;
        toolDatabase_["smartctl"] = rootsmartctlDb;
    }
    else
    {
        toolDatabase_["smartctl"][diskLogical] = smartctlDatabase_;
    }


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
    QFile inputDeviceFile(DEVICEINFO_PATH + "/input.txt");
    if( false == inputDeviceFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    inputDeviceOut = inputDeviceFile.readAll();
    inputDeviceFile.close();
#endif

    // cat /proc/bus/input/devices
    DatabaseMap catInputDeviceDatabase_;
    QStringList secondOrder;

    QMap<QString, QString> DeviceInfoMap;
    int startIndex = 0;

    for( int i = 0; i < inputDeviceOut.size(); ++i )
    {
         if( inputDeviceOut[i] != '\n' && i != inputDeviceOut.size() -1 )
         {
             continue;
         }

         QString line = inputDeviceOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         if( i == inputDeviceOut.size() -1 || line.trimmed().isEmpty() )
         {
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
                secondOrder.push_back(deviceSysfs);
             }

             DeviceInfoMap.clear();
         }


        QString cutLine = line.mid(line.indexOf(Devicetype_Separator)+1).trimmed();
        int first_index = cutLine.indexOf(DeviceType_CatDevice_Separator);
        if( first_index < 0 )
        {
            continue;
        }

        if(cutLine.startsWith("Name="))
        {
            DeviceInfoMap[cutLine.left(first_index).trimmed()] = cutLine.mid(first_index+1).remove("\"").trimmed();
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


    toolDatabase_["catinput"] = catInputDeviceDatabase_;
    secondOrder.removeDuplicates();
    toolDatabaseSecondOrder_["catinput"] = secondOrder;
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
    QFile xrandrFile(DEVICEINFO_PATH + "/xrandr.txt");
    if( false == xrandrFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    xrandrOut = xrandrFile.readAll();
    xrandrFile.close();
#endif

    // xrandr --verbose
    DatabaseMap xrandrDatabase_;
    QStringList secondOrder;
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
                secondOrder.push_back(title);
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
            secondOrder.push_back(strList.first().trimmed());
            continue;
        }
        static QString EDID = "";
        static bool startLogEDID = false;
        static int EDID_NO = 0;
        if (line.contains("EDID:",Qt::CaseInsensitive)) {
            startLogEDID = true;
            EDID = "";
        }
        if(startLogEDID == true && !line.contains("EDID:",Qt::CaseInsensitive)) {
            if (line.startsWith(Devicetype_Xrandr_Tab) || line.startsWith(Devicetype_Xrandr_Twospace)) {
                if (line.trimmed().count() == QString("00ffffffffffff0030aed86100000000").count()){
                    EDID += line.trimmed()+"\n";
                }
                else {
                    startLogEDID = false;
                    QString edidStr = QString("EDID__%1").arg(EDID_NO);
                    EDID_NO++;
                    DeviceInfoMap[edidStr] = EDID;
                }
            }
        }
        if( line.startsWith(Devicetype_Xrandr_Tab) || line.startsWith(Devicetype_Xrandr_Twospace) )
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

        if( line.startsWith(Devicetype_Xrandr_Twotab) || line.startsWith(Devicetype_Xrandr_TabAndspace))
        {
            content += line;
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
                secondOrder.push_back(title);
            }

            DeviceInfoMap.clear();

            title = line.trimmed();
            QString resolutionStr;
            QRegExp reg("[ ]([0-9]\\d*x[0-9]\\d*)");
            int pos = reg.indexIn(line);
            if( pos > 0 )
            {
                DeviceInfoMap["Resolution"] = line.mid(pos+1, reg.matchedLength()-1).trimmed();
            }

            reg.setPattern("[ ]([0-9]\\d*mm x [0-9]\\d*mm)");
            pos = reg.indexIn(line);
            if( pos > 0 )
            {
                DeviceInfoMap["Size"] = line.mid(pos+1, reg.matchedLength()-1).trimmed();
            }

            continue;
        }
    }

    toolDatabase_["xrandr"] = xrandrDatabase_;
    secondOrder.removeDuplicates();
    toolDatabaseSecondOrder_["xrandr"] = secondOrder;
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
    QFile hciconfigFile(DEVICEINFO_PATH + "/upower.txt");
    if( false == hciconfigFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    upowerOut = hciconfigFile.readAll();
    hciconfigFile.close();
#endif

    // hciconfig
    DatabaseMap upowerDatabase_;
    QStringList secondOrder;

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
                secondOrder.push_back(deviceName);
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
    secondOrder.removeDuplicates();
    toolDatabaseSecondOrder_["upower"] = secondOrder;

    return true;
}

//  http://pci-ids.ucw.cz/read/PD/
//  /usr/share/misc/pci.ids
bool DeviceInfoParser::loadLspciDatabase()
{
    if( false == executeProcess("sudo lspci -v"))
    {
        return false;
    }
    QString lspciOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lspciFile(DEVICEINFO_PATH + "/lspci.txt");
    if( false == lspciFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    lspciOut = lspciFile.readAll();
    lspciFile.close();
#endif
    // lspci --verbose
    DatabaseMap lspciDatabase_;
    QStringList secondOrder;

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

        if(line.trimmed().isEmpty() || i == lspciOut.size() -1)
        {
            lspciDatabase_[deviceName] = DeviceInfoMap;
            secondOrder.push_back(deviceName);

            DeviceInfoMap.clear();
            deviceName = "";
            continue;
        }

        if(line.contains(Devicetype_Lspci_Tab))
        {
            if(line.contains(Devicetype_Lspci_Memory) )
            {
                if( line.contains(Devicetype_Lspci_non_prefetchable) )
                {
                    QRegExp rx("^[\\s\\S]*\\[size=([\\d]*M)\\]$");
                    if( rx.exactMatch(line) )
                    {
                        //DeviceInfoMap[Devicetype_Lspci_Memory] = QString(rx.cap(1)).trimmed();
                    }
                    continue;
                }

                if( line.contains(Devicetype_Lspci_prefetchable) )
                {
                    QRegExp rx("^[\\s\\S]*\\[size=([\\d]*M)\\]$");
                    if( rx.exactMatch(line) && DeviceInfoMap.contains(Devicetype_Lspci_Memory) == false )
                    {
                        DeviceInfoMap[Devicetype_Lspci_Memory] = QString(rx.cap(1)).trimmed();
                    }
                    continue;
                }
            }

            int index = line.indexOf(Devicetype_Lspci_Seperator);
            if( index > 0 )
            {
                DeviceInfoMap[line.mid(0,index).trimmed()] = line.mid(index+1).trimmed();
            }
            else
            {
                DeviceInfoMap[line.trimmed()] = "";
            }
            continue;
        }

        int index = line.indexOf(Devicetype_Lspci_Seperator);
        if( index > 0 )
        {
            if(deviceName.isEmpty() == false)
            {
                lspciDatabase_[deviceName] = DeviceInfoMap;
                secondOrder.push_back(deviceName);

                DeviceInfoMap.clear();
                deviceName = "";
            }
            DeviceInfoMap[Devicetype_Name] = line.mid(index+1).trimmed();
            deviceName = line.mid(0,index);
            continue;
        }
    }

    toolDatabase_["lspci"] = lspciDatabase_;
    secondOrder.removeDuplicates();
    toolDatabaseSecondOrder_["lspci"] = secondOrder;
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
    QFile hciconfigFile(DEVICEINFO_PATH + "/hciconfig.txt");
    if( false == hciconfigFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    hciconfigOut = hciconfigFile.readAll();
    hciconfigFile.close();
#endif

    // hciconfig
    DatabaseMap hciconfigDatabase_;
    QStringList secondOrder;

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
                secondOrder.push_back(deviceName);
            }
            DeviceInfoMap.clear();
            deviceName = line.mid(0, line.indexOf(Devicetype_Separator));
            continue;
        }

        if( line.startsWith(Devicetype_Hciconfig_Multispace) || line.startsWith(Devicetype_Hciconfig_Tab) )
        {
            int index = line.indexOf(Devicetype_Separator);

            QString key = (index > 0)?line.mid(0, index).trimmed():line.trimmed();

            if( line.contains("BD Address") )
            {
                QString bdLine = line.trimmed();
                int sepIndex = index;

                //BD Address: 00:19:86:00:25:4E  ACL MTU: 1021:8  SCO MTU: 64:1
                while(sepIndex > 1 )
                {
                    int spaceIndex = bdLine.indexOf(" ", sepIndex + 2);

                    if( spaceIndex > sepIndex + 1 )
                    {
                        DeviceInfoMap[ bdLine.left(sepIndex-1).trimmed()] = bdLine.mid(sepIndex+1, spaceIndex - sepIndex - 1).trimmed();
                        bdLine = bdLine.mid(spaceIndex).trimmed();
                        sepIndex = bdLine.indexOf(Devicetype_Separator);
                    }
                    else
                    {
                        DeviceInfoMap[bdLine.left(sepIndex-1)] = line.mid(sepIndex+1).trimmed();

                        break;
                    }
                }

                continue;
            }

            if( index > 0 )
            {
                DeviceInfoMap[key] = line.mid(index+1).trimmed();
            }
            else
            {
                DeviceInfoMap[key] = "";
            }
        }

        if( i == hciconfigOut.size() -1 || line.trimmed().isEmpty() )
        {
            if(deviceName.isEmpty() == false)
            {
                hciconfigDatabase_[deviceName] = DeviceInfoMap;
                secondOrder.push_back(deviceName);
            }
            DeviceInfoMap.clear();
            deviceName = "";
            continue;
        }
    }

    toolDatabase_["hciconfig"] = hciconfigDatabase_;
    secondOrder.removeDuplicates();
    toolDatabaseSecondOrder_["hciconfig"] = secondOrder;
    return true;
}

bool DeviceInfoParser::loadAllBluetoothctlControllerDatabase()
{
    QStringList bList = DeviceInfoParser::Instance().getHciconfigBluetoothControllerList();

    foreach(const QString& b, bList)
    {
        QString controller = DeviceInfoParser::Instance().queryData("hciconfig", b, "BD Address");
        DeviceInfoParser::Instance().loadBluetoothctlDatabase(controller);
    }

    return true;
}

bool DeviceInfoParser::loadBluetoothctlDatabase(const QString& controller)
{
    if( false == executeProcess("bluetoothctl show " + controller))
    {
        return false;
    }

    QString bluetoothctlOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile bluetoothctlFile(DEVICEINFO_PATH + "/bluetoothctl.txt");
    if( false == bluetoothctlFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    bluetoothctlOut = bluetoothctlFile.readAll();
    bluetoothctlFile.close();
#endif

    QStringList secondOrder;
    secondOrder.push_back(controller);

    QMap<QString, QString> DeviceInfoMap;
    int startIndex = 0;

    for( int i = 0; i < bluetoothctlOut.size(); ++i )
    {
        if( bluetoothctlOut[i] != '\n' && i != bluetoothctlOut.size() -1)
        {
            continue;
        }

        QString line = bluetoothctlOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if(line.startsWith("Controller "))
        {
            continue;
        }

        if( line.startsWith(Devicetype_Hciconfig_Multispace) || line.startsWith(Devicetype_Hciconfig_Tab) )
        {
            int index = line.indexOf(Devicetype_Separator);
            if( index > 0 )
            {
                QString nameStr = line.mid(0, index).trimmed();
                if( DeviceInfoMap.contains(nameStr) )
                {
                    DeviceInfoMap[nameStr] += ", ";
                    DeviceInfoMap[nameStr] += line.mid(index+1).trimmed();
                }
                else
                {
                    DeviceInfoMap[nameStr] = line.mid(index+1).trimmed();
                }
            }
            else
            {
                DeviceInfoMap[line.trimmed()] = "";
            }
        }

        if( i == bluetoothctlOut.size() -1 || line.trimmed().isEmpty() )
        {
            continue;
        }
    }


    if(false == toolDatabase_.contains("bluetoothctl"))
    {
        DatabaseMap bluetoothctlDatabase_;
        bluetoothctlDatabase_[controller] = DeviceInfoMap;
        toolDatabase_["bluetoothctl"] = bluetoothctlDatabase_;
        toolDatabaseSecondOrder_["bluetoothctl"] = secondOrder;
    }
    else
    {
        toolDatabase_["bluetoothctl"][controller] = DeviceInfoMap;
        toolDatabaseSecondOrder_["bluetoothctl"] += secondOrder;
    }

    secondOrder.removeDuplicates();

    return true;
}

QStringList DeviceInfoParser::getAllBluetoothctlPairedDevices()
{
    QStringList pairedDevices;
    if( false == executeProcess("bluetoothctl paired-devices") )
    {
        return pairedDevices;
    }

    QString pairedDevicesOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile bluetoothctlFile(DEVICEINFO_PATH + "/paired-devices.txt");
    if( false == bluetoothctlFile.open(QIODevice::ReadOnly) )
    {
        return pairedDevices;
    }

    pairedDevicesOut = bluetoothctlFile.readAll();
    bluetoothctlFile.close();
#endif

    int startIndex = 0;

    for( int i = 0; i < pairedDevicesOut.size(); ++i )
    {
        if( pairedDevicesOut[i] != '\n' && i != pairedDevicesOut.size() -1)
        {
            continue;
        }

        QString line = pairedDevicesOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if(line.startsWith("Device ") == false)
        {
            continue;
        }

        QStringList lst = line.trimmed().split(" ");
        if(lst.size() > 1)
        {
            pairedDevices.push_back(lst[1]);
        }
    }

    //bluetoothctl info D3:6A:FF:19:CE:06
    return pairedDevices;
}

bool DeviceInfoParser::loadAllBluethctlPairedDeviceDatabase()
{
    QStringList bList = DeviceInfoParser::Instance().getHciconfigBluetoothControllerList();
    if(bList.size() < 1)
    {
        return true;
    }

    QStringList pairedDevices = DeviceInfoParser::Instance().getAllBluetoothctlPairedDevices();

    foreach(const QString& mac, pairedDevices)
    {
        DeviceInfoParser::Instance().loadBluetoothctlPairedDeviceDatabase(mac);
    }

    return true;
}

bool DeviceInfoParser::loadBluetoothctlPairedDeviceDatabase(const QString& mac)
{
    if( false == executeProcess("bluetoothctl info " + mac))
    {
        return false;
    }

    QString bluetoothctlOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile bluetoothctlFile(DEVICEINFO_PATH + "/" + mac.left(2) + ".txt");
    if( false == bluetoothctlFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    bluetoothctlOut = bluetoothctlFile.readAll();
    bluetoothctlFile.close();
#endif

    QStringList secondOrder;
    secondOrder.push_back(mac);

    QMap<QString, QString> DeviceInfoMap;
    int startIndex = 0;

    for( int i = 0; i < bluetoothctlOut.size(); ++i )
    {
        if( bluetoothctlOut[i] != '\n' && i != bluetoothctlOut.size() -1)
        {
            continue;
        }

        QString line = bluetoothctlOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if(line.startsWith("Device "))
        {
            continue;
        }

        if( line.startsWith(Devicetype_Hciconfig_Multispace) || line.startsWith(Devicetype_Hciconfig_Tab) )
        {
            int index = line.indexOf(Devicetype_Separator);
            if( index > 0 )
            {
                QString nameStr = line.mid(0, index).trimmed();
                if( DeviceInfoMap.contains(nameStr) )
                {
                    DeviceInfoMap[nameStr] += ", ";
                    DeviceInfoMap[nameStr] += line.mid(index+1).trimmed();
                }
                else
                {
                    DeviceInfoMap[nameStr] = line.mid(index+1).trimmed();
                }
            }
        }

        if( i == bluetoothctlOut.size() -1 || line.trimmed().isEmpty() )
        {
            continue;
        }
    }


    if(false == toolDatabase_.contains("paired-devices"))
    {
        DatabaseMap bluetoothctlDatabase_;
        bluetoothctlDatabase_[mac] = DeviceInfoMap;
        toolDatabase_["paired-devices"] = bluetoothctlDatabase_;
        toolDatabaseSecondOrder_["paired-devices"] = secondOrder;
    }
    else
    {
        toolDatabase_["paired-devices"][mac] = DeviceInfoMap;
        toolDatabaseSecondOrder_["paired-devices"] += secondOrder;

    }

    secondOrder.removeDuplicates();

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
    QFile hwinfoFile(DEVICEINFO_PATH + "/hwinfo.txt");
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
    QStringList secondOrder;

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
                secondOrder.push_back(deviceName);
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
                    if(DeviceInfoMap.contains(Devicetype_HwInfo_Currentresolution))
                    {
                        //DeviceInfoMap[Devicetype_HwInfo_Currentresolution] += ", ";
                        //DeviceInfoMap[Devicetype_HwInfo_Currentresolution] +=line.mid(index+1).trimmed();
                    }
                    else
                    {
                        DeviceInfoMap[Devicetype_HwInfo_Currentresolution] = line.mid(index+1).trimmed();
                    }

                    continue;
                }
                if(false == DeviceInfoMap.contains( line.mid(0, index).trimmed() ))
                {
                    DeviceInfoMap[ line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
                }

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
                        DeviceInfoMap[Devicetype_HwInfo_ResolutionList] += ", ";
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
    secondOrder.removeDuplicates();
    toolDatabaseSecondOrder_["hwinfo"] = secondOrder;
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
    QFile lpstatFile(DEVICEINFO_PATH + "/lpstat.txt");
    if( false == lpstatFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    lpstatOut = lpstatFile.readAll();
    lpstatFile.close();
#endif

    DatabaseMap lpstatDatabase_;
    QStringList secondOrder;

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
                secondOrder.push_back(deviceName);
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
    secondOrder.removeDuplicates();
    toolDatabaseSecondOrder_["lpstat"] = secondOrder;
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

int getDestInfo(void *user_data, unsigned flags, cups_dest_t *dest)
{
    if (flags & CUPS_DEST_FLAGS_REMOVED)
    {
        return -1;
    }

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

    cupsEnumDests(CUPS_DEST_FLAGS_NONE, 1000, nullptr, CUPS_PRINTER_CLASS, 0, getDestInfo, &cupsDatabase);

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
    if( false == executeProcess("sudo whoami"))
    {
        return false;
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

    QString newCmd = "pkexec deepin-devicemanager-authenticateProxy \"";
    newCmd += cmd;
    newCmd += "\"";
    newCmd.remove("sudo");
    return runCmd(newCmd);
}

bool DeviceInfoParser::runCmd(const QString& cmd)
{
    QProcess process_;
    int msecs = 10000;
    if(cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy") )
    {
        msecs = -1;
    }

    process_.start(cmd);

    bool res = process_.waitForFinished(msecs);
    standOutput_ = process_.readAllStandardOutput();
    int exitCode = process_.exitCode();
    if( cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy") && (exitCode == 127 || exitCode == 126) )
    {
        dError( "Run \'" + cmd + "\' failed: Password Error! " + QString::number(exitCode) + "\n");

        if(cmd.contains("whoami"))
        {
            //if(exitCode == 126)
            //{
                //DMessageBox::critical(nullptr, "", tr("Password Error!" ));
            //}

            //exit(-1);
        }

        return false;
    }

    if(res == false)
    {
        dError( "Run \'" + cmd + "\' failed\n" );
    }

    return res;
}

bool DeviceInfoParser::runCmd(const QStringList& cmdList)
{
    QProcess process_;
    process_.start("/bin/bash",cmdList);
    bool res = process_.waitForFinished(10000);
    standOutput_ = process_.readAllStandardOutput();
    return res;
}
