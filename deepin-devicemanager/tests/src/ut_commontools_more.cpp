// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// =============================================================================
// 单元测试: CommonTools 补充分支(重写版)
// 覆盖方法: parseEDID(空输入/短行/非法行/多输入/行字节数不足)、preGenerateGpuInfo
//           (静态缓存二次调用)、getGpuBaseInfo(键值解析)、feedback(接口无效)
// 桩策略: QProcess(启动/等待/读取输出) 注入构造 hexdump/
//         glxinfo 输出; QDBusInterface::isValid/call 控制分支; addMonitor 采集计数。
// 注: preGenerateGpuInfo 有函数级 static 缓存, 本文件与 ut_commontools.cpp 的
//     preGenerateGpuInfo 用例使用相同的 glxinfo/DBus 内容, 保证任意执行顺序下断言一致。
//
// 用例清单(用例数声明, 依据源码分支):
//  - parseEDID: 空输入列表; isHW 行 token 数不为 9 跳过; -C 非法行跳过;
//    多输入路径循环; -C 行非 32 字符跳过                                    (5)
//  - preGenerateGpuInfo: 二次调用命中缓存                                    (1)
//  - getGpuBaseInfo: 键值解析与无关行过滤                                    (1)
//  - feedback: 显示接口无效不发起调用                                        (1)
// =============================================================================

#include "commontools.h"
#include "ut_Head.h"
#include "stub.h"

#include <QString>
#include <QStringList>
#include <QMap>
#include <QProcess>
#include <QByteArray>
#include <QDBusInterface>
#include <QDBusAbstractInterface>
#include <QDBusMessage>
#include <QDBusVariant>
#include "DeviceManager.h"
#include "DeviceMonitor.h"

#include <gtest/gtest.h>

// ---------------- QProcess 桩 ----------------
static QByteArray ut_cm_dump_out;
static bool ut_cm_timeout = false;

static void ut_cm_start(QProcess *, const QString &, const QStringList &, QIODevice::OpenMode)
{
}

static bool ut_cm_wait(QProcess *, int)
{
    return !ut_cm_timeout;
}

static QByteArray ut_cm_readOut(QProcess *)
{
    return ut_cm_dump_out;
}

// ---------------- 采集桩 ----------------
// QProcess::start 的桩目标成员指针(全文件共用): __DBG_STUB_INVOKE__
using QProcStartFn = void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode);
static constexpr QProcStartFn ut_cm_procStartFn = &QProcess::start; // __DBG_STUB_INVOKE__

static int ut_cm_added = 0;
static void ut_cm_addMonitor(DeviceManager *, DeviceMonitor *const)
{
    ++ut_cm_added;
}

static int ut_cm_customCount = 0;
static void ut_cm_setInfoFromEdidForCustom(DeviceMonitor *, const QMap<QString, QString> &)
{
    ++ut_cm_customCount;
}

// ---------------- 与 ut_commontools.cpp 一致的 EDID 行与格式化辅助 ----------------
static const char *ut_cm_edid_lines[] = {
    "00ffffffffffff005a63384001010101",
    "0d1e010380351d782ece65a657519f27",
    "0f5054bfef80b300a940a9c095009040",
    "8180814081c0023a801871382d40582c",
    "45000f282100001e000000ff00565351",
    "3230313332313330320a000000fd0032",
    "4b185311000a202020202020000000fc",
    "005641323433302d4648440a20200141",
    "020320f14d9005040302121113141e1d",
    "1f0123097f078301000065030c001000",
    "023a801871382d40582c45000f282100",
    "001e011d8018711c1620582c25000f28",
    "2100009e011d007251d01e206e285500",
    "0f282100001e8c0ad08a20e02d10103e",
    "96000f28210000188c0ad09020403120",
    "0c4055000f28210000180000000000d6",
};
static const int ut_cm_edid_count = 16;

static QString ut_cm_toCHexdump()
{
    QString out;
    for (int i = 0; i < ut_cm_edid_count; ++i) {
        const QString line = QString::fromLatin1(ut_cm_edid_lines[i]);
        QStringList bytes;
        for (int j = 0; j + 1 < line.length(); j += 2)
            bytes << line.mid(j, 2);
        QString hexPart;
        for (int j = 0; j < bytes.size(); ++j) {
            hexPart += bytes.at(j);
            hexPart += " ";
            if (j == 7)
                hexPart += " ";
        }
        hexPart += " |";
        for (int j = 0; j < bytes.size(); ++j) {
            bool ok = false;
            int v = bytes.at(j).toInt(&ok, 16);
            hexPart += (ok && v >= 0x20 && v < 0x7f) ? QChar(v) : QChar('.');
        }
        hexPart += "|";
        out += QString("%1  %2\n").arg(i * 16, 8, 16, QLatin1Char('0')).arg(hexPart);
    }
    return out;
}

// glxinfo 输出与 DBus 回复内容须与 ut_commontools.cpp 的 preGenerateGpuInfo 用例一致
static const char *ut_cm_glxinfo =
    "name of display: :0\n"
    "OpenGL vendor string: TestVendor\n"
    "OpenGL renderer string: TestRenderer\n"
    "OpenGL version string: 3.0\n";

static bool ut_cm_devinfo_valid = true;
static QDBusMessage ut_cm_devinfo_reply;
static bool ut_cm_devinfo_callUsed = false;
static QString ut_cm_devinfo_method;

static bool ut_cm_ifValid(QDBusAbstractInterface *)
{
    return ut_cm_devinfo_valid;
}

// 同 ut_commontools.cpp: 桩接静态 systemBus() 返回无效连接, 避免 QDBusInterface 构造内省阻塞 25s
static QDBusConnection ut_cm_conn_sysBus()
{
    return QDBusConnection(QStringLiteral("__ut_no_bus__"));
}

static QDBusMessage ut_cm_ifCall1(QDBusInterface *, const QString &)
{
    ut_cm_devinfo_callUsed = true;
    return ut_cm_devinfo_reply;
}

// feedback 内 call("ServiceSession", uint8_t) 是变参模板实例化, 9 参成员指针拦截不到;
// 改桩唯一非模板实现 callWithArgumentList, 所有 call() 重载均汇入此处。
static QDBusMessage ut_cm_ifCallWal(QDBusInterface *, QDBus::CallMode, const QString &method,
                                    const QList<QVariant> &)
{
    ut_cm_devinfo_callUsed = true;
    ut_cm_devinfo_method = method;
    return QDBusMessage().createErrorReply(QStringLiteral("org.freedesktop.DBus.Error.Failed"), QStringLiteral("stub error reply"));
}

class UT_CommonToolsMore : public ::testing::Test
{
public:
    void SetUp() override
    {
        ut_cm_dump_out.clear();
        ut_cm_timeout = false;
        ut_cm_added = 0;
        ut_cm_devinfo_valid = true;
        ut_cm_devinfo_reply = QDBusMessage();
        ut_cm_devinfo_callUsed = false;
        ut_cm_devinfo_method.clear();
        ut_cm_customCount = 0;
    }
    void TearDown() override {}
};

// parseEDID: 空输入列表 => 循环体不执行
TEST_F(UT_CommonToolsMore, parseEDID_emptyList_noDispatch)
{
    // Arrange: 桩 cat 输出空串
    // Act: parseEDID 解析空输入
    // Assert: 不崩溃且不派发监视器
    EXPECT_NO_FATAL_FAILURE(CommonTools::parseEDID(QStringList(), "DP-1", false));
    EXPECT_NO_FATAL_FAILURE(CommonTools::parseEDID(QStringList(), "DP-1", true));
    EXPECT_EQ(0, ut_cm_added);
    EXPECT_EQ(0, ut_cm_customCount);
}

// parseEDID: isHW=true 且所有行 token 数 != 9(默认格式需 偏移+8词) => 全部跳过
TEST_F(UT_CommonToolsMore, parseEDID_hw_fewTokensLinesSkipped)
{
    // Arrange: 每行仅 4 个 token(hexdump -C 风格), 不满足默认格式的 9 词
    ut_cm_dump_out =
        "00000000  ff 00 ff ff ff ff 00 ff\n"
        "00000010  ff ff ff ff ff ff ff ff\n";
    Stub stub;
    stub.set(ut_cm_procStartFn, ut_cm_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_cm_wait);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_cm_readOut);
    stub.set(ADDR(DeviceManager, addMonitor), ut_cm_addMonitor);

    // Act
    CommonTools::parseEDID(QStringList() << "few-tokens", "HDMI-1", true);

    // Assert: edidStr 为空 => 行数不足不添加显示器
    EXPECT_EQ(0, ut_cm_added);
    EXPECT_EQ(0, ut_cm_customCount);
}

// parseEDID: isHW=false 且 -C 行缺少 ASCII 列(无 | 分隔) => 行全部跳过
TEST_F(UT_CommonToolsMore, parseEDID_custom_malformedLinesSkipped)
{
    // Arrange: 无 | 的行与 firstSpace>=lastPipe 的行
    ut_cm_dump_out =
        "00000000  00 ff ff ff ff ff ff 00\n"
        "|only-pipe|\n";
    Stub stub;
    stub.set(ut_cm_procStartFn, ut_cm_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_cm_wait);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_cm_readOut);
    stub.set(ADDR(DeviceManager, addMonitor), ut_cm_addMonitor);

    // Act
    CommonTools::parseEDID(QStringList() << "malformed", "HDMI-1", false);

    // Assert
    EXPECT_EQ(0, ut_cm_added);
    EXPECT_EQ(0, ut_cm_customCount);
}

// parseEDID: 多输入路径 => 每个路径各自解析并添加显示器
TEST_F(UT_CommonToolsMore, parseEDID_multiPaths_joinedInput)
{
    // Arrange
    ut_cm_dump_out = ut_cm_toCHexdump().toLocal8Bit();
    Stub stub;
    stub.set(ut_cm_procStartFn, ut_cm_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_cm_wait);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_cm_readOut);
    stub.set(ADDR(DeviceMonitor, setInfoFromEdidForCustom), ut_cm_setInfoFromEdidForCustom);
    stub.set(ADDR(DeviceManager, addMonitor), ut_cm_addMonitor);

    // Act
    CommonTools::parseEDID(QStringList() << "edid-a" << "edid-b", "DP-1", false);

    // Assert: 两个输入路径各添加 1 台, 且均走自定义派发
    EXPECT_EQ(2, ut_cm_added);
    EXPECT_EQ(2, ut_cm_customCount);
}

// parseEDID: isHW=false 且数据行非 32 字符 => 行被过滤, 有效行数不足 => 跳过
TEST_F(UT_CommonToolsMore, parseEDID_custom_lineNot32CharsSkipped)
{
    // Arrange: 首行 16 字节(32 字符)有效, 后两行各 15 字节(30 字符)被过滤
    ut_cm_dump_out =
        "00000000  00 ff ff ff ff ff ff 00  5a 63 38 40 01 01 01 01  |........Zc8@....|\n"
        "00000010  0d 1e 01 03 80 35 1d 78  2e ce 65 a6 57 51 9f  |.....5.x..e.WQ.|\n"
        "0000001f  27 0f 50 54 bf ef 80 b3  00 a9 40 a9 c0 95 00  |'..PT......@....|\n";
    Stub stub;
    stub.set(ut_cm_procStartFn, ut_cm_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_cm_wait);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_cm_readOut);
    stub.set(ADDR(DeviceManager, addMonitor), ut_cm_addMonitor);

    // Act
    CommonTools::parseEDID(QStringList() << "short-line", "HDMI-1", false);

    // Assert: edidStr 仅 1 行 => lines.size() <= 3 => 不添加
    EXPECT_EQ(0, ut_cm_added);
    EXPECT_EQ(0, ut_cm_customCount);
}

// preGenerateGpuInfo: 静态缓存二次调用 => 两次结果一致
TEST_F(UT_CommonToolsMore, preGenerateGpuInfo_secondCall_cacheHit)
{
    // Arrange: 与 ut_commontools.cpp 相同的注入内容, 保证任意执行顺序下断言一致
    ut_cm_dump_out = ut_cm_glxinfo;
    ut_cm_devinfo_valid = true;
    ut_cm_devinfo_reply = QDBusMessage().createReply(QVariant(QStringLiteral("GPU-MEM-INFO")));
    Stub stub;
    stub.set(ut_cm_procStartFn, ut_cm_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_cm_wait);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_cm_readOut);
    stub.set(ADDR(QDBusInterface, isValid), ut_cm_ifValid);
    QDBusMessage (QDBusInterface::*pc1)(const QString &) = &QDBusInterface::call;
    stub.set(pc1, ut_cm_ifCall1);
    QDBusConnection (*psysBus)() = &QDBusConnection::systemBus;
    stub.set(psysBus, ut_cm_conn_sysBus);

    // Act
    QString first = CommonTools::preGenerateGpuInfo();
    QString second = CommonTools::preGenerateGpuInfo();

    // Assert: 缓存命中, 结果一致
    EXPECT_EQ(first.toStdString(), second.toStdString());
    EXPECT_TRUE(first.contains("TestRenderer"));
    EXPECT_TRUE(first.endsWith("GPU-MEM-INFO"));
}

// getGpuBaseInfo: 解析 glxinfo 键值, 无关键(如 version)被过滤
TEST_F(UT_CommonToolsMore, getGpuBaseInfo_glxinfoValues_mapFilled)
{
    // Arrange
    ut_cm_dump_out = ut_cm_glxinfo;
    Stub stub;
    stub.set(ut_cm_procStartFn, ut_cm_start);
    stub.set(ADDR(QProcess, waitForFinished), ut_cm_wait);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_cm_readOut);

    // Act
    QMap<QString, QString> mapInfo;
    bool ret = CommonTools::getGpuBaseInfo(mapInfo);

    // Assert: 成功; 仅 vendor/renderer 相关键入库(version 行被过滤)
    EXPECT_TRUE(ret);
    EXPECT_EQ(3, mapInfo.size());
    EXPECT_TRUE(mapInfo.values().contains(QStringLiteral("TestRenderer")));
    EXPECT_TRUE(mapInfo.values().contains(QStringLiteral("TestVendor")));
    EXPECT_FALSE(mapInfo.values().contains(QStringLiteral("3.0")));
}

// feedback: DBus 调用返回错误回复 => 失败日志分支且不崩溃
TEST_F(UT_CommonToolsMore, feedback_errorReply_dispatchStillRuns)
{
    // Arrange: feedback 无 isValid 检查, call 总会发生; 桩返回错误回复
    Stub stub;
    QDBusMessage (QDBusInterface::*pcwal)(QDBus::CallMode, const QString &, const QList<QVariant> &) =
        &QDBusInterface::callWithArgumentList;
    stub.set(pcwal, ut_cm_ifCallWal);

    // Act
    EXPECT_NO_FATAL_FAILURE(CommonTools::feedback());

    // Assert: 确实发起了 ServiceSession 调用
    EXPECT_TRUE(ut_cm_devinfo_callUsed);
    EXPECT_STREQ("ServiceSession", ut_cm_devinfo_method.toStdString().c_str());
}
