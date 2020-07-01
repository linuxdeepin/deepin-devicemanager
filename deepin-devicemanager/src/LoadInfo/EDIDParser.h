#ifndef EDIDPARSER_H
#define EDIDPARSER_H
#include<QString>
#include<QStringList>
#include<QMap>

class EDIDParser
{
public:
    EDIDParser();

    /**@brief:设置edid原始数据*/
    bool setEdid(const QString &edid, QString &errorMsg, const QString &ch = "\n", bool littleEndianMode = false);

    /**@brief:获取厂商信息*/
    const QString &vendor()const;
    /**@brief:获取生产日期*/
    const QString &releaseDate()const;
    /**@brief:获取屏幕大小*/
    const QString &screenSize()const;

private:
    /**@brief:从edid中获取厂商信息*/
    void parserVendor();
    /**@brief:从edid中获取发布日期*/
    void parseReleaseDate();
    /**@brief:从edid中获取屏幕大小*/
    void parseScreenSize();


    /**@brief:将二进制转换成十进制*/
    QString binToDec(QString strBin);
    /**@brief:将十进制转换成二进制*/
    QString decTobin(QString strDec);
    /**@brief:将十进制转换成十六进制*/
    QString decToHex(QString strDec);
    /**@brief:符号转换*/
    int hex2(unsigned char ch);
    /**@brief:将十六进制转换成十进制*/
    QString hexToDec(QString strHex);
    /**@brief:将十六进制转换成二进制*/
    QString hexToBin(QString strHex);

    /**@brief:获取第几个字节的值*/
    QString getBytes(int l, int n);


private:
    /**@brief:机器的存储模式不同，会导致计算结果不同，所以在解析的时候需要考虑大小端模式*/
    QString                m_Vendor;                           // 显示屏的厂商信息
    QString                m_ReleaseDate;                      // 显示屏的生产日期
    QString                m_ScreenSize;                       // 屏幕大小
    bool                   m_LittleEndianMode;                 // 小端模式
    QStringList            m_ListEdid;                         // edid数据
    QMap<QString, QString> m_MapCh;                            //

};

#endif // EDIDPARSER_H
