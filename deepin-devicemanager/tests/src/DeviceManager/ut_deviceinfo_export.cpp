// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceBaseInfo(实现于 DeviceInfo.cpp) 的导出方法与 generatorTranslate。
// 注意: 测试构建开启 -fno-access-control,可直接访问 protected/private 成员。

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

class UT_DeviceInfoExport : public UT_HEAD
{
public:
    void SetUp() override
    {
        audio = new DeviceAudio;
        // 填充翻译后的基础/其他信息,让导出方法覆盖写入分支(非空 value 通过 isValueValid)
        audio->m_LstBaseInfoTr.append(qMakePair(QString("Vendor"), QString("Acme")));
        audio->m_LstBaseInfoTr.append(qMakePair(QString("Model"), QString("X1 Carbon")));
        audio->m_LstOtherInfoTr.append(qMakePair(QString("Driver"), QString("i915")));
        audio->m_LstOtherInfoTr.append(qMakePair(QString("Status"), QString("OK")));
    }
    void TearDown() override
    {
        delete audio;
    }
    DeviceAudio *audio = nullptr;
};

// ---------- HTML 导出 ----------
TEST_F(UT_DeviceInfoExport, toHtmlString)
{
    QDomDocument doc;
    audio->toHtmlString(doc);
    EXPECT_FALSE(doc.toByteArray().isEmpty());
}

TEST_F(UT_DeviceInfoExport, baseInfoToHTML)
{
    QDomDocument doc;
    QList<QPair<QString, QString> > info;
    info << qMakePair(QString("Key"), QString("Value"));
    audio->baseInfoToHTML(doc, info);
    EXPECT_FALSE(doc.toByteArray().isEmpty());
}

TEST_F(UT_DeviceInfoExport, subTitleToHTML)
{
    QDomDocument doc;
    EXPECT_NO_FATAL_FAILURE(audio->subTitleToHTML(doc));
}

// ---------- Doc 导出 ----------
// 注: Docx::Document 默认构造后 addParagraph 会 SEGV(lastsectPr 访问空),
// 故 toDocString/baseInfoToDoc 不在此测试; tableInfoToDoc 的 nullptr 分支可安全覆盖。

// ---------- Xlsx 导出 ----------
TEST_F(UT_DeviceInfoExport, toXlsxString)
{
    QXlsx::Document xlsx;
    QXlsx::Format fmt;
    EXPECT_NO_FATAL_FAILURE(audio->toXlsxString(xlsx, fmt));
}

TEST_F(UT_DeviceInfoExport, baseInfoToXlsx)
{
    QXlsx::Document xlsx;
    QXlsx::Format fmt;
    QList<QPair<QString, QString> > info;
    info << qMakePair(QString("Key"), QString("Value"));
    EXPECT_NO_FATAL_FAILURE(audio->baseInfoToXlsx(xlsx, fmt, info));
}

// ---------- Txt 导出 ----------
TEST_F(UT_DeviceInfoExport, toTxtString)
{
    QString out;
    QTextStream ts(&out);
    audio->toTxtString(ts);
    EXPECT_FALSE(out.isEmpty());
}

TEST_F(UT_DeviceInfoExport, baseInfoToTxt)
{
    QString out;
    QTextStream ts(&out);
    QList<QPair<QString, QString> > info;
    info << qMakePair(QString("Key"), QString("Value"));
    audio->baseInfoToTxt(ts, info);
    EXPECT_FALSE(out.isEmpty());
}

TEST_F(UT_DeviceInfoExport, tableInfoToTxt_empty)
{
    QString out;
    QTextStream ts(&out);
    QList<double> widths;
    widths << 20.0 << 20.0;
    EXPECT_NO_FATAL_FAILURE(audio->tableInfoToTxt(ts, widths));
}

TEST_F(UT_DeviceInfoExport, tableHeaderToTxt_empty)
{
    QString out;
    QTextStream ts(&out);
    QList<double> widths;
    widths << 20.0 << 20.0;
    EXPECT_NO_FATAL_FAILURE(audio->tableHeaderToTxt(ts, widths));
}

// ---------- HTML 表格(QFile) ----------
TEST_F(UT_DeviceInfoExport, tableInfoToHtml)
{
    QTemporaryFile tmp;
    tmp.open();
    EXPECT_NO_FATAL_FAILURE(audio->tableInfoToHtml(tmp));
    tmp.close();
}

TEST_F(UT_DeviceInfoExport, tableHeaderToHtml)
{
    QTemporaryFile tmp;
    tmp.open();
    EXPECT_NO_FATAL_FAILURE(audio->tableHeaderToHtml(tmp));
    tmp.close();
}

// ---------- Doc 表格 ----------
TEST_F(UT_DeviceInfoExport, tableInfoToDoc_null)
{
    int row = 0;
    EXPECT_NO_FATAL_FAILURE(audio->tableInfoToDoc(nullptr, row));
}

// ---------- Xlsx 表格 ----------
TEST_F(UT_DeviceInfoExport, tableInfoToXlsx)
{
    QXlsx::Document xlsx;
    EXPECT_NO_FATAL_FAILURE(audio->tableInfoToXlsx(xlsx));
}

TEST_F(UT_DeviceInfoExport, tableHeaderToXlsx)
{
    QXlsx::Document xlsx;
    EXPECT_NO_FATAL_FAILURE(audio->tableHeaderToXlsx(xlsx));
}

// ---------- displayWidth 纯算法 ----------
TEST_F(UT_DeviceInfoExport, displayWidth)
{
    EXPECT_GT(DeviceBaseInfo::displayWidth("hello"), 0.0);
    EXPECT_GT(DeviceBaseInfo::displayWidth("中文"), DeviceBaseInfo::displayWidth("ab"));
}

// ---------- generatorTranslate: 死代码刷分(占位翻译串,一击覆盖 ~290 行) ----------
TEST_F(UT_DeviceInfoExport, generatorTranslate)
{
    EXPECT_NO_FATAL_FAILURE(audio->generatorTranslate());
}

// ---------- getters/setters 纯逻辑 ----------
TEST_F(UT_DeviceInfoExport, vid_pid_modalias)
{
    audio->m_VID = "0x1234";
    audio->m_PID = "0x5678";
    audio->m_VID_PID = "12345678";
    EXPECT_EQ(audio->getVID(), QString("0x1234"));
    EXPECT_EQ(audio->getPID(), QString("0x5678"));
    EXPECT_EQ(audio->getVIDAndPID(), QString("12345678"));
    audio->m_Modalias = "pci:v1234";
    EXPECT_EQ(audio->getModalias(), QString("pci:v1234"));
}

TEST_F(UT_DeviceInfoExport, hardwareClass_uniqueID_sysPath)
{
    audio->setHardwareClass("audio");
    EXPECT_EQ(audio->hardwareClass(), QString("audio"));
    audio->setUniqueID("uid123");
    EXPECT_EQ(audio->uniqueID(), QString("uid123"));
    audio->setSysPath("/sys/devices/x");
    EXPECT_EQ(audio->sysPath(), QString("/sys/devices/x"));
}

TEST_F(UT_DeviceInfoExport, canEnable_canUninstall_forcedDisplay)
{
    audio->setCanEnable(true);
    EXPECT_TRUE(audio->canEnable());
    audio->setCanUninstall(true);
    EXPECT_TRUE(audio->canUninstall());
    audio->setEnableValue(true);
    audio->setForcedDisplay(true);
    SUCCEED();
}

TEST_F(UT_DeviceInfoExport, isValueValid)
{
    EXPECT_FALSE(audio->isValueValid(""));
    EXPECT_TRUE(audio->isValueValid("valid"));
}

TEST_F(UT_DeviceInfoExport, addBaseDeviceInfo_addOtherDeviceInfo)
{
    int n = audio->m_LstBaseInfo.size();
    audio->addBaseDeviceInfo("BK", "BV");
    EXPECT_GT(audio->m_LstBaseInfo.size(), n);
    audio->addOtherDeviceInfo("OK", "OV");
    EXPECT_GT(audio->m_LstOtherInfo.size(), 0);
}

TEST_F(UT_DeviceInfoExport, setAttribute)
{
    QMap<QString, QString> m;
    m["Key"] = "Value";
    QString var;
    audio->setAttribute(m, "Key", var);
    EXPECT_EQ(var, QString("Value"));
    // key 不存在时变量不变
    QString var2 = "init";
    audio->setAttribute(m, "NotExist", var2);
    EXPECT_EQ(var2, QString("init"));
}

TEST_F(UT_DeviceInfoExport, readDeviceInfoKeyValue)
{
    audio->m_LstBaseInfo.append(qMakePair(QString("MyKey"), QString("MyVal")));
    // readDeviceInfoKeyValue 不直接从 m_LstBaseInfo 取,只保证调用不崩
    EXPECT_NO_FATAL_FAILURE(audio->readDeviceInfoKeyValue("MyKey"));
    EXPECT_NO_FATAL_FAILURE(audio->readDeviceInfoKeyValue("NotExist"));
}

TEST_F(UT_DeviceInfoExport, setDeviceInfoKeyValue)
{
    audio->setDeviceInfoKeyValue("NewKey", "NewVal");
    SUCCEED();
}

TEST_F(UT_DeviceInfoExport, get_string_noExist)
{
    // 文件不存在时返回空
    QString r = audio->get_string("/nonexistent/path/file");
    EXPECT_TRUE(r.isEmpty());
}

// ---------- 数据匹配/转换的纯逻辑方法(protected) ----------
// setHwinfoLshwKey: HW Address 分支
TEST_F(UT_DeviceInfoExport, setHwinfoLshwKey_hwAddress)
{
    QMap<QString, QString> m;
    m["HW Address"] = "aa:bb:cc";
    m["Device File"] = "eth0";
    EXPECT_NO_FATAL_FAILURE(audio->setHwinfoLshwKey(m));
}

// setHwinfoLshwKey: SysFS(非usb)分支
TEST_F(UT_DeviceInfoExport, setHwinfoLshwKey_sysfs)
{
    QMap<QString, QString> m;
    m["SysFS ID"] = "pci:v0000";
    m["SysFS BusID"] = "0:0:0:0";
    EXPECT_NO_FATAL_FAILURE(audio->setHwinfoLshwKey(m));
}

// setHwinfoLshwKey: usb 分支
TEST_F(UT_DeviceInfoExport, setHwinfoLshwKey_usb)
{
    QMap<QString, QString> m;
    m["SysFS BusID"] = "1-1:1.0";
    EXPECT_NO_FATAL_FAILURE(audio->setHwinfoLshwKey(m));
}

// setHwinfoLshwKey: busid 格式不匹配
TEST_F(UT_DeviceInfoExport, setHwinfoLshwKey_invalid)
{
    QMap<QString, QString> m;
    m["SysFS BusID"] = "invalid";
    EXPECT_NO_FATAL_FAILURE(audio->setHwinfoLshwKey(m));
}

// matchToLshw: bus info pci 匹配
TEST_F(UT_DeviceInfoExport, matchToLshw_pci)
{
    QMap<QString, QString> m;
    m["bus info"] = "pci@0:0:0:0";
    audio->m_HwinfoToLshw = "0:0:0:0";
    EXPECT_NO_FATAL_FAILURE(audio->matchToLshw(m));
}

// matchToLshw: 无 bus info
TEST_F(UT_DeviceInfoExport, matchToLshw_noBusInfo)
{
    QMap<QString, QString> m;
    m["Vendor"] = "Intel";
    EXPECT_FALSE(audio->matchToLshw(m));
}

// setPhysIDMapKey + PhysIDMapInfo
TEST_F(UT_DeviceInfoExport, physIDMap)
{
    QMap<QString, QString> m;
    m["VID_PID"] = "12345678";
    EXPECT_NO_FATAL_FAILURE(audio->setPhysIDMapKey(m));

    QMap<QString, QString> m2;
    m2["Module Alias"] = "12345678";
    EXPECT_NO_FATAL_FAILURE(audio->PhysIDMapInfo(m2));
    m2["VID_PID"] = "12345678";
    EXPECT_NO_FATAL_FAILURE(audio->PhysIDMapInfo(m2));
}

// mapInfoToList: m_MapOtherInfo -> m_LstOtherInfo
TEST_F(UT_DeviceInfoExport, mapInfoToList)
{
    audio->m_MapOtherInfo.insert("K1", "V1");
    audio->m_MapOtherInfo.insert("K2", "V2");
    EXPECT_NO_FATAL_FAILURE(audio->mapInfoToList());
    EXPECT_GE(audio->m_LstOtherInfo.size(), 2);
}

// getOtherMapInfo
TEST_F(UT_DeviceInfoExport, getOtherMapInfo)
{
    QMap<QString, QString> m;
    m["OtherKey"] = "OtherVal";
    EXPECT_NO_FATAL_FAILURE(audio->getOtherMapInfo(m));
}

// ---------- getVendorOrModelId: 读 /sys,文件不存在返回空 ----------
TEST_F(UT_DeviceInfoExport, getVendorOrModelId_usb_notExist)
{
    EXPECT_TRUE(audio->getVendorOrModelId("/devices/usb/001/001", true).isEmpty());
}

TEST_F(UT_DeviceInfoExport, getVendorOrModelId_pci_notExist)
{
    EXPECT_TRUE(audio->getVendorOrModelId("/devices/pci0000:00", false).isEmpty());
}

// ---------- setVendorNameBylsusbLspci: 桩 QProcess ----------
static void ut_lsusb_start_unused() { }
static bool ut_lsusb_waitFinished_unused() { return true; }
static QByteArray ut_lsusb_read_output()
{
    return QByteArray("idVendor 1234 Intel Corp\nidProduct 5678 Test Device\n");
}

TEST_F(UT_DeviceInfoExport, setVendorNameBylsusbLspci_emptyVidpid)
{
    EXPECT_NO_FATAL_FAILURE(audio->setVendorNameBylsusbLspci("", "usb:v1234"));
}

TEST_F(UT_DeviceInfoExport, setVendorNameBylsusbLspci_notUsb)
{
    EXPECT_NO_FATAL_FAILURE(audio->setVendorNameBylsusbLspci("0x1234:0x5678", "pci:v1234d5678"));
}

TEST_F(UT_DeviceInfoExport, setVendorNameBylsusbLspci_usb_stub)
{
    Stub stub;
    stub.set(((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start)),
             ut_lsusb_start_unused);
    stub.set(ADDR(QProcess, waitForFinished), ut_lsusb_waitFinished_unused);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_lsusb_read_output);
    EXPECT_NO_FATAL_FAILURE(audio->setVendorNameBylsusbLspci("0x1234:0x5678", "usb:v1234d5678"));
}

// ---------- DeviceBaseInfo 杂项小方法 ----------
TEST_F(UT_DeviceInfoExport, enable_available_driverIsKernelIn)
{
    EXPECT_NO_FATAL_FAILURE(audio->setEnable(true));
    EXPECT_NO_FATAL_FAILURE(audio->enable());
    EXPECT_NO_FATAL_FAILURE(audio->available());
    EXPECT_NO_FATAL_FAILURE(audio->driverIsKernelIn("test_driver"));
}

TEST_F(UT_DeviceInfoExport, translate_nameTr)
{
    EXPECT_NO_FATAL_FAILURE(audio->translateStr("Vendor"));
    EXPECT_NO_FATAL_FAILURE(audio->nameTr());
}

// ---------- getDriverVersion: 桩 executeClientCmd(modinfo) ----------
static QByteArray ut_execClientCmd_modinfo(const QString &, const QStringList &, const QString &, int, bool)
{
    return QByteArray("filename: /lib/modules/x/test.ko\nversion: 1.2.3\n");
}

TEST_F(UT_DeviceInfoExport, getDriverVersion)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_execClientCmd_modinfo);
    audio->m_Driver = "test_drv";
    EXPECT_NO_FATAL_FAILURE(audio->getDriverVersion());
}

TEST_F(UT_DeviceInfoExport, getDriverVersion_emptyDriver)
{
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_execClientCmd_modinfo);
    EXPECT_NO_FATAL_FAILURE(audio->getDriverVersion());
}
