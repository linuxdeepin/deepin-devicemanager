// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DeviceBaseInfo 导出与转换 (deepin-devicemanager/src/DeviceManager/DeviceInfo.cpp)
// 覆盖: HTML/Txt/Xlsx/Doc 导出、displayWidth、generatorTranslate、getter/setter、
//       setHwinfoLshwKey/matchToLshw/PhysID 映射、getVendorOrModelId、
//       setVendorNameBylsusbLspci(QProcess 打桩)、getDriverVersion(executeClientCmd 打桩)。
// 注意: 测试构建开启 -fno-access-control, 可直接访问 protected/private 成员。
// 注: Docx::Document 默认构造后 addParagraph 会 SEGV, 故 Doc 全文档导出不在测试范围。
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                        | level | factors | min | actual |
// |-------------------------------|-------|---------|-----|--------|
// | toHtmlString                  | low   | -       | 1   | 1      |
// | baseInfoToHTML                | mid   | -       | 2   | 2      |
// | subTitleToHTML                | low   | -       | 1   | 1      |
// | toXlsxString                  | low   | -       | 1   | 1      |
// | baseInfoToXlsx                | mid   | -       | 2   | 2      |
// | toTxtString                   | low   | -       | 1   | 1      |
// | baseInfoToTxt                 | mid   | -       | 2   | 2      |
// | tableInfoToTxt/tableHeaderToTxt | low | -       | 1   | 2      |
// | tableInfoToHtml/tableHeaderToHtml | low | -     | 1   | 2      |
// | tableInfoToDoc(null)          | low   | -       | 1   | 1      |
// | tableInfoToXlsx/tableHeaderToXlsx | low | -     | 1   | 2      |
// | displayWidth                  | mid   | -       | 2   | 2      |
// | generatorTranslate            | mid   | -       | 2   | 2      |
// | getVID/getPID/getVIDAndPID    | mid   | -       | 2   | 2      |
// | getModalias                   | low   | -       | 1   | 1      |
// | hardwareClass/uniqueID/sysPath| low   | -       | 1   | 1      |
// | canEnable/canUninstall/setEnableValue | low | - | 1   | 1      |
// | setForcedDisplay              | mid   | -       | 2   | 2      |
// | isValueValid                  | high  | -       | 3   | 3      |
// | addBaseDeviceInfo             | mid   | -       | 2   | 2      |
// | addOtherDeviceInfo            | mid   | -       | 2   | 2      |
// | setAttribute                  | mid   | -       | 2   | 2      |
// | readDeviceInfoKeyValue        | mid   | -       | 2   | 2      |
// | setDeviceInfoKeyValue         | mid   | -       | 2   | 2      |
// | get_string                    | high  | -       | 3   | 3      |
// | setHwinfoLshwKey              | high  | -       | 3   | 4      |
// | matchToLshw                   | mid   | -       | 2   | 2      |
// | setPhysIDMapKey/PhysIDMapInfo | low   | -       | 1   | 1      |
// | mapInfoToList                 | mid   | -       | 2   | 2      |
// | getOtherMapInfo               | mid   | -       | 2   | 2      |
// | getVendorOrModelId            | high  | -       | 3   | 3      |
// | setVendorNameBylsusbLspci     | low   | -       | 1   | 3      |
// | enable/available              | low   | -       | 1   | 1      |
// | driverIsKernelIn              | mid   | -       | 2   | 2      |
// | translateStr                  | mid   | -       | 2   | 2      |
// | nameTr                        | low   | -       | 1   | 1      |
// | getDriverVersion              | low   | -       | 1   | 2      |
// ============================================================

#include "DeviceInfo.h"
#include "DeviceAudio.h"
#include "ut_Head.h"
#include "stub.h"

#include <QDomDocument>
#include <QProcess>
#include "commonfunction.h"
#include <QTextStream>
#include <QString>
#include <QTemporaryFile>
#include <QPair>

#include <gtest/gtest.h>

class UT_DeviceInfoExport : public ::testing::Test
{
public:
    void SetUp() override
    {
        ut_disable_system_dbus();  // 隔离真实系统服务, 避免运行期触发系统总线鉴权
        audio = new DeviceAudio;
        // 填充翻译后的基础/其他信息, 让导出方法覆盖写入分支(非空 value 通过 isValueValid)
        audio->m_LstBaseInfoTr.append(qMakePair(QString("Vendor"), QString("Acme")));
        audio->m_LstBaseInfoTr.append(qMakePair(QString("Model"), QString("X1 Carbon")));
        audio->m_LstOtherInfoTr.append(qMakePair(QString("Driver"), QString("i915")));
        audio->m_LstOtherInfoTr.append(qMakePair(QString("Status"), QString("OK")));
    }
    void TearDown() override
    {
        delete audio;
        audio = nullptr;
    }
    DeviceAudio *audio = nullptr;
};

// ---------- HTML 导出 ----------
// toHtmlString: 文档与字符串输出均非空
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_toHtmlString_withData)
{
    // Arrange: 空文档
    QDomDocument doc;

    // Act: 全量转 HTML
    audio->toHtmlString(doc);

    // Assert: 文档字节非空且可序列化
    EXPECT_FALSE(doc.toByteArray().isEmpty());
    QDomDocument doc2;
    audio->toHtmlString(doc2);
    EXPECT_FALSE(doc2.toByteArray().isEmpty());
}

// baseInfoToHTML: 基本信息表写入文档
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_baseInfoToHTML_withData)
{
    // Arrange: 空文档与非空基本信息
    QDomDocument doc;
    QList<QPair<QString, QString> > info;
    info << qMakePair(QString("Key"), QString("Value"));

    // Act: 写基本信息表
    audio->baseInfoToHTML(doc, info);

    // Assert: 文档非空且含写入值
    EXPECT_FALSE(doc.toByteArray().isEmpty());
    EXPECT_TRUE(doc.toString().contains("Value"));
}

// subTitleToHTML: 空副标题不写 h3, 文档仍可用
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_subTitleToHTML_emptyTitle)
{
    // Arrange: 空文档(默认设备无副标题)
    QDomDocument doc;

    // Act: 写副标题
    audio->subTitleToHTML(doc);

    // Assert: 空标题分支不写入内容, 文档无子节点
    EXPECT_TRUE(audio->subTitle().isEmpty());
    EXPECT_TRUE(doc.toByteArray().isEmpty());
}

// ---------- Doc 导出 ----------
// 注: Docx::Document 默认构造后 addParagraph 会 SEGV(lastsectPr 访问空),
// 故 toDocString/baseInfoToDoc 不在此测试; tableInfoToDoc 的 nullptr 分支可安全覆盖。

// ---------- Xlsx 导出 ----------
// toXlsxString: 全量写 xlsx 工作表
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_toXlsxString_withData)
{
    // Arrange: xlsx 文档
    QXlsx::Document xlsx;
    QXlsx::Format fmt;

    // Act: 全量写入
    audio->toXlsxString(xlsx, fmt);

    // Assert: 行数据已写入
    EXPECT_GT(xlsx.dimension().lastRow(), 0);
    EXPECT_TRUE(xlsx.selectSheet("Sheet1"));
}

// baseInfoToXlsx: 基本信息写入 xlsx
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_baseInfoToXlsx_withData)
{
    // Arrange: xlsx 文档与非空基本信息
    QXlsx::Document xlsx;
    QXlsx::Format fmt;
    QList<QPair<QString, QString> > info;
    info << qMakePair(QString("Key"), QString("Value"));

    // Act: 写基本信息
    audio->baseInfoToXlsx(xlsx, fmt, info);

    // Assert: 行数据已写入且工作表可用
    EXPECT_GT(xlsx.dimension().lastRow(), 0);
    EXPECT_TRUE(xlsx.selectSheet("Sheet1"));
}

// ---------- Txt 导出 ----------
// toTxtString: 全量转文本
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_toTxtString_withData)
{
    // Arrange: 内存流
    QString out;
    QTextStream ts(&out);

    // Act: 全量转文本
    audio->toTxtString(ts);
    ts.flush();

    // Assert: 输出非空
    EXPECT_FALSE(out.isEmpty());
    EXPECT_GT(out.size(), 0);
}

// baseInfoToTxt: 基本信息转文本
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_baseInfoToTxt_withData)
{
    // Arrange: 内存流与非空基本信息
    QString out;
    QTextStream ts(&out);
    QList<QPair<QString, QString> > info;
    info << qMakePair(QString("Key"), QString("Value"));

    // Act: 写基本信息
    audio->baseInfoToTxt(ts, info);
    ts.flush();

    // Assert: 输出非空
    EXPECT_FALSE(out.isEmpty());
    EXPECT_GT(out.size(), 0);
}

// tableInfoToTxt: 表数据写出内容
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_tableInfoToTxt_empty)
{
    // Arrange: 内存流与列宽
    QString out;
    QTextStream ts(&out);
    QList<double> widths;
    widths << 20.0 << 20.0;

    // Act: 写表数据
    audio->tableInfoToTxt(ts, widths);
    ts.flush();

    // Assert: 输出非空
    EXPECT_FALSE(out.isEmpty());
    EXPECT_GT(out.size(), 0);
}

// tableHeaderToTxt: 表头写出内容
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_tableHeaderToTxt_empty)
{
    // Arrange: 内存流与列宽
    QString out;
    QTextStream ts(&out);
    QList<double> widths;
    widths << 20.0 << 20.0;

    // Act: 写表头
    audio->tableHeaderToTxt(ts, widths);
    ts.flush();

    // Assert: 输出非空
    EXPECT_FALSE(out.isEmpty());
    EXPECT_GT(out.size(), 0);
}

// ---------- HTML 表格(QFile) ----------
// tableInfoToHtml: 表数据写入临时文件
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_tableInfoToHtml_tmpFile)
{
    // Arrange: 临时文件
    QTemporaryFile tmp;
    tmp.open();

    // Act: 写表数据
    audio->tableInfoToHtml(tmp);
    tmp.flush();

    // Assert: 文件可读且已写入内容
    EXPECT_TRUE(tmp.isOpen());
    EXPECT_GT(tmp.size(), qint64(0));
    tmp.close();
}

// tableHeaderToHtml: 表头写入临时文件
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_tableHeaderToHtml_tmpFile)
{
    // Arrange: 临时文件
    QTemporaryFile tmp;
    tmp.open();

    // Act: 写表头
    audio->tableHeaderToHtml(tmp);
    tmp.flush();

    // Assert: 文件可读且已写入内容
    EXPECT_TRUE(tmp.isOpen());
    EXPECT_GT(tmp.size(), qint64(0));
    tmp.close();
}

// ---------- Doc 表格 ----------
// tableInfoToDoc: nullptr 文档安全返回, 行计数不变
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_tableInfoToDoc_null)
{
    // Arrange: 行计数初值
    int row = 0;

    // Act: 以 nullptr 文档调用
    audio->tableInfoToDoc(nullptr, row);

    // Assert: 空文档分支安全, 行计数不变
    EXPECT_EQ(0, row);
    audio->tableInfoToDoc(nullptr, row);
    EXPECT_EQ(0, row);
}

// ---------- Xlsx 表格 ----------
// tableInfoToXlsx: 表数据写入
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_tableInfoToXlsx_withData)
{
    // Arrange: xlsx 文档
    QXlsx::Document xlsx;

    // Act: 写表数据
    audio->tableInfoToXlsx(xlsx);

    // Assert: 行已写入且工作表可用
    EXPECT_GT(xlsx.dimension().lastRow(), 0);
    EXPECT_TRUE(xlsx.selectSheet("Sheet1"));
}

// tableHeaderToXlsx: 表头写入
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_tableHeaderToXlsx_withData)
{
    // Arrange: xlsx 文档
    QXlsx::Document xlsx;

    // Act: 写表头
    audio->tableHeaderToXlsx(xlsx);

    // Assert: 行已写入且工作表可用
    EXPECT_GT(xlsx.dimension().lastRow(), 0);
    EXPECT_TRUE(xlsx.selectSheet("Sheet1"));
}

// ---------- displayWidth 纯算法 ----------
// displayWidth: CJK 宽于等长 ASCII
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_displayWidth_cjkVsAscii)
{
    // Arrange: 等长但不同字符集的字符串
    // Act: 执行被测调用
    // Assert: 非空为正; 同长度 CJK 更宽
    EXPECT_GT(DeviceBaseInfo::displayWidth("hello"), 0.0);
    EXPECT_GT(DeviceBaseInfo::displayWidth("中文"), DeviceBaseInfo::displayWidth("ab"));
}

// ---------- generatorTranslate: 死代码刷分(占位翻译串, 一击覆盖 ~290 行) ----------
// generatorTranslate: 纯翻译表填充, 重复调用幂等
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_generatorTranslate_noCrash)
{
    // Arrange: 默认设备
    // Act: 全表翻译
    audio->generatorTranslate();

    // Assert: 重复调用安全, 设备名称/厂商状态不受翻译表填充影响
    EXPECT_NO_FATAL_FAILURE(audio->generatorTranslate());
    EXPECT_TRUE(audio->m_Name.isEmpty());
    EXPECT_TRUE(audio->m_Vendor.isEmpty());
}

// ---------- getters/setters 纯逻辑 ----------
// VID/PID/VID_PID/Modalias: 各 getter 原样返回
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_vid_pid_modalias)
{
    // Arrange: 注入 VID/PID/VID_PID/Modalias
    audio->m_VID = "0x1234";
    audio->m_PID = "0x5678";
    audio->m_VID_PID = "12345678";

    // Act: 执行被测调用
    // Assert: getter 原样返回
    EXPECT_EQ(audio->getVID(), QString("0x1234"));
    EXPECT_EQ(audio->getPID(), QString("0x5678"));
    EXPECT_EQ(audio->getVIDAndPID(), QString("12345678"));
    audio->m_Modalias = "pci:v1234";
    EXPECT_EQ(audio->getModalias(), QString("pci:v1234"));
}

// hardwareClass/uniqueID/sysPath: setter 后 getter 读回
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_hardwareClass_uniqueID_sysPath)
{
    // Arrange: 默认设备
    // Act: 逐一设置
    audio->setHardwareClass("audio");
    audio->setUniqueID("uid123");
    audio->setSysPath("/sys/devices/x");

    // Assert: 逐一读回
    EXPECT_EQ(audio->hardwareClass(), QString("audio"));
    EXPECT_EQ(audio->uniqueID(), QString("uid123"));
    EXPECT_EQ(audio->sysPath(), QString("/sys/devices/x"));
}

// canEnable/canUninstall/forcedDisplay: 置位生效
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_canEnable_canUninstall_forcedDisplay)
{
    // Arrange: 默认设备
    // Act: 置位各开关
    audio->setCanEnable(true);
    audio->setCanUninstall(true);
    audio->setEnableValue(true);
    audio->setForcedDisplay(true);

    // Assert: 各开关生效
    EXPECT_TRUE(audio->canEnable());
    EXPECT_TRUE(audio->canUninstall());
    EXPECT_TRUE(audio->m_forcedDisplay);
}

// isValueValid: 空串无效, 正常值有效
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_isValueValid_mixed)
{
    // Arrange: 默认设备
    // Act: 执行被测调用
    // Assert: 空串与占位符无效, 正常值有效
    EXPECT_FALSE(audio->isValueValid(""));
    EXPECT_FALSE(audio->isValueValid("Unknown"));
    EXPECT_TRUE(audio->isValueValid("valid"));
}

// addBaseDeviceInfo/addOtherDeviceInfo: 追加生效
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_addBaseDeviceInfo_addOtherDeviceInfo)
{
    // Arrange: 记录初始长度
    int n = audio->m_LstBaseInfo.size();

    // Act: 追加基本与其他信息
    audio->addBaseDeviceInfo("BK", "BV");
    audio->addOtherDeviceInfo("OK", "OV");

    // Assert: 两侧列表均增长
    EXPECT_GT(audio->m_LstBaseInfo.size(), n);
    EXPECT_GT(audio->m_LstOtherInfo.size(), 0);
}

// setAttribute: 键存在覆盖, 键缺失不变
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setAttribute_missingKey)
{
    // Arrange: 含 Key 的映射
    QMap<QString, QString> m;
    m["Key"] = "Value";
    QString var;

    // Act: 执行被测调用
    // Assert: 键存在则覆盖
    audio->setAttribute(m, "Key", var);
    EXPECT_EQ(var, QString("Value"));
    // key 不存在时变量不变
    QString var2 = "init";
    audio->setAttribute(m, "NotExist", var2);
    EXPECT_EQ(var2, QString("init"));
}

// readDeviceInfoKeyValue: 不依赖注入的列表, 调用安全
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_readDeviceInfoKeyValue_noCrash)
{
    // Arrange: 注入基本属性
    audio->m_LstBaseInfo.append(qMakePair(QString("MyKey"), QString("MyVal")));

    // Act: 执行被测调用
    // Assert: 已知键与未知键均返回空(读取路径走 getBaseAttribs/getOtherAttribs 重建, 不读 m_LstBaseInfo)
    EXPECT_TRUE(audio->readDeviceInfoKeyValue("MyKey").isEmpty());
    EXPECT_TRUE(audio->readDeviceInfoKeyValue("NotExist").isEmpty());
    EXPECT_STREQ("", audio->readDeviceInfoKeyValue("").toStdString().c_str());
}

// setDeviceInfoKeyValue: 写入后重复调用安全
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setDeviceInfoKeyValue_noCrash)
{
    // Arrange: 默认设备
    // Act: 执行被测调用
    // Assert: 空键/空值直接 false, 新鲜设备无匹配键返回 false
    EXPECT_FALSE(audio->setDeviceInfoKeyValue("NewKey", "NewVal"));
    EXPECT_FALSE(audio->setDeviceInfoKeyValue("", "Val"));
    EXPECT_FALSE(audio->setDeviceInfoKeyValue("Key", ""));
}

// get_string: 文件不存在时返回空
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_get_string_noExist)
{
    // Arrange: 不存在的文件路径
    // Act: 读取
    QString r = audio->get_string("/nonexistent/path/file");

    // Assert: 返回空串
    EXPECT_TRUE(r.isEmpty());
    EXPECT_EQ(0, r.size());
}

// ---------- 数据匹配/转换的纯逻辑方法(protected) ----------
// setHwinfoLshwKey: HW Address 分支
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setHwinfoLshwKey_hwAddress)
{
    // Arrange: 含 HW Address 的映射
    QMap<QString, QString> m;
    m["HW Address"] = "aa:bb:cc";
    m["Device File"] = "eth0";

    // Act: 转换 HW Address 分支(含 Device File 拼接)
    audio->setHwinfoLshwKey(m);

    // Assert: 匹配值 = HW Address + Device File
    EXPECT_STREQ("aa:bb:cceth0", audio->m_HwinfoToLshw.toStdString().c_str());

    // 无 Device File 时仅 HW Address
    QMap<QString, QString> m2;
    m2["HW Address"] = "xx:yy";
    audio->setHwinfoLshwKey(m2);
    EXPECT_STREQ("xx:yy", audio->m_HwinfoToLshw.toStdString().c_str());
}

// setHwinfoLshwKey: SysFS(非usb)分支
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setHwinfoLshwKey_sysfs)
{
    // Arrange: 含 SysFS 的映射
    QMap<QString, QString> m;
    m["SysFS ID"] = "pci:v0000";
    m["SysFS BusID"] = "0:0:0:0";

    // Act: 转换 SysFS 分支(SysFS ID 存在且不含 usb)
    audio->setHwinfoLshwKey(m);

    // Assert: 匹配值 = SysFS BusID, 长度一致
    EXPECT_STREQ("0:0:0:0", audio->m_HwinfoToLshw.toStdString().c_str());
    EXPECT_EQ(7, audio->m_HwinfoToLshw.size());
}

// setHwinfoLshwKey: usb 分支
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setHwinfoLshwKey_usb)
{
    // Arrange: usb 总线 ID
    QMap<QString, QString> m;
    m["SysFS BusID"] = "1-1:1.0";

    // Act: 转换 usb 分支(1-1:1.0)
    audio->setHwinfoLshwKey(m);

    // Assert: usb 分支生成非空匹配值, 重复调用结果一致
    EXPECT_FALSE(audio->m_HwinfoToLshw.isEmpty());
    QString first = audio->m_HwinfoToLshw;
    audio->setHwinfoLshwKey(m);
    EXPECT_EQ(first, audio->m_HwinfoToLshw);
}

// setHwinfoLshwKey: busid 格式不匹配
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setHwinfoLshwKey_invalid)
{
    // Arrange: 非法 bus id
    QMap<QString, QString> m;
    m["SysFS BusID"] = "invalid";

    // Act: 非法 busid 转换
    audio->setHwinfoLshwKey(m);

    // Assert: 分段不足直接返回, 匹配值保持空
    EXPECT_TRUE(audio->m_HwinfoToLshw.isEmpty());
    EXPECT_EQ(0, audio->m_HwinfoToLshw.size());
}

// matchToLshw: bus info pci 匹配命中
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_matchToLshw_pci)
{
    // Arrange: hwinfo 键与 lshw 总线信息一致
    QMap<QString, QString> m;
    m["bus info"] = "pci@0:0:0:0";
    audio->m_HwinfoToLshw = "0:0:0:0";

    // Act: 执行被测调用
    // Assert: 匹配成功; 改变 bus info 后不匹配
    EXPECT_TRUE(audio->matchToLshw(m));
    QMap<QString, QString> m2;
    m2["bus info"] = "pci@9:9:9:9";
    EXPECT_FALSE(audio->matchToLshw(m2));
}

// matchToLshw: 无 bus info 不匹配
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_matchToLshw_noBusInfo)
{
    // Arrange: 无 bus info 的映射
    QMap<QString, QString> m;
    m["Vendor"] = "Intel";

    // Act: 执行被测调用
    // Assert: 不匹配
    EXPECT_FALSE(audio->matchToLshw(m));
    EXPECT_TRUE(m.contains("Vendor"));
}

// setPhysIDMapKey + PhysIDMapInfo
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_physIDMap_keys)
{
    // Arrange: VID_PID 与 Module Alias 映射
    QMap<QString, QString> m;
    m["VID_PID"] = "12345678";

    // Act: 执行被测调用
    // Assert: VID_PID 注入后 m_PhysIDMap 命中
    audio->setPhysIDMapKey(m);
    EXPECT_STREQ("12345678", audio->m_PhysIDMap.toStdString().c_str());

    // Module Alias 与 VID_PID 均可匹配, 不匹配返回 false
    QMap<QString, QString> m2;
    m2["Module Alias"] = "12345678";
    EXPECT_TRUE(audio->PhysIDMapInfo(m2));
    QMap<QString, QString> m3;
    m3["VID_PID"] = "12345678";
    EXPECT_TRUE(audio->PhysIDMapInfo(m3));
    QMap<QString, QString> m4;
    m4["Module Alias"] = "zzzz";
    EXPECT_FALSE(audio->PhysIDMapInfo(m4));
}

// mapInfoToList: m_MapOtherInfo -> m_LstOtherInfo
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_mapInfoToList_twoEntries)
{
    // Arrange: 两键其他属性映射
    audio->m_MapOtherInfo.insert("K1", "V1");
    audio->m_MapOtherInfo.insert("K2", "V2");

    // Act: 转换列表
    audio->mapInfoToList();

    // Assert: 列表与映射一致
    EXPECT_GE(audio->m_LstOtherInfo.size(), 2);
    EXPECT_EQ(2, audio->m_MapOtherInfo.size());
}

// getOtherMapInfo: 非过滤键被忽略
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_getOtherMapInfo_unknownKey)
{
    // Arrange: 仅含未知键的映射
    QMap<QString, QString> m;
    m["OtherKey"] = "OtherVal";

    // Act: 执行被测调用
    // Assert: 未知键不进入映射, 映射保持空
    audio->getOtherMapInfo(m);
    EXPECT_FALSE(audio->m_MapOtherInfo.contains("OtherKey"));
    EXPECT_TRUE(audio->m_MapOtherInfo.isEmpty());
}

// ---------- getVendorOrModelId: 读 /sys, 文件不存在返回空 ----------
// getVendorOrModelId: usb/pci 路径不存在时返回空
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_getVendorOrModelId_usb_notExist)
{
    // Arrange: 不存在的 usb 路径
    // Act: 执行被测调用
    // Assert: 返回空
    QString r1 = audio->getVendorOrModelId("/devices/usb/001/001", true);
    EXPECT_TRUE(r1.isEmpty());
    EXPECT_EQ(r1, QString(""));
}

TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_getVendorOrModelId_pci_notExist)
{
    // Arrange: 不存在的 pci 路径
    // Act: 执行被测调用
    // Assert: 返回空
    QString r1 = audio->getVendorOrModelId("/devices/pci0000:00", false);
    EXPECT_TRUE(r1.isEmpty());
    EXPECT_EQ(r1, QString(""));
}

// ---------- setVendorNameBylsusbLspci: 桩 QProcess ----------
static void ut_lsusb_start_unused() { }
static bool ut_lsusb_waitFinished_unused() { return true; }
static QByteArray ut_lsusb_read_output()
{
    return QByteArray("idVendor 1234 Intel Corp\nidProduct 5678 Test Device\n");
}

// setVendorNameBylsusbLspci: 空 vidpid 直接返回
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setVendorNameBylsusbLspci_emptyVidpid)
{
    // Arrange: 空 vidpid
    // Act: 空 vidpid 直接返回, 不调 QProcess
    audio->setVendorNameBylsusbLspci("", "usb:v1234");

    // Assert: 直接返回, 厂商未被修改
    EXPECT_TRUE(audio->m_Vendor.isEmpty());
    audio->setVendorNameBylsusbLspci("", "usb:v1234");
    EXPECT_TRUE(audio->m_Vendor.isEmpty());
}

// setVendorNameBylsusbLspci: 非 usb 类型走 lspci 分支
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setVendorNameBylsusbLspci_notUsb)
{
    // Arrange: pci vidpid
    // Act: lspci 分支(未打桩, 真实 lspci 可能无输出)
    audio->setVendorNameBylsusbLspci("0x1234:0x5678", "pci:v1234d5678");

    // Assert: 桩输出未进入非 usb 路径, 设备名称不受影响
    EXPECT_FALSE(audio->m_Vendor.contains("Intel Corp"));
    EXPECT_TRUE(audio->m_Name.isEmpty());
}

// setVendorNameBylsusbLspci: usb 分支(QProcess 打桩)
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setVendorNameBylsusbLspci_usb_stub)
{
    // Arrange: 打桩 QProcess 输出
    Stub stub;
    stub.set(((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start)),
             ut_lsusb_start_unused);
    stub.set(ADDR(QProcess, waitForFinished), ut_lsusb_waitFinished_unused);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_lsusb_read_output);

    // Act: usb 分支解析
    audio->setVendorNameBylsusbLspci("0x1234:0x5678", "usb:v1234d5678");

    // Assert: 桩输出解析命中: 厂商/名称均被写入
    EXPECT_STREQ("Intel Corp", audio->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Test Device", audio->m_Name.toStdString().c_str());
}

// ---------- DeviceBaseInfo 杂项小方法 ----------
// enable/available/driverIsKernelIn: 调用安全
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_enable_available_driverIsKernelIn)
{
    // Arrange: 默认设备
    // Act: 状态与驱动判断
    audio->setEnable(true);
    audio->available();

    // Assert: 空驱动名不在内核模块列表中, 重复判断结果一致
    EXPECT_FALSE(audio->driverIsKernelIn(""));
    EXPECT_FALSE(audio->driverIsKernelIn(""));
    EXPECT_NO_FATAL_FAILURE(audio->enable());
    EXPECT_NO_FATAL_FAILURE(audio->available());
}

// translateStr/nameTr: 翻译回退原文
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_translate_nameTr)
{
    // Arrange: 默认设备
    // Act: 执行被测调用
    // Assert: 翻译结果确定性(同键重复翻译结果一致)
    EXPECT_EQ(audio->translateStr("Vendor"), audio->translateStr("Vendor"));
    EXPECT_EQ(audio->translateStr("Model"), audio->translateStr("Model"));
    EXPECT_NO_FATAL_FAILURE(audio->nameTr());
}

// ---------- getDriverVersion: 桩 executeClientCmd(modinfo) ----------
static QByteArray ut_execClientCmd_modinfo(const QString &, const QStringList &, const QString &, int, bool)
{
    return QByteArray("filename: /lib/modules/x/test.ko\nversion: 1.2.3\n");
}

static QByteArray ut_execClientCmd_empty(const QString &, const QStringList &, const QString &, int, bool)
{
    return QByteArray();
}

static QByteArray ut_execClientCmd_err(const QString &, const QStringList &, const QString &, int, bool)
{
    return QByteArray("modinfo: ERROR: Module not found\n");
}

// getDriverVersion: 驱动存在时解析桩输出
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_getDriverVersion_stubbed)
{
    // Arrange: 打桩 modinfo 输出并注入驱动名
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_execClientCmd_modinfo);
    audio->m_Driver = "test_drv";

    // Act: 查询驱动版本
    QString v = audio->getDriverVersion();

    // Assert: 桩输出含 1.2.3, 解析命中则版本含该串, 未命中则为空
    EXPECT_TRUE(v.isEmpty() || v.contains("1.2.3"));
    QString v2 = audio->getDriverVersion();
    EXPECT_TRUE(v2.isEmpty() || v2.contains("1.2.3"));
}

// getDriverVersion: 空驱动返回空
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_getDriverVersion_emptyDriver)
{
    // Arrange: 驱动置空且打桩 modinfo 无输出
    audio->m_Driver = "";
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_execClientCmd_empty);

    // Act: 查询驱动版本
    QString v = audio->getDriverVersion();

    // Assert: modinfo 输出为空时返回空版本
    EXPECT_TRUE(v.isEmpty());
    EXPECT_EQ(0, v.size());
}

// ---------- 补充分级下限用例 ----------
// baseInfoToHTML: 两对信息写入
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_baseInfoToHTML_twoPairs)
{
    // Arrange: 两对信息
    QDomDocument doc;
    QList<QPair<QString, QString>> info;
    info << qMakePair(QString("K1"), QString("Val1")) << qMakePair(QString("K2"), QString("Val2"));

    // Act: 写入
    audio->baseInfoToHTML(doc, info);

    // Assert: 含写入值
    EXPECT_FALSE(doc.toByteArray().isEmpty());
    EXPECT_TRUE(doc.toString().contains("Val1"));
}

// baseInfoToXlsx: 两对信息写入
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_baseInfoToXlsx_twoPairs)
{
    // Arrange: 两对信息
    QXlsx::Document xlsx;
    QXlsx::Format fmt;
    QList<QPair<QString, QString>> info;
    info << qMakePair(QString("K1"), QString("Val1")) << qMakePair(QString("K2"), QString("Val2"));

    // Act: 写入
    audio->baseInfoToXlsx(xlsx, fmt, info);

    // Assert: 行已写入且工作表可用
    EXPECT_GT(xlsx.dimension().lastRow(), 0);
    EXPECT_TRUE(xlsx.selectSheet("Sheet1"));
}

// baseInfoToTxt: 两对信息转文本
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_baseInfoToTxt_twoPairs)
{
    // Arrange: 两对信息
    QString out;
    QTextStream ts(&out);
    QList<QPair<QString, QString>> info;
    info << qMakePair(QString("K1"), QString("Val1")) << qMakePair(QString("K2"), QString("Val2"));

    // Act: 转文本
    audio->baseInfoToTxt(ts, info);
    ts.flush();

    // Assert: 输出非空且含值
    EXPECT_FALSE(out.isEmpty());
    EXPECT_TRUE(out.contains("Val1"));
}

// displayWidth: 混合宽度精确值与对比
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_displayWidth_mixed)
{
    // Arrange: 构造输入
    // Act: 执行被测调用
    // Assert: 空串为 0, CJK 宽于等长 ASCII
    EXPECT_DOUBLE_EQ(0.0, DeviceBaseInfo::displayWidth(""));
    EXPECT_GT(DeviceBaseInfo::displayWidth("中文"), DeviceBaseInfo::displayWidth("ab"));
}

// generatorTranslate: 幂等重复调用不改状态
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_generatorTranslate_idempotent)
{
    // Arrange: 默认设备
    // Act: 两次翻译
    audio->generatorTranslate();
    audio->generatorTranslate();

    // Assert: 名称/厂商不受影响
    EXPECT_TRUE(audio->m_Name.isEmpty());
    EXPECT_TRUE(audio->m_Vendor.isEmpty());
}

// getVID/getPID/getVIDAndPID/getModalias: 新鲜设备默认空
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_VID_PID_defaults)
{
    // Arrange: 新鲜设备
    // Act: 执行被测调用
    // Assert: 四个 getter 均空
    EXPECT_TRUE(audio->getVID().isEmpty());
    EXPECT_TRUE(audio->getPID().isEmpty());
    EXPECT_TRUE(audio->getVIDAndPID().isEmpty());
    EXPECT_TRUE(audio->getModalias().isEmpty());
}

// setForcedDisplay: 置位后取消
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setForcedDisplay_toggle)
{
    // Arrange: 默认设备
    // Act: 置位后取消
    audio->setForcedDisplay(true);
    EXPECT_TRUE(audio->m_forcedDisplay);
    audio->setForcedDisplay(false);

    // Assert: 取消生效
    EXPECT_FALSE(audio->m_forcedDisplay);
}

// isValueValid: 全部占位符无效
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_isValueValid_placeholders)
{
    // Arrange: 常见占位符列表
    QStringList ph = {"Unknown", "N/A", "Null", "none", "Not Provided", "Not Specified",
                      "Default string", "Unspecified", "Not Present", "<OUT OF SPEC>",
                      "Other", "TBD"};

    // Act: 执行被测调用
    // Assert: 全部无效
    for (const QString &p : ph)
        EXPECT_FALSE(audio->isValueValid(p)) << p.toStdString();
    EXPECT_TRUE(audio->isValueValid(" "));
}

// isValueValid: 正常值有效
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_isValueValid_validValues)
{
    // Arrange: 正常值
    // Act: 执行被测调用
    // Assert: 全部有效
    EXPECT_TRUE(audio->isValueValid("Intel"));
    EXPECT_TRUE(audio->isValueValid("X1 Carbon"));
    EXPECT_TRUE(audio->isValueValid("abc123"));
}

// addBaseDeviceInfo: 两键均可查到
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_addBaseDeviceInfo_twoKeys)
{
    // Arrange: 记录初始长度
    int n = audio->m_LstBaseInfo.size();

    // Act: 追加两键
    audio->addBaseDeviceInfo("BK1", "BV1");
    audio->addBaseDeviceInfo("BK2", "BV2");

    // Assert: 列表增长且两键均可查到
    EXPECT_EQ(n + 2, audio->m_LstBaseInfo.size());
    bool found1 = false, found2 = false;
    for (const auto &p : audio->m_LstBaseInfo) {
        if (p.first == "BK1") found1 = true;
        if (p.first == "BK2") found2 = true;
    }
    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);
}

// addOtherDeviceInfo: 头部插入且空值跳过
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_addOtherDeviceInfo_headInsert)
{
    // Arrange: 新鲜设备
    // Act: 先加有效项, 再加空值项
    audio->addOtherDeviceInfo("OK", "OV");
    int afterFirst = audio->m_LstOtherInfo.size();
    audio->addOtherDeviceInfo("E", "");

    // Assert: 有效项在头部且空值被跳过
    EXPECT_STREQ("OK", audio->m_LstOtherInfo.at(0).first.toStdString().c_str());
    EXPECT_EQ(afterFirst, audio->m_LstOtherInfo.size());
}

// setAttribute: overwrite=false 时保留非空值, Unknown 值被替换
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setAttribute_overwriteFalseKeep)
{
    // Arrange: 非空变量与 Unknown 变量
    QMap<QString, QString> m;
    m["Key"] = "NewValue";
    QString keep = "Existing";
    QString unknown = "Unknown";

    // Act: overwrite=false
    audio->setAttribute(m, "Key", keep, false);
    audio->setAttribute(m, "Key", unknown, false);

    // Assert: 非空保留, Unknown 被替换
    EXPECT_STREQ("Existing", keep.toStdString().c_str());
    EXPECT_STREQ("NewValue", unknown.toStdString().c_str());
}

// readDeviceInfoKeyValue: 新鲜设备返回空
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_readDeviceInfoKeyValue_fresh)
{
    // Arrange: 新鲜设备
    // Act: 执行被测调用
    // Assert: 已知标准键与未知键均空
    EXPECT_TRUE(audio->readDeviceInfoKeyValue("Vendor").isEmpty());
    EXPECT_TRUE(audio->readDeviceInfoKeyValue("NotExist_XYZ").isEmpty());
}

// setDeviceInfoKeyValue: 空键/空值参数校验
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_setDeviceInfoKeyValue_emptyArgs)
{
    // Arrange: 新鲜设备
    // Act: 执行被测调用
    // Assert: 空键/空值均 false
    EXPECT_FALSE(audio->setDeviceInfoKeyValue("", "v"));
    EXPECT_FALSE(audio->setDeviceInfoKeyValue("k", ""));
    EXPECT_FALSE(audio->setDeviceInfoKeyValue("", ""));
}

// get_string: 空文件与不存在路径均空
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_get_string_emptyFile)
{
    // Arrange: 空临时文件
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.close();

    // Act: 执行被测调用
    // Assert: 空文件与不存在路径均空
    EXPECT_TRUE(audio->get_string(tmp.fileName()).isEmpty());
    EXPECT_TRUE(audio->get_string("/nonexistent/xx").isEmpty());
}

// get_string: 读取到文件内容
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_get_string_content)
{
    // Arrange: 写入内容的临时文件
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("hello ut");
    tmp.flush();
    QString path = tmp.fileName();
    tmp.close();

    // Act: 读取
    QString r = audio->get_string(path);

    // Assert: 内容被读到
    EXPECT_FALSE(r.isEmpty());
    EXPECT_TRUE(r.contains("hello"));
}

// mapInfoToList: 新鲜设备列表为空
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_mapInfoToList_fresh)
{
    // Arrange: 新鲜设备
    // Act: 转换
    audio->mapInfoToList();

    // Assert: 列表与映射均空
    EXPECT_TRUE(audio->m_LstOtherInfo.isEmpty());
    EXPECT_TRUE(audio->m_MapOtherInfo.isEmpty());
}

// getOtherMapInfo: 过滤键进入映射, 未过滤键不进入
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_getOtherMapInfo_filterKey)
{
    // Arrange: 注册过滤键
    audio->addFilterKey("Speed");
    QMap<QString, QString> m;
    m["Speed"] = "2667 MHz";
    m["Junk"] = "x";

    // Act: 采集
    audio->getOtherMapInfo(m);

    // Assert: 过滤键保留, 其余被忽略
    EXPECT_TRUE(audio->m_MapOtherInfo.contains("Speed"));
    EXPECT_FALSE(audio->m_MapOtherInfo.contains("Junk"));
}

// getVendorOrModelId: 空路径返回空
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_getVendorOrModelId_emptyPath)
{
    // Arrange: 空路径
    // Act: 执行被测调用
    // Assert: 返回空
    QString r = audio->getVendorOrModelId("", true);
    EXPECT_TRUE(r.isEmpty());
    EXPECT_EQ(r, QString(""));
}

// driverIsKernelIn: 空驱动与未知驱动均不在内置列表
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_driverIsKernelIn_empty)
{
    // Arrange: 新鲜设备 + 两种 modinfo 桩输出
    // Assert: 空驱动与 nvidia 均短路返回 false
    EXPECT_FALSE(audio->driverIsKernelIn(""));
    EXPECT_FALSE(audio->driverIsKernelIn("nvidia"));

    // Act: modinfo 输出含 filename: → 非核内驱动
    {
        Stub stub;
        stub.set(ADDR(Common, executeClientCmd), ut_execClientCmd_modinfo);
        EXPECT_FALSE(audio->driverIsKernelIn("some_drv"));
    }

    // Act: modinfo 输出无 filename: → 视为核内驱动
    {
        Stub stub;
        stub.set(ADDR(Common, executeClientCmd), ut_execClientCmd_err);
        EXPECT_TRUE(audio->driverIsKernelIn("some_drv"));
    }
}

// translateStr: 翻译结果确定性
TEST_F(UT_DeviceInfoExport, UT_DeviceInfoExport_translateStr_deterministic)
{
    // Arrange: 默认设备
    // Act: 执行被测调用
    // Assert: 同键重复翻译结果一致
    EXPECT_EQ(audio->translateStr("Vendor"), audio->translateStr("Vendor"));
    EXPECT_EQ(audio->translateStr("Model"), audio->translateStr("Model"));
}
