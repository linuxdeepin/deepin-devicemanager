#include "DBusEnableInterface.h"
#include "EnableSqlManager.h"
#include "DeviceInfoManager.h"
#include "MainJob.h"
#include "EnableUtils.h"

#include <QFile>
#include <QDir>
#include <QDebug>
#include <QDateTime>

// 系统库文件
#include <cups.h>
#include <string>

# define HTTP_MAX_URI 1024	/* Max length of URI string */

DBusEnableInterface::DBusEnableInterface(QObject *parent)
    :QObject (parent)
{

}

QString DBusEnableInterface::getRemoveInfo()
{
    return EnableSqlManager::getInstance()->removedInfo();
}

QString DBusEnableInterface::getAuthorizedInfo()
{
    return EnableSqlManager::getInstance()->authorizedInfo();
}

bool DBusEnableInterface::enable(const QString& hclass, const QString& name, const QString& path, const QString& value, bool enable_device, const QString strDriver)
{
    // 网卡通过ioctl禁用
    // 先判断是否是网卡
    QRegExp reg("^[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}$");
    if(reg.exactMatch(value)){
        return ioctlEnableNetwork(hclass,name,path,value,enable_device,strDriver);
    }

    // 先从数据库中查找路径，防止设备更换usb接口
    QString sPath = EnableSqlManager::getInstance()->authorizedPath(value);
    if(sPath.isEmpty()){
        sPath = path;
    }

    // 判断是内置设备，还是外设，内置设备通过remove文件禁用，外设通过authorized文件禁用
    bool res = false;
    if(QFile::exists("/sys" + sPath + QString("/authorized"))){
        modifyPath(sPath);
        res = authorizedEnable(hclass, name, sPath, value, enable_device, strDriver);
    }else/* if(QFile::exists("/sys" + sPath + QString("/remove")))*/{
        res = removeEnable(hclass, name, path, value, enable_device, strDriver);
    }
    emit update();
    return res;
}

Q_SCRIPTABLE bool DBusEnableInterface::enablePrinter(const QString& hclass, const QString& name, const QString& path, bool enable_device)
{
    ipp_op_t op = enable_device ? IPP_OP_RESUME_PRINTER : IPP_OP_PAUSE_PRINTER;
    char uri[HTTP_MAX_URI];
    ipp_t *request = nullptr;
    ipp_t *answer = nullptr;
    request = ippNewRequest(op);
    construct_uri(uri, sizeof (uri), "ipp://localhost/printers/", name.toStdString().c_str());
    ippAddString(request,IPP_TAG_OPERATION,IPP_TAG_URI,"printer-uri",NULL,uri);
    const char* host = cupsServer();
    if(!host){
        return false;
    }
    int port = ippPort();
    int encrption = (http_encryption_t)cupsEncryption();
    int cancel = 0;
    http_t *http = httpConnect2(host,port,nullptr,AF_UNSPEC,(http_encryption_t)encrption,1,30000,&cancel);
    if(!http){
        return false;
    }
    answer = cupsDoRequest(http,request,"/admin/");
    ippDelete(answer);

    // 禁用成功后将信息保存到数据库
    if(enable_device){
        EnableSqlManager::getInstance()->removeDataFromPrinterTable(name);
    }else{
        EnableSqlManager::getInstance()->insertDataToPrinterTable(hclass, name, path);
    }

    return true;
}

Q_SCRIPTABLE bool DBusEnableInterface::isDeviceEnabled(const QString& unique_id)
{
    return EnableSqlManager::getInstance()->isUniqueIdEnabled(unique_id);
}

bool DBusEnableInterface::authorizedEnable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id, bool enable_device, const QString strDriver)
{
    // 通过authorized文件启用禁用设备
    // 0:表示禁用 ，1:表示启用
    QFile file("/sys" + path+QString("/authorized"));
    if(!file.open(QIODevice::ReadWrite)){
        return false;
    }
    if(enable_device){
        /*
         启用的流程为：以 /devices/pci0000:00/0000:00:14.0/usb1/1-5/1-5:1.0 为例
         第一步: 向 /sys/devices/pci0000:00/0000:00:14.0/usb1/1-5/1-5:1.0/authorized 文件中写 1
         第二步: 向 /sys/devices/pci0000:00/0000:00:14.0/usb1/1-5/authorized 文件中写 0
         第三步: 向 /sys/devices/pci0000:00/0000:00:14.0/usb1/1-5/authorized 文件中写 1
         */
        // 第一步
        file.write("1");
        file.close();

        // 第二步
        QFileInfo fi(path);
        QString pop = fi.path();
        QFile fpop("/sys" + pop + QString("/authorized"));
        if(!fpop.open(QIODevice::ReadWrite))
            return false;
        fpop.write("0");
        fpop.close();

        // 第三步
        if(!fpop.open(QIODevice::ReadWrite))
            return false;
        fpop.write("1");
        fpop.close();

        EnableSqlManager::getInstance()->removeDataFromAuthorizedTable(unique_id);
    }else{
        file.write("0");
        file.close();
        EnableSqlManager::getInstance()->insertDataToAuthorizedTable(hclass,name,path,unique_id,true, strDriver);
    }
    return true;
}

bool DBusEnableInterface::removeEnable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id, bool enable, const QString strDriver)
{
    if(enable){
        // 1. 先rescan 向rescan写入1,则重新加载
        QFile file("/sys/bus/pci/rescan");
        if(!file.open(QIODevice::WriteOnly)){
            return false;
        }
        file.write("1");
        file.close();

        // platform disable with reset
        if(path.contains("platform")){
            QFile filep("/sys" + path + QString("/reset"));
            if(!filep.open(QIODevice::WriteOnly)){
                return false;
            }
            filep.write("1");
            filep.close();
        }
        EnableSqlManager::getInstance()->removeDateFromRemoveTable(path);

        // 2. 通知后台更新数据
        //*先判断后台内存里面有没有该数据
        if(DeviceInfoManager::getInstance()->isPathExisted(path)){
            return true;
        }

        // 由于rescan会将所有的remove数据都回复，因此需要重新禁用其它设备
        QStringList rpList;
        EnableSqlManager::getInstance()->removePathList(rpList);
        foreach(const QString& path,rpList){
            QFile filerp("/sys" + path+QString("/remove"));
            if(filerp.open(QIODevice::WriteOnly)){
                filerp.write("1");
                filerp.close();
            }
        }
    }else{
        // 1. 直接remove写入
        // 通过remove文件禁用
        // 1:表示禁用 ，0:表示启用
        qInfo() << "" << "/sys" + path + QString("/remove");
//        return false;
        QFile file("/sys" + path + QString("/remove"));
        if(!file.open(QIODevice::WriteOnly)){
            return false;
        }
        file.write("1");
        file.close();

        // 2. 持久化保存
        EnableSqlManager::getInstance()->insertDataToRemoveTable(hclass, name, path, unique_id, strDriver);
    }
    return true;
}

bool DBusEnableInterface::ioctlEnableNetwork(const QString& hclass, const QString& name, const QString& logical_name, const QString& unique_id, bool enable, const QString strDriver)
{
    // 1. 通过ioctl禁用
    if(!EnableUtils::ioctlOperateNetworkLogicalName(logical_name,enable))
        return false;
    qInfo() << "ioctlEnableNetwork ****** " << enable;
    // 2. 持久化保存
    if(enable){
        EnableSqlManager::getInstance()->removeDataFromAuthorizedTable(unique_id);
    }else{
        EnableSqlManager::getInstance()->insertDataToAuthorizedTable(hclass, name, logical_name, unique_id, enable, strDriver);
    }
    return true;
}

void DBusEnableInterface::construct_uri(char *buffer, size_t buflen, const char *base, const char *value)
{
    char *d = buffer;
    const unsigned char *s = (const unsigned char *)value;
    if (strlen(base) < buflen) {
        strcpy(buffer, base);
        d += strlen(base);
    } else {
        strncpy(buffer, base, buflen);
        d += buflen;
    }

    while (*s && d < buffer + buflen) {
        if (isalpha(*s) || isdigit(*s) || *s == '-')
            *d++ = *s++;
        else if (*s == ' ') {
            *d++ = '+';
            s++;
        } else {
            if (d + 2 < buffer + buflen) {
                *d++ = '%';
                *d++ = "0123456789ABCDEF"[((*s) & 0xf0) >> 4];
                *d++ = "0123456789ABCDEF"[((*s) & 0x0f)];
                s++;
            } else {
                break;
            }
        }
    }

    if (d < buffer + buflen)
        *d = '\0';
}

void DBusEnableInterface::modifyPath(QString& path)
{
    path.replace(QRegExp("[1-9]$"),"0");
}
