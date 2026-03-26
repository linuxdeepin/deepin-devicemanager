// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "EDIDParser.h"
#include "DDLog.h"
#include "commonfunction.h"

// Qt库文件
#include<QLoggingCategory>
#include<QDate>

// 其它头文件
#include<qmath.h>

using namespace DDLog;

EDIDParser::EDIDParser()
    : m_Vendor()
    , m_ReleaseDate()
    , m_ScreenSize()
    , m_LittleEndianMode(true)
    , m_Width(0)
    , m_Height(0)
{
    qCDebug(appLog) << "EDIDParser constructor called";
    m_MapCh.insert("00001", "A");
    m_MapCh.insert("00010", "B");
    m_MapCh.insert("00011", "C");
    m_MapCh.insert("00100", "D");
    m_MapCh.insert("00101", "E");
    m_MapCh.insert("00110", "F");
    m_MapCh.insert("00111", "G");
    m_MapCh.insert("01000", "H");
    m_MapCh.insert("01001", "I");
    m_MapCh.insert("01010", "J");
    m_MapCh.insert("01011", "K");
    m_MapCh.insert("01100", "L");
    m_MapCh.insert("01101", "M");
    m_MapCh.insert("01110", "N");
    m_MapCh.insert("01111", "O");
    m_MapCh.insert("10000", "P");
    m_MapCh.insert("10001", "Q");
    m_MapCh.insert("10010", "R");
    m_MapCh.insert("10011", "S");
    m_MapCh.insert("10100", "T");
    m_MapCh.insert("10101", "U");
    m_MapCh.insert("10110", "V");
    m_MapCh.insert("10111", "W");
    m_MapCh.insert("11000", "X");
    m_MapCh.insert("11001", "Y");
    m_MapCh.insert("11010", "Z");
}

bool EDIDParser::setEdid(const QString &edid, QString &errorMsg, const QString &ch, bool littleEndianMode)
{
    m_LittleEndianMode = littleEndianMode;
    // 判断是否是合理的edid
    if (m_LittleEndianMode) {
        if (!edid.startsWith("00ffffffffffff00")) {
            errorMsg = "Error edid info in little endian mode!";
            return false;
        }
    } else {
        if (!edid.startsWith("ff00ffffffff00ff")) {
            errorMsg = "Error edid info in big endian mode!";
            return false;
        }
    }

    QStringList lines = edid.split(ch);
    foreach (const QString &line, lines) {
        if (line == "")
            continue;

        m_ListEdid.append(line);
    }

    // 解析厂商信息
    qCDebug(appLog) << "Parsing vendor info";
    parseVendor();

    // 解析发布日期
    qCDebug(appLog) << "Parsing release date";
    parseReleaseDate();
    // 解析屏幕尺寸
    qCDebug(appLog) << "Parsing screen size";
    parseScreenSize();
    // 解析监视器名称
    parseMonitorName();

    return true;
}

const QString &EDIDParser::vendor()const
{
    // qCDebug(appLog) << "Getting vendor";
    return m_Vendor;
}

const QString &EDIDParser::model()const
{
    // qCDebug(appLog) << "Getting model";
    return m_Model;
}

const QString &EDIDParser::releaseDate()const
{
    // qCDebug(appLog) << "Getting release date";
    return m_ReleaseDate;
}

const QString &EDIDParser::screenSize()const
{
    // qCDebug(appLog) << "Getting screen size";
    return m_ScreenSize;
}

const QString &EDIDParser::monitorName() const
{
    return m_MonitorName;
}

int EDIDParser::width()
{
    // qCDebug(appLog) << "Getting width";
    return m_Width;
}

int EDIDParser::height()
{
    // qCDebug(appLog) << "Getting height";
    return m_Height;
}

void EDIDParser::parseVendor()
{
    qCDebug(appLog) << "Parsing vendor info from EDID";

    // 获取制造商信息，edid中的 08h 和 09h 是厂商信息
    // 08表示 第0行  第9个字节
    QString vendorStr;
    QString h08 = getBytes(0, 8);
    QString h09 = getBytes(0, 9);
    char h0809[2];
    if (m_LittleEndianMode) {
        qCDebug(appLog) << "Little endian mode";
        vendorStr = h08 + h09;
        h0809[0] = static_cast<char>(hexToDec(h08).toInt());
        h0809[1] = static_cast<char>(hexToDec(h09).toInt());
    } else {
        qCDebug(appLog) << "Big endian mode";
        vendorStr = h09 + h08;
        h0809[0] = static_cast<char>(hexToDec(h09).toInt());
        h0809[1] = static_cast<char>(hexToDec(h08).toInt());
    }

    char name[4];
    name[0] = ((h0809[0] & 0x7C) >> 2) + '@';
    name[1] = ((h0809[0] & 0x03) << 3) + ((h0809[1] & 0xE0) >> 5) + '@';
    name[2] = (h0809[1] & 0x1F) + '@';
    name[3] = 0;

    m_Vendor = QString(name);
    qCDebug(appLog) << "Vendor parsed:" << m_Vendor;

    QString h0a = getBytes(0, 10);
    QString h0b = getBytes(0, 11);

    m_Model = h0a + h0b;
    qCDebug(appLog) << "Model parsed:" << m_Model;

//    // 将16进制的厂商信息转换成二进制的厂商信息
//    QString binStr = hexToBin(vendorStr);

//    // 转换后的二进制厂商信息格式为(6,5,5),如：010110 10011 00011
//    // 分别表示三个字符，就是厂商的缩写
//    QString bin1 = binStr.mid(1, 5);
//    QString bin2 = binStr.mid(6, 5);
//    QString bin3 = binStr.mid(11, 5);

//    // 获取厂商信息
//    m_Vendor = m_MapCh[bin1] +  m_MapCh[bin2] + m_MapCh[bin3];
}

void EDIDParser::parseReleaseDate()
{
    qCDebug(appLog) << "Parsing release date from EDID";

    // edid中的  10H和11H就是发布日期信息
    QString hWeek = getBytes(1, m_LittleEndianMode ? 0 : 1);
    QString hYear = getBytes(1, m_LittleEndianMode ? 1 : 0);

    int week = hexToDec(hWeek).toInt();
    int year = hexToDec(hYear).toInt() + 1990;

    QDate date(year, 1, 1);
    date = date.addDays(week * 7 - 1);
    m_ReleaseDate = date.toString("yyyy-MM");
}

void EDIDParser::parseScreenSize()
{
    qCDebug(appLog) << "Parsing screen size from EDID";

    parseDTDs();
    // edid中的  15H和16H是基本屏幕大小(单位cm), 与 Detailed Timing 相差超10mm 则用15H和16H的。
    int width15 = hexToDec(getBytes(1, m_LittleEndianMode ? 5 : 4)).toInt() * 10;
    int height16 = hexToDec(getBytes(1, m_LittleEndianMode ? 6 : 7)).toInt() * 10;
    if (width15 != 0 && height16 != 0) {
        m_Width = width15;
        m_Height = height16;
        // 从所有的详细尺寸中寻找接近基本屏幕大小的值
        for (int i = 0; i < m_DTDSizeInfoList.size(); ++i) {
            if (abs(m_DTDSizeInfoList.at(i).width - width15) < 10
                && abs(m_DTDSizeInfoList.at(i).height - height16) < 10) {
                m_Width = m_DTDSizeInfoList.at(i).width;
                m_Height = m_DTDSizeInfoList.at(i).height;
                break;
            }
        }
    } else {
        if (!m_DTDSizeInfoList.isEmpty()) {
            m_Width = m_DTDSizeInfoList.at(0).width;
            m_Height = m_DTDSizeInfoList.at(0).height;
        } else {
            m_Width = 0;
            m_Height = 0;
        }
    }

    if (Common::specialComType == Common::kSpecialType7){ // sepcial task:378963
        m_Width = 296;
        m_Height = 197;
    }
    double inch = sqrt((m_Width / 2.54) * (m_Width / 2.54) + (m_Height / 2.54) * (m_Height / 2.54))/10;
    m_ScreenSize = QString("%1 %2(%3mm×%4mm)")
                       .arg(QString::number(inch, '0', Common::specialComType == Common::kSpecialType7 ? 0 : 1))
                       .arg(QObject::tr("inch")).arg(m_Width).arg(m_Height);
    qCDebug(appLog) << "Screen size parsed:" << m_ScreenSize << "Width:" << m_Width << "Height:" << m_Height;
}

void EDIDParser::parseMonitorName()
{
    // EDID中从字节54开始有4个18字节的Descriptor Block
    // 每个Descriptor Block可能包含显示器名称信息
    // 显示器名称的标识符是0xFC（Display Product Name） 注意：部分机型有一些特殊，标识符为0xFE

    // 4个Descriptor Block的起始字节位置
    int descriptorStarts[4] = {54, 72, 90, 108};

    for (int i = 0; i < 4; i++) {
        int startByte = descriptorStarts[i];

        // 计算行号和字节位置（每行16字节）
        int line = startByte / 16;
        int byteInLine = startByte % 16;

        // 获取Descriptor Block的关键字节
        QString byte0 = getBytes(line, byteInLine);      // 第0字节
        QString byte1 = getBytes(line, byteInLine + 1);  // 第1字节
        QString byte3 = getBytes(line, byteInLine + 3);  // 第3字节（类型标识）

        // 检查是否为Display Descriptor (字节0-1为0x0000) 且类型为Display Product Name (字节3为0xFC)
        if (byte0.toUpper() == "00" && byte1.toUpper() == "00" && (byte3.toUpper() == "FC" || byte3.toUpper() == "FE")) {

            // 找到显示器名称描述符，提取ASCII字符串（字节5-17）
            QString monitorName;

            for (int j = 5; j <= 17; j++) {
                int currentByte = startByte + j;
                int currentLine = currentByte / 16;
                int currentByteInLine = currentByte % 16;

                QString charByte = getBytes(currentLine, currentByteInLine);

                if (!charByte.isEmpty()) {
                    int asciiValue = hexToDec(charByte).toInt();

                    // ASCII可打印字符范围：32-126，0x0A为换行符，0x00为结束符
                    if (asciiValue == 0x00 || asciiValue == 0x0A) {
                        break; // 遇到结束符或换行符，停止解析
                    } else if (asciiValue >= 32 && asciiValue <= 126) {
                        monitorName.append(QChar(asciiValue));
                    }
                }
            }

            // 去除首尾空白字符
            m_MonitorName = monitorName.trimmed();

            // 如果找到有效的监视器名称，记录日志并返回
            if (!m_MonitorName.isEmpty())
                return;
        }
    }

    // 如果没有找到有效的监视器名称，设置默认值
    if (m_MonitorName.isEmpty())
        m_MonitorName = "Unknown Monitor";
}

void EDIDParser::parseDTDs()
{
    // 清空之前的 DTD 信息列表
    m_DTDSizeInfoList.clear();

    // 基础块中最多有 4 个 DTD，起始字节为 54, 72, 90, 108
    for (int i = 0; i < 4; i++) {
        int startByte = 54 + i * 18;
        parseOneDTD(startByte, i, true);
    }
}

void EDIDParser::parseOneDTD(int startByte, int dtdIndex, bool isBaseBlock)
{
    // 字节布局（在 DTD 内的相对位置）：
    // 字节0-1: 像素时钟（2字节，用于验证是否为有效 DTD）
    // 字节2-3: H Active（水平主动像素数）低字节 | H Blank
    // 字节4-5: V Active（垂直主动像素数）低字节 | V Blank
    // 字节12: Image Size Width（物理宽度）低8位
    // 字节13: Image Size Height（物理高度）低8位
    // 字节14: 高4位=宽度高4位，低4位=高度高4位

    // 注意：在 EDID 的 16 进制字符串格式中
    // 每行 16 字节 = 32 个字符
    // 每字节 = 2 个字符

    // 注意：大端模式下，字节位置会交换：
    // - 相邻字节对会交换（如 byte12 <-> byte13）
    // - byte14 会变成 byte15

    // 计算 DTD 内的字节位置对应的 EDID 位置
    int line = startByte / 16;
    int byteInLine = startByte % 16;

    // 获取像素时钟（字节0-1），用于验证 DTD 是否有效
    // 大端模式下字节0和字节1交换
    QString pixelClockByte0 = getBytes(line, m_LittleEndianMode ? byteInLine : byteInLine + 1);
    QString pixelClockByte1 = getBytes(line, m_LittleEndianMode ? byteInLine + 1 : byteInLine);

    if (pixelClockByte0.isEmpty() || pixelClockByte1.isEmpty()) {
        return;
    }

    int pixelClockLow = hexToDec(pixelClockByte0).toInt();
    int pixelClockHigh = hexToDec(pixelClockByte1).toInt();
    int pixelClock = pixelClockLow + (pixelClockHigh << 8);

    // 如果像素时钟为 0，说明这不是一个有效的 DTD
    if (pixelClock == 0) {
        return;
    }

    // 获取物理尺寸信息
    // 字节12: 宽度低8位
    // 字节13: 高度低8位
    // 字节14: 高4位=宽度高位，低4位=高度高位
    // 大端模式下：byte12 <-> byte13 交换，byte14 -> byte15

    int byte12Pos = m_LittleEndianMode ? (startByte + 12) : (startByte + 13);
    int byte13Pos = m_LittleEndianMode ? (startByte + 13) : (startByte + 12);
    int byte14Pos = m_LittleEndianMode ? (startByte + 14) : (startByte + 15);

    int lineWidthByte12 = byte12Pos / 16;
    int byteInLineWidthByte12 = byte12Pos % 16;

    int lineWidthByte13 = byte13Pos / 16;
    int byteInLineWidthByte13 = byte13Pos % 16;

    int lineWidthByte14 = byte14Pos / 16;
    int byteInLineWidthByte14 = byte14Pos % 16;

    QString s12 = getBytes(lineWidthByte12, byteInLineWidthByte12);  // 宽度低8位
    QString s13 = getBytes(lineWidthByte13, byteInLineWidthByte13);  // 高度低8位
    QString s14 = getBytes(lineWidthByte14, byteInLineWidthByte14);  // [宽度高4位|高度高4位]

    if (s12.isEmpty() || s13.isEmpty() || s14.isEmpty()) {
        return;
    }

    int byte14_val = hexToDec(s14).toInt();
    // 宽度(mm) = (byte14高4位 << 8) + s12
    int width = ((byte14_val & 0xF0) << 4) + hexToDec(s12).toInt();
    // 高度(mm) = (byte14低4位 << 8) + s13
    int height = ((byte14_val & 0x0F) << 8) + hexToDec(s13).toInt();

    // 保存 DTD 尺寸信息
    DTDSizeInfo info;
    info.dtdIndex = dtdIndex;
    info.width = width;
    info.height = height;
    info.isBaseBlock = isBaseBlock;

    m_DTDSizeInfoList.append(info);
}

QString EDIDParser::binToDec(QString strBin)   //二进制转十进制
{
    // qCDebug(appLog) << "Converting binary to decimal";
    // 二进制转十进制
    QString decimal;
    int nDec = 0, nLen;
    int i, j, k;
    nLen = strBin.length();

    for (i = 0; i < nLen; i++) {
        if (strBin[nLen - i - 1] == "0")
            continue;
        else {
            k = 1;
            for (j = 0; j < i; j++)
                k = k * 2;
            nDec += k;
        }
    }

    decimal = QString::number(nDec);
    // qCDebug(appLog) << "Converted binary" << strBin << "to decimal" << decimal;
    return decimal;
}

QString EDIDParser::decTobin(QString strDec)   //十进制转二进制
{
    // qCDebug(appLog) << "Converting decimal to binary";
    int nDec = strDec.toInt();
    int nYushu = 0;
    int nShang = 0;
    QString strBin, strTemp;

    bool bContinue = true;
    while (bContinue) {
        nYushu = nDec % 2;
        nShang = nDec / 2;
        strBin = QString::number(nYushu) + strBin; //qCInfo(appLog)<<strBin;
        strTemp = strBin;
        //strBin.Format("%s%s", buf, strTemp);
        nDec = nShang;
        if (nShang == 0) {
            // qCDebug(appLog) << "Decimal to binary conversion finished.";
            bContinue = false;
        }
    }
    int nTemp = strBin.length() % 4;
    switch (nTemp) {
    case 1:
        //strTemp.Format(_T("000%s"), strBin);
        strTemp = "000" + strBin;
        strBin = strTemp;
        break;
    case 2:
        //strTemp.Format(_T("00%s"), strBin);
        strTemp = "00" + strBin;
        strBin = strTemp;
        break;
    case 3:
        //strTemp.Format(_T("0%s"), strBin);
        strTemp = "0" + strBin;
        strBin = strTemp;
        break;
    default:
        break;
    }
    return strBin;
}

QString EDIDParser::decToHex(QString strDec)   //十进制转十六进制
{
    // qCDebug(appLog) << "Converting decimal to hexadecimal";
    int hex = strDec.toInt();
    QString hex1 = QString("%1").arg(hex, 8, 16, QLatin1Char('0'));
    return hex1;
}

int EDIDParser::hex2(unsigned char ch)           //十六进制转换工具
{
    // qCDebug(appLog) << "Converting hexadecimal to decimal";
    if ((ch >= '0') && (ch <= '9'))
        return ch - 0x30;
    else if ((ch >= 'A') && (ch <= 'F'))
        return ch - 'A' + 10;
    else if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;
    return 0;
}

QString EDIDParser::hexToDec(QString strHex)   //十六进制转十进制
{
    // qCDebug(appLog) << "Converting hexadecimal to decimal";
    int i;
    int v = 0;
    for (i = 0; i < strHex.length(); i++) {
        v *= 16;
        v += hex2(strHex[i].toLatin1());
    }
    return QString::number(v);
}

QString EDIDParser::hexToBin(QString strHex)
{
    // qCDebug(appLog) << "Converting hexadecimal to binary";
    // 十六进制转二进制
    QString dec = hexToDec(strHex);
    return decTobin(dec);
}

QString EDIDParser::getBytes(int l, int n)
{
    // qCDebug(appLog) << "Getting bytes";
    // 获取指定字节
    int index = n * 2;
    if (m_ListEdid.size() < l + 1) {
        qCWarning(appLog) << "Line index" << l << "is out of bounds.";
        return "";
    }

    QString line = m_ListEdid[l];
    if (line.length() < n + 1) {
        qCWarning(appLog) << "Byte index" << n << "is out of bounds for line" << l;
        return "";
    }

    return line.mid(index, 2);
}
