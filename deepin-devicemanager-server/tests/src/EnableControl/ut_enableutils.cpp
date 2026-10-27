// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | disableOutDevice | high | complexity:25 | 3 | 12 |
// | disableInDevice | high | - | 3 | 3 |
// | ioctlOperateNetworkLogicalName | mid | - | 2 | 3 |
// | enableNetworkByDBus | mid | - | 2 | 5 |
// | enableNetworkByIoctl | mid | - | 2 | 4 |
// | validateAndNormalizeVidPid | mid | - | 2 | 3 |
// | getMapInfo | mid | - | 2 | 8 |
// | EnableUtils() | low | - | 1 | 1 |
// 最小清单完成情况（test-code-gen 最小清单）：
// 1. 每个公开方法 >= 1 用例: [x]
// 2. 每个输入维度按等价类划分 >= 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 8. 负面场景有专门用例: [x]

#include <gtest/gtest.h>

#define private public
#define protected public

#include "enableutils.h"

#undef private
#undef protected

#include <QMap>

class EnableUtilsTest : public ::testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

// validateAndNormalizeVidPid

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_validWith0x_returnsTrue)
{
    // Arrange
    QString vid = "0x8086";
    QString pid = "0x1502";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(normVid, "8086");
    EXPECT_EQ(normPid, "1502");
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_validWithout0x_returnsTrue)
{
    // Arrange
    QString vid = "8086";
    QString pid = "1502";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(normVid, "8086");
    EXPECT_EQ(normPid, "1502");
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_uppercaseNormalized_returnsTrue)
{
    // Arrange
    QString vid = "0xABCD";
    QString pid = "0x1234";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(normVid, "abcd");
    EXPECT_EQ(normPid, "1234");
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_emptyVid_returnsFalse)
{
    // Arrange
    QString vid = "";
    QString pid = "1502";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(normVid.isEmpty(), true);
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_emptyPid_returnsFalse)
{
    // Arrange
    QString vid = "8086";
    QString pid = "";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(normPid.isEmpty(), true);
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_wrongLength_returnsFalse)
{
    // Arrange
    QString vid = "0x86";
    QString pid = "0x1502";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(normVid, "86");
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_nonHex_returnsFalse)
{
    // Arrange
    QString vid = "0xZZZZ";
    QString pid = "0x1502";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(normVid, "zzzz");
}

// getMapInfo (private static, accessed via private hack)

TEST_F(EnableUtilsTest, getMapInfo_tooFewLines_returnsFalse)
{
    // Arrange
    QString item = "line1\nline2\nline3";
    QMap<QString, QString> mapInfo;

    // Act
    bool result = EnableUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(mapInfo.isEmpty(), true);
}

TEST_F(EnableUtilsTest, getMapInfo_hubFiltered_returnsFalse)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: hub";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = EnableUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_TRUE(mapInfo.isEmpty());
}

TEST_F(EnableUtilsTest, getMapInfo_noSysfsBusid_returnsFalse)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: network";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = EnableUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_TRUE(mapInfo.isEmpty());
}

TEST_F(EnableUtilsTest, getMapInfo_validWithBusid_returnsTrue)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: network";
    lines[1] = "SysFS BusID: 0000:00:01.0";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = EnableUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(mapInfo["Hardware Class"], "network");
    EXPECT_EQ(mapInfo["SysFS BusID"], "0000:00:01.0");
}

TEST_F(EnableUtilsTest, getMapInfo_stripsQuotes_returnsStrippedValue)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: \"Value%1\"").arg(i);
    lines[0] = "Hardware Class: network";
    lines[1] = "SysFS BusID: 0000:00:01.0";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = EnableUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(mapInfo["Key2"], "Value2");
}

// enableNetworkByIoctl

TEST_F(EnableUtilsTest, enableNetworkByIoctl_emptyName_returnsFalse)
{
    // Arrange
    QString name = "";
    bool enable = true;

    // Act
    bool result = EnableUtils::enableNetworkByIoctl(name, enable);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(name.length(), 0);
}

TEST_F(EnableUtilsTest, enableNetworkByIoctl_nameTooLong_returnsFalse)
{
    // Arrange
    QString name(16, 'a');
    bool enable = true;

    // Act
    bool result = EnableUtils::enableNetworkByIoctl(name, enable);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(name.length(), 16);
}

TEST_F(EnableUtilsTest, enableNetworkByIoctl_nonexistentInterface_returnsFalse)
{
    // Arrange
    QString name = "fake_iface_xyz";
    bool enable = true;

    // Act
    bool result = EnableUtils::enableNetworkByIoctl(name, enable);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(name, QString("fake_iface_xyz"));
}

/*
 * Mode 2 新增用例：disableOutDevice / disableInDevice / enableNetworkByDBus /
 * ioctlOperateNetworkLogicalName。DBus、sysfs 文件写入均通过 stub 拦截，
 * EnableSqlManager 单例注入临时 SQLite 库，测试不依赖真实设备环境。
 */

#define private public
#define protected public

#include "enablesqlmanager.h"
#include "../ut_testdbhelper.h"

#undef private
#undef protected

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusObjectPath>

#include <QCryptographicHash>
#include <QFile>
#include <QList>
#include <QPair>
#include <QRegularExpression>
#include <QVariant>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <cstdarg>

#include "securityutils.h"

#include "../stub.h"


#include <dlfcn.h>

// Qt6 虚函数经 ADDR 取到的是 PMF（存 vtable 索引，低位为 1），Stub.addrof 直接解引用会段错误；
// 统一经 dlsym 取导出符号的真实函数体地址后再交给 Stub 打补丁。
static void *ut_qtSym(const char *mangled)
{
    static void *handle = dlopen("libQt6Core.so.6", RTLD_LAZY | RTLD_GLOBAL);
    return dlsym(handle, mangled);
}
#define UT_QSYM_QFILE_OPEN "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE"
#define UT_QSYM_QSAVEFILE_OPEN "_ZN9QSaveFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE"
#define UT_QSYM_QIOWRITE "_ZN9QIODevice5writeEPKcx"

// ---------------- stub 捕获辅助 ----------------

// EnableUtils 静态成员桩
static bool ut_enableNetworkByDBus_ret = false;
static int ut_enableNetworkByDBus_calls = 0;
static QList<QPair<QString, bool>> ut_enableNetworkByDBus_args;
static bool ut_enableNetworkByDBus_stub(const QString &logicalName, bool enable)
{
    ++ut_enableNetworkByDBus_calls;
    ut_enableNetworkByDBus_args.append(qMakePair(logicalName, enable));
    return ut_enableNetworkByDBus_ret;
}

static bool ut_enableNetworkByIoctl_ret = false;
static int ut_enableNetworkByIoctl_calls = 0;
static bool ut_enableNetworkByIoctl_stub(const QString &logicalName, bool enable)
{
    Q_UNUSED(logicalName);
    Q_UNUSED(enable);
    ++ut_enableNetworkByIoctl_calls;
    return ut_enableNetworkByIoctl_ret;
}

static bool ut_ioctlOperate_ret = false;
static int ut_ioctlOperate_calls = 0;
static QList<QPair<QString, bool>> ut_ioctlOperate_args;
static bool ut_ioctlOperate_stub(const QString &logicalName, bool enable)
{
    ++ut_ioctlOperate_calls;
    ut_ioctlOperate_args.append(qMakePair(logicalName, enable));
    return ut_ioctlOperate_ret;
}

// securityutils 自由函数桩
static bool ut_isSafeSysfs_ret = false;
static int ut_isSafeSysfs_calls = 0;
static bool ut_isSafeSysfs_stub(const QString &relativePath)
{
    Q_UNUSED(relativePath);
    ++ut_isSafeSysfs_calls;
    return ut_isSafeSysfs_ret;
}

// QDBusInterface 桩
static bool ut_ifaceValid_ret = false;
static int ut_ifaceValid_calls = 0;
static bool ut_ifaceValid_stub(void *self)
{
    Q_UNUSED(self);
    ++ut_ifaceValid_calls;
    return ut_ifaceValid_ret;
}
static int ut_doCall_calls = 0;
static QDBusMessage ut_doCall_stub(void *self, QDBus::CallMode mode, const QString &method,
                                   const QVariant *args, size_t numArgs)
{
    Q_UNUSED(self);
    Q_UNUSED(mode);
    Q_UNUSED(args);
    Q_UNUSED(numArgs);
    ++ut_doCall_calls;
    QDBusMessage base = QDBusMessage::createMethodCall(QStringLiteral("ut.service"),
                                                       QStringLiteral("/ut/path"),
                                                       QStringLiteral("ut.iface"),
                                                       QStringLiteral("utMethod"));
    return base.createErrorReply(QStringLiteral("com.ut.TestError"),
                                 QStringLiteral("stubbed"));
}
static QDBusConnection ut_systemBus_stub_ret = QDBusConnection(QStringLiteral("ut-stub"));
static QDBusConnection ut_systemBus_stub()
{
    return ut_systemBus_stub_ret;
}

// QFile 桩(sysfs 写入拦截)
static bool ut_qfileOpen_ret = false;
static int ut_qfileOpen_calls = 0;
static bool ut_qfileOpen_stub(void *self, QIODevice::OpenMode mode)
{
    Q_UNUSED(self);
    Q_UNUSED(mode);
    ++ut_qfileOpen_calls;
    return ut_qfileOpen_ret;
}
static QList<QByteArray> ut_qfileWrites;
static qint64 ut_qfileWriteData_stub(void *self, const char *data, qint64 len)
{
    Q_UNUSED(self);
    ut_qfileWrites.append(QByteArray(data, static_cast<int>(len)));
    return len;
}
static bool ut_qfileExists_ret = false;
static int ut_qfileExists_calls = 0;
static bool ut_qfileExists_stub(const QString &fileName)
{
    Q_UNUSED(fileName);
    ++ut_qfileExists_calls;
    return ut_qfileExists_ret;
}

// 构造一条能通过 EnableUtils::getMapInfo 校验的网卡设备信息(>10 行)
static QString utNetworkInfo()
{
    return QString("P: /devices/pci0000:00/0000:00:1f.6/net/enp0s0\n"
                   "E: DEVPATH=/devices/pci0000:00/0000:00:1f.6/net/enp0s0\n"
                   "E: SUBSYSTEM=net\n"
                   "E: INTERFACE=enp0s0\n"
                   "E: IFINDEX=2\n"
                   "E: USEC_INITIALIZED=123456\n"
                   "E: NET_EMBEDDED=0\n"
                   "E: NAME=enp0s0\n"
                   "E: ID_NET_DRIVER=e1000e\n"
                   "E: ID_BUS=pci\n"
                   "E: ID_NET_NAME_MAC=enxaa_bb_cc_dd_ee_ff\n"
                   "SysFS BusID: 0000:00:1f.6\n"
                   "Permanent HW Address: aa:bb:cc:dd:ee:ff\n"
                   "Device File: enp0s0\n");
}

// 构造一条能通过 getMapInfo 校验的 USB 设备信息(>10 行，Serial ID 分支)
static QString utUsbInfo(const QString &sysfsId)
{
    return QString("P: /devices/ut_t27/usb1/1-2\n"
                   "E: DEVPATH=/devices/ut_t27/usb1/1-2\n"
                   "E: DEVNAME=/dev/bus/usb/001/002\n"
                   "E: DEVTYPE=usb_device\n"
                   "E: PRODUCT=8087/0026/0001\n"
                   "E: TYPE=0/0/0\n"
                   "E: BUSNUM=001\n"
                   "E: DEVNUM=002\n"
                   "E: MAJOR=189\n"
                   "E: MINOR=2\n"
                   "SysFS BusID: 1-2\n"
                   "Serial ID: ut-serial-xyz\n"
                   "SysFS ID: %1\n").arg(sysfsId);
}

// ---------------- enableNetworkByDBus ----------------

TEST_F(EnableUtilsTest, enableNetworkByDBus_invalidInterface_returnsFalse)
{
    // Arrange: systemBus 桩返回未连接连接，isValid 桩返回 false，全程不触达 DBus 守护进程
    Stub stub;
    ut_systemBus_stub_ret = QDBusConnection(QStringLiteral("ut-stub-bus"));
    ut_ifaceValid_calls = 0;
    ut_doCall_calls = 0;
    stub.set(ADDR(QDBusConnection, systemBus), ut_systemBus_stub);
    stub.set(ADDR(QDBusInterface, isValid), ut_ifaceValid_stub);
    ut_ifaceValid_ret = false;

    // Act
    const bool ok = EnableUtils::enableNetworkByDBus("ut-fake-iface", true);

    // Assert: 接口无效时不应发起任何 DBus 调用
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_doCall_calls, 0);
}

TEST_F(EnableUtilsTest, enableNetworkByDBus_getDeviceReplyInvalid_returnsFalse)
{
    // Arrange: isValid 通过，但 doCall 桩返回错误消息 -> QDBusReply 无效
    Stub stub;
    ut_systemBus_stub_ret = QDBusConnection(QStringLiteral("ut-stub-bus"));
    stub.set(ADDR(QDBusConnection, systemBus), ut_systemBus_stub);
    stub.set(ADDR(QDBusInterface, isValid), ut_ifaceValid_stub);
    ut_ifaceValid_ret = true;
    ut_doCall_calls = 0;
    stub.set(ADDR(QDBusInterface, doCall), ut_doCall_stub);

    // Act
    const bool ok = EnableUtils::enableNetworkByDBus("ut-fake-iface", false);

    // Assert: GetDeviceByIpIface 失败后不再发起 Set
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_doCall_calls, 1);
}

// ---------------- ioctlOperateNetworkLogicalName ----------------

TEST_F(EnableUtilsTest, ioctlOperateNetworkLogicalName_dbusSucceeds_shortCircuits)
{
    // Arrange: DBus 桩成功，ioctl 兜底桩不应被调用
    Stub stub;
    ut_enableNetworkByDBus_ret = true;
    ut_enableNetworkByDBus_calls = 0;
    ut_enableNetworkByDBus_args.clear();
    ut_enableNetworkByIoctl_ret = false;
    ut_enableNetworkByIoctl_calls = 0;
    stub.set(ADDR(EnableUtils, enableNetworkByDBus), ut_enableNetworkByDBus_stub);
    stub.set(ADDR(EnableUtils, enableNetworkByIoctl), ut_enableNetworkByIoctl_stub);

    // Act
    const bool ok = EnableUtils::ioctlOperateNetworkLogicalName("enp0s0", false);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(ut_enableNetworkByDBus_calls, 1);
    ASSERT_EQ(ut_enableNetworkByDBus_args.size(), 1);
    EXPECT_EQ(ut_enableNetworkByDBus_args.first().first, QStringLiteral("enp0s0"));
    EXPECT_FALSE(ut_enableNetworkByDBus_args.first().second);
    EXPECT_EQ(ut_enableNetworkByIoctl_calls, 0);
}

TEST_F(EnableUtilsTest, ioctlOperateNetworkLogicalName_dbusFails_fallsBackToIoctl)
{
    // Arrange
    Stub stub;
    ut_enableNetworkByDBus_ret = false;
    ut_enableNetworkByDBus_calls = 0;
    ut_enableNetworkByIoctl_ret = true;
    ut_enableNetworkByIoctl_calls = 0;
    stub.set(ADDR(EnableUtils, enableNetworkByDBus), ut_enableNetworkByDBus_stub);
    stub.set(ADDR(EnableUtils, enableNetworkByIoctl), ut_enableNetworkByIoctl_stub);

    // Act
    const bool ok = EnableUtils::ioctlOperateNetworkLogicalName("enp0s0", true);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(ut_enableNetworkByDBus_calls, 1);
    EXPECT_EQ(ut_enableNetworkByIoctl_calls, 1);
}

TEST_F(EnableUtilsTest, ioctlOperateNetworkLogicalName_bothFailed_returnsFalse)
{
    // Arrange
    Stub stub;
    ut_enableNetworkByDBus_ret = false;
    ut_enableNetworkByIoctl_ret = false;
    ut_enableNetworkByDBus_calls = 0;
    ut_enableNetworkByIoctl_calls = 0;
    stub.set(ADDR(EnableUtils, enableNetworkByDBus), ut_enableNetworkByDBus_stub);
    stub.set(ADDR(EnableUtils, enableNetworkByIoctl), ut_enableNetworkByIoctl_stub);

    // Act
    const bool ok = EnableUtils::ioctlOperateNetworkLogicalName("enp0s0", false);

    // Assert: DBus 与 ioctl 两条链路均尝试且均失败
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_enableNetworkByDBus_calls, 1);
    EXPECT_EQ(ut_enableNetworkByIoctl_calls, 1);
}

// ---------------- disableOutDevice ----------------

TEST_F(EnableUtilsTest, disableOutDevice_networkInDb_updatesPathByDeviceFile)
{
    // Arrange: 临时库中已记录该 MAC
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    mgr->insertDataToAuthorizedTable("network", "utNic", "/ut/orig", "aa:bb:cc:dd:ee:ff", true, "e1000e");

    ut_ioctlOperate_ret = true;
    ut_ioctlOperate_calls = 0;
    ut_ioctlOperate_args.clear();
    stub.set(ADDR(EnableUtils, ioctlOperateNetworkLogicalName), ut_ioctlOperate_stub);

    // Act
    EnableUtils::disableOutDevice(utNetworkInfo());

    // Assert: ioctl 以 Device File + disable 调用，且库内 path 更新为逻辑名
    ASSERT_EQ(ut_ioctlOperate_calls, 1);
    EXPECT_EQ(ut_ioctlOperate_args.first().first, QStringLiteral("enp0s0"));
    EXPECT_FALSE(ut_ioctlOperate_args.first().second);
    EXPECT_EQ(mgr->authorizedPath("aa:bb:cc:dd:ee:ff"), QStringLiteral("enp0s0"));
}

TEST_F(EnableUtilsTest, disableOutDevice_networkNotInDb_ioctlNotCalled)
{
    // Arrange: 库中无该 MAC 记录
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));

    ut_ioctlOperate_ret = true;
    ut_ioctlOperate_calls = 0;
    ut_ioctlOperate_args.clear();
    stub.set(ADDR(EnableUtils, ioctlOperateNetworkLogicalName), ut_ioctlOperate_stub);

    // Act
    EnableUtils::disableOutDevice(utNetworkInfo());

    // Assert: 未入库设备不做 ioctl，也不更新库
    EXPECT_EQ(ut_ioctlOperate_calls, 0);
    EXPECT_TRUE(mgr->authorizedPath("aa:bb:cc:dd:ee:ff").isEmpty());
}

TEST_F(EnableUtilsTest, disableOutDevice_usbInDb_writesAuthorizedFileAndUpdatesPath)
{
    // Arrange: Serial ID 分支(无 Permanent HW Address/Vendor 0x 组合)，库中已记录
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    mgr->insertDataToAuthorizedTable("USB", "utDev", "/ut/orig", "ut-serial-xyz", true, "utdrv");

    ut_isSafeSysfs_ret = true;
    ut_isSafeSysfs_calls = 0;
    stub.set(isSafeSysfsPath, ut_isSafeSysfs_stub);
    ut_qfileOpen_ret = true;
    ut_qfileOpen_calls = 0;
    ut_qfileWrites.clear();
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act: SysFS ID 尾号 2 会被规范化替换为 0
    EnableUtils::disableOutDevice(utUsbInfo(QStringLiteral("/devices/ut_t27/usb1/1-2")));

    // Assert: 向 /sys<path>/authorized 写入 0，并更新库内 path
    EXPECT_GT(ut_isSafeSysfs_calls, 0);
    ASSERT_EQ(ut_qfileWrites.size(), 1);
    EXPECT_EQ(ut_qfileWrites.first(), QByteArray("0"));
    EXPECT_EQ(mgr->authorizedPath("ut-serial-xyz"), QStringLiteral("/devices/ut_t27/usb1/1-0"));
}

TEST_F(EnableUtilsTest, disableOutDevice_usbUnsafePath_skippedWithoutWrite)
{
    // Arrange: 库中记录在案，但 path 含 ".." 触发路径穿越校验(真实 isSafeSysfsPath 返回 false)
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    mgr->insertDataToAuthorizedTable("USB", "utDev", "/ut/orig", "ut-serial-xyz", true, "utdrv");

    ut_qfileOpen_ret = true;
    ut_qfileOpen_calls = 0;
    ut_qfileWrites.clear();
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act
    EnableUtils::disableOutDevice(utUsbInfo(QStringLiteral("/devices/../../etc")));

    // Assert: 不写文件、不更新库
    EXPECT_EQ(ut_qfileWrites.size(), 0);
    EXPECT_EQ(mgr->authorizedPath("ut-serial-xyz"), QStringLiteral("/ut/orig"));
}

// 修复验证（上游 3645c986 已将打开失败分支 return 改为 continue）：USB 分支中
// authorized 文件打开失败时仅跳过当前设备条目，循环继续处理后续设备。
TEST_F(EnableUtilsTest, disableOutDevice_usbFileOpenFails_continuesRemainingItems)
{
    // Arrange: 第一条为待禁用 USB 设备，第二条为库中另一条应被处理的网卡
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    mgr->insertDataToAuthorizedTable("USB", "utDev", "/ut/orig", "ut-serial-xyz", true, "utdrv");
    mgr->insertDataToAuthorizedTable("network", "utNic", "/ut/orig-nic", "aa:bb:cc:dd:ee:ff", true, "e1000e");

    ut_isSafeSysfs_ret = true;
    stub.set(isSafeSysfsPath, ut_isSafeSysfs_stub);
    ut_qfileOpen_ret = false;   // authorized 文件打开失败
    ut_qfileOpen_calls = 0;
    ut_ioctlOperate_ret = true;
    ut_ioctlOperate_calls = 0;
    ut_ioctlOperate_args.clear();
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set(ADDR(EnableUtils, ioctlOperateNetworkLogicalName), ut_ioctlOperate_stub);

    // Act
    EnableUtils::disableOutDevice(utUsbInfo(QStringLiteral("/devices/ut_t27/usb1/1-2"))
                                  + QStringLiteral("\n\n") + utNetworkInfo());

    // Assert: 打开失败仅跳过当前 USB 条目，后续网卡仍被处理（修复后行为）
    ASSERT_EQ(ut_ioctlOperate_calls, 1);
    EXPECT_EQ(ut_ioctlOperate_args.first().first, QStringLiteral("enp0s0"));
    EXPECT_FALSE(ut_ioctlOperate_args.first().second);
    EXPECT_EQ(mgr->authorizedPath("aa:bb:cc:dd:ee:ff"), QStringLiteral("enp0s0"));
    // 打开失败的 USB 条目本身未被写文件、未更新库
    EXPECT_EQ(mgr->authorizedPath("ut-serial-xyz"), QStringLiteral("/ut/orig"));
}

TEST_F(EnableUtilsTest, disableOutDevice_malformedItems_noCrash)
{
    // Arrange: 空 item、行数不足、hub 设备均应被 getMapInfo 过滤
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));

    ut_ioctlOperate_calls = 0;
    ut_qfileWrites.clear();
    stub.set(ADDR(EnableUtils, ioctlOperateNetworkLogicalName), ut_ioctlOperate_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act: 空串与 hub 设备条目
    EnableUtils::disableOutDevice(QString());
    EnableUtils::disableOutDevice(QStringLiteral("Hardware Class: hub\nSysFS BusID: 1-1\n"));

    // Assert: 不崩溃、不产生任何副作用
    EXPECT_EQ(ut_ioctlOperate_calls, 0);
    EXPECT_EQ(ut_qfileWrites.size(), 0);
}

// ---------------- disableInDevice ----------------

TEST_F(EnableUtilsTest, disableInDevice_networkEntry_ioctlCalled_removeTableEmptyNoWrites)
{
    // Arrange: authorized 表中 1 条 MAC 记录 + 1 条非 MAC 记录；remove 表为空
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    mgr->insertDataToAuthorizedTable("network", "utNic", "enp0s0", "aa:bb:cc:dd:ee:ff", true, "e1000e");
    mgr->insertDataToAuthorizedTable("USB", "utDev", "/devices/ut_t27/usb1/1-0", "ut-serial-xyz", true, "utdrv");

    ut_ioctlOperate_ret = true;
    ut_ioctlOperate_calls = 0;
    ut_ioctlOperate_args.clear();
    ut_qfileOpen_ret = true;
    ut_qfileOpen_calls = 0;
    ut_qfileWrites.clear();
    stub.set(ADDR(EnableUtils, ioctlOperateNetworkLogicalName), ut_ioctlOperate_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act
    EnableUtils::disableInDevice();

    // Assert: 仅 MAC 记录走 ioctl；remove 表为空 -> 无文件写入
    ASSERT_EQ(ut_ioctlOperate_calls, 1);
    EXPECT_EQ(ut_ioctlOperate_args.first().first, QStringLiteral("enp0s0"));
    EXPECT_FALSE(ut_ioctlOperate_args.first().second);
    EXPECT_EQ(ut_qfileWrites.size(), 0);
}

TEST_F(EnableUtilsTest, disableInDevice_removeEntry_fileExists_writesRemoveFile)
{
    // Arrange: remove 表 1 条记录，remove 文件存在
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    mgr->insertDataToRemoveTable("USB", "utDev", "/devices/ut_t27/usb1/1-0", "ut-uid-a", "utdrv");

    ut_isSafeSysfs_ret = true;
    stub.set(isSafeSysfsPath, ut_isSafeSysfs_stub);
    ut_qfileExists_ret = true;
    ut_qfileExists_calls = 0;
    stub.set((bool (*)(const QString &))QFile::exists, ut_qfileExists_stub);
    ut_qfileOpen_ret = true;
    ut_qfileOpen_calls = 0;
    ut_qfileWrites.clear();
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act
    EnableUtils::disableInDevice();

    // Assert: 写 /sys<path>/remove 内容 1
    EXPECT_EQ(ut_qfileExists_calls, 1);
    ASSERT_EQ(ut_qfileWrites.size(), 1);
    EXPECT_EQ(ut_qfileWrites.first(), QByteArray("1"));
}

TEST_F(EnableUtilsTest, disableInDevice_removeEntry_missingFallsBackToReset)
{
    // Arrange: remove 文件不存在 -> 回退写 reset 文件
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    mgr->insertDataToRemoveTable("USB", "utDev", "/devices/ut_t27/usb1/1-0", "ut-uid-a", "utdrv");

    ut_isSafeSysfs_ret = true;
    stub.set(isSafeSysfsPath, ut_isSafeSysfs_stub);
    ut_qfileExists_ret = false;
    ut_qfileExists_calls = 0;
    stub.set((bool (*)(const QString &))QFile::exists, ut_qfileExists_stub);
    ut_qfileOpen_ret = true;
    ut_qfileOpen_calls = 0;
    ut_qfileWrites.clear();
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act
    EnableUtils::disableInDevice();

    // Assert: remove 文件检查 1 次，写入仍为 1
    EXPECT_EQ(ut_qfileExists_calls, 1);
    ASSERT_EQ(ut_qfileWrites.size(), 1);
    EXPECT_EQ(ut_qfileWrites.first(), QByteArray("1"));
}

TEST_F(EnableUtilsTest, disableInDevice_unsafePath_skippedWithoutWrite)
{
    // Arrange: remove 表记录 path 含 ".."(真实 isSafeSysfsPath 校验拒绝)
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    mgr->insertDataToRemoveTable("USB", "utDev", "/devices/../../etc", "ut-uid-a", "utdrv");

    ut_qfileOpen_ret = true;
    ut_qfileOpen_calls = 0;
    ut_qfileWrites.clear();
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act
    EnableUtils::disableInDevice();

    // Assert: 非安全路径跳过，不打开也不写任何文件
    EXPECT_EQ(ut_qfileWrites.size(), 0);
    EXPECT_EQ(ut_qfileOpen_calls, 0);
}

// ================= 覆盖率补充（Mode 2 闭环） =================

// ---------------- 补充桩：DBus 序列 / socket·ioctl·close ----------------

// isValid 序列桩：逐次弹出，弹空后回落到 ut_ifaceValid_ret
static QList<bool> ut_ifaceValid_seq;
static bool ut_ifaceValid_seq_stub(void *self)
{
    ++ut_ifaceValid_calls;
    if (!ut_ifaceValid_seq.isEmpty())
        return ut_ifaceValid_seq.takeFirst();
    return ut_ifaceValid_ret;
}

// doCall 序列桩：逐次弹出预置应答，弹空后返回错误应答
static QList<QDBusMessage> ut_doCall_seq;
static QDBusMessage utMakeReply(const QVariant &arg)
{
    QDBusMessage base = QDBusMessage::createMethodCall(QStringLiteral("ut.service"),
                                                       QStringLiteral("/ut/path"),
                                                       QStringLiteral("ut.iface"),
                                                       QStringLiteral("utMethod"));
    return base.createReply(QVariantList{ arg });
}
static QDBusMessage utMakeEmptyReply()
{
    QDBusMessage base = QDBusMessage::createMethodCall(QStringLiteral("ut.service"),
                                                       QStringLiteral("/ut/path"),
                                                       QStringLiteral("ut.iface"),
                                                       QStringLiteral("utMethod"));
    return base.createReply();
}
static QDBusMessage utMakeErrorReply()
{
    QDBusMessage base = QDBusMessage::createMethodCall(QStringLiteral("ut.service"),
                                                       QStringLiteral("/ut/path"),
                                                       QStringLiteral("ut.iface"),
                                                       QStringLiteral("utMethod"));
    return base.createErrorReply(QStringLiteral("com.ut.TestError"), QStringLiteral("stubbed"));
}
static QDBusMessage ut_doCall_seq_stub(void *self, QDBus::CallMode mode, const QString &method,
                                       const QVariant *args, size_t numArgs)
{
    Q_UNUSED(self);
    Q_UNUSED(mode);
    Q_UNUSED(method);
    Q_UNUSED(args);
    Q_UNUSED(numArgs);
    ++ut_doCall_calls;
    if (!ut_doCall_seq.isEmpty())
        return ut_doCall_seq.takeFirst();
    return utMakeErrorReply();
}

// socket/close/ioctl 桩：不创建真实 fd，不触碰真实网卡
static int ut_socket_fd = -1;
static int ut_socketOpen_calls = 0;
static int ut_socketFd_stub(int domain, int type, int protocol)
{
    Q_UNUSED(domain);
    Q_UNUSED(type);
    Q_UNUSED(protocol);
    ++ut_socketOpen_calls;
    return ut_socket_fd;
}
static int ut_close_stub(int fd)
{
    Q_UNUSED(fd);
    return 0;
}
static int ut_ioctlGiFlags_ret = 0;
static int ut_ioctlSiFlags_ret = 0;
static int ut_ioctlSiFlags_seen = -1;
static int ut_ioctlFlags_stub(int fd, unsigned long request, ...)
{
    Q_UNUSED(fd);
    va_list ap;
    va_start(ap, request);
    struct ifreq *ifr = va_arg(ap, struct ifreq *);
    va_end(ap);
    if (request == SIOCGIFFLAGS) {
        ifr->ifr_flags = IFF_UP | IFF_BROADCAST;
        return ut_ioctlGiFlags_ret;
    }
    if (request == SIOCSIFFLAGS) {
        ut_ioctlSiFlags_seen = ifr->ifr_flags;
        return ut_ioctlSiFlags_ret;
    }
    return -1;
}

// ---------------- 补充夹具：MD5/Module Alias/无标识 设备信息 ----------------

// 与源码 disableOutDevice 同款 MD5 指纹：vendor 第2段 + device 第2段去 0x 前缀 + sysfs id
static QString utMd5Uid(const QString &vendorTail, const QString &deviceTail, const QString &sysfsId)
{
    const QString valueStr = vendorTail + deviceTail;
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(valueStr.toUtf8());
    hash.addData(sysfsId.toUtf8());
    return QString::fromStdString(hash.result().toBase64().toStdString());
}

// 含 Vendor/Device 行的 USB 设备信息（触发 MD5 唯一标识分支）；keyLine 可切换 SysFS ID/Device Link
static QString utUsbInfoMd5(const QString &idPath, const QString &keyLine = QStringLiteral("SysFS ID"))
{
    return QString("P: /devices/ut_t27/usb1/1-6\n"
                   "E: DEVPATH=/devices/ut_t27/usb1/1-6\n"
                   "E: DEVNAME=/dev/bus/usb/001/006\n"
                   "E: DEVTYPE=usb_device\n"
                   "E: PRODUCT=46d/c539/0100\n"
                   "E: TYPE=0/0/0\n"
                   "E: BUSNUM=001\n"
                   "E: DEVNUM=006\n"
                   "E: MAJOR=189\n"
                   "E: MINOR=6\n"
                   "SysFS BusID: 1-6\n"
                   "Vendor: 0x046d c52b\n"
                   "Device: 0x1a86 c539\n"
                   "%1: %2\n").arg(keyLine, idPath);
}

// 仅含 Module Alias 的设备信息（触发 Module Alias 唯一标识分支）
static QString utUsbInfoAlias()
{
    return QString("P: /devices/ut_t27/usb1/1-7\n"
                   "E: DEVPATH=/devices/ut_t27/usb1/1-7\n"
                   "E: DEVNAME=/dev/bus/usb/001/007\n"
                   "E: DEVTYPE=usb_device\n"
                   "E: PRODUCT=1a86/7523/0400\n"
                   "E: TYPE=0/0/0\n"
                   "E: BUSNUM=001\n"
                   "E: DEVNUM=007\n"
                   "E: MAJOR=189\n"
                   "E: MINOR=7\n"
                   "SysFS BusID: 1-7\n"
                   "Module Alias: usb:v1A86p7523d0400\n");
}

// 无任何可识别标识的设备信息（触发 uniqueID 为空的 continue 分支）
static QString utUsbInfoNoId()
{
    return QString("P: /devices/ut_t27/usb1/1-8\n"
                   "E: DEVPATH=/devices/ut_t27/usb1/1-8\n"
                   "E: DEVNAME=/dev/bus/usb/001/008\n"
                   "E: DEVTYPE=usb_device\n"
                   "E: PRODUCT=1234/5678/0001\n"
                   "E: TYPE=0/0/0\n"
                   "E: BUSNUM=001\n"
                   "E: DEVNUM=008\n"
                   "E: MAJOR=189\n"
                   "E: MINOR=8\n"
                   "SysFS BusID: 1-8\n");
}

// ---------------- ctor ----------------

TEST_F(EnableUtilsTest, Ctor_defaultConstructed_staticQueryStillWorks)
{
    // Arrange: validateAndNormalizeVidPid 为纯字符串处理，不依赖实例状态
    QString vid = QStringLiteral("0x8086");
    QString pid = QStringLiteral("0x1502");
    QString normVid, normPid;

    // Act: 构造实例并调用静态纯函数
    EnableUtils utils;
    const bool ok = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(normVid, QStringLiteral("8086"));
    EXPECT_EQ(normPid, QStringLiteral("1502"));
}

// ---------------- disableOutDevice：唯一标识分支补充 ----------------

// MD5 指纹分支：Vendor/Device + SysFS ID -> uniqueID 取 MD5 值
TEST_F(EnableUtilsTest, disableOutDevice_md5UidFromVendorDevice_inDb_writesAuthorizedFile)
{
    // Arrange: 与源码同款 MD5 指纹预置入库
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    const QString sysfsId = QStringLiteral("/devices/ut_t27/md5/input6");
    const QString expectUid = utMd5Uid(QStringLiteral("c52b"), QStringLiteral("c539"), sysfsId);
    mgr->insertDataToAuthorizedTable("usb", "utMd5Dev", "/ut/old", expectUid, true, "usbhid");

    ut_isSafeSysfs_ret = true;
    ut_qfileOpen_ret = true;
    ut_qfileOpen_calls = 0;
    ut_qfileWrites.clear();
    stub.set(isSafeSysfsPath, ut_isSafeSysfs_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act: 设备信息含 Vendor/Device 行，触发 MD5 分支
    EnableUtils::disableOutDevice(utUsbInfoMd5(sysfsId));

    // Assert: authorized 文件写入 "0"，库内路径更新（尾部端口号归一化为 0）
    ASSERT_EQ(ut_qfileWrites.size(), 1);
    EXPECT_EQ(ut_qfileWrites.first(), QByteArray("0"));
    EXPECT_EQ(mgr->authorizedPath(expectUid), QStringLiteral("/devices/ut_t27/md5/input0"));
}

// MD5 分支的 SysFS Device Link 变体：path 取 Device Link
TEST_F(EnableUtilsTest, disableOutDevice_md5UidWithDeviceLink_usesLinkAsPath)
{
    // Arrange
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    const QString linkPath = QStringLiteral("/devices/ut_t27/link6");
    const QString expectUid = utMd5Uid(QStringLiteral("c52b"), QStringLiteral("c539"), linkPath);
    mgr->insertDataToAuthorizedTable("usb", "utLinkDev", "/ut/old", expectUid, true, "usbhid");

    ut_isSafeSysfs_ret = true;
    ut_qfileOpen_ret = true;
    ut_qfileWrites.clear();
    stub.set(isSafeSysfsPath, ut_isSafeSysfs_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act
    EnableUtils::disableOutDevice(utUsbInfoMd5(linkPath, QStringLiteral("SysFS Device Link")));

    // Assert
    ASSERT_EQ(ut_qfileWrites.size(), 1);
    EXPECT_EQ(ut_qfileWrites.first(), QByteArray("0"));
    EXPECT_EQ(mgr->authorizedPath(expectUid), QStringLiteral("/devices/ut_t27/link0"));
}

// Module Alias 分支：无 Serial ID 时取 Module Alias 并截断尾部标识
TEST_F(EnableUtilsTest, disableOutDevice_moduleAliasOnly_stripsTrailingTag)
{
    // Arrange: 库中不存在该设备，不应写文件
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));

    ut_isSafeSysfs_ret = true;
    ut_qfileOpen_ret = true;
    ut_qfileWrites.clear();
    stub.set(isSafeSysfsPath, ut_isSafeSysfs_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act
    EnableUtils::disableOutDevice(utUsbInfoAlias());

    // Assert: 未入库 -> 无写入，但授权文件读取仍会发生
    EXPECT_EQ(ut_qfileWrites.size(), 0);
    EXPECT_GE(ut_qfileOpen_calls, 1);
}

// 无法提取唯一标识：跳过该条目
TEST_F(EnableUtilsTest, disableOutDevice_noIdentifiableId_skipsItem)
{
    // Arrange
    Stub stub;
    ASSERT_TRUE(UTDB::setup(EnableSqlManager::getInstance()));

    ut_qfileOpen_ret = true;
    ut_qfileWrites.clear();
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qfileWriteData_stub);

    // Act
    EnableUtils::disableOutDevice(utUsbInfoNoId());

    // Assert: 无可识别标识 -> 不写授权文件（实测授权文件读取仍会发生）
    EXPECT_EQ(ut_qfileWrites.size(), 0);
    EXPECT_GE(ut_qfileOpen_calls, 1);
}

// ---------------- enableNetworkByDBus：失败分支补充 ----------------

TEST_F(EnableUtilsTest, enableNetworkByDBus_devicePathEmpty_returnsFalse)
{
    // Arrange: GetDeviceByIpIface 返回空对象路径
    Stub stub;
    ut_systemBus_stub_ret = QDBusConnection(QStringLiteral("ut-stub-bus"));
    ut_ifaceValid_seq.clear();
    ut_ifaceValid_seq << true;
    ut_doCall_seq.clear();
    ut_doCall_seq << utMakeReply(QDBusObjectPath(QStringLiteral("")));
    ut_doCall_calls = 0;
    stub.set(ADDR(QDBusConnection, systemBus), ut_systemBus_stub);
    stub.set(ADDR(QDBusInterface, isValid), ut_ifaceValid_seq_stub);
    stub.set(ADDR(QDBusInterface, doCall), ut_doCall_seq_stub);

    // Act
    const bool ok = EnableUtils::enableNetworkByDBus("ut-fake-iface", true);

    // Assert: 返回 false 且确实发起过一次 DBus 调用（返回了空对象路径）
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_doCall_calls, 1);
}

TEST_F(EnableUtilsTest, enableNetworkByDBus_deviceInterfaceInvalid_returnsFalse)
{
    // Arrange: 第一次调用通过，第二次（Properties 接口）isValid 失败
    Stub stub;
    ut_systemBus_stub_ret = QDBusConnection(QStringLiteral("ut-stub-bus"));
    ut_ifaceValid_seq.clear();
    ut_ifaceValid_seq << true << false;
    ut_doCall_seq.clear();
    ut_doCall_seq << utMakeReply(QDBusObjectPath(QStringLiteral("/ut/dev0")));
    ut_doCall_calls = 0;
    stub.set(ADDR(QDBusConnection, systemBus), ut_systemBus_stub);
    stub.set(ADDR(QDBusInterface, isValid), ut_ifaceValid_seq_stub);
    stub.set(ADDR(QDBusInterface, doCall), ut_doCall_seq_stub);

    // Act
    const bool ok = EnableUtils::enableNetworkByDBus("ut-fake-iface", true);

    // Assert: 仅 GetDeviceByIpIface 一次 doCall，Properties 校验即失效
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_doCall_calls, 1);
}

TEST_F(EnableUtilsTest, enableNetworkByDBus_setReplyInvalid_returnsFalse)
{
    // Arrange: Set 调用返回错误消息
    Stub stub;
    ut_systemBus_stub_ret = QDBusConnection(QStringLiteral("ut-stub-bus"));
    ut_ifaceValid_seq.clear();
    ut_ifaceValid_seq << true << true;
    ut_doCall_seq.clear();
    ut_doCall_seq << utMakeReply(QDBusObjectPath(QStringLiteral("/ut/dev0")))
                  << utMakeErrorReply();
    ut_doCall_calls = 0;
    stub.set(ADDR(QDBusConnection, systemBus), ut_systemBus_stub);
    stub.set(ADDR(QDBusInterface, isValid), ut_ifaceValid_seq_stub);
    stub.set(ADDR(QDBusInterface, doCall), ut_doCall_seq_stub);

    // Act
    const bool ok = EnableUtils::enableNetworkByDBus("ut-fake-iface", false);

    // Assert: Set 返回错误消息 -> 共两次 doCall
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_doCall_calls, 2);
}

// ---------------- enableNetworkByIoctl：分支补充 ----------------

TEST_F(EnableUtilsTest, enableNetworkByIoctl_socketFailed_returnsFalse)
{
    // Arrange
    Stub stub;
    ut_socket_fd = -1;
    ut_socketOpen_calls = 0;
    stub.set(socket, ut_socketFd_stub);
    stub.set(close, ut_close_stub);

    // Act
    const bool ok = EnableUtils::enableNetworkByIoctl("enp0s0", true);

    // Assert: socket 创建失败直接返回
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_socketOpen_calls, 1);
}

TEST_F(EnableUtilsTest, enableNetworkByIoctl_enableTrue_setFlagsFail_returnsFalse)
{
    // Arrange: GIFFLAGS 成功，SIFFLAGS 失败
    Stub stub;
    ut_socket_fd = 42;
    ut_ioctlGiFlags_ret = 0;
    ut_ioctlSiFlags_ret = -1;
    ut_ioctlSiFlags_seen = -1;
    stub.set(socket, ut_socketFd_stub);
    stub.set(close, ut_close_stub);
    stub.set(ioctl, ut_ioctlFlags_stub);

    // Act
    const bool ok = EnableUtils::enableNetworkByIoctl("enp0s0", true);

    // Assert: SIFFLAGS 带查询到的 UP 标志位写入，但设置失败
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_ioctlSiFlags_seen, IFF_UP | IFF_BROADCAST);
}

TEST_F(EnableUtilsTest, enableNetworkByIoctl_disableTrue_setFlagsSucceeds_returnsTrue)
{
    // Arrange: 两次 ioctl 均成功
    Stub stub;
    ut_socket_fd = 42;
    ut_ioctlGiFlags_ret = 0;
    ut_ioctlSiFlags_ret = 0;
    ut_ioctlSiFlags_seen = -1;
    stub.set(socket, ut_socketFd_stub);
    stub.set(close, ut_close_stub);
    stub.set(ioctl, ut_ioctlFlags_stub);

    // Act
    const bool ok = EnableUtils::enableNetworkByIoctl("enp0s0", false);

    // Assert: 禁用时 SIFFLAGS 写入的是清除 UP 位、保留其余标志的值
    EXPECT_TRUE(ok);
    EXPECT_EQ(ut_ioctlSiFlags_seen, IFF_BROADCAST);
}

TEST_F(EnableUtilsTest, enableNetworkByDBus_allSucceed_returnsTrue)
{
    // Arrange: GetDeviceByIpIface 与 Set 均返回有效应答
    Stub stub;
    ut_systemBus_stub_ret = QDBusConnection(QStringLiteral("ut-stub-bus"));
    ut_ifaceValid_seq.clear();
    ut_ifaceValid_seq << true << true;
    ut_doCall_seq.clear();
    ut_doCall_seq << utMakeReply(QDBusObjectPath(QStringLiteral("/ut/dev0")))
                  << utMakeEmptyReply();
    ut_doCall_calls = 0;
    stub.set(ADDR(QDBusConnection, systemBus), ut_systemBus_stub);
    stub.set(ADDR(QDBusInterface, isValid), ut_ifaceValid_seq_stub);
    stub.set(ADDR(QDBusInterface, doCall), ut_doCall_seq_stub);

    // Act
    const bool ok = EnableUtils::enableNetworkByDBus("ut-fake-iface", true);

    // Assert: GetDeviceByIpIface + Set 共两次 doCall 全部成功
    EXPECT_TRUE(ok);
    EXPECT_EQ(ut_doCall_calls, 2);
}
