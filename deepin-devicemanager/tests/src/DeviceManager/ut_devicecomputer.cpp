// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Mode 2 重写：DeviceComputer 单元测试（旧版断言空洞/无分支映射，评级 D，整体重写）
// 覆盖方法：ctor/setInfoFromTomlOneByOne/setName(4参)/setVendor(2参)/setHomeUrl/setOsDescription/
//          setOS/setType/getOSInfo/getOverviewInfo/initFilterKey/loadBaseDeviceInfo/
//          loadOtherDeviceInfo/loadTableData/name/vendor/driver/subTitle
// 已知缺陷（只标红不修码）：头文件声明的 setName(const QString&) 与 setVendor(const QString&)
//   单参重载在 DeviceComputer.cpp 中无定义，测试不得调用（链接失败），见缺陷清单
//
// 用例计数声明（min 为分级下限，actual 为实际用例数，均 ≤ 现有用例规模）
// | method                  | level | min | actual |
// | ctor/initFilterKey      | low   |   1 |      1 |
// | setInfoFromTomlOneByOne | low   |   3 |      3 |
// | setName(4参)            | low   |   5 |      5 |
// | setVendor(2参)          | low   |   2 |      2 |
// | setters/getOSInfo       | low   |   2 |      2 |
// | getOverviewInfo         | low   |   1 |      1 |
// | loadBaseDeviceInfo      | low   |   1 |      1 |
// | loadOtherDeviceInfo     | low   |   1 |      1 |
// | loadTableData           | low   |   1 |      1 |
//
// 打桩依据（dependency-tracer）：全类无外部依赖（无 DBus/网络/文件/进程），无需打桩

#include "DeviceComputer.h"
#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

static bool ut_containsPair(const QList<QPair<QString, QString>> &lst, const QString &k, const QString &v)
{
    for (const auto &p : lst) {
        if (p.first == k && p.second == v)
            return true;
    }
    return false;
}

class UT_DeviceComputer : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_pComputer = new DeviceComputer;
    }
    void TearDown() override
    {
        delete m_pComputer;
        m_pComputer = nullptr;
    }
    DeviceComputer *m_pComputer = nullptr;
};

// ===================== ctor =====================
// Branch: DeviceComputer.cpp:15-23 成员初始化为空; initFilterKey 为空实现(191-195) -> m_FilterKey 无键
TEST_F(UT_DeviceComputer, UT_DeviceComputer_ctor_default)
{
    // Arrange: SetUp 已构造默认设备（构造即被测路径）
    EXPECT_NE(nullptr, m_pComputer);
    // Act: 读取构造后的初始状态；直接调用空实现 initFilterKey（白盒，构造器未调用）
    const QString name = m_pComputer->name();
    const QString os = m_pComputer->getOSInfo();
    m_pComputer->initFilterKey();
    // Assert: 全部基础属性为空
    EXPECT_EQ(QString(""), name);
    EXPECT_EQ(QString(""), m_pComputer->vendor());
    EXPECT_EQ(QString(""), m_pComputer->driver());
    EXPECT_EQ(QString(""), m_pComputer->subTitle());
    EXPECT_EQ(QString(""), m_pComputer->m_HomeUrl);
    EXPECT_EQ(QString(""), m_pComputer->m_OsDescription);
    EXPECT_EQ(QString(""), m_pComputer->m_OS);
    EXPECT_EQ(QString(""), m_pComputer->m_Type);
    // Assert: getOSInfo 空串拼接为单个空格
    EXPECT_EQ(QString(" "), os);
    // Assert: 过滤键为空（initFilterKey 空实现）
    EXPECT_TRUE(m_pComputer->m_FilterKey.isEmpty());
    // Assert: 其它信息为空
    EXPECT_EQ(0, m_pComputer->getOtherAttribs().size());
}

// ===================== setInfoFromTomlOneByOne =====================
// Branch: DeviceComputer.cpp:25-42 HOME_URL/Type/vendor/OS/OsDescription 写入;
//         ret 取 OsDescription 结果; 40 getOtherMapInfo 因 m_FilterKey 为空不收集
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setInfoFromToml_cover)
{
    // Arrange: toml 全量属性 + 一个不在任何集合内的干扰键
    QMap<QString, QString> map;
    map.insert("HOME_URL", "https://www.deepin.org");
    map.insert("Type", "Laptop");
    map.insert("vendor", "Uniontech");
    map.insert("OS", "UOS 20");
    map.insert("OsDescription", "deepin 23");
    map.insert("Random", "xyz");
    // Act
    TomlFixMethod ret = m_pComputer->setInfoFromTomlOneByOne(map);
    // Assert
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_EQ(QString("https://www.deepin.org"), m_pComputer->m_HomeUrl);
    EXPECT_EQ(QString("Laptop"), m_pComputer->m_Type);
    EXPECT_EQ(QString("Uniontech"), m_pComputer->m_Vendor);
    EXPECT_EQ(QString("UOS 20"), m_pComputer->m_OS);
    EXPECT_EQ(QString("deepin 23"), m_pComputer->m_OsDescription);
    // Assert: 过滤键为空 -> 其它信息恒为空（Random 也不收集）
    EXPECT_EQ(0, m_pComputer->getOtherAttribs().size());
}

// Branch: DeviceBaseInfo.cpp:990-993 键缺失 -> TOML_None; 995-998 值为空 -> TOML_None
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setInfoFromToml_none)
{
    // Arrange: 缺 OsDescription 键
    QMap<QString, QString> mapNoKey;
    mapNoKey.insert("vendor", "Uniontech");
    // Act: 缺键调用
    TomlFixMethod ret1 = m_pComputer->setInfoFromTomlOneByOne(mapNoKey);
    // Assert: 其余属性写入，OsDescription 缺失 -> TOML_None
    EXPECT_EQ(TOML_None, ret1);
    EXPECT_EQ(QString("Uniontech"), m_pComputer->m_Vendor);
    EXPECT_EQ(QString(""), m_pComputer->m_OsDescription);

    // Arrange: OsDescription 为空值
    QMap<QString, QString> mapEmpty;
    mapEmpty.insert("OsDescription", "");
    // Act: 空值调用
    TomlFixMethod ret2 = m_pComputer->setInfoFromTomlOneByOne(mapEmpty);
    // Assert
    EXPECT_EQ(TOML_None, ret2);
    EXPECT_EQ(QString(""), m_pComputer->m_OsDescription);
}

// Branch: DeviceBaseInfo.cpp:1000-1003 值含 nouse -> 清空 m_OsDescription 返回 TOML_nouse
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setInfoFromToml_nouse)
{
    // Arrange
    m_pComputer->m_OsDescription = "deepin 23";
    QMap<QString, QString> map;
    map.insert("OsDescription", "noUse");
    // Act
    TomlFixMethod ret = m_pComputer->setInfoFromTomlOneByOne(map);
    // Assert
    EXPECT_EQ(TOML_nouse, ret);
    EXPECT_EQ(QString(""), m_pComputer->m_OsDescription);
}

// ===================== setName（4 参重载） =====================
// Branch: DeviceComputer.cpp:127-132 dm1Name 不含 "System Product Name" -> 用 dm1Name;
//         134-144 family 无排除词; 146-155 version 无排除词; 158-163 去重不生效; 165 拼接
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setName_normal)
{
    // Arrange: 先设置 vendor 保证去重阶段 remove 不作用于空串
    m_pComputer->setVendor("Lenovo", "");
    // Act
    m_pComputer->setName("ThinkPad X1", "PC", "Carbon", "Win11");
    // Assert: family + " " + version + " " + pname
    EXPECT_EQ(QString("Lenovo"), m_pComputer->vendor());
    EXPECT_EQ(QString("Carbon Win11 ThinkPad X1"), m_pComputer->m_Name);
}

// Branch: DeviceComputer.cpp:128-126 dm1Name 含 "System Product Name" -> pname = dm2Name
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setName_systemProductName)
{
    // Arrange
    m_pComputer->setVendor("Dell", "");
    // Act: dm1Name 为通用名 -> 回退 dm2Name
    m_pComputer->setName("System Product Name", "OptiPlex 7090", "Precision", "Win11");
    // Assert
    EXPECT_EQ(QString("Dell"), m_pComputer->vendor());
    EXPECT_EQ(QString("Precision Win11 OptiPlex 7090"), m_pComputer->m_Name);
}

// Branch: DeviceComputer.cpp:134-141 family 含排除词(unknown/System Version/Not Specified/x.x/Not Applicable) -> family 置空
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setName_familyExclusion)
{
    // Arrange: 五类排除词 + 混合大小写变体
    m_pComputer->setVendor("Lenovo", "");
    QStringList exclusions;
    exclusions << "unknown" << "System Version" << "Not Specified" << "x.x" << "Not Applicable" << "UNKNOWN-MODEL";
    // Act: 逐一以排除词为 family 调用
    for (const QString &bad : exclusions) {
        m_pComputer->m_Name.clear();
        m_pComputer->setName("ThinkPad X1", "PC", bad, "Win11");
        // Assert: family 被置空，拼接产生前导空格
        EXPECT_EQ(QString(" Win11 ThinkPad X1"), m_pComputer->m_Name) << "family=" << bad.toStdString();
    }
    // Assert: 全部排除词均生效（循环覆盖 6 个等价类）
    EXPECT_EQ(6, exclusions.size());
}

// Branch: DeviceComputer.cpp:143-155 version 含排除词 -> version 置空（family/version 拼接出现连续空格）
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setName_versionExclusion)
{
    // Arrange
    m_pComputer->setVendor("Lenovo", "");
    // Act
    m_pComputer->setName("ThinkPad X1", "PC", "Carbon", "x.x");
    // Assert: version 空串拼接产生双空格（实现不 trim，为现状行为）
    EXPECT_EQ(QString("Lenovo"), m_pComputer->vendor());
    EXPECT_EQ(QString("Carbon  ThinkPad X1"), m_pComputer->m_Name);
}

// Branch: DeviceComputer.cpp:158-163 去重：pname/family 去除 vendor、version 子串（remove 不 trim，保留多余空格）
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setName_dedup)
{
    // Arrange: vendor 为 "Dell"，名称中含 "Dell" 子串
    m_pComputer->setVendor("Dell", "");
    // Act
    m_pComputer->setName("Dell OptiPlex", "PC", "Dell Precision", "Win11");
    // Assert: "Dell" 子串被移除但空格保留
    EXPECT_EQ(QString("Dell"), m_pComputer->vendor());
    EXPECT_EQ(QString(" Precision Win11  OptiPlex"), m_pComputer->m_Name);
}

// ===================== setVendor（2 参重载） =====================
// Branch: DeviceComputer.cpp:112-116 dm1Vendor 含 "System manufacturer" -> m_Vendor = dm2Vendor
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setVendor_dm2)
{
    // Arrange: dm1Vendor 为通用名
    const QString dm1 = "System manufacturer";
    // Act
    m_pComputer->setVendor(dm1, "Dell Inc.");
    // Assert: 回退 dm2Vendor
    EXPECT_EQ(QString("Dell Inc."), m_pComputer->vendor());
    EXPECT_EQ(QString(""), m_pComputer->m_Name);   // vendor 不影响名称
}

// Branch: DeviceComputer.cpp:117-120 dm1Vendor 不含 "System manufacturer" -> m_Vendor = dm1Vendor
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setVendor_dm1)
{
    // Arrange: dm1Vendor 为正常厂商
    const QString dm1 = "Lenovo";
    // Act
    m_pComputer->setVendor(dm1, "Dell Inc.");
    // Assert: dm2Vendor 不生效
    EXPECT_EQ(QString("Lenovo"), m_pComputer->vendor());
    EXPECT_EQ(QString(""), m_pComputer->m_Name);
}

// ===================== 简单 setter / getOSInfo =====================
// Branch: DeviceComputer.cpp:62-81 setHomeUrl/setOsDescription/setOS/setType 直接赋值;
//         183-189 getOSInfo = m_OsDescription + " " + m_OS
TEST_F(UT_DeviceComputer, UT_DeviceComputer_setters_getOSInfo)
{
    // Arrange: 初始为空（空成员 -> getOSInfo 为单个空格）
    EXPECT_EQ(QString(" "), m_pComputer->getOSInfo());
    // Act
    m_pComputer->setHomeUrl("https://www.deepin.org");
    m_pComputer->setOsDescription("deepin 23");
    m_pComputer->setOS("UOS 20");
    m_pComputer->setType("Laptop");
    // Assert
    EXPECT_EQ(QString("https://www.deepin.org"), m_pComputer->m_HomeUrl);
    EXPECT_EQ(QString("Laptop"), m_pComputer->m_Type);
    EXPECT_EQ(QString("deepin 23 UOS 20"), m_pComputer->getOSInfo());
}

// Branch: DeviceComputer.cpp:185 m_OsDescription/m_OS 均为空 -> getOSInfo 为单个空格
TEST_F(UT_DeviceComputer, UT_DeviceComputer_getOSInfo_empty)
{
    // Arrange: 成员均为空
    EXPECT_EQ(QString(""), m_pComputer->m_OsDescription);
    EXPECT_EQ(QString(""), m_pComputer->m_OS);
    // Act
    const QString os = m_pComputer->getOSInfo();
    // Assert: 空串 + 空格 + 空串
    EXPECT_EQ(QString(" "), os);
}

// ===================== getOverviewInfo =====================
// Branch: DeviceComputer.cpp:170-181 model = vendor + " " + name + " " + type + " "（尾部空格）
TEST_F(UT_DeviceComputer, UT_DeviceComputer_getOverviewInfo)
{
    // Arrange
    m_pComputer->setVendor("Uniontech", "");
    m_pComputer->m_Name = "PC-2020";
    m_pComputer->setType("Laptop");
    // Act
    const QString overview = m_pComputer->getOverviewInfo();
    const QString sub = m_pComputer->subTitle();
    // Assert: 尾部保留空格；DeviceComputer 未重写 subTitle，基类返回空串
    EXPECT_EQ(QString("Uniontech PC-2020 Laptop "), overview);
    EXPECT_EQ(QString(""), sub);
}

// ===================== loadBaseDeviceInfo =====================
// Branch: DeviceComputer.cpp:197-202 仅 addBaseDeviceInfo("Name", m_Name);
//         DeviceBaseInfo.cpp:896-904 m_Name 为空时不追加
TEST_F(UT_DeviceComputer, UT_DeviceComputer_loadBaseDeviceInfo)
{
    // Arrange: 名称为空
    m_pComputer->m_Name = "";
    // Act: 空名称时读取
    const QList<QPair<QString, QString>> &lstEmpty = m_pComputer->getBaseAttribs();
    // Assert: 基本信息为空列表
    EXPECT_EQ(0, lstEmpty.size());

    // Arrange: 设置名称
    m_pComputer->m_Name = "PC-2020";
    // Act: 非空名称时读取
    const QList<QPair<QString, QString>> &lst = m_pComputer->getBaseAttribs();
    // Assert: 仅 Name 一项
    EXPECT_EQ(1, lst.size());
    EXPECT_TRUE(ut_containsPair(lst, "Name", "PC-2020"));
}

// ===================== loadOtherDeviceInfo =====================
// Branch: DeviceComputer.cpp:204-209 mapInfoToList(); m_MapOtherInfo 恒空（过滤键为空）
TEST_F(UT_DeviceComputer, UT_DeviceComputer_loadOtherDeviceInfo)
{
    // Arrange: setInfoFromTomlOneByOne 后再取其它信息
    QMap<QString, QString> map;
    map.insert("vendor", "Uniontech");
    map.insert("Random", "xyz");
    m_pComputer->setInfoFromTomlOneByOne(map);
    // Act
    const QList<QPair<QString, QString>> &lst = m_pComputer->getOtherAttribs();
    // Assert: 过滤键为空 -> 无任何其它信息
    EXPECT_EQ(0, lst.size());
    EXPECT_FALSE(ut_containsPair(lst, "Random", "xyz"));
}

// ===================== loadTableData =====================
// Branch: DeviceComputer.cpp:211-215 空实现 -> m_TableData 恒空
TEST_F(UT_DeviceComputer, UT_DeviceComputer_loadTableData)
{
    // Arrange: 即使成员有值
    m_pComputer->m_Name = "PC-2020";
    m_pComputer->setVendor("Uniontech", "");
    m_pComputer->setType("Laptop");
    // Act
    const QStringList &data = m_pComputer->getTableData();
    const QStringList &header = m_pComputer->getTableHeader();
    // Assert: 空实现不产生表格数据
    EXPECT_EQ(0, data.size());
    // Assert: 表头惰性加载：默认 Name/Vendor/Model + m_CanEnable=false -> "no" 列
    EXPECT_EQ(QStringList() << "Name" << "Vendor" << "Model" << "no", header);
}
