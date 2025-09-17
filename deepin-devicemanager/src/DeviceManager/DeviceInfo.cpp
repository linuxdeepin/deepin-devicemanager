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
}

DeviceBaseInfo::~DeviceBaseInfo()
{

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
    // 获取其他设备信息列表
    m_LstOtherInfo.clear();
    loadOtherDeviceInfo();
    return m_LstOtherInfo;
}

const QList<QPair<QString, QString> > &DeviceBaseInfo::getBaseAttribs()
{
    // 获取基本信息列表
    m_LstBaseInfo.clear();
    loadBaseDeviceInfo();
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
    // 获取表头
    if (m_TableHeader.size() == 0) {
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

    return m_TableHeaderTr;
}

const QStringList &DeviceBaseInfo::getTableData()
{
    // 获取表格数据
    m_TableData.clear();
    loadTableData();
    m_TableDataTr.clear();
    for (const auto &item : m_TableData) {
        QString trKey = (item);
        if (trKey.isEmpty())
            m_TableDataTr.append(item);
        else
            m_TableDataTr.append(trKey);
    }
    return m_TableDataTr;
}

QString DeviceBaseInfo::subTitle()
{
    return QString("");
}

bool DeviceBaseInfo::isValueValid(const QString &value)
{
    // 判断属性值是否有效
    if (value.isEmpty())
        return false;

    if (value == translateStr("Unknown"))
        return false;

    if (value == QString("Unknown"))
        return false;

    if (value.compare(QString("N/A"), Qt::CaseInsensitive) == 0)
        return false;

    if (value == QString(""))
        return false;

    if (value.compare(QString("Null"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("none"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Not Provided"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Not Specified"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Default string"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Unspecified"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Not Present"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("<OUT OF SPEC>"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("Other"), Qt::CaseInsensitive) == 0)
        return false;

    if (value.compare(QString("TBD"), Qt::CaseInsensitive) == 0)
        return false;

    return true;
}

void DeviceBaseInfo::setForcedDisplay(const bool &flag)
{
    m_forcedDisplay = flag;
}

void DeviceBaseInfo::toHtmlString(QDomDocument &doc)
{
    // 设备信息转为Html
    baseInfoToHTML(doc, m_LstBaseInfoTr);
    baseInfoToHTML(doc, m_LstOtherInfoTr);
}

void DeviceBaseInfo::baseInfoToHTML(QDomDocument &doc, QList<QPair<QString, QString> > &infoLst)
{
    // 设备信息转为HTML
    QDomElement table = doc.createElement("table");
    table.setAttribute("border", "0");
    table.setAttribute("width", "100%");
    table.setAttribute("cellpadding", "3");

    // 添加HTML表格内容
    foreach (auto info, infoLst) {
        if (isValueValid(info.second)) {
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
}

void DeviceBaseInfo::subTitleToHTML(QDomDocument &doc)
{
    // 子标题转为HTML格式
    if (false == this->subTitle().isEmpty()) {
        QDomElement h3 = doc.createElement("h3");
        QDomText valueText = doc.createTextNode(this->subTitle());
        h3.appendChild(valueText);
        doc.appendChild(h3);
    }
}

void DeviceBaseInfo::toDocString(Docx::Document &doc)
{
    // 设备信息转为doc
    baseInfoToDoc(doc, m_LstBaseInfoTr);
    baseInfoToDoc(doc, m_LstOtherInfoTr);
}

void DeviceBaseInfo::baseInfoToDoc(Docx::Document &doc, QList<QPair<QString, QString> > &infoLst)
{
    // 设备信息保存为Doc
    foreach (auto item, infoLst) {
        QString value = item.second;

        // 判断属性值是否有效
        if (false == isValueValid(value))
            continue;

        // 添加doc段落
        QString line = item.first + ":  " + item.second;
        doc.addParagraph(line);
    }
}

void DeviceBaseInfo::toXlsxString(QXlsx::Document &xlsx, QXlsx::Format &boldFont)
{
    // 设备信息转为xlxs表格
    baseInfoToXlsx(xlsx, boldFont, m_LstBaseInfoTr);
    baseInfoToXlsx(xlsx, boldFont, m_LstOtherInfoTr);
}

void DeviceBaseInfo::baseInfoToXlsx(QXlsx::Document &xlsx, QXlsx::Format &boldFont, QList<QPair<QString, QString> > &infoLst)
{
    // 设置表格内容字体不加粗,字号10
    boldFont.setFontBold(false);
    boldFont.setFontSize(10);

    foreach (auto item, infoLst) {
        QString value = item.second;

        // 判断属性值是否有效
        if (false == isValueValid(value))
            continue;

        // 获取行数
        int _row = DeviceManager::instance()->currentXlsRow();
        xlsx.write(_row, 1, item.first, boldFont);
        xlsx.write(_row, 2, item.second, boldFont);
    }
}

void DeviceBaseInfo::toTxtString(QTextStream &out)
{
    // 设备信息转为txt
    baseInfoToTxt(out, m_LstBaseInfoTr);
    baseInfoToTxt(out, m_LstOtherInfoTr);
}

void DeviceBaseInfo::baseInfoToTxt(QTextStream &out, QList<QPair<QString, QString> > &infoLst)
{
    foreach (auto item, infoLst) {
        QString value = item.second;

        // 判断属性值是否有效
        if (false == isValueValid(value))
            continue;

        // 设置第一列占21个字符
        out.setFieldWidth(21);
        out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
        out << item.first + ": ";
        out.setFieldWidth(0);
        out << item.second;
        out << "\n";
    }
}

void DeviceBaseInfo::tableInfoToTxt(QTextStream &out)
{
    // 获取表格内容
    getTableData();

    // 判断是否有表格内容
    if (m_TableDataTr.size() < 1)
        return;

    // 设置占位宽度
    QString text = m_TableDataTr[0];
    out.setFieldWidth(int(text.size() * 1.5));
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignRight);

    foreach (auto item, m_TableDataTr) {
        out.setFieldWidth(28);
        out << item;
    }

    out.setFieldWidth(0);
    out << "\n";
}

void DeviceBaseInfo::tableHeaderToTxt(QTextStream &out)
{
    // 获取表头
    getTableHeader();

    // 判断是否有表头
    if (m_TableHeaderTr.size() < 1)
        return;

    // 设置占位宽度
    QString text = m_TableHeaderTr[0];
    out.setFieldWidth(int(text.size() * 1.5));
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);

    out << "\n";
    for (int col = 0; col < m_TableHeaderTr.size() - 1; ++col) {
        out.setFieldWidth(30);
        out << m_TableHeaderTr[col];
    }
    out.setFieldWidth(0);
    out << "\n";
}

void DeviceBaseInfo::tableInfoToHtml(QFile &html)
{
    // 获取表格内容
    getTableData();

    // 判断是否有表格内容
    if (m_TableDataTr.size() < 1)
        return;

    // 写表格内容
    foreach (auto item, m_TableDataTr) {
        html.write(QString("<td style=\"width:200px;text-align:left;\">" + item + "</td>").toUtf8().data());
    }

    html.write("</tr>\n");
}

void DeviceBaseInfo::tableHeaderToHtml(QFile &html)
{
    // 获取表头信息
    getTableHeader();

    // 判断是否有表头
    if (m_TableHeaderTr.size() < 1)
        return;

    html.write("<thead><tr>\n");

    // 写表头内容
    for (int col = 0; col < m_TableHeaderTr.size() - 1; ++col)
        html.write(QString("<th style=\"width:200px;text-align:left; white-space:pre;\">" + m_TableHeaderTr[col] + "</th>").toUtf8().data());

    html.write("</tr></thead>\n");
}

void DeviceBaseInfo::tableInfoToDoc(Docx::Table *tab, int &row)
{
    // 表格信息保存为Doc
    if (nullptr == tab)
        return;

    // 获取表格数据
    getTableData();

    if (m_TableDataTr.size() < 1)
        return;

    // 添加doc表格
    for (int col = 0; col < m_TableDataTr.size(); ++col) {
        auto cel = tab->cell(row, col);
        cel->addText(m_TableDataTr[col]);
    }
}

void DeviceBaseInfo::tableHeaderToDoc(Docx::Table *tab)
{
    // 表头保存为doc
    getTableHeader();

    if (m_TableHeaderTr.size() < 1)
        return;

    // 添加表头信息
    for (int col = 0; col < m_TableHeaderTr.size() - 1; ++col)  {
        tab->addColumn();
        auto cel = tab->cell(0, col);
        cel->addText(m_TableHeaderTr[col]);
    }
}

void DeviceBaseInfo::tableInfoToXlsx(QXlsx::Document &xlsx)
{
    // 获取表格信息
    getTableData();

    if (m_TableDataTr.size() < 1)
        return;

    // 添加表格信息
    int curRow = DeviceManager::instance()->currentXlsRow();
    for (int col = 0; col < m_TableDataTr.size(); ++col)
        xlsx.write(curRow, col + 1, m_TableDataTr[col]);
}

void DeviceBaseInfo::tableHeaderToXlsx(QXlsx::Document &xlsx)
{
    // 获取表头
    getTableHeader();

    if (m_TableHeaderTr.size() < 1)
        return;

    // 添加表头信息
    int curRow = DeviceManager::instance()->currentXlsRow();
    for (int col = 0; col < m_TableHeaderTr.size() - 1; ++col) {
        QXlsx::Format boldFont;
        boldFont.setFontSize(10);
        boldFont.setFontBold(true);
        xlsx.write(curRow, col + 1, m_TableHeaderTr[col], boldFont);
    }
}

void DeviceBaseInfo::setOtherDeviceInfo(const QString &key, const QString &value)
{
    m_MapOtherInfo[key] = value;
}

TomlFixMethod DeviceBaseInfo::setInfoFromTomlBase(const QMap<QString, QString> &mapInfo)
{
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

    setTomlAttribute(mapInfo, "Revision", m_Version);
    setTomlAttribute(mapInfo, "Description", m_Description);
    m_VID = m_VID.toLower();
    m_PID = m_PID.toLower();
    m_VID_PID = m_VID + m_PID.remove("0x");
    m_PhysID = m_VID_PID;
    m_forcedDisplay = true;
    return ret;
}

EnableDeviceStatus DeviceBaseInfo::setEnable(bool)
{
    return EDS_Success;
}

bool DeviceBaseInfo::enable()
{
    return m_Enable;
}

bool DeviceBaseInfo::available()
{
    if (driver().isEmpty()) {
        m_Available = false;
    }
    return m_forcedDisplay ? m_forcedDisplay : m_Available;
}

bool DeviceBaseInfo::driverIsKernelIn(const QString &driver)
{
    // 驱动为空情况:
    // 1. 驱动被卸载了 此时驱动属于核外驱动
    // 2. ps/2 笔记本触摸板 暂无法获取驱动 此时当成核内驱动处理
    // 3. 但是由于判断是否是ps/2或者笔记本触摸板在子类判断(无需放在基类)，因此此处为空时先返回false，而在子类(DeviceInput)调用后判断是否是ps/2鼠标
    if (driver.isEmpty()) {
        return false;
    }

    // 英伟达驱动无法获取驱动模块，但是不属于核内驱动
    if ("nvidia" == driver) {
        return false;
    }

    // 判断lsmod是否能查询
    QString outInfo = Common::executeClientCmd("modinfo", QStringList() << driver, QString(), -1);
    return !outInfo.contains("filename:");
}

void DeviceBaseInfo::setCanEnable(bool can)
{
    m_CanEnable = can;
}

bool DeviceBaseInfo::canEnable()
{
    return m_CanEnable;
}

void DeviceBaseInfo::setEnableValue(bool e)
{
    m_Enable = e;
}

bool DeviceBaseInfo::canUninstall()
{
    return m_CanUninstall;
}

void DeviceBaseInfo::setCanUninstall(bool can)
{
    m_CanUninstall = can;
}

void DeviceBaseInfo::setHardwareClass(const QString &hclass)
{
    m_HardwareClass = hclass;
}

const QString &DeviceBaseInfo::hardwareClass() const
{
    return m_HardwareClass;
}

const QString &DeviceBaseInfo::uniqueID() const
{
    return m_UniqueID;
}

const QString &DeviceBaseInfo::sysPath() const
{
    return m_SysPath;
}

const QString DeviceBaseInfo::getVendorOrModelId(const QString &sysPath, bool flag)
{
    // 从文件中获取制造商ID信息
    QFile vendorFile;
    QString strVendorFile("/vendor");
    QString strDeviceFile("/device");
    QString strSysFSLink = sysPath;

    if (sysPath.contains("usb")) {
        strVendorFile = "/idVendor";
        strDeviceFile = "/idProduct";
        if (!QFile::exists("/sys" + strSysFSLink + strVendorFile)) {
            strSysFSLink = strSysFSLink.mid(0, sysPath.lastIndexOf('/'));
        }
    }

    if (flag) {
        vendorFile.setFileName(QString("/sys") + strSysFSLink + strVendorFile);
    } else {
        vendorFile.setFileName(QString("/sys") + strSysFSLink + strDeviceFile);
    }

    if (false == vendorFile.open(QIODevice::ReadOnly))
        return QString();

    QString vendor = vendorFile.readAll();
    vendorFile.close();

    return vendor;
}

void DeviceBaseInfo::setVendorNameBylsusbLspci(const QString &vidpid, const QString &modalias)
{
    if (!vidpid.isEmpty() && modalias.contains("usb")) {
        QProcess process;
        QString vendorId = vidpid.toLower().remove("0x").trimmed().left(4);
        QString deviceId = vidpid.toLower().remove("0x").trimmed().right(4);
        process.start("lsusb -v -d " + vendorId + ":" + deviceId);
        process.waitForFinished(-1);

        QString output = process.readAllStandardOutput();

        foreach (QString out, output.split("\n")) {
            // 从USB设备获取制造商和设备名称
            if (out.contains("idVendor", Qt::CaseSensitive)) {
                m_Vendor = out.remove(0, out.indexOf(vendorId) + 4).trimmed();
            } else if (out.contains("idProduct", Qt::CaseSensitive)) {
                m_Name = out.remove(0, out.indexOf(deviceId) + 4).trimmed();
            }
        }
    }
}

const QString DeviceBaseInfo::getDriverVersion()
{
    QString outInfo = Common::executeClientCmd("modinfo", QStringList() << driver(), QString(), -1);
    if (outInfo.isEmpty())
        return  QString("");

    foreach (QString out, outInfo.split("\n")) {
        QStringList item = out.split(":", QString::SkipEmptyParts);
        if (!item.isEmpty() && "version" == item[0].trimmed()) {
            return item[1].trimmed();
        }
    }

    return QString("");
}

const QString DeviceBaseInfo::getOverviewInfo()
{
    return QString("");
}

const QString &DeviceBaseInfo::getVID() const
{
    return m_VID;
}

const QString &DeviceBaseInfo::getPID() const
{
    return m_PID;
}

const QString &DeviceBaseInfo::getVIDAndPID() const
{
    return m_VID_PID;
}

const QString &DeviceBaseInfo::getModalias() const
{
    return m_Modalias;
}

void DeviceBaseInfo::loadTableHeader()
{
    // 添加表头信息
    m_TableHeader.append("Name");
    m_TableHeader.append("Vendor");
    m_TableHeader.append("Model");
}

void DeviceBaseInfo::addFilterKey(const QString &key)
{
    // 添加可显示设备属性
    m_FilterKey.insert(key);
}

void DeviceBaseInfo::getOtherMapInfo(const QMap<QString, QString> &mapInfo)
{
    // 获取其他设备信息
    QMap<QString, QString>::const_iterator it = mapInfo.begin();
    for (; it != mapInfo.end(); ++it) {
        QString k = it.key();

        // 可显示设备属性中存在该属性
        if (m_FilterKey.find(k) != m_FilterKey.end()) {
            if (it.value().toLower().contains("nouse"))
                m_MapOtherInfo.remove(k);
            else
                m_MapOtherInfo.insert(k, it.value().trimmed());
        }
    }
}

void DeviceBaseInfo::addBaseDeviceInfo(const QString &key, const QString &value)
{
    // 添加基础设备信息
    if (!value.isEmpty())
        m_LstBaseInfo.append(QPair<QString, QString>(key, value));
}

void DeviceBaseInfo::addOtherDeviceInfo(const QString &key, const QString &value)
{
    // 添加其他设备信息
    if (!value.isEmpty())
        m_LstOtherInfo.insert(0, QPair<QString, QString>(key, value));
}

const QString DeviceBaseInfo::readDeviceInfoKeyValue(const QString &key)
{
    if (key.isEmpty())
        return QString("");
    QList<QPair<QString, QString> > allBaseAttribs = getBaseAttribs();
    for (const QPair<QString, QString>& pair : allBaseAttribs) {
        if (key == pair.first)
            return pair.second;
    }
    QList<QPair<QString, QString> > allOtherAttribs = getOtherAttribs();
    for (const QPair<QString, QString>& pair : allOtherAttribs) {
        if (key == pair.first)
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
    // map中存在该属性
    if (mapInfo.find(key) == mapInfo.end())
        return;

    // 属性值不能为空
    if (mapInfo[key] == "")
        return;

    // overwrite 为true直接覆盖
    if (overwrite) {
        variable = mapInfo[key].trimmed();
    } else {

        // overwrite 为false,如果当前属性值为空或unknown时可覆盖
        if (variable.isEmpty())
            variable = mapInfo[key].trimmed();

        if (variable.contains("Unknown", Qt::CaseInsensitive))
            variable = mapInfo[key].trimmed();
    }
}

TomlFixMethod DeviceBaseInfo::setTomlAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite)
{
    // map中存在该属性
    if (mapInfo.find(key) == mapInfo.end())
        return TOML_None;

    // 属性值不能为空
    if (mapInfo[key] == "")
        return TOML_None;
    //如果有 关键字nouse 为直接接去掉清除
    if (mapInfo[key].toLower().contains("nouse")) {
        variable.clear();
        return TOML_nouse;
    }

    // overwrite 为true直接覆盖
    if (overwrite) {
        setAttribute(mapInfo, key, variable, true);
        return TOML_Cover;
    } else {
        setAttribute(mapInfo, key, variable, false);
        return TOML_Cover;
    }
}

void DeviceBaseInfo::mapInfoToList()
{
    // m_MapOtherInfo --> m_LstOtherInfo
    // QMap内容转为QList存储
    QMap<QString, QString>::const_iterator iter = m_MapOtherInfo.constBegin();

    for (; iter != m_MapOtherInfo.constEnd(); ++iter) {
        if (isValueValid(iter.value()))
            m_LstOtherInfo.append(QPair<QString, QString>(iter.key(), iter.value()));
    }
}

void DeviceBaseInfo::setHwinfoLshwKey(const QMap<QString, QString> &mapInfo)
{
    // 网卡使用物理地址+逻辑设备名作为匹配值
    if (mapInfo.find("HW Address") != mapInfo.end()) {
        m_HwinfoToLshw = (mapInfo.find("Device File") != mapInfo.end()) ? mapInfo["HW Address"] + mapInfo["Device File"] : mapInfo["HW Address"];
        return;
    }

    // 非usb总线设备直接使用 SysFS BusID
    if (mapInfo.find("SysFS ID") != mapInfo.end()
            && mapInfo.find("SysFS BusID") != mapInfo.end()
            && !mapInfo["SysFS ID"].contains("usb")) {
        m_HwinfoToLshw = mapInfo["SysFS BusID"];
        return;
    }

    // usb总线设备
    QStringList words = mapInfo["SysFS BusID"].split(":");
    if (words.size() != 2) {
        return;
    }
    QStringList chs = words[0].split("-");
    if (chs.size() != 2) {
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
    QRegExp reg("([0-9a-zA-Z]+)-([0-9a-zA-Z]+)\\.([0-9a-zA-Z]+)");
    if (reg.exactMatch(words[0])) {
        int first = reg.cap(1).toInt();
        int second = reg.cap(2).toInt();
        int third = reg.cap(3).toInt();
        m_HwinfoToLshw = QString("usb@%1:%2.%3").arg(nums.at(first)).arg(nums.at(second)).arg(nums.at(third));
    } else {
        int first = chs[0].toInt();
        int second = chs[1].toInt();
        m_HwinfoToLshw = QString("usb@%1:%2").arg(nums.at(first)).arg(nums.at(second));
    }
}

bool DeviceBaseInfo::matchToLshw(const QMap<QString, QString> &mapInfo)
{
    // 网卡设备与序列号匹配上 或者加上逻辑设备名
    if (mapInfo.find("logical name") != mapInfo.end() && mapInfo.find("serial") != mapInfo.end()) {
        if (m_HwinfoToLshw == mapInfo["serial"] + mapInfo["logical name"]) {
            return true;
        } else if (m_HwinfoToLshw == mapInfo["serial"]) {
            return true;
        }
    }

    if (mapInfo.find("bus info") == mapInfo.end()) {
        return false;
    }
    // 非usb设备
    if (mapInfo["bus info"].startsWith("pci")) {
        QStringList words = mapInfo["bus info"].split("@");
        if (2 == words.size() && words[1] == m_HwinfoToLshw) {
            return true;
        }
    }

    // USB 设备
    if (m_HwinfoToLshw == mapInfo["bus info"]) {
        return true;
    }
    return false;
}

void DeviceBaseInfo::setPhysIDMapKey(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.find("VID_PID") != mapInfo.end()) {
        m_PhysIDMap = mapInfo["VID_PID"];
        return;
    }
}

bool DeviceBaseInfo::PhysIDMapInfo(const QMap<QString, QString> &mapInfo)
{
    // Modalias 或 "Module Alias" 匹配上  ，硬件 IDS 参考内核 Module Alias 规则标准
    if (mapInfo.find("Module Alias") != mapInfo.end()) {
        if (m_PhysIDMap == mapInfo["Module Alias"]) {
            return true;
        }
    }
    // VID_PID 匹配上
    if (mapInfo.find("VID_PID") != mapInfo.end()) {
        if (m_PhysIDMap == mapInfo["VID_PID"]) {
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
    // 从文件中获取D信息
    QFile file(sysPathfile);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QString info = file.readAll();
    info = info.trimmed();
    file.close();
    return info;
}
//只是为了lupdate自动增加翻译而占位
void DeviceBaseInfo::generatorTranslate()
{
    QStringList translationStrings;
    translationStrings \
    <<  tr("Core(s)") \
    <<  tr("Processor") \
    <<  tr("ACL MTU") \
    <<  tr("Address") \
    <<  tr("Alias") \
    <<  tr("ansiversion") \
    <<  tr("Application") \
    <<  tr("Architecture") \
    <<  tr("Array Handle") \
    <<  tr("Asset Tag") \
    <<  tr("Auto Negotiation") \
    <<  tr("Bank Locator") \
    <<  tr("Base Board Information") \
    <<  tr("BD Address") \
    <<  tr("BIOS Information") \
    <<  tr("BIOS Revision") \
    <<  tr("BIOS ROMSIZE") \
    <<  tr("Board name") \
    <<  tr("BogoMIPS") \
    <<  tr("Boot-up State") \
    <<  tr("Broadcast") \
    <<  tr("Bus") \
    <<  tr("bus info") \
    <<  tr("Bus Info") \
    <<  tr("Cache Size") \
    <<  tr("Capabilities") \
    <<  tr("Capacity") \
    <<  tr("Characteristics") \
    <<  tr("Chassis Handle") \
    <<  tr("Chassis Information") \
    <<  tr("Chip") \
    <<  tr("Chipset") \
    <<  tr("Class") \
    <<  tr("Clock") \
    <<  tr("Config Status") \
    <<  tr("Configured Speed") \
    <<  tr("Configured Voltage") \
    <<  tr("Contained Elements") \
    <<  tr("Contained Object Handles") \
    <<  tr("copies") \
    <<  tr("Core ID") \
    <<  tr("CPU architecture") \
    <<  tr("CPU Family") \
    <<  tr("CPU ID") \
    <<  tr("CPU implementer") \
    <<  tr("CPU part") \
    <<  tr("CPU revision") \
    <<  tr("CPU variant") \
    <<  tr("critical-action") \
    <<  tr("Currently Installed Language") \
    <<  tr("Current Resolution") \
    <<  tr("daemon-version") \
    <<  tr("Data Width") \
    <<  tr("Date") \
    <<  tr("description") \
    <<  tr("Description") \
    <<  tr("Design Capacity") \
    <<  tr("Design Voltage") \
    <<  tr("Device") \
    <<  tr("Device Class") \
    <<  tr("Device File") \
    <<  tr("Device Files") \
    <<  tr("Device Name") \
    <<  tr("Device Number") \
    <<  tr("DigitalOutput") \
    <<  tr("Disable") \
    <<  tr("Discoverable") \
    <<  tr("Discovering") \
    <<  tr("Display Input") \
    <<  tr("Display Output") \
    <<  tr("Display Ratio") \
    <<  tr("DP") \
    <<  tr("Driver") \
    <<  tr("Driver Activation Cmd") \
    <<  tr("Driver Modules") \
    <<  tr("Driver Status") \
    <<  tr("Driver Version") \
    <<  tr("Duplex") \
    <<  tr("DVI") \
    <<  tr("eDP") \
    <<  tr("EGL client APIs") \
    <<  tr("EGL version") \
    <<  tr("energy") \
    <<  tr("energy-empty") \
    <<  tr("energy-full") \
    <<  tr("energy-full-design") \
    <<  tr("energy-rate") \
    <<  tr("Error Correction Type") \
    <<  tr("Error Information Handle") \
    <<  tr("EV") \
    <<  tr("Extensions") \
    <<  tr("Family") \
    <<  tr("Features") \
    <<  tr("Firmware") \
    <<  tr("Firmware Revision") \
    <<  tr("Firmware Version") \
    <<  tr("Flags") \
    <<  tr("Form Factor") \
    <<  tr("GDDR capacity") \
    <<  tr("Geometry (Logical)") \
    <<  tr("GLSL version") \
    <<  tr("GL version") \
    <<  tr("GPU type") \
    <<  tr("GPU vendor") \
    <<  tr("Graphics Memory") \
    <<  tr("guid") \
    <<  tr("Handlers") \
    <<  tr("Hardware Class") \
    <<  tr("has history") \
    <<  tr("has statistics") \
    <<  tr("HCI Version") \
    <<  tr("HDMI") \
    <<  tr("Height") \
    <<  tr("icon-name") \
    <<  tr("Input/Output") \
    <<  tr("Installable Languages") \
    <<  tr("Interface") \
    <<  tr("Interface Type") \
    <<  tr("ioport") \
    <<  tr("IO Port") \
    <<  tr("IP") \
    <<  tr("IRQ") \
    <<  tr("job-cancel-after") \
    <<  tr("job-hold-until") \
    <<  tr("job-priority") \
    <<  tr("KernelModeDriver") \
    <<  tr("KEY") \
    <<  tr("L1d Cache") \
    <<  tr("L1i Cache") \
    <<  tr("L2 Cache") \
    <<  tr("L3 Cache") \
    <<  tr("L4 Cache") \
    <<  tr("Language Description Format") \
    <<  tr("latency") \
    <<  tr("Latency") \
    <<  tr("lid-is-closed") \
    <<  tr("lid-is-present") \
    <<  tr("Link") \
    <<  tr("Link mode") \
    <<  tr("Link policy") \
    <<  tr("LMP Version") \
    <<  tr("Location") \
    <<  tr("Location In Chassis") \
    <<  tr("Locator") \
    <<  tr("Lock") \
    <<  tr("logical name") \
    <<  tr("Logical Name") \
    <<  tr("logicalsectorsize") \
    <<  tr("Logical Size") \
    <<  tr("MAC Address") \
    <<  tr("marker-change-time") \
    <<  tr("Max Frequency") \
    <<  tr("Maximum Capacity") \
    <<  tr("Maximum Current") \
    <<  tr("Maximum Power") \
    <<  tr("Maximum Rate") \
    <<  tr("Maximum Resolution") \
    <<  tr("Maximum Voltage") \
    <<  tr("Media Type") \
    <<  tr("Memory") \
    <<  tr("Memory Address") \
    <<  tr("Memory Operating Mode Capability") \
    <<  tr("Memory Subsystem Controller Manufacturer ID") \
    <<  tr("Memory Subsystem Controller Product ID") \
    <<  tr("Memory Technology") \
    <<  tr("Minimum Resolution") \
    <<  tr("Minimum Voltage") \
    <<  tr("Modalias") \
    <<  tr("Model") \
    <<  tr("Module Alias") \
    <<  tr("Module Alias") \
    <<  tr("Module Manufacturer ID") \
    <<  tr("Module Product ID") \
    <<  tr("MSC") \
    <<  tr("Multicast") \
    <<  tr("Name") \
    <<  tr("native-path") \
    <<  tr("Negotiation Rate") \
    <<  tr("network") \
    <<  tr("Non-Volatile Size") \
    <<  tr("Number Of Devices") \
    <<  tr("Number Of Power Cords") \
    <<  tr("number-up") \
    <<  tr("OEM Information") \
    <<  tr("on-battery") \
    <<  tr("online") \
    <<  tr("orientation-requested") \
    <<  tr("Packet type") \
    <<  tr("Pairable") \
    <<  tr("Part Number") \
    <<  tr("percentage") \
    <<  tr("Phys") \
    <<  tr("physical id") \
    <<  tr("Physical ID") \
    <<  tr("Physical Memory Array") \
    <<  tr("Port") \
    <<  tr("Powered") \
    <<  tr("power supply") \
    <<  tr("Power Supply State") \
    <<  tr("Primary Monitor") \
    <<  tr("print-color-mode") \
    <<  tr("printer-is-accepting-jobs") \
    <<  tr("printer-is-shared") \
    <<  tr("printer-is-temporary") \
    <<  tr("printer-make-and-model") \
    <<  tr("printer-state-change-time") \
    <<  tr("printer-state-reasons") \
    <<  tr("printer-type") \
    <<  tr("printer-uri-supported") \
    <<  tr("product") \
    <<  tr("Product Date") \
    <<  tr("Product Name") \
    <<  tr("PROP") \
    <<  tr("Rank") \
    <<  tr("rechargeable") \
    <<  tr("Refresh Rate") \
    <<  tr("Release date") \
    <<  tr("Release Date") \
    <<  tr("Revision") \
    <<  tr("ROM Size") \
    <<  tr("Rotation Rate") \
    <<  tr("Runtime Size") \
    <<  tr("SBDS Chemistry") \
    <<  tr("SBDS Manufacture Date") \
    <<  tr("SBDS Serial Number") \
    <<  tr("SBDS Version") \
    <<  tr("SCO MTU") \
    <<  tr("sectorsize") \
    <<  tr("Security Status") \
    <<  tr("Serial ID") \
    <<  tr("Serial Number") \
    <<  tr("Service Classes") \
    <<  tr("Set") \
    <<  tr("Shared") \
    <<  tr("sides") \
    <<  tr("Size") \
    <<  tr("SKU Number") \
    <<  tr("Slot") \
    <<  tr("SMBIOS Version") \
    <<  tr("state") \
    <<  tr("status") \
    <<  tr("Status") \
    <<  tr("Stepping") \
    <<  tr("SubDevice") \
    <<  tr("SubVendor") \
    <<  tr("Subversion") \
    <<  tr("Support Resolution") \
    <<  tr("Sysfs") \
    <<  tr("SysFS_Path") \
    <<  tr("System Information") \
    <<  tr("technology") \
    <<  tr("temperature") \
    <<  tr("Temperature") \
    <<  tr("Thermal State") \
    <<  tr("Threads") \
    <<  tr("Total Width") \
    <<  tr("Type") \
    <<  tr("Type Detail") \
    <<  tr("Unavailable") \
    <<  tr("Uniq") \
    <<  tr("updated") \
    <<  tr("URI") \
    <<  tr("UUID") \
    <<  tr("Vendor") \
    <<  tr("Version") \
    <<  tr("VGA") \
    <<  tr("Virtualization") \
    <<  tr("Volatile Size") \
    <<  tr("voltage") \
    <<  tr("Voltage") \
    <<  tr("Wake-up Type") \
    <<  tr("warning-level") \
    <<  tr("Width") \
    <<  tr("battery") \
    <<  tr("inch") \
    <<  tr("Architecture") \
    <<  tr("Frequency") \
    <<  tr("Max Frequency") \
    <<  tr("Media Type") \
    <<  tr("Name") \
    <<  tr("Size") \
    <<  tr("Speed") \
    <<  tr("Type") \
    <<  tr("Vendor") \
    <<  tr("Processor");
}

void DeviceBaseInfo::setSysPath(const QString &newSysPath)
{
    m_SysPath = newSysPath;
}

void DeviceBaseInfo::setUniqueID(const QString &UniqueID)
{
    m_UniqueID = UniqueID;
}
