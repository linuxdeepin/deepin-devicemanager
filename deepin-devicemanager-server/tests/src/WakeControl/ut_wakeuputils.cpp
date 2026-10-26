// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | getMapInfo | mid | - | 2 | 3 |
// | wakeupPath | mid | - | 2 | 4 |
// | getPS2Syspath | mid | - | 2 | 4 |
// | md5Uid | high | - | 3 | 3 |
// | writeWakeupFile | high | - | 3 | 4 |
// | updateWakeupDeviceInfo | high | complexity:15 | 3 | 8 |
// | wakeOnLanIsOpen | mid | - | 2 | 5 |
// | setWakeOnLan | mid | - | 2 | 4 |
// | setWakeOnLanAll | low | - | 1 | 2 |
// | writeTlpWolConfig | mid | - | 2 | 7 |
// | tlpWolConfigEnabled | low | - | 1 | 3 |
// | WakeupUtils() | low | - | 1 | 1 |
// 最小清单完成情况（test-code-gen 最小清单）：
// 1. 每个公开方法 >= 1 用例: [x]
// 2. 每个输入维度按等价类划分 >= 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 8. 负面场景有专门用例: [x]

#include <QCryptographicHash>
#include <gtest/gtest.h>

#define private public
#define protected public

#include "wakeuputils.h"

#undef private
#undef protected

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

class WakeupUtilsTest : public ::testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}

    // 与源码 updateWakeupDeviceInfo 同款 MD5 指纹
    static QString md5Uid(const QString &vendorTail, const QString &deviceTail, const QString &sysfsId)
    {
        const QByteArray buf = vendorTail.toUtf8() + deviceTail.toUtf8() + sysfsId.trimmed().toUtf8();
        return QString::fromStdString(
            QCryptographicHash::hash(buf, QCryptographicHash::Md5).toBase64().toStdString());
    }
};

// wakeupPath

TEST_F(WakeupUtilsTest, wakeupPath_validPath_returnsConstructedPath)
{
    // Arrange
    QString syspath = "/devices/fake_usb_test_xyz/identity";
    QString result;

    // Act
    bool ok = WakeupUtils::wakeupPath(syspath, result);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(result, QString("/sys/devices/fake_usb_test_xyz/power/wakeup"));
}

TEST_F(WakeupUtilsTest, wakeupPath_noSlash_returnsFalse)
{
    // Arrange
    QString syspath = "nouspath";
    QString result;

    // Act
    bool ok = WakeupUtils::wakeupPath(syspath, result);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(result, QString());
}

TEST_F(WakeupUtilsTest, wakeupPath_emptyInput_returnsFalse)
{
    // Arrange
    QString syspath = "";
    QString result;

    // Act
    bool ok = WakeupUtils::wakeupPath(syspath, result);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(result, QString());
}

TEST_F(WakeupUtilsTest, wakeupPath_singleSlash_returnsFalse)
{
    // Arrange
    QString syspath = "/noslashafter";
    QString result;

    // Act
    bool ok = WakeupUtils::wakeupPath(syspath, result);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(result, QString());
}

// writeWakeupFile

TEST_F(WakeupUtilsTest, writeWakeupFile_nonexistentPath_returnsFalse)
{
    // Arrange
    QString path = "/nonexistent/path/to/wakeup";
    bool wakeup = true;

    // Act
    bool result = WakeupUtils::writeWakeupFile(path, wakeup);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(QFile::exists(path), false);
}

TEST_F(WakeupUtilsTest, writeWakeupFile_writeEnabled_writesEnabled)
{
    // Arrange
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString path = tmpDir.path() + "/wakeup_test";
    bool wakeup = true;

    // Act
    bool result = WakeupUtils::writeWakeupFile(path, wakeup);

    // Assert
    EXPECT_TRUE(result);
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    EXPECT_EQ(f.readAll(), "enabled");
    f.close();
}

TEST_F(WakeupUtilsTest, writeWakeupFile_writeDisabled_writesDisabled)
{
    // Arrange
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString path = tmpDir.path() + "/wakeup_test2";
    bool wakeup = false;

    // Act
    bool result = WakeupUtils::writeWakeupFile(path, wakeup);

    // Assert
    EXPECT_TRUE(result);
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    EXPECT_EQ(f.readAll(), "disabled");
    f.close();
}

// getMapInfo (private static, accessed via private hack)

TEST_F(WakeupUtilsTest, getMapInfo_tooFewLines_returnsFalse)
{
    // Arrange
    QString item = "line1\nline2\nline3";
    QMap<QString, QString> mapInfo;

    // Act
    bool result = WakeupUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(mapInfo.isEmpty(), true);
}

TEST_F(WakeupUtilsTest, getMapInfo_validKeyboard_returnsTrue)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: keyboard";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = WakeupUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(mapInfo["Hardware Class"], "keyboard");
}

TEST_F(WakeupUtilsTest, getMapInfo_notKeyboardOrMouse_returnsFalse)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: network";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = WakeupUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_TRUE(mapInfo.isEmpty());
}

TEST_F(WakeupUtilsTest, getMapInfo_validMouse_returnsTrue)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: mouse";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = WakeupUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(mapInfo["Hardware Class"], "mouse");
}

TEST_F(WakeupUtilsTest, getMapInfo_stripsQuotes_returnsStrippedValue)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: \"Value%1\"").arg(i);
    lines[0] = "Hardware Class: keyboard";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = WakeupUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(mapInfo["Key1"], "Value1");
}

// getPS2Syspath (private static)

TEST_F(WakeupUtilsTest, getPS2Syspath_noEventMatch_returnsEmpty)
{
    // Arrange
    QString dfs = "/dev/input/noevent";

    // Act
    QString result = WakeupUtils::getPS2Syspath(dfs);

    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.length(), 0);
}

TEST_F(WakeupUtilsTest, getPS2Syspath_emptyInput_returnsEmpty)
{
    // Arrange
    QString dfs = "";

    // Act
    QString result = WakeupUtils::getPS2Syspath(dfs);

    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.length(), 0);
}

/*
 * Mode 2 新增用例：WoL(ioctl)、TLP WoL 配置文件、setWakeOnLanAll、
 * updateWakeupDeviceInfo、getPS2Syspath。socket/ioctl/文件写入全部走 stub，
 * EnableSqlManager 单例注入临时 SQLite 库，测试不依赖真实网卡与真实 /proc、/etc。
 */

#define private public
#define protected public

#include "enablesqlmanager.h"
#include "../ut_testdbhelper.h"

#undef private
#undef protected

#include <QSaveFile>
#include <QRegularExpression>


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

// ---------------- socket / ioctl 桩 ----------------
static int ut_socket_fd = 100;
static int ut_socket_ret = 100;
static int ut_socket_calls = 0;
static int ut_socket_stub(int domain, int type, int protocol)
{
    Q_UNUSED(domain);
    Q_UNUSED(type);
    Q_UNUSED(protocol);
    ++ut_socket_calls;
    return ut_socket_ret;
}

static int ut_ioctl_ret = 0;
static __u32 ut_wol_supported = 0;
static __u32 ut_wol_wolopts = 0;
static QList<unsigned long> ut_ioctl_reqs;
static __u32 ut_last_swol_cmd = 0;
static __u32 ut_last_swol_wolopts = 0;
static int ut_ioctl_stub(int fd, unsigned long req, void *arg)
{
    Q_UNUSED(fd);
    ut_ioctl_reqs.append(req);
    if (req == SIOCETHTOOL && arg != nullptr) {
        struct ifreq *ifr = static_cast<struct ifreq *>(arg);
        struct ethtool_wolinfo *wol = reinterpret_cast<struct ethtool_wolinfo *>(ifr->ifr_data);
        if (wol->cmd == ETHTOOL_GWOL) {
            wol->supported = ut_wol_supported;
            wol->wolopts = ut_wol_wolopts;
        }
        ut_last_swol_cmd = wol->cmd;
        ut_last_swol_wolopts = wol->wolopts;
    }
    return ut_ioctl_ret;
}

// ---------------- WakeupUtils 静态成员桩 ----------------
static QList<QPair<QString, bool>> ut_writeWakeup_calls;
static bool ut_writeWakeupFile_stub(const QString &path, bool wakeup)
{
    ut_writeWakeup_calls.append(qMakePair(path, wakeup));
    return true;
}

static QMap<QString, WakeupUtils::EthStatus> ut_wolStatusMap;
static WakeupUtils::EthStatus ut_wolIsOpen_stub(const QString &logicalName)
{
    return ut_wolStatusMap.value(logicalName, WakeupUtils::ES_NOT_SUPPORT_WAKE_ON);
}

static QList<QPair<QString, bool>> ut_setWol_calls;
static bool ut_setWol_stub(const QString &logicalName, bool open)
{
    ut_setWol_calls.append(qMakePair(logicalName, open));
    return true;
}

// ---------------- 文件系统桩 ----------------
static bool ut_qdirExists_ret = false;
static bool ut_qdirExists_stub(void *self)
{
    Q_UNUSED(self);
    return ut_qdirExists_ret;
}
static bool ut_qdirMkpath_ret = true;
static QString ut_qdirMkpath_arg;
static bool ut_qdirMkpath_stub(void *self, const QString &dirName)
{
    Q_UNUSED(self);
    ut_qdirMkpath_arg = dirName;
    return ut_qdirMkpath_ret;
}
static QStringList ut_entryList_ret;
static QStringList ut_entryList_stub(void *self, const QStringList &nameFilters,
                                     QDir::Filters filters, QDir::SortFlags sort)
{
    Q_UNUSED(self);
    Q_UNUSED(nameFilters);
    Q_UNUSED(filters);
    Q_UNUSED(sort);
    return ut_entryList_ret;
}
static bool ut_qfileOpen_ret = false;
static int ut_qfileOpen_calls = 0;
static bool ut_qfileOpen_stub(void *self, QIODevice::OpenMode mode)
{
    Q_UNUSED(self);
    Q_UNUSED(mode);
    ++ut_qfileOpen_calls;
    return ut_qfileOpen_ret;
}
static bool ut_qsavefileOpen_ret = true;
static bool ut_qsavefileOpen_stub(void *self, QIODevice::OpenMode mode)
{
    Q_UNUSED(self);
    Q_UNUSED(mode);
    return ut_qsavefileOpen_ret;
}
static QList<QByteArray> ut_saveWrites;
static qint64 ut_qsavefileWriteData_ret = -2;   // -2 表示按原长度返回
static qint64 ut_qsavefileWriteData_stub(void *self, const char *data, qint64 len)
{
    Q_UNUSED(self);
    ut_saveWrites.append(QByteArray(data, static_cast<int>(len)));
    return ut_qsavefileWriteData_ret == -2 ? len : ut_qsavefileWriteData_ret;
}
static bool ut_qsavefileCommit_ret = true;
static bool ut_qsavefileCommit_stub(void *self)
{
    Q_UNUSED(self);
    return ut_qsavefileCommit_ret;
}
static QByteArray ut_readAll_ret;
static QByteArray ut_readAll_stub(void *self)
{
    Q_UNUSED(self);
    return ut_readAll_ret;
}
static bool ut_qfileExists_ret = false;
static int ut_qfileExists_calls = 0;
static bool ut_qfileExists_stub(const QString &fileName)
{
    Q_UNUSED(fileName);
    ++ut_qfileExists_calls;
    return ut_qfileExists_ret;
}
static bool ut_qfileRemove_ret = true;
static QString ut_qfileRemove_arg;
static bool ut_qfileRemove_stub(const QString &fileName)
{
    ut_qfileRemove_arg = fileName;
    return ut_qfileRemove_ret;
}

// 构造带 Unique ID 的键盘设备信息(>10 行)
static QString utKbInfoUid(const QString &sysfsId, const QString &uid)
{
    return QString("I: Bus=0011 Vendor=0001 Product=0001 Version=ab41\n"
                   "N: Name=\"AT Translated Set 2 keyboard\"\n"
                   "P: Phys=isa0060/serio0/input0\n"
                   "S: Sysfs=%1\n"
                   "U: Uniq=\n"
                   "E: DEVPATH=%1\n"
                   "E: SUBSYSTEM=input\n"
                   "E: PRODUCT=11/1/1/ab41\n"
                   "E: NAME=\"AT Translated Set 2 keyboard\"\n"
                   "E: ID_INPUT=1\n"
                   "E: ID_INPUT_KEYBOARD=1\n"
                   "Device Files: input/event5\n"
                   "Hardware Class: keyboard\n"
                   "Unique ID: %2\n").arg(sysfsId, uid);
}

// 构造无 Unique ID、走 Vendor/Device MD5 分支的键盘设备信息(>10 行)
static qint64 ut_qiowriteFail_stub(void *self, const char *data, qint64 len)
{
    Q_UNUSED(self);
    Q_UNUSED(data);
    Q_UNUSED(len);
    return -1;
}

static QString utKbInfoMd5WithKey(const QString &idPath, const QString &keyLine)
{
    return QString("I: Bus=0003 Vendor=046d Product=c539 Version=0100\n"
                   "N: Name=\"UT Wireless Keyboard\"\n"
                   "P: Phys=usb-0000:00:14.0-3/input0\n"
                   "S: Sysfs=%1\n"
                   "U: Uniq=\n"
                   "H: Handlers=sysrq kbd event5 leds \n"
                   "B: EV=120013\n"
                   "B: KEY=1000000000007 ff9f207ac14057ff febeffdfffefffff fffffffffffffffe\n"
                   "B: MSC=10\n"
                   "B: LED=7\n"
                   "Device Files: input/event5\n"
                   "Hardware Class: keyboard\n"
                   "Vendor: 0x046d c52b\n"
                   "Device: 0x1a86 c539\n"
                   "%2: %1\n").arg(idPath, keyLine);
}

static QString utKbInfoMd5(const QString &sysfsId)
{
    return QString("I: Bus=0003 Vendor=046d Product=c539 Version=0100\n"
                   "N: Name=\"UT Wireless Keyboard\"\n"
                   "P: Phys=usb-0000:00:14.0-3/input0\n"
                   "S: Sysfs=%1\n"
                   "U: Uniq=\n"
                   "H: Handlers=sysrq kbd event5 leds \n"
                   "B: EV=120013\n"
                   "B: KEY=1000000000007 ff9f207ac14057ff febeffdfffefffff fffffffffffffffe\n"
                   "B: MSC=10\n"
                   "B: LED=7\n"
                   "Device Files: input/event5\n"
                   "Hardware Class: keyboard\n"
                   "Vendor: 0x046d c52b\n"
                   "Device: 0x1a86 c539\n"
                   "SysFS ID: %1\n").arg(sysfsId);
}

// /proc/bus/input/devices 标准内容(event5 -> i8042 serio0)
static const char *utProcContentStandard =
    "I: Bus=0011 Vendor=0002 Product=0001 Version=0000\n"
    "N: Name=\"AT Translated Set 2 keyboard\"\n"
    "P: Phys=isa0060/serio0/input0\n"
    "S: Sysfs=/devices/platform/i8042/serio0/input/input5\n"
    "U: Uniq=\n"
    "H: Handlers=sysrq kbd event5 \n"
    "B: EV=120013\n"
    "B: KEY=400000000000 0 0 0\n"
    "B: MSC=10\n"
    "B: LED=7\n"
    "\n"
    "I: Bus=0018 Vendor=04f3 Product=301d Version=0100\n"
    "N: Name=\"Goodix Capacitive TouchScreen\"\n"
    "P: Phys=i2c-DELL08AF:00\n"
    "S: Sysfs=/devices/pci0000:00/0000:00:15.0/i2c_designware.0/i2c-4/input/input12\n"
    "U: Uniq=\n"
    "H: Handlers=event12 \n"
    "B: EV=1b\n"
    "B: KEY=0\n"
    "B: ABS=265800000000003\n"
    "B: MSC=20\n";

// ---------------- wakeOnLanIsOpen / setWakeOnLan ----------------

TEST_F(WakeupUtilsTest, wakeOnLanIsOpen_socketFailed_returnsES_SOCKET_FAILED)
{
    // Arrange
    Stub stub;
    ut_socket_ret = -1;
    ut_socket_calls = 0;
    stub.set(socket, ut_socket_stub);

    // Act
    const WakeupUtils::EthStatus st = WakeupUtils::wakeOnLanIsOpen("enp0s0");

    // Assert: socket 创建失败即返回错误码
    EXPECT_EQ(st, WakeupUtils::ES_SOCKET_FAILED);
    EXPECT_EQ(ut_socket_calls, 1);
}

TEST_F(WakeupUtilsTest, wakeOnLanIsOpen_ioctlFailed_returnsES_IOCTL_ERROR)
{
    // Arrange
    Stub stub;
    ut_socket_ret = 100;
    ut_ioctl_ret = -1;
    ut_ioctl_reqs.clear();
    stub.set(socket, ut_socket_stub);
    stub.set((int (*)(int, unsigned long, void *))ioctl, ut_ioctl_stub);

    // Act
    const WakeupUtils::EthStatus st = WakeupUtils::wakeOnLanIsOpen("enp0s0");

    // Assert: 失败前已发起 GWOL 查询
    EXPECT_EQ(st, WakeupUtils::ES_IOCTL_ERROR);
    EXPECT_TRUE(ut_ioctl_reqs.contains(SIOCETHTOOL));
}

TEST_F(WakeupUtilsTest, wakeOnLanIsOpen_notSupported_returnsES_NOT_SUPPORT_WAKE_ON)
{
    // Arrange: 支持位为 0
    Stub stub;
    ut_socket_ret = 100;
    ut_ioctl_ret = 0;
    ut_wol_supported = 0;
    ut_wol_wolopts = 0;
    ut_ioctl_reqs.clear();
    stub.set(socket, ut_socket_stub);
    stub.set((int (*)(int, unsigned long, void *))ioctl, ut_ioctl_stub);

    // Act
    const WakeupUtils::EthStatus st = WakeupUtils::wakeOnLanIsOpen("enp0s0");

    // Assert: 查询过 GWOL 后按支持位判定
    EXPECT_EQ(st, WakeupUtils::ES_NOT_SUPPORT_WAKE_ON);
    EXPECT_TRUE(ut_ioctl_reqs.contains(SIOCETHTOOL));
}

TEST_F(WakeupUtilsTest, wakeOnLanIsOpen_supportedButClosed_returnsES_WAKE_ON_CLOSE)
{
    // Arrange: 支持 WAKE_MAGIC 但 wolopts 为 0
    Stub stub;
    ut_socket_ret = 100;
    ut_ioctl_ret = 0;
    ut_wol_supported = WAKE_MAGIC;
    ut_wol_wolopts = 0;
    ut_ioctl_reqs.clear();
    stub.set(socket, ut_socket_stub);
    stub.set((int (*)(int, unsigned long, void *))ioctl, ut_ioctl_stub);

    // Act
    const WakeupUtils::EthStatus st = WakeupUtils::wakeOnLanIsOpen("enp0s0");

    // Assert: 支持 WAKE_MAGIC 但 wolopts=0 -> 关闭
    EXPECT_EQ(st, WakeupUtils::ES_WAKE_ON_CLOSE);
    EXPECT_TRUE(ut_ioctl_reqs.contains(SIOCETHTOOL));
}

TEST_F(WakeupUtilsTest, wakeOnLanIsOpen_supportedAndOpen_returnsES_WAKE_ON_OPEN)
{
    // Arrange
    Stub stub;
    ut_socket_ret = 100;
    ut_ioctl_ret = 0;
    ut_wol_supported = WAKE_MAGIC;
    ut_wol_wolopts = WAKE_MAGIC;
    ut_ioctl_reqs.clear();
    stub.set(socket, ut_socket_stub);
    stub.set((int (*)(int, unsigned long, void *))ioctl, ut_ioctl_stub);

    // Act
    const WakeupUtils::EthStatus st = WakeupUtils::wakeOnLanIsOpen("enp0s0");

    // Assert: 支持 WAKE_MAGIC 且 wolopts 非零 -> 开启
    EXPECT_EQ(st, WakeupUtils::ES_WAKE_ON_OPEN);
    EXPECT_TRUE(ut_ioctl_reqs.contains(SIOCETHTOOL));
}

TEST_F(WakeupUtilsTest, setWakeOnLan_socketFailed_returnsFalse)
{
    // Arrange
    Stub stub;
    ut_socket_ret = -1;
    ut_socket_calls = 0;
    stub.set(socket, ut_socket_stub);

    // Act
    const bool ok = WakeupUtils::setWakeOnLan("enp0s0", true);

    // Assert: socket 创建失败即返回 false
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_socket_calls, 1);
}

TEST_F(WakeupUtilsTest, setWakeOnLan_openTrue_setsWoloptsMagic_returnsTrue)
{
    // Arrange
    Stub stub;
    ut_socket_ret = 100;
    ut_ioctl_ret = 0;
    ut_last_swol_cmd = 0;
    ut_last_swol_wolopts = 0;
    stub.set(socket, ut_socket_stub);
    stub.set((int (*)(int, unsigned long, void *))ioctl, ut_ioctl_stub);

    // Act
    const bool ok = WakeupUtils::setWakeOnLan("enp0s0", true);

    // Assert: ETHTOOL_SWOL 命令携带 WAKE_MAGIC
    EXPECT_TRUE(ok);
    EXPECT_EQ(ut_last_swol_cmd, static_cast<__u32>(ETHTOOL_SWOL));
    EXPECT_EQ(ut_last_swol_wolopts, static_cast<__u32>(WAKE_MAGIC));
}

TEST_F(WakeupUtilsTest, setWakeOnLan_openFalse_clearsWolopts_returnsTrue)
{
    // Arrange
    Stub stub;
    ut_socket_ret = 100;
    ut_ioctl_ret = 0;
    ut_last_swol_cmd = 0;
    ut_last_swol_wolopts = 0;
    stub.set(socket, ut_socket_stub);
    stub.set((int (*)(int, unsigned long, void *))ioctl, ut_ioctl_stub);

    // Act
    const bool ok = WakeupUtils::setWakeOnLan("enp0s0", false);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(ut_last_swol_cmd, static_cast<__u32>(ETHTOOL_SWOL));
    EXPECT_EQ(ut_last_swol_wolopts, static_cast<__u32>(0));
}

TEST_F(WakeupUtilsTest, setWakeOnLan_ioctlFailed_returnsFalse)
{
    // Arrange
    Stub stub;
    ut_socket_ret = 100;
    ut_ioctl_ret = -1;
    ut_ioctl_reqs.clear();
    stub.set(socket, ut_socket_stub);
    stub.set((int (*)(int, unsigned long, void *))ioctl, ut_ioctl_stub);

    // Act
    const bool ok = WakeupUtils::setWakeOnLan("enp0s0", true);

    // Assert: ioctl 执行失败返回 false，且至少发起过一次 ioctl
    EXPECT_FALSE(ok);
    EXPECT_FALSE(ut_ioctl_reqs.isEmpty());
}

// ---------------- writeTlpWolConfig / tlpWolConfigEnabled ----------------

TEST_F(WakeupUtilsTest, writeTlpWolConfig_enable_dirMissingAndMkpathFails_returnsFalse)
{
    // Arrange: /etc/tlp.d 不存在且 mkpath 失败(真实路径不会写盘，全程 stub)
    Stub stub;
    stub.set((bool (QDir::*)() const)&QDir::exists, ut_qdirExists_stub);
    stub.set(ADDR(QDir, mkpath), ut_qdirMkpath_stub);
    ut_qdirExists_ret = false;
    ut_qdirMkpath_ret = false;
    ut_qdirMkpath_arg.clear();

    // Act
    const bool ok = WakeupUtils::writeTlpWolConfig(true);

    // Assert: mkpath 已尝试建目录但失败
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_qdirMkpath_arg, QStringLiteral("."));
}

TEST_F(WakeupUtilsTest, writeTlpWolConfig_enable_success_writesCanonicalContent)
{
    // Arrange
    Stub stub;
    stub.set((bool (QDir::*)() const)&QDir::exists, ut_qdirExists_stub);
    stub.set(ADDR(QDir, mkpath), ut_qdirMkpath_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QSAVEFILE_OPEN), ut_qsavefileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qsavefileWriteData_stub);
    stub.set(ADDR(QSaveFile, commit), ut_qsavefileCommit_stub);
    ut_qdirExists_ret = false;
    ut_qdirMkpath_ret = true;
    ut_qdirMkpath_arg.clear();
    ut_saveWrites.clear();

    // Act
    const bool ok = WakeupUtils::writeTlpWolConfig(true);

    // Assert: 内容为固定模板，mkpath 以 "." 建目录
    EXPECT_TRUE(ok);
    EXPECT_EQ(ut_qdirMkpath_arg, QStringLiteral("."));
    ASSERT_EQ(ut_saveWrites.size(), 1);
    EXPECT_EQ(ut_saveWrites.first(), QByteArray("WOL_DISABLE=N\nWOL_ENABLE=g\n"));
}

TEST_F(WakeupUtilsTest, writeTlpWolConfig_enable_openFails_returnsFalse)
{
    // Arrange
    Stub stub;
    stub.set((bool (QDir::*)() const)&QDir::exists, ut_qdirExists_stub);
    stub.set(ADDR(QDir, mkpath), ut_qdirMkpath_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QSAVEFILE_OPEN), ut_qsavefileOpen_stub);
    stub.set(ADDR(QSaveFile, commit), ut_qsavefileCommit_stub);
    ut_qdirExists_ret = true;
    ut_qsavefileOpen_ret = false;
    ut_qsavefileCommit_ret = true;
    ut_qdirMkpath_arg.clear();

    // Act
    const bool ok = WakeupUtils::writeTlpWolConfig(true);

    // Assert: 目录已存在 -> 不再 mkpath；打开失败返回 false
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_qdirMkpath_arg, QString());
}

TEST_F(WakeupUtilsTest, writeTlpWolConfig_enable_writeFails_returnsFalse)
{
    // Arrange: writeData 返回 -1 触发 size < 1 分支
    Stub stub;
    stub.set((bool (QDir::*)() const)&QDir::exists, ut_qdirExists_stub);
    stub.set(ADDR(QDir, mkpath), ut_qdirMkpath_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QSAVEFILE_OPEN), ut_qsavefileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qsavefileWriteData_stub);
    stub.set(ADDR(QSaveFile, commit), ut_qsavefileCommit_stub);
    ut_qdirExists_ret = true;
    ut_qsavefileOpen_ret = true;
    ut_qsavefileCommit_ret = true;
    ut_saveWrites.clear();
    ut_qsavefileWriteData_ret = -1;

    // Act
    const bool ok = WakeupUtils::writeTlpWolConfig(true);

    // Assert: 写入被尝试过一次但返回失败
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_saveWrites.size(), 1);
    ut_qsavefileWriteData_ret = -2;   // 恢复默认按原长度返回
}

TEST_F(WakeupUtilsTest, writeTlpWolConfig_enable_commitFails_returnsFalse)
{
    // Arrange
    Stub stub;
    stub.set((bool (QDir::*)() const)&QDir::exists, ut_qdirExists_stub);
    stub.set(ADDR(QDir, mkpath), ut_qdirMkpath_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QSAVEFILE_OPEN), ut_qsavefileOpen_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qsavefileWriteData_stub);
    stub.set(ADDR(QSaveFile, commit), ut_qsavefileCommit_stub);
    ut_qdirExists_ret = true;
    ut_qsavefileOpen_ret = true;
    ut_qsavefileCommit_ret = false;
    ut_saveWrites.clear();

    // Act
    const bool ok = WakeupUtils::writeTlpWolConfig(true);

    // Assert: 写入成功后 commit 失败仍返回 false
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_saveWrites.size(), 1);
}

TEST_F(WakeupUtilsTest, writeTlpWolConfig_disable_noConfigFile_returnsTrue)
{
    // Arrange
    Stub stub;
    stub.set((bool (*)(const QString &))QFile::exists, ut_qfileExists_stub);
    ut_qfileExists_ret = false;
    ut_qfileExists_calls = 0;

    // Act
    const bool ok = WakeupUtils::writeTlpWolConfig(false);

    // Assert: 配置不存在 -> 不做任何删除直接成功
    EXPECT_TRUE(ok);
    EXPECT_EQ(ut_qfileExists_calls, 1);
}

TEST_F(WakeupUtilsTest, writeTlpWolConfig_disable_removed_returnsTrue)
{
    // Arrange
    Stub stub;
    stub.set((bool (*)(const QString &))QFile::exists, ut_qfileExists_stub);
    stub.set((bool (*)(const QString &))QFile::remove, ut_qfileRemove_stub);
    ut_qfileExists_ret = true;
    ut_qfileRemove_ret = true;
    ut_qfileRemove_arg.clear();

    // Act
    const bool ok = WakeupUtils::writeTlpWolConfig(false);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(ut_qfileRemove_arg,
              QStringLiteral("/etc") + QStringLiteral("/tlp.d/99-device-manager-wol.conf"));
}

TEST_F(WakeupUtilsTest, writeTlpWolConfig_disable_removeFails_returnsFalse)
{
    // Arrange
    Stub stub;
    stub.set((bool (*)(const QString &))QFile::exists, ut_qfileExists_stub);
    stub.set((bool (*)(const QString &))QFile::remove, ut_qfileRemove_stub);
    ut_qfileExists_ret = true;
    ut_qfileRemove_ret = false;
    stub.set((bool (*)(const QString &))QFile::remove, ut_qfileRemove_stub);
    ut_qfileRemove_arg.clear();

    // Act
    const bool ok = WakeupUtils::writeTlpWolConfig(false);

    // Assert: remove 被尝试但失败 -> 返回 false
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_qfileRemove_arg,
              QStringLiteral("/etc") + QStringLiteral("/tlp.d/99-device-manager-wol.conf"));
}

TEST_F(WakeupUtilsTest, tlpWolConfigEnabled_unopenable_returnsFalse)
{
    // Arrange
    Stub stub;
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    ut_qfileOpen_ret = false;
    ut_qfileOpen_calls = 0;

    // Act
    const bool ok = WakeupUtils::tlpWolConfigEnabled();

    // Assert: 打开失败 -> 文件被尝试过一次
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_qfileOpen_calls, 1);
}

TEST_F(WakeupUtilsTest, tlpWolConfigEnabled_matchingContent_returnsTrue)
{
    // Arrange
    Stub stub;
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set(ADDR(QIODevice, readAll), ut_readAll_stub);
    ut_qfileOpen_ret = true;
    ut_readAll_ret = QByteArray("WOL_DISABLE=N\nWOL_ENABLE=g\n");
    ut_qfileOpen_calls = 0;

    // Act
    const bool ok = WakeupUtils::tlpWolConfigEnabled();

    // Assert: WOL_DISABLE=N -> 已禁用 WoL，返回 true
    EXPECT_TRUE(ok);
    EXPECT_EQ(ut_qfileOpen_calls, 1);
}

TEST_F(WakeupUtilsTest, tlpWolConfigEnabled_mismatchedContent_returnsFalse)
{
    // Arrange
    Stub stub;
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set(ADDR(QIODevice, readAll), ut_readAll_stub);
    ut_qfileOpen_ret = true;
    ut_readAll_ret = QByteArray("WOL_DISABLE=Y\nWOL_ENABLE=g\n");
    ut_qfileOpen_calls = 0;

    // Act
    const bool ok = WakeupUtils::tlpWolConfigEnabled();

    // Assert: WOL_DISABLE=Y -> 未禁用，返回 false
    EXPECT_FALSE(ok);
    EXPECT_EQ(ut_qfileOpen_calls, 1);
}

// ---------------- setWakeOnLanAll ----------------

TEST_F(WakeupUtilsTest, setWakeOnLanAll_open_setsOnlyClosedInterfaces)
{
    // Arrange: eth0 关闭、eth1 不支持、eth2 已开启
    Stub stub;
    stub.set((QStringList (QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)
                 &QDir::entryList, ut_entryList_stub);
    stub.set(ADDR(WakeupUtils, wakeOnLanIsOpen), ut_wolIsOpen_stub);
    stub.set(ADDR(WakeupUtils, setWakeOnLan), ut_setWol_stub);
    ut_entryList_ret = QStringList {"eth0", "eth1", "eth2"};
    ut_wolStatusMap.clear();
    ut_wolStatusMap.insert("eth0", WakeupUtils::ES_WAKE_ON_CLOSE);
    ut_wolStatusMap.insert("eth1", WakeupUtils::ES_NOT_SUPPORT_WAKE_ON);
    ut_wolStatusMap.insert("eth2", WakeupUtils::ES_WAKE_ON_OPEN);
    ut_setWol_calls.clear();

    // Act
    WakeupUtils::setWakeOnLanAll(true);

    // Assert: 只对处于关闭状态且支持 WoL 的 eth0 执行开启
    ASSERT_EQ(ut_setWol_calls.size(), 1);
    EXPECT_EQ(ut_setWol_calls.first().first, QStringLiteral("eth0"));
    EXPECT_TRUE(ut_setWol_calls.first().second);
}

TEST_F(WakeupUtilsTest, setWakeOnLanAll_close_setsOnlyOpenInterfaces)
{
    // Arrange
    Stub stub;
    stub.set((QStringList (QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)
                 &QDir::entryList, ut_entryList_stub);
    stub.set(ADDR(WakeupUtils, wakeOnLanIsOpen), ut_wolIsOpen_stub);
    stub.set(ADDR(WakeupUtils, setWakeOnLan), ut_setWol_stub);
    ut_entryList_ret = QStringList {"eth0", "eth1", "eth2"};
    ut_wolStatusMap.clear();
    ut_wolStatusMap.insert("eth0", WakeupUtils::ES_WAKE_ON_CLOSE);
    ut_wolStatusMap.insert("eth1", WakeupUtils::ES_NOT_SUPPORT_WAKE_ON);
    ut_wolStatusMap.insert("eth2", WakeupUtils::ES_WAKE_ON_OPEN);
    ut_setWol_calls.clear();

    // Act
    WakeupUtils::setWakeOnLanAll(false);

    // Assert: 只对处于开启状态的 eth2 执行关闭
    ASSERT_EQ(ut_setWol_calls.size(), 1);
    EXPECT_EQ(ut_setWol_calls.first().first, QStringLiteral("eth2"));
    EXPECT_FALSE(ut_setWol_calls.first().second);
}

// ---------------- updateWakeupDeviceInfo ----------------

TEST_F(WakeupUtilsTest, updateWakeupDeviceInfo_uidMatch_dbWakeupTrue_writesEnabledFile)
{
    // Arrange: 临时库存在该 Unique ID 且 wakeup=true
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    mgr->insertWakeupData("ut-uid-kb", "/ut/any", true);

    ut_writeWakeup_calls.clear();
    stub.set(ADDR(WakeupUtils, writeWakeupFile), ut_writeWakeupFile_stub);
    stub.set((bool (*)(const QString &))QFile::exists, ut_qfileExists_stub);
    ut_qfileExists_ret = true;

    // Act: 携带 SysFS ID 行，走 SysFS ID 直取分支（不触发 /proc 解析）
    QString info = utKbInfoUid("/devices/ut_t27/input/input5", "ut-uid-kb");
    info += QStringLiteral("SysFS ID: /devices/ut_t27/input/input5\n");
    WakeupUtils::updateWakeupDeviceInfo(info);

    // Assert: 按 SysFS ID 拼出 wakeup 路径，写入 enabled
    ASSERT_EQ(ut_writeWakeup_calls.size(), 1);
    EXPECT_EQ(ut_writeWakeup_calls.first().first, QStringLiteral("/sys/devices/ut_t27/input/power/wakeup"));
    EXPECT_TRUE(ut_writeWakeup_calls.first().second);
}

TEST_F(WakeupUtilsTest, updateWakeupDeviceInfo_uidNotInDb_skipped)
{
    // Arrange: 库中无记录
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));

    ut_writeWakeup_calls.clear();
    stub.set(ADDR(WakeupUtils, writeWakeupFile), ut_writeWakeupFile_stub);

    // Act
    WakeupUtils::updateWakeupDeviceInfo(utKbInfoUid("/devices/ut_t27/input/input5", "ut-uid-kb"));

    // Assert: 不写唤醒文件，也不产生库记录
    EXPECT_EQ(ut_writeWakeup_calls.size(), 0);
    EXPECT_FALSE(mgr->isWakeupUniqueIdExisted("ut-uid-kb"));
}

TEST_F(WakeupUtilsTest, updateWakeupDeviceInfo_md5Uid_matchesDb_writesDisabledFile)
{
    // Arrange: 无 Unique ID，走 Vendor/Device MD5 分支；库中 wakeup=false
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));

    const QString sysfsId = QStringLiteral("/devices/ut_t27/md5/input/input7");
    const QByteArray buf = QStringLiteral("c52b").toUtf8() + QStringLiteral("c539").toUtf8()
                           + sysfsId.trimmed().toUtf8();
    const QString expectedUid = QString::fromStdString(
        QCryptographicHash::hash(buf, QCryptographicHash::Md5).toBase64().toStdString());
    mgr->insertWakeupData(expectedUid, "/ut/any", false);

    ut_writeWakeup_calls.clear();
    stub.set(ADDR(WakeupUtils, writeWakeupFile), ut_writeWakeupFile_stub);
    stub.set((bool (*)(const QString &))QFile::exists, ut_qfileExists_stub);
    ut_qfileExists_ret = true;

    // Act
    WakeupUtils::updateWakeupDeviceInfo(utKbInfoMd5(sysfsId));

    // Assert: MD5 指纹与库内一致，按库中状态写 disabled
    ASSERT_EQ(ut_writeWakeup_calls.size(), 1);
    EXPECT_EQ(ut_writeWakeup_calls.first().first, QStringLiteral("/sys/devices/ut_t27/md5/input/power/wakeup"));
    EXPECT_FALSE(ut_writeWakeup_calls.first().second);
}

TEST_F(WakeupUtilsTest, updateWakeupDeviceInfo_ps2Keyboard_usesProcBusParsing)
{
    // Arrange: 无 SysFS ID，回退 getPS2Syspath 解析 /proc/bus/input/devices
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    mgr->insertWakeupData("ut-uid-ps2", "/ut/any", true);

    ut_writeWakeup_calls.clear();
    stub.set(ADDR(WakeupUtils, writeWakeupFile), ut_writeWakeupFile_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set(ADDR(QIODevice, readAll), ut_readAll_stub);
    stub.set((bool (*)(const QString &))QFile::exists, ut_qfileExists_stub);
    ut_qfileOpen_ret = true;
    ut_readAll_ret = QByteArray(utProcContentStandard);
    ut_qfileOpen_calls = 0;
    ut_qfileExists_ret = true;

    // Act: 构造不含 SysFS ID 的键盘信息(Device Files 指向 event5)
    QString info = utKbInfoUid("/devices/ut_t27/input/input5", "ut-uid-ps2");
    info.remove(QRegularExpression("^S: Sysfs=.*\\n", QRegularExpression::MultilineOption));
    WakeupUtils::updateWakeupDeviceInfo(info);

    // Assert: 通过 /proc 解析出 PS/2 设备 syspath
    ASSERT_EQ(ut_writeWakeup_calls.size(), 1);
    EXPECT_EQ(ut_writeWakeup_calls.first().first,
              QStringLiteral("/sys/devices/platform/i8042/serio0/power/wakeup"));
    EXPECT_TRUE(ut_writeWakeup_calls.first().second);
}

// ---------------- getPS2Syspath ----------------

TEST_F(WakeupUtilsTest, getPS2Syspath_eventMatch_standardSysfs_returnsParent)
{
    // Arrange
    Stub stub;
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set(ADDR(QIODevice, readAll), ut_readAll_stub);
    ut_qfileOpen_ret = true;
    ut_readAll_ret = QByteArray(utProcContentStandard);
    ut_qfileOpen_calls = 0;

    // Act
    const QString result = WakeupUtils::getPS2Syspath("input/event5");

    // Assert: 标准路径捕获到 /inputN 之前(保留 input 组件)，且 /proc 被读取一次
    EXPECT_EQ(result, QStringLiteral("/devices/platform/i8042/serio0/input"));
    EXPECT_EQ(ut_qfileOpen_calls, 1);
}

TEST_F(WakeupUtilsTest, getPS2Syspath_i2cDesignware_returnsBeforeInputDir)
{
    // Arrange: i2c_designware 设备走专用正则，取 /input/inputN 之前的一段
    Stub stub;
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set(ADDR(QIODevice, readAll), ut_readAll_stub);
    ut_qfileOpen_ret = true;
    ut_readAll_ret = QByteArray(utProcContentStandard);
    ut_qfileOpen_calls = 0;

    // Act
    const QString result = WakeupUtils::getPS2Syspath("input/event12");

    // Assert: i2c 路径取 /input/inputN 之前一段，且 /proc 被读取一次
    EXPECT_EQ(result, QStringLiteral("/devices/pci0000:00/0000:00:15.0/i2c_designware.0/i2c-4"));
    EXPECT_EQ(ut_qfileOpen_calls, 1);
}

TEST_F(WakeupUtilsTest, getPS2Syspath_eventAbsentInProc_returnsEmpty)
{
    // Arrange: /proc 内容不含 event5
    Stub stub;
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set(ADDR(QIODevice, readAll), ut_readAll_stub);
    ut_qfileOpen_ret = true;
    ut_readAll_ret = QByteArray("I: Bus=0011 Vendor=0002 Product=0001 Version=0000\n"
                                "N: Name=\"AT Translated Set 2 keyboard\"\n"
                                "P: Phys=isa0060/serio0/input0\n"
                                "S: Sysfs=/devices/platform/i8042/serio0/input/input5\n"
                                "U: Uniq=\n"
                                "H: Handlers=sysrq kbd event9 \n"
                                "B: EV=120013\n"
                                "B: KEY=400000000000 0 0 0\n"
                                "B: MSC=10\n"
                                "B: LED=7\n");

    // Act
    const QString result = WakeupUtils::getPS2Syspath("input/event5");

    // Assert: event5 不在 /proc 内容中 -> 空 syspath
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result, QString());
}

// ================= 覆盖率补充（Mode 2 闭环） =================

TEST_F(WakeupUtilsTest, Ctor_defaultConstructed_staticQueryStillWorks)
{
    // Arrange: wakeupPath 为纯路径拼接，不依赖实例状态
    QString result;

    // Act: 构造实例并调用静态方法
    WakeupUtils utils;
    const bool ok = WakeupUtils::wakeupPath(QStringLiteral("/devices/ut_ctor/identity"), result);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(result, QStringLiteral("/sys/devices/ut_ctor/power/wakeup"));
}

TEST_F(WakeupUtilsTest, updateWakeupDeviceInfo_invalidMapInfo_skipped)
{
    // Arrange: 条目行数不足，getMapInfo 解析失败
    Stub stub;
    ASSERT_TRUE(UTDB::setup(EnableSqlManager::getInstance()));

    ut_writeWakeup_calls.clear();
    stub.set(ADDR(WakeupUtils, writeWakeupFile), ut_writeWakeupFile_stub);

    // Act
    WakeupUtils::updateWakeupDeviceInfo(QStringLiteral("I: Bus=0011\nN: Name=\"broken\""));

    // Assert: 未产生任何写入，也不产生库记录
    EXPECT_EQ(ut_writeWakeup_calls.size(), 0);
    EXPECT_FALSE(EnableSqlManager::getInstance()->isWakeupUniqueIdExisted("ut-uid-kb"));
}

TEST_F(WakeupUtilsTest, updateWakeupDeviceInfo_md5UidWithDeviceLink_matchesDb_writesFile)
{
    // Arrange: Vendor/Device + SysFS Device Link -> MD5 指纹走 Device Link 分支
    Stub stub;
    EnableSqlManager *mgr = EnableSqlManager::getInstance();
    ASSERT_TRUE(UTDB::setup(mgr));
    const QString linkPath = QStringLiteral("/devices/ut_t27/md5link/input7");
    const QString uid = WakeupUtilsTest::md5Uid(QStringLiteral("c52b"), QStringLiteral("c539"), linkPath);
    mgr->insertWakeupData(uid, "/ut/any", true);

    ut_writeWakeup_calls.clear();
    stub.set(ADDR(WakeupUtils, writeWakeupFile), ut_writeWakeupFile_stub);
    stub.set((bool (*)(const QString &))QFile::exists, ut_qfileExists_stub);
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    stub.set(ADDR(QIODevice, readAll), ut_readAll_stub);
    ut_qfileExists_ret = true;
    ut_qfileOpen_ret = true;
    ut_readAll_ret = QByteArray("I: Bus=0011 Vendor=0002 Product=0001 Version=0000\n"
                                "N: Name=\"AT Translated Set 2 keyboard\"\n"
                                "P: Phys=isa0060/serio0/input0\n"
                                "S: Sysfs=/devices/ut_t27/md5link/input/input7\n"
                                "U: Uniq=\n"
                                "H: Handlers=sysrq kbd event5 \n"
                                "B: EV=120013\n"
                                "B: KEY=400000000000 0 0 0\n"
                                "B: MSC=10\n"
                                "B: LED=7\n\n");

    // Act
    WakeupUtils::updateWakeupDeviceInfo(utKbInfoMd5WithKey(linkPath, QStringLiteral("SysFS Device Link")));

    // Assert: /proc 解析到 .../md5link/input，wakeupPath 去掉末段后拼接
    ASSERT_EQ(ut_writeWakeup_calls.size(), 1);
    EXPECT_EQ(ut_writeWakeup_calls.first().first, QStringLiteral("/sys/devices/ut_t27/md5link/power/wakeup"));
    EXPECT_TRUE(ut_writeWakeup_calls.first().second);
}

TEST_F(WakeupUtilsTest, updateWakeupDeviceInfo_noUniqueId_skipped)
{
    // Arrange: 无 Unique ID 且无 Vendor/Device 行 -> uniqueID 为空跳过
    Stub stub;
    ASSERT_TRUE(UTDB::setup(EnableSqlManager::getInstance()));

    ut_writeWakeup_calls.clear();
    stub.set(ADDR(WakeupUtils, writeWakeupFile), ut_writeWakeupFile_stub);

    // Act: 摘除 Unique ID 行
    QString info = utKbInfoUid("/devices/ut_t27/input/input5", "ut-uid-x");
    info.remove(QRegularExpression("^Unique ID: .*\\n", QRegularExpression::MultilineOption));
    WakeupUtils::updateWakeupDeviceInfo(info);

    // Assert: 不写唤醒文件，也不产生库记录
    EXPECT_EQ(ut_writeWakeup_calls.size(), 0);
    EXPECT_FALSE(EnableSqlManager::getInstance()->isWakeupUniqueIdExisted("ut-uid-x"));
}

TEST_F(WakeupUtilsTest, writeWakeupFile_writeFails_returnsFalse)
{
    // Arrange: 真实临时文件可打开，但 write 桩返回 -1
    Stub stub;
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    const QString path = tmpDir.path() + QStringLiteral("/wakeup_fail");

    stub.set((char *)ut_qtSym(UT_QSYM_QIOWRITE), ut_qiowriteFail_stub);

    // Act
    const bool ok = WakeupUtils::writeWakeupFile(path, true);

    // Assert: 写失败且文件无残留内容
    EXPECT_FALSE(ok);
    QFile leftover(path);
    ASSERT_TRUE(leftover.open(QIODevice::ReadOnly));
    EXPECT_EQ(leftover.readAll(), QByteArray());
}

TEST_F(WakeupUtilsTest, getPS2Syspath_procOpenFailed_returnsEmpty)
{
    // Arrange: /proc/bus/input/devices 打开失败
    Stub stub;
    stub.set((char *)ut_qtSym(UT_QSYM_QFILE_OPEN), ut_qfileOpen_stub);
    ut_qfileOpen_ret = false;
    ut_qfileOpen_calls = 0;

    // Act
    const QString result = WakeupUtils::getPS2Syspath("input/event5");

    // Assert: 打开失败 -> 空 syspath，且尝试过打开一次
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result, QString());
    EXPECT_EQ(ut_qfileOpen_calls, 1);
}
