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

#ifndef EDIDPARSER_H
#define EDIDPARSER_H
#include<QString>
#include<QStringList>
#include<QMap>

/**
 * @brief The EDIDParser class
 * 用于解析edid的类
 */

class EDIDParser
{
public:
    EDIDParser();
    /**
     * @brief setEdid:设置edid原始数据
     * @param edid:edid信息
     * @param errorMsg：错误提示信息
     * @param ch：分隔符
     * @param littleEndianMode：大端、小端
     * @return 布尔值：true-设置成功；false-设置失败
     */
    bool setEdid(const QString &edid, QString &errorMsg, const QString &ch = "\n", bool littleEndianMode = false);

    /**
     * @brief vendor：获取厂商信息
     * @return 厂商信息
     */
    const QString &vendor()const;

    /**
     * @brief releaseDate:获取生产日期
     * @return 生产日期
     */
    const QString &releaseDate()const;

    /**
     * @brief screenSize:获取屏幕大小
     * @return 屏幕大小
     */
    const QString &screenSize()const;

private:

    /**
     * @brief parserVendor:从edid中获取厂商信息
     */
    void parserVendor();

    /**
     * @brief parseReleaseDate:从edid中获取发布日期
     */
    void parseReleaseDate();

    /**
     * @brief parseScreenSize:从edid中获取屏幕大小
     */
    void parseScreenSize();

    /**
     * @brief binToDec:将二进制转换成十进制
     * @param strBin：二进制字符串
     * @return 十进制字符串
     */
    QString binToDec(QString strBin);

    /**
     * @brief decTobin:将十进制转换成二进制
     * @param strDec::十进制字符串
     * @return 二进制字符串
     */
    QString decTobin(QString strDec);

    /**
     * @brief decToHex:将十进制转换成十六进制
     * @param strDec:十进制字符串
     * @return 十六进制字符串
     */
    QString decToHex(QString strDec);

    /**
     * @brief hex2:符号转换
     * @param ch:字符
     * @return 整数
     */
    int hex2(unsigned char ch);

    /**
     * @brief hexToDec:将十六进制转换成十进制
     * @param strHex:十六进制字符串
     * @return 十进制字符串
     */
    QString hexToDec(QString strHex);

    /**
     * @brief hexToBin:十六进制转换成二进制
     * @param strHex：十六进制字符串
     * @return 二进制字符串
     */
    QString hexToBin(QString strHex);

    /**@brief:获取第几个字节的值*/

    /**
     * @brief getBytes:获取第几个字节的值
     * @param l:行数
     * @param n：字节数
     * @return l行第n个字节字符串
     */
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
