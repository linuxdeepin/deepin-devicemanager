#include "HttpDriverInterface.h"
#include "commonfunction.h"

#include <QJsonDocument>
#include <QtNetwork>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <DSysInfo>

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<HttpDriverInterface *> HttpDriverInterface::s_Instance;
std::mutex HttpDriverInterface::m_mutex;

static QString strRepoUrl = "http://drive-pre.uniontech.com/api/v1/drive/search";

HttpDriverInterface::HttpDriverInterface(QObject *parent) : QObject(parent)
{
}

HttpDriverInterface::~HttpDriverInterface()
{

}

QString HttpDriverInterface::getRequestJson(QString strUrl)
{
    strJsonDriverInfo = "";
    const QUrl newUrl = QUrl::fromUserInput(strUrl);
    qInfo() << "strUrl : " << newUrl;

    QNetworkRequest request(newUrl);
    reply.reset(qnam.get(request));

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply.get(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();

    strJsonDriverInfo = reply->readAll();
    //! [networkreply-error-handling-1]
    QNetworkReply::NetworkError error = reply->error();

    reply.reset();
    qInfo() << error;
    if (error != QNetworkReply::NoError) {
        return "network error";
    }
    return strJsonDriverInfo;
}

void HttpDriverInterface::getRequest(DriverInfo *driverInfo)
{
    QString strJson;
    switch (driverInfo->type()) {
    case DR_Printer:
        strJson = getRequestPrinter(driverInfo->vendorName(), driverInfo->modelName()); break;
    //case DR_Camera:
    case DR_Scaner:
//        strJson = getRequestCamera(driverInfo->modelName());
//        break;
    case DR_Sound:
    case DR_Gpu:
    case DR_Network:
    case DR_WiFi:
        strJson = getRequestBoard(driverInfo->vendorId(), driverInfo->modelId()); break;
    default:
        break;
    }

    if (strJson.contains("network error")) {
        emit sigRequestFinished(false, "network error");
    } else {
        checkDriverInfo(strJson, driverInfo);
        qInfo() << "m_VendorId:" << driverInfo->m_VendorId;
        qInfo() << "m_ModelId:" << driverInfo->m_ModelId;
        qInfo() << "m_Packages:" << driverInfo->m_Packages;
        qInfo() << "m_DebVersion:" << driverInfo->m_DebVersion;

        qInfo() << "m_Status:" << driverInfo->m_Status;
    }
}

QString HttpDriverInterface::getRequestBoard(QString strManufacturer, QString strModels, int iClassP, int iClass)
{
    QString arch = Common::getArchStore();
    QString strUrl = strRepoUrl + "?arch=" + arch;
    int iType = DTK_CORE_NAMESPACE::DSysInfo::uosType();
    int iEditionType = DTK_CORE_NAMESPACE::DSysInfo::uosEditionType();
    strUrl += "&system=" + QString::number(iType) + '-' + QString::number(iEditionType);

    if (!strManufacturer.isEmpty()) {
        strUrl += "&deb_manufacturer=" + strManufacturer;
    }
    if (!strModels.isEmpty()) {
        strUrl += "&product=" + strModels;
    }
    if (0 < iClassP) {
        strUrl += "&class_p=" + QString(iClassP);
    }
    if (0 < iClass) {
        strUrl += "&class=" + QString(iClass);
    }
    return getRequestJson(strUrl);
}

QString HttpDriverInterface::getRequestPrinter(QString strDebManufacturer, QString strDesc)
{
    QString arch = Common::getArchStore();
    QString strUrl = strRepoUrl + "?arch=" + arch;
    int iType = DTK_CORE_NAMESPACE::DSysInfo::uosType();
    int iEditionType = DTK_CORE_NAMESPACE::DSysInfo::uosEditionType();
    strUrl += "&system=" + QString::number(iType) + '-' + QString::number(iEditionType);

    if (!strDebManufacturer.isEmpty()) {
        if (strDebManufacturer == "HP" || strDebManufacturer == "Hewlett-Packard") {
            strDebManufacturer = "HP";
        }
        strUrl += "&deb_manufacturer=" + strDebManufacturer;
    }
    if (!strDesc.isEmpty()) {
        strUrl += "&desc=" + strDesc;
    }
    return getRequestJson(strUrl);
}

QString HttpDriverInterface::getRequestCamera(QString strDesc)
{
    QString arch = Common::getArchStore();
    QString strUrl = strRepoUrl + "?arch=" + arch;
    int iType = DTK_CORE_NAMESPACE::DSysInfo::uosType();
    int iEditionType = DTK_CORE_NAMESPACE::DSysInfo::uosEditionType();
    strUrl += "&system=" + QString::number(iType) + '-' + QString::number(iEditionType);

    if (!strDesc.isEmpty()) {
        strUrl += "&desc=" + strDesc;
    }
    return getRequestJson(strUrl);
}

void HttpDriverInterface::checkDriverInfo(QString strJson, DriverInfo *driverInfo)
{
    if (strJson.isEmpty()) {
        return;
    }
    QList<strDriverInfo> lstDriverInfo;
    if (convertJsonToDeviceList(strJson, lstDriverInfo)) {
        if (lstDriverInfo.size() == 0) {
            return;
        }

        int max = 0;
        foreach (const strDriverInfo& info , lstDriverInfo) {
            if (max < info.iLevel) {
                max = info.iLevel;
            }
        }
        //因为无法从设备型号查询到驱动包的名称（可能有多种同样推荐等级的驱动），所以就查询到这些推荐驱动并遍历，查询本地是否安装。
        foreach (const strDriverInfo& info , lstDriverInfo) {
            if (max == info.iLevel) {
                if (driverInfo->driverName().isEmpty() || driverInfo->version().isEmpty()) {
                    //如果有一个，且版本也对，则不需要更新，否则要更新
                    QProcess process;
                    QStringList options;
                    options << "-c" << "apt policy " + info.strPackages;
                    process.start("/bin/bash", options);
                    process.waitForFinished(-1);
                    QStringList infoList = QString(process.readAllStandardOutput()).split("\n");
                    if (infoList.size() > 2) {
                        if (infoList[1].contains(info.strDebVersion)) {
                            driverInfo->m_Status = ST_DRIVER_IS_NEW;
                            driverInfo->m_DebVersion  = info.strDebVersion;
                            return;
                        } else if (!infoList[1].contains("（") || !infoList[1].contains("(")) { //不是(none)（无），则是更新
                            //本地有安装包，但版本较低
                            driverInfo->m_DebVersion  = info.strDebVersion;
                            driverInfo->m_Packages = info.strPackages;
                            driverInfo->m_Status   = ST_CAN_UPDATE;
                            return;
                        }
                    }
                } else {
                    //如果本地获取的驱动能与仓库推荐驱动中任意一个匹配，则返回false，不需要更新
                    if (info.strPackages.contains(driverInfo->driverName())) {
                        if (info.strDebVersion == driverInfo->version()) {
                            driverInfo->m_Status = ST_DRIVER_IS_NEW;
                            return;
                        } else {
                            //本地有安装包，但版本较低
                            driverInfo->m_DebVersion  = info.strDebVersion;
                            driverInfo->m_Packages = info.strPackages;
                            driverInfo->m_Status   = ST_CAN_UPDATE;
                            return;
                        }
                    }
                }
            }
        }
        foreach (const strDriverInfo& info , lstDriverInfo) {
            if (max == info.iLevel) {
                driverInfo->m_DebVersion = info.strDebVersion;
                driverInfo->m_Packages = info.strPackages;
                driverInfo->m_Status = ST_NOT_INSTALL;
                return;
            }
        }
    }
    driverInfo->m_Status = ST_DRIVER_IS_NEW;
    return;
}


bool HttpDriverInterface::convertJsonToDeviceList(QString strJson, QList<strDriverInfo> &lstDriverInfo)
{
    QJsonArray ja;
    QJsonArray jappds;
    QJsonArray jamodel;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(strJson.toLocal8Bit(), &json_error));

    lstDriverInfo.clear();
    if (strJson.isEmpty() || json_error.error != QJsonParseError::NoError) {
        return false;
    }
    if ("success" != jsonDoc.object().value("msg").toString()) {
        return false;
    }
    ja = jsonDoc.object().value("data").toObject().value("list").toArray();
    if (ja.size() < 1) {
        return false;
    }

    QJsonObject _jsonObj;
    QJsonObject _jsonObjppds;
    for (int i = 0; i < ja.size(); i++) {
        _jsonObj = ja.at(i).toObject();
        strDriverInfo _driverinfo;
        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if (_jsonObj.contains("arch")) {
            _driverinfo.strArch = _jsonObj.value("arch").toString();
        }
        if (_jsonObj.contains("manufacturer")) {
            _driverinfo.strManufacturer = _jsonObj.value("manufacturer").toString();
        }
        if (_jsonObj.contains("deb_manufacturer")) {
            _driverinfo.strDebManufacturer = _jsonObj.value("deb_manufacturer").toString();
        }
        if (_jsonObj.contains("version")) {
            _driverinfo.strVersion = _jsonObj.value("version").toString();
        }
        if (_jsonObj.contains("deb_version")) {
            _driverinfo.strDebVersion = _jsonObj.value("deb_version").toString();
        }
        if (_jsonObj.contains("packages")) {
            _driverinfo.strPackages = _jsonObj.value("packages").toString();
        }
        if (_jsonObj.contains("class_p")) {
            _driverinfo.strClass_p = _jsonObj.value("class_p").toString();
        }
        if (_jsonObj.contains("class")) {
            _driverinfo.strClass = _jsonObj.value("class").toString();
        }
        if (_jsonObj.contains("models")) {
            //_driverinfo.strModels = ;
            jamodel = _jsonObj.value("models").toArray();
            for (int j = 0; j < jamodel.size(); j++) {
                _driverinfo.strModels.push_back(jamodel.at(j).toString());
            }
        }
        if (_jsonObj.contains("products")) {
            _driverinfo.strProducts = _jsonObj.value("products").toString();
        }
        if (_jsonObj.contains("deb")) {
            _driverinfo.strDeb = _jsonObj.value("deb").toString();
        }
        if (_jsonObj.contains("level")) {
            _driverinfo.iLevel = _jsonObj.value("level").toInt();
        }
        if (_jsonObj.contains("system")) {
            _driverinfo.strSystem = _jsonObj.value("system").toString();
        }
        if (_jsonObj.contains("desc")) {
            _driverinfo.strDesc = _jsonObj.value("desc").toString();
        }
        if (_jsonObj.contains("adaptation")) {
            _driverinfo.strAdaptation = _jsonObj.value("adaptation").toString();
        }
        if (_jsonObj.contains("source")) {
            _driverinfo.strSource = _jsonObj.value("source").toString();
        }
        if (_jsonObj.contains("download_url")) {
            _driverinfo.strDownloadUrl = _jsonObj.value("download_url").toString();
        }
        if (_jsonObj.contains("size")) {
            double size = _jsonObj.value("size").toInt();
            if (size < 1024 * 1024) {
                _driverinfo.strSize = QString::number(size / 1024, 'f', 2) + "KB";
            } else if (size < 1024 * 1024 * 1024) {
                _driverinfo.strSize = QString::number(size / 1024 / 1024, 'f', 2) + "MB";
            } else {
                _driverinfo.strSize = QString::number(size / 1024 / 1024 / 1024, 'f', 2) + "GB";
            }
        }
        if (_jsonObj.contains("ppds")) {
            jappds = _jsonObj.value("ppds").toArray();
            strPpds _ppds;
            for (int j = 0; j < jappds.size(); j++) {
                _jsonObjppds = jappds.at(j).toObject();
                if (_jsonObjppds.contains("desc")) {
                    _ppds.strDesc = _jsonObjppds.value("desc").toString();
                }
                if (_jsonObjppds.contains("manufacturer")) {
                    _ppds.strManufacturer = _jsonObjppds.value("manufacturer").toString();
                }
                if (_jsonObjppds.contains("source")) {
                    _ppds.strSource = _jsonObjppds.value("source").toString();
                }
                _driverinfo.lstPpds.push_back(_ppds);
            }
        }

        lstDriverInfo.push_back(_driverinfo);
    }

    return true;
}

