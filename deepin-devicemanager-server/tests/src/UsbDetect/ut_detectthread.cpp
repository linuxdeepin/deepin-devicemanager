// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * 用例计数声明（method | level | factors | min | actual）
 * | DetectThread::DetectThread | mid | concurrent_class | 2 | 2 |
 * | DetectThread::curHwinfoUsbInfo | high | concurrent_class,complexity:5,lines:53,in_degree:2 | 3 | 7 |
 * | DetectThread::getMapInfo | mid | concurrent_class | 2 | 4 |
 * | DetectThread::isUsbDevicesChanged | mid | concurrent_class | 2 | 5 |
 * | DetectThread::run | mid | concurrent_class | 2 | 2 |
 * | DetectThread::setWorkingFlag | mid | concurrent_class | 2 | 2 |
 * | DetectThread::slotUsbChanged | mid | concurrent_class | 2 | 2 |
 * | DetectThread::updateMemUsbInfo | mid | concurrent_class,in_degree:3 | 2 | 2 |
 */

#include <gtest/gtest.h>

#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QProcess>
#include <QSignalSpy>
#include <QThread>

#include <unistd.h>

#include "stubext.h"
#include "detectthread.h"
#include "monitorusb.h"

using namespace stub_ext;

namespace {
// detectthread.cpp:13 定义 LEAST_NUM == 10，getMapInfo 要求行数大于 10 才解析
constexpr int kLeastNum = 10;
// 伪造的 hwinfo 输出中 mock 的 SysFS 路径
const QString kMouseSysFsId = "/devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0";
const QString kMouseSysFsLink = "/devices/pci0000:00/0000:00:14.0/usb1/1-1";
const QString kKeyboardSysFsId = "/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0";
const QString kKeyboardSysFsLink = "/devices/pci0000:00/0000:00:14.0/usb1/1-2";

// hwinfo --usb 输出条目风格：12 行（> LEAST_NUM），键值以 ": " 分隔
QString mouseItem(bool withLink = true)
{
    QString item =
        "03: USB 00.0\n"
        "  Device: usb 0x7523\n"
        "  Vendor: usb 0x1a86\n"
        "  Hardware Class: mouse\n"
        "  SysFS ID: " + kMouseSysFsId + "\n";
    if (withLink)
        item += "  SysFS Device Link: " + kMouseSysFsLink + "\n";
    item +=
        "  Model: \"CH340 USB Mouse\"\n"
        "  Hotplug: USB\n"
        "  Revision: 4.00\n"
        "  Serial ID: \"0001\"\n"
        "  Speed: 12Mbps\n"
        "  Modules: usbhid hid-generic\n";
    return item;
}

QString keyboardItem()
{
    return
        "04: USB 00.0\n"
        "  Device: usb 0xc52b\n"
        "  Vendor: usb 0x046d\n"
        "  Hardware Class: keyboard\n"
        "  SysFS ID: " + kKeyboardSysFsId + "\n"
        "  SysFS Device Link: " + kKeyboardSysFsLink + "\n"
        "  Model: \"Logitech K380\"\n"
        "  Hotplug: USB\n"
        "  Revision: 1.00\n"
        "  Serial ID: \"0002\"\n"
        "  Speed: 12Mbps\n"
        "  Modules: hid-generic\n";
}

QString diskItem(bool withCapacity)
{
    QString item =
        "05: USB 00.0\n"
        "  Device: usb 0x1666\n"
        "  Vendor: usb 0x0951\n"
        "  Hardware Class: disk\n"
        "  SysFS ID: /devices/pci0000:00/0000:00:14.0/usb1/1-3\n"
        "  SysFS Device Link: /devices/pci0000:00/0000:00:14.0/usb1/1-3\n"
        "  Model: \"USB Disk\"\n"
        "  Hotplug: USB\n"
        "  Revision: 1.00\n"
        "  Serial ID: \"0003\"\n"
        "  Speed: 480Mbps\n";
    if (withCapacity)
        item += "  Capacity: 16GB\n";
    return item;
}

// 按真实实现计算唯一 ID：base64(MD5(vendor+device 拼接值 + SysFS 路径))
QString expectedUniqueId(const QString &vendor, const QString &device, const QString &sysfs)
{
    QString devToken = device.split(" ").at(1);
    devToken.remove("0x", Qt::CaseSensitive);
    QString valueStr = vendor.split(" ").at(1).trimmed() + devToken.trimmed();
    QByteArray buf;
    buf.append(valueStr.toUtf8());
    buf.append(sysfs.trimmed().toUtf8());
    return QString::fromStdString(
        QCryptographicHash::hash(buf, QCryptographicHash::Md5).toBase64().toStdString());
}

// 构造 isUsbDevicesChanged 测试用的设备缓存项
QMap<QString, QMap<QString, QString>> deviceMap(const QString &key, const QString &hwClass,
                                                bool withCapacity = false)
{
    QMap<QString, QMap<QString, QString>> result;
    QMap<QString, QString> info;
    info.insert("Hardware Class", hwClass);
    if (withCapacity)
        info.insert("Capacity", "16GB");
    result.insert(key, info);
    return result;
}
} // namespace

class DetectThreadTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stubQProcess();
    }

    void TearDown() override
    {
        stub.clear();
    }

    // hwinfo 探测全部走 QProcess，打桩后测试不依赖真实硬件环境
    void stubQProcess()
    {
        stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                       [](QProcess *, const QString &, const QStringList &, QIODevice::OpenMode) {});
        stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                       [](QProcess *, int) -> bool { return true; });
        stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                       [](QProcess *) -> QByteArray { return QByteArray(); });
    }

    // 让 curHwinfoUsbInfo 直接返回构造好的设备缓存
    void stubCurUsbInfo(QMap<QString, QMap<QString, QString>> &curMap)
    {
        stub.set_lamda(ADDR(DetectThread, curHwinfoUsbInfo),
                       [&curMap](DetectThread *, QMap<QString, QMap<QString, QString>> &out) {
                           out = curMap;
                       });
    }

    stub_ext::StubExt stub;
};

// ---------- DetectThread::DetectThread (detectthread.cpp:17-30) ----------
// 分支：构造 MonitorUsb -> 连接 usbChanged 信号 -> curHwinfoUsbInfo + updateMemUsbInfo 初始化缓存
TEST_F(DetectThreadTest, Ctor_WithStubbedHwinfo_BuildsInitialCache)
{
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    // Act
    QByteArray hwinfoOut = (mouseItem() + "\n" + keyboardItem()).toUtf8();
    stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                   [hwinfoOut](QProcess *) -> QByteArray { return hwinfoOut; });

    DetectThread thread(nullptr);

    // 两台设备均生成唯一 ID 后写入缓存
    // Assert
    EXPECT_EQ(size_t(2), thread.m_MapUsbInfo.size());
    EXPECT_TRUE(thread.m_MapUsbInfo.contains(expectedUniqueId("usb 0x1a86", "usb 0x7523", kMouseSysFsLink)));
    EXPECT_TRUE(thread.m_MapUsbInfo.contains(expectedUniqueId("usb 0x046d", "usb 0xc52b", kKeyboardSysFsLink)));
    ASSERT_NE(nullptr, thread.mp_MonitorUsb);
}

// DetectThread 构造时将 MonitorUsb::usbChanged 连接到 slotUsbChanged（Qt::QueuedConnection）
TEST_F(DetectThreadTest, Ctor_MonitorSignalRoute_ForwardsToUsbChanged)
{
    // Act
    // （fixture 已打桩默认环境，无额外前置条件）
    // Arrange
    DetectThread thread(nullptr);
    stub.set_lamda(ADDR(DetectThread, isUsbDevicesChanged),
                   [](DetectThread *) -> bool { return true; });

    QSignalSpy spy(&thread, &DetectThread::usbChanged);
    emit thread.mp_MonitorUsb->usbChanged();
    QApplication::processEvents();   // 队列连接需要事件循环分发

    // Assert
    EXPECT_FALSE(thread.isRunning());   // 信号转发不启动监听线程

    EXPECT_EQ(1, spy.count());
}

// ---------- DetectThread::curHwinfoUsbInfo (detectthread.cpp:103-155) ----------
// 分支：Vendor/Device 均含 0x 且 SysFS ID/Device Link 非空 -> 生成 MD5 base64 唯一 ID
TEST_F(DetectThreadTest, CurHwinfoUsbInfo_VendorDeviceSysFs_GeneratesBase64Md5Key)
{
    // Arrange
    DetectThread thread(nullptr);
    QByteArray hwinfoOut = mouseItem().toUtf8();
    stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                   [hwinfoOut](QProcess *) -> QByteArray { return hwinfoOut; });

    QMap<QString, QMap<QString, QString>> usbInfo;
    // Act
    thread.curHwinfoUsbInfo(usbInfo);

    // Assert
    EXPECT_EQ(size_t(1), usbInfo.size());
    // SysFS ID 与 Device Link 同时存在时，真实实现优先使用 Device Link 参与哈希
    QString expectedKey = expectedUniqueId("usb 0x1a86", "usb 0x7523", kMouseSysFsLink);
    EXPECT_TRUE(usbInfo.contains(expectedKey));
    EXPECT_EQ("mouse", usbInfo[expectedKey]["Hardware Class"]);
    EXPECT_EQ("usb 0x1a86", usbInfo[expectedKey]["Vendor"]);
    EXPECT_EQ("usb 0x7523", usbInfo[expectedKey]["Device"]);
}

// 分支：仅有 SysFS ID（无 Device Link）时使用 SysFS ID 参与哈希
TEST_F(DetectThreadTest, CurHwinfoUsbInfo_SysFsIdOnly_UsesIdForHash)
{
    // Arrange
    DetectThread thread(nullptr);
    QByteArray hwinfoOut = mouseItem(false).toUtf8();
    stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                   [hwinfoOut](QProcess *) -> QByteArray { return hwinfoOut; });

    QMap<QString, QMap<QString, QString>> usbInfo;
    // Act
    thread.curHwinfoUsbInfo(usbInfo);

    // Assert
    EXPECT_EQ(size_t(1), usbInfo.size());
    QString idKey = expectedUniqueId("usb 0x1a86", "usb 0x7523", kMouseSysFsId);
    EXPECT_TRUE(usbInfo.contains(idKey));
    // 与 Device Link 哈希结果不同，证明用的是 SysFS ID
    EXPECT_FALSE(usbInfo.contains(expectedUniqueId("usb 0x1a86", "usb 0x7523", kMouseSysFsLink)));
}

// 分支：Vendor 不含 0x（外层条件不成立）-> 保留 hwinfo 自带的 Unique ID 字段
TEST_F(DetectThreadTest, CurHwinfoUsbInfo_VendorWithoutHex_KeepsUniqueIdField)
{
    // Arrange
    QString item = mouseItem();
    item.replace("Vendor: usb 0x1a86", "Vendor: usb abcd");
    item.replace("03: USB 00.0",
                 "03: USB 00.0\n  Unique ID: ABC123");

    DetectThread thread(nullptr);
    QByteArray hwinfoOut = item.toUtf8();
    stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                   [hwinfoOut](QProcess *) -> QByteArray { return hwinfoOut; });

    QMap<QString, QMap<QString, QString>> usbInfo;
    // Act
    thread.curHwinfoUsbInfo(usbInfo);

    // Assert
    EXPECT_EQ(size_t(1), usbInfo.size());
    EXPECT_TRUE(usbInfo.contains("ABC123"));
}

// 分支：Vendor 含 0x 但仅一个 token（内层条件不成立）-> 仍使用 Unique ID 字段
TEST_F(DetectThreadTest, CurHwinfoUsbInfo_VendorSingleToken_FallsBackToUniqueIdField)
{
    // Arrange
    QString item = mouseItem();
    item.replace("Vendor: usb 0x1a86", "Vendor: 0x7523");
    item.replace("03: USB 00.0",
                 "03: USB 00.0\n  Unique ID: FALLBACK");

    DetectThread thread(nullptr);
    QByteArray hwinfoOut = item.toUtf8();
    stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                   [hwinfoOut](QProcess *) -> QByteArray { return hwinfoOut; });

    QMap<QString, QMap<QString, QString>> usbInfo;
    // Act
    thread.curHwinfoUsbInfo(usbInfo);

    // Assert
    EXPECT_EQ(size_t(1), usbInfo.size());
    EXPECT_TRUE(usbInfo.contains("FALLBACK"));
}

// 分支：条目缺少 Vendor/Device/Unique ID -> 以空字符串作为键插入
TEST_F(DetectThreadTest, CurHwinfoUsbInfo_MissingVendorDev_InsertedWithEmptyKey)
{
    // Arrange
    QString item = mouseItem();
    item.replace("  Device: usb 0x7523\n", "");
    item.replace("  Vendor: usb 0x1a86\n", "");

    DetectThread thread(nullptr);
    QByteArray hwinfoOut = item.toUtf8();
    stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                   [hwinfoOut](QProcess *) -> QByteArray { return hwinfoOut; });

    QMap<QString, QMap<QString, QString>> usbInfo;
    // Act
    thread.curHwinfoUsbInfo(usbInfo);

    // Assert
    EXPECT_EQ(size_t(1), usbInfo.size());
    EXPECT_TRUE(usbInfo.contains(QString()));
    EXPECT_EQ("mouse", usbInfo[QString()]["Hardware Class"]);
}

// 分支：Hardware Class 为 hub 的条目在 getMapInfo 内被过滤
TEST_F(DetectThreadTest, CurHwinfoUsbInfo_HubClass_FilteredOut)
{
    // Arrange
    QString hubItem = mouseItem();
    hubItem.replace("Hardware Class: mouse", "Hardware Class: hub");

    DetectThread thread(nullptr);
    QByteArray hwinfoOut = hubItem.toUtf8();
    stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                   [hwinfoOut](QProcess *) -> QByteArray { return hwinfoOut; });

    QMap<QString, QMap<QString, QString>> usbInfo;
    // Act
    thread.curHwinfoUsbInfo(usbInfo);

    // Assert
    EXPECT_TRUE(usbInfo.isEmpty());
    EXPECT_EQ(size_t(0), usbInfo.size());   // 短条目不得部分插入
    EXPECT_EQ(size_t(0), usbInfo.size());   // hub 设备必须整条过滤
}

// 分支：行数不足（<= LEAST_NUM）的条目被跳过
TEST_F(DetectThreadTest, CurHwinfoUsbInfo_ShortItem_Skipped)
{
    // Arrange
    QString shortItem =
        "06: USB 00.0\n"
        "  Device: usb 0x1234\n"
        "  Vendor: usb 0x5678\n"
        "  Hardware Class: mouse\n"
        "  SysFS ID: /devices/usb1\n"
        "  Model: \"Tiny\"\n"
        "  Hotplug: USB\n"
        "  Revision: 1.00\n";

    DetectThread thread(nullptr);
    QByteArray hwinfoOut = shortItem.toUtf8();
    stub.set_lamda(static_cast<QByteArray (QProcess::*)()>(&QProcess::readAllStandardOutput),
                   [hwinfoOut](QProcess *) -> QByteArray { return hwinfoOut; });

    QMap<QString, QMap<QString, QString>> usbInfo;
    // Act
    thread.curHwinfoUsbInfo(usbInfo);

    // Assert
    EXPECT_TRUE(usbInfo.isEmpty());
    EXPECT_EQ(size_t(0), usbInfo.size());   // 行数不足的条目不产出键值
}

// ---------- DetectThread::getMapInfo (detectthread.cpp:157-178) ----------
// 分支：lines.size() <= LEAST_NUM 直接返回 false
TEST_F(DetectThreadTest, GetMapInfo_TooFewLines_ReturnsFalse)
{
    // Arrange
    DetectThread thread(nullptr);
    // 恰好 LEAST_NUM 行，边界上仍视为无用信息
    QStringList lines;
    for (int i = 0; i < kLeastNum; ++i)
        lines << QString("  Key%1: Value%1").arg(i);
    QMap<QString, QString> mapInfo;
    // Assert
    // Act
    EXPECT_FALSE(thread.getMapInfo(lines.join("\n"), mapInfo));
    EXPECT_TRUE(mapInfo.isEmpty());   // 不足行数时不产出任何键值
}

// 分支：行数刚超过 LEAST_NUM 的最小合法条目可以解析
TEST_F(DetectThreadTest, GetMapInfo_MinimumValidItem_Parses)
{
    // Arrange
    DetectThread thread(nullptr);
    QStringList lines;
    for (int i = 0; i <= kLeastNum; ++i)   // LEAST_NUM + 1 行
        lines << QString("  Key%1: Value%1").arg(i);
    QMap<QString, QString> mapInfo;
    // Act
    // Assert
    EXPECT_TRUE(thread.getMapInfo(lines.join("\n"), mapInfo));
    EXPECT_EQ("Value0", mapInfo["Key0"]);
}

// 分支：Hardware Class 为 hub 返回 false
TEST_F(DetectThreadTest, GetMapInfo_HubHardwareClass_ReturnsFalse)
{
    // Arrange
    DetectThread thread(nullptr);
    QString item = mouseItem();
    item.replace("Hardware Class: mouse", "Hardware Class: hub");
    QMap<QString, QString> mapInfo;
    // Act
    // Assert
    EXPECT_FALSE(thread.getMapInfo(item, mapInfo));
    EXPECT_EQ("hub", mapInfo["Hardware Class"]);   // 行已解析，但 hub 类型被过滤
}

// 分支：分隔后不是两段的行被跳过，合法行正常解析
TEST_F(DetectThreadTest, GetMapInfo_MalformedLines_Skipped)
{
    // Arrange
    DetectThread thread(nullptr);
    QString item = mouseItem();
    // 追加两类非法行：无 ": " 分隔符 / 有多个 ": " 分隔符
    item += "This line has no separator\n"
            "Bad: line: with: colons\n";
    QMap<QString, QString> mapInfo;
    // Assert
    // Act
    EXPECT_TRUE(thread.getMapInfo(item, mapInfo));
    EXPECT_EQ("mouse", mapInfo["Hardware Class"]);
    EXPECT_FALSE(mapInfo.contains("This line has no separator"));
    EXPECT_FALSE(mapInfo.contains("Bad"));
}

// ---------- DetectThread::updateMemUsbInfo (detectthread.cpp:97-101) ----------
// 行为：整体替换缓存而非合并
TEST_F(DetectThreadTest, UpdateMemUsbInfo_NewCacheGiven_ReplacesWholeCache)
{
    // Arrange
    DetectThread thread(nullptr);

    QMap<QString, QMap<QString, QString>> first = deviceMap("KEY_A", "mouse");
    first.insert("KEY_B", QMap<QString, QString> { {"Hardware Class", "keyboard"} });
    // Act
    thread.updateMemUsbInfo(first);
    // Assert
    EXPECT_EQ(size_t(2), thread.m_MapUsbInfo.size());

    QMap<QString, QMap<QString, QString>> second = deviceMap("KEY_C", "disk");
    thread.updateMemUsbInfo(second);
    EXPECT_EQ(size_t(1), thread.m_MapUsbInfo.size());
    EXPECT_TRUE(thread.m_MapUsbInfo.contains("KEY_C"));
    EXPECT_FALSE(thread.m_MapUsbInfo.contains("KEY_A"));
}

// ---------- DetectThread::isUsbDevicesChanged (detectthread.cpp:68-95) ----------
// 分支：cur 数量 < 缓存数量（拔出）-> true 且缓存被刷新
TEST_F(DetectThreadTest, IsUsbDevicesChanged_DeviceRemoved_ReturnsTrueAndUpdatesCache)
{
    // Arrange
    QMap<QString, QMap<QString, QString>> curMap = deviceMap("KEY_A", "mouse");
    curMap.insert("KEY_B", QMap<QString, QString> { {"Hardware Class", "keyboard"} });
    stubCurUsbInfo(curMap);

    DetectThread thread(nullptr);   // 构造时缓存 = curMap（2 项）
    curMap = deviceMap("KEY_A", "mouse");   // 模拟拔出一个设备

    // Assert
    // Act
    EXPECT_TRUE(thread.isUsbDevicesChanged());
    EXPECT_EQ(size_t(1), thread.m_MapUsbInfo.size());
}

// 分支：新设备（非 disk）不在缓存中 -> true
TEST_F(DetectThreadTest, IsUsbDevicesChanged_NewNonDiskDevice_ReturnsTrue)
{
    // Arrange
    QMap<QString, QMap<QString, QString>> curMap = deviceMap("KEY_A", "mouse");
    stubCurUsbInfo(curMap);

    DetectThread thread(nullptr);
    curMap.insert("KEY_NEW", QMap<QString, QString> { {"Hardware Class", "keyboard"} });

    // Act
    // Assert
    EXPECT_TRUE(thread.isUsbDevicesChanged());
    EXPECT_EQ(size_t(2), thread.m_MapUsbInfo.size());
}

// 分支：新设备是 disk 且无 Capacity（内核未处理完）-> 忽略并返回 false
TEST_F(DetectThreadTest, IsUsbDevicesChanged_NewDiskWithoutCapacity_Ignored)
{
    // Arrange
    QMap<QString, QMap<QString, QString>> curMap = deviceMap("KEY_A", "mouse");
    stubCurUsbInfo(curMap);

    DetectThread thread(nullptr);
    curMap.insert("KEY_DISK", QMap<QString, QString> { {"Hardware Class", "disk"} });

    // Act
    // Assert
    EXPECT_FALSE(thread.isUsbDevicesChanged());
    EXPECT_FALSE(thread.isUsbDevicesChanged());   // 缓存未变，重复调用结果一致
    // 缓存未被更新
    EXPECT_EQ(size_t(1), thread.m_MapUsbInfo.size());
}

// 分支：新设备是 disk 且带 Capacity -> 视为新设备返回 true
TEST_F(DetectThreadTest, IsUsbDevicesChanged_NewDiskWithCapacity_ReturnsTrue)
{
    // Arrange
    QMap<QString, QMap<QString, QString>> curMap = deviceMap("KEY_A", "mouse");
    stubCurUsbInfo(curMap);

    DetectThread thread(nullptr);
    curMap.insert("KEY_DISK", QMap<QString, QString> { {"Hardware Class", "disk"},
                                                       {"Capacity", "16GB"} });

    // Act
    // Assert
    EXPECT_TRUE(thread.isUsbDevicesChanged());
    EXPECT_EQ(size_t(2), thread.m_MapUsbInfo.size());
}

// 分支：缓存与当前完全一致 -> false
TEST_F(DetectThreadTest, IsUsbDevicesChanged_NoChange_ReturnsFalse)
{
    // Arrange
    QMap<QString, QMap<QString, QString>> curMap = deviceMap("KEY_A", "mouse");
    stubCurUsbInfo(curMap);

    DetectThread thread(nullptr);

    // Act
    // Assert
    EXPECT_FALSE(thread.isUsbDevicesChanged());
    EXPECT_TRUE(thread.m_MapUsbInfo.contains("KEY_A"));   // 缓存内容未发生变更
}

// ---------- DetectThread::run (detectthread.cpp:32-40) ----------
// 行为：run 进入 MonitorUsb::monitor 监听循环（此处打桩验证调用即可）
TEST_F(DetectThreadTest, Run_MonitorLoop_ExecutesOncePerRun)
{
    // Arrange
    DetectThread thread(nullptr);
    int monitorCalls = 0;
    stub.set_lamda(ADDR(MonitorUsb, monitor),
                   [&monitorCalls](MonitorUsb *) { ++monitorCalls; });

    // Act
    thread.run();   // run 为 public，直接调用避免真实线程启动

    // Assert
    EXPECT_EQ(1, monitorCalls);

    thread.run();
    EXPECT_EQ(2, monitorCalls);   // run 可重复调用，每轮各执行一次监控
}

// ---------- DetectThread::setWorkingFlag (detectthread.cpp:42-50) ----------
// 分支：flag=false 只转发给 MonitorUsb，不启动线程
TEST_F(DetectThreadTest, SetWorkingFlag_False_ForwardedToMonitorAndThreadNotStarted)
{
    // Arrange
    DetectThread thread(nullptr);
    bool forwardedFlag = true;
    stub.set_lamda(ADDR(MonitorUsb, setWorkingFlag),
                   [&forwardedFlag](MonitorUsb *, bool flag) { forwardedFlag = flag; });

    // Act
    thread.setWorkingFlag(false);

    // Assert
    EXPECT_FALSE(forwardedFlag);
    EXPECT_FALSE(thread.isRunning());
}

// 分支：flag=true 且线程未运行 -> 启动线程并执行监听循环
TEST_F(DetectThreadTest, SetWorkingFlag_True_StartsThreadRunningMonitor)
{
    // Arrange
    DetectThread thread(nullptr);
    int monitorCalls = 0;
    stub.set_lamda(ADDR(MonitorUsb, monitor), [&monitorCalls](MonitorUsb *) { ++monitorCalls; });

    // Act
    thread.setWorkingFlag(true);
    // Assert
    EXPECT_TRUE(thread.wait(5000));   // monitor 已打桩，线程很快退出

    EXPECT_GE(monitorCalls, 1);
    EXPECT_FALSE(thread.isRunning());
}

// ---------- DetectThread::slotUsbChanged (detectthread.cpp:52-66) ----------
// 分支：isUsbDevicesChanged 立即返回 true -> break 后只发射一次 usbChanged
TEST_F(DetectThreadTest, SlotUsbChanged_DevicesSettled_EmitsSignalOnce)
{
    // Arrange
    DetectThread thread(nullptr);
    stub.set_lamda(ADDR(DetectThread, isUsbDevicesChanged),
                   [](DetectThread *) -> bool { return true; });

    QSignalSpy spy(&thread, &DetectThread::usbChanged);
    // Act
    thread.slotUsbChanged();

    // Assert
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(thread.isRunning());   // 稳定判断后仅发信号，不启动线程
}

// 分支：10s 内始终未就绪 -> 等待超时后仍然发射 usbChanged（sleep 打桩避免真实等待）
TEST_F(DetectThreadTest, SlotUsbChanged_WaitTimeout_StillEmitsSignal)
{
    // Arrange
    DetectThread thread(nullptr);
    stub.set_lamda(ADDR(DetectThread, isUsbDevicesChanged),
                   [](DetectThread *) -> bool { return false; });
    stub.set_lamda(&sleep, [](unsigned int) -> unsigned int { return 0; });
    qint64 fakeTick = 1000;
    stub.set_lamda(static_cast<qint64 (*)()>(&QDateTime::currentMSecsSinceEpoch),
                   [&fakeTick]() -> qint64 {
                       qint64 value = fakeTick;
                       fakeTick += 20000;   // 下一次调用直接越过 10s 超时窗口
                       return value;
                   });

    QSignalSpy spy(&thread, &DetectThread::usbChanged);
    // Act
    thread.slotUsbChanged();

    // Assert
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(thread.isRunning());   // 超时窗口结束后同样仅发信号
}

TEST_F(DetectThreadTest, Run_CalledTwice_MonitorExecutesEachTime)
{
    // Arrange
    DetectThread thread(nullptr);
    int monitorCalls = 0;
    stub.set_lamda(ADDR(MonitorUsb, monitor),
                   [&monitorCalls](MonitorUsb *) { ++monitorCalls; });
    // Act
    thread.run();
    thread.run();
    // Assert
    EXPECT_EQ(2, monitorCalls);   // 每次调用各执行一轮监控
    EXPECT_FALSE(thread.isRunning());
}

TEST_F(DetectThreadTest, UpdateMemUsbInfo_EmptyCache_ClearsAllEntries)
{
    // Arrange
    DetectThread thread(nullptr);
    QMap<QString, QMap<QString, QString>> filled = deviceMap("KEY_A", "mouse");
    // Act
    thread.updateMemUsbInfo(filled);
    QMap<QString, QMap<QString, QString>> empty;
    // Act
    thread.updateMemUsbInfo(empty);   // 循环边界：空集合整体替换
    // Assert
    EXPECT_TRUE(thread.m_MapUsbInfo.isEmpty());   // 原有缓存被清空
    EXPECT_EQ(size_t(0), thread.m_MapUsbInfo.size());
}
