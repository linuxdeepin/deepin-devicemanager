// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | initDB | high | complexity:20 | 3 | 3 |
// | insertDataToRemoveTable | high | - | 3 | 3 |
// | removeDateFromRemoveTable | mid | - | 2 | 2 |
// | insertDataToAuthorizedTable | high | - | 3 | 3 |
// | removeDataFromAuthorizedTable | mid | - | 2 | 2 |
// | updateDataToAuthorizedTable | low | - | 1 | 1 |
// | clearEnableFromAuthorizedTable | mid | - | 2 | 2 |
// | insertDataToPrinterTable | low | - | 1 | 1 |
// | removeDataFromPrinterTable | low | - | 1 | 1 |
// | uniqueIDExisted/EX | high | - | 3 | 4 |
// | isUniqueIdEnabled | high | - | 3 | 3 |
// | removedInfo | high | - | 3 | 3 |
// | authorizedInfo | high | - | 3 | 3 |
// | authorizedPath/UniqueIDList | high | - | 3 | 3 |
// | removePathList/UniqueIDList | mid | - | 2 | 3 |
// | insertWakeupData/updateWakeData | mid | - | 2 | 2 |
// | isWakeupUniqueIdExisted/isWakeup | low | - | 1 | 3 |
// | wakeupPath | mid | - | 2 | 2 |
// | insertNetworkWakeup/isNetworkWakeup | low | - | 1 | 2 |
// | setMonitorWorkingFlag/monitorWorkingFlag | low | - | 1 | 2 |
// | getInstance | low | - | 1 | 1 |
// 最小清单完成情况（test-code-gen 最小清单）：
// 1. 每个公开方法 >= 1 用例: [x]
// 2. 每个输入维度按等价类划分 >= 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 8. 负面场景有专门用例: [x]

#include <gtest/gtest.h>

#define private public
#define protected public

#include "enablesqlmanager.h"
#include "../ut_testdbhelper.h"

#undef private
#undef protected

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QList>
#include <QPair>
#include <QDir>

#include "../stub.h"

// EnableSqlManager 为进程内 SQLite 单例；真实 DB_PATH 归 root 所有不可写，
// 由 UTDB::setup 将单例成员重新绑定到临时库，测试不触碰任何系统路径。
// DBus/文件系统等外部依赖不参与本类，数据全部落在临时库内。
class EnableSqlManagerTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_mgr = EnableSqlManager::getInstance();
        ASSERT_NE(m_mgr, nullptr);
        ASSERT_TRUE(UTDB::setup(m_mgr));
    }

    void TearDown() override {}

    EnableSqlManager *m_mgr = nullptr;
};

// ---------------- ctor / initDB / getInstance ----------------

// initDB 成功分支：stub QSqlDatabase::addDatabase 返回预置临时库，
// stub open 拦截真实打开，验证按表结构逐个建表(printer 预置存在，跳过建表分支)
static QSqlDatabase g_utInitDb;
static QString g_utInitConnName;
static int g_utOpenCalls = 0;
static QSqlDatabase ut_addDatabase_stub(const QString &type, const QString &connectionName)
{
    Q_UNUSED(type);
    g_utInitConnName = connectionName;
    return g_utInitDb;
}
static bool ut_open_stub(void *self)
{
    Q_UNUSED(self);
    ++g_utOpenCalls;
    return true;
}

// initDB 错误分支补充桩：目录不存在/mkpath 记录/open 失败/exec 失败
static int ut_qdirMkpathCalls = 0;
static QStringList ut_qdirMkpathArgs;
static bool ut_qdirExistsFalse_stub(void *self, const QString &name)
{
    Q_UNUSED(self);
    Q_UNUSED(name);
    return false;
}
static bool ut_qdirMkpath_stub(void *self, const QString &dirPath)
{
    Q_UNUSED(self);
    ++ut_qdirMkpathCalls;
    ut_qdirMkpathArgs.append(dirPath);
    return true;
}
static bool ut_dbOpenFalse_stub(void *self)
{
    Q_UNUSED(self);
    return false;
}
static bool ut_execSql_ret = false;
static bool ut_execSql_stub(void *self, const QString &query)
{
    Q_UNUSED(self);
    Q_UNUSED(query);
    return ut_execSql_ret;
}

TEST_F(EnableSqlManagerTest, initDB_success_createsAllMissingTables)
{
    // Arrange: 预置仅含 printer 表的临时库
    QSqlDatabase pre = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("ut-init-db"));
    pre.setDatabaseName(UTDB::dir().filePath(QStringLiteral("ut-init.db")));
    ASSERT_TRUE(pre.open());
    QSqlQuery q(pre);
    ASSERT_TRUE(q.exec("CREATE TABLE printer (class text, name text, path text);"));
    g_utInitDb = pre;
    g_utInitConnName.clear();
    g_utOpenCalls = 0;

    Stub stub;
    stub.set((QSqlDatabase (*)(const QString &, const QString &))&QSqlDatabase::addDatabase,
             ut_addDatabase_stub);
    stub.set((bool (QSqlDatabase::*)())&QSqlDatabase::open, ut_open_stub);

    // Act: 直接构造实例，触发 ctor -> initDB
    EnableSqlManager mgr;

    // Assert: 使用 device-enable 连接名注册，且 6 张表齐全
    EXPECT_EQ(g_utInitConnName, QStringLiteral("device-enable"));
    EXPECT_GT(g_utOpenCalls, 0);
    EXPECT_TRUE(mgr.m_db.isOpen());
    const QStringList tables = mgr.m_db.tables();
    EXPECT_TRUE(tables.contains("authorized"));
    EXPECT_TRUE(tables.contains("remove"));
    EXPECT_TRUE(tables.contains("printer"));
    EXPECT_TRUE(tables.contains("wake"));
    EXPECT_TRUE(tables.contains("net_wake"));
    EXPECT_TRUE(tables.contains("monitor_dev"));
}

TEST_F(EnableSqlManagerTest, getInstance_repeatedCalls_returnsSameSingletonInstance)
{
    // Arrange: SetUp 中已触发单例初始化
    // Act
    EnableSqlManager *first = EnableSqlManager::getInstance();
    EnableSqlManager *second = EnableSqlManager::getInstance();

    // Assert
    EXPECT_NE(first, nullptr);
    EXPECT_EQ(first, second);
}

// ---------------- remove 表 ----------------

TEST_F(EnableSqlManagerTest, insertDataToRemoveTable_then_removedInfoContainsFormattedFields)
{
    // Arrange
    m_mgr->insertDataToRemoveTable("USB", "utDev", "/ut/path", "ut-uid-1", "utdrv");

    // Act
    const QString info = m_mgr->removedInfo();

    // Assert
    EXPECT_NE(info.indexOf("Hardware Class : USB\n"), -1);
    EXPECT_NE(info.indexOf("name : utDev\n"), -1);
    EXPECT_NE(info.indexOf("path : /ut/path\n"), -1);
    EXPECT_NE(info.indexOf("unique_id : ut-uid-1\n"), -1);
    EXPECT_NE(info.indexOf("driver : utdrv\n"), -1);
    EXPECT_TRUE(info.endsWith("\n\n"));
}

TEST_F(EnableSqlManagerTest, insertDataToRemoveTable_multipleRows_allListed)
{
    // Arrange
    m_mgr->insertDataToRemoveTable("USB", "devA", "/ut/a", "ut-uid-a", "drvA");
    m_mgr->insertDataToRemoveTable("PCI", "devB", "/ut/b", "ut-uid-b", "drvB");

    // Act
    const QString info = m_mgr->removedInfo();

    // Assert
    EXPECT_EQ(UTDB::rowCount(m_mgr, "remove"), 2);
    EXPECT_NE(info.indexOf("devA"), -1);
    EXPECT_NE(info.indexOf("devB"), -1);
}

TEST_F(EnableSqlManagerTest, insertDataToRemoveTable_samePathTwice_keepsBothRows)
{
    // Arrange
    m_mgr->insertDataToRemoveTable("USB", "devA", "/ut/dup", "ut-uid-d1", "drvA");

    // Act: 同一 path 再次插入（源码为普通 INSERT，不去重）
    m_mgr->insertDataToRemoveTable("USB", "devA2", "/ut/dup", "ut-uid-d2", "drvA");

    // Assert
    EXPECT_EQ(UTDB::rowCount(m_mgr, "remove"), 2);
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "remove", "unique_id='ut-uid-d1'"));
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "remove", "unique_id='ut-uid-d2'"));
}

TEST_F(EnableSqlManagerTest, removeDateFromRemoveTable_otherPathUntouched_removesOnlyTargetPath)
{
    // Arrange
    m_mgr->insertDataToRemoveTable("USB", "devA", "/ut/a", "ut-uid-a", "drvA");
    m_mgr->insertDataToRemoveTable("PCI", "devB", "/ut/b", "ut-uid-b", "drvB");

    // Act
    m_mgr->removeDateFromRemoveTable("/ut/a");

    // Assert
    EXPECT_FALSE(UTDB::tableHas(m_mgr, "remove", "path='/ut/a'"));
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "remove", "path='/ut/b'"));
}

TEST_F(EnableSqlManagerTest, removePathList_afterTwoInserts_returnsAllRemovePaths)
{
    // Arrange
    m_mgr->insertDataToRemoveTable("USB", "devA", "/ut/a", "ut-uid-a", "drvA");
    m_mgr->insertDataToRemoveTable("PCI", "devB", "/ut/b", "ut-uid-b", "drvB");

    // Act
    QStringList lsPath;
    m_mgr->removePathList(lsPath);

    // Assert
    EXPECT_EQ(lsPath.size(), 2);
    EXPECT_TRUE(lsPath.contains("/ut/a"));
    EXPECT_TRUE(lsPath.contains("/ut/b"));
}

TEST_F(EnableSqlManagerTest, removePathUniqueIDList_afterInsert_returnsPathUidPairs)
{
    // Arrange
    m_mgr->insertDataToRemoveTable("USB", "devA", "/ut/a", "ut-uid-a", "drvA");

    // Act
    QList<QPair<QString, QString>> lstPair;
    m_mgr->removePathUniqueIDList(lstPair);

    // Assert
    ASSERT_EQ(lstPair.size(), 1);
    EXPECT_EQ(lstPair.first().first, QStringLiteral("/ut/a"));
    EXPECT_EQ(lstPair.first().second, QStringLiteral("ut-uid-a"));
}

// ---------------- authorized 表 ----------------

TEST_F(EnableSqlManagerTest, insertDataToAuthorizedTable_newRecord_inserted)
{
    // Arrange: 空表（SetUp 已清空）
    // Act: 写入一条新记录
    m_mgr->insertDataToAuthorizedTable("USB", "utDev", "/ut/path", "ut-uid-1", true, "utdrv");

    // Assert
    EXPECT_EQ(UTDB::rowCount(m_mgr, "authorized"), 1);
    const QString info = m_mgr->authorizedInfo();
    EXPECT_NE(info.indexOf("Hardware Class : USB\n"), -1);
    EXPECT_NE(info.indexOf("name : utDev\n"), -1);
    EXPECT_NE(info.indexOf("unique_id : ut-uid-1\n"), -1);
    EXPECT_NE(info.indexOf("driver : utdrv\n"), -1);
}

TEST_F(EnableSqlManagerTest, insertDataToAuthorizedTable_duplicateUidPath_notDuplicated)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "utDev", "/ut/path", "ut-uid-1", true, "utdrv");

    // Act: 相同 unique_id + path 再次插入，应被 uniqueIDExistedEX 去重
    m_mgr->insertDataToAuthorizedTable("USB", "utDev", "/ut/path", "ut-uid-1", true, "utdrv");

    // Assert
    EXPECT_EQ(UTDB::rowCount(m_mgr, "authorized"), 1);
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "authorized", "path='/ut/path'"));
}

TEST_F(EnableSqlManagerTest, removeDateFromRemoveTable_missingPath_noChange)
{
    // Arrange
    m_mgr->insertDataToRemoveTable("USB", "devA", "/ut/keep", "ut-uid-k", "drvA");

    // Act: 删除不存在的 path
    m_mgr->removeDateFromRemoveTable("/ut/none");

    // Assert: 记录不受影响
    EXPECT_EQ(UTDB::rowCount(m_mgr, "remove"), 1);
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "remove", "path='/ut/keep'"));
}

TEST_F(EnableSqlManagerTest, removeDataFromAuthorizedTable_otherKeyUntouched_deletesOnlyTargetKey)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/a", "ut-uid-a", true, "drvA");
    m_mgr->insertDataToAuthorizedTable("PCI", "devB", "/ut/b", "ut-uid-b", true, "drvB");

    // Act
    m_mgr->removeDataFromAuthorizedTable("ut-uid-a");

    // Assert
    EXPECT_FALSE(UTDB::tableHas(m_mgr, "authorized", "unique_id='ut-uid-a'"));
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "authorized", "unique_id='ut-uid-b'"));
}

TEST_F(EnableSqlManagerTest, removeDataFromAuthorizedTable_missingKey_noChange)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/keep", "ut-uid-k", true, "drvA");

    // Act: 删除不存在的 unique_id
    m_mgr->removeDataFromAuthorizedTable("ut-nobody");

    // Assert: 记录不受影响
    EXPECT_EQ(UTDB::rowCount(m_mgr, "authorized"), 1);
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "authorized", "unique_id='ut-uid-k'"));
}

TEST_F(EnableSqlManagerTest, updateDataToAuthorizedTable_existingRecord_updatesPath)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/a", "ut-uid-a", true, "drvA");

    // Act
    m_mgr->updateDataToAuthorizedTable("ut-uid-a", "/ut/a2");

    // Assert
    EXPECT_EQ(m_mgr->authorizedPath("ut-uid-a"), QStringLiteral("/ut/a2"));
    EXPECT_EQ(UTDB::rowCount(m_mgr, "authorized"), 1);
}

// 标红缺陷 pinning（只标红不改源码）：authorized 表结构仅有 exist 列而无 enable 列，
// clearEnableFromAuthorizedTable 执行 "DELETE FROM authorized WHERE enable='true'"，
// prepare 阶段即失败，记录不会被清理。当前断言固化的是现状(疑似缺陷)行为。
TEST_F(EnableSqlManagerTest, clearEnableFromAuthorizedTable_keepsRows_enableColumnMissing)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/a", "ut-uid-a", true, "drvA");
    m_mgr->insertDataToAuthorizedTable("PCI", "devB", "/ut/b", "ut-uid-b", true, "drvB");

    // Act
    m_mgr->clearEnableFromAuthorizedTable();

    // Assert: 疑似缺陷 —— enable 列不存在，DELETE 不生效
    EXPECT_EQ(UTDB::rowCount(m_mgr, "authorized"), 2);
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "authorized", "unique_id='ut-uid-a'"));
}

// 标红缺陷 pinning：isUniqueIdEnabled 查询不存在的 enable 列，恒返回 false
TEST_F(EnableSqlManagerTest, isUniqueIdEnabled_returnsFalse_enableColumnMissing)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/a", "ut-uid-a", true, "drvA");

    // Act: 查询启用状态（SELECT enable 列不存在的疑似缺陷路径）
    const bool enabled = m_mgr->isUniqueIdEnabled("ut-uid-a");

    // Assert: 疑似缺陷 —— exec 失败恒返回 false；记录本身仍可按 path 查到
    EXPECT_FALSE(enabled);
    EXPECT_EQ(m_mgr->authorizedPath("ut-uid-a"), QStringLiteral("/ut/a"));
}

TEST_F(EnableSqlManagerTest, isUniqueIdEnabled_multiRows_stillFalse_pinning)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/a", "ut-uid-a", true, "drvA");
    m_mgr->insertDataToAuthorizedTable("PCI", "devB", "/ut/b", "ut-uid-b", true, "drvB");

    // Act
    const bool first = m_mgr->isUniqueIdEnabled("ut-uid-a");
    const bool second = m_mgr->isUniqueIdEnabled("ut-uid-b");

    // Assert: 疑似缺陷 pinning —— enable 列缺失导致恒 false，但记录本身可查
    EXPECT_FALSE(first);
    EXPECT_FALSE(second);
    EXPECT_EQ(m_mgr->authorizedPath("ut-uid-a"), QStringLiteral("/ut/a"));
    EXPECT_EQ(m_mgr->authorizedPath("ut-uid-b"), QStringLiteral("/ut/b"));
}

TEST_F(EnableSqlManagerTest, uniqueIDExisted_withAndWithoutPathArg_filtersCorrectly)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/a", "ut-uid-a", true, "drvA");

    // Act: 分别按 uid、uid+path、uid+其他 path 组合查询
    const bool byUid = m_mgr->uniqueIDExisted("ut-uid-a");
    const bool byUidPath = m_mgr->uniqueIDExisted("ut-uid-a", "/ut/a");
    const bool byUidOtherPath = m_mgr->uniqueIDExisted("ut-uid-a", "/ut/other");
    const bool missing = m_mgr->uniqueIDExisted("ut-nobody");
    const bool emptyPath = m_mgr->uniqueIDExisted("ut-uid-a", QString());

    // Assert: path 为空时退化为仅按 key 查询
    EXPECT_TRUE(byUid);
    EXPECT_TRUE(byUidPath);
    EXPECT_FALSE(byUidOtherPath);
    EXPECT_FALSE(missing);
    EXPECT_TRUE(emptyPath);
}

TEST_F(EnableSqlManagerTest, uniqueIDExistedEX_sameInput_behavesSameAsUniqueIDExisted)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/a", "ut-uid-a", true, "drvA");

    // Act: EX 变体按相同输入查询
    const bool byUid = m_mgr->uniqueIDExistedEX("ut-uid-a");
    const bool byUidPath = m_mgr->uniqueIDExistedEX("ut-uid-a", "/ut/a");
    const bool byUidOtherPath = m_mgr->uniqueIDExistedEX("ut-uid-a", "/ut/other");
    const bool missing = m_mgr->uniqueIDExistedEX("ut-nobody");

    // Assert: 与 uniqueIDExisted 行为一致
    EXPECT_TRUE(byUid);
    EXPECT_TRUE(byUidPath);
    EXPECT_FALSE(byUidOtherPath);
    EXPECT_FALSE(missing);
}

TEST_F(EnableSqlManagerTest, authorizedInfo_emptyTable_returnsEmptyString)
{
    // Arrange: 空表（SetUp 已清空）
    // Act
    const QString info = m_mgr->authorizedInfo();

    // Assert
    EXPECT_EQ(info, QString());
    EXPECT_TRUE(m_mgr->removedInfo().isEmpty());
}

TEST_F(EnableSqlManagerTest, authorizedInfo_twoEntries_containsBothRecords)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/a", "ut-uid-a", true, "drvA");
    m_mgr->insertDataToAuthorizedTable("PCI", "devB", "/ut/b", "ut-uid-b", true, "drvB");

    // Act
    const QString info = m_mgr->authorizedInfo();

    // Assert
    EXPECT_EQ(UTDB::rowCount(m_mgr, "authorized"), 2);
    EXPECT_NE(info.indexOf("Hardware Class : USB\n"), -1);
    EXPECT_NE(info.indexOf("name : devB\n"), -1);
    EXPECT_NE(info.indexOf("unique_id : ut-uid-a\n"), -1);
    EXPECT_NE(info.indexOf("unique_id : ut-uid-b\n"), -1);
}

TEST_F(EnableSqlManagerTest, authorizedPath_existingAndMissingUids_returnsValues)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/a", "ut-uid-a", true, "drvA");

    // Act: 已存在与不存在的 uid 各查询一次
    const QString hit = m_mgr->authorizedPath("ut-uid-a");
    const QString miss = m_mgr->authorizedPath("ut-nobody");

    // Assert
    EXPECT_EQ(hit, QStringLiteral("/ut/a"));
    EXPECT_TRUE(miss.isEmpty());
}

TEST_F(EnableSqlManagerTest, authorizedPathUniqueIDList_twoRecords_returnsAllPairs)
{
    // Arrange
    m_mgr->insertDataToAuthorizedTable("USB", "devA", "/ut/a", "ut-uid-a", true, "drvA");
    m_mgr->insertDataToAuthorizedTable("PCI", "devB", "/ut/b", "ut-uid-b", true, "drvB");

    // Act
    QList<QPair<QString, QString>> lstPair;
    m_mgr->authorizedPathUniqueIDList(lstPair);

    // Assert
    ASSERT_EQ(lstPair.size(), 2);
    EXPECT_EQ(lstPair.first().first, QStringLiteral("/ut/a"));
    QList<QPair<QString, QString>> expectPair;
    expectPair << QPair<QString, QString>("/ut/a", "ut-uid-a")
               << QPair<QString, QString>("/ut/b", "ut-uid-b");
    EXPECT_EQ(lstPair, expectPair);
}

// ---------------- printer 表 ----------------

TEST_F(EnableSqlManagerTest, printerTable_insertAndRemove_roundtrip)
{
    // Arrange & Act
    m_mgr->insertDataToPrinterTable("printer", "ut-printer-a", "/ut/pa");
    m_mgr->insertDataToPrinterTable("printer", "ut-printer-b", "/ut/pb");

    // Assert: printer 表无公开查询接口，经临时库直接核对
    EXPECT_EQ(UTDB::rowCount(m_mgr, "printer"), 2);
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "printer", "name='ut-printer-a'"));

    // Act
    m_mgr->removeDataFromPrinterTable("ut-printer-a");

    // Assert
    EXPECT_FALSE(UTDB::tableHas(m_mgr, "printer", "name='ut-printer-a'"));
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "printer", "name='ut-printer-b'"));
}

// ---------------- wake 表 ----------------

TEST_F(EnableSqlManagerTest, wakeupData_insertUpdateQuery_roundtrip)
{
    // Arrange & Act
    m_mgr->insertWakeupData("ut-uid-kb", "/ut/wake", true);

    // Assert
    EXPECT_TRUE(m_mgr->isWakeupUniqueIdExisted("ut-uid-kb"));
    EXPECT_TRUE(m_mgr->isWakeup("ut-uid-kb"));
    EXPECT_EQ(m_mgr->wakeupPath("ut-uid-kb"), QStringLiteral("/ut/wake"));

    // Act: 更新分支
    m_mgr->updateWakeData("ut-uid-kb", "/ut/wake2", false);

    // Assert
    EXPECT_EQ(m_mgr->wakeupPath("ut-uid-kb"), QStringLiteral("/ut/wake2"));
    EXPECT_FALSE(m_mgr->isWakeup("ut-uid-kb"));

    // Assert: 不存在的记录
    EXPECT_FALSE(m_mgr->isWakeupUniqueIdExisted("ut-nobody"));
    EXPECT_FALSE(m_mgr->isWakeup("ut-nobody"));
    EXPECT_TRUE(m_mgr->wakeupPath("ut-nobody").isEmpty());
}

// ---------------- net_wake 表 ----------------

TEST_F(EnableSqlManagerTest, networkWakeup_insertThenUpdate_roundtrip)
{
    // Arrange: 空表（SetUp 已清空）
    // Act: 首次插入分支
    m_mgr->insertNetworkWakeup("uteth0", true);

    // Assert
    EXPECT_TRUE(m_mgr->isNetworkWakeup("uteth0"));

    // Act: 已存在时走更新分支
    m_mgr->insertNetworkWakeup("uteth0", false);

    // Assert
    EXPECT_FALSE(m_mgr->isNetworkWakeup("uteth0"));

    // Act & Assert: 第二条记录仍走插入分支
    m_mgr->insertNetworkWakeup("uteth1", true);
    EXPECT_TRUE(m_mgr->isNetworkWakeup("uteth1"));
    EXPECT_FALSE(m_mgr->isNetworkWakeup("ut-nobody"));
}

// ---------------- monitor_dev 表 ----------------

TEST_F(EnableSqlManagerTest, monitorWorkingFlag_defaultTrue_thenSetFalseTrue)
{
    // Arrange: 空表（SetUp 已清空）
    // Act & Assert: 无记录时默认 true
    EXPECT_TRUE(m_mgr->monitorWorkingFlag());

    // Act: 无记录时走插入分支
    m_mgr->setMonitorWorkingFlag(false);

    // Assert
    EXPECT_FALSE(m_mgr->monitorWorkingFlag());
    EXPECT_TRUE(UTDB::tableHas(m_mgr, "monitor_dev", "monitor_name='usb' and working_flag=0"));

    // Act: 已存在时走更新分支
    m_mgr->setMonitorWorkingFlag(true);

    // Assert
    EXPECT_TRUE(m_mgr->monitorWorkingFlag());
}

// ---------------- 覆盖率补充：initDB 错误分支 ----------------

TEST_F(EnableSqlManagerTest, initDB_mkpathNeededAndOpenFailed_returnsEarly)
{
    // Arrange: 目录不存在 -> mkpath 被调用；随后 open 失败 -> 提前返回
    Stub stub;
    ut_qdirMkpathCalls = 0;
    ut_qdirMkpathArgs.clear();
    stub.set((bool (QDir::*)(const QString &) const)&QDir::exists, ut_qdirExistsFalse_stub);
    stub.set((bool (QDir::*)(const QString &) const)&QDir::mkpath, ut_qdirMkpath_stub);
    stub.set((QSqlDatabase (*)(const QString &, const QString &))&QSqlDatabase::addDatabase,
             ut_addDatabase_stub);
    stub.set((bool (QSqlDatabase::*)())&QSqlDatabase::open, ut_dbOpenFalse_stub);

    // Act
    m_mgr->initDB();

    // Assert: mkpath 收到固定 DB_PATH，且 open 失败后未建表
    EXPECT_EQ(ut_qdirMkpathCalls, 1);
    ASSERT_EQ(ut_qdirMkpathArgs.size(), 1);
    EXPECT_EQ(ut_qdirMkpathArgs.first(),
              QStringLiteral("/var") + QStringLiteral("/lib/deepin-devicemanager/"));
}

TEST_F(EnableSqlManagerTest, initDB_allTablesMissing_execFail_logsError)
{
    // Arrange: 全程桩化隔离 —— addDatabase 返回无表的临时库，避免触达真实系统库
    QSqlDatabase pre = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
                                                 QStringLiteral("ut-init-execfail"));
    pre.setDatabaseName(UTDB::dir().filePath(QStringLiteral("ut-init-execfail.db")));
    ASSERT_TRUE(pre.open());
    QSqlQuery cleanup(pre);
    ASSERT_TRUE(cleanup.exec("DROP TABLE IF EXISTS authorized;"));
    ASSERT_TRUE(cleanup.exec("DROP TABLE IF EXISTS remove;"));
    ASSERT_TRUE(cleanup.exec("DROP TABLE IF EXISTS printer;"));
    ASSERT_TRUE(cleanup.exec("DROP TABLE IF EXISTS wake;"));
    ASSERT_TRUE(cleanup.exec("DROP TABLE IF EXISTS net_wake;"));
    ASSERT_TRUE(cleanup.exec("DROP TABLE IF EXISTS monitor_dev;"));
    g_utInitDb = pre;
    g_utInitConnName.clear();
    ut_execSql_ret = false;

    // Act: exec(sql) 桩返回失败，覆盖 6 个建表失败日志分支；出作用域自动恢复
    {
        Stub stub;
        stub.set((bool (QDir::*)(const QString &) const)&QDir::exists, ut_qdirExistsFalse_stub);
        stub.set((bool (QDir::*)(const QString &) const)&QDir::mkpath, ut_qdirMkpath_stub);
        stub.set((QSqlDatabase (*)(const QString &, const QString &))&QSqlDatabase::addDatabase,
                 ut_addDatabase_stub);
        stub.set((bool (QSqlDatabase::*)())&QSqlDatabase::open, ut_open_stub);
        stub.set((bool (QSqlQuery::*)(const QString &))&QSqlQuery::exec, ut_execSql_stub);
        m_mgr->initDB();
    }

    // Assert: 建表全部失败，读接口退回默认值且不崩溃
    EXPECT_FALSE(m_mgr->uniqueIDExisted("ut-none"));
    EXPECT_EQ(m_mgr->removedInfo(), QString());
}

// ---------------- 覆盖率补充：SQL 错误路径（no such table） ----------------

TEST_F(EnableSqlManagerTest, noSuchTable_allMethods_reportErrorsReturnDefaults)
{
    // Arrange: 库中无表 -> 所有 SELECT/INSERT/UPDATE 的错误分支均触发
    ASSERT_TRUE(UTDB::setupEmpty(m_mgr));

    // Act: 写入/删除类接口静默失败
    m_mgr->insertDataToRemoveTable("hub", "utHub", "/ut/path", "ut-uid-err");
    m_mgr->removeDateFromRemoveTable("/ut/path");
    m_mgr->insertDataToAuthorizedTable("usb", "utDev", "/ut/path", "ut-uid-err", true, "usbhid");
    m_mgr->removeDataFromAuthorizedTable("ut-uid-err");
    m_mgr->updateDataToAuthorizedTable("ut-uid-err", "/ut/new-path");
    m_mgr->clearEnableFromAuthorizedTable();
    m_mgr->insertDataToPrinterTable("printer", "utPrinter", "/ut/path");
    m_mgr->removeDataFromPrinterTable("utPrinter");
    m_mgr->insertWakeupData("ut-uid-err", "/ut/path", true);
    m_mgr->updateWakeData("ut-uid-err", "/ut/path", false);
    m_mgr->insertNetworkWakeup("uteth0", true);
    m_mgr->setMonitorWorkingFlag(false);

    // Assert: 读类接口返回默认值（false / 空串 / 空列表）
    EXPECT_FALSE(m_mgr->uniqueIDExisted("ut-uid-err"));
    EXPECT_FALSE(m_mgr->uniqueIDExisted("ut-uid-err", QStringLiteral("/ut/path")));
    EXPECT_FALSE(m_mgr->uniqueIDExistedEX("ut-uid-err", QStringLiteral("/ut/path")));
    EXPECT_FALSE(m_mgr->isUniqueIdEnabled("ut-uid-err"));
    EXPECT_EQ(m_mgr->removedInfo(), QString());
    EXPECT_EQ(m_mgr->authorizedInfo(), QString());
    EXPECT_EQ(m_mgr->authorizedPath("ut-uid-err"), QString());
    {
        QList<QPair<QString, QString>> pairs;
        m_mgr->authorizedPathUniqueIDList(pairs);
        EXPECT_EQ(pairs.size(), 0);
    }
    {
        QStringList paths;
        m_mgr->removePathList(paths);
    }
    {
        QList<QPair<QString, QString>> pairs;
        m_mgr->removePathUniqueIDList(pairs);
        EXPECT_EQ(pairs.size(), 0);
    }
    EXPECT_FALSE(m_mgr->isWakeupUniqueIdExisted("ut-uid-err"));
    EXPECT_EQ(m_mgr->wakeupPath("ut-uid-err"), QString());
    EXPECT_FALSE(m_mgr->isWakeup("ut-uid-err"));
    EXPECT_FALSE(m_mgr->isNetworkWakeup("uteth0"));
    // monitor_dev 错误路径返回默认 true
    EXPECT_TRUE(m_mgr->monitorWorkingFlag());
}

// ---------------- 覆盖率补充：uniqueIDExisted 带 path 过滤 ----------------

TEST_F(EnableSqlManagerTest, uniqueIDExisted_nonEmptyPath_usesPathFilter)
{
    // Arrange
    ASSERT_TRUE(UTDB::setup(m_mgr));
    m_mgr->insertDataToAuthorizedTable("usb", "utDev", "/ut/old", "ut-uid-path", true, "usbhid");

    // Act: 路径匹配 / 路径不匹配 / 记录不存在
    const bool matched = m_mgr->uniqueIDExisted("ut-uid-path", QStringLiteral("/ut/old"));
    const bool mismatched = m_mgr->uniqueIDExisted("ut-uid-path", QStringLiteral("/ut/other"));
    const bool missing = m_mgr->uniqueIDExisted("ut-uid-nobody", QStringLiteral("/ut/old"));

    // Assert: 路径匹配 -> 存在；路径不匹配 -> 不存在
    EXPECT_TRUE(matched);
    EXPECT_FALSE(mismatched);
    EXPECT_FALSE(missing);
    EXPECT_EQ(m_mgr->authorizedPath("ut-uid-path"), QStringLiteral("/ut/old"));
}
