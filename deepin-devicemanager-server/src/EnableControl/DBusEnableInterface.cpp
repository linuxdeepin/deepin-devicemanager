#include "DBusEnableInterface.h"
#include "EnableSqlManager.h"
#include "DeviceInfoManager.h"
#include "MainJob.h"

#include <QFile>
#include <QDir>
#include <QDebug>

// 系统库文件
#include <cups.h>
#include <unistd.h>
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

QString DBusEnableInterface::enable(const QString& hclass, const QString& name, const QString& path, const QString& value, bool enable_device)
{
    // 先从数据库中查找
    QString sPath = EnableSqlManager::getInstance()->authorizedPath(value);
    if(sPath.isEmpty()){
        sPath = path;
    }
    QFile file;
    if(file.exists(sPath+QString("/authorized"))){
        return authorizedEnable(hclass, name, sPath, value, enable_device);
    }else{
        return removeEnable(hclass, name, path, enable_device);
    }
}

Q_SCRIPTABLE QString DBusEnableInterface::enablePrinter(const QString& hclass, const QString& name, const QString& path, bool enable_device)
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
        return "0";
    }
    int port = ippPort();
    int encrption = (http_encryption_t)cupsEncryption();
    int cancel = 0;
    http_t *http = httpConnect2(host,port,nullptr,AF_UNSPEC,(http_encryption_t)encrption,1,30000,&cancel);
    if(!http){
        return "0";
    }
    answer = cupsDoRequest(http,request,"/admin/");
    ippDelete(answer);

    // 禁用成功后将信息保存到数据库
    if(enable_device){
        EnableSqlManager::getInstance()->removeDataFromPrinterTable(name);
    }else{
        EnableSqlManager::getInstance()->insertDataToPrinterTable(hclass, name, path);
    }

    return "1";
}


QString DBusEnableInterface::authorizedEnable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id, bool enable_device)
{
    // 通过authorized文件启用禁用设备
    // 0:表示禁用 ，1:表示启用
    QFile file(path+QString("/authorized"));
    if(!file.open(QIODevice::ReadWrite)){
        return "0";
    }
    if(enable_device){
        qint64 len = file.write("1");
        file.close();
        if(len < 1){
            return "0";
        }
        EnableSqlManager::getInstance()->removeDataFromAuthorizedTable(unique_id);

        //*先判断后台内存里面有没有该数据
        if(DeviceInfoManager::getInstance()->isPathExisted(path)){
            return "1";
        }
        //*如果后台的内存里面没有数据则需要等待后台更新数据
        emit update();
        while(true){
            QDir dir(path);
            if(dir.exists())
                break;
            usleep(100);
        }
        while (MainJob::serverIsRunning()) {
            usleep(1000);
        }
    }else{
        qint64 len = file.write("0");
        file.close();
        if(len < 1){
            return "0";
        }
        EnableSqlManager::getInstance()->insertDataToAuthorizedTable(hclass,name,path,unique_id);
    }
    return "1";
}

QString DBusEnableInterface::removeEnable(const QString& hclass, const QString& name, const QString& path, bool enable)
{
    if(enable){
        // 1. 先rescan 向rescan写入1,则重新加载
        QFile file("/sys/bus/pci/rescan");
        if(!file.open(QIODevice::WriteOnly)){
            return "0";
        }
        qint64 len = file.write("1");
        file.close();
        if(len < 1){
            return "0";
        }
        EnableSqlManager::getInstance()->removeDateFromRemoveTable(path);

        // 2. 通知后台更新数据
        //*先判断后台内存里面有没有该数据
        if(DeviceInfoManager::getInstance()->isPathExisted(path)){
            return "1";
        }
        //*如果后台的内存里面没有数据则需要等待后台更新数据
        while(true){
            QDir dir(path);
            if(dir.exists())
                break;
            usleep(100);
        }
        emit update();
        while (MainJob::serverIsRunning()) {
            usleep(1000);
        }

        // 由于rescan会将所有的remove数据都回复，因此需要重新禁用其它设备
        QStringList rpList;
        EnableSqlManager::getInstance()->removePathList(rpList);
        foreach(const QString& path,rpList){
            QFile filerp(path+QString("/remove"));
            if(filerp.open(QIODevice::WriteOnly)){
                filerp.write("1");
                filerp.close();
            }
        }

        // 3. 持久化保存
    }else{
        // 1. 直接remove写入
        // 通过remove文件禁用
        // 1:表示禁用 ，0:表示启用
        QFile file(path+QString("/remove"));
        if(!file.open(QIODevice::WriteOnly)){
            return "0";
        }
        qint64 len = file.write("1");
        file.close();
        if(len < 1){
            return "0";
        }

        // 2. 持久化保存
        EnableSqlManager::getInstance()->insertDataToRemoveTable(hclass, name, path);
    }

    return "1";
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
