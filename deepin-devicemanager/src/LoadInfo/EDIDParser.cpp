// 项目自身文件
#include "EDIDParser.h"

// Qt库文件
#include<QDebug>
#include<QDate>

// 其它头文件
#include<qmath.h>

EDIDParser::EDIDParser()
    : m_LittleEndianMode(true)
{
    // 判断大小端
//    int a = 0x12345678;
//    char *c = (char *)&a;
//    for (int i = 0; i < 4; i++) {
//        printf("%x\n", c[i]);
//    }
//    m_LittleEndianMode = ((c[0] == 0x78) && (c[1] == 0x56) && (c[2] == 0x34) && (c[3] == 0x12));

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
            errorMsg = "Error edid info";
            return false;
        }
    } else {
        if (!edid.startsWith("ff00ffffffff00ff")) {
            errorMsg = "Error edid info";
            return false;
        }
    }

    QStringList lines = edid.split(ch);
    foreach (const QString &line, lines) {
        if (line == "") {
            continue;
        }
        m_ListEdid.append(line);
    }

    // 解析厂商信息
    parserVendor();
    // 解析发布日期
    parseReleaseDate();
    // 解析屏幕尺寸
    parseScreenSize();


    return true;
}

const QString &EDIDParser::vendor()const
{
    return m_Vendor;
}

const QString &EDIDParser::releaseDate()const
{
    return m_ReleaseDate;
}

const QString &EDIDParser::screenSize()const
{
    return m_ScreenSize;
}

void EDIDParser::parserVendor()
{
    // 获取制造商信息，edid中的 08h 和 09h 是厂商信息
    // 08表示 第0行  第9个字节
    QString vendorStr;
    QString h08 = getBytes(0, 8);
    QString h09 = getBytes(0, 9);
    if (m_LittleEndianMode) {
        vendorStr = h08 + h09;
    } else {
        vendorStr = h09 + h08;
    }

    // 将16进制的厂商信息转换成二进制的厂商信息
    QString binStr = hexToBin(vendorStr);

    // 转换后的二进制厂商信息格式为(6,5,5),如：010110 10011 00011
    // 分别表示三个字符，就是厂商的缩写
    QString bin1 = binStr.mid(1, 5);
    QString bin2 = binStr.mid(6, 5);
    QString bin3 = binStr.mid(11, 5);

    // 获取厂商信息
    m_Vendor = m_MapCh[bin1] +  m_MapCh[bin2] + m_MapCh[bin3];
}

void EDIDParser::parseReleaseDate()
{
    // edid中的  10H和11H就是发布日期信息
    QString hWeek = getBytes(1, m_LittleEndianMode ? 0 : 1);
    QString hYear = getBytes(1, m_LittleEndianMode ? 1 : 0);

    int week = hexToDec(hWeek).toInt();
    int year = hexToDec(hYear).toInt() + 1990;

    QDate date(year, 1, 1);
    date = date.addDays(week * 7 + 1);
    m_ReleaseDate = date.toString("yyyy年MM月");
}

void EDIDParser::parseScreenSize()
{
    // edid中的  15H和16H就是屏幕大小
    int width = hexToDec(getBytes(1, m_LittleEndianMode ? 5 : 4)).toInt();
    int height = hexToDec(getBytes(1, m_LittleEndianMode ? 6 : 7)).toInt();
    double inch = sqrt((width / 2.54) * (width / 2.54) + (height / 2.54) * (height / 2.54));
    m_ScreenSize = QString("%1英寸(%2cm * %3cm)").arg(QString::number(inch, 'f', 1)).arg(width).arg(height);
}

QString EDIDParser::binToDec(QString strBin)   //二进制转十进制
{
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
    return decimal;
}

QString EDIDParser::decTobin(QString strDec)   //十进制转二进制
{
    int nDec = strDec.toInt();
    int nYushu = 0;
    int nShang = 0;
    QString strBin, strTemp;
    //TCHAR buf[2];
    bool bContinue = true;
    while (bContinue) {
        nYushu = nDec % 2;
        nShang = nDec / 2;
        strBin = QString::number(nYushu) + strBin; //qInfo()<<strBin;
        strTemp = strBin;
        //strBin.Format("%s%s", buf, strTemp);
        nDec = nShang;
        if (nShang == 0)
            bContinue = false;
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
    int hex = strDec.toInt();
    QString hex1 = QString("%1").arg(hex, 8, 16, QLatin1Char('0'));
    return hex1;
}

int EDIDParser::hex2(unsigned char ch)           //十六进制转换工具
{
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
    QString dec = hexToDec(strHex);
    return decTobin(dec);
}

QString EDIDParser::getBytes(int l, int n)
{
    int index = n * 2;
    if (m_ListEdid.size() < l + 1) {
        return "";
    }
    QString line = m_ListEdid[l];
    if (line.length() < n + 1) {
        return "";
    }
    return line.mid(index, 2);
}
