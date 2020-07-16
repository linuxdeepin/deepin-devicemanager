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
#include <QDomDocument>
#include <QFile>

#include "document.h"
#include "xlsxdocument.h"
#include "table.h"

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
    //const QMap<QString, QString> &getOtherAttribs()const;
    const QList<QPair<QString, QString>> &getOtherAttribs();

    /**
     * @brief getBaseAttribs::获取基本设备信息
     * @return 基本信息组成的list
     */
    const QList<QPair<QString, QString>> &getBaseAttribs();

    /**
     * @brief getTableHeader : 用于存放表格的头部
     * @return : 用于存放表格的头部
     */
    const QStringList &getTableHeader();

    /**
     * @brief getTableData ：获取表头数据
     * @return 返回表头数据
     */
    const QStringList &getTableData();

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

    /**
     * @brief toHtmlString:导出信息为html格式
     * @param doc xml格式文本
     */
    void toHtmlString(QDomDocument &doc);

    /**
     * @brief baseInfoToHTML:基本信息导出html
     * @param doc:xml格式文本
     * @param infoLst:信息列表
     */
    void baseInfoToHTML(QDomDocument &doc, QList<QPair<QString, QString>> &infoLst);

    /**
     * @brief subTitleToHTML:子标题导出
     * @param doc:xml格式文本
     */
    void subTitleToHTML(QDomDocument &doc);

    /**
     * @brief toDocString:导出信息为html格式
     * @param doc:doc文档
     */
    void toDocString(Docx::Document &doc);

    /**
     * @brief baseInfoToDoc:基本信息导出doc
     * @param doc:doc文档
     * @param infoLst:信息列表
     */
    void baseInfoToDoc(Docx::Document &doc, QList<QPair<QString, QString>> &infoLst);

    /**
     * @brief toXlsxString:导出信息为xlsx格式
     * @param xlsx:xlsx文档
     * @param boldFontc:字体格式
     */
    void toXlsxString(QXlsx::Document &xlsx, QXlsx::Format &boldFont);

    /**
     * @brief baseInfoToXlsx:基本信息导出xlsx
     * @param xlsx:xlsx文档
     * @param boldFont:字体格式
     * @param infoLst:信息列表
     */
    void baseInfoToXlsx(QXlsx::Document &xlsx, QXlsx::Format &boldFont, QList<QPair<QString, QString>> &infoLst);

    /**
     * @brief toTxtString:导出信息为txt格式
     * @param out:文本输出流
     */
    void toTxtString(QTextStream &out);

    /**
     * @brief baseInfoToTxt:导出信息为txt格式
     * @param out:文本输出流
     * @param infoLst:信息列表
     */
    void baseInfoToTxt(QTextStream &out, QList<QPair<QString, QString>> &infoLst);

    void tableInfoToTxt(QTextStream &out);
    void tableHeaderToTxt(QTextStream &out);

    void tableInfoToHtml(QFile &html);
    void tableHeaderToHtml(QFile &html);

    void tableInfoToDoc(Docx::Table *tab, int &row);
    void tableHeaderToDoc(Docx::Table *tab);
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
     * @brief loadOtherDeviceInfo:获取其它设备信息
     */
    virtual void loadOtherDeviceInfo() = 0;

    /**
     * @brief loadTableHeader : 过去表格的表头数据
     */
    virtual void loadTableHeader();

    /**
     * @brief loadTableData : 获取表格数据信息
     */
    virtual void loadTableData() = 0;

    /**
     * @brief addFilterKey:添加过滤信息
     * @param key:过滤信息的字符串
     */
    virtual void addFilterKey(const QString &key);

    /**
     * @brief loadOtherDeviceInfo:加载其它设备信息
     * @param mapInfo:其他设备信息map
     */
    void getOtherMapInfo(const QMap<QString, QString> &mapInfo);

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
    QList<QPair<QString, QString>> m_LstBaseInfo;   //<! 基本信息
    QList<QPair<QString, QString>> m_LstOtherInfo;  //<! 其它信息
    QStringList                    m_TableHeader;   //<! 用于存放表格的表头
    QStringList                    m_TableData;     //<! 用于存放表格的内容
    QSet<QString>                  m_FilterKey;     //<! 用于避免添加重复信息
    QString                        m_SubTitle;

private:
    QMap<QString, QString>  m_MapOtherInfo;         //<! 其它信息
};
#endif // DEVICEINFO_H
