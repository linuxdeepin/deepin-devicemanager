#include "DeviceInfo.h"
#include "../commondefine.h"
#include<QDebug>
#include"DeviceManager.h"
#include <DApplication>
#include "../deviceinfoparser.h"

DWIDGET_USE_NAMESPACE

DeviceBaseInfo::DeviceBaseInfo(QObject *parent)
    : QObject(parent)
{

}

DeviceBaseInfo::~DeviceBaseInfo()
{

}

//const QMap<QString, QString> &DeviceBaseInfo::getOtherAttribs()const
//{
//    return m_MapOtherInfo;
//}

const QList<QPair<QString, QString>> &DeviceBaseInfo::getOtherAttribs()
{
    m_LstOtherInfo.clear();
    loadOtherDeviceInfo();
    return m_LstOtherInfo;
}

const QList<QPair<QString, QString> > &DeviceBaseInfo::getBaseAttribs()
{
    m_LstBaseInfo.clear();
    loadBaseDeviceInfo();
    return m_LstBaseInfo;
}

const QStringList &DeviceBaseInfo::getTableHeader()
{
    if (m_TableHeader.size() == 0)
        loadTableHeader();
    return m_TableHeader;
}

const QStringList &DeviceBaseInfo::getTableData()
{
    m_TableData.clear();
    loadTableData();
    return m_TableData;
}

QString DeviceBaseInfo::subTitle()
{
    return QString("");
}

bool DeviceBaseInfo::isValueValid(QString &value)
{
    if (value.isEmpty()) {
        return false;
    }

    if (value == QObject::tr("Unknown")) {
        return false;
    }

    if (value == QString("Unknown")) {
        return false;
    }

    if (value.compare(QString("N/A"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if (value == QString("")) {
        return false;
    }

    if (value.compare(QString("Null"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if (value.compare(QString("none"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if (value.compare(QString("Not Provided"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if (value.compare(QString("Not Specified"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if (value.compare(QString("Default string"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if (value.compare(QString("Unspecified"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if (value.compare(QString("Not Present"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if (value.compare(QString("<OUT OF SPEC>"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if (value.compare(QString("Other"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    if (value.compare(QString("TBD"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    return true;
}

void DeviceBaseInfo::toHtmlString(QDomDocument &doc)
{
    baseInfoToHTML(doc, m_LstBaseInfo);
    baseInfoToHTML(doc, m_LstOtherInfo);
}

void DeviceBaseInfo::baseInfoToHTML(QDomDocument &doc, QList<QPair<QString, QString> > &infoLst)
{
    QDomElement table = doc.createElement("table");
    table.setAttribute("border", "0");
    table.setAttribute("width", "100%");
    table.setAttribute("cellpadding", "3");

    foreach (auto info, infoLst) {
        if (isValueValid(info.second)) {
            QDomElement tr = doc.createElement("tr");

            QDomElement td = doc.createElement("td");
            td.setAttribute("width", "15%");
            td.setAttribute("style", "text-align:left;");

            QDomText nameText = doc.createTextNode(info.first + ": ");
            td.appendChild(nameText);
            tr.appendChild(td);

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
    if (this->subTitle().isEmpty() == false) {
        QDomElement h3 = doc.createElement("h3");
        QDomText valueText = doc.createTextNode(this->subTitle());
        h3.appendChild(valueText);
        doc.appendChild(h3);
    }
}

void DeviceBaseInfo::toDocString(Docx::Document &doc)
{
    baseInfoToDoc(doc, m_LstBaseInfo);
    baseInfoToDoc(doc, m_LstOtherInfo);
}

void DeviceBaseInfo::baseInfoToDoc(Docx::Document &doc, QList<QPair<QString, QString> > &infoLst)
{
    foreach (auto item, infoLst) {
        QString value = item.second;
        if (isValueValid(value) == false) {
            continue;
        }

        QString line = item.first + ":  " + item.second;
        doc.addParagraph(line);
    }
}

void DeviceBaseInfo::toXlsxString(QXlsx::Document &xlsx, QXlsx::Format &boldFont)
{
    baseInfoToXlsx(xlsx, boldFont, m_LstBaseInfo);
    baseInfoToXlsx(xlsx, boldFont, m_LstOtherInfo);
}

void DeviceBaseInfo::baseInfoToXlsx(QXlsx::Document &xlsx, QXlsx::Format &boldFont, QList<QPair<QString, QString> > &infoLst)
{
    boldFont.setFontBold(false);
    boldFont.setFontSize(10);

    foreach (auto item, infoLst) {
        QString value = item.second;
        if (isValueValid(value) == false) {
            continue;
        }
        int _row = DeviceManager::instance()->currentXlsRow();
        xlsx.write(_row, 1, item.first, boldFont);
        xlsx.write(_row, 2, item.second, boldFont);
    }
}

void DeviceBaseInfo::toTxtString(QTextStream &out)
{
    baseInfoToTxt(out, m_LstBaseInfo);
    baseInfoToTxt(out, m_LstOtherInfo);
}

void DeviceBaseInfo::baseInfoToTxt(QTextStream &out, QList<QPair<QString, QString> > &infoLst)
{
    foreach (auto item, infoLst) {

        QString value = item.second;
        if (isValueValid(value) == false) {
            continue;
        }

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
    getTableData();
    if (m_TableData.size() < 1) {
        return;
    }
    QString text = m_TableData[0];
    out.setFieldWidth(int(text.size() * 1.5));
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignRight);

    foreach (auto item, m_TableData) {
        out.setFieldWidth(28);
        out << item;

    }
    out.setFieldWidth(0);
    out << "\n";
}

void DeviceBaseInfo::tableHeaderToTxt(QTextStream &out)
{
    getTableHeader();
    if (m_TableHeader.size() < 1) {
        return;
    }
    QString text = m_TableHeader[0];
    out.setFieldWidth(int(text.size() * 1.5));
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);

    out << "\n";
    foreach (auto item, m_TableHeader) {
        out.setFieldWidth(30);
        out << item;
    }
    out.setFieldWidth(0);
    out << "\n";
}

void DeviceBaseInfo::tableInfoToHtml(QFile &html)
{
    getTableData();

    if (m_TableData.size() < 1) {
        return;
    }

    foreach (auto item, m_TableData) {
        html.write(QString("<td style=\"width:200px;text-align:left;\">" + item + "</td>").toUtf8().data());
    }
    html.write("</tr>\n");
}

void DeviceBaseInfo::tableHeaderToHtml(QFile &html)
{
    getTableHeader();

    if (m_TableHeader.size() < 1) {
        return;
    }

    html.write("<thead><tr>\n");

    foreach (auto item, m_TableHeader) {
        html.write(QString("<th style=\"width:200px;text-align:left; white-space:pre;\">" + item + "</th>").toUtf8().data());
    }

    html.write("</tr></thead>\n");
}

void DeviceBaseInfo::tableInfoToDoc(Docx::Table *tab, int &row)
{
    if (tab == nullptr) {
        return;
    }

    getTableData();

    if (m_TableData.size() < 1) {
        return;
    }

    for (int col = 0; col < m_TableData.size(); ++col) {
        auto cel = tab->cell(row, col);
        cel->addText(m_TableData[col]);
    }
}

void DeviceBaseInfo::tableHeaderToDoc(Docx::Table *tab)
{
    getTableHeader();

    if (m_TableHeader.size() < 1) {
        return;
    }

    for (int col = 0; col < m_TableHeader.size(); ++col)  {
        tab->addColumn();
        auto cel = tab->cell(0, col);
        cel->addText(m_TableHeader[col]);

    }
}

void DeviceBaseInfo::tableInfoToXlsx(QXlsx::Document &xlsx)
{
    getTableData();

    if (m_TableData.size() < 1) {
        return;
    }

    int curRow = DeviceManager::instance()->currentXlsRow();
    for (int col = 0; col < m_TableData.size(); ++col) {
        xlsx.write(curRow, col + 1, m_TableData[col]);
    }
}

void DeviceBaseInfo::tableHeaderToXlsx(QXlsx::Document &xlsx)
{
    getTableHeader();

    if (m_TableHeader.size() < 1) {
        return;
    }

    int curRow = DeviceManager::instance()->currentXlsRow();
    for (int col = 0; col < m_TableHeader.size(); ++col) {
        QXlsx::Format boldFont;
        boldFont.setFontSize(10);
        boldFont.setFontBold(true);
        xlsx.write(curRow, col + 1, m_TableHeader[col], boldFont);
    }

}

const QString DeviceBaseInfo::getOverviewInfo()
{
    return QString("");
}

void DeviceBaseInfo::loadTableHeader()
{
    m_TableHeader.append(tr("Name"));
    m_TableHeader.append(tr("Vendor"));
    m_TableHeader.append(tr("Model"));
}

void DeviceBaseInfo::addFilterKey(const QString &key)
{
    m_FilterKey.insert(key);
}

void DeviceBaseInfo::getOtherMapInfo(const QMap<QString, QString> &mapInfo)
{
    QMap<QString, QString>::const_iterator it = mapInfo.begin();
    for (; it != mapInfo.end(); ++it) {
        QString k = DApplication::translate("QObject", it.key().trimmed().toStdString().data());
        if (m_FilterKey.find(k) != m_FilterKey.end()) {
            m_MapOtherInfo.insert(k, it.value().trimmed());
//            m_LstOtherInfo.append(QPair<QString, QString>(k, it.value().trimmed()));
        }
    }
}

void DeviceBaseInfo::addBaseDeviceInfo(const QString &key, const QString &value)
{
    if (!value.isEmpty()) {
        m_LstBaseInfo.append(QPair<QString, QString>(key, value));
    }
}

void DeviceBaseInfo::addOtherDeviceInfo(const QString &key, const QString &value)
{
    if (!value.isEmpty()) {
        m_LstOtherInfo.insert(0, QPair<QString, QString>(key, value));
    }
}

void DeviceBaseInfo::setAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite)
{
    if (mapInfo.find(key) == mapInfo.end()) {
        return;
    }
    if (mapInfo[key] == "") {
        return;
    }
    if (overwrite) {
        variable = mapInfo[key].trimmed();
    } else {
        if (variable.isEmpty()) {
            variable = mapInfo[key].trimmed();
        }

        if (variable.contains("Unknown", Qt::CaseInsensitive)) {
            variable = mapInfo[key].trimmed();
        }
    }
}

void DeviceBaseInfo::mapInfoToList()
{
//    m_MapOtherInfo --> m_LstOtherInfo

    auto iter = m_MapOtherInfo.begin();

    for (; iter != m_MapOtherInfo.end(); ++iter) {
        if (isValueValid(iter.value())) {
            m_LstOtherInfo.append(QPair<QString, QString>(iter.key(), iter.value()));
        }
    }
}
