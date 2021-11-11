/**
* @brief: 设备信息的基类
* @author: liujun
* @date:2020-04-09
*/
#ifndef DEVICEINFO_H
#define DEVICEINFO_H
#include<QString>
#include<QMap>
#include<QSet>
#include <QObject>

class DeviceBaseInfo
{
public:
    DeviceBaseInfo();
    virtual ~DeviceBaseInfo();

    /**@brief:获取其它设备信息*/
    const QMap<QString, QString> &getOtherAttribs()const;

protected:
    /**@brief:初始化过滤信息*/
    virtual void initFilterKey() = 0;
    virtual void addFilterKey(const QString &key);

    /**@brief:获取其它设备信息*/
    void loadOtherDeviceInfo(const QMap<QString, QString> &mapInfo);

    /**@brief:将属性设置到成员变量*/
    void setAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite = true);
protected:
    QMap<QString, QString>  m_MapOtherInfo;         //<! 其它信息
    QSet<QString>          m_FilterKey;             //<! 用于避免添加重复信息
};
#endif // DEVICEINFO_H
