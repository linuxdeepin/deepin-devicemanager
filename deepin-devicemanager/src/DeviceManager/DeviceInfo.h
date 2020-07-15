/**
* @brief: 设备信息的基类
* @author: liujun
* @date:2020-04-09
*/
#ifndef DEVICEINFO_H
#define DEVICEINFO_H
#include <QString>
#include <QMap>
#include <QSet>
#include <QObject>
#include <QList>
#include <QPair>

class DeviceBaseInfo : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceBaseInfo)
public:
    explicit DeviceBaseInfo(QObject *parent = nullptr);
    virtual ~DeviceBaseInfo();

    /**
     * @brief getOtherAttribs:获取设备的其它信息
     * @return 其他信息组成的map
     */
    const QMap<QString, QString> &getOtherAttribs()const;

    /**
     * @brief getBaseAttribs::获取基本设备信息
     * @return 基本信息组成的list
     */
    const QList<QPair<QString, QString>> &getBaseAttribs();

    /**
     * @brief overviewStr:获取概况
     * @return 概况
     */
    const QString &overviewStr()const;

    /**
     * @brief subTitle:获取子标题
     * @return 子标题
     */
    const QString &subTitle()const;

    /**
     * @brief isValid：判断属性值是否有效
     * @param value：属性值
     * @return true:属性值有效
     */
    bool isValueValid(QString &value);

protected:
    /**
     * @brief:初始化过滤信息
    */
    virtual void initFilterKey() = 0;

    /**
     * @brief loadBaseDeviceInfo:加载基本信息
     */
    virtual void loadBaseDeviceInfo() = 0;
    /**
     * @brief addFilterKey:添加过滤信息
     * @param key:过滤信息的字符串
     */
    virtual void addFilterKey(const QString &key);

    /**
     * @brief loadOtherDeviceInfo:加载其它设备信息
     * @param mapInfo:其他设备信息map
     */
    void loadOtherDeviceInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief addBaseDeviceInfo:添加基本信息
     * @param key:属性名称
     * @param value:属性值
     */
    void addBaseDeviceInfo(const QString &key, const QString &value);

    /**@brief:将属性设置到成员变量*/
    /**
     * @brief setAttribute:将属性设置到成员变量
     * @param mapInfo:设备信息map
     * @param key:属性名称
     * @param variable:设备类成员变量
     * @param overwrite:是否覆盖
     */
    void setAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite = true);

protected:
    QList<QPair<QString, QString>> m_LstBaseInfo;//<! 基本信息
    QMap<QString, QString>  m_MapOtherInfo;         //<! 其它信息
    QSet<QString>          m_FilterKey;             //<! 用于避免添加重复信息

    QString                m_SubTitle;
    QString                m_OverviewStr;
};
#endif // DEVICEINFO_H
