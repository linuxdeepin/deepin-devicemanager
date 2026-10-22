// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Mode 2 重写：DevicePrint 单元测试（旧版断言空洞/缺打桩，评级 D，整体重写）
// 覆盖方法：ctor/setInfo/setInfoFromTomlOneByOne/setEnable/enable/getOverviewInfo/initFilterKey/
//          loadBaseDeviceInfo/loadOtherDeviceInfo/loadTableData/name/vendor/makeAndeModel/subTitle
//
// 用例计数声明（min 为分级下限，actual 为实际用例数，均 ≤ 现有用例规模）
// | method               | level | min | actual |
// | ctor/initFilterKey   | low   |   1 |      1 |
// | setInfo              | low   |   3 |      4 |
// | setInfoFromTomlOneByOne | low |   3 |      3 |
// | setEnable/enable     | low   |   3 |      3 |
// | getOverviewInfo      | low   |   1 |      1 |
// | loadBaseDeviceInfo   | low   |   1 |      1 |
// | loadOtherDeviceInfo  | low   |   1 |      1 |
// | loadTableData        | low   |   2 |      2 |
//
// 打桩依据（dependency-tracer）：
//   - DevicePrint.cpp:118 setEnable 经 DBusEnableInterface::enablePrinter 发起真实 DBus 调用，
//     必须打桩，测试不依赖真实打印服务/DBus 会话
//   - setInfo/setInfoFromTomlOneByOne 为纯内存处理（cups 数据以 QMap 注入），无需其它打桩

#include "DevicePrint.h"
#include "DBusEnableInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

// ===================== 桩函数 =====================
static bool ut_enablePrinter_true()
{
    return true;
}
static bool ut_enablePrinter_false()
{
    return false;
}

static bool ut_containsPair(const QList<QPair<QString, QString>> &lst, const QString &k, const QString &v)
{
    for (const auto &p : lst) {
        if (p.first == k && p.second == v)
            return true;
    }
    return false;
}

class UT_DevicePrint : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_pPrint = new DevicePrint;
    }
    void TearDown() override
    {
        delete m_pPrint;
        m_pPrint = nullptr;
    }
    DevicePrint *m_pPrint = nullptr;
};

// ===================== ctor + initFilterKey =====================
// Branch: DevicePrint.cpp:11-27 成员置空 + initFilterKey 16 键 + m_CanEnable=true/m_CanUninstall=false
TEST_F(UT_DevicePrint, UT_DevicePrint_ctor_default)
{
    // Arrange: SetUp 已构造默认打印机设备（构造即被测路径）
    EXPECT_NE(nullptr, m_pPrint);
    // Act: 读取构造后的初始状态
    const QString name = m_pPrint->name();
    // Assert: 全部基础属性为空
    EXPECT_EQ(QString(""), name);
    EXPECT_EQ(QString(""), m_pPrint->vendor());
    EXPECT_EQ(QString(""), m_pPrint->driver());
    EXPECT_EQ(QString(""), m_pPrint->subTitle());
    EXPECT_EQ(QString(""), m_pPrint->makeAndeModel());
    EXPECT_EQ(QString(""), m_pPrint->m_InterfaceType);
    EXPECT_EQ(QString(""), m_pPrint->m_URI);
    EXPECT_EQ(QString(""), m_pPrint->m_Status);
    // Assert: 状态位：available 恒 true；enable 依赖 m_Status（空非"5"）为 true；可禁用
    EXPECT_TRUE(m_pPrint->available());
    EXPECT_TRUE(m_pPrint->enable());
    EXPECT_TRUE(m_pPrint->m_CanEnable);
    EXPECT_FALSE(m_pPrint->m_CanUninstall);
    // Assert: 过滤键 16 个，含 printer-is-shared / printer-state-change-time，
    //        不含 printer-state / printer-make-and-model（后者被注释）
    EXPECT_EQ(16, m_pPrint->m_FilterKey.size());
    EXPECT_TRUE(m_pPrint->m_FilterKey.contains("printer-is-shared"));
    EXPECT_TRUE(m_pPrint->m_FilterKey.contains("printer-state-change-time"));
    EXPECT_FALSE(m_pPrint->m_FilterKey.contains("printer-state"));
    EXPECT_FALSE(m_pPrint->m_FilterKey.contains("printer-make-and-model"));
    // Assert: 表格数据惰性加载：成员为空 -> ["","",""]；表头含可禁用列 "yes"
    EXPECT_EQ(QStringList() << "" << "" << "", m_pPrint->getTableData());
    EXPECT_EQ(QStringList() << "Name" << "Vendor" << "Model" << "yes", m_pPrint->getTableHeader());
}

// ===================== setInfo =====================
// Branch: DevicePrint.cpp:48-76 printer-info 首词->m_Vendor；Name/device-uri/printer-state/
//         printer-make-and-model 写入；URI 按 ':' 取段->m_InterfaceType；getOtherMapInfo
TEST_F(UT_DevicePrint, UT_DevicePrint_setInfo_cover)
{
    // Arrange: cups 全量属性（printer-info 含空格 -> 厂商取首词）
    QMap<QString, QString> map;
    map.insert("printer-info", "HP LaserJet 1020 Plus");
    map.insert("Name", "HP LaserJet 1020");
    map.insert("device-uri", "usb://HP/LaserJet");
    map.insert("printer-state", "3");
    map.insert("printer-make-and-model", "HP LaserJet 1020");
    map.insert("printer-is-shared", "true");
    // Act
    m_pPrint->setInfo(map);
    // Assert: 厂商/型号按空格拆分
    EXPECT_EQ(QString("HP"), m_pPrint->m_Vendor);
    EXPECT_EQ(QString("LaserJet 1020 Plus"), m_pPrint->m_Model);
    EXPECT_EQ(QString("HP LaserJet 1020"), m_pPrint->name());
    EXPECT_EQ(QString("usb://HP/LaserJet"), m_pPrint->m_URI);
    EXPECT_EQ(QString("usb"), m_pPrint->m_InterfaceType);
    EXPECT_EQ(QString("3"), m_pPrint->m_Status);
    EXPECT_EQ(QString("HP LaserJet 1020"), m_pPrint->makeAndeModel());
    // Assert: 其它信息 = 非空成员项（URI/Status/Interface Type/makeAndModel）
    //         + 过滤键命中项（printer-is-shared）；printer-state/make-and-model 不在过滤键
    const QList<QPair<QString, QString>> &lst = m_pPrint->getOtherAttribs();
    EXPECT_EQ(5, lst.size());
    EXPECT_TRUE(ut_containsPair(lst, "printer-is-shared", "true"));
    EXPECT_TRUE(ut_containsPair(lst, "URI", "usb://HP/LaserJet"));
    EXPECT_TRUE(ut_containsPair(lst, "Status", "3"));
    EXPECT_TRUE(ut_containsPair(lst, "Interface Type", "usb"));
    EXPECT_FALSE(ut_containsPair(lst, "printer-state", "3"));
    // printer-make-and-model 同时是成员项（loadOtherDeviceInfo 采集）
    EXPECT_TRUE(ut_containsPair(lst, "printer-make-and-model", "HP LaserJet 1020"));
}

// Branch: DevicePrint.cpp:67-71 URI 无 ':' -> lstUri.size()==1 -> m_InterfaceType 保持空
TEST_F(UT_DevicePrint, UT_DevicePrint_setInfo_uriNoColon)
{
    // Arrange: URI 不含冒号
    QMap<QString, QString> map;
    map.insert("printer-info", "Canon MP280");
    map.insert("device-uri", "no-colon-uri");
    map.insert("printer-state", "3");
    // Act
    m_pPrint->setInfo(map);
    // Assert: 接口类型为空；厂商型号正常拆分
    EXPECT_EQ(QString(""), m_pPrint->m_InterfaceType);
    EXPECT_EQ(QString("no-colon-uri"), m_pPrint->m_URI);
    EXPECT_EQ(QString("Canon"), m_pPrint->m_Vendor);
    EXPECT_EQ(QString("MP280"), m_pPrint->m_Model);
}

// Branch: DevicePrint.cpp:51-57 printer-info 无空格 -> lstInfo.size()==1 -> 不拆厂商型号
TEST_F(UT_DevicePrint, UT_DevicePrint_setInfo_singleWord)
{
    // Arrange: printer-info 单词
    QMap<QString, QString> map;
    map.insert("printer-info", "Canon");
    // Act
    m_pPrint->setInfo(map);
    // Assert: 厂商/型号均不写入
    EXPECT_EQ(QString(""), m_pPrint->m_Vendor);
    EXPECT_EQ(QString(""), m_pPrint->m_Model);
    EXPECT_EQ(QString(""), m_pPrint->name());
}

// Branch: DeviceBaseInfo.cpp:936-946 setAttribute overwrite=false：键缺失不覆盖已有值
TEST_F(UT_DevicePrint, UT_DevicePrint_setInfo_retainsOnMissingKey)
{
    // Arrange: 第一次完整设置
    QMap<QString, QString> map1;
    map1.insert("printer-info", "HP LaserJet");
    map1.insert("Name", "HP LaserJet");
    map1.insert("device-uri", "usb://HP/LJ");
    m_pPrint->setInfo(map1);
    // Act: 第二次不提供 Name/device-uri（键缺失）
    QMap<QString, QString> map2;
    map2.insert("printer-info", "HP LaserJet");
    m_pPrint->setInfo(map2);
    // Assert: 已有成员保持不变
    EXPECT_EQ(QString("HP LaserJet"), m_pPrint->name());
    EXPECT_EQ(QString("usb://HP/LJ"), m_pPrint->m_URI);
    EXPECT_EQ(QString("HP"), m_pPrint->m_Vendor);
}

// ===================== setInfoFromTomlOneByOne =====================
// Branch: DevicePrint.cpp:29-44 Model/Serial Number/Shared/URI/Status/Interface Type 写入,
//         ret 取 printer-make-and-model 结果; 43 getOtherMapInfo
TEST_F(UT_DevicePrint, UT_DevicePrint_setInfoFromToml_cover)
{
    // Arrange: toml 全量属性
    QMap<QString, QString> map;
    map.insert("Model", "LaserJet 1020");
    map.insert("Serial Number", "SN001");
    map.insert("Shared", "Y");
    map.insert("URI", "usb://HP/LJ");
    map.insert("Status", "3");
    map.insert("Interface Type", "usb");
    map.insert("printer-make-and-model", "HP LaserJet 1020");
    // Act
    TomlFixMethod ret = m_pPrint->setInfoFromTomlOneByOne(map);
    // Assert: 返回最后一个属性处理结果；成员逐项写入
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_EQ(QString("LaserJet 1020"), m_pPrint->m_Model);
    EXPECT_EQ(QString("SN001"), m_pPrint->m_SerialNumber);
    EXPECT_EQ(QString("Y"), m_pPrint->m_Shared);
    EXPECT_EQ(QString("usb://HP/LJ"), m_pPrint->m_URI);
    EXPECT_EQ(QString("3"), m_pPrint->m_Status);
    EXPECT_EQ(QString("usb"), m_pPrint->m_InterfaceType);
    EXPECT_EQ(QString("HP LaserJet 1020"), m_pPrint->m_MakeAndModel);
    // Assert: 过滤键不含 toml 键 -> 其它信息仅成员项（5 项，空值跳过）
    EXPECT_EQ(5, m_pPrint->getOtherAttribs().size());
}

// Branch: DeviceInfo.cpp:988-998 键缺失/值为空 -> TOML_None
TEST_F(UT_DevicePrint, UT_DevicePrint_setInfoFromToml_none)
{
    // Arrange: 缺 printer-make-and-model 键
    QMap<QString, QString> mapNoKey;
    mapNoKey.insert("Model", "LaserJet 1020");
    // Act: 缺键调用
    TomlFixMethod ret1 = m_pPrint->setInfoFromTomlOneByOne(mapNoKey);
    // Assert: 其余属性写入，ret 为 TOML_None
    EXPECT_EQ(TOML_None, ret1);
    EXPECT_EQ(QString("LaserJet 1020"), m_pPrint->m_Model);
    EXPECT_EQ(QString(""), m_pPrint->m_MakeAndModel);

    // Arrange: printer-make-and-model 为空值
    QMap<QString, QString> mapEmpty;
    mapEmpty.insert("Model", "LaserJet 1020");
    mapEmpty.insert("printer-make-and-model", "");
    // Act: 空值调用
    TomlFixMethod ret2 = m_pPrint->setInfoFromTomlOneByOne(mapEmpty);
    // Assert
    EXPECT_EQ(TOML_None, ret2);
    EXPECT_EQ(QString(""), m_pPrint->m_MakeAndModel);
}

// Branch: DeviceInfo.cpp:1000-1003 值含 nouse -> 清空 m_MakeAndModel 返回 TOML_nouse
TEST_F(UT_DevicePrint, UT_DevicePrint_setInfoFromToml_nouse)
{
    // Arrange
    m_pPrint->m_MakeAndModel = "HP LaserJet 1020";
    QMap<QString, QString> map;
    map.insert("printer-make-and-model", "NOUSE");
    // Act
    TomlFixMethod ret = m_pPrint->setInfoFromTomlOneByOne(map);
    // Assert
    EXPECT_EQ(TOML_nouse, ret);
    EXPECT_EQ(QString(""), m_pPrint->m_MakeAndModel);
}

// ===================== setEnable / enable =====================
// Branch: DevicePrint.cpp:117-126 enablePrinter 打桩成功 -> m_Enable 更新，返回 EDS_Success
TEST_F(UT_DevicePrint, UT_DevicePrint_setEnable_success)
{
    // Arrange: 打桩 DBus 成功
    m_pPrint->m_Enable = true;
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enablePrinter), ut_enablePrinter_true);
    // Act: 禁用打印机
    const EnableDeviceStatus r1 = m_pPrint->setEnable(false);
    // Assert: 禁用成功，m_Enable 同步为 false
    EXPECT_EQ(EDS_Success, r1);
    EXPECT_FALSE(m_pPrint->m_Enable);
    // Act: 重新启用
    const EnableDeviceStatus r2 = m_pPrint->setEnable(true);
    // Assert: 启用成功且无副作用（其它信息不变）
    EXPECT_EQ(EDS_Success, r2);
    EXPECT_TRUE(m_pPrint->m_Enable);
    EXPECT_EQ(0, m_pPrint->getOtherAttribs().size());
}

// Branch: DevicePrint.cpp:121-126 enablePrinter 打桩失败 -> m_Enable 不变，返回 EDS_Faild
TEST_F(UT_DevicePrint, UT_DevicePrint_setEnable_fail)
{
    // Arrange: 打桩 DBus 失败
    m_pPrint->m_Enable = true;
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enablePrinter), ut_enablePrinter_false);
    // Act: 尝试禁用
    const EnableDeviceStatus r1 = m_pPrint->setEnable(false);
    // Assert: 禁用失败，m_Enable 不变
    EXPECT_EQ(EDS_Faild, r1);
    EXPECT_TRUE(m_pPrint->m_Enable);
    // Act: 尝试启用
    const EnableDeviceStatus r2 = m_pPrint->setEnable(true);
    // Assert: 仍失败且无副作用
    EXPECT_EQ(EDS_Faild, r2);
    EXPECT_TRUE(m_pPrint->m_Enable);
    EXPECT_EQ(0, m_pPrint->getOtherAttribs().size());
}

// Branch: DevicePrint.cpp:128-132 enable() = m_Status == "5" ? false : true
TEST_F(UT_DevicePrint, UT_DevicePrint_enable_byStatus)
{
    // Arrange: 状态 5（停用）
    m_pPrint->m_Status = "5";
    // Act: 读取可用状态
    const bool disabled = m_pPrint->enable();
    // Assert
    EXPECT_EQ(QString("5"), m_pPrint->m_Status);
    EXPECT_FALSE(disabled);

    // Arrange: 状态 3（空闲）
    m_pPrint->m_Status = "3";
    // Act
    const bool idle = m_pPrint->enable();
    // Assert: 非 5 即可用
    EXPECT_TRUE(idle);
}

// ===================== getOverviewInfo / subTitle =====================
// Branch: DevicePrint.cpp:103-109 m_Name 为空 -> m_Model；非空 -> m_Name
TEST_F(UT_DevicePrint, UT_DevicePrint_getOverviewInfo)
{
    // Arrange: 名称为空、型号有值
    m_pPrint->m_Name = "";
    m_pPrint->m_Model = "LaserJet 1020";
    // Act: 空名称时概况
    const QString overviewEmptyName = m_pPrint->getOverviewInfo();
    // Assert: 回退返回型号
    EXPECT_EQ(QString("LaserJet 1020"), overviewEmptyName);

    // Arrange: 名称非空
    m_pPrint->m_Name = "HP LaserJet 1020";
    // Act: 概况与副标题
    const QString overview = m_pPrint->getOverviewInfo();
    const QString sub = m_pPrint->subTitle();
    // Assert: 返回名称
    EXPECT_EQ(QString("HP LaserJet 1020"), overview);
    EXPECT_EQ(QString("HP LaserJet 1020"), sub);
}

// ===================== loadBaseDeviceInfo =====================
// Branch: DevicePrint.cpp:150-157 Name/Model/Vendor/Serial Number 四项;
//         DeviceInfo.cpp:896-904 空值跳过
TEST_F(UT_DevicePrint, UT_DevicePrint_loadBaseDeviceInfo)
{
    // Arrange: Serial Number 留空验证空值过滤
    m_pPrint->m_Name = "HP LaserJet 1020";
    m_pPrint->m_Model = "LaserJet 1020";
    m_pPrint->m_Vendor = "HP";
    // Act: 序列号为空时读取
    const QList<QPair<QString, QString>> &lst3 = m_pPrint->getBaseAttribs();
    // Assert: 仅非空项
    EXPECT_EQ(3, lst3.size());
    EXPECT_TRUE(ut_containsPair(lst3, "Name", "HP LaserJet 1020"));
    EXPECT_TRUE(ut_containsPair(lst3, "Model", "LaserJet 1020"));
    EXPECT_TRUE(ut_containsPair(lst3, "Vendor", "HP"));
    EXPECT_FALSE(ut_containsPair(lst3, "Serial Number", ""));

    // Arrange: 序列号非空
    m_pPrint->m_SerialNumber = "SN001";
    // Act: 再次读取
    const QList<QPair<QString, QString>> &lst4 = m_pPrint->getBaseAttribs();
    // Assert: 四项齐全（getBaseAttribs 每次清空重建，不重复）
    EXPECT_EQ(4, lst4.size());
    EXPECT_TRUE(ut_containsPair(lst4, "Serial Number", "SN001"));
}

// ===================== loadOtherDeviceInfo =====================
// Branch: DevicePrint.cpp:159-172 Shared/URI/Status/Interface Type/printer-make-and-model
//         + mapInfoToList()（isValueValid 过滤空/Unknown/N/A）
TEST_F(UT_DevicePrint, UT_DevicePrint_loadOtherDeviceInfo)
{
    // Arrange: Shared 留空跳过；URI 经 map 注入 N/A 无效应被丢弃；
    //         printer-state-change-time 在过滤键内但值无效
    m_pPrint->m_Status = "3";
    m_pPrint->m_InterfaceType = "usb";
    m_pPrint->m_MakeAndModel = "HP LaserJet 1020";
    QMap<QString, QString> map;
    map.insert("device-uri", "N/A");
    map.insert("printer-state-change-time", "Unknown");
    map.insert("printer-is-shared", "true");
    m_pPrint->setInfo(map);
    // Act
    const QList<QPair<QString, QString>> &lst = m_pPrint->getOtherAttribs();
    // Assert: 成员项 URI(N/A)/Status/Interface Type/makeAndModel 4 项
    //         （addOtherDeviceInfo 只过滤空值，N/A 保留；mapInfoToList 仅过滤 map 项）
    //         + map 有效项 printer-is-shared 1 项 = 5
    EXPECT_EQ(5, lst.size());
    EXPECT_TRUE(ut_containsPair(lst, "Status", "3"));
    EXPECT_TRUE(ut_containsPair(lst, "Interface Type", "usb"));
    EXPECT_TRUE(ut_containsPair(lst, "printer-is-shared", "true"));
    EXPECT_TRUE(ut_containsPair(lst, "URI", "N/A"));
    // Assert: map 无效值（Unknown state-change-time）经 isValueValid 丢弃
    EXPECT_FALSE(ut_containsPair(lst, "printer-state-change-time", "Unknown"));
    EXPECT_FALSE(ut_containsPair(lst, "Shared", ""));
    // Assert: 重复读取幂等
    EXPECT_EQ(5, m_pPrint->getOtherAttribs().size());
}

// ===================== loadTableData =====================
// Branch: DevicePrint.cpp:174-186 enable() 为真 -> 原始名称 [name, vendor, model]
TEST_F(UT_DevicePrint, UT_DevicePrint_loadTableData_enable)
{
    // Arrange
    m_pPrint->m_Name = "HP LaserJet 1020";
    m_pPrint->m_Vendor = "HP";
    m_pPrint->m_Model = "LaserJet 1020";
    m_pPrint->m_Status = "3";
    // Act
    const QStringList &data = m_pPrint->getTableData();
    const QStringList &header = m_pPrint->getTableHeader();
    // Assert: 数据与表头（m_CanEnable=true -> "yes" 列）
    EXPECT_EQ(QStringList() << "HP LaserJet 1020" << "HP" << "LaserJet 1020", data);
    EXPECT_EQ(QStringList() << "Name" << "Vendor" << "Model" << "yes", header);
}

// Branch: DevicePrint.cpp:178-181 enable() 为假（m_Status=="5"）-> "(Disable) name" 前缀
TEST_F(UT_DevicePrint, UT_DevicePrint_loadTableData_disable)
{
    // Arrange: 状态 5 -> enable() 为假
    m_pPrint->m_Name = "Canon MP280";
    m_pPrint->m_Vendor = "Canon";
    m_pPrint->m_Model = "MP280";
    m_pPrint->m_Status = "5";
    // Act
    const QStringList &data = m_pPrint->getTableData();
    // Assert: Disable 前缀（无翻译器 -> translateStr 返回源文本）
    EXPECT_EQ(QStringList() << "(Disable) Canon MP280" << "Canon" << "MP280", data);
    EXPECT_FALSE(m_pPrint->enable());
}
