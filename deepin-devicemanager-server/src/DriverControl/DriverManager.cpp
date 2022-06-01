/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     kongyunzhen <kongyunzhen@uniontech.com>
*
* Maintainer: kongyunzhen <kongyunzhen@uniontech.com>
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
#include "DriverManager.h"
#include "Utils.h"
#include "ModCore.h"
#include "DebInstaller.h"
#include "DriverInstaller.h"
#include "DeviceInfoManager.h"
#include "HttpDriverInterface.h"

#include <QThread>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QPair>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QEventLoop>
#include <QTimer>
#include <QScopedPointer>
#include <QProcess>
#include <QJsonArray>
#include <QThread>
#include <QDBusInterface>
#include <QNetworkConfigurationManager>

#include <fstream>
#include <string>
#include <vector>


#define SD_KEY_excat    "excat"
#define SD_KEY_ver      "version"
#define SD_KEY_code     "client_code"
#define SD_KEY_mfg      "MFG"
#define SD_KEY_mdl      "MDL"
#define SD_KEY_ieeeid   "ieee1284_id"

#define SD_KEY_describe "describe"
#define SD_KEY_driver   "driver"
#define SD_KEY_excat    "excat"
#define SD_KEY_ppd      "ppd"
#define SD_KEY_sid      "sid"

#define E_FILE_NOT_EXISTED 100 // file_not_existed 文件不存在，主要比如点击下一步时拔掉优盘
#define E_NOT_DRIVER      101 // not driver 非驱动文件
#define E_NOT_SIGNED      102 // not signed 没有数字签名

#define RETURN_VALUE(flag) \
    {   \
        sigFinished(flag, errmsg);\
        return flag; \
    }

DriverManager::DriverManager(QObject *parent)
    : QObject(parent)
    , mp_modcore(new ModCore(this))
{
    mp_deboperatethread = new QThread(this);
    mp_debinstaller = new DebInstaller;
    mp_debinstaller->moveToThread(mp_deboperatethread);
    mp_deboperatethread->start();

    mp_driverOperateThread = new QThread(this);
    mp_driverInstaller = new DriverInstaller;
    mp_driverInstaller->moveToThread(mp_driverOperateThread);
    mp_driverOperateThread->start();

    initConnections();
}

DriverManager::~DriverManager()
{
    mp_deboperatethread->quit();
    mp_deboperatethread->wait();
}


bool DriverManager::checkDriverInfo()
{
    //1.遍历硬件查询结果。
    //2.过滤，只检测显示适配器（显卡），音频适配器（声卡），网络适配器（网卡/无线网卡），蓝牙适配器，外设输入输出设备（打印机，图像设备（扫描仪，摄像头））
    //2.1 查找前端的处理，找到硬件的驱动部分。
    //3.查询。

    if (checkPrinterInfo() || checkBoardCardInfo()) return true;

    return false;
}

void DriverManager::initConnections()
{
    connect(mp_debinstaller, &DebInstaller::installFinished, [&](bool bsuccess) {
        if (bsuccess) {
            sigProgressDetail(90, tr("Install success"));
        } else {
            this->sigProgressDetail(m_installprocess, errmsg);
        }
        sigFinished(bsuccess, errmsg);
    });
    connect(mp_debinstaller, &DebInstaller::progressChanged, [&](int iprocess) {
        m_installprocess = 20 + static_cast<int>(iprocess * 0.8);
        sigProgressDetail(m_installprocess, errmsg);
    });
    connect(mp_debinstaller, &DebInstaller::errorOccurred, [&](QString errmsg) {
        qInfo() << "signal_installFailedReason:" << errmsg;
        sigFinished(false, errmsg);
        this->errmsg = errmsg;
    });
    connect(this, &DriverManager::sigDebInstall, mp_debinstaller, &DebInstaller::installPackage);
    connect(this, &DriverManager::sigDebUnstall, mp_debinstaller, &DebInstaller::uninstallPackage);

    connect(mp_driverInstaller, &DriverInstaller::installProgressFinished, [&](bool bsuccess) {
        if (bsuccess) {
            sigInstallProgressFinished(bsuccess, EC_NULL);
            qInfo() << "Driver installed successfully";
        } else {
            qInfo() << "Driver installation failed , reason : unknow";
        }
    });

    connect(mp_driverInstaller, &DriverInstaller::errorOccurred, [this](int err) {
        if (EC_NETWORK != err){
            qInfo() << "Driver installation failed , reason : " << err;
            sigInstallProgressFinished(false, err);
            return;
        }

        // 如果错误是网络异常，则需要尝试60s
        // 先通知前台网络异常
        sigInstallProgressFinished(false, EC_NOTIFY_NETWORK);
        qInfo() << "Network error : We are listening to the network";

        // 60s等待操作
        m_IsNetworkOnline = false;
        qint64 seconds = QDateTime::currentSecsSinceEpoch();
        while (QDateTime::currentSecsSinceEpoch() - seconds < 30){

            if(m_StopQueryNetwork){
                sigInstallProgressFinished(false, EC_CANCEL);
                m_StopQueryNetwork = false;
                return ;
            }

            if(isNetworkOnline()){
                sigInstallProgressFinished(false, EC_REINSTALL);
                return ;
            }else{
                sleep(1);
            }
        }
        m_IsNetworkOnline = true;
        sigInstallProgressFinished(false, EC_NETWORK);
    });

    connect(mp_driverInstaller, &DriverInstaller::downloadProgressChanged, [&](QStringList msg) {
        sigDownloadProgressChanged(msg);
        qInfo() << "Downloading driver , " << "progress rate : " << msg[0] << " downloaded size : " << msg[1] << " download speed : " << msg[2];
    });

    connect(mp_driverInstaller, &DriverInstaller::downloadFinished, [&]() {
        sigDownloadFinished();
    });

    connect(mp_driverInstaller, &DriverInstaller::installProgressChanged, [&](int progress) {
        sigInstallProgressChanged(progress);
        qInfo() << "Installing driver ,  installation progress : " << progress;
    });
}

/**
 * @brief DriverManager::unInstallDriver 卸载驱动
 * @param modulename 驱动模块名
 * @return  true:卸载成功 false:卸载失败
 */
bool DriverManager::unInstallDriver(const QString &modulename)
{
    bool bsuccess = false;
    sigProgressDetail(5, "");
    QString modulePath = mp_modcore->modGetPath(modulename);
    //失败返回空
    QString modulePackageName = Utils::file2PackageName(modulePath);
    QString kernelRelease = Utils::kernelRelease();
    sigProgressDetail(10, "");
    //当模块包为内核模块包时不允许删除，采用卸载加黑名单禁用方式
    if (mp_modcore->modIsBuildIn(mp_modcore->modGetName(modulename)) ||
            modulePackageName.isEmpty() ||
            modulePackageName.contains(kernelRelease)) {
        sigProgressDetail(20, "");
        bsuccess = unInstallModule(modulename, errmsg);
        if (bsuccess) {
            sigProgressDetail(90, "");
        }
        sigFinished(bsuccess, errmsg);

    } else {
        sigProgressDetail(15, "");
        mp_modcore->rmModForce(modulename, errmsg);
        sigProgressDetail(20, "");
        emit sigDebUnstall(modulePackageName);
    }

    return  bsuccess;
}

bool DriverManager::installDriver(const QString &filepath)
{
    /*1.检查模块是否已存在 存在报错返回，不报错返回
     *2.拷贝、安装（重复目前不处理，提供接口由前端判断，也不指定KMOD_INSERT_FORCE_MODVERSION）
     *3.检测是否在黑名单，如果在黑名单则需要先去除黑名单设置
     *4.设置开机自起
     */
    sigProgressDetail(1, "start");
    if (!QFile::exists(filepath)) {
        sigProgressDetail(5, "file not exist");
        errmsg = QString("%1").arg(E_FILE_NOT_EXISTED);
        sigFinished(false, errmsg);
        return  false;
    }

    // 判断是否是驱动包
    sigProgressDetail(5, "");
    if (!isDriverPackage(filepath)) {
        errmsg = QString("%1").arg(E_NOT_DRIVER);
        sigFinished(false, errmsg);
        return  false;
    }

    if (!isSigned(filepath)) {
        errmsg = QString("%1").arg(E_NOT_SIGNED);
        sigFinished(false, errmsg);
        return  false;
    }

    //模块已被加载
    /*
     * 下面代码由 liujun 于 2021年11月9号 注释
     * 为修复bug https://pms.uniontech.com/zentao/bug-view-101808.html
    */
//    if (mp_modcore->modIsLoaded(filepath)) {
//        sigProgressDetail(10, QString("could not insert module %1 :file exist").arg(filepath));
//        return  false;
//    }
    sigProgressDetail(10, "");
    QFileInfo fileinfo(filepath);
    QMimeDatabase typedb;
    QMimeType filetype = typedb.mimeTypeForFile(fileinfo);
    if (filetype.filterString().contains("deb")) {
        qInfo() << __func__ << "deb install start";
        sigProgressDetail(15, "");
        emit sigDebInstall(filepath);
        sigProgressDetail(20, "");
    } else {
        //已判断文件是否存在所以必然存在文件名
        QString filename = fileinfo.fileName();
        QString installdir = QString("/lib/modules/%1/custom/%2").arg(Utils::kernelRelease()).arg(mp_modcore->modGetName(filepath));
        QDir installDir(installdir);
        //判断安装路径是否已存在，如果不存在先创建安装目录
        if (installDir.exists() ||
                installDir.mkpath(installdir)) {
            sigProgressDetail(30, "");
            //将文件拷贝到安装目录
            QString installpath = installDir.absoluteFilePath(filename);
            /*
             * 下面代码由 liujun 于 2021年11月10号 修改
             * 驱动可以重复安装 因此路径如果存在则删除
             */
            QFile::remove(installpath);
            if (QFile::copy(filepath, installpath)) {
                sigProgressDetail(40, "");
                //更新依赖
                Utils::updateModDeps();
                sigProgressDetail(50, "");
                QString modname = mp_modcore->modGetName(installpath);
                //处理黑名单
                mp_modcore->rmFromBlackList(modname);
                bool success = mp_modcore->modInstall(modname, errmsg);
                sigProgressDetail(60, "");
                if (success) {
                    sigProgressDetail(70, "");
                    //如果非内建模块设置开机自启动
                    if (!mp_modcore->modIsBuildIn(modname) && !mp_modcore->setModLoadedOnBoot(modname)) {
                        sigFinished(false, errmsg);
                        return  false;
                    }
                    sigProgressDetail(90, "");
                    sigFinished(true, "");
                } else {
                    //失败将文件移除,只删文件不删路径
                    QFile::remove(installpath);

                    //删除后执行depmod更新
                    QProcess process;
                    process.start("depmod -a");
                    process.waitForFinished(-1);

                    sigFinished(false, errmsg);
                    return  false;
                }
            } else {
                sigFinished(false, errmsg);
                return  false;
            }
        }
    }
    return  true;
}

void DriverManager::installDriver(const QString &pkgName, const QString &version)
{
    if (!mp_driverOperateThread->isRunning())
        mp_driverOperateThread->start();
    mp_driverInstaller->installPackage(pkgName, version);
}

void DriverManager::undoInstallDriver()
{
    if(m_IsNetworkOnline){
        mp_driverInstaller->undoInstallDriver();
    }else{
        m_StopQueryNetwork = true;
    }
}

/**
 * @brief DriverManager::checkModuleInUsed 获取依赖当前模块在使用的模块
 * @param modName 模块名 sample: hid or hid.ko /xx/xx/hid.ko
 * @return 返回当前依赖模块列表 like ['usbhid', 'hid_generic']
 */
QStringList DriverManager::checkModuleInUsed(const QString &modName)
{
    if (nullptr == mp_modcore)
        return QStringList();
    return  mp_modcore->checkModuleInUsed(modName);
}

/**
 * @brief DriverManager::isBlackListed 模块是否在黑名单中
 * @param modName 模块名 sample:hid or hid.ko /xx/xx/hid.ko
 * @return true 是 false 否
 */
bool DriverManager::isBlackListed(const QString &modName)
{
    return mp_modcore->modIsBuildIn(modName);
}

/**
 * @brief DriverManager::isModFile 判断文件是否未驱动文件
 * @param filePath 文件路径
 * @return true:是 false 否
 */
bool DriverManager::isDriverPackage(const QString &filepath)
{
    bool bdriver = false;
    QMimeDatabase typedb;
    QMimeType filetype = typedb.mimeTypeForFile(filepath);
    if (filetype.filterString().contains("deb")) {
        bdriver = Utils::isDriverPackage(filepath);
    } else {
        bdriver = mp_modcore->isModFile(filepath);
    }

    return  bdriver;
}
/**
 * @brief DriverManager::isModFile 判断文件是否有数字签名
 * @param filePath 文件路径
 * @return true:是 false 否
 */
bool DriverManager::isSigned(const QString &filepath)
{
    //如果是开发者模式，直接返回true
    QDBusInterface *dbusInterFace = new QDBusInterface("com.deepin.sync.Helper", "/com/deepin/sync/Helper",
                                                       "com.deepin.sync.Helper", QDBusConnection::systemBus());
    bool isDeveloperMode = dbusInterFace->property("DeveloperMode").toBool();                            // 判断当前是否处于开发者模式

    if (isDeveloperMode) {
        return true;
    }

    QProcess process;
    QStringList options;
    QString strSignTool;
    QString strSignCheckString;

    if (filepath.contains("deb")) {
        strSignTool = "deepin-deb-verify ";
        strSignCheckString = "signature verified";
    } else {
        strSignTool = "deepin-elf-sign -f ";
        strSignCheckString = "Verified successfully";
    }
    options << "-c" << strSignTool + filepath;

    process.start("/bin/bash", options);
    process.waitForFinished(-1);

    QString str = process.readAll();
    return str.contains(strSignCheckString);
}
bool DriverManager::isArchMatched(const QString &path)
{
    QMimeDatabase typedb;
    QMimeType filetype = typedb.mimeTypeForFile(path);
    if (filetype.filterString().contains("deb"))
        return mp_debinstaller->isArchMatched(path);
    return true;
}

bool DriverManager::isDebValid(const QString &filePath)
{
    QMimeDatabase typedb;
    QMimeType filetype = typedb.mimeTypeForFile(filePath);
    if (filetype.filterString().contains("deb"))
        return  mp_debinstaller->isDebValid(filePath);
    return true;
}

/**
 * @brief DriverManager::unInstallModule 驱动卸载
 * @param moduleName 模块名称 like hid or hid.ko
 * @return true: 成功 false: 失败
 */
bool DriverManager::unInstallModule(const QString &moduleName, QString &msg)
{
    bool bsuccess = true;
    sigProgressDetail(40, "");
    if (!mp_modcore->rmModForce(moduleName, msg)) {
        bsuccess = false;
    } else {
        sigProgressDetail(60, "");
        QString shortname = mp_modcore->modGetName(moduleName);
        if (!mp_modcore->addModBlackList(shortname)) {
            bsuccess = false;
        } else {
            //如果非内建模块检测去除自启动设置,不影响结果所以不以该操作结果做最终结果判断
            if (!mp_modcore->modIsBuildIn(shortname)) {
                mp_modcore->rmModLoadedOnBoot(shortname);
            }
        }
    }
    sigProgressDetail(80, "");
    return  bsuccess;
}

bool DriverManager::checkPrinterInfo()
{
    //打印机
    QMap<QString, QString> mapInfo;
    loadPrinterInfo(mapInfo);
    if(mapInfo.size() < 1){
        return false;
    }
    //qInfo() << mapInfo;//"printer-make-and-model" //
    QStringList lstInfo = mapInfo["printer-info"].split(" ");
    QString strVendor = "";
    if (lstInfo.size() > 1) {
        strVendor = lstInfo[0];
    }
    DriverInfo di;
    di.type       = DR_Printer;
    di.modelName  = mapInfo["printer-make-and-model"];
    di.vendorName = strVendor;
    if (HttpDriverInterface::getInstance()->checkDriverInfo(di))
        return true;
    return false;
}

void DriverManager::loadPrinterInfo(QMap<QString, QString> &mapInfo)
{
    // 通过cups获取打印机信息
    //cups会识别打印机信息，之前通过文件判断概率性出现文件无信息的情况
    cups_dest_t *dests = nullptr;
    http_t *http = nullptr;
    int num_dests;
    num_dests = cupsGetDests2(http, &dests);
    if (nullptr == dests) {
        cupsFreeDests(num_dests, dests);
        return;
    }
    for (int i = 0; i < num_dests; i++) {
        cups_dest_t *dest = nullptr;
        dest = dests + i;
        getMapInfo(mapInfo, dest);
        // 这里为了和打印机管理保持一致，做出限制
//        if (mapInfo.size() > 10)
//            addMapInfo("printer", mapInfo);
    }
    cupsFreeDests(num_dests, dests);
}

void DriverManager::getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if ((*it).contains("PS/2 Mouse")) {
            words.clear();
            words << "Hotplug" << "PS/2";
        }
        if (words.size() != 2)
            continue;

        if (mapInfo.find(words[0].trimmed()) != mapInfo.end())
            mapInfo[words[0].trimmed()] += QString(" ");

        QRegExp re(".*\"(.*)\".*");
        if (re.exactMatch(words[1].trimmed())) {
            QString key = words[0].trimmed();
            QString value = re.cap(1);

            //这里是为了防止  "usb-storage", "sr"  -》 usb-storage", "sr
            // bug112311 驱动模块显示异常
            if ("Driver" ==  key || "Driver Modules" ==  key)
                value.replace("\"", "");

            // 如果信息中有unknown 则过滤
            if (!value.contains("unknown"))
                mapInfo[key] += value;

        } else {
            if ("Resolution" == words[0].trimmed()) {
                mapInfo[words[0].trimmed()] += words[1].trimmed();
            } else {
                // 如果信息中有unknown 则过滤
                if (!words[1].trimmed().contains("unknown"))
                    mapInfo[words[0].trimmed()] = words[1].trimmed();
            }
        }
    }

    if (mapInfo.find("Module Alias") != mapInfo.end()) {
        mapInfo["Module Alias"].replace(QRegExp("[0-9a-zA-Z]{10}$"), "");
    }
}

bool DriverManager::isNetworkOnline()
{
    /*
       -c 2（代表ping次数，ping 2次后结束ping操作） -w 2（代表超时时间，2秒后结束ping操作）
    */
 // system("ping www.google.com -c 2 -w 2 >netlog.bat");
    system("ping www.baidu.com -c 2 -w 2 >netlog.bat");
    sleep(2);

    //把文件一行一行读取放入vector
    std::ifstream infile;
    infile.open("netlog.bat");
    string s;
    std::vector<string> v;
    while(infile)
    {
        getline(infile,s);
        if(infile.fail())
            break;
        v.push_back(s);
    }
    infile.close();

    //读取倒数第二行 2 packets transmitted, 2 received, 0% packet loss, time 1001ms
    if (v.size() > 1)
    {
        string data = v[v.size()-2];
        int iPos = data.find("received,");
        if (iPos != -1 )
        {
            data = data.substr(iPos+10,3);//截取字符串返回packet loss
            int  n = atoi(data.c_str());
            if(n == 0)
             return 1;
            else
            return 0 ;
        }else{
            return 0;
        }

    }else{
        return 0;
    }
}

bool DriverManager::checkBoardCardInfo(const DriverType type, QMap<QString, QString> &mapInfo)
{
    QString strVendor, strDevice;
    bool flag = false;//是否需要查sysfs
    if(mapInfo.find("Vendor") != mapInfo.end()){
        QStringList strListVendor = mapInfo["Vendor"].split(" ");
        foreach (QString strSub, strListVendor) {
            if (strSub.contains("0x")) {
                strVendor = strSub;
                break;
            }
        }
        if(strVendor.isEmpty()){
            flag = true;
        }
        QStringList strListDevice = mapInfo["Device"].split(" ");
        foreach(QString strSub, strListDevice){
            if(strSub.contains("0x")){
                strDevice = strSub;
                break;
            }
        }
        if(strDevice.isEmpty()){
            flag = true;
        }
    }
    else {
        flag = true;
    }
    if(flag) {
        //
        QString strSysFSLink;
        if (mapInfo.find("SysFS ID") != mapInfo.end() && mapInfo["SysFS ID"].contains("/devices/")) {
            strSysFSLink = mapInfo["SysFS ID"];
        }
        //
        else if (mapInfo.find("SysFS Device Link") != mapInfo.end() && mapInfo["SysFS Device Link"].contains("/devices/")) {
            strSysFSLink = mapInfo["SysFS Device Link"];
        }
        QString strVendorFile = "/vendor";
        QString strDeviceFile = "/device";
        if(strSysFSLink.contains("usb")){
            strVendorFile = "/idVendor";
            strDeviceFile = "/idProduct";
        }
        if (strSysFSLink.isEmpty()) {
            return false;
        }
        if (!QFile::exists("/sys" + strSysFSLink)) {
            return false;
        }
        if(strSysFSLink.contains("usb")){
            if (!QFile::exists("/sys" + strSysFSLink + strVendorFile)) {
                strSysFSLink = strSysFSLink.mid(0, strSysFSLink.lastIndexOf('/'));
            }
        }
        QFile file("/sys" + strSysFSLink + strVendorFile);
        if(!file.open(QIODevice::ReadOnly)){
            return false;
        }
        strVendor = file.readAll();
        file.close();
        QFile fileDevice("/sys" + strSysFSLink + strDeviceFile);
        if(!fileDevice.open(QIODevice::ReadOnly)){
            return false;
        }
        strDevice = fileDevice.readAll();
        fileDevice.close();
        if(strSysFSLink.contains("usb")){
            strVendor = "0x" + strVendor;
            strDevice = "0x" + strDevice;
        }
    }

    if(strVendor.isEmpty() && strDevice.isEmpty()){
        return false;
    }
    DriverInfo di;
    di.type       = type;
    di.vendorId   = strVendor;
    di.modelId   = strDevice;

    di.driverName = mapInfo["Driver"];//如果没有，后续会处理，无影响
    di.version = getDriverVersion(mapInfo["Driver"]);
    if(di.version.isEmpty()){
        di.version = getDriverVersion(mapInfo["Driver Modules"]);
        if(!di.version.isEmpty()){
            di.driverName = mapInfo["Driver Modules"];//如果没有，后续会处理，无影响
        }
    }
    if (HttpDriverInterface::getInstance()->checkDriverInfo(di))
        return true;
    return false;
}
bool DriverManager::checkBoardCardInfo()
{
    QMap<QString, QString> mapInfo;

    const QString &info = DeviceInfoManager::getInstance()->getInfo("hwinfo");

    // 获取信息
    QStringList items = info.split("\n\n");

    foreach (const QString &item, items) {
        if (item.isEmpty())
            continue;

        mapInfo.clear();
        getMapInfoFromHwinfo(item, mapInfo);
        if (mapInfo["Hardware Class"] == "sound" || mapInfo["Device"].contains("USB Audio")) {
            if (checkBoardCardInfo(DR_Sound, mapInfo)) return true;
        } else if (mapInfo["Hardware Class"].contains("network")) {
            if (checkBoardCardInfo(DR_Network, mapInfo)) return true;
        } else if ("keyboard" == mapInfo["Hardware Class"]) {
            continue;
        } else if ("mouse" == mapInfo["Hardware Class"]) {
            continue;
        } else if ("cdrom" == mapInfo["Hardware Class"]) {
            continue;
        } else if ("disk" == mapInfo["Hardware Class"]) {
            continue;
        } else if ("graphics card" == mapInfo["Hardware Class"]) {
            if (mapInfo["Device"].contains("Graphics Processing Unit"))
                continue;
            qInfo() << item;
            if (checkBoardCardInfo(DR_Gpu, mapInfo)) return true;
        } else {
            if (mapInfo["Hardware Class"] == "hub" || mapInfo["Hardware Class"] == "mouse" || mapInfo["Hardware Class"] == "keyboard")
                continue;

            // USB包括：蓝牙、图像设备
            // hwinfo中对camera的分类不明确，通过camera等关键字认定图像设备
            // 当前只做扫描仪。无法区分摄像头和扫描仪。
            if (mapInfo["Model"].contains("camera", Qt::CaseInsensitive) ||
                    mapInfo["Device"].contains("camera", Qt::CaseInsensitive) ||
                    mapInfo["Driver"].contains("uvcvideo", Qt::CaseInsensitive) ||
                    mapInfo["Model"].contains("webcam", Qt::CaseInsensitive) ||
                    mapInfo["Hardware Class"] != "camera") {
                if (checkCameraInfo(mapInfo)) return true;
            }
            if (mapInfo["Hardware Class"] == "bluetooth" || mapInfo["Driver"] == "btusb" || mapInfo["Device"] == "BCM20702A0") {
                if (checkBoardCardInfo(DR_Bluetooth, mapInfo)) return true;
            }
        }
    }
    return false;
}

bool DriverManager::checkCameraInfo(QMap<QString, QString> &mapInfo)
{
    QString strVendor, strDevice;
    QStringList strListVendor = mapInfo["Vendor"].split(" ");
    foreach (QString strSub, strListVendor) {
        if (strSub.contains("0x")) {
            strVendor = strSub;
        }
    }
    QStringList strListDevice = mapInfo["Device:"].split(" ");
    foreach (QString strSub, strListDevice) {
        if (strSub.contains("0x")) {
            strDevice = strSub;
        }
    }
    if(strVendor.isEmpty() && strDevice.isEmpty()){
        return false;
    }
    DriverInfo di;
    di.type       = DR_Camera;
    di.vendorId   = strVendor;
    di.modelId   = strDevice;
    if (HttpDriverInterface::getInstance()->checkDriverInfo(di))
        return true;
    return false;
}

QString DriverManager::getDriverVersion(QString strDriver)
{
    QProcess process;
    QStringList options;
    options << "-c" << "modinfo " + strDriver + " | grep version";

    process.start("/bin/bash", options);
    process.waitForFinished(-1);

    QStringList str = QString(process.readAll()).split("\n\n");
    foreach(QString temp, str){
        if(temp.startsWith("version") && temp.contains(":") && temp.split(':').size() > 1){
            return temp.split(':')[2];
        }
    }
    return "";
}
void DriverManager::getMapInfo(QMap<QString, QString> &mapInfo, cups_dest_t *src)
{
    // 获取打印机信息
    mapInfo.insert("Name", src->name);
    for (int i = 0; i < src->num_options; i++)
        mapInfo.insert(src->options[i].name, src->options[i].value);
}
/**
 * @brief DriverManager::uninstallPrinter 卸载打印机
 * @param name 打印机名
 * @param vendor 制造商
 * @param model 型号
 * @return true: 成功 false: 失败
 */
bool DriverManager::uninstallPrinter(const QString &vendor, const QString &model)
{
    auto archMap = QMap<QString, QString> {
        {"x86_64", "x86"},
        {"i386", "x86"},
        {"i686", "x86"},
        {"mips64", "mips64"},
        {"aarch64", "aarch64"},
        {"loongarch64", "loongarch64"}
    };
    QJsonObject obj = {
        QPair<QString, QJsonValue>(SD_KEY_ver, "1.2.0"),
        QPair<QString, QJsonValue>(SD_KEY_code, "godfather"),
        QPair<QString, QJsonValue>(SD_KEY_mfg, vendor),
        QPair<QString, QJsonValue>(SD_KEY_mdl, model),
        QPair<QString, QJsonValue>(SD_KEY_ieeeid, ""),
    };

    QString url = "http://drive-pre.uniontech.com/api/v1/drive/search?arch=" + archMap[Utils::machineArch()];
    url += "&desc=" + model;
    url += "&deb_manufacturer=" + vendor;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkAccessManager networkManager;
    QNetworkReply *reply = networkManager.get(request);
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> replyPtr(reply);

    sigProgressDetail(0, "");
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();

    //超时
    sigProgressDetail(10, "");
    if (!timer.isActive())
        RETURN_VALUE(false);

    //无返回数据
    sigProgressDetail(20, "");
    QByteArray data = reply->readAll();
    if (data.isEmpty())
        RETURN_VALUE(false);

    //获取packageName
    sigProgressDetail(30, "");
    QList<TDriverInfo> lstDrivers = parsePrinterInfo(data);
    if (lstDrivers.isEmpty())
        RETURN_VALUE(false);

    QString packageName = lstDrivers[0].strDriver;
    //1.若驱动已经删除，但是驱动名还在，会执行孔的卸载，然后删除打印机
    //2.卸载打印机要确保联网状态
    //3.驱动不存在，直接返回false
    //未安装package
    sigProgressDetail(40, "");
    if (!printerHasInstalled(packageName)) {
        RETURN_VALUE(false);
    }

    //卸载package
    sigProgressDetail(40, "");
    if (!unInstallPrinter(packageName))
        RETURN_VALUE(false);

    //卸载成功
    sigProgressDetail(90, "");
    RETURN_VALUE(true);
}

/**
 * @brief DriverManager::parsePrinterInfo 解析打印服务器返回的数据
 * @param byteArray 服务返回的数据
 * @return list 返回打印机包的列表
 */
QList<DriverManager::TDriverInfo> DriverManager::parsePrinterInfo(const QByteArray &byteArray)
{
    QList<TDriverInfo> lstDrivers;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(byteArray, &err);
    QJsonArray array = doc.object()["solutions"].toArray();
    if (!array.isEmpty()) {
        foreach (QJsonValue value, array) {
            QJsonObject ppdobject = value.toObject();
            TDriverInfo driver;
            driver.intSid = ppdobject.value(SD_KEY_sid).toInt();
            driver.strDescribe = ppdobject.value(SD_KEY_describe).toString();
            driver.strPpd = ppdobject.value(SD_KEY_ppd).toString();
            driver.isExcat = ppdobject.value(SD_KEY_excat).toBool();
            driver.strDriver = ppdobject.value(SD_KEY_driver).toString();

            lstDrivers.append(driver);
        }
    }
    return lstDrivers;
}

/**
 * @brief DriverManager::printerHasInstalled 打印机是否被安装
 * @param packageName 包名
 * @return true: 成功 false: 失败
 */
bool DriverManager::printerHasInstalled(const QString &packageName)
{
    QProcess p;
    QString cmd = "sudo dpkg -s " + packageName;
    p.start(cmd);
    p.waitForFinished(-1);

    QByteArray r = p.readAll();
    return r.contains("installed");
}

/**
 * @brief DriverManager::installPrinter 安装打印机驱动
 * @param packageName 包名
 * @return  true: 成功 false: 失败
 */
bool DriverManager::installPrinter(const QString &packageName)
{
    QProcess p;
    p.start("sudo apt install " + packageName);
    p.waitForFinished(-1);

    return printerHasInstalled(packageName);
}

/**
 * @brief DriverManager::unInstallPrinter 卸载打印机驱动
 * @param packageName 包名
 * @return  true: 成功 false: 失败
 */
bool DriverManager::unInstallPrinter(const QString &packageName)
{
    QProcess p;
    p.start("sudo dpkg -r " + packageName);
    p.waitForFinished(-1);

    return !printerHasInstalled(packageName);
}
