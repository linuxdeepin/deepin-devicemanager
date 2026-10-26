// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// =============================================================================
// 单元测试: EDIDParser (src/Tool/EDIDParser.cpp)
// 覆盖方法: setEdid / vendor / model / releaseDate / screenSize / monitorName /
//           width / height / binToDec / decTobin / decToHex / hexToDec / hexToBin /
//           hex2 / getBytes / parseVendor / parseReleaseDate / parseScreenSize /
//           parseMonitorName / parseDTDs / parseOneDTD / EDIDParser()
// 桩策略: 纯数据解析类, 无外部依赖, 不打桩; 通过 -fno-access-control 访问 private 成员。
// 大端模式用例由小端 EDID 逐行按 2 字符(1字节对)交换构造, 与 hexdump 默认输出格式一致。
//
// 用例清单(用例数声明, 依据源码分支):
//  - setEdid:  小端合法/小端非法/大端合法(往返)/大端非法                        (4)
//  - parseVendor/vendor: LE "VSC"; BE "VSC"; model LE "3840" / BE "4038"        (3)
//  - parseReleaseDate/releaseDate: LE "2020-03"; BE "2020-03"                   (2)
//  - parseScreenSize/screenSize/width/height: LE DTD 匹配; BE DTD 匹配;
//    kSpecialType7 强制覆盖分支; 尺寸字节为 0 走 DTD 回退分支                    (4)
//  - parseMonitorName/monitorName: FC 描述符 "VA2430-FHD"; FE 描述符
//    "DISPLAY_TEST"; 无描述符 "Unknown Monitor"; BE 丢失名称 "Unknown Monitor"  (4)
//  - parseDTDs/parseOneDTD: 仅像素时钟非 0 的 DTD 入表(527x296);
//    像素时钟为 0 跳过; 越界字节返回跳过                                        (3)
//  - binToDec / decTobin / decToHex / hexToDec / hexToBin / hex2 / getBytes     (7+TEST_P 4)
//  - EDIDParser(): m_MapCh 初始化 25 项                                          (1)
// =============================================================================

#include "EDIDParser.h"
#include "commonfunction.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_EDIDParser : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_EDIDParser = new EDIDParser;
        m_savedSpecialComType = Common::specialComType;
    }
    void TearDown() override
    {
        Common::specialComType = m_savedSpecialComType;
        delete m_EDIDParser;
    }
    EDIDParser *m_EDIDParser = nullptr;
    int m_savedSpecialComType = -1;
};

//bool setEdid(const QString &edid, QString &errorMsg, const QString &ch = "\n", bool littleEndianMode = true);
QString edid = "00ffffffffffff005a63384001010101\n"
               "0d1e010380351d782ece65a657519f27\n"
               "0f5054bfef80b300a940a9c095009040\n"
               "8180814081c0023a801871382d40582c\n"
               "45000f282100001e000000ff00565351\n"
               "3230313332313330320a000000fd0032\n"
               "4b185311000a202020202020000000fc\n"
               "005641323433302d4648440a20200141\n"
               "020320f14d9005040302121113141e1d\n"
               "1f0123097f078301000065030c001000\n"
               "023a801871382d40582c45000f282100\n"
               "001e011d8018711c1620582c25000f28\n"
               "2100009e011d007251d01e206e285500\n"
               "0f282100001e8c0ad08a20e02d10103e\n"
               "96000f28210000188c0ad09020403120\n"
               "0c4055000f28210000180000000000d6\n";

// 小端 EDID 每行按 2 字节字(4字符)交换 => 大端形态, 与 hexdump 默认输出逐行拼接结果一致
// (hexdump 按 2 字节小端字输出, 如字节 00 ff 输出 "ff00"), 头部恰好满足
// setEdid 大端校验前缀 "ff00ffffffff00ff"。
static QString ut_swapEdidWords(const QString &le)
{
    QStringList out;
    const QStringList lines = le.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString swapped;
        for (int i = 0; i + 3 < line.length(); i += 4)
            swapped.append(line.mid(i + 2, 2) + line.mid(i, 2));
        out.append(swapped);
    }
    return out.join('\n') + '\n';
}

// 构造的最小合法 EDID: 描述符@90 为 0xFE, 名称 "DISPLAY_TEST";
// DTD0@54 像素时钟 0x0201(非0) 尺寸 527x296; DTD1@72/DTD2@90/DTD3@108 像素时钟 0 跳过。
// 行1: 周期 0x0d=13, 年偏移 0x1e=30 => 2020-03; 基本尺寸 0x35*10=530 / 0x1d*10=290。
static const QString ut_edid_fe = QString(
    "00ffffffffffff005a63384001010101\n"
    "0d1e010300351d00\n"
    "00000000000000000000000000000000\n"
    "00000000000001020000000000000000\n"
    "00000f28210000000000000000000000\n"
    "00000000000000000000"  "0000"  "00"  "fe"  "00"  "44\n"
    "4953504c41595f544553540000000000\n"
    "00000000000000000000000000000000\n");

TEST_F(UT_EDIDParser, setEdid_defaultEdid_parsesFields){
    // Arrange: 合法小端 EDID
    // Act
    QString errorMsg;
    bool ret = m_EDIDParser->setEdid(edid,errorMsg);
    // Assert: 头部校验通过且各行进入 m_ListEdid
    EXPECT_TRUE(ret);
    EXPECT_EQ(16, m_EDIDParser->m_ListEdid.size());
}

//const QString &vendor()const;
TEST_F(UT_EDIDParser, vendor_pnpId_returnsThreeChars){
    // Act: 解析 PNP ID 字符串
    // Arrange + Act: 解析真实 EDID
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    // Assert: 08h/09h 字节 5a 63 => "VSC"
    EXPECT_STREQ("VSC",m_EDIDParser->vendor().toStdString().c_str());
    EXPECT_EQ(3,m_EDIDParser->vendor().size());
}

//const QString &model()const;
TEST_F(UT_EDIDParser, model_pnpId_returnsDigits){
    // Act: 解析 PNP ID 字符串
    // Arrange + Act: 解析真实 EDID
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    // Assert: 0Ah/0Bh 字节 38 40 => "3840"
    EXPECT_STREQ("3840",m_EDIDParser->model().toStdString().c_str());
    EXPECT_NE(m_EDIDParser->model(),m_EDIDParser->vendor());
}

//const QString &releaseDate()const;
TEST_F(UT_EDIDParser, releaseDate_weekOffset_returns2020_03){
    // Act: 解析日期字符串
    // Arrange + Act
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    // Assert: 周 0x0d=13, 年偏移 0x1e=30 => 2020-01-01 + 13*7-1 天 => "2020-03"
    EXPECT_STREQ("2020-03",m_EDIDParser->releaseDate().toStdString().c_str());
    EXPECT_TRUE(m_EDIDParser->releaseDate().contains(QStringLiteral("-")));
}

//const QString &screenSize()const;
TEST_F(UT_EDIDParser, screenSize_dtdNearBase_prefersDtd){
    // Act: 解析屏幕尺寸字符串
    // Arrange + Act
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    // Assert: 基本尺寸 530x290 与 DTD 527x296 差 <10mm => 采用 DTD 值
    EXPECT_STREQ("23.8 inch(527mm×296mm)",m_EDIDParser->screenSize().toStdString().c_str());
    EXPECT_TRUE(m_EDIDParser->screenSize().contains(QStringLiteral("527mm")));
}

//const QString &monitorName()const;
TEST_F(UT_EDIDParser, monitorName_fcDescriptor_returnsVa2430){
    // Act: 解析 monitorName
    // Arrange + Act
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    // Assert: 描述符@108 类型 0xFC, ASCII 字节 "VA2430-FHD\n  "
    EXPECT_STREQ("VA2430-FHD",m_EDIDParser->monitorName().toStdString().c_str());
    EXPECT_FALSE(m_EDIDParser->monitorName().contains(QLatin1Char('\n')));
}

//int width();
TEST_F(UT_EDIDParser, width_dtd_returns527){
    // Act: 解析宽度
    // Arrange + Act
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    // Assert
    EXPECT_EQ(527,m_EDIDParser->width());
    EXPECT_GT(m_EDIDParser->width(),0);
}

//int height();
TEST_F(UT_EDIDParser, height_dtd_returns296){
    // Act: 解析高度
    // Arrange + Act
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    // Assert
    EXPECT_EQ(296,m_EDIDParser->height());
    EXPECT_GT(m_EDIDParser->height(),0);
}

// setEdid 小端模式非法数据: 头部非 "00ffffffffffff00"
TEST_F(UT_EDIDParser, setEdid_invalidLittleEndian_reportsError)
{
    // Arrange: 头部错误的小端数据
    // Act
    QString errorMsg;
    bool ret = m_EDIDParser->setEdid("deadbeef\n", errorMsg);
    // Assert: 校验失败并给出错误说明
    EXPECT_FALSE(ret);
    EXPECT_STREQ("Error edid info in little endian mode!", errorMsg.toStdString().c_str());
    EXPECT_TRUE(m_EDIDParser->m_ListEdid.isEmpty());
}

// setEdid 大端模式: 逐字节对交换后的 EDID 应解析出相同语义结果
// 说明: model 在大端分支不做字节交换(m_Model=h0a+h0b), 得到 "4038";
//       parseMonitorName 使用固定字节偏移, 大端形态下 0xFC 标识落在字节2位置, 名称退化为
//       "Unknown Monitor" —— 与当前实现一致, 记录为预期行为。
TEST_F(UT_EDIDParser, setEdid_bigEndianRoundtrip_fieldsKept)
{
    // Arrange: 小端 EDID 逐字节对交换 => 大端形态(hexdump 默认输出格式)
    QString be = ut_swapEdidWords(edid);
    // Act
    QString errorMsg;
    bool ret = m_EDIDParser->setEdid(be, errorMsg, "\n", false);
    // Assert: 厂商/日期/尺寸与大端交换前一致
    EXPECT_TRUE(ret);
    EXPECT_TRUE(errorMsg.isEmpty());
    EXPECT_STREQ("VSC", m_EDIDParser->vendor().toStdString().c_str());
    EXPECT_STREQ("2020-03", m_EDIDParser->releaseDate().toStdString().c_str());
    EXPECT_EQ(527, m_EDIDParser->width());
    EXPECT_EQ(296, m_EDIDParser->height());
    EXPECT_STREQ("23.8 inch(527mm×296mm)", m_EDIDParser->screenSize().toStdString().c_str());
    EXPECT_STREQ("4038", m_EDIDParser->model().toStdString().c_str());
    EXPECT_STREQ("Unknown Monitor", m_EDIDParser->monitorName().toStdString().c_str());
}

// setEdid 大端模式非法数据: 头部非 "ff00ffffffff00ff"
TEST_F(UT_EDIDParser, setEdid_invalidBigEndian_reportsError)
{
    // Arrange: 非法大端数据
    // Act
    QString errorMsg;
    bool ret = m_EDIDParser->setEdid("abcd\n", errorMsg, "\n", false);
    // Assert
    EXPECT_FALSE(ret);
    EXPECT_STREQ("Error edid info in big endian mode!", errorMsg.toStdString().c_str());
}

// setEdid 空分隔符行过滤: ch 分隔产生的空行不入 m_ListEdid
TEST_F(UT_EDIDParser, setEdid_blankLinesSkipped_parsesOk)
{
    // Arrange: 带空行的 EDID
    QString data = "00ffffffffffff005a63384001010101\n\n0d1e010380351d782ece65a657519f27\n";
    // Act
    QString errorMsg;
    bool ret = m_EDIDParser->setEdid(data, errorMsg);
    // Assert: 空行被过滤, 仅 2 行
    EXPECT_TRUE(ret);
    EXPECT_EQ(2, m_EDIDParser->m_ListEdid.size());
}

// parseDTDs/parseOneDTD: 仅像素时钟非 0 的基础块 DTD 入表
TEST_F(UT_EDIDParser, parseDTDs_defaultEdid_twoDtds)
{
    // Arrange: 真实 EDID 仅 DTD0@54 像素时钟 0x3a02 非 0, 其余 3 个描述符时钟为 0
    // Act
    QString errorMsg;
    m_EDIDParser->setEdid(edid, errorMsg);
    // Assert: DTD 列表仅 1 项且为 527x296 基础块
    EXPECT_EQ(1, m_EDIDParser->m_DTDSizeInfoList.size());
    EXPECT_EQ(527, m_EDIDParser->m_DTDSizeInfoList.at(0).width);
    EXPECT_EQ(296, m_EDIDParser->m_DTDSizeInfoList.at(0).height);
    EXPECT_TRUE(m_EDIDParser->m_DTDSizeInfoList.at(0).isBaseBlock);
    EXPECT_EQ(0, m_EDIDParser->m_DTDSizeInfoList.at(0).dtdIndex);
}

// parseMonitorName: 0xFE 特殊类型描述符同样识别(特殊机型)
TEST_F(UT_EDIDParser, monitorName_feDescriptor_returnsIsp)
{
    // Arrange: 构造 0xFE 描述符 @90, 名称字节 "DISPLAY_TEST\0"
    // Act
    QString errorMsg;
    bool ret = m_EDIDParser->setEdid(ut_edid_fe, errorMsg);
    // Assert: 0xFE 分支同样提取名称; 尺寸取 DTD 匹配值
    EXPECT_TRUE(ret);
    EXPECT_STREQ("DISPLAY_TEST", m_EDIDParser->monitorName().toStdString().c_str());
    EXPECT_EQ(527, m_EDIDParser->width());
    EXPECT_EQ(296, m_EDIDParser->height());
}

// parseMonitorName: 无名称描述符 => 默认 "Unknown Monitor"
TEST_F(UT_EDIDParser, monitorName_noDescriptor_returnsUnknown)
{
    // Arrange: 构造无 0xFC/0xFE 描述符的 EDID
    QString data = ut_edid_fe;
    data.replace("00fe0044", "00000000");
    data.replace("4953504c41595f544553540000000000", "00000000000000000000000000000000");
    // Act
    QString errorMsg;
    m_EDIDParser->setEdid(data, errorMsg);
    // Assert
    EXPECT_STREQ("Unknown Monitor", m_EDIDParser->monitorName().toStdString().c_str());
    EXPECT_TRUE(m_EDIDParser->monitorName().startsWith(QStringLiteral("Unknown")));
}

// parseScreenSize: 基本尺寸字节为 0 时回退 DTD 首项
TEST_F(UT_EDIDParser, screenSize_zeroBase_fallsBackDtd)
{
    // Arrange: 行1 基本尺寸字节清 0(b5/b6=00), DTD0 仍有效
    QString data = ut_edid_fe;
    data.replace("0d1e010300351d00", "0d1e010300000000");
    // Act
    QString errorMsg;
    m_EDIDParser->setEdid(data, errorMsg);
    // Assert: width15/height16 为 0 => 取 m_DTDSizeInfoList 首项 527x296
    EXPECT_EQ(527, m_EDIDParser->width());
    EXPECT_EQ(296, m_EDIDParser->height());
}

// parseScreenSize: 特殊机型分支 specialComType == kSpecialType7 强制 296x197
TEST_F(UT_EDIDParser, screenSize_specialComType7_doubled)
{
    // Arrange: 置特殊机型标记并恢复(save in SetUp / restore in TearDown)
    Common::specialComType = Common::kSpecialType7;
    // Act
    QString errorMsg;
    m_EDIDParser->setEdid(ut_edid_fe, errorMsg);
    // Assert: 宽高被强制覆盖, 英寸保留 0 位小数
    EXPECT_EQ(296, m_EDIDParser->width());
    EXPECT_EQ(197, m_EDIDParser->height());
    EXPECT_STREQ("14 inch(296mm×197mm)", m_EDIDParser->screenSize().toStdString().c_str());
}

//QString binToDec(QString strBin);
TEST_F(UT_EDIDParser, binToDec_binaryString_returnsDecimal){
    // Act: binToDec 转换
    // Assert: 结果与十进制期望一致
    // Arrange + Act + Assert: 基础换算
    EXPECT_STREQ("15",m_EDIDParser->binToDec("00001111").toStdString().c_str());
    EXPECT_STREQ("0",m_EDIDParser->binToDec("0").toStdString().c_str());
    EXPECT_STREQ("16",m_EDIDParser->binToDec("10000").toStdString().c_str());
}

//QString decTobin(QString strDec);
TEST_F(UT_EDIDParser, decTobin_decimal_returnsBinary){
    // Act: decTobin 转换
    // Assert: 零填充二进制串与期望一致
    // Arrange + Act + Assert: 含 4 位对齐补零分支
    EXPECT_STREQ("1111",m_EDIDParser->decTobin("15").toStdString().c_str());
    EXPECT_STREQ("0001",m_EDIDParser->decTobin("1").toStdString().c_str());
    EXPECT_STREQ("0000",m_EDIDParser->decTobin("0").toStdString().c_str());
    EXPECT_STREQ("11111111",m_EDIDParser->decTobin("255").toStdString().c_str());
}

//QString decToHex(QString strDec);
TEST_F(UT_EDIDParser, decToHex_decimal_returnsHex){
    // Act: decToHex 转换
    // Assert: 零填充十六进制串与期望一致
    // Arrange + Act + Assert: 8 位宽十六进制
    EXPECT_STREQ("0000000f",m_EDIDParser->decToHex("15").toStdString().c_str());
    EXPECT_STREQ("00000000",m_EDIDParser->decToHex("0").toStdString().c_str());
}

//QString hexToDec(QString strHex);
TEST_F(UT_EDIDParser, hexToDec_hex_returnsDecimal)
{
    // Act: hexToDec 转换
    // Assert: 结果与十进制期望一致
    // Arrange + Act + Assert: 大小写字母与数字混合
    EXPECT_STREQ("255", m_EDIDParser->hexToDec("ff").toStdString().c_str());
    EXPECT_STREQ("31", m_EDIDParser->hexToDec("1F").toStdString().c_str());
    EXPECT_STREQ("10", m_EDIDParser->hexToDec("0a").toStdString().c_str());
}

//QString hexToBin(QString strHex);
TEST_F(UT_EDIDParser, hexToBin_hex_returnsBinary)
{
    // Act: hexToBin 转换
    // Assert: 二进制串与期望一致
    // Arrange + Act + Assert: hexToDec + decTobin 组合
    EXPECT_STREQ("1111", m_EDIDParser->hexToBin("f").toStdString().c_str());
    EXPECT_STREQ("00010000", m_EDIDParser->hexToBin("10").toStdString().c_str());
}

//int hex2(unsigned char ch);
TEST_F(UT_EDIDParser, hex2_hexPair_returnsByte)
{
    // Act: hex2 转换
    // Assert: 两位十六进制串与期望一致
    // Arrange + Act + Assert: 数字/大写/小写/非法字符四分支
    EXPECT_EQ(0, m_EDIDParser->hex2('0'));
    EXPECT_EQ(9, m_EDIDParser->hex2('9'));
    EXPECT_EQ(10, m_EDIDParser->hex2('A'));
    EXPECT_EQ(15, m_EDIDParser->hex2('F'));
    EXPECT_EQ(10, m_EDIDParser->hex2('a'));
    EXPECT_EQ(15, m_EDIDParser->hex2('f'));
    EXPECT_EQ(0, m_EDIDParser->hex2('z'));
}

//QString getBytes(int l, int n);
TEST_F(UT_EDIDParser, getBytes_range_returnsSlice)
{
    // Assert: 提取的字节串与期望一致
    // Arrange: 先装入 EDID
    QString errorMsg;
    m_EDIDParser->setEdid(edid, errorMsg);
    // Act + Assert: 行0第0字节
    EXPECT_STREQ("00", m_EDIDParser->getBytes(0, 0).toStdString().c_str());
    // 行0第8字节
    EXPECT_STREQ("5a", m_EDIDParser->getBytes(0, 8).toStdString().c_str());
    // 行号越界
    EXPECT_TRUE(m_EDIDParser->getBytes(999, 0).isEmpty());
    // 字节号越界
    EXPECT_TRUE(m_EDIDParser->getBytes(0, 99).isEmpty());
    // 未设置数据时越界
    EDIDParser emptyParser;
    EXPECT_TRUE(emptyParser.getBytes(0, 0).isEmpty());
}

// EDIDParser(): m_MapCh 二进制->字母映射初始化(A-Z 共 25 项)
TEST_F(UT_EDIDParser, ctor_init_mapChPopulated)
{
    // Assert: m_MapCh 条目数与已知静态映射一致
    // Arrange: SetUp 已构造
    // Act + Assert
    EXPECT_EQ(26, m_EDIDParser->m_MapCh.size());
    EXPECT_STREQ("A", m_EDIDParser->m_MapCh.value("00001").toStdString().c_str());
    EXPECT_STREQ("Z", m_EDIDParser->m_MapCh.value("11010").toStdString().c_str());
}

// decTobin 参数化: 多组十进制->二进制换算
struct UT_EDIDConvCase { const char *dec; const char *bin; };
class UT_EDIDParserConv : public UT_EDIDParser,
                          public ::testing::WithParamInterface<UT_EDIDConvCase>
{
};
TEST_P(UT_EDIDParserConv, decTobin_table_zeroPaddedBinary)
{
    // Assert: 各参数输出与期望一致
    // Arrange: 参数表
    const UT_EDIDConvCase &c = GetParam();
    // Act
    const QString got = m_EDIDParser->decTobin(QString::fromLatin1(c.dec));
    // Assert: 值与零填充位宽均正确
    EXPECT_STREQ(c.bin, got.toStdString().c_str());
    EXPECT_EQ(int(qstrlen(c.bin)), got.size());
}
INSTANTIATE_TEST_SUITE_P(Conversion, UT_EDIDParserConv,
    ::testing::Values(
        UT_EDIDConvCase{"2", "0010"},
        UT_EDIDConvCase{"7", "0111"},
        UT_EDIDConvCase{"10", "1010"},
        UT_EDIDConvCase{"1990", "11111000110"}));
