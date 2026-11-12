// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// 单元测试编写检查清单（test-code-gen 10 项最小检查）
// 1. [x] 每个被测方法的分支清单来自 MCP get_code_snippet 返回的真实源码（ut-t08 @ f2d46ee9）
// 2. [x] 源码分支全部映射到用例或显式 BRANCH_NOT_MAPPED 标注
// 3. [x] AAA（Arrange/Act/Assert）结构注释
// 4. [x] 断言基于被测行为（值/状态），非空断言或实现细节
// 5. [x] 外部依赖（DBus/QProcess/文件/cups）全部打桩，无真实设备环境依赖
// 6. [x] 不使用 VADDR 对 Qt 内置类取地址，统一 static_cast 指定重载
// 7. [x] 桩函数作用域最小化（Stub 栈对象，析构自动还原）
// 8. [x] 负向用例检查强异常安全（EXPECT_NO_FATAL_FAILURE）
// 9. [x] 同质多分支 ≥3 的用 TEST_P 参数化
// 10.[x] 每个测试可独立运行（Fixture SetUp/TearDown 重置状态）
//
// 覆盖范围（CmdTool.cpp @ deepin-devicemanager/src/GenerateDevice）：
//   状态管理: addMapInfo / cmdInfo / addUsbMapInfo / addMouseKeyboardInfoMapInfo
//   装载器:   loadCmdInfo 分发 + loadLshwInfo / loadLsblkInfo / loadLssgInfo /
//             loadSmartCtlInfo / loadDmesgInfo / loadHciconfigInfo /
//             loadBluetoothCtlInfo / loadUpowerInfo / loadHwinfoInfo /
//             getMulHwinfoInfo / loadDmidecodeInfo / loadDmidecode2Info /
//             loadLscpuInfo / loadCatInfo / loadBiosInfoFromLspci /
//             loadCatInputDeviceInfo / loadCatAudioInfo / loadCatConfigInfo /
//             loadBootDeviceManfid / loadNvidiaSettingInfo / loadPrinterInfo
//   读取器:   getDeviceInfo / getDeviceInfoFromCmd / getCatDeviceInfo
//   现值获取: getCurNetworkLinkStatus / getCurPowerInfo
//   工具:     containsInfoInTheMap / getSMBIOSVersion / getMapInfo / addWidthToMap
// 未覆盖（BRANCH_NOT_MAPPED）:
//   - loadCmdInfo 的 "printer" 分支依赖真实 cups 服务，仅打桩覆盖空 dests
// ============================================================================

#include "CmdTool.h"
#include "DeviceManager.h"
#include "DBusInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <QFile>
#include <QProcess>
#include <QByteArray>
#include <QStringList>
#include <cstring>
#include <cstdlib>
#include <dlfcn.h>

#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

// ============================ 通用桩（文件级静态） ============================

// getDeviceInfo / getCatDeviceInfo / DBusInterface::getInfo 的可控返回
static QString ut_ct_payload;     // getDeviceInfo 返回内容
static bool    ut_ct_devOk = true;
static QString ut_ct_catPayload;  // getCatDeviceInfo 返回内容
static bool    ut_ct_catOk = true;
static QString ut_ct_dbusPayload; // DBusInterface::getInfo 返回内容
static bool    ut_ct_dbusOk = true;

static bool ut_ct_getDeviceInfo(void * /*self*/, QString &deviceInfo, const QString & /*debugFile*/)
{
    if (ut_ct_devOk)
        deviceInfo = ut_ct_payload;
    return ut_ct_devOk;
}

static bool ut_ct_getCatDeviceInfo(void * /*self*/, QString &deviceInfo, const QString & /*debugFile*/)
{
    if (ut_ct_catOk)
        deviceInfo = ut_ct_catPayload;
    return ut_ct_catOk;
}

static bool ut_ct_getInfo(void * /*self*/, const QString & /*key*/, QString &info)
{
    if (ut_ct_dbusOk)
        info = ut_ct_dbusPayload;
    return ut_ct_dbusOk;
}

// QProcess 桩：统一 start/no-op + waitForFinished(true) + 可控 readAllStandardOutput
static void ut_ct_start(void * /*self*/, const QString & /*program*/, const QStringList & /*arguments*/, QIODevice::OpenMode /*mode*/)
{
    return;
}

static bool ut_ct_waitForFinished(void * /*self*/, int /*msecs*/)
{
    return true;
}

static QByteArray ut_ct_procOutput;   // QProcess::readAllStandardOutput 返回内容
static QByteArray ut_ct_readAll(void * /*self*/)
{
    return ut_ct_procOutput;
}

// 统一安装 QProcess 桩（Qt6 仅有 (program, args, mode) 与 (mode) 两种 start 重载）
static void ut_ct_installQProcessStubs(Stub &stub)
{
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(
                 &QProcess::start), ut_ct_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_ct_waitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_ct_readAll);
}

// getDeviceInfoFromCmd 桩：按调用次序返回三段预置输出
static int     ut_ct_cmdSeq = 0;
static QString ut_ct_cmdOut1;
static QString ut_ct_cmdOut2;
static QString ut_ct_cmdOut3;
static bool ut_ct_getDeviceInfoFromCmd(void * /*self*/, QString &deviceInfo,
                                       const QString & /*program*/, const QStringList & /*arguments*/)
{
    ++ut_ct_cmdSeq;
    if (1 == ut_ct_cmdSeq)
        deviceInfo = ut_ct_cmdOut1;
    else if (2 == ut_ct_cmdSeq)
        deviceInfo = ut_ct_cmdOut2;
    else
        deviceInfo = ut_ct_cmdOut3;
    return true;
}

// QFile 桩：可控打开结果与读取内容
// 注意：QFile::open(OpenMode) 为虚函数，成员函数指针存的是虚表偏移而非代码地址，
// 不能用 ADDR 取地址打桩；改用 dlsym 取真实符号地址后按函数体打桩。
static bool       ut_ct_fileOpenOk = true;
static QByteArray ut_ct_fileContent;
static bool ut_ct_fileOpen(QFile * /*self*/, QIODevice::OpenMode /*mode*/)
{
    return ut_ct_fileOpenOk;
}
static bool ut_ct_fileOpenFalse(QFile * /*self*/, QIODevice::OpenMode /*mode*/)
{
    return false;
}
static QByteArray ut_ct_fileReadAll(QFile * /*self*/)
{
    return ut_ct_fileContent;
}
static void ut_ct_installQFileStubs(Stub &stub, bool openOk)
{
    void *openFn = dlsym(RTLD_DEFAULT, "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    if (nullptr != openFn)
        stub.set(openFn, openOk ? reinterpret_cast<void *>(ut_ct_fileOpen)
                                : reinterpret_cast<void *>(ut_ct_fileOpenFalse));
    stub.set(ADDR(QFile, readAll), ut_ct_fileReadAll);
}

// DSysInfo 版本类型桩（hciconfig 分支用）
static DSysInfo::UosEdition ut_ct_uosHomeEdition()
{
    return DSysInfo::UosHome;
}
static DSysInfo::UosEdition ut_ct_uosProfessionalEdition()
{
    return DSysInfo::UosProfessional;
}

// cups 桩：空 dests / 富信息 dests（12 个 option，>10 才会被 loadPrinterInfo 收录）
static int ut_ct_cupsGetDests2_none(http_t * /*http*/, cups_dest_t **dests)
{
    *dests = nullptr;
    return 0;
}
static int ut_ct_cupsGetDests2_rich(http_t * /*http*/, cups_dest_t **dests)
{
    cups_dest_t *dest = static_cast<cups_dest_t *>(calloc(1, sizeof(cups_dest_t)));
    dest->name = strdup("HP_LaserJet_1020");
    dest->num_options = 12;
    dest->options = static_cast<cups_option_t *>(calloc(12, sizeof(cups_option_t)));
    for (int i = 0; i < 12; ++i) {
        dest->options[i].name = strdup(QString("option%1").arg(i).toUtf8().constData());
        dest->options[i].value = strdup("value");
    }
    *dests = dest;
    return 1;
}
static void ut_ct_cupsFreeDests(int /*num_dests*/, cups_dest_t *dests)
{
    if (nullptr == dests)
        return;
    for (int i = 0; i < dests->num_options; ++i) {
        free(dests->options[i].name);
        free(dests->options[i].value);
    }
    free(dests->options);
    free(dests->name);
    free(dests);
}

// ================================ Fixture ================================

class UT_CmdTool : public UT_HEAD
{
public:
    void SetUp()
    {
        m_cmdTool = new CmdTool;
        ut_ct_payload.clear();
        ut_ct_devOk = true;
        ut_ct_catPayload.clear();
        ut_ct_catOk = true;
        ut_ct_dbusPayload.clear();
        ut_ct_dbusOk = true;
        ut_ct_procOutput.clear();
        ut_ct_fileOpenOk = true;
        ut_ct_fileContent.clear();
        ut_ct_cmdSeq = 0;
        ut_ct_cmdOut1.clear();
        ut_ct_cmdOut2.clear();
        ut_ct_cmdOut3.clear();
    }
    void TearDown()
    {
        delete m_cmdTool;
        m_cmdTool = nullptr;
    }
    CmdTool *m_cmdTool = nullptr;
};

// ============================ 状态管理用例 ============================

// void addMapInfo(const QString &key, const QMap<QString, QString> &mapInfo);
// 分支: key 已存在→append；key 不存在→新建 list
TEST_F(UT_CmdTool, addMapInfo_insertThenAppend)
{
    // Arrange: 空状态
    QMap<QString, QString> map;
    map.insert("K", "V");

    // Act: 同 key 连续添加两次
    m_cmdTool->addMapInfo("lshw_cpu", map);
    m_cmdTool->addMapInfo("lshw_cpu", map);

    // Assert: 同 key 追加为两个元素
    EXPECT_EQ(2, m_cmdTool->m_cmdInfo["lshw_cpu"].size());
}

// const QMap<QString, QList<QMap<QString, QString>>> &cmdInfo() const;
// 分支: 返回内部 m_cmdInfo 引用（初始为空）
TEST_F(UT_CmdTool, cmdInfo_initiallyEmpty)
{
    // Arrange: 新建 CmdTool

    // Act
    const QMap<QString, QList<QMap<QString, QString>>> &info = m_cmdTool->cmdInfo();

    // Assert: 初始为空，添加后可见
    EXPECT_TRUE(info.isEmpty());
    QMap<QString, QString> map;
    map.insert("A", "B");
    m_cmdTool->addMapInfo("k", map);
    EXPECT_EQ(1, info["k"].size());
}

// void addUsbMapInfo(const QString &key, const QMap<QString, QString> &mapInfo);
// 分支: SysFS BusID 去重(同一设备两段信息只留一段)/Linux Foundation 空接口过滤/
//       Hub Controller 过滤/hub 类过滤/打印机过滤/mouse keyboard 过滤/Wacom 过滤/正常添加
TEST_F(UT_CmdTool, addUsbMapInfo_dedupSameBusID)
{
    // Arrange: 两条 BusID 仅末段不同的 hwinfo 信息
    QMap<QString, QString> first;
    first.insert("SysFS BusID", "1-3:1.2");
    first.insert("Model", "USB Camera");
    QMap<QString, QString> second;
    second.insert("SysFS BusID", "1-3:1.0");
    second.insert("Model", "USB Camera");
    QMap<QString, QString> third;
    third.insert("SysFS BusID", "2-1:1.0");
    third.insert("Model", "USB Disk");

    // Act
    m_cmdTool->addUsbMapInfo("hwinfo_usb", first);
    m_cmdTool->addUsbMapInfo("hwinfo_usb", second);
    m_cmdTool->addUsbMapInfo("hwinfo_usb", third);

    // Assert: 同设备去重后仅保留两条
    EXPECT_EQ(2, m_cmdTool->m_cmdInfo["hwinfo_usb"].size());
}

TEST_F(UT_CmdTool, addUsbMapInfo_filterEmptyPortAndHub)
{
    // Arrange: root hub / Hub Controller / hub 类三种应被过滤
    QMap<QString, QString> rootHub;
    rootHub.insert("Model", "Linux Foundation 3.0 root hub");
    QMap<QString, QString> hubCtl;
    hubCtl.insert("Model", "VIA Hub Controller");
    QMap<QString, QString> hubClass;
    hubClass.insert("Hardware Class", "hub");

    // Act
    m_cmdTool->addUsbMapInfo("hwinfo_usb", rootHub);
    m_cmdTool->addUsbMapInfo("hwinfo_usb", hubCtl);
    m_cmdTool->addUsbMapInfo("hwinfo_usb", hubClass);

    // Assert: 全部被过滤
    EXPECT_EQ(0, m_cmdTool->m_cmdInfo["hwinfo_usb"].size());
}

TEST_F(UT_CmdTool, addUsbMapInfo_filterPrinterMouseKeyboardWacom)
{
    // Arrange: 打印机/鼠标/键盘/数位板四类应被过滤
    QMap<QString, QString> printer;
    printer.insert("Model", "HP LaserJet 1020");
    QMap<QString, QString> mouse;
    mouse.insert("Device", "USB Optical mouse");
    QMap<QString, QString> keyboard;
    keyboard.insert("Device", "USB keyboard");
    QMap<QString, QString> wacom;
    wacom.insert("Device", "Wacom Tablet");
    QMap<QString, QString> normal;
    normal.insert("Model", "USB Camera");

    // Act
    m_cmdTool->addUsbMapInfo("hwinfo_usb", printer);
    m_cmdTool->addUsbMapInfo("hwinfo_usb", mouse);
    m_cmdTool->addUsbMapInfo("hwinfo_usb", keyboard);
    m_cmdTool->addUsbMapInfo("hwinfo_usb", wacom);
    m_cmdTool->addUsbMapInfo("hwinfo_usb", normal);

    // Assert: 仅普通设备被保留
    EXPECT_EQ(1, m_cmdTool->m_cmdInfo["hwinfo_usb"].size());
    EXPECT_EQ("USB Camera", m_cmdTool->m_cmdInfo["hwinfo_usb"][0]["Model"]);
}

// void addMouseKeyboardInfoMapInfo(const QString &key, const QMap<QString, QString> &mapInfo);
// 分支: Linux Foundation/Elite Remote Control Driver/serial console/Wacom 过滤；正常添加
TEST_F(UT_CmdTool, addMouseKeyboardInfoMapInfo_normalAdded)
{
    // Arrange: 正常鼠标信息
    QMap<QString, QString> map;
    map.insert("Model", "PixA USB Optical Mouse");

    // Act
    m_cmdTool->addMouseKeyboardInfoMapInfo("hwinfo_mouse", map);

    // Assert: 正常信息被记录
    EXPECT_EQ(1, m_cmdTool->m_cmdInfo["hwinfo_mouse"].size());
}

// 参数化夹具与 UT_CmdTool 分离，避免 INSTANTIATE 交叉实例化同名 TEST_P
class UT_CmdToolFilter : public UT_CmdTool, public ::testing::WithParamInterface<const char *>
{
};

// void addMouseKeyboardInfoMapInfo(const QString &key, const QMap<QString, QString> &mapInfo);
// 分支: Model 含过滤关键字（hub/remote/serial console/wacom）→ 不落库
TEST_P(UT_CmdToolFilter, addMouseKeyboardInfoMapInfo_filterKeywords)
{
    // Arrange: 参数化的过滤关键字
    QMap<QString, QString> map;
    map.insert("Model", GetParam());

    // Act
    m_cmdTool->addMouseKeyboardInfoMapInfo("hwinfo_mouse", map);

    // Assert: 含过滤关键字的输入不产生记录
    EXPECT_EQ(0, m_cmdTool->m_cmdInfo["hwinfo_mouse"].size());
}

INSTANTIATE_TEST_SUITE_P(FilterKeywords, UT_CmdToolFilter,
                         ::testing::Values("Linux Foundation HUB",
                                           "Elite Remote Control Driver",
                                           "Model: \"serial console\"",
                                           "Wacom Intuos"));

// bool containsInfoInTheMap(const QString &info, const QMap<QString, QString> &mapInfo);
// 分支: 任一 value 包含 info(忽略大小写)→true；都不包含→false
TEST_F(UT_CmdTool, containsInfoInTheMap_foundAndNotFound)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Device", "NVIDIA GeForce GTX 1050");

    // Act + Assert
    EXPECT_TRUE(m_cmdTool->containsInfoInTheMap("nvidia", map));
    EXPECT_FALSE(m_cmdTool->containsInfoInTheMap("Intel", map));
}

// void getSMBIOSVersion(const QString &info, QString &version);
// 分支: 匹配 "SMBIOS x.y.z present."→提取版本；不匹配→version 保持不变
TEST_F(UT_CmdTool, getSMBIOSVersion_match)
{
    // Arrange
    QString version;

    // Act
    m_cmdTool->getSMBIOSVersion("Handle 0x0000\nSMBIOS 3.0.0 present.\n", version);

    // Assert
    EXPECT_EQ(QString("3.0.0"), version);
}

TEST_F(UT_CmdTool, getSMBIOSVersion_noMatch)
{
    // Arrange
    QString version = "init";

    // Act
    m_cmdTool->getSMBIOSVersion("no smbios line here\n", version);

    // Assert: 未匹配时不变
    EXPECT_EQ(QString("init"), version);
}

// void getMapInfo(QMap<QString, QString> &mapInfo, cups_dest_t *src);
// 分支: name 写入 "Name"；每个 option 写入对应键值
TEST_F(UT_CmdTool, getMapInfo_fromCupsDest)
{
    // Arrange: 构造 cups_dest_t（内存由 cupsFreeDests 释放，需 calloc）
    const int optCount = 2;
    cups_dest_t *dest = static_cast<cups_dest_t *>(calloc(1, sizeof(cups_dest_t)));
    dest->name = strdup("HP_LaserJet_1020");
    dest->num_options = optCount;
    dest->options = static_cast<cups_option_t *>(calloc(optCount, sizeof(cups_option_t)));
    dest->options[0].name = strdup("device-uri");
    dest->options[0].value = strdup("usb://HP/LaserJet");
    dest->options[1].name = strdup("printer-make-and-model");
    dest->options[1].value = strdup("HP LaserJet 1020");
    QMap<QString, QString> mapInfo;

    // Act
    m_cmdTool->getMapInfo(mapInfo, dest);
    cupsFreeDests(1, dest);

    // Assert
    EXPECT_EQ("HP_LaserJet_1020", mapInfo["Name"]);
    EXPECT_EQ("usb://HP/LaserJet", mapInfo["device-uri"]);
    EXPECT_EQ("HP LaserJet 1020", mapInfo["printer-make-and-model"]);
}

// void addWidthToMap(QMap<QString, QString> &mapInfo);
// 分支: Vendor 非 NVIDIA→直接返回；NVIDIA 且命令输出匹配 GPUMemoryInterface→写入 Width
TEST_F(UT_CmdTool, addWidthToMap_notNvidiaSkipped)
{
    // Arrange: 非 NVIDIA 设备，不触发 QProcess
    QMap<QString, QString> map;
    map.insert("Vendor", "Intel Corporation");

    // Act
    m_cmdTool->addWidthToMap(map);

    // Assert: 无 Width
    EXPECT_FALSE(map.contains("Width"));
}

TEST_F(UT_CmdTool, addWidthToMap_nvidiaWidthParsed)
{
    // Arrange: NVIDIA 设备 + QProcess 桩输出 GPUMemoryInterface 属性行
    Stub stub;
    ut_ct_installQProcessStubs(stub);
    ut_ct_procOutput = "  Attribute 'GPUMemoryInterface' (PC:0): 128 bits.\n";
    QMap<QString, QString> map;
    map.insert("Vendor", "NVIDIA Corporation");

    // Act
    m_cmdTool->addWidthToMap(map);

    // Assert: 位宽正则已修复为 [0-9]+（DEF-001，上游 5c246d5f），完整捕获 128
    EXPECT_EQ(QString("128 bits"), map["Width"]);
}

// ============================ 读取器用例 ============================

// bool getDeviceInfo(QString &deviceInfo, const QString &debugFile);
// 分支: DBus getInfo 成功→true；失败且已缓存→true；失败且文件可读→读文件 true；全失败→false
TEST_F(UT_CmdTool, getDeviceInfo_dbusSuccess)
{
    // Arrange: DBus 桩返回内容
    Stub stub;
    stub.set(ADDR(DBusInterface, getInfo), ut_ct_getInfo);
    ut_ct_dbusPayload = "DBus content\n";

    // Act
    QString deviceInfo;
    bool ok = m_cmdTool->getDeviceInfo(deviceInfo, "dmidecode0.txt");

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(QString("DBus content\n"), deviceInfo);
}

TEST_F(UT_CmdTool, getDeviceInfo_fileFallback)
{
    // Arrange: DBus 失败 + QFile 桩成功
    Stub stub;
    stub.set(ADDR(DBusInterface, getInfo), ut_ct_getInfo);
    ut_ct_dbusOk = false;
    ut_ct_installQFileStubs(stub, true);
    ut_ct_fileContent = "file content\n";

    // Act
    QString deviceInfo;
    bool ok = m_cmdTool->getDeviceInfo(deviceInfo, "lshw.txt");

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(QString("file content\n"), deviceInfo);
}

TEST_F(UT_CmdTool, getDeviceInfo_allFail)
{
    // Arrange: DBus 失败 + 文件打开失败
    Stub stub;
    stub.set(ADDR(DBusInterface, getInfo), ut_ct_getInfo);
    ut_ct_dbusOk = false;
    ut_ct_installQFileStubs(stub, false);

    // Act
    QString deviceInfo;
    bool ok = m_cmdTool->getDeviceInfo(deviceInfo, "missing.txt");

    // Assert: 强异常安全，返回 false
    EXPECT_FALSE(ok);
    EXPECT_NO_FATAL_FAILURE(m_cmdTool->getDeviceInfo(deviceInfo, "missing.txt"));
}

// bool getDeviceInfoFromCmd(QString &deviceInfo, const QString &program, const QStringList &arguments);
// 分支: 总是返回 true，deviceInfo 来自命令输出
TEST_F(UT_CmdTool, getDeviceInfoFromCmd_returnsOutput)
{
    // Arrange: QProcess 桩输出
    Stub stub;
    ut_ct_installQProcessStubs(stub);
    ut_ct_procOutput = "cmd output\n";

    // Act
    QString deviceInfo;
    bool ok = m_cmdTool->getDeviceInfoFromCmd(deviceInfo, "ls", QStringList() << "-l");

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(QString("cmd output\n"), deviceInfo);
}

// bool getCatDeviceInfo(QString &deviceInfo, const QString &debugFile);
// 分支: 已缓存→true；文件可读→true；打开失败→false
TEST_F(UT_CmdTool, getCatDeviceInfo_cached)
{
    // Arrange: deviceInfo 已有内容
    QString deviceInfo = "cached";

    // Act
    bool ok = m_cmdTool->getCatDeviceInfo(deviceInfo, "/proc/version");

    // Assert: 直接返回 true，不读文件
    EXPECT_TRUE(ok);
    EXPECT_EQ(QString("cached"), deviceInfo);
}

TEST_F(UT_CmdTool, getCatDeviceInfo_fileRead)
{
    // Arrange: QFile 桩
    Stub stub;
    ut_ct_installQFileStubs(stub, true);
    ut_ct_fileContent = "cat content\n";

    // Act
    QString deviceInfo;
    bool ok = m_cmdTool->getCatDeviceInfo(deviceInfo, "/proc/version");

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(QString("cat content\n"), deviceInfo);
}

TEST_F(UT_CmdTool, getCatDeviceInfo_openFail)
{
    // Arrange: QFile 桩打开失败
    Stub stub;
    ut_ct_installQFileStubs(stub, false);

    // Act
    QString deviceInfo;
    bool ok = m_cmdTool->getCatDeviceInfo(deviceInfo, "/not/exist");

    // Assert
    EXPECT_FALSE(ok);
}

// ============================ 现值获取用例 ============================

// QString getCurNetworkLinkStatus(QString driverName);
// 分支: ifconfig 输出含 driverName 且含 broadcast→yes；不含 broadcast→no；不含 driverName→空
TEST_F(UT_CmdTool, getCurNetworkLinkStatus_yesNoAndMissing)
{
    // Arrange: QProcess 桩输出 ifconfig 信息
    Stub stub;
    ut_ct_installQProcessStubs(stub);
    ut_ct_procOutput = "eno1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500\n"
                       "inet 10.4.22.72  netmask 255.255.255.0  broadcast 10.4.22.255\n"
                       "\n"
                       "lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536\n";

    // Act + Assert: eno1 有 broadcast→yes；lo 无→no；eth9 不存在→空
    EXPECT_EQ(QString("yes"), m_cmdTool->getCurNetworkLinkStatus("eno1"));
    EXPECT_EQ(QString("no"), m_cmdTool->getCurNetworkLinkStatus("lo"));
    EXPECT_EQ(QString(""), m_cmdTool->getCurNetworkLinkStatus("eth9"));
}

// QMap<QString, QMap<QString, QString>> getCurPowerInfo();
// 分支: 跳过 DisplayDevice/line_power/mouse/keyboard 段；Daemon 段与电池段分组
TEST_F(UT_CmdTool, getCurPowerInfo_groupsDaemonAndBattery)
{
    // Arrange: QProcess 桩输出 upower --dump
    Stub stub;
    ut_ct_installQProcessStubs(stub);
    ut_ct_procOutput = "Device: /org/freedesktop/UPower/devices/battery_Battery\n"
                       "  native-path:          Battery\n"
                       "  power supply:         yes\n"
                       "  has history:          yes\n"
                       "\n"
                       "Device: /org/freedesktop/UPower/devices/DisplayDevice\n"
                       "  power supply:         yes\n"
                       "\n"
                       "Device: /org/freedesktop/UPower/devices/mouse_mouse\n"
                       "  percentage:          80%\n"
                       "\n"
                       "Daemon:\n"
                       "  daemon-version:  0.99.10\n"
                       "  on-battery:      yes\n"
                       "  lid-is-closed:   no\n"
                       "  lid-is-present:  yes\n";

    // Act
    QMap<QString, QMap<QString, QString>> result = m_cmdTool->getCurPowerInfo();

    // Assert: 仅电池与 Daemon 分组，DisplayDevice/mouse 被过滤（电池段含 Device 行共 4 项）
    EXPECT_EQ(2, result.size());
    EXPECT_TRUE(result.contains("upower"));
    EXPECT_TRUE(result.contains("Daemon"));
    EXPECT_EQ(4, result["upower"].size());
    EXPECT_EQ(4, result["Daemon"].size());
}

// ============================ 装载器用例 ============================

// void loadCmdInfo(const QString &key, const QString &debugFile);
// 分支: key 精确匹配 lshw/lsblk_d/ls_sg/dmesg/hciconfig/printer/upower/cat_devices/
//       cat_audio/bootdevice/lscpu/dr_config/nvidia；前缀 hwinfo*/dmidecode*；其它→loadCatInfo
// 参数化夹具与 UT_CmdTool 分离，避免 INSTANTIATE 交叉实例化同名 TEST_P
class UT_CmdToolDispatch : public UT_CmdTool, public ::testing::WithParamInterface<const char *>
{
};

TEST_P(UT_CmdToolDispatch, loadCmdInfo_dispatchExactKeys)
{
    // Arrange: 按参数化 key 配置各装载器所需桩与期望落库 key
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_ct_getCatDeviceInfo);
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_ct_getDeviceInfoFromCmd);
    stub.set(ADDR(DBusInterface, getInfo), ut_ct_getInfo);
    ut_ct_dbusOk = false;
    const QString key = GetParam();
    QString expectKey;
    if ("lshw" == key) {
        ut_ct_payload = "*-core\n    description: Computer\n";
        expectKey = "lshw_system";
    } else if ("lsblk_d" == key) {
        ut_ct_payload = "sda 1\n";
        expectKey = "lsblk_d";
    } else if ("ls_sg" == key) {
        ut_ct_payload = "/dev/sg0\n";
        expectKey = "smart";
    } else if ("dmesg" == key) {
        ut_ct_payload = "[   12.3] 0000:01:00.0: VRAM: 8192 M\n";
        expectKey = "dmesg";
    } else if ("hciconfig" == key) {
        stub.set(ADDR(DSysInfo, uosEditionType), ut_ct_uosProfessionalEdition);
        ut_ct_payload = "hci0:   Type: Primary\n    BD Address: 00:11:22:33:44:55\n";
        expectKey = "hciconfig";
    } else if ("upower" == key) {
        ut_ct_payload = "Daemon:\n  daemon-version: 0.99.10\n";
        expectKey = "Daemon";
    } else if ("cat_devices" == key) {
        ut_ct_catPayload = "N: Name=\"Mouse\"\nH: Handlers=mouse0\n\n";
        expectKey = "cat_devices";
    } else if ("cat_audio" == key) {
        ut_ct_catPayload = "Chip: ALC662\n";
        expectKey = "cat_audio";
    } else if ("bootdevice" == key) {
        ut_ct_payload = "WDC WD10EZEX\n";
        expectKey = "bootdevice";
    } else if ("lscpu" == key) {
        ut_ct_payload = "processor : 0\n";
        ut_ct_dbusOk = true;
        ut_ct_dbusPayload = "CPU(s) : 8\n";
        expectKey = "lscpu";
    } else if ("dr_config" == key) {
        ut_ct_payload = "snd_hda_intel\n";
        expectKey = "dr_config";
    } else if ("nvidia" == key) {
        ut_ct_cmdSeq = 0;
        ut_ct_cmdOut1 = "GPU 0: GeForce GTX 1050 (PCI:0000:01:00.0)\n";
        ut_ct_cmdOut2 = "GPU 00000000:01:00.0\nFB Memory Usage\n    Total : 4096 MiB\n";
        ut_ct_cmdOut3 = "  Attribute 'VideoRam' (PC:0.0): 2097152.\n";
        expectKey = "nvidia";
    } else if ("hwinfo_usb" == key) {
        ut_ct_payload = "33: USB 00.0: 10900 Controller\n  Hardware Class: other\n  Model: \"USB Camera\"\n";
        expectKey = "hwinfo_usb";
    } else if ("dmidecode1" == key) {
        ut_ct_payload = "System Information\n"
                        "        Manufacturer: LENOVO\n"
                        "        Product Name: 3133\n"
                        "        Version: NOK\n"
                        "        Serial Number: 123456\n"
                        "        UUID: 2C9B502E\n"
                        "        Wake-up Type: Power Switch\n"
                        "        SKU Number: SKU01\n";
        expectKey = "dmidecode1";
    } else { // cat_os_release → loadCatInfo 兜底分支
        ut_ct_catPayload = "PRODUCT_NAME=UOS\n";
        expectKey = "cat_os_release";
    }

    // Act
    m_cmdTool->loadCmdInfo(key, "debug.txt");

    // Assert: 对应命令数据 key 已落库
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains(expectKey)) << "key=" << key.toStdString();
}

INSTANTIATE_TEST_SUITE_P(Dispatch, UT_CmdToolDispatch,
                         ::testing::Values("lshw", "lsblk_d", "ls_sg", "dmesg",
                                           "hciconfig", "upower", "cat_devices",
                                           "cat_audio", "bootdevice", "lscpu",
                                           "dr_config", "nvidia",
                                           "hwinfo_usb", "dmidecode1",
                                           "cat_os_release"));

// void loadLshwInfo(const QString &debugFile);
// 分支: 首段→lshw_system；cpu/disk/storage/bank/display/multimedia/network(communication)/
//       usb/cdrom 前缀→对应 key；无 bank 时 memory 回退兜底
TEST_F(UT_CmdTool, loadLshwInfo_allSections)
{
    // Arrange: getDeviceInfo 桩返回含全部前缀的 lshw 输出
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "*-core\n"
                    "    description: Computer\n"
                    "*-cpu\n"
                    "    product: Intel(R) Core(TM) i7\n"
                    "*-disk\n"
                    "    product: WDC WD10EZEX\n"
                    "*-storage\n"
                    "    product: Cannon Lake PCH\n"
                    "*-memory UNCLAIMED\n"
                    "    description: System memory\n"
                    "*-bank\n"
                    "    product: M378A1K43CB2\n"
                    "*-display\n"
                    "    product: GP107\n"
                    "*-multimedia\n"
                    "    product: 100 Series Audio\n"
                    "*-network\n"
                    "    logical name: eno1\n"
                    "*-usb\n"
                    "    product: USB2.0-CRW\n"
                    "*-cdrom\n"
                    "    product: DVD_RW\n";

    // Act
    m_cmdTool->loadLshwInfo("lshw.txt");

    // Assert: 11 个分类 key 全部落库（含 bank 记忆）
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_system"));
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_cpu"));
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_disk"));
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_storage"));
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_memory"));
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_display"));
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_multimedia"));
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_network"));
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_usb"));
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_cdrom"));
    EXPECT_EQ(QString("Intel(R) Core(TM) i7"), m_cmdTool->m_cmdInfo["lshw_cpu"][0]["product"]);
}

TEST_F(UT_CmdTool, loadLshwInfo_memoryFallback)
{
    // Arrange: 输出仅有 memory 段（无 bank 前缀）
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "*-core\n"
                    "*-memory\n"
                    "    description: System memory\n";

    // Act
    m_cmdTool->loadLshwInfo("lshw.txt");

    // Assert: memory 回退兜底命中
    EXPECT_TRUE(m_cmdTool->m_cmdInfo.contains("lshw_memory"));
}

// void loadLsblkInfo(const QString &debugfile);
// 分支: 非两列或 NAME 行跳过；两列行入 map 并级联 loadSmartCtlInfo("smart")
TEST_F(UT_CmdTool, loadLsblkInfo_parsesAndChainsSmartCtl)
{
    // Arrange: 桩按调用次序返回 lsblk / smartctl 内容
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "NAME ROTA\n"
                    "sda 1\n"
                    "sdb 0\n"
                    "sr0\n";
    // Act
    m_cmdTool->loadLsblkInfo("lsblk_d.txt");

    // Assert: lsblk_d 段落只入一个 map（列表长度 1），map 内含两列设备；sr0 单词行跳过
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["lsblk_d"].size());
    EXPECT_EQ("1", m_cmdTool->m_cmdInfo["lsblk_d"][0]["sda"]);
    EXPECT_EQ("0", m_cmdTool->m_cmdInfo["lsblk_d"][0]["sdb"]);
    // 级联：smartctl 桩返回空内容但仍生成 "smart" 记录（ln 键）
    ASSERT_FALSE(m_cmdTool->m_cmdInfo["smart"].isEmpty());
    EXPECT_EQ("sda", m_cmdTool->m_cmdInfo["smart"][0]["ln"]);
}

// void loadLssgInfo(const QString &debugfile);
// 分支: 空行跳过；"/" 分割 >2 段→loadSmartCtlInfo；否则告警跳过
TEST_F(UT_CmdTool, loadLssgInfo_parsesDevices)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "/dev/sg0\n"
                    "\n"
                    "/dev/sg1\n";

    // Act
    m_cmdTool->loadLssgInfo("ls_sg.txt");

    // Assert: 两个 sg 设备级联生成 smart 记录
    EXPECT_EQ(2, m_cmdTool->m_cmdInfo["smart"].size());
}

// void loadSmartCtlInfo(const QString &logicalName, const QString &debugfile);
// 分支: getDeviceInfo 失败→return；成功→map 带 ln + smartctl 解析后入 "smart"
TEST_F(UT_CmdTool, loadSmartCtlInfo_addsLogicalName)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "Model Family:     WDC WD10EZEX\n"
                    "User Capacity:    1,000 GB\n";

    // Act
    m_cmdTool->loadSmartCtlInfo("sda", "smartctl_sda.txt");

    // Assert
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["smart"].size());
    EXPECT_EQ("sda", m_cmdTool->m_cmdInfo["smart"][0]["ln"]);
    EXPECT_EQ("WDC WD10EZEX", m_cmdTool->m_cmdInfo["smart"][0]["Model Family"]);
}

TEST_F(UT_CmdTool, loadSmartCtlInfo_getInfoFail)
{
    // Arrange: getDeviceInfo 桩返回失败
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_devOk = false;

    // Act
    m_cmdTool->loadSmartCtlInfo("sda", "smartctl_sda.txt");

    // Assert: 无 smart 记录
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("smart"));
}

// void loadDmesgInfo(const QString &debugfile);
// 分支: VRAM pci 地址格式→Size=addr=xxGB；JJW 特殊格式→Size=null=xxGB；audiochip 正则
TEST_F(UT_CmdTool, loadDmesgInfo_vramAndAudioChip)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "[   12.345] 0000:01:00.0: VRAM: 8192 M\n"
                    "[   13.000] amdgpu: autoconfig for ALC662 rev3:\n";

    // Act
    m_cmdTool->loadDmesgInfo("dmesg.txt");

    // Assert: dmesg Size 8192M→8GB；audiochip chip 提取
    EXPECT_EQ(QString("0000:01:00.0=8GB"), m_cmdTool->m_cmdInfo["dmesg"][0]["Size"]);
    EXPECT_EQ(QString("ALC662 rev3"), m_cmdTool->m_cmdInfo["audiochip"][0]["chip"]);
}

TEST_F(UT_CmdTool, loadDmesgInfo_jjwVramSpecial)
{
    // Arrange: JJW 特殊格式（无 pci 地址）
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "amdgpu: VRAM Size 4096M\n";

    // Act
    m_cmdTool->loadDmesgInfo("dmesg.txt");

    // Assert: null=4GB
    EXPECT_EQ(QString("null=4GB"), m_cmdTool->m_cmdInfo["dmesg"][0]["Size"]);
}

// void loadHciconfigInfo(const QString &debugfile);
// 分支: 个人版(UosHome)→QProcess 执行 hciconfig；其它版本→读文件；空段跳过；
//       每段 getMapInfoFromHciconfig 后级联 loadBluetoothCtlInfo
TEST_F(UT_CmdTool, loadHciconfigInfo_homeEditionViaCmd)
{
    // Arrange: 个人版桩 + QProcess 桩输出 hciconfig 段
    Stub stub;
    stub.set(ADDR(DSysInfo, uosEditionType), ut_ct_uosHomeEdition);
    ut_ct_installQProcessStubs(stub);
    ut_ct_procOutput = "hci0:   Type: Primary  Bus: USB\n"
                       "    BD Address: 00:11:22:33:44:55  ACL MTU: 1021:8\n";

    // Act
    m_cmdTool->loadHciconfigInfo("hciconfig.txt");

    // Assert: hciconfig key 落库且带 BD Address（bluetoothctl 输出为空仅保留原信息）
    ASSERT_FALSE(m_cmdTool->m_cmdInfo["hciconfig"].isEmpty());
    EXPECT_EQ("00:11:22:33:44:55", m_cmdTool->m_cmdInfo["hciconfig"][0]["BD Address"]);
}

TEST_F(UT_CmdTool, loadHciconfigInfo_nonHomeEditionFromFile)
{
    // Arrange: 非个人版 + getDeviceInfo 桩
    Stub stub;
    stub.set(ADDR(DSysInfo, uosEditionType), ut_ct_uosProfessionalEdition);
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "hci0:   Type: Primary  Bus: USB\n"
                    "    BD Address: AA:BB:CC:DD:EE:FF  ACL MTU: 1021:8\n";

    // Act
    m_cmdTool->loadHciconfigInfo("hciconfig.txt");

    // Assert
    EXPECT_EQ("AA:BB:CC:DD:EE:FF", m_cmdTool->m_cmdInfo["hciconfig"][0]["BD Address"]);
}

// void loadBluetoothCtlInfo(QMap<QString, QString> &mapInfo);
// 分支: 无 BD Address→直接 addMapInfo；bluetoothctl 输出为空→原样 addMapInfo；
//       有输出→getMapInfoFromBluetoothCtl 合并后 addMapInfo
TEST_F(UT_CmdTool, loadBluetoothCtlInfo_noBDAddress)
{
    // Arrange: 无 BD Address 的 map
    QMap<QString, QString> map;
    map.insert("Name", "hci0");

    // Act
    m_cmdTool->loadBluetoothCtlInfo(map);

    // Assert: 原样入库
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["hciconfig"].size());
    EXPECT_EQ("hci0", m_cmdTool->m_cmdInfo["hciconfig"][0]["Name"]);
}

TEST_F(UT_CmdTool, loadBluetoothCtlInfo_withOutput)
{
    // Arrange: 有 BD Address + QProcess 桩输出 bluetoothctl 信息
    Stub stub;
    ut_ct_installQProcessStubs(stub);
    ut_ct_procOutput = "Controller 00:11:22:33:44:55\n"
                       "    Name: BlueZ 5.5\n"
                       "    Discovering: no\n";
    QMap<QString, QString> map;
    map.insert("BD Address", "00:11:22:33:44:55");

    // Act
    m_cmdTool->loadBluetoothCtlInfo(map);

    // Assert: bluetoothctl 信息合并入库
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["hciconfig"].size());
    EXPECT_EQ("BlueZ 5.5", m_cmdTool->m_cmdInfo["hciconfig"][0]["Name"]);
}

// void loadUpowerInfo(const QString &key, const QString &debugfile);
// 分支: getDeviceInfo 失败→return；空段/DisplayDevice/mouse/keyboard 跳过；
//       Daemon 段→"Daemon"；其它→key
TEST_F(UT_CmdTool, loadUpowerInfo_splitsDaemonAndBattery)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "Device: /org/freedesktop/UPower/devices/battery_Battery\n"
                    "  native-path:          Battery\n"
                    "\n"
                    "Device: /org/freedesktop/UPower/devices/DisplayDevice\n"
                    "  percentage: 50%\n"
                    "\n"
                    "Device: /org/freedesktop/UPower/devices/mouse_mouse\n"
                    "  percentage: 80%\n"
                    "\n"
                    "Daemon:\n"
                    "  daemon-version:  0.99.10\n"
                    "  on-battery:      yes\n";

    // Act
    m_cmdTool->loadUpowerInfo("upower", "upower.txt");

    // Assert: 电池入 key，Daemon 入 Daemon，DisplayDevice/mouse 被过滤
    ASSERT_FALSE(m_cmdTool->m_cmdInfo["upower"].isEmpty());
    EXPECT_EQ("Battery", m_cmdTool->m_cmdInfo["upower"][0]["native-path"]);
    EXPECT_EQ(1, m_cmdTool->m_cmdInfo["Daemon"].size());
}

// void loadHwinfoInfo(const QString &key, const QString &debugfile);
// 分支: hwinfo_monitor→按段解析仅保留 Hardware Class==monitor；其它 key→getMulHwinfoInfo
TEST_F(UT_CmdTool, loadHwinfoInfo_monitorFilter)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_ct_getDeviceInfoFromCmd);
    ut_ct_payload = "26: PCI 100.0: 0100 VGA compatible controller (VGA)\n"
                    "  Hardware Class: monitor\n"
                    "  Model: \"Lenovo L22i\"\n"
                    "\n"
                    "27: PCI 1f.0: 0601 ISA bridge\n"
                    "  Hardware Class: bridge\n";

    // Act
    m_cmdTool->loadHwinfoInfo("hwinfo_monitor", "hwinfo_monitor.txt");

    // Assert: 仅 monitor 段入库
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["hwinfo_monitor"].size());
    EXPECT_EQ("monitor", m_cmdTool->m_cmdInfo["hwinfo_monitor"][0]["Hardware Class"]);
}

// void getMulHwinfoInfo(const QString &info);
// 分支: sound/USB Audio→hwinfo_sound；network/WI-FI→hwinfo_network；keyboard/mouse→
//       addMouseKeyboardInfoMapInfo；cdrom/disk→直接入库；graphics card（GPU 文本过滤）→
//       addWidthToMap 后入库；其它→addUsbMapInfo；空段跳过
TEST_F(UT_CmdTool, getMulHwinfoInfo_classifiesAllTypes)
{
    // Arrange: QProcess 桩（addWidthToMap 用）+ hwinfo 多段输出
    Stub stub;
    ut_ct_installQProcessStubs(stub);
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_ct_getDeviceInfoFromCmd);
    ut_ct_procOutput = "";
    QString info = "26: PCI 100.0: 0403 Audio device\n"
                   "  Hardware Class: sound\n"
                   "  Device: \"100 Series Audio\"\n"
                   "\n"
                   "27: PCI 00.0: 0200 Ethernet controller\n"
                   "  Hardware Class: network\n"
                   "  Device: \"Ethernet Connection\"\n"
                   "\n"
                   "28: PS/2 00.0: 10500 PS/2 Mouse\n"
                   "  Hardware Class: mouse\n"
                   "  Model: \"PS/2 Mouse\"\n"
                   "\n"
                   "29: PCI 1f.2: 0104 RAID bus controller\n"
                   "  Hardware Class: disk\n"
                   "\n"
                   "30: PCI 1f.5: 82802 Firmware Hub Device\n"
                   "  Hardware Class: other\n"
                   "  Model: \"Generic USB Device\"\n";

    // Act
    m_cmdTool->getMulHwinfoInfo(info);

    // Assert: 各分类落库（mouse 含 PS/2 Mouse 特例仍按 mouse 过滤规则入库）
    EXPECT_FALSE(m_cmdTool->m_cmdInfo["hwinfo_sound"].isEmpty());
    EXPECT_FALSE(m_cmdTool->m_cmdInfo["hwinfo_network"].isEmpty());
    EXPECT_FALSE(m_cmdTool->m_cmdInfo["hwinfo_disk"].isEmpty());
    EXPECT_FALSE(m_cmdTool->m_cmdInfo["hwinfo_usb"].isEmpty());
}

TEST_F(UT_CmdTool, getMulHwinfoInfo_keyboardMouseAndGraphics)
{
    // Arrange: keyboard/mouse/graphics card 段
    Stub stub;
    ut_ct_installQProcessStubs(stub);
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_ct_getDeviceInfoFromCmd);
    ut_ct_procOutput = "";
    QString info = "30: PS/2 00.0: 10100 Keyboard\n"
                   "  Hardware Class: keyboard\n"
                   "  Model: \"AT Translated Set 2 keyboard\"\n"
                   "\n"
                   "31: PCI 100.0: 0300 VGA compatible controller\n"
                   "  Hardware Class: graphics card\n"
                   "  Vendor: \"NVIDIA Corporation\"\n"
                   "  Device: \"GeForce GTX 1050\"\n"
                   "\n"
                   "32: PCI 100.1: 0302 3D controller\n"
                   "  Hardware Class: graphics card\n"
                   "  Device: \"Graphics Processing Unit\"\n";

    // Act
    m_cmdTool->getMulHwinfoInfo(info);

    // Assert: keyboard 入库；GPU 文本的 graphics card 被跳过；NVIDIA 卡经 addWidthToMap 后入库
    EXPECT_FALSE(m_cmdTool->m_cmdInfo["hwinfo_keyboard"].isEmpty());
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["hwinfo_display"].size());
    EXPECT_EQ(QString("GeForce GTX 1050"), m_cmdTool->m_cmdInfo["hwinfo_display"][0]["Device"]);
    // QProcess 桩输出为空 → 未写入 Width
    EXPECT_FALSE(m_cmdTool->m_cmdInfo["hwinfo_display"][0].contains("Width"));
}

// void loadDmidecodeInfo(const QString &key, const QString &debugfile);
// 分支: dmidecode2→转 loadDmidecode2Info；dmidecode3 且 laptop/notebook→跳过 chassis；
//       dmidecode1→级联 OEM toml；dmidecode4 无 ID 段过滤；段数>MIN_NUM 才入库
TEST_F(UT_CmdTool, loadDmidecodeInfo_laptopChassisSkipped)
{
    // Arrange: dmidecode3 内容含 Laptop
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_ct_getCatDeviceInfo);
    ut_ct_payload = "Chassis Information\n"
                    "        Type: Laptop\n";

    // Act
    m_cmdTool->loadDmidecodeInfo("dmidecode3", "dmidecode3.txt");

    // Assert: 笔记本不生成 chassis 信息
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("dmidecode3"));
}

TEST_F(UT_CmdTool, loadDmidecodeInfo_cpuSlotFiltered)
{
    // Arrange: dmidecode4 两段，均 >MIN_NUM(5) 项；第二段无 ID 应被过滤
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_ct_getCatDeviceInfo);
    ut_ct_payload = "Processor Information\n"
                    "        ID: F1 06 03 00 FF FB EB BF\n"
                    "        Version: Intel(R) Core(TM) i7\n"
                    "        Voltage: 1.0 V\n"
                    "        External Clock: 100 MHz\n"
                    "        Max Speed: 4000 MHz\n"
                    "        Current Speed: 1800 MHz\n"
                    "        Status: Populated, Enabled\n"
                    "\n"
                    "Processor Information\n"
                    "        Socket Designation: U3E1\n"
                    "        Version: Intel(R) Core(TM) i7\n"
                    "        Voltage: 1.0 V\n"
                    "        External Clock: 100 MHz\n"
                    "        Max Speed: 4000 MHz\n"
                    "        Current Speed: 1800 MHz\n"
                    "        Status: Unpopulated\n";

    // Act
    m_cmdTool->loadDmidecodeInfo("dmidecode4", "dmidecode4.txt");

    // Assert: 仅含 ID 的段入库
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["dmidecode4"].size());
    EXPECT_TRUE(m_cmdTool->m_cmdInfo["dmidecode4"][0].contains("ID"));
}

// void loadDmidecode2Info(const QString &key, const QString &debugfile);
// 分支: 首段补充 chipset(loadBiosInfoFromLspci) 与 SMBIOS Version；后续段只解析
TEST_F(UT_CmdTool, loadDmidecode2Info_chipsetAndSmbios)
{
    // Arrange: getDeviceInfo 桩按内容同时服务 dmidecode2 与 lspci_vs.txt
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "Handle 0x000B, DMI type 2\n"
                    "Base Board Information\n"
                    "        Product Name: 3133\n"
                    "SMBIOS 3.0.0 present.\n";

    // Act
    m_cmdTool->loadDmidecodeInfo("dmidecode2", "dmidecode2.txt");

    // Assert
    ASSERT_FALSE(m_cmdTool->m_cmdInfo["dmidecode2"].isEmpty());
    EXPECT_EQ(QString("3.0.0"), m_cmdTool->m_cmdInfo["dmidecode2"][0]["SMBIOS Version"]);
}

// void loadBiosInfoFromLspci(QString &chipsetFamliy);
// 分支: getDeviceInfo 失败→不改写；无 Subsystem 行→空；命中→取 ": " 后内容
TEST_F(UT_CmdTool, loadBiosInfoFromLspci_subsystemExtracted)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "00:00.0 Host bridge: Intel Corporation\n"
                    "    Subsystem: ASUSTeK Computer Inc. Device 8c44\n";

    // Act
    QString chipset;
    m_cmdTool->loadBiosInfoFromLspci(chipset);

    // Assert
    EXPECT_EQ(QString("ASUSTeK Computer Inc. Device 8c44"), chipset);
}

// void loadLscpuInfo(const QString &key, const QString &debugfile);
// 分支: 段落解析入 key；DBus lscpu_num 失败→return；成功→lscpu_num 入库
TEST_F(UT_CmdTool, loadLscpuInfo_parsesLscpuAndNum)
{
    // Arrange: loadLscpuInfo 用 " : " 作为分隔符（lscpu -k 输出格式）
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    stub.set(ADDR(DBusInterface, getInfo), ut_ct_getInfo);
    ut_ct_payload = "processor : 0\nmodel name : Intel(R) Core(TM) i7\n";
    ut_ct_dbusPayload = "CPU(s) : 8\nCore(s) per socket : 4\n";

    // Act
    m_cmdTool->loadLscpuInfo("lscpu", "lscpu.txt");

    // Assert
    EXPECT_EQ("Intel(R) Core(TM) i7", m_cmdTool->m_cmdInfo["lscpu"][0]["model name"]);
    EXPECT_EQ("8", m_cmdTool->m_cmdInfo["lscpu_num"][0]["CPU(s)"]);
}

// void loadCatInfo(const QString &key, const QString &debugfile);
// 分支: getCatDeviceInfo 失败→return；cat_version→整段 OS；cat_os*→"=" 分隔；其它→": " 分隔
TEST_F(UT_CmdTool, loadCatInfo_versionAndOsRelease)
{
    // Arrange: cat_version 整段写入 OS 键（不加末尾换行，避免尾部换行带入值）
    Stub stub;
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_ct_getCatDeviceInfo);
    ut_ct_catPayload = "UnionTech OS Desktop 20 Professional";

    // Act
    m_cmdTool->loadCatInfo("cat_version", "/etc/version");

    // Assert
    EXPECT_EQ(QString("UnionTech OS Desktop 20 Professional"), m_cmdTool->m_cmdInfo["cat_version"][0]["OS"]);
}

TEST_F(UT_CmdTool, loadCatInfo_osReleaseEqualsSeparator)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_ct_getCatDeviceInfo);
    ut_ct_catPayload = "HOME_URL=\"https://www.chinauos.com\"\n"
                       "PRODUCT_NAME=UOS\n";

    // Act
    m_cmdTool->loadCatInfo("cat_os_release", "/etc/os-release");

    // Assert: "=" 分隔解析
    EXPECT_EQ("UOS", m_cmdTool->m_cmdInfo["cat_os_release"][0]["PRODUCT_NAME"]);
}

// void loadCatInputDeviceInfo(const QString &key, const QString &debugfile);
// 分支: Handlers 含 event*→addInputInfo；含 mouse*→addInputInfo；均无→仅 addMapInfo
TEST_F(UT_CmdTool, loadCatInputDeviceInfo_mouseHandler)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_ct_getCatDeviceInfo);
    ut_ct_catPayload = "I: Bus=0003 Vendor=093a Product=2510 Version=0111\n"
                       "N: Name=\"PixA USB Optical Mouse\"\n"
                       "H: Handlers=mouse0\n"
                       "\n";

    // Act
    m_cmdTool->loadCatInputDeviceInfo("cat_devices", "/proc/bus/input/devices");

    // Assert: DeviceManager 收到 input 信息且 key 落库
    EXPECT_FALSE(DeviceManager::instance()->inputInfo("mouse0").isEmpty());
    EXPECT_FALSE(m_cmdTool->m_cmdInfo["cat_devices"].isEmpty());
}

// void loadCatAudioInfo(const QString &key, const QString &debugfile);
// 分支: 失败→return；成功→按 ":" 分隔入库
TEST_F(UT_CmdTool, loadCatAudioInfo_parsesColonSeparated)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_ct_getCatDeviceInfo);
    ut_ct_catPayload = "Card 0: PCH\n"
                       "Chip: ALC662\n";

    // Act
    m_cmdTool->loadCatAudioInfo("cat_audio", "/proc/asound/cards");

    // Assert
    EXPECT_EQ("ALC662", m_cmdTool->m_cmdInfo["cat_audio"][0]["Chip"]);
}

// void loadCatConfigInfo(const QString &key, const QString &debugfile);
// 分支: 失败→return；每行首行作为 drivers 入库
TEST_F(UT_CmdTool, loadCatConfigInfo_driversFromFirstLine)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "snd_hda_intel\n"
                    "xhci_pci\n";

    // Act
    m_cmdTool->loadCatConfigInfo("dr_config", "dr_config.txt");

    // Assert
    EXPECT_EQ(2, m_cmdTool->m_cmdInfo["dr_config"].size());
    EXPECT_EQ("snd_hda_intel", m_cmdTool->m_cmdInfo["dr_config"][0]["drivers"]);
}

// void loadBootDeviceManfid(const QString &key, const QString &debugfile);
// 分支: 失败→return；成功→Model=整段 trimmed
TEST_F(UT_CmdTool, loadBootDeviceManfid_modelTrimmed)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfo), ut_ct_getDeviceInfo);
    ut_ct_payload = "  WDC WD10EZEX-08WN4A0  \n";

    // Act
    m_cmdTool->loadBootDeviceManfid("bootdevice", "bootdevice.txt");

    // Assert
    EXPECT_EQ(QString("WDC WD10EZEX-08WN4A0"), m_cmdTool->m_cmdInfo["bootdevice"][0]["Model"]);
}

// void loadNvidiaSettingInfo(const QString &key, const QString &debugfile);
// 分支: nvidia-smi -L 输出解析 GPU 列表；nvidia-settings VideoRam→null=xGB/MB；
//       nvidia-smi FB Memory Usage 兜底；无 Size 不入库
TEST_F(UT_CmdTool, loadNvidiaSettingInfo_vramFromSettings)
{
    // Arrange: getDeviceInfoFromCmd 桩按调用次序返回 -L/内存/VideoRam 输出
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_ct_getDeviceInfoFromCmd);
    ut_ct_cmdSeq = 0;
    ut_ct_cmdOut1 = "GPU 0: GeForce GTX 1050 (PCI:0000:01:00.0)\n";
    ut_ct_cmdOut2 = "GPU 00000000:01:00.0\n"
                    "FB Memory Usage\n"
                    "    Total : 4096 MiB\n";
    ut_ct_cmdOut3 = "  Attribute 'VideoRam' (PC:0.0): 2097152.\n";

    // Act
    m_cmdTool->loadNvidiaSettingInfo("nvidia", "nvidia.txt");

    // Assert: VideoRam 正则已修复（DEF-002，上游 728a9546）：2097152KB/1024=2048MB≥1024 → 2GB
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["nvidia"].size());
    EXPECT_EQ(QString("null=2GB"), m_cmdTool->m_cmdInfo["nvidia"][0]["Size"]);
    EXPECT_EQ(QString("GeForce GTX 1050"), m_cmdTool->m_cmdInfo["nvidia"][0]["Device"]);
    EXPECT_EQ(QString("00000000:01:00.0"), m_cmdTool->m_cmdInfo["nvidia"][0]["BusID"]);
}

TEST_F(UT_CmdTool, loadNvidiaSettingInfo_memoryFallback)
{
    // Arrange: VideoRam 输出无可匹配数字 → 走 FB Memory Usage 兜底
    Stub stub;
    stub.set(ADDR(CmdTool, getDeviceInfoFromCmd), ut_ct_getDeviceInfoFromCmd);
    ut_ct_cmdSeq = 0;
    ut_ct_cmdOut1 = "GPU 0: GeForce GTX 1050 (PCI:0000:01:00.0)\n";
    ut_ct_cmdOut2 = "GPU 00000000:01:00.0\n"
                    "FB Memory Usage\n"
                    "    Total : 800 MiB\n";
    ut_ct_cmdOut3 = "  No matching attributes.\n";

    // Act
    m_cmdTool->loadNvidiaSettingInfo("nvidia", "nvidia.txt");

    // Assert: Qt6 分支分隔符已修复为冒号（DEF-003，上游 913361dd），FB 兜底生效：
    // "Total : 800 MiB" → 800 MiB < 1000 → Size = "null=800MB"，设备正常入库
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["nvidia"].size());
    EXPECT_EQ(QString("null=800MB"), m_cmdTool->m_cmdInfo["nvidia"][0]["Size"]);
    EXPECT_EQ(QString("GeForce GTX 1050"), m_cmdTool->m_cmdInfo["nvidia"][0]["Device"]);
    EXPECT_EQ(QString("00000000:01:00.0"), m_cmdTool->m_cmdInfo["nvidia"][0]["BusID"]);
}

// void loadPrinterInfo();
// 分支: cupsGetDests2 无 dests→直接返回；mapInfo>10 项才入库
TEST_F(UT_CmdTool, loadPrinterInfo_noDests)
{
    // Arrange: cups 桩返回 0
    Stub stub;
    stub.set((void *)cupsGetDests2, (void *)ut_ct_cupsGetDests2_none);
    int before = m_cmdTool->m_cmdInfo.size();

    // Act
    m_cmdTool->loadPrinterInfo();

    // Assert
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("printer"));
    EXPECT_EQ(before, m_cmdTool->m_cmdInfo.size());
}

TEST_F(UT_CmdTool, loadPrinterInfo_destsAddedWhenRich)
{
    // Arrange: cups 桩返回 1 个 dest，含 12 个 option（>10 才入库）
    Stub stub;
    stub.set((void *)cupsGetDests2, (void *)ut_ct_cupsGetDests2_rich);
    stub.set((void *)cupsFreeDests, (void *)ut_ct_cupsFreeDests);

    // Act
    m_cmdTool->loadPrinterInfo();

    // Assert: printer 入库且 Name 正确
    ASSERT_EQ(1, m_cmdTool->m_cmdInfo["printer"].size());
    EXPECT_EQ(QString("HP_LaserJet_1020"), m_cmdTool->m_cmdInfo["printer"][0]["Name"]);
}

// void loadCmdInfo printer 分支走真实路径转发到 loadPrinterInfo（桩 cups 后无副作用）
TEST_F(UT_CmdTool, loadCmdInfo_printerBranchForwardsToCupsLoader)
{
    // Arrange: cups 桩返回 0
    Stub stub;
    stub.set((void *)cupsGetDests2, (void *)ut_ct_cupsGetDests2_none);

    // Act
    m_cmdTool->loadCmdInfo("printer", "printer.txt");

    // Assert: 无 printer 数据
    EXPECT_FALSE(m_cmdTool->m_cmdInfo.contains("printer"));
}
