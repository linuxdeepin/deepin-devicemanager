// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * DeviceAudio 单元测试（qt-autotest-generator 自动生成/重构）
 *
 * 打桩决策（依赖矩阵，外部依赖一律 stub，测试不依赖真实设备环境）：
 *  - DBusEnableInterface::enable —— DBus 禁用/启用接口，一律 stub（成功/失败双路径）
 *  - DeviceBaseInfo::get_string —— 静态方法，真实实现读 /sys 文件，一律 stub（map 假数据）
 *  - QFile::open —— dlsym 符号地址打桩（QFile 覆盖了 QIODevice::open，
 *    打桩 QIODevice::open 无法拦截，且虚成员指针打桩会 vcall 段错误），vendor_id 探测用
 *  - QDir::exists / QFileInfo::isSymLink —— /sys/class/sound 目录与链接探测，PMF 打桩
 * 其余（setAttribute/matchToLshw/setHwinfoLshwKey/driverIsKernelIn/getOtherMapInfo 等）
 * 均为纯内存逻辑，使用真实实现。
 *
 * 用例计数声明（min 按技能 §4 level/factors 下限；actual 为本文件实际用例数）：
 *
 * | 方法                     | level | factors                                                | min | actual |
 * |--------------------------|-------|--------------------------------------------------------|-----|--------|
 * | DeviceAudio              | mid   | in_degree:1                                            | 2   | 2      |
 * | chip_name                | low   | -                                                      | 1   | 1      |
 * | driver                   | mid   | in_degree:1                                            | 2   | 2      |
 * | enable                   | mid   | in_degree:3                                            | 2   | 2      |
 * | getOverviewInfo          | low   | -                                                      | 1   | 2      |
 * | initFilterKey            | mid   | in_degree:1                                            | 2   | 2      |
 * | loadBaseDeviceInfo       | low   | -                                                      | 1   | 1      |
 * | loadOtherDeviceInfo      | low   | -                                                      | 1   | 2      |
 * | loadTableData            | low   | -                                                      | 1   | 2      |
 * | loadTableHeader          | low   | -                                                      | 1   | 1      |
 * | name                     | low   | -                                                      | 1   | 1      |
 * | setAudioChipFromDmesg    | mid   | in_degree:1                                            | 2   | 2      |
 * | setEnable                | mid   | in_degree:1                                            | 2   | 4      |
 * | setInfoFromCatAudio      | low   | -                                                      | 1   | 2      |
 * | setInfoFromCatDevices    | low   | -                                                      | 1   | 2      |
 * | setInfoFromHwinfo        | mid   | lines:52                                               | 2   | 3      |
 * | setInfoFromLshw          | mid   | in_degree:1                                            | 2   | 4      |
 * | setInfoFrom_sysFS        | high  | complexity:6;lines:77;transitive_loop_depth:4          | 4   | 4      |
 * | setInfoFromTomlOneByOne  | mid   | in_degree:1                                            | 2   | 2      |
 * | subTitle                 | low   | -                                                      | 1   | 1      |
 * | uniqueID                 | mid   | in_degree:2                                            | 2   | 2      |
 * | vendor                   | low   | -                                                      | 1   | 1      |
 */

#include "DeviceAudio.h"
#include "DeviceInfo.h"
#include "commonfunction.h"
#include "DBusEnableInterface.h"
#include "stub.h"
#include "ut_Head.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>

#include <dlfcn.h>
#include <gtest/gtest.h>

/* ---------------- stub 实现 ---------------- */

// DBus enable 打桩返回值（非静态成员函数，stub 首参接收 this）
static bool ut_audio_dbusRet = true;
static bool ut_audio_dbusEnable(void *, const QString &, const QString &, const QString &,
                                const QString &, bool, const QString &)
{
    return ut_audio_dbusRet;
}

// Common::executeClientCmd 打桩：阻断 modinfo 真实子进程，driverIsKernelIn 依赖其输出
static QByteArray ut_audio_execCmdRet = QByteArray();
static QByteArray ut_audio_execCmd_stub(const QString &, const QStringList &, const QString &, int, bool)
{
    return ut_audio_execCmdRet;
}

// QFile::open 导出符号地址（QFile 覆盖 QIODevice::open，必须打桩 QFile 符号本体）
static char *ut_audio_qfileOpenAddr()
{
    static char *addr = (char *)dlsym(RTLD_DEFAULT, "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    return addr;
}
static bool ut_audio_fileOpenRet = true;
static bool ut_audio_open_stub(void *, int) { return ut_audio_fileOpenRet; }

// QDir::exists / QFileInfo::isSymLink 打桩返回值
static bool ut_audio_dirExistsRet = true;
static bool ut_audio_dirExists(void *) { return ut_audio_dirExistsRet; }
static bool ut_audio_isSymLinkRet = true;
static bool ut_audio_isSymLink(void *) { return ut_audio_isSymLinkRet; }

// DeviceBaseInfo::get_string 打桩：非静态成员函数（stub 首参接收 this），按路径后缀返回假数据
static QMap<QString, QString> ut_audio_stringMap;
static QString ut_audio_getString_stub(void *, const QString &path)
{
    const QString key = path.mid(path.lastIndexOf('/') + 1);
    return ut_audio_stringMap.value(key);
}

// 分支清单（来源：DeviceAudio::setEnable）
// B1: SysPath 不含 "usb" → m_UniqueID 被置为 m_Name
// B2: UniqueID 或 SysPath 为空 → 早退返回 EDS_Faild
// B3: DBus enable 成功 → m_Enable=e
// B4: DBus enable 失败 → 仅记录，不改 m_Enable
// B5: res ? EDS_Success : EDS_Faild → 三元选择返回值
// 分支清单（来源：DeviceAudio::setInfoFromHwinfo）
// B1: mapInfo 含 "path" → 取小写 name/driver 键并 return false（禁用态）
// B2: 驱动为内核模块（Driver Modules 或 Driver）→ m_CanUninstall=false
// B3: 正常路径 → 属性齐备并 return true
// 分支清单（来源：DeviceAudio::setInfoFromLshw）
// B1: matchToLshw 不匹配 → return false
// B2: vendor 为 "0000" → 清空 vendor
// B3: version 为 "00" → 清空 version
// B4: 正常路径 → 属性齐备并 return true
// 分支清单（来源：DeviceAudio::setInfoFrom_sysFS）
// B1: card 路径为符号链接 → m_BusInfo/m_SysPath 取链接目标
// B2: hwC0Dn 目录存在且 vendor_id 可打开 → break
// B3: n==9 仍未找到有效 hwC0Dn → return false
// B4: VID 为空 → m_VID_PID 取 vendor_id 内容
// B5: PID 为空 → 同样走 vendor_id 分支（|| 子分支）
// B6: VID/PID 均有效 → 去除 0x 前缀后拼接
// B7: 成功 → m_CanEnable/m_CanUninstall=false 并 return true

class UT_DeviceAudio : public ::testing::Test
{
public:
    void SetUp()
    {
        m_deviceAudio = new DeviceAudio;
        ut_audio_dbusRet = true;
        ut_audio_fileOpenRet = true;
        ut_audio_dirExistsRet = true;
        ut_audio_isSymLinkRet = true;
        ut_audio_stringMap.clear();
    }
    void TearDown()
    {
        delete m_deviceAudio;
    }
    DeviceAudio *m_deviceAudio;
};

/* ===================== DeviceAudio 构造 ===================== */

// 等价类：构造默认状态
TEST_F(UT_DeviceAudio, UT_DeviceAudio_DeviceAudio_Defaults)
{
    // Arrange：SetUp 已构造新对象
    ASSERT_NE(m_deviceAudio, nullptr);

    // Act
    QString name = m_deviceAudio->name();

    // Assert：默认名称为空，非 cat 设备
    EXPECT_STREQ("", name.toStdString().c_str());
    EXPECT_FALSE(m_deviceAudio->m_IsCatDevice);
}

// 等价类：构造后标识/路径成员为空
TEST_F(UT_DeviceAudio, UT_DeviceAudio_DeviceAudio_EmptyIdentity)
{
    // Arrange：SetUp 已构造新对象
    ASSERT_NE(m_deviceAudio, nullptr);

    // Act：读取初始状态
    const QString uid = m_deviceAudio->uniqueID();

    // Assert：标识与路径成员均为空
    EXPECT_STREQ("", uid.toStdString().c_str());
    EXPECT_STREQ("", m_deviceAudio->m_SysPath.toStdString().c_str());
}

/* ===================== chip_name ===================== */

// 等价类：chip_name 与成员值一致
TEST_F(UT_DeviceAudio, UT_DeviceAudio_chip_name)
{
    // Arrange：通过 dmesg 设置芯片型号
    m_deviceAudio->setAudioChipFromDmesg("Realtek ALC256");

    // Act
    QString chip = m_deviceAudio->chip_name();

    // Assert
    EXPECT_STREQ("Realtek ALC256", chip.toStdString().c_str());
    EXPECT_EQ(m_deviceAudio->m_Chip, chip);
}

/* ===================== driver ===================== */

// 等价类：Driver Modules 非空 → 优先返回 Driver Modules
TEST_F(UT_DeviceAudio, UT_DeviceAudio_driver_PrefersDriverModules)
{
    // Arrange：两个键都有值
    QMap<QString, QString> map;
    map.insert("Driver", "snd_hda_intel");
    map.insert("Driver Modules", "snd_module");
    m_deviceAudio->setInfoFromHwinfo(map);

    // Act
    QString value = m_deviceAudio->driver();

    // Assert：返回 Driver Modules 且与成员一致
    EXPECT_STREQ("snd_module", value.toStdString().c_str());
    EXPECT_EQ(m_deviceAudio->m_DriverModules, value);
}

// 等价类：无 Driver Modules → 回退返回 Driver
TEST_F(UT_DeviceAudio, UT_DeviceAudio_driver_FallbackToDriver)
{
    // Arrange：只有 Driver 键
    QMap<QString, QString> map;
    map.insert("Driver", "snd_hda_intel");
    m_deviceAudio->setInfoFromHwinfo(map);

    // Act
    QString value = m_deviceAudio->driver();

    // Assert：回退返回 Driver 且与成员一致
    EXPECT_STREQ("snd_hda_intel", value.toStdString().c_str());
    EXPECT_EQ(m_deviceAudio->m_Driver, value);
}

/* ===================== enable ===================== */

// 等价类：enable 返回当前状态（true）
TEST_F(UT_DeviceAudio, UT_DeviceAudio_enable_True)
{
    // Arrange：置启用状态
    m_deviceAudio->m_Enable = true;

    // Act
    bool state = m_deviceAudio->enable();

    // Assert
    EXPECT_TRUE(state);
    EXPECT_EQ(m_deviceAudio->m_Enable, state);
}

// 等价类：enable 返回当前状态（false）
TEST_F(UT_DeviceAudio, UT_DeviceAudio_enable_False)
{
    // Arrange：置禁用状态
    m_deviceAudio->m_Enable = false;

    // Act
    bool state = m_deviceAudio->enable();

    // Assert
    EXPECT_FALSE(state);
    EXPECT_EQ(m_deviceAudio->m_Enable, state);
}

/* ===================== getOverviewInfo ===================== */

// 等价类：名称非空 → 概览即名称
TEST_F(UT_DeviceAudio, UT_DeviceAudio_getOverviewInfo_NameUsed)
{
    // Arrange：设置名称
    QMap<QString, QString> map;
    map.insert("Name", "Cannon Lake PCH cAVS");
    m_deviceAudio->setInfoFromCatAudio(map);

    // Act
    QString value = m_deviceAudio->getOverviewInfo();

    // Assert
    EXPECT_STREQ("Cannon Lake PCH cAVS", value.toStdString().c_str());
    EXPECT_EQ(m_deviceAudio->name(), value);
}

// 边界：名称为空 → 概览回退为型号
TEST_F(UT_DeviceAudio, UT_DeviceAudio_getOverviewInfo_EmptyNameFallsToModel)
{
    // Arrange：仅设置型号
    m_deviceAudio->m_Model = "Model A";

    // Act
    QString value = m_deviceAudio->getOverviewInfo();

    // Assert
    EXPECT_STREQ("Model A", value.toStdString().c_str());
    EXPECT_EQ(m_deviceAudio->m_Model, value);
}

/* ===================== initFilterKey ===================== */

// 等价类：过滤键初始化（构造即含 16 个键）
TEST_F(UT_DeviceAudio, UT_DeviceAudio_initFilterKey)
{
    // Arrange：构造后已含初始键集
    const int initialSize = m_deviceAudio->m_FilterKey.size();

    // Act
    m_deviceAudio->initFilterKey();

    // Assert
    EXPECT_EQ(16, m_deviceAudio->m_FilterKey.size());
    EXPECT_EQ(16, initialSize);
    EXPECT_TRUE(m_deviceAudio->m_FilterKey.contains("Device Name"));
    EXPECT_TRUE(m_deviceAudio->m_FilterKey.contains("Driver"));
}

// 等价类：重复调用幂等（QSet 去重）
TEST_F(UT_DeviceAudio, UT_DeviceAudio_initFilterKey_Idempotent)
{
    // Arrange：先初始化一次
    m_deviceAudio->initFilterKey();

    // Act：再初始化一次
    m_deviceAudio->initFilterKey();

    // Assert：数量不变
    EXPECT_EQ(16, m_deviceAudio->m_FilterKey.size());
    EXPECT_TRUE(m_deviceAudio->m_FilterKey.contains("latency"));
}

/* ===================== loadBaseDeviceInfo ===================== */

// 等价类：基本信息按固定顺序装载
TEST_F(UT_DeviceAudio, UT_DeviceAudio_loadBaseDeviceInfo)
{
    // Arrange：填充基础信息
    QMap<QString, QString> map;
    map.insert("Device", "Cannon Lake PCH cAVS");
    map.insert("Vendor", "Intel Corporation");
    map.insert("Driver", "snd_hda_intel");
    m_deviceAudio->setInfoFromHwinfo(map);

    // Act
    m_deviceAudio->loadBaseDeviceInfo();

    // Assert：六项基本信息，空值项被跳过，实际生成 3 项
    ASSERT_EQ(3, m_deviceAudio->m_LstBaseInfo.size());
    EXPECT_STREQ("Cannon Lake PCH cAVS", m_deviceAudio->m_LstBaseInfo.at(0).second.toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceAudio->m_LstBaseInfo.at(1).second.toStdString().c_str());
    EXPECT_STREQ("snd_hda_intel", m_deviceAudio->m_LstBaseInfo.at(2).second.toStdString().c_str());
}

/* ===================== loadOtherDeviceInfo ===================== */

// 等价类：空设备 → 无其他信息
TEST_F(UT_DeviceAudio, UT_DeviceAudio_loadOtherDeviceInfo_Empty)
{
    // Arrange：空设备
    ASSERT_NE(m_deviceAudio, nullptr);

    // Act
    m_deviceAudio->loadOtherDeviceInfo();

    // Assert：其他信息列表为空
    EXPECT_EQ(0, m_deviceAudio->m_LstOtherInfo.size());
    EXPECT_TRUE(m_deviceAudio->m_LstOtherInfo.isEmpty());
}

// 等价类：有 Chip/物理 ID → 对应条目生成
TEST_F(UT_DeviceAudio, UT_DeviceAudio_loadOtherDeviceInfo_WithChip)
{
    // Arrange：设置芯片与物理 ID
    m_deviceAudio->m_Chip = "Realtek ALC256";
    m_deviceAudio->m_PhysID = "0000:00:1f.3";

    // Act
    m_deviceAudio->loadOtherDeviceInfo();

    // Assert：包含 Chip 与 Physical ID 条目
    ASSERT_EQ(2, m_deviceAudio->m_LstOtherInfo.size());
    EXPECT_STREQ("Chip", m_deviceAudio->m_LstOtherInfo.at(0).first.toStdString().c_str());
    EXPECT_TRUE(m_deviceAudio->m_LstOtherInfo.contains(QPair<QString, QString>("Chip", "Realtek ALC256")));
    EXPECT_TRUE(m_deviceAudio->m_LstOtherInfo.contains(QPair<QString, QString>("Physical ID", "0000:00:1f.3")));
}

/* ===================== loadTableData ===================== */

// 等价类：不可用且禁用 → 禁用前缀覆盖不可用前缀
TEST_F(UT_DeviceAudio, UT_DeviceAudio_loadTableData_UnavailableAndDisabled)
{
    // Arrange：不可用 + 禁用
    m_deviceAudio->m_Name = "PCH cAVS";
    m_deviceAudio->m_Vendor = "Intel Corporation";
    m_deviceAudio->m_Available = false;
    m_deviceAudio->m_Enable = false;

    // Act
    m_deviceAudio->loadTableData();

    // Assert：两列，名称列带 Disable 前缀，厂商列原值
    ASSERT_EQ(2, m_deviceAudio->m_TableData.size());
    EXPECT_STREQ("(Disable) PCH cAVS", m_deviceAudio->m_TableData.at(0).toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceAudio->m_TableData.at(1).toStdString().c_str());
}

// 等价类：可用且启用 → 名称无前缀（需预置驱动，否则 available() 因驱动为空强制不可用）
TEST_F(UT_DeviceAudio, UT_DeviceAudio_loadTableData_AvailableEnabled)
{
    // Arrange：可用 + 启用 + 非空驱动
    m_deviceAudio->m_Name = "PCH cAVS";
    m_deviceAudio->m_Vendor = "Intel Corporation";
    m_deviceAudio->m_Driver = "snd_hda_intel";
    m_deviceAudio->m_Available = true;
    m_deviceAudio->m_Enable = true;

    // Act
    m_deviceAudio->loadTableData();

    // Assert：名称列无前缀，厂商列原值
    ASSERT_EQ(2, m_deviceAudio->m_TableData.size());
    EXPECT_STREQ("PCH cAVS", m_deviceAudio->m_TableData.at(0).toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceAudio->m_TableData.at(1).toStdString().c_str());
}

/* ===================== loadTableHeader ===================== */

// 等价类：表头为两列（Name/Vendor）
TEST_F(UT_DeviceAudio, UT_DeviceAudio_loadTableHeader)
{
    // Arrange：空设备即可
    ASSERT_NE(m_deviceAudio, nullptr);

    // Act
    m_deviceAudio->loadTableHeader();

    // Assert
    EXPECT_EQ(2, m_deviceAudio->m_TableHeader.size());
    EXPECT_STREQ("Name", m_deviceAudio->m_TableHeader.at(0).toStdString().c_str());
    EXPECT_STREQ("Vendor", m_deviceAudio->m_TableHeader.at(1).toStdString().c_str());
}

/* ===================== name ===================== */

// 等价类：name 与成员值一致
TEST_F(UT_DeviceAudio, UT_DeviceAudio_name)
{
    // Arrange：设置名称
    QMap<QString, QString> map;
    map.insert("Name", "Cannon Lake PCH cAVS");
    m_deviceAudio->setInfoFromCatAudio(map);

    // Act
    QString name = m_deviceAudio->name();

    // Assert
    EXPECT_STREQ("Cannon Lake PCH cAVS", name.toStdString().c_str());
    EXPECT_EQ(m_deviceAudio->m_Name, name);
}

/* ===================== setAudioChipFromDmesg ===================== */

// 等价类：设置芯片型号并返回 true
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setAudioChipFromDmesg)
{
    // Arrange：空芯片状态
    ASSERT_TRUE(m_deviceAudio->m_Chip.isEmpty());

    // Act
    bool ret = m_deviceAudio->setAudioChipFromDmesg("x86");

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_STREQ("x86", m_deviceAudio->m_Chip.toStdString().c_str());
}

// 等价类：再次设置覆盖旧值
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setAudioChipFromDmesg_Overwrite)
{
    // Arrange：先设置一个旧值
    m_deviceAudio->setAudioChipFromDmesg("old chip");

    // Act
    bool ret = m_deviceAudio->setAudioChipFromDmesg("new chip");

    // Assert：值被覆盖
    EXPECT_TRUE(ret);
    EXPECT_STREQ("new chip", m_deviceAudio->m_Chip.toStdString().c_str());
}

/* ===================== setEnable ===================== */

// 分支 B3：DBus 成功 → EDS_Success 且状态更新
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setEnable_Success)
{
    // Arrange：标识齐备，DBus 打桩成功
    m_deviceAudio->m_SysPath = "syspath";
    m_deviceAudio->m_UniqueID = "uniqueid";
    m_deviceAudio->m_Name = "name";
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_audio_dbusEnable);
    ut_audio_dbusRet = true;

    // Act
    EnableDeviceStatus value = m_deviceAudio->setEnable(false);

    // Assert
    EXPECT_EQ(EnableDeviceStatus::EDS_Success, value);
    EXPECT_FALSE(m_deviceAudio->m_Enable);
}

// 分支 B2：UniqueID 为空 → EDS_Faild（不触发 DBus，m_Enable 保持默认）
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setEnable_EmptyUniqueID_Fail)
{
    // Arrange：UniqueID 为空
    m_deviceAudio->m_SysPath = "syspath";
    m_deviceAudio->m_UniqueID = "";

    // Act
    EnableDeviceStatus value = m_deviceAudio->setEnable(false);

    // Assert：返回失败，m_Enable 保持构造默认值 true
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, value);
    EXPECT_TRUE(m_deviceAudio->m_Enable);
}

// 分支 B1：SysPath 含 usb → UniqueID 保持原值
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setEnable_UsbPath_KeepsUniqueID)
{
    // Arrange：USB 设备路径
    m_deviceAudio->m_SysPath = "usb:0000:00:1f.3";
    m_deviceAudio->m_UniqueID = "keep-me";
    m_deviceAudio->m_Name = "name";
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_audio_dbusEnable);
    ut_audio_dbusRet = true;

    // Act
    EnableDeviceStatus value = m_deviceAudio->setEnable(true);

    // Assert：UniqueID 未被改写，状态成功
    EXPECT_EQ(EnableDeviceStatus::EDS_Success, value);
    EXPECT_STREQ("keep-me", m_deviceAudio->m_UniqueID.toStdString().c_str());
    EXPECT_TRUE(m_deviceAudio->m_Enable);
}

// 分支 B4：DBus 失败 → EDS_Faild 且 m_Enable 不被改写
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setEnable_DbusFail)
{
    // Arrange：标识齐备，DBus 打桩失败，预置启用状态
    m_deviceAudio->m_SysPath = "usb:dev";
    m_deviceAudio->m_UniqueID = "uid";
    m_deviceAudio->m_Enable = true;
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_audio_dbusEnable);
    ut_audio_dbusRet = false;

    // Act
    EnableDeviceStatus value = m_deviceAudio->setEnable(false);

    // Assert：返回失败，m_Enable 保持原值 true
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, value);
    EXPECT_TRUE(m_deviceAudio->m_Enable);
}

/* ===================== setInfoFromCatAudio ===================== */

// 等价类：全键赋值
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromCatAudio)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Name", "Cannon Lake PCH cAVS");
    map.insert("driver", "snd_hda_intel");
    map.insert("Vendor", "Intel Corporation");
    map.insert("Model", "ALC256");

    // Act
    m_deviceAudio->setInfoFromCatAudio(map);

    // Assert
    EXPECT_STREQ("Cannon Lake PCH cAVS", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_STREQ("snd_hda_intel", m_deviceAudio->m_Driver.toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceAudio->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("ALC256", m_deviceAudio->m_Model.toStdString().c_str());
}

// 边界：空 map → 成员保持空
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromCatAudio_EmptyMap)
{
    // Arrange：空 map
    QMap<QString, QString> map;

    // Act
    m_deviceAudio->setInfoFromCatAudio(map);

    // Assert
    EXPECT_STREQ("", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_STREQ("", m_deviceAudio->m_Vendor.toStdString().c_str());
}

/* ===================== setInfoFromCatDevices ===================== */

// 等价类：赋值成功并置 m_IsCatDevice
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromCatDevices)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Name", "Cannon Lake PCH cAVS");
    map.insert("Vendor", "Intel Corporation");
    map.insert("Version", "10");

    // Act
    bool ret = m_deviceAudio->setInfoFromCatDevices(map);

    // Assert：返回 true，成员赋值，cat 设备标记置位
    EXPECT_TRUE(ret);
    EXPECT_STREQ("Cannon Lake PCH cAVS", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceAudio->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("10", m_deviceAudio->m_Version.toStdString().c_str());
    EXPECT_TRUE(m_deviceAudio->m_IsCatDevice);
}

// 边界：空 map → 成员保持空，仍返回 true
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromCatDevices_EmptyMap)
{
    // Arrange：空 map
    QMap<QString, QString> map;

    // Act
    bool ret = m_deviceAudio->setInfoFromCatDevices(map);

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_STREQ("", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_TRUE(m_deviceAudio->m_IsCatDevice);
}

/* ===================== setInfoFromHwinfo ===================== */

// 分支 B3：正常路径 → 属性齐备并返回 true
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromHwinfo_Normal)
{
    // Arrange：hwinfo 标准键；stub executeClientCmd 阻断 driverIsKernelIn 的 modinfo 子进程
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_audio_execCmd_stub);
    ut_audio_execCmdRet = QByteArray();
    QMap<QString, QString> map;
    map.insert("Device", "Cannon Lake PCH cAVS");
    map.insert("Vendor", "Intel Corporation");
    map.insert("Model", "Intel Cannon Lake PCH cAVS");
    map.insert("Driver", "snd_hda_intel");

    // Act
    bool ret = m_deviceAudio->setInfoFromHwinfo(map);

    // Assert：返回 true 且属性赋值
    EXPECT_TRUE(ret);
    EXPECT_STREQ("Cannon Lake PCH cAVS", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceAudio->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Intel Cannon Lake PCH cAVS", m_deviceAudio->m_Model.toStdString().c_str());
    EXPECT_STREQ("snd_hda_intel", m_deviceAudio->m_Driver.toStdString().c_str());
}

// 分支 B1：含 "path" 键 → 禁用态，取小写键并返回 false
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromHwinfo_PathKey_Disabled)
{
    // Arrange：lshw 风格小写键 + path
    QMap<QString, QString> map;
    map.insert("name", "name");
    map.insert("Hardware Class", "Hardware Class");
    map.insert("path", "path");

    // Act
    bool ret = m_deviceAudio->setInfoFromHwinfo(map);

    // Assert：返回 false，禁用态，路径与小写键生效
    EXPECT_FALSE(ret);
    EXPECT_STREQ("name", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_STREQ("path", m_deviceAudio->m_SysPath.toStdString().c_str());
    EXPECT_FALSE(m_deviceAudio->m_Enable);
}

// 分支 B2：内核驱动 → 不可卸载（stub modinfo 无 filename 输出 → 判定为核内驱动）
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromHwinfo_KernelDriver_CannotUninstall)
{
    // Arrange：内核模块驱动 + executeClientCmd 返回空（模拟 modinfo 无 filename）
    Stub stub;
    stub.set(ADDR(Common, executeClientCmd), ut_audio_execCmd_stub);
    ut_audio_execCmdRet = QByteArray();
    QMap<QString, QString> map;
    map.insert("Device", "Audio Device");
    map.insert("Driver", "snd_hda_intel");

    // Act
    bool ret = m_deviceAudio->setInfoFromHwinfo(map);

    // Assert：返回 true 但不可卸载，名称已赋值
    EXPECT_TRUE(ret);
    EXPECT_STREQ("Audio Device", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_FALSE(m_deviceAudio->m_CanUninstall);
}

/* ===================== setInfoFromLshw ===================== */

// 分支 B4：正常路径 → 属性齐备并返回 true
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromLshw_Normal)
{
    // Arrange：lshw 键 + 匹配的 hwinfo 总线标识
    QMap<QString, QString> map;
    map.insert("product", "Cannon Lake PCH cAVS");
    map.insert("vendor", "Intel Corporation");
    map.insert("version", "10");
    map.insert("width", "64 bits");
    map.insert("clock", "33MHz");
    map.insert("description", "Audio device");
    map.insert("bus info", "pci@0000:00:1f.3");
    m_deviceAudio->m_HwinfoToLshw = "0000:00:1f.3";

    // Act
    bool ret = m_deviceAudio->setInfoFromLshw(map);

    // Assert：返回 true 且属性赋值
    EXPECT_TRUE(ret);
    EXPECT_STREQ("Cannon Lake PCH cAVS", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_STREQ("Intel Corporation", m_deviceAudio->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("10", m_deviceAudio->m_Version.toStdString().c_str());
    EXPECT_STREQ("64 bits", m_deviceAudio->m_Width.toStdString().c_str());
    EXPECT_STREQ("Audio device", m_deviceAudio->m_Description.toStdString().c_str());
}

// 分支 B2：vendor 为 "0000" 占位 → 清空
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromLshw_VendorPlaceholder_Cleared)
{
    // Arrange：vendor 为占位 "0000"
    QMap<QString, QString> map;
    map.insert("product", "PCH cAVS");
    map.insert("vendor", "0000");
    map.insert("bus info", "pci@0000:00:1f.3");
    m_deviceAudio->m_HwinfoToLshw = "0000:00:1f.3";

    // Act
    bool ret = m_deviceAudio->setInfoFromLshw(map);

    // Assert：返回 true 且 vendor 清空
    EXPECT_TRUE(ret);
    EXPECT_STREQ("", m_deviceAudio->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("PCH cAVS", m_deviceAudio->m_Name.toStdString().c_str());
}

// 分支 B3：version 为 "00" → 清空
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromLshw_Version00_Cleared)
{
    // Arrange：version 为占位 "00"
    QMap<QString, QString> map;
    map.insert("product", "PCH cAVS");
    map.insert("version", "00");
    map.insert("bus info", "pci@0000:00:1f.3");
    m_deviceAudio->m_HwinfoToLshw = "0000:00:1f.3";

    // Act
    bool ret = m_deviceAudio->setInfoFromLshw(map);

    // Assert：返回 true 且 version 清空
    EXPECT_TRUE(ret);
    EXPECT_STREQ("", m_deviceAudio->m_Version.toStdString().c_str());
    EXPECT_STREQ("PCH cAVS", m_deviceAudio->m_Name.toStdString().c_str());
}

// 分支 B1：总线不匹配 → 返回 false
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromLshw_Mismatch_ReturnsFalse)
{
    // Arrange：hwinfo 标识与 lshw 总线不同
    QMap<QString, QString> map;
    map.insert("product", "Other Device");
    map.insert("bus info", "pci@0000:00:1f.3");
    m_deviceAudio->m_HwinfoToLshw = "0000:99:99.9";

    // Act
    bool ret = m_deviceAudio->setInfoFromLshw(map);

    // Assert：返回 false，名称未被赋值
    EXPECT_FALSE(ret);
    EXPECT_STREQ("", m_deviceAudio->m_Name.toStdString().c_str());
}

/* ===================== setInfoFrom_sysFS ===================== */

// 分支 B1+B5+B6：符号链接 + VID/PID 有效 → 拼接 VID_PID，成功
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFrom_sysFS_Symlink_VidPidPresent)
{
    // Arrange：符号链接 + hwC0D0 目录存在 + 文件可打开 + 假数据
    Stub stub;
    stub.set(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::isSymLink), ut_audio_isSymLink);
    stub.set(static_cast<bool (QDir::*)() const>(&QDir::exists), ut_audio_dirExists);
    stub.set(ut_audio_qfileOpenAddr(), ut_audio_open_stub);
    stub.set(ADDR(DeviceBaseInfo, get_string), ut_audio_getString_stub);
    ut_audio_isSymLinkRet = true;
    ut_audio_dirExistsRet = true;
    ut_audio_fileOpenRet = true;
    ut_audio_stringMap.insert("vendor", "0x8086");
    ut_audio_stringMap.insert("device", "0x0c0c");
    ut_audio_stringMap.insert("vendor_name", "Intel");
    ut_audio_stringMap.insert("chip_name", "HDA PCH");
    ut_audio_stringMap.insert("id", "PCH");
    QMap<QString, QString> mapInfo;

    // Act
    bool ret = m_deviceAudio->setInfoFrom_sysFS(mapInfo, 0);

    // Assert：返回 true，VID_PID 拼接（vid/pid 统一去 0x 前缀，bug 修复后格式）
    EXPECT_TRUE(ret);
    EXPECT_STREQ("80860c0c", m_deviceAudio->m_VID_PID.toStdString().c_str());
    EXPECT_STREQ("Intel HDA PCH", m_deviceAudio->m_Chip.toStdString().c_str());
    EXPECT_STREQ("Intel", m_deviceAudio->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("PCH /card0", m_deviceAudio->m_Name.toStdString().c_str());
    EXPECT_FALSE(m_deviceAudio->m_CanEnable);
    EXPECT_FALSE(m_deviceAudio->m_CanUninstall);
    EXPECT_TRUE(mapInfo.contains("VID_PID"));
    EXPECT_TRUE(mapInfo.contains("chip"));
}

// 分支 B4：VID/PID 为空 → m_VID_PID 取 vendor_id 内容
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFrom_sysFS_VidPidEmpty_UsesVendorId)
{
    // Arrange：device/vendor 读不到，vendor_id 有值
    Stub stub;
    stub.set(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::isSymLink), ut_audio_isSymLink);
    stub.set(static_cast<bool (QDir::*)() const>(&QDir::exists), ut_audio_dirExists);
    stub.set(ut_audio_qfileOpenAddr(), ut_audio_open_stub);
    stub.set(ADDR(DeviceBaseInfo, get_string), ut_audio_getString_stub);
    ut_audio_isSymLinkRet = false;
    ut_audio_dirExistsRet = true;
    ut_audio_fileOpenRet = true;
    ut_audio_stringMap.insert("vendor_id", "0x1234");
    ut_audio_stringMap.insert("vendor_name", "Intel");
    QMap<QString, QString> mapInfo;

    // Act
    bool ret = m_deviceAudio->setInfoFrom_sysFS(mapInfo, 0);

    // Assert：返回 true，VID_PID 即 vendor_id 内容
    EXPECT_TRUE(ret);
    EXPECT_STREQ("0x1234", m_deviceAudio->m_VID_PID.toStdString().c_str());
    EXPECT_TRUE(mapInfo.contains("SysFS ID"));
}

// 分支 B3：无有效 hwC0Dn 目录 → return false
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFrom_sysFS_NoHwDir_ReturnsFalse)
{
    // Arrange：目录不存在 → 循环走到 n==9 早退
    Stub stub;
    stub.set(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::isSymLink), ut_audio_isSymLink);
    stub.set(static_cast<bool (QDir::*)() const>(&QDir::exists), ut_audio_dirExists);
    stub.set(ADDR(DeviceBaseInfo, get_string), ut_audio_getString_stub);
    ut_audio_isSymLinkRet = false;
    ut_audio_dirExistsRet = false;
    QMap<QString, QString> mapInfo;

    // Act
    bool ret = m_deviceAudio->setInfoFrom_sysFS(mapInfo, 0);

    // Assert：返回 false，成员未被赋值
    EXPECT_FALSE(ret);
    EXPECT_STREQ("", m_deviceAudio->m_Chip.toStdString().c_str());
    EXPECT_STREQ("", m_deviceAudio->m_Name.toStdString().c_str());
}

// 分支 B2：hwC0D0 可打开 → break（不再遍历后续编号）
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFrom_sysFS_HwDirFound_BreakLoop)
{
    // Arrange：目录存在且 vendor_id 可打开，n=0 即 break
    Stub stub;
    stub.set(static_cast<bool (QFileInfo::*)() const>(&QFileInfo::isSymLink), ut_audio_isSymLink);
    stub.set(static_cast<bool (QDir::*)() const>(&QDir::exists), ut_audio_dirExists);
    stub.set(ut_audio_qfileOpenAddr(), ut_audio_open_stub);
    stub.set(ADDR(DeviceBaseInfo, get_string), ut_audio_getString_stub);
    ut_audio_isSymLinkRet = false;
    ut_audio_dirExistsRet = true;
    ut_audio_fileOpenRet = true;
    ut_audio_stringMap.insert("vendor", "0x10de");
    ut_audio_stringMap.insert("device", "0x0fbc");
    ut_audio_stringMap.insert("vendor_name", "NVIDIA");
    ut_audio_stringMap.insert("chip_name", "GK208");
    ut_audio_stringMap.insert("id", "HDMI");
    QMap<QString, QString> mapInfo;

    // Act
    bool ret = m_deviceAudio->setInfoFrom_sysFS(mapInfo, 0);

    // Assert：返回 true，芯片取 hwC0D0 的 vendor_name+chip_name，VID_PID 统一去 0x
    EXPECT_TRUE(ret);
    EXPECT_STREQ("NVIDIA GK208", m_deviceAudio->m_Chip.toStdString().c_str());
    EXPECT_STREQ("10de0fbc", m_deviceAudio->m_VID_PID.toStdString().c_str());
}

/* ===================== setInfoFromTomlOneByOne ===================== */

// 等价类：带键赋值
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromTomlOneByOne_WithKeys)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS_Path", "/class/sound/card0");
    mapInfo.insert("KernelModeDriver", "snd_hda_intel");
    mapInfo.insert("Chip", "ALC256");
    mapInfo.insert("IRQ", "16");

    // Act
    TomlFixMethod ret = m_deviceAudio->setInfoFromTomlOneByOne(mapInfo);

    // Assert：成员赋值，已存在的普通值键返回 TOML_Cover
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_STREQ("/class/sound/card0", m_deviceAudio->m_SysPath.toStdString().c_str());
    EXPECT_STREQ("snd_hda_intel", m_deviceAudio->m_Driver.toStdString().c_str());
    EXPECT_STREQ("ALC256", m_deviceAudio->m_Chip.toStdString().c_str());
    EXPECT_STREQ("16", m_deviceAudio->m_Irq.toStdString().c_str());
}

// 边界：空 map → 成员保持空
TEST_F(UT_DeviceAudio, UT_DeviceAudio_setInfoFromTomlOneByOne_EmptyMap)
{
    // Arrange：空 map
    QMap<QString, QString> mapInfo;

    // Act
    TomlFixMethod ret = m_deviceAudio->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_None, ret);
    EXPECT_STREQ("", m_deviceAudio->m_SysPath.toStdString().c_str());
    EXPECT_STREQ("", m_deviceAudio->m_Driver.toStdString().c_str());
    EXPECT_STREQ("", m_deviceAudio->m_Chip.toStdString().c_str());
}

/* ===================== subTitle ===================== */

// 等价类：subTitle 即名称
TEST_F(UT_DeviceAudio, UT_DeviceAudio_subTitle)
{
    // Arrange：设置名称
    QMap<QString, QString> map;
    map.insert("Name", "Cannon Lake PCH cAVS");
    m_deviceAudio->setInfoFromCatAudio(map);

    // Act
    QString value = m_deviceAudio->subTitle();

    // Assert
    EXPECT_STREQ("Cannon Lake PCH cAVS", value.toStdString().c_str());
    EXPECT_EQ(m_deviceAudio->m_Name, value);
}

/* ===================== uniqueID ===================== */

// 等价类：uniqueID 与成员值一致
TEST_F(UT_DeviceAudio, UT_DeviceAudio_uniqueID_Value)
{
    // Arrange：设置 Module Alias
    QMap<QString, QString> map;
    map.insert("Module Alias", "alias-123");
    m_deviceAudio->setInfoFromHwinfo(map);

    // Act
    const QString &uid = m_deviceAudio->uniqueID();

    // Assert
    EXPECT_STREQ("alias-123", uid.toStdString().c_str());
    EXPECT_EQ(m_deviceAudio->m_UniqueID, uid);
}

// 边界：未设置 → 空串
TEST_F(UT_DeviceAudio, UT_DeviceAudio_uniqueID_Empty)
{
    // Arrange：空设备
    ASSERT_NE(m_deviceAudio, nullptr);

    // Act
    const QString &uid = m_deviceAudio->uniqueID();

    // Assert
    EXPECT_STREQ("", uid.toStdString().c_str());
    EXPECT_TRUE(uid.isEmpty());
}

/* ===================== vendor ===================== */

// 等价类：vendor 与成员值一致
TEST_F(UT_DeviceAudio, UT_DeviceAudio_vendor)
{
    // Arrange：设置厂商
    QMap<QString, QString> map;
    map.insert("Vendor", "Intel Corporation");
    m_deviceAudio->setInfoFromCatDevices(map);

    // Act
    const QString &vendor = m_deviceAudio->vendor();

    // Assert
    EXPECT_STREQ("Intel Corporation", vendor.toStdString().c_str());
    EXPECT_EQ(m_deviceAudio->m_Vendor, vendor);
}
