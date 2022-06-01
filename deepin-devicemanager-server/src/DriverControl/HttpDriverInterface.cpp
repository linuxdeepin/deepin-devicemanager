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

static QString strRepoUrl = "https://driver.uniontech.com/api/v1/drive/search";

HttpDriverInterface::HttpDriverInterface(QObject *parent) : QObject(parent)
{
}

HttpDriverInterface::~HttpDriverInterface()
{

}

QString HttpDriverInterface::getRequestJson(QString strUrl)
{
    const QUrl newUrl = QUrl::fromUserInput(strUrl);

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
    if (error != QNetworkReply::NoError) {
        return "";
    }

    return strJsonDriverInfo;
}

bool HttpDriverInterface::checkDriverInfo(DriverInfo &driverInfo)
{
    QString strJson;
    switch (driverInfo.type) {
    case DR_Printer:
        strJson = getRequestPrinter(driverInfo.vendorName, driverInfo.modelName); break;
    //case DR_Camera:
    case DR_Scaner:
//        strJson = getRequestCamera(driverInfo.modelName);
//        break;
    case DR_Sound:
    case DR_Gpu:
    case DR_Network:
    case DR_WiFi:
        strJson = getRequestBoard(driverInfo.vendorId, driverInfo.modelId); break;
    default:
        break;
    }
    return checkDriverInfo(strJson, driverInfo);//todo: 需要增加逻辑：判断是否已安装驱动，且驱动与推荐驱动level相同。
}

QString HttpDriverInterface::getRequestBoard(QString strManufacturer, QString strProducts, int iClassP, int iClass)
{
    QString arch = Common::getArchStore();
    QString strUrl = strRepoUrl + "?arch=" + arch;
    int iType = DTK_CORE_NAMESPACE::DSysInfo::uosType();
    int iEditionType = DTK_CORE_NAMESPACE::DSysInfo::uosEditionType();
    strUrl += "&system=" + QString::number(iType) + '-' + QString::number(iEditionType);

    if(!strManufacturer.isEmpty()){
        strUrl += "&deb_manufacturer=" + strManufacturer;
    }
    if(!strProducts.isEmpty()){
        strUrl += "&product=" + strProducts;
    }
    if(0 < iClassP){
        strUrl += "&class_p=" + QString(iClassP);
    }
    if(0 < iClass){
        strUrl += "&class=" + QString(iClass);
    }
    qInfo() << strUrl;
    return getRequestJson(strUrl);
}

QString HttpDriverInterface::getRequestPrinter(QString strDebManufacturer, QString strDesc)
{
    QString arch = Common::getArchStore();
    QString strUrl = strRepoUrl + "?arch=" + arch;
    int iType = DTK_CORE_NAMESPACE::DSysInfo::uosType();
    int iEditionType = DTK_CORE_NAMESPACE::DSysInfo::uosEditionType();
    strUrl += "&system=" + QString::number(iType) + '-' + QString::number(iEditionType);

    if(!strDebManufacturer.isEmpty()){
        if (strDebManufacturer == "HP" || strDebManufacturer == "Hewlett-Packard") {
            strDebManufacturer = "HP";
        }
        strUrl += "&deb_manufacturer=" + strDebManufacturer;
    }
    if(!strDesc.isEmpty()){
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

    if(!strDesc.isEmpty()){
        strUrl += "&desc=" + strDesc;
    }
    return getRequestJson(strUrl);
}

bool HttpDriverInterface::checkDriverInfo(QString strJson, DriverInfo &driverInfo)
{
    if(strJson.isEmpty()){
        return false;
    }
    QList<RepoDriverInfo> lstDriverInfo;

    if(getDriverInfoFromJson(strJson, lstDriverInfo)){
        if(lstDriverInfo.size() == 0){
            return false;
        }

        int max = 0;
        foreach (const RepoDriverInfo& strDriverInfo , lstDriverInfo){
            if(max < strDriverInfo.iLevel){
                max = strDriverInfo.iLevel;
            }
        }
        //因为无法从设备型号查询到驱动包的名称（可能有多种同样推荐等级的驱动），所以就查询到这些推荐驱动并遍历，查询本地是否安装。
        foreach (const RepoDriverInfo& strDriverInfo , lstDriverInfo){
            if(max == strDriverInfo.iLevel){
                if(driverInfo.driverName.isEmpty() || strDriverInfo.strPackages != driverInfo.driverName || driverInfo.version.isEmpty()){//
                    //如果有一个，且版本也对，则返回false，否则返回true
                    if(isPkgInstalled(strDriverInfo.strPackages, strDriverInfo.strDebVersion)){
                        return false;
                    }
                }
                else {
                    //如果本地获取的驱动能与仓库推荐驱动中任意一个匹配，则返回false，不需要更新
                    if(strDriverInfo.strDebVersion == driverInfo.version){
                        return false;
                    }
                }
            }
        }
        qInfo() << "packages: "   << driverInfo.packages;
        qInfo() << "debVersion: " << driverInfo.debVersion;
        qInfo() << "driverName: " << driverInfo.driverName;
        qInfo() << "version: "    << driverInfo.version;

        qInfo() << lstDriverInfo.size();
        foreach(const RepoDriverInfo &di, lstDriverInfo){
            qInfo() << "strPackages: " << di.strPackages;
            qInfo() << "strDebVersion: " << di.strDebVersion;
        }
        return true;
    }
    return false;
}

bool HttpDriverInterface::isPkgInstalled(QString strPkgName, QString strVersion)
{
    //调用apt命令查看包是否安装。
    QProcess process;
    QStringList options;
    options << "-c" << "apt policy " + strPkgName;
    process.start("/bin/bash", options);
    process.waitForFinished(-1);
    QStringList infoList = QString(process.readAllStandardOutput()).split("\n");

    if(infoList.size() > 2 && infoList[1].contains(strVersion)){
        return true;
    }
    return false;
}

bool HttpDriverInterface::getDriverInfoFromJson(QString strJson, QList<RepoDriverInfo> &lstDriverInfo)
{
    QJsonArray ja;
    QJsonArray jappds;
    QJsonArray jamodel;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(strJson.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return false;
    }
    if("success" != jsonDoc.object().value("msg").toString()){
        return false;
    }
    ja = jsonDoc.object().value("data").toObject().value("list").toArray();

    QJsonObject _jsonObj;
    QJsonObject _jsonObjppds;
    for (int i = 0; i < ja.size(); i++) {
        _jsonObj = ja.at(i).toObject();
        RepoDriverInfo _driverinfo;
        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if (_jsonObj.contains("deb_manufacturer")) {
            _driverinfo.strDebManufacturer = _jsonObj.value("deb_manufacturer").toString();
        }
        if (_jsonObj.contains("deb_version")) {
            _driverinfo.strDebVersion = _jsonObj.value("deb_version").toString();
        }
        if (_jsonObj.contains("packages")) {
            _driverinfo.strPackages = _jsonObj.value("packages").toString();
        }
        if (_jsonObj.contains("level")) {
            _driverinfo.iLevel = _jsonObj.value("level").toInt();
        }

        lstDriverInfo.push_back(_driverinfo);
    }
    return true;
}

