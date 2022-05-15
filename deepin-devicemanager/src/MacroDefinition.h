/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
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

#pragma once
#include <QString>
#include <QList>

/**@brief:定义一个删除指针的宏*/
#define DELETE_PTR(p) \
    if (p) { \
        delete p; \
        p = nullptr; \
    } \

/**
 * @brief EXPORT_TO_TXT:导出设备信息到txt
 * @param htmlFile:txt文件
 * @param deviceLst:设备列表
 * @param type:设备类型
 * @param msg:没有设备时的提示信息
 */
#define EXPORT_TO_TXT(out, deviceLst, type, msg) \
    /**添加设备类型**/                                                                \
    out <<  "[" << type << "]\n-------------------------------------------------";  \
    \
    /**无设备添加提示信息**/                                                           \
    if (deviceLst.size() < 1) {                                                     \
        out << "\n";                                                                \
        out << msg;                                                                 \
    }                                                                               \
    \
    /**添加Table信息**/                                                              \
    if (deviceLst.size() > 1) {                                                     \
        deviceLst[0]->tableHeaderToTxt(out);                                        \
        foreach (auto device, deviceLst) {                                          \
            device->tableInfoToTxt(out);                                            \
        }                                                                           \
    }                                                                               \
    \
    /**添加每个设备的信息**/\
    foreach (auto device, deviceLst) {                                              \
        out << "\n";                                                                \
        \
        device->getBaseAttribs();                                                   \
        device->getOtherAttribs();                                                  \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size() > 1) {                                                 \
            out << device->subTitle();                                              \
            out << "\n";                                                            \
        }                                                                           \
        \
        /**添加设备的详细信息**/                                                       \
        device->toTxtString(out);\
    }                                                                               \
    \
    out << "\n";                                                                    \


/**
 * @brief EXPORT_TO_XLSX:导出设备信息到xlsx
 * @param boldFont:字体格式
 * @param htmlFile:xlsx文件
 * @param deviceLst:设备列表
 * @param type:设备类型
 * @param msg:没有设备时的提示信息
 */
#define EXPORT_TO_XLSX(xlsx, boldFont, deviceLst, type, msg)                        \
    /**添加设备类型**/                                                                \
    boldFont.setFontBold(true);                                                     \
    xlsx.write(m_CurrentXlsRow++, 1, "[" + type + "]", boldFont);                   \
    \
    /**无设备添加提示信息**/                                                           \
    if (deviceLst.size() < 1) {                                                     \
        xlsx.write(m_CurrentXlsRow++, 1, msg, boldFont);                            \
    }                                                                               \
    \
    /**添加Table信息**/                                                              \
    if (deviceLst.size() > 1) {                                                     \
        deviceLst[0]->tableHeaderToXlsx(xlsx);                                      \
        foreach (auto device, deviceLst) {                                          \
            device->tableInfoToXlsx(xlsx);                                          \
        }                                                                           \
    }                                                                               \
    \
    /**添加每个设备的信息**/                                                           \
    foreach (auto device, deviceLst) {                                              \
        device->getBaseAttribs();                                                   \
        device->getOtherAttribs();                                                  \
        \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size() > 1) {                                                 \
            boldFont.setFontSize(10);                                               \
            boldFont.setFontBold(true);                                             \
            \
            xlsx.write(m_CurrentXlsRow++, 1, device->subTitle(), boldFont);         \
        }                                                                           \
        \
        /**添加设备的详细信息**/                                                       \
        device->toXlsxString(xlsx, boldFont);                                       \
        m_CurrentXlsRow++;\
    }                                                                               \
    \

/**
 * @brief EXPORT_TO_DOC:导出设备信息到doc
 * @param htmlFile:doc文件
 * @param deviceLst:设备列表
 * @param type:设备类型
 * @param msg:没有设备时的提示信息
 */
#define EXPORT_TO_DOC(doc, deviceLst, type, msg)                                    \
    /**添加设备类型**/                                                                \
    doc.addHeading("[" + type + "]", 2);                                            \
    doc.addParagraph("-------------------------------------------------");          \
    \
    /**无设备添加提示信息**/                                                           \
    if (deviceLst.size() < 1) {                                                     \
        doc.addParagraph(msg);                                                      \
    }                                                                               \
    \
    /**添加表格信息**/\
    if (deviceLst.size() > 1) {\
        Docx::Table *tab = doc.addTable(1,0);\
        deviceLst[0]->tableHeaderToDoc(tab);\
        for (int row = 1; row <= deviceLst.size(); ++row) {\
            tab->addRow();\
            deviceLst[row - 1]->tableInfoToDoc(tab, row);\
        }\
    }\
    /**添加每个设备的信息**/                                                           \
    foreach (auto device, deviceLst) {                                              \
        \
        device->getBaseAttribs();                                                   \
        device->getOtherAttribs();                                                  \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size() > 1) {                                                 \
            doc.addParagraph(device->subTitle());                                   \
        }                                                                           \
        \
        /**添加设备的详细信息**/                                                       \
        device->toDocString(doc);                                                   \
        \
        doc.addParagraph("\n");                                                     \
    }                                                                               \
    \

/**
 * @brief EXPORT_TO_HTML:导出设备信息到html
 * @param htmlFile:html文件
 * @param deviceLst:设备列表
 * @param type:设备类型
 * @param msg:没有设备时的提示信息
 */
#define EXPORT_TO_HTML(htmlFile, deviceLst, type, msg)                              \
    \
    /**添加设备类型**/                                                                \
    htmlFile.write((QString("<h2>") + "[" + type + "]" + "</h2>").toUtf8());        \
    \
    /**无设备添加提示信息**/                                                           \
    if (deviceLst.size() < 1) {                                                     \
        htmlFile.write((QString("<h2>") + msg + "</h2>").toUtf8());                 \
    }                                                                               \
    \
    /**添加表格信息**/\
    if(deviceLst.size() > 1) {\
        html.write("<table border=\"0\" white-space:pre>\n");\
        deviceLst[0]->tableHeaderToHtml(html);\
        foreach(auto device, deviceLst) {\
            device->tableInfoToHtml(html);\
        }\
        html.write("</table>\n");\
    }\
    \
    /**添加每个设备的信息**/                                                           \
    foreach (auto device, deviceLst) {                                              \
        QDomDocument doc;                                                           \
        \
        device->getBaseAttribs();                                                   \
        device->getOtherAttribs();                                                  \
        /**设备数目大于1，添加子标题**/                                                 \
        if (deviceLst.size() > 1) {                                                     \
            device->subTitleToHTML(doc);                                            \
        }                                                                           \
        \
        /**添加设备的详细信息**/                                                       \
        device->toHtmlString(doc);\
        doc.appendChild(doc.createElement("br"));                                   \
        \
        htmlFile.write(doc.toString().toStdString().data());                        \
        \
    }                                                                               \
    \


//HeaderTableView的相关宏定义
#define   TABLE_HEIGHT       180   // Table的高度
#define   ROW_HEIGHT         40    // 每一行的高度
#define   LINE_WIDTH         1     // 表头的线宽
#define   ROW_NUM            4     // TreeWidget 默认行数
#define   TREE_ROW_HEIGHT    36    // TreeWidget 行高
#define   WIDGET_MARGIN      2     // 页边距
#define   BOTTOM_MARGIN      11    // 下边距
#define   LABEL_MARGIN       12    // 页面标签的上下边距
#define   HORSCROLL_WIDTH    11    // TreeWidget 横向滚动条高度
#define   MIN_NUM            5     // num

const QString LINK_STR = "<a style=\"text-decoration:none\" href=https://www.chinauos.com/home>";        // uos官网链接
const QString DEEPIN_LINK = "<a style=\"text-decoration:none\" href=https://www.deepin.org/zh>";       // 社区版链接
const QString END_STR = " </a>";                                                                         // end html


// 驱动管理公用数据结构

/**
 * @brief The DDeviceType enum 可更新驱动类型
 */
enum DriverType {
    DR_Null = 0,
    DR_Bluetooth = 1,
    DR_Camera = 2,
    DR_Gpu = 3,
    DR_Keyboard = 4,
    DR_Sound = 5,
    DR_Mouse = 6,
    DR_Network = 7,
    DR_Printer = 8,
    DR_Scaner = 9,
    DR_Tablet = 10,
    DR_WiFi = 11
};

/**
 * @brief STR_DRIVER_INFO 从仓库查询驱动信息
 */
typedef struct STR_PPDS {
    QString strSource;
    QString strDesc;
    QString strManufacturer;
} strPpds;

typedef struct STR_DRIVER_INFO {
    QString strManufacturer;//厂商
    QString strDebManufacturer;//包厂商
    QString strArch;//架构
    QString strVersion;//驱动版本
    QString strDebVersion;//包版本
    QString strPackages;//包名
    QString strClass_p;//父级(驱动分类)
    QString strClass;//驱动分类
    QStringList strModels;//设备型号
    QString strProducts;//包设备型号
    QString strDeb;//包URL地址
    int iLevel;//
    QString strSystem;//
    QString strDesc;//
    QString strAdaptation;//
    QString strSource;//
    QString strDownloadUrl;//
    QString strSize;//包大小

    QList<strPpds> lstPpds;//Ppd文件
} strDriverInfo;
/**
 * @brief The Status enum 驱动状态
 */
enum Status {
    ST_SUCESS      = 0,   // 成功了
    ST_FAILED      = 1,   // 失败了
    ST_DOWNLOADING = 2,   // 下载中
    ST_INSTALL     = 3,   // 安装中
    ST_NOT_INSTALL = 4,   // 驱动未安装
    ST_CAN_UPDATE  = 5,   // 驱动可更新
    ST_WAITING     = 6,   // 等待中
    ST_NetWorkErr = 7,    // 网络异常
    ST_DRIVER_IS_NEW = 8, // 此驱动不需要更新
};

/**
 * @brief The ScanResult enum 驱动扫描结果
 */
enum ScanResult {
    SR_Failed      = 0,
    SR_SUCESS      = 1,
    SR_NETWORD_ERR = 2
};

struct DriverInfo {

    DriverInfo()
        : m_Type(DR_Null)
        , m_Name("")
        , m_VendorId("")
        , m_VendorName("")
        , m_ModelId("")
        , m_ModelName("")
        , m_DriverName("")
        , m_Version("")
        , m_Size("")
        , m_Status(ST_DRIVER_IS_NEW)
        , m_Checked(false)
        , m_DebVersion("")
        , m_Packages("")
    {

    }

    DriverType m_Type;         //
    QString    m_Name;         //
    QString    m_VendorId;     // 板载用
    QString    m_VendorName;   // 打印机用
    QString    m_ModelId;      // 板载用
    QString    m_ModelName;    // 打印机用
    QString    m_DriverName;   // 返回值
    QString    m_Version;
    QString    m_Size;         // 返回值
    Status     m_Status;       // 检测后要更新
    bool       m_Checked;      //

    QString    m_DebVersion;   //包版本 返回值
    QString    m_Packages;     //包名  返回值

    DriverType type() { return m_Type; }
    QString    name() { return m_Name; }
    QString    vendorId() { return m_VendorId; }
    QString    vendorName() { return m_VendorName; }
    QString    modelId() { return m_ModelId; }
    QString    modelName() { return m_ModelName; }
    QString    driverName() {return m_DriverName; }
    QString    version() {return m_Version; }
    QString    size() { return m_Size; }
    Status     status() { return m_Status; }
    bool       checked() { return m_Checked; }
    QString    debVersion() { return m_DebVersion; }
    QString    packages() {return m_Packages; }
};


//enum ErrorCode {
//    EC_NULL = 0,
//    EC_NETWORK = 1,
//    EC_CANCEL = 2,
//    EC_3 = 3,
//    EC_4 = 4,
//    EC_5 = 5,
//    EC_6 = 6
//};

#define EC_NULL 0
#define EC_NETWORK 1
#define EC_CANCEL 2
#define EC_3 3
#define EC_4 4
#define EC_5 5
#define EC_6 6
