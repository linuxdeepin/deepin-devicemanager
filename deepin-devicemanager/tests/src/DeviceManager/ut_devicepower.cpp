// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Mode 2 重写：DevicePower 单元测试（旧版 LOW_ASSERT/SOLE_BOOL_ASSERT/无分支映射，评级 D，整体重写）
// 覆盖方法：ctor/initFilterKey/setInfoFromUpower/setInfoFromTomlOneByOne/setDaemonInfo/
//          name/vendor/driver/available/subTitle/getOverviewInfo/loadBaseDeviceInfo/
//          loadOtherDeviceInfo/loadTableData（inventory: high=ctor,initFilterKey; mid=setInfoFromUpower）
//
// 用例计数声明（min 为分级下限，actual 为实际用例数，均 ≤ 现有用例规模）
// | method                  | level | min | actual |
// | ctor                    | high  |   1 |      1 |
// | initFilterKey           | high  |   2 |      2 |
// | setInfoFromUpower       | mid   |   4 |      4 |
// | setInfoFromTomlOneByOne | low   |   3 |      3 |
// | setDaemonInfo           | low   |   2 |      2 |
// | getOverviewInfo         | low   |   1 |      1 |
// | getters/subTitle        | low   |   1 |      1 |
// | available               | low   |   1 |      1 |
// | loadBaseDeviceInfo      | low   |   1 |      1 |
// | loadOtherDeviceInfo     | low   |   1 |      1 |
// | loadTableData           | low   |   1 |      1 |
//
// 打桩依据（dependency-tracer）：
//   - Common::specialVendorType → boardVendorType → checkBoardVendorFlag → QProcess("dmidecode")
//     真实外部进程/文件访问，必须打桩（DevicePower 构造路径必经 initFilterKey）
//   - QDBusInterface::isValid/property（setInfoFromUpower percentage 分支）打桩，
//     避免依赖真实 system bus（QDBusInterface::call/asyncCall 已由 test_main.cpp 进程级打桩）
//   - 其余被测逻辑为纯内存处理，无外部依赖

#include "DevicePower.h"
#include "commonfunction.h"
#include "ut_Head.h"
#include "stub.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QVariant>
#include <gtest/gtest.h>

// ===================== 桩函数 =====================
// 正常机型路径：不等于 specialHString()（"HUAWEI"），走添加 energy-full 分支
static QString ut_power_vendor_normal()
{
    return QString("normalmagical");
}
// 华为机型路径：等于 specialHString()，跳过 energy-full 分支
static QString ut_power_vendor_huawei()
{
    return QString("HUAWEI");
}
// DBus 接口不可用（percentage 分支 qCritical 路径）
static bool ut_dbi_isValid_false()
{
    return false;
}
// DBus 接口可用（percentage 分支读取 BatteryPercentage 路径）
static bool ut_dbi_isValid_true()
{
    return true;
}
// property("BatteryPercentage") 固定返回 88.6%（88.6 -> int 88 -> "88%"）
static QVariant ut_dbi_property_88(QDBusInterface * /*self*/, const char * /*name*/)
{
    return QVariant(88.6);
}

static bool ut_containsPair(const QList<QPair<QString, QString>> &lst, const QString &k, const QString &v)
{
    for (const auto &p : lst) {
        if (p.first == k && p.second == v)
            return true;
    }
    return false;
}

class UT_DevicePower : public ::testing::Test
{
public:
    void SetUp() override
    {
        // 构造路径 initFilterKey 依赖 Common::specialVendorType（dmidecode 外部进程），打桩后构造
        Stub stub;
        stub.set(ADDR(Common, specialVendorType), ut_power_vendor_normal);
        m_pPower = new DevicePower;
    }
    void TearDown() override
    {
        delete m_pPower;
        m_pPower = nullptr;
    }
    DevicePower *m_pPower = nullptr;
};

// ===================== ctor + initFilterKey =====================
// Branch: DevicePower.cpp:15-42 ctor 成员初始化 + initFilterKey();
//         DevicePower.cpp:176-208 initFilterKey 全部分支（type != specialHString -> addFilterKey("energy-full"/"energy-full-design")）
TEST_F(UT_DevicePower, UT_DevicePower_ctor_default)
{
    // Arrange: SetUp 中以 normalmagical 机型构造完成（构造即被测路径）
    EXPECT_NE(nullptr, m_pPower);
    // Act: 读取构造后的初始状态
    const QString name = m_pPower->name();
    const QString vendor = m_pPower->vendor();
    // Assert: 基础属性为空，状态位为基类默认值
    EXPECT_EQ(QString(""), name);
    EXPECT_EQ(QString(""), vendor);
    EXPECT_EQ(QString(""), m_pPower->driver());
    EXPECT_EQ(QString(""), m_pPower->subTitle());
    EXPECT_TRUE(m_pPower->available());      // DevicePower::available 覆写恒 true（DevicePower.cpp:157-161）
    EXPECT_TRUE(m_pPower->enable());         // 基类 m_Enable 默认 true（DeviceInfo.h:541）
    EXPECT_FALSE(m_pPower->canEnable());     // 基类默认 false，DevicePower 未置位
    EXPECT_FALSE(m_pPower->canUninstall());
    // Assert: initFilterKey 正常机型分支：包含 energy-full / energy-full-design，共 19 个过滤键
    EXPECT_TRUE(m_pPower->m_FilterKey.contains("power supply"));
    EXPECT_TRUE(m_pPower->m_FilterKey.contains("energy-full"));
    EXPECT_TRUE(m_pPower->m_FilterKey.contains("energy-full-design"));
    EXPECT_TRUE(m_pPower->m_FilterKey.contains("energy-rate"));
    EXPECT_EQ(19, m_pPower->m_FilterKey.size());
}

// Branch: DevicePower.cpp:180-181 if (type != Common::specialHString()) 为假 -> 不添加 energy-full 两键
TEST_F(UT_DevicePower, UT_DevicePower_initFilterKey_huawei)
{
    // Arrange: 华为机型（specialVendorType == specialHString == "HUAWEI"）
    DevicePower *p = nullptr;
    {
        Stub stub;
        stub.set(ADDR(Common, specialVendorType), ut_power_vendor_huawei);
        // Act: 构造触发 initFilterKey
        p = new DevicePower;
    }
    // Assert: 华为分支跳过 energy-full / energy-full-design，其余键齐全，共 17 个
    EXPECT_FALSE(p->m_FilterKey.contains("energy-full"));
    EXPECT_FALSE(p->m_FilterKey.contains("energy-full-design"));
    EXPECT_TRUE(p->m_FilterKey.contains("energy-rate"));
    EXPECT_TRUE(p->m_FilterKey.contains("energy"));
    EXPECT_EQ(17, p->m_FilterKey.size());
    delete p;
}

// ===================== setInfoFromUpower =====================
// Branch: DevicePower.cpp:77-83 if(Device.contains("line_power")||state.contains("empty")||state.contains("unknown")) -> false
TEST_F(UT_DevicePower, UT_DevicePower_setInfoFromUpower_linePower)
{
    // Arrange: 三类拒绝输入（line_power / state=empty / state=unknown）
    QMap<QString, QString> mapLine;
    mapLine.insert("Device", "line_power");
    QMap<QString, QString> mapEmpty;
    mapEmpty.insert("Device", "BAT0");
    mapEmpty.insert("state", "empty");
    QMap<QString, QString> mapUnknown;
    mapUnknown.insert("Device", "BAT0");
    mapUnknown.insert("state", "unknown");
    QMap<QString, QString> mapMixedCase;
    mapMixedCase.insert("Device", "LINE_POWER");
    // Act: 逐一调用
    const bool r1 = m_pPower->setInfoFromUpower(mapLine);
    const bool r2 = m_pPower->setInfoFromUpower(mapEmpty);
    const bool r3 = m_pPower->setInfoFromUpower(mapUnknown);
    const bool r4 = m_pPower->setInfoFromUpower(mapMixedCase);
    // Assert: 全部拒绝
    EXPECT_FALSE(r1);
    EXPECT_FALSE(r2);
    EXPECT_FALSE(r3);
    EXPECT_FALSE(r4);
    // Assert: 拒绝路径不写入任何成员
    EXPECT_EQ(QString(""), m_pPower->m_SerialNumber);
    EXPECT_EQ(QString(""), m_pPower->m_Capacity);
    EXPECT_EQ(0, m_pPower->getOtherAttribs().size());
}

// Branch: DevicePower.cpp:88-108 setAttribute(serial/capacity/voltage/temperature) 写入;
//         109-125 tmpMapInfo 无 percentage -> 不进 DBus 分支; 128 return true
TEST_F(UT_DevicePower, UT_DevicePower_setInfoFromUpower_battery)
{
    // Arrange: 正常电池信息（无 percentage，避免 DBus 依赖）
    QMap<QString, QString> map;
    map.insert("Device", "BAT0");
    map.insert("serial", "SER123456");
    map.insert("capacity", "85%");
    map.insert("voltage", "12.5V");
    map.insert("temperature", "28 degrees C");
    map.insert("state", "not-charging");
    // Act
    const bool ret = m_pPower->setInfoFromUpower(map);
    // Assert: 接受并返回 true
    EXPECT_TRUE(ret);
    // Assert: 上行属性逐项写入
    EXPECT_EQ(QString("SER123456"), m_pPower->m_SerialNumber);
    EXPECT_EQ(QString("85%"), m_pPower->m_Capacity);
    EXPECT_EQ(QString("12.5V"), m_pPower->m_Voltage);
    EXPECT_EQ(QString("28 degrees C"), m_pPower->m_Temp);
    // Assert: m_Name 不经 upower 设置，保持为空
    EXPECT_EQ(QString(""), m_pPower->name());
    // Assert: "state" 在过滤键内 -> 进入其它信息；serial/capacity 不在
    EXPECT_TRUE(ut_containsPair(m_pPower->getOtherAttribs(), "state", "not-charging"));
    EXPECT_FALSE(ut_containsPair(m_pPower->getOtherAttribs(), "serial", "SER123456"));
    EXPECT_EQ(1, m_pPower->getOtherAttribs().size());
}

// Branch: DevicePower.cpp:106-113 if(percentage){ if(!interface.isValid()) qCritical -> 保留原值 }
TEST_F(UT_DevicePower, UT_DevicePower_setInfoFromUpower_percentage_noDBus)
{
    // Arrange: percentage 存在且 DBus 不可用
    QMap<QString, QString> map;
    map.insert("Device", "BAT0");
    map.insert("percentage", "75%");
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_dbi_isValid_false);
    // Act
    const bool ret = m_pPower->setInfoFromUpower(map);
    // Assert: DBus 无效分支保留 upower 上报的原始 percentage
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ut_containsPair(m_pPower->getOtherAttribs(), "percentage", "75%"));
    EXPECT_EQ(1, m_pPower->getOtherAttribs().size());
}

// Branch: DevicePower.cpp:108-112 if(interface.isValid()) property("BatteryPercentage") -> int 转百分比
TEST_F(UT_DevicePower, UT_DevicePower_setInfoFromUpower_percentage_dBus)
{
    // Arrange: percentage 存在且 DBus 可用，property 返回 88.6 -> "88%"
    QMap<QString, QString> map;
    map.insert("Device", "BAT0");
    map.insert("percentage", "75%");
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_dbi_isValid_true);
    stub.set(ADDR(QDBusInterface, property), ut_dbi_property_88);
    // Act
    const bool ret = m_pPower->setInfoFromUpower(map);
    // Assert: DBus 读取值 88.6 截断为 88 覆盖原 75%
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ut_containsPair(m_pPower->getOtherAttribs(), "percentage", "88%"));
    EXPECT_FALSE(ut_containsPair(m_pPower->getOtherAttribs(), "percentage", "75%"));
    EXPECT_EQ(1, m_pPower->getOtherAttribs().size());
}

// ===================== setInfoFromTomlOneByOne =====================
// Branch: DevicePower.cpp:51-73 全部 setTomlAttribute 命中 + ret 取 Temperature 结果 TOML_Cover
TEST_F(UT_DevicePower, UT_DevicePower_setInfoFromToml_cover)
{
    // Arrange: toml 全量属性（Temperature 存在 -> 返回 TOML_Cover）
    QMap<QString, QString> map;
    map.insert("Model", "BAT-CR40");
    map.insert("Serial Number", "SN001");
    map.insert("Type", "Battery");
    map.insert("Status", "Good");
    map.insert("Capacity", "4200mAh");
    map.insert("Voltage", "11.4V");
    map.insert("Slot", "Primary");
    map.insert("Design Capacity", "5000mAh");
    map.insert("Design Voltage", "11.4V");
    map.insert("SBDS Version", "1.0");
    map.insert("SBDS Serial Number", "SBD001");
    map.insert("SBDS Manufacture Date", "2020-01-01");
    map.insert("SBDS Chemistry", "LION");
    map.insert("Temperature", "50 degrees C");
    // Act
    TomlFixMethod ret = m_pPower->setInfoFromTomlOneByOne(map);
    // Assert: 返回最后一个属性 Temperature 的处理结果
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_EQ(QString("BAT-CR40"), m_pPower->m_Model);
    EXPECT_EQ(QString("SN001"), m_pPower->m_SerialNumber);
    EXPECT_EQ(QString("Battery"), m_pPower->m_Type);
    EXPECT_EQ(QString("Good"), m_pPower->m_Status);
    EXPECT_EQ(QString("4200mAh"), m_pPower->m_Capacity);
    EXPECT_EQ(QString("11.4V"), m_pPower->m_Voltage);
    EXPECT_EQ(QString("Primary"), m_pPower->m_Slot);
    EXPECT_EQ(QString("5000mAh"), m_pPower->m_DesignCapacity);
    EXPECT_EQ(QString("11.4V"), m_pPower->m_DesignVoltage);
    EXPECT_EQ(QString("1.0"), m_pPower->m_SBDSVersion);
    EXPECT_EQ(QString("SBD001"), m_pPower->m_SBDSSerialNumber);
    EXPECT_EQ(QString("2020-01-01"), m_pPower->m_SBDSManufactureDate);
    EXPECT_EQ(QString("LION"), m_pPower->m_SBDSChemistry);
    EXPECT_EQ(QString("50 degrees C"), m_pPower->m_Temp);
    // Assert: toml 属性键均不在过滤键内 -> 其它信息为空
    EXPECT_EQ(0, m_pPower->getOtherAttribs().size());
}

// Branch: DeviceBaseInfo.cpp:990-993 key 缺失 -> TOML_None;
//         DeviceBaseInfo.cpp:995-998 值为空 -> TOML_None
TEST_F(UT_DevicePower, UT_DevicePower_setInfoFromToml_none)
{
    // Arrange: 不含 Temperature 键
    QMap<QString, QString> mapNoKey;
    mapNoKey.insert("Model", "BAT-CR40");
    // Act: 缺键调用
    TomlFixMethod ret1 = m_pPower->setInfoFromTomlOneByOne(mapNoKey);
    // Assert: 缺键 -> TOML_None，Temperature 不写入
    EXPECT_EQ(TOML_None, ret1);
    EXPECT_EQ(QString("BAT-CR40"), m_pPower->m_Model);
    EXPECT_EQ(QString(""), m_pPower->m_Temp);

    // Arrange: Temperature 为空值
    QMap<QString, QString> mapEmpty;
    mapEmpty.insert("Model", "BAT-CR40");
    mapEmpty.insert("Temperature", "");
    // Act: 空值调用
    TomlFixMethod ret2 = m_pPower->setInfoFromTomlOneByOne(mapEmpty);
    // Assert: 空值 -> TOML_None
    EXPECT_EQ(TOML_None, ret2);
    EXPECT_EQ(QString(""), m_pPower->m_Temp);
}

// Branch: DeviceBaseInfo.cpp:1000-1003 值含 nouse -> variable.clear() 返回 TOML_nouse
TEST_F(UT_DevicePower, UT_DevicePower_setInfoFromToml_nouse)
{
    // Arrange: 预置 m_Temp 非空，toml Temperature 值含 nouse
    m_pPower->m_Temp = "50 degrees C";
    QMap<QString, QString> map;
    map.insert("Temperature", "NOUSE");   // 大小写不敏感命中 nouse
    // Act
    TomlFixMethod ret = m_pPower->setInfoFromTomlOneByOne(map);
    // Assert: 清空属性值
    EXPECT_EQ(TOML_nouse, ret);
    EXPECT_EQ(QString(""), m_pPower->m_Temp);
}

// ===================== setDaemonInfo =====================
// Branch: DevicePower.cpp:135-136 if (m_Name == translateStr("battery")) -> getOtherMapInfo
TEST_F(UT_DevicePower, UT_DevicePower_setDaemonInfo_battery)
{
    // Arrange: m_Name 置为 battery（translateStr("battery") 无翻译时为 "battery"）
    m_pPower->m_Name = "battery";
    QMap<QString, QString> map;
    map.insert("state", "ok");
    map.insert("percentage", "66%");
    map.insert("serial", "XXX");          // 不在过滤键内，应被丢弃
    // Act
    m_pPower->setDaemonInfo(map);
    // Assert: 过滤键命中项进入其它信息
    EXPECT_TRUE(ut_containsPair(m_pPower->getOtherAttribs(), "state", "ok"));
    EXPECT_TRUE(ut_containsPair(m_pPower->getOtherAttribs(), "percentage", "66%"));
    EXPECT_FALSE(ut_containsPair(m_pPower->getOtherAttribs(), "serial", "XXX"));
    EXPECT_EQ(2, m_pPower->getOtherAttribs().size());
}

// Branch: DevicePower.cpp:135-136 m_Name != "battery" -> 忽略 map
TEST_F(UT_DevicePower, UT_DevicePower_setDaemonInfo_notBattery)
{
    // Arrange: m_Name 保持为空
    m_pPower->m_Name = "";
    QMap<QString, QString> map;
    map.insert("state", "ok");
    // Act
    m_pPower->setDaemonInfo(map);
    // Assert: 非电池设备忽略守护进程信息
    EXPECT_EQ(QString(""), m_pPower->name());
    EXPECT_EQ(0, m_pPower->getOtherAttribs().size());
}

// ===================== 概况 / 副标题 / getter =====================
// Branch: DevicePower.cpp:169-174 DApplication::translate("ManulTrack", m_Name.trimmed())
TEST_F(UT_DevicePower, UT_DevicePower_getOverviewInfo)
{
    // Arrange: 名称带首尾空格
    m_pPower->m_Name = " Battery ";
    // Act
    QString overview = m_pPower->getOverviewInfo();
    // Assert: 先 trim 再翻译（无翻译器 -> 返回源文本）
    EXPECT_EQ(QString("Battery"), overview);
    EXPECT_EQ(QString(" Battery "), m_pPower->name());   // 原成员不被修改
}

// Branch: DevicePower.cpp:163-167 subTitle 返回 m_Name; 139-155 name/vendor/driver getter
TEST_F(UT_DevicePower, UT_DevicePower_getters)
{
    // Arrange: 成员赋值
    m_pPower->m_Name = "Battery";
    m_pPower->m_Vendor = "VendorX";
    m_pPower->m_Driver = "battery";
    // Act: 逐一读取
    const QString name = m_pPower->name();
    const QString vendor = m_pPower->vendor();
    const QString driver = m_pPower->driver();
    const QString sub = m_pPower->subTitle();
    // Assert: 各 getter 返回成员值
    EXPECT_EQ(QString("Battery"), name);
    EXPECT_EQ(QString("VendorX"), vendor);
    EXPECT_EQ(QString("battery"), driver);
    EXPECT_EQ(QString("Battery"), sub);
}

// Branch: DevicePower.cpp:157-161 available() 覆写恒 true（不检查 driver）
TEST_F(UT_DevicePower, UT_DevicePower_available_alwaysTrue)
{
    // Arrange: driver 为空（基类实现会因此返回 false，覆写必须仍为 true）
    m_pPower->m_Driver = "";
    // Act
    const bool avail = m_pPower->available();
    // Assert: 覆写恒真且 driver 保持为空
    EXPECT_TRUE(avail);
    EXPECT_EQ(QString(""), m_pPower->driver());
}

// ===================== loadBaseDeviceInfo =====================
// Branch: DevicePower.cpp:210-230 addBaseDeviceInfo 全键 + Temperature replace("degrees C","℃");
//         DeviceBaseInfo.cpp:896-904 空值不追加
TEST_F(UT_DevicePower, UT_DevicePower_loadBaseDeviceInfo)
{
    // Arrange: Vendor/Serial 留空验证空值过滤；Temperature 带单位触发替换
    m_pPower->m_Name = "Battery";
    m_pPower->m_Model = "BAT-CR40";
    m_pPower->m_Temp = "28 degrees C";
    // Act
    const QList<QPair<QString, QString>> &lst = m_pPower->getBaseAttribs();
    // Assert: 只包含非空项，Temperature 已做单位替换
    EXPECT_TRUE(ut_containsPair(lst, "Name", "Battery"));
    EXPECT_TRUE(ut_containsPair(lst, "Model", "BAT-CR40"));
    EXPECT_TRUE(ut_containsPair(lst, "Temperature", "28 ℃"));
    EXPECT_FALSE(ut_containsPair(lst, "Vendor", ""));
    EXPECT_FALSE(ut_containsPair(lst, "Serial Number", ""));
    EXPECT_EQ(3, lst.size());
    // Assert: m_Temp 被 replace 原地修改
    EXPECT_EQ(QString("28 ℃"), m_pPower->m_Temp);
}

// ===================== loadOtherDeviceInfo =====================
// Branch: DevicePower.cpp:232-237 mapInfoToList();
//         DeviceBaseInfo.cpp:1021-1033 isValueValid 过滤无效值（N/A）
TEST_F(UT_DevicePower, UT_DevicePower_loadOtherDeviceInfo)
{
    // Arrange: energy=N/A 无效应被丢弃；state/warning-level 有效
    QMap<QString, QString> map;
    map.insert("Device", "BAT0");
    map.insert("state", "ok");
    map.insert("warning-level", "good");
    map.insert("energy", "N/A");
    ASSERT_TRUE(m_pPower->setInfoFromUpower(map));
    // Act
    const QList<QPair<QString, QString>> &lst = m_pPower->getOtherAttribs();
    // Assert: 无效值丢弃，有效值保留
    EXPECT_EQ(2, lst.size());
    EXPECT_TRUE(ut_containsPair(lst, "state", "ok"));
    EXPECT_TRUE(ut_containsPair(lst, "warning-level", "good"));
    EXPECT_FALSE(ut_containsPair(lst, "energy", "N/A"));
    // Assert: 重复调用幂等（clear + reload）
    EXPECT_EQ(2, m_pPower->getOtherAttribs().size());
}

// ===================== loadTableData =====================
// Branch: DevicePower.cpp:239-246 m_TableData = [m_Name, m_Vendor, m_Model]
TEST_F(UT_DevicePower, UT_DevicePower_loadTableData)
{
    // Arrange
    m_pPower->m_Name = "Battery";
    m_pPower->m_Vendor = "VendorX";
    m_pPower->m_Model = "BAT-CR40";
    // Act
    const QStringList &data = m_pPower->getTableData();
    const QStringList &header = m_pPower->getTableHeader();
    // Assert: 表格数据按 名称/制造商/型号 顺序
    EXPECT_EQ(QStringList() << "Battery" << "VendorX" << "BAT-CR40", data);
    // Assert: 默认表头（DeviceBaseInfo.cpp:859-866）+ getTableHeader 追加的可禁用列
    //（m_CanEnable=false -> "no"）
    EXPECT_EQ(QStringList() << "Name" << "Vendor" << "Model" << "no", header);
}
