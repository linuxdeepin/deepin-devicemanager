// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceInfo.h"
#include "commonfunction.h"
#include "commondefine.h"
#include"DeviceManager.h"
#include "DDLog.h"

#include <DApplication>

#include <QLoggingCategory>
#include <QProcess>
#include <QMap>
#include <QRegularExpression>
using namespace DDLog;

DWIDGET_USE_NAMESPACE

DeviceBaseInfo::DeviceBaseInfo(QObject *parent)
    : QObject(parent)
    , m_Name("")
    , m_Vendor("")
    , m_UniqueID("")
    , m_SerialID("")
    , m_SysPath("")
    , m_HardwareClass("")
    , m_HwinfoToLshw("")
    , m_VID_PID("")
    , m_Modalias("")
    , m_Enable(true)
    , m_CanEnable(false)
    , m_CanUninstall(false)
    , m_Available(true)
    , m_Index(0)
    , m_forcedDisplay(false)
    , m_Driver("")
{
    qCDebug(appLog) << "DeviceBaseInfo constructor initialized.";
}

DeviceBaseInfo::~DeviceBaseInfo()
{
    qCDebug(appLog) << "DeviceBaseInfo destructor called.";
}

const QString DeviceBaseInfo::translateStr(const QString &inStr)
{
    return tr(inStr.toLatin1());
}

const QString DeviceBaseInfo::nameTr()
{
    return translateStr(m_Name);
}

const QList<QPair<QString, QString>> &DeviceBaseInfo::getOtherAttribs()
{
    qCDebug(appLog) << "DeviceBaseInfo::getOtherAttribs called.";
    // 获取其他设备信息列表
    m_LstOtherInfo.clear();
    loadOtherDeviceInfo();
    qCDebug(appLog) << "Other attributes loaded. Count: " << m_LstOtherInfo.count();
    return m_LstOtherInfo;
}

const QList<QPair<QString, QString> > &DeviceBaseInfo::getBaseAttribs()
{
    qCDebug(appLog) << "DeviceBaseInfo::getBaseAttribs called.";
    // 获取基本信息列表
    m_LstBaseInfo.clear();
    loadBaseDeviceInfo();
    qCDebug(appLog) << "Base attributes loaded. Count: " << m_LstBaseInfo.count();
    return m_LstBaseInfo;
}

const QList<QPair<QString, QString> > &DeviceBaseInfo::getOtherTranslationAttribs()
{
    m_LstOtherInfoTr.clear();
    getOtherAttribs();
    for (const auto &pair : m_LstOtherInfo) {
        QString trKey =translateStr(pair.first);
        if (trKey.isEmpty())
            m_LstOtherInfoTr.append(qMakePair(pair.first, pair.second)); // 添加到目标列表
        else
            m_LstOtherInfoTr.append(qMakePair(trKey, pair.second));
    }
    return m_LstOtherInfoTr;
}

const QList<QPair<QString, QString> > &DeviceBaseInfo::getBaseTranslationAttribs()
{
    m_LstBaseInfoTr.clear();
    getBaseAttribs();
    for (const auto &pair : m_LstBaseInfo) {
        QString trKey = translateStr(pair.first);
        if (trKey.isEmpty())
            m_LstBaseInfoTr.append(qMakePair(pair.first, pair.second)); // 添加到目标列表
        else
            m_LstBaseInfoTr.append(qMakePair(trKey, pair.second));
    }
    return m_LstBaseInfoTr;
}

const QStringList &DeviceBaseInfo::getTableHeader()
{
    m_TableHeaderTr.clear();
    qCDebug(appLog) << "DeviceBaseInfo::getTableHeader called.";
    // 获取表头
    if (m_TableHeader.size() == 0) {
        qCDebug(appLog) << "Table header is empty, loading.";
        loadTableHeader();
        m_TableHeader.append(m_CanEnable ? "yes" : "no");
    }

    for (const auto &item : m_TableHeader) {
        QString trKey = tr(item.toLatin1());
        if (trKey.isEmpty())
            m_TableHeaderTr.append(item);
        else
            m_TableHeaderTr.append(trKey);
    }

    qCDebug(appLog) << "Table header: " << m_TableHeader;
    return m_TableHeaderTr;
}

const QStringList &DeviceBaseInfo::getTableData()
{
    qCDebug(appLog) << "DeviceBaseInfo::getTableData called.";
    // 获取表格数据
    m_TableData.clear();
    loadTableData();
    m_TableDataTr.clear();
    for (const auto &item : m_TableData) {
        QString trKey = translateStr(item);
        if(trKey.isEmpty())
            m_TableDataTr.append(item);
        else
            m_TableDataTr.append(trKey);
    }
    qCDebug(appLog) << "Table data loaded. Count: " << m_TableData.count();
    return m_TableDataTr;
}

QString DeviceBaseInfo::subTitle()
{
    qCDebug(appLog) << "DeviceBaseInfo::subTitle called, returning empty string as default.";
    return QString("");
}

bool DeviceBaseInfo::isValueValid(QString &value)
{
    // qCDebug(appLog) << "DeviceBaseInfo::isValueValid called for value: " << value;
    // 判断属性值是否有效
    if (value.isEmpty()) {
        // qCDebug(appLog) << "Value is empty, returning false.";
        return false;
    }

    if (value == QObject::tr("Unknown")) {
        // qCDebug(appLog) << "Value is 'Unknown', returning false.";
        return false;
    }

    if (value == QString("Unknown")) {
        // qCDebug(appLog) << "Value is 'Unknown' (QString), returning false.";
        return false;
    }

    if (value.compare(QString("N/A"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is 'N/A', returning false.";
        return false;
    }

    if (value == QString("")) {
        // qCDebug(appLog) << "Value is empty (QString), returning false.";
        return false;
    }

    if (value.compare(QString("Null"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is 'Null', returning false.";
        return false;
    }

    if (value.compare(QString("none"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is 'none', returning false.";
        return false;
    }

    if (value.compare(QString("Not Provided"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is 'Not Provided', returning false.";
        return false;
    }

    if (value.compare(QString("Not Specified"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is 'Not Specified', returning false.";
        return false;
    }

    if (value.compare(QString("Default string"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is 'Default string', returning false.";
        return false;
    }

    if (value.compare(QString("Unspecified"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is 'Unspecified', returning false.";
        return false;
    }

    if (value.compare(QString("Not Present"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is 'Not Present', returning false.";
        return false;
    }

    if (value.compare(QString("<OUT OF SPEC>"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is '<OUT OF SPEC>', returning false.";
        return false;
    }

    if (value.compare(QString("Other"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is 'Other', returning false.";
        return false;
    }

    if (value.compare(QString("TBD"), Qt::CaseInsensitive) == 0) {
        // qCDebug(appLog) << "Value is 'TBD', returning false.";
        return false;
    }
    // qCDebug(appLog) << "Value is valid, returning true.";
    return true;
}

void DeviceBaseInfo::setForcedDisplay(const bool &flag)
{
    qCDebug(appLog) << "DeviceBaseInfo::setForcedDisplay called with flag: " << flag;
    m_forcedDisplay = flag;
    qCDebug(appLog) << "m_forcedDisplay set to: " << m_forcedDisplay;
}

void DeviceBaseInfo::toHtmlString(QDomDocument &doc)
{
    qCDebug(appLog) << "DeviceBaseInfo::toHtmlString called.";
    // 设备信息转为Html
    baseInfoToHTML(doc, m_LstBaseInfoTr);
    baseInfoToHTML(doc, m_LstOtherInfoTr);
    qCDebug(appLog) << "HTML string conversion finished.";
}

void DeviceBaseInfo::baseInfoToHTML(QDomDocument &doc, QList<QPair<QString, QString> > &infoLst)
{
    qCDebug(appLog) << "DeviceBaseInfo::baseInfoToHTML called for info list.";
    // 设备信息转为HTML
    QDomElement table = doc.createElement("table");
    table.setAttribute("border", "0");
    table.setAttribute("width", "100%");
    table.setAttribute("cellpadding", "3");
    qCDebug(appLog) << "HTML table element created.";

    // 添加HTML表格内容
    foreach (auto info, infoLst) {
        if (isValueValid(info.second)) {
            // qCDebug(appLog) << "Adding valid info to HTML table: " << info.first << ": " << info.second;
            QDomElement tr = doc.createElement("tr");

            // 第一列
            QDomElement td = doc.createElement("td");
            td.setAttribute("width", "15%");
            td.setAttribute("style", "text-align:left;");

            QDomText nameText = doc.createTextNode(info.first + ": ");
            td.appendChild(nameText);
            tr.appendChild(td);

            // 第二列
            QDomElement td2 = doc.createElement("td");
            td2.setAttribute("width", "85%");

            QDomText valueText;
            valueText = doc.createTextNode(info.second);
            td2.appendChild(valueText);

            tr.appendChild(td2);

            table.appendChild(tr);
        }

        doc.appendChild(table);
    }
    qCDebug(appLog) << "Finished adding items to HTML table.";
}

void DeviceBaseInfo::subTitleToHTML(QDomDocument &doc)
{
    qCDebug(appLog) << "DeviceBaseInfo::subTitleToHTML called.";
    // 子标题转为HTML格式
    if (false == this->subTitle().isEmpty()) {
        qCDebug(appLog) << "Subtitle is not empty, adding to HTML: " << this->subTitle();
        QDomElement h3 = doc.createElement("h3");
        QDomText valueText = doc.createTextNode(this->subTitle());
        h3.appendChild(valueText);
        doc.appendChild(h3);
    } else {
        qCDebug(appLog) << "Subtitle is empty, skipping HTML addition.";
    }
}

void DeviceBaseInfo::toDocString(Docx::Document &doc)
{
    qCDebug(appLog) << "DeviceBaseInfo::toDocString called.";
    // 设备信息转为doc
    baseInfoToDoc(doc, m_LstBaseInfoTr);
    baseInfoToDoc(doc, m_LstOtherInfoTr);
    qCDebug(appLog) << "Doc string conversion finished.";
}

void DeviceBaseInfo::baseInfoToDoc(Docx::Document &doc, QList<QPair<QString, QString> > &infoLst)
{
    qCDebug(appLog) << "DeviceBaseInfo::baseInfoToDoc called for info list.";
    // 设备信息保存为Doc
    foreach (auto item, infoLst) {
        QString value = item.second;

        // 判断属性值是否有效
        if (false == isValueValid(value)) {
            // qCDebug(appLog) << "Skipping invalid info for Doc: " << item.first << ": " << item.second;
            continue;
        }

        // 添加doc段落
        QString line = item.first + ":  " + item.second;
        doc.addParagraph(line);
        // qCDebug(appLog) << "Added paragraph to Doc: " << line;
    }
    qCDebug(appLog) << "Finished adding items to Doc.";
}

void DeviceBaseInfo::toXlsxString(QXlsx::Document &xlsx, QXlsx::Format &boldFont)
{
    qCDebug(appLog) << "DeviceBaseInfo::toXlsxString called.";
    // 设备信息转为xlxs表格
    baseInfoToXlsx(xlsx, boldFont, m_LstBaseInfoTr);
    baseInfoToXlsx(xlsx, boldFont, m_LstOtherInfoTr);
    qCDebug(appLog) << "Xlsx string conversion finished.";
}

void DeviceBaseInfo::baseInfoToXlsx(QXlsx::Document &xlsx, QXlsx::Format &boldFont, QList<QPair<QString, QString> > &infoLst)
{
    qCDebug(appLog) << "DeviceBaseInfo::baseInfoToXlsx called for info list.";
    // 设置表格内容字体不加粗,字号10
    boldFont.setFontBold(false);
    boldFont.setFontSize(10);
    qCDebug(appLog) << "Xlsx format set: bold=" << boldFont.fontBold() << ", size=" << boldFont.fontSize();

    foreach (auto item, infoLst) {
        QString value = item.second;

        // 判断属性值是否有效
        if (false == isValueValid(value)) {
            // qCDebug(appLog) << "Skipping invalid info for Xlsx: " << item.first << ": " << item.second;
            continue;
        }

        // 获取行数
        int _row = DeviceManager::instance()->currentXlsRow();
        xlsx.write(_row, 1, item.first, boldFont);
        xlsx.write(_row, 2, item.second, boldFont);
        // qCDebug(appLog) << "Written to Xlsx at row " << _row << ": " << item.first << " = " << item.second;
    }
    qCDebug(appLog) << "Finished writing items to Xlsx.";
}

void DeviceBaseInfo::toTxtString(QTextStream &out)
{
    qCDebug(appLog) << "DeviceBaseInfo::toTxtString called.";
    // 设备信息转为txt
    baseInfoToTxt(out, m_LstBaseInfoTr);
    baseInfoToTxt(out, m_LstOtherInfoTr);
    qCDebug(appLog) << "Txt string conversion finished.";
}

void DeviceBaseInfo::baseInfoToTxt(QTextStream &out, QList<QPair<QString, QString> > &infoLst)
{
    qCDebug(appLog) << "DeviceBaseInfo::baseInfoToTxt called for info list.";
    foreach (auto item, infoLst) {
        QString value = item.second;

        // 判断属性值是否有效
        if (false == isValueValid(value)) {
            // qCDebug(appLog) << "Skipping invalid info for Txt: " << item.first << ": " << item.second;
            continue;
        }

        // 设置第一列占21个字符
        out.setFieldWidth(21);
        out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
        out << item.first + ": ";
        out.setFieldWidth(0);
        out << item.second;
        out << "\n";
        // qCDebug(appLog) << "Written to Txt: " << item.first << " = " << item.second;
    }
    qCDebug(appLog) << "Finished writing items to Txt.";
}

void DeviceBaseInfo::tableInfoToTxt(QTextStream &out)
{
    qCDebug(appLog) << "DeviceBaseInfo::tableInfoToTxt called.";
    // 获取表格内容
    getTableData();

    // 判断是否有表格内容
    if (m_TableDataTr.size() < 1) {
        qCDebug(appLog) << "Table data is empty, skipping Txt conversion.";
        return;
    }

    // 设置占位宽度
    QString text = m_TableDataTr[0];
    out.setFieldWidth(int(text.size() * 1.5));
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignRight);
    qCDebug(appLog) << "Txt field width set for table info.";

    foreach (auto item, m_TableDataTr) {
        out.setFieldWidth(28);
        out << item;
        // qCDebug(appLog) << "Written table item to Txt: " << item;
    }

    out.setFieldWidth(0);
    out << "\n";
    qCDebug(appLog) << "Finished writing table info to Txt.";
}

void DeviceBaseInfo::tableHeaderToTxt(QTextStream &out)
{
    qCDebug(appLog) << "DeviceBaseInfo::tableHeaderToTxt called.";
    // 获取表头
    getTableHeader();

    // 判断是否有表头
    if (m_TableHeaderTr.size() < 1) {
        qCDebug(appLog) << "Table header is empty, skipping Txt conversion.";
        return;
    }

    // 设置占位宽度
    QString text = m_TableHeaderTr[0];
    out.setFieldWidth(int(text.size() * 1.5));
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
    qCDebug(appLog) << "Txt field width set for table header.";

    out << "\n";
    for (int col = 0; col < m_TableHeaderTr.size() - 1; ++col) {
        out.setFieldWidth(30);
        out << m_TableHeaderTr[col];
        // qCDebug(appLog) << "Written table header item to Txt: " << m_TableHeaderTr[col];
    }
    out.setFieldWidth(0);
    out << "\n";
    qCDebug(appLog) << "Finished writing table header to Txt.";
}

void DeviceBaseInfo::tableInfoToHtml(QFile &html)
{
    qCDebug(appLog) << "DeviceBaseInfo::tableInfoToHtml called.";
    // 获取表格内容
    getTableData();

    // 判断是否有表格内容
    if (m_TableData.size() < 1) {
        qCDebug(appLog) << "Table data is empty, skipping HTML table info conversion.";
        return;
    }

    // 写表格内容
    foreach (auto item, m_TableData) {
        html.write(QString("<td style=\"width:200px;text-align:left;\">" + item + "</td>").toUtf8().data());
        // qCDebug(appLog) << "Written table info item to HTML: " << item;
    }

    html.write("</tr>\n");
    qCDebug(appLog) << "Finished writing table info to HTML.";
}

void DeviceBaseInfo::tableHeaderToHtml(QFile &html)
{
    qCDebug(appLog) << "DeviceBaseInfo::tableHeaderToHtml called.";
    // 获取表头信息
    getTableHeader();

    // 判断是否有表头
    if (m_TableHeaderTr.size() < 1) {
        qCDebug(appLog) << "Table header is empty, skipping HTML table header conversion.";
        return;
    }

    html.write("<thead><tr>\n");
    qCDebug(appLog) << "Written HTML table header start tags.";

    // 写表头内容
    for (int col = 0; col < m_TableHeaderTr.size() - 1; ++col) {
        html.write(QString("<th style=\"width:200px;text-align:left; white-space:pre;\">" + m_TableHeaderTr[col] + "</th>").toUtf8().data());
        // qCDebug(appLog) << "Written table header item to HTML: " << m_TableHeaderTr[col];
    }

    html.write("</tr></thead>\n");
    qCDebug(appLog) << "Finished writing table header to HTML.";
}

void DeviceBaseInfo::tableInfoToDoc(Docx::Table *tab, int &row)
{
    qCDebug(appLog) << "DeviceBaseInfo::tableInfoToDoc called with row: " << row;
    // 表格信息保存为Doc
    if (nullptr == tab) {
        qCDebug(appLog) << "Table pointer is null, returning.";
        return;
    }

    // 获取表格数据
    getTableData();

    if (m_TableDataTr.size() < 1) {
        qCDebug(appLog) << "Table data is empty, skipping Doc table info conversion.";
        return;
    }

    // 添加doc表格
    for (int col = 0; col < m_TableDataTr.size(); ++col) {
        auto cel = tab->cell(row, col);
        cel->addText(m_TableDataTr[col]);
        // qCDebug(appLog) << "Added table info to Doc cell (" << row << ", " << col << "): " << m_TableDataTr[col];
    }
    qCDebug(appLog) << "Finished adding table info to Doc.";
}

void DeviceBaseInfo::tableHeaderToDoc(Docx::Table *tab)
{
    qCDebug(appLog) << "DeviceBaseInfo::tableHeaderToDoc called.";
    // 表头保存为doc
    getTableHeader();

    if (m_TableHeaderTr.size() < 1) {
        qCDebug(appLog) << "Table header is empty, skipping Doc table header conversion.";
        return;
    }

    // 添加表头信息
    for (int col = 0; col < m_TableHeaderTr.size() - 1; ++col)  {
        tab->addColumn();
        auto cel = tab->cell(0, col);
        cel->addText(m_TableHeaderTr[col]);
        // qCDebug(appLog) << "Added table header to Doc cell (0, " << col << "): " << m_TableHeaderTr[col];
    }
    qCDebug(appLog) << "Finished adding table header to Doc.";
}

void DeviceBaseInfo::tableInfoToXlsx(QXlsx::Document &xlsx)
{
    qCDebug(appLog) << "DeviceBaseInfo::tableInfoToXlsx called.";
    // 获取表格信息
    getTableData();

    if (m_TableDataTr.size() < 1) {
        qCDebug(appLog) << "Table data is empty, skipping Xlsx table info conversion.";
        return;
    }

    // 添加表格信息
    int curRow = DeviceManager::instance()->currentXlsRow();
    for (int col = 0; col < m_TableDataTr.size(); ++col) {
        xlsx.write(curRow, col + 1, m_TableDataTr[col]);
        // qCDebug(appLog) << "Written table info to Xlsx at row " << curRow << ", col " << col + 1 << ": " << m_TableDataTr[col];
    }
    qCDebug(appLog) << "Finished writing table info to Xlsx.";
}

void DeviceBaseInfo::tableHeaderToXlsx(QXlsx::Document &xlsx)
{
    qCDebug(appLog) << "DeviceBaseInfo::tableHeaderToXlsx called.";
    // 获取表头
    getTableHeader();

    if (m_TableHeaderTr.size() < 1) {
        qCDebug(appLog) << "Table header is empty, skipping Xlsx table header conversion.";
        return;
    }

    // 添加表头信息
    int curRow = DeviceManager::instance()->currentXlsRow();
    for (int col = 0; col < m_TableHeaderTr.size() - 1; ++col) {
        QXlsx::Format boldFont;
        boldFont.setFontSize(10);
        boldFont.setFontBold(true);
        xlsx.write(curRow, col + 1, m_TableHeaderTr[col], boldFont);
        // qCDebug(appLog) << "Written table header to Xlsx at row " << curRow << ", col " << col + 1 << ": " << m_TableHeaderTr[col];
    }
    qCDebug(appLog) << "Finished writing table header to Xlsx.";
}

void DeviceBaseInfo::setOtherDeviceInfo(const QString &key, const QString &value)
{
    // qCDebug(appLog) << "DeviceBaseInfo::setOtherDeviceInfo called for key: " << key << ", value: " << value;
    m_MapOtherInfo[key] = value;
}

TomlFixMethod DeviceBaseInfo::setInfoFromTomlBase(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBaseInfo::setInfoFromTomlBase started.";
    TomlFixMethod ret = TOML_None;
    TomlFixMethod ret1 = TOML_None;
    TomlFixMethod ret2 = TOML_None;

    if (TOML_nouse == setTomlAttribute(mapInfo, "Modalias", m_Modalias)) {
        qCInfo(appLog) << "toml del the device by modalias";
        return TOML_Del;
    }
    ret1 = setTomlAttribute(mapInfo,  "Vendor_ID", m_VID);
    ret2 = setTomlAttribute(mapInfo, "Product_ID", m_PID);
    if ((TOML_nouse == ret1) && (TOML_nouse == ret2)) {
        qCInfo(appLog) << "toml del the device by vid and pid";
        return TOML_Del;
    }
    ret1 = setTomlAttribute(mapInfo,  "Vendor", m_Vendor);
    ret2 = setTomlAttribute(mapInfo, "Name", m_Name);
    if ((TOML_nouse == ret1) && (TOML_nouse == ret2)) {
        qCInfo(appLog) << "toml del the device by vendor and name";
        return TOML_Del;
    }

    ret2 = setTomlAttribute(mapInfo, "Revision", m_Version);
    ret2 = setTomlAttribute(mapInfo, "Description", m_Description);
    m_VID = m_VID.toLower();
    m_PID = m_PID.toLower();
    m_VID_PID = m_VID + m_PID.remove("0x");
    m_PhysID = m_VID_PID;
    m_forcedDisplay = true;
    return ret;
}

EnableDeviceStatus DeviceBaseInfo::setEnable(bool e)
{
    // qCDebug(appLog) << "DeviceBaseInfo::setEnable called with status: " << e;
    return EDS_Success;
}

bool DeviceBaseInfo::enable()
{
    // qCDebug(appLog) << "DeviceBaseInfo::enable called, returning current enable status: " << m_Enable;
    return m_Enable;
}

bool DeviceBaseInfo::available()
{
    // qCDebug(appLog) << "DeviceBaseInfo::available called.";
    if (driver().isEmpty()) {
        // qCDebug(appLog) << "Driver is empty, setting m_Available to false.";
        m_Available = false;
    }
    // qCDebug(appLog) << "Returning available status: " << (m_forcedDisplay ? m_forcedDisplay : m_Available);
    return m_forcedDisplay ? m_forcedDisplay : m_Available;
}

bool DeviceBaseInfo::driverIsKernelIn(const QString &driver)
{
    qCDebug(appLog) << "DeviceBaseInfo::driverIsKernelIn called for driver: " << driver;
    // 驱动为空情况:
    // 1. 驱动被卸载了 此时驱动属于核外驱动
    // 2. ps/2 笔记本触摸板 暂无法获取驱动 此时当成核内驱动处理
    // 3. 但是由于判断是否是ps/2或者笔记本触摸板在子类判断(无需放在基类)，因此此处为空时先返回false，而在子类(DeviceInput)调用后判断是否是ps/2鼠标
    if (driver.isEmpty()) {
        qCDebug(appLog) << "Driver is empty, returning false.";
        return false;
    }

    // 英伟达驱动无法获取驱动模块，但是不属于核内驱动
    if ("nvidia" == driver) {
        qCDebug(appLog) << "Driver is 'nvidia', returning false.";
        return false;
    }

    // 判断lsmod是否能查询
    QString outInfo = Common::executeClientCmd("modinfo", QStringList() << driver, QString(), -1);
    bool isKernelIn = !outInfo.contains("filename:");
    qCDebug(appLog) << "Driver: " << driver << ", modinfo output contains filename: " << !isKernelIn << ", returning: " << isKernelIn;
    return isKernelIn;
}

void DeviceBaseInfo::setCanEnale(bool can)
{
    // qCDebug(appLog) << "DeviceBaseInfo::setCanEnale called with can: " << can;
    m_CanEnable = can;
    // qCDebug(appLog) << "m_CanEnable set to: " << m_CanEnable;
}

bool DeviceBaseInfo::canEnable()
{
    // qCDebug(appLog) << "DeviceBaseInfo::canEnable called, returning: " << m_CanEnable;
    return m_CanEnable;
}

void DeviceBaseInfo::setEnableValue(bool e)
{
    // qCDebug(appLog) << "DeviceBaseInfo::setEnableValue called with e: " << e;
    m_Enable = e;
    // qCDebug(appLog) << "m_Enable set to: " << m_Enable;
}

bool DeviceBaseInfo::canUninstall()
{
    // qCDebug(appLog) << "DeviceBaseInfo::canUninstall called, returning: " << m_CanUninstall;
    return m_CanUninstall;
}

void DeviceBaseInfo::setCanUninstall(bool can)
{
    // qCDebug(appLog) << "DeviceBaseInfo::setCanUninstall called with can: " << can;
    m_CanUninstall = can;
    // qCDebug(appLog) << "m_CanUninstall set to: " << m_CanUninstall;
}

void DeviceBaseInfo::setHardwareClass(const QString &hclass)
{
    // qCDebug(appLog) << "DeviceBaseInfo::setHardwareClass called with hclass: " << hclass;
    m_HardwareClass = hclass;
    // qCDebug(appLog) << "m_HardwareClass set to: " << m_HardwareClass;
}

const QString &DeviceBaseInfo::hardwareClass() const
{
    // qCDebug(appLog) << "DeviceBaseInfo::hardwareClass called, returning: " << m_HardwareClass;
    return m_HardwareClass;
}

const QString &DeviceBaseInfo::uniqueID() const
{
    // qCDebug(appLog) << "DeviceBaseInfo::uniqueID called, returning: " << m_UniqueID;
    return m_UniqueID;
}

const QString &DeviceBaseInfo::sysPath() const
{
    // qCDebug(appLog) << "DeviceBaseInfo::sysPath called, returning: " << m_SysPath;
    return m_SysPath;
}

const QString DeviceBaseInfo::getVendorOrModelId(const QString &sysPath, bool flag)
{
    qCDebug(appLog) << "DeviceBaseInfo::getVendorOrModelId called with sysPath: " << sysPath << ", flag: " << flag;
    // 从文件中获取制造商ID信息
    QFile vendorFile;
    QString strVendorFile("/vendor");
    QString strDeviceFile("/device");
    QString strSysFSLink = sysPath;

    if (sysPath.contains("usb")) {
        qCDebug(appLog) << "SysPath contains 'usb', adjusting file names and sys path link.";
        strVendorFile = "/idVendor";
        strDeviceFile = "/idProduct";
        if (!QFile::exists("/sys" + strSysFSLink + strVendorFile)) {
            qCDebug(appLog) << "Vendor file does not exist, adjusting sys path link.";
            strSysFSLink = strSysFSLink.mid(0, sysPath.lastIndexOf('/'));
        }
    }

    if (flag) {
        qCDebug(appLog) << "Flag is true, setting file name for vendor: " << QString("/sys") + strSysFSLink + strVendorFile;
        vendorFile.setFileName(QString("/sys") + strSysFSLink + strVendorFile);
    } else {
        qCDebug(appLog) << "Flag is false, setting file name for device: " << QString("/sys") + strSysFSLink + strDeviceFile;
        vendorFile.setFileName(QString("/sys") + strSysFSLink + strDeviceFile);
    }

    if (false == vendorFile.open(QIODevice::ReadOnly)) {
        qCWarning(appLog) << "Failed to open vendor/device file: " << vendorFile.fileName();
        return QString();
    }

    QString vendor = vendorFile.readAll();
    vendorFile.close();
    qCDebug(appLog) << "Read vendor/device info: " << vendor.trimmed();

    return vendor.trimmed();
}

const QString DeviceBaseInfo::getDriverVersion()
{
    qCDebug(appLog) << "DeviceBaseInfo::getDriverVersion called.";
    QString outInfo = Common::executeClientCmd("modinfo", QStringList() << driver(), QString(), -1);
    if(outInfo.isEmpty()) {
        qCDebug(appLog) << "Modinfo output is empty, returning empty string.";
        return  QString("");
    }

    foreach (QString out, outInfo.split("\n")) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QStringList item = out.split(":", QString::SkipEmptyParts);
#else
        QStringList item = out.split(":", Qt::SkipEmptyParts);
#endif
        if (!item.isEmpty() && "version" == item[0].trimmed()) {
            // qCDebug(appLog) << "Found driver version: " << item[1].trimmed();
            return item[1].trimmed();
        }
    }
    qCDebug(appLog) << "Driver version not found, returning empty string.";
    return QString("");
}

const QString DeviceBaseInfo::getOverviewInfo()
{
    // qCDebug(appLog) << "DeviceBaseInfo::getOverviewInfo called, returning empty string as default.";
    return QString("");
}

const QString &DeviceBaseInfo::getVID() const
{
    // qCDebug(appLog) << "DeviceBaseInfo::getVID called, returning: " << m_VID;
    return m_VID;
}

const QString &DeviceBaseInfo::getPID() const
{
    // qCDebug(appLog) << "DeviceBaseInfo::getPID called, returning: " << m_PID;
    return m_PID;
}

const QString &DeviceBaseInfo::getVIDAndPID() const
{
    // qCDebug(appLog) << "DeviceBaseInfo::getVIDAndPID called, returning: " << m_VID_PID;
    return m_VID_PID;
}

const QString &DeviceBaseInfo::getModalias() const
{
    // qCDebug(appLog) << "DeviceBaseInfo::getModalias called, returning: " << m_Modalias;
    return m_Modalias;
}

void DeviceBaseInfo::loadTableHeader()
{
    qCDebug(appLog) << "DeviceBaseInfo::loadTableHeader called. Adding default table headers.";
    // 添加表头信息
    m_TableHeader.append("Name");
    m_TableHeader.append("Vendor");
    m_TableHeader.append("Model");
}

void DeviceBaseInfo::addFilterKey(const QString &key)
{
    // qCDebug(appLog) << "DeviceBaseInfo::addFilterKey called with key: " << key;
    // 添加可显示设备属性
    m_FilterKey.insert(key);
}

void DeviceBaseInfo::getOtherMapInfo(const QMap<QString, QString> &mapInfo)
{
    // qCDebug(appLog) << "DeviceBaseInfo::getOtherMapInfo called.";
    // 获取其他设备信息
    QMap<QString, QString>::const_iterator it = mapInfo.begin();
    for (; it != mapInfo.end(); ++it) {
        QString k = it.key();

        // 可显示设备属性中存在该属性
        if (m_FilterKey.find(k) != m_FilterKey.end()) {
            if (it.value().toLower().contains("nouse")) {
                // qCDebug(appLog) << "DeviceBaseInfo::getOtherMapInfo remove key: " << k;
                m_MapOtherInfo.remove(k);
            } else {
                // qCDebug(appLog) << "DeviceBaseInfo::getOtherMapInfo insert key: " << k;
                m_MapOtherInfo.insert(k, it.value().trimmed());
            }
        }
    }
}

void DeviceBaseInfo::addBaseDeviceInfo(const QString &key, const QString &value)
{
    qCDebug(appLog) << "DeviceBaseInfo::addBaseDeviceInfo called with key: " << key << ", value: " << value;
    // 添加基础设备信息
    if (!value.isEmpty()) {
        qCDebug(appLog) << "DeviceBaseInfo::addBaseDeviceInfo append key: " << key;
        m_LstBaseInfo.append(QPair<QString, QString>(key, value));
    }
}

void DeviceBaseInfo::addOtherDeviceInfo(const QString &key, const QString &value)
{
    qCDebug(appLog) << "DeviceBaseInfo::addOtherDeviceInfo called with key: " << key << ", value: " << value;
    // 添加其他设备信息
    if (!value.isEmpty()) {
        qCDebug(appLog) << "DeviceBaseInfo::addOtherDeviceInfo insert key: " << key;
        m_LstOtherInfo.insert(0, QPair<QString, QString>(key, value));
    }
}

const QString DeviceBaseInfo::readDeviceInfoKeyValue(const QString &key)
{
    if (key.isEmpty())
        return QString("");
    // qCInfo(appLog) << __FILE__ << __LINE__  << key << "after translation:"<< kk;
    QList<QPair<QString, QString> > allBaseAttribs = getBaseAttribs();
    for (const QPair<QString, QString>& pair : allBaseAttribs) {
        if (key == pair.first)
            return pair.second;
    }
    QList<QPair<QString, QString> > allOtherAttribs = getOtherAttribs();
    for (const QPair<QString, QString>& pair : allOtherAttribs) {
        if (key == key)
            return pair.second;
    }
    return QString("");
}

bool DeviceBaseInfo::setDeviceInfoKeyValue(const QString &key, const QString &value)
{
    if (key.isEmpty() ||value.isEmpty())
        return false;

    for (QPair<QString, QString> pair : getBaseAttribs()) {
        if (key == pair.first) {
            pair.second = value;
            return true;
        }
    }

    for (QPair<QString, QString> pair : getOtherAttribs()) {
        if (key == pair.first) {
            pair.second = value;
            return true;
        }
    }
    return false;
}

void DeviceBaseInfo::setAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite)
{
    qCDebug(appLog) << "DeviceBaseInfo::setAttribute called with key: " << key << ", overwrite: " << overwrite;
    // map中存在该属性
    if (mapInfo.find(key) == mapInfo.end()) {
        qCDebug(appLog) << "DeviceBaseInfo::setAttribute map does not contain key: " << key;
        return;
    }

    // 属性值不能为空
    if (mapInfo[key] == "") {
        qCDebug(appLog) << "DeviceBaseInfo::setAttribute attribute value is empty";
        return;
    }

    // overwrite 为true直接覆盖
    if (overwrite) {
        variable = mapInfo[key].trimmed();
    } else {
        qCDebug(appLog) << "DeviceBaseInfo::setAttribute overwrite is false";
        // overwrite 为false,如果当前属性值为空或unknown时可覆盖
        if (variable.isEmpty()) {
            qCDebug(appLog) << "DeviceBaseInfo::setAttribute variable is empty, set value from map";
            variable = mapInfo[key].trimmed();
        }

        if (variable.contains("Unknown", Qt::CaseInsensitive)) {
            qCDebug(appLog) << "DeviceBaseInfo::setAttribute variable contains Unknown, set value from map";
            variable = mapInfo[key].trimmed();
        }
    }
    qCDebug(appLog) << "DeviceBaseInfo::setAttribute end";
}

TomlFixMethod DeviceBaseInfo::setTomlAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite)
{
    qCDebug(appLog) << "DeviceBaseInfo::setTomlAttribute called with key: " << key << ", overwrite: " << overwrite;
    // map中存在该属性
    if (mapInfo.find(key) == mapInfo.end()) {
        qCDebug(appLog) << "DeviceBaseInfo::setTomlAttribute map does not contain key: " << key;
        return TOML_None;
    }

    // 属性值不能为空
    if (mapInfo[key] == "") {
        qCDebug(appLog) << "DeviceBaseInfo::setTomlAttribute attribute value is empty";
        return TOML_None;
    }
    //如果有 关键字nouse 为直接接去掉清除
    if (mapInfo[key].toLower().contains("nouse")) {
        qCDebug(appLog) << "DeviceBaseInfo::setTomlAttribute attribute value contains nouse";
        variable.clear();
        return TOML_nouse;
    }

    // overwrite 为true直接覆盖
    if (overwrite) {
        setAttribute(mapInfo, key, variable, true);
        qCDebug(appLog) << "DeviceBaseInfo::setTomlAttribute overwrite is true, return TOML_Cover";
        return TOML_Cover;
    } else {
        setAttribute(mapInfo, key, variable, false);
        qCDebug(appLog) << "DeviceBaseInfo::setTomlAttribute overwrite is false, return TOML_Cover";
        return TOML_Cover;
    }
}

void DeviceBaseInfo::mapInfoToList()
{
    qCDebug(appLog) << "DeviceBaseInfo::mapInfoToList called";
    // m_MapOtherInfo --> m_LstOtherInfo
    // QMap内容转为QList存储
    auto iter = m_MapOtherInfo.begin();

    for (; iter != m_MapOtherInfo.end(); ++iter) {
        if (isValueValid(iter.value()))
            m_LstOtherInfo.append(QPair<QString, QString>(iter.key(), iter.value()));
    }
    qCDebug(appLog) << "DeviceBaseInfo::mapInfoToList end";
}

void DeviceBaseInfo::setHwinfoLshwKey(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBaseInfo::setHwinfoLshwKey called";
    // 网卡使用物理地址+逻辑设备名作为匹配值
    if (mapInfo.find("HW Address") != mapInfo.end()) {
        m_HwinfoToLshw = (mapInfo.find("Device File") != mapInfo.end()) ? mapInfo["HW Address"] + mapInfo["Device File"] : mapInfo["HW Address"];
        qCDebug(appLog) << "DeviceBaseInfo::setHwinfoLshwKey match HW Address";
        return;
    }

    // 非usb总线设备直接使用 SysFS BusID
    if (mapInfo.find("SysFS ID") != mapInfo.end()
            && mapInfo.find("SysFS BusID") != mapInfo.end()
            && !mapInfo["SysFS ID"].contains("usb")) {
        m_HwinfoToLshw = mapInfo["SysFS BusID"];
        qCDebug(appLog) << "DeviceBaseInfo::setHwinfoLshwKey match SysFS BusID";
        return;
    }

    // usb总线设备
    QStringList words = mapInfo["SysFS BusID"].split(":");
    if (words.size() != 2) {
        qCDebug(appLog) << "DeviceBaseInfo::setHwinfoLshwKey words size is not 2";
        return;
    }
    QStringList chs = words[0].split("-");
    if (chs.size() != 2) {
        qCDebug(appLog) << "DeviceBaseInfo::setHwinfoLshwKey chs size is not 2";
        return;
    }

    // 1-1.3
    // 1-3
    QStringList nums = QStringList() << "0" << "1" << "2" << "3" << "4" << "5"
                       << "6" << "7" << "8" << "9" << "a" << "b"
                       << "c" << "d" << "e" << "f" << "g" << "h"
                       << "i" << "j" << "k" << "l" << "m" << "n"
                       << "o" << "p" << "q" << "r" << "s" << "t"
                       << "u" << "v" << "w" << "x" << "y" << "z";

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRegularExpression reg("([0-9a-zA-Z]+)-([0-9a-zA-Z]+)\\.([0-9a-zA-Z]+)");
    QRegularExpressionMatch match = reg.match(words[0]);
    if (match.hasMatch()) {
        int first = match.captured(1).toInt();
        int second = match.captured(2).toInt();
        int third = match.captured(3).toInt();
        m_HwinfoToLshw = QString("usb@%1:%2.%3").arg(nums.at(first)).arg(nums.at(second)).arg(nums.at(third));
        qCDebug(appLog) << "DeviceBaseInfo::setHwinfoLshwKey match";
    } else {
        int first = chs[0].toInt();
        int second = chs[1].toInt();
        m_HwinfoToLshw = QString("usb@%1:%2").arg(nums.at(first)).arg(nums.at(second));
        qCDebug(appLog) << "DeviceBaseInfo::setHwinfoLshwKey not match";
    }
#else
    QRegExp reg("([0-9a-zA-Z]+)-([0-9a-zA-Z]+)\\.([0-9a-zA-Z]+)");
    if (reg.exactMatch(words[0])) {
        int first = reg.cap(1).toInt();
        int second = reg.cap(2).toInt();
        int third = reg.cap(3).toInt();
        m_HwinfoToLshw = QString("usb@%1:%2.%3").arg(nums.at(first)).arg(nums.at(second)).arg(nums.at(third));
        qCDebug(appLog) << "DeviceBaseInfo::setHwinfoLshwKey match";
    } else {
        int first = chs[0].toInt();
        int second = chs[1].toInt();
        m_HwinfoToLshw = QString("usb@%1:%2").arg(nums.at(first)).arg(nums.at(second));
        qCDebug(appLog) << "DeviceBaseInfo::setHwinfoLshwKey no match";
    }
#endif
}

bool DeviceBaseInfo::matchToLshw(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBaseInfo::matchToLshw called";
    // 网卡设备与序列号匹配上 或者加上逻辑设备名
    if (mapInfo.find("logical name") != mapInfo.end() && mapInfo.find("serial") != mapInfo.end()) {
        if (m_HwinfoToLshw == mapInfo["serial"] + mapInfo["logical name"]) {
            qCDebug(appLog) << "DeviceBaseInfo::matchToLshw match serial and logical name";
            return true;
        } else if (m_HwinfoToLshw == mapInfo["serial"]) {
            qCDebug(appLog) << "DeviceBaseInfo::matchToLshw match serial";
            return true;
        }
    }

    if (mapInfo.find("bus info") == mapInfo.end()) {
        qCDebug(appLog) << "DeviceBaseInfo::matchToLshw bus info not found";
        return false;
    }
    // 非usb设备
    if (mapInfo["bus info"].startsWith("pci")) {
        QStringList words = mapInfo["bus info"].split("@");
        if (2 == words.size() && words[1] == m_HwinfoToLshw) {
            qCDebug(appLog) << "DeviceBaseInfo::matchToLshw match pci";
            return true;
        }
    }

    // USB 设备
    if (m_HwinfoToLshw == mapInfo["bus info"]) {
        qCDebug(appLog) << "DeviceBaseInfo::matchToLshw match usb";
        return true;
    }
    qCDebug(appLog) << "DeviceBaseInfo::matchToLshw no match";
    return false;
}

void DeviceBaseInfo::setPhysIDMapKey(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBaseInfo::setPhysIDMapKey called";
    if (mapInfo.find("VID_PID") != mapInfo.end()) {
        m_PhysIDMap = mapInfo["VID_PID"];
        qCDebug(appLog) << "DeviceBaseInfo::setPhysIDMapKey match VID_PID";
        return;
    }
}

bool DeviceBaseInfo::PhysIDMapInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBaseInfo::PhysIDMapInfo called";
    // Modalias 或 "Module Alias" 匹配上  ，硬件 IDS 参考内核 Module Alias 规则标准
    if (mapInfo.find("Module Alias") != mapInfo.end()) {
        if (m_PhysIDMap == mapInfo["Module Alias"]) {
            qCDebug(appLog) << "PhysIDMapInfo match Module Alias";
            return true;
        }
    }
    // VID_PID 匹配上
    if (mapInfo.find("VID_PID") != mapInfo.end()) {
        if (m_PhysIDMap == mapInfo["VID_PID"]) {
            qCDebug(appLog) << "PhysIDMapInfo match VID_PID";
            return true;
        }
    }
    //to  do  “设备分类名称 + : + Vendor + Name”作为硬件 IDS
    /*  toml 方案内容定义：
    - 硬件 IDS 参考内核 Module Alias 规则标准，示例 Module Alias: "pci:v00008086d0000A3C8sv00001849sd0000A3C8bc06sc01i00"
    - 若有的设备读不到Module Alias，请以格式“设备分类名+ : + v0000 + Vendor_ID + d0000 + Product_ID + sv0000+SubVendor_ID+sd0000+SubProduct_ID” 作为硬件IDS，若  SubVendor_ID 和 SubProduct_ID 无，则填默认为0000，格式尽量跟 Module Alias 一样
    - 若有的设备读不到 Vendor_ID+Product_ID，请以格式“设备分类名称 + : + Vendor + Name”作为硬件 IDS ，所有字母全转化为小写，并去掉不可显示字符和空格
    */
    return false;
}

const  QString DeviceBaseInfo::get_string(const QString &sysPathfile)
{
    // qCDebug(appLog) << "DeviceBaseInfo::get_string called with sysPathfile: " << sysPathfile;
    // 从文件中获取D信息
    QFile file(sysPathfile);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QString info = file.readAll();
    info = info.trimmed();
    file.close();
    // qCDebug(appLog) << "DeviceBaseInfo::get_string end";
    return info;
}

