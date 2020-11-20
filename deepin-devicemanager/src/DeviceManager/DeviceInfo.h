/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
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

/**
 * @brief The EnableDeviceStatus enum
 * 用来标识对设备启用禁用的结果(包括成功、失败、取消三种情况)
 */
enum EnableDeviceStatus {
    EDS_Cancle,
    EDS_Faild,
    EDS_Success
};

/**
 * @brief The DeviceBaseInfo class
 * 各个摄像头描述类的基类
 */
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
    virtual QString subTitle();

    /**
     * @brief getOverviewInfo:获取概况信息
     * @return 概况信息
     */
    virtual const QString getOverviewInfo();

    /**
     * @brief name:获取设备名称
     * @return 设备名称
     */
    virtual const QString &name() const = 0;

    /**
     * @brief driver:获取设备驱动
     * @return 设备驱动
     */
    virtual const QString &driver() const = 0;

    /**
     * @brief setEnable 设置设备的禁用状态
     * @param enable 是否禁用
     * @return 禁用是否成功
     */
    virtual EnableDeviceStatus setEnable(bool enable);

    /**
     * @brief enable 获取设备的禁用状态
     * @return
     */
    virtual bool enable();

    /**
     * @brief setCanEnale : set can enable or not
     * @param can
     */
    void setCanEnale(bool can);

    /**
     * @brief canEnable : 该设备是否可以禁用
     * @return
     */
    bool canEnable();

    /**
     * @brief setIndex
     * @param index
     */
    void setIndex(int index);

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

    /**
     * @brief tableInfoToTxt:表格内容写到txt
     * @param out:txt文件流
     */
    void tableInfoToTxt(QTextStream &out);

    /**
     * @brief tableHeaderToTxt:表头信息写到txt
     * @param out:txt文件流
     */
    void tableHeaderToTxt(QTextStream &out);

    /**
     * @brief tableInfoToHtml:表格内容写到html
     * @param html:html文档
     */
    void tableInfoToHtml(QFile &html);

    /**
     * @brief tableHeaderToHtml:表头信息写到html
     * @param html:html文档
     */
    void tableHeaderToHtml(QFile &html);

    /**
     * @brief tableInfoToDoc:表格信息写到doc
     * @param tab:doc文档表格
     * @param row:表格行数，第row行
     */
    void tableInfoToDoc(Docx::Table *tab, int &row);

    /**
     * @brief tableHeaderToDoc:表头信息写到doc
     * @param tab doc文档的表格
     */
    void tableHeaderToDoc(Docx::Table *tab);

    /**
     * @brief tableInfoToXlsx:表格信息写到xlsx
     * @param xlsx xlsx文件
     */
    void tableInfoToXlsx(QXlsx::Document &xlsx);

    /**
     * @brief tableHeaderToXlsx:表头信息写到xlsx
     * @param xlsx xlsx文件
     */
    void tableHeaderToXlsx(QXlsx::Document &xlsx);

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

    /**
     * @brief addOtherDeviceInfo:添加其他信息信息
     * @param key:属性名称
     * @param value:属性值
     */
    void addOtherDeviceInfo(const QString &key, const QString &value);

    /**@brief:将属性设置到成员变量*/
    /**
     * @brief setAttribute:将属性设置到成员变量
     * @param mapInfo:设备信息map
     * @param key:属性名称
     * @param variable:设备类成员变量
     * @param overwrite:是否覆盖
     */
    void setAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite = true);

    /**
     * @brief mapInfoToList:QMap转换为QList
     */
    void mapInfoToList();

protected:
    QList<QPair<QString, QString>> m_LstBaseInfo;   //<! 基本信息
    QList<QPair<QString, QString>> m_LstOtherInfo;  //<! 其它信息
    QStringList                    m_TableHeader;   //<! 用于存放表格的表头
    QStringList                    m_TableData;     //<! 用于存放表格的内容
    QSet<QString>                  m_FilterKey;     //<! 用于避免添加重复信息
    bool                           m_Enable;        //<! 设备是否是启用状态
    bool                           m_CanEnable;     //<! 设备是否可以启用禁用
    int                            m_Index;         //<! 同名设备的索引

private:
    QMap<QString, QString>  m_MapOtherInfo;         //<! 其它信息
};
#endif // DEVICEINFO_H
