// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_TESTDBHELPER_H
#define UT_TESTDBHELPER_H

#include "enablesqlmanager.h"

#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>

// EnableSqlManager 单测用临时 SQLite 库注入辅助。
//
// EnableSqlManager::initDB 使用固定 DB_PATH(/var/lib/deepin-devicemanager/)，
// 该目录归 root 所有、测试用户不可写：initDB 打开失败(或只读打开)后成员
// m_db/m_sqlQuery 不指向可写库。这里在实例构造完成后，把成员重新绑定到
// 进程内临时库并按 initDB 同款语句建表，保证测试自建自清，不触碰任何系统路径。
//
// 注意：本头文件访问私有成员 m_db/m_sqlQuery，依赖调用方先以
// "#define private public" 方式包含 enablesqlmanager.h(仓库既有测试惯例)。
namespace UTDB {

inline QTemporaryDir &dir()
{
    static QTemporaryDir tmpDir;
    return tmpDir;
}

inline QString connectionName()
{
    return QStringLiteral("ut-enable-db");
}

inline QString dbFilePath()
{
    return dir().filePath(QStringLiteral("ut-enable.db"));
}

inline QSqlDatabase db()
{
    const QString name = connectionName();
    if (QSqlDatabase::contains(name))
        return QSqlDatabase::database(name);
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
    db.setDatabaseName(dbFilePath());
    return db;
}

// 将单例成员 m_db/m_sqlQuery 重新绑定到临时库，按 initDB 同款结构建表并清空数据
inline bool setup(EnableSqlManager *mgr)
{
    QSqlDatabase db = UTDB::db();
    if (!db.isOpen() && !db.open())
        return false;

    mgr->m_db = db;
    mgr->m_sqlQuery = QSqlQuery(db);

    const QStringList createSqls = {
        "CREATE TABLE IF NOT EXISTS authorized (class text, name text, path text, unique_id text, exist boolean, driver text);",
        "CREATE TABLE IF NOT EXISTS remove (class text, name text, path text, unique_id text, driver text);",
        "CREATE TABLE IF NOT EXISTS printer (class text, name text, path text);",
        "CREATE TABLE IF NOT EXISTS wake (unique_id text, path text, wakeup boolean);",
        "CREATE TABLE IF NOT EXISTS net_wake (logical_name text, wakeup boolean);",
        "CREATE TABLE IF NOT EXISTS monitor_dev (monitor_name text, working_flag boolean);"
    };
    for (const QString &sql : createSqls) {
        if (!mgr->m_sqlQuery.exec(sql))
            return false;
    }

    // 清空历史数据，保证用例互不影响
    const QStringList tables = { "authorized", "remove", "printer", "wake", "net_wake", "monitor_dev" };
    for (const QString &table : tables)
        mgr->m_sqlQuery.exec(QStringLiteral("DELETE FROM %1;").arg(table));
    return true;
}

// 绑定临时库但删除全部表，用于触发 "no such table" 的 SQL 错误分支
inline bool setupEmpty(EnableSqlManager *mgr)
{
    QSqlDatabase db = UTDB::db();
    if (!db.isOpen() && !db.open())
        return false;

    mgr->m_db = db;
    mgr->m_sqlQuery = QSqlQuery(db);

    QSqlQuery drop(db);
    const QStringList tables = { "authorized", "remove", "printer", "wake", "net_wake", "monitor_dev" };
    for (const QString &table : tables) {
        if (!drop.exec(QStringLiteral("DROP TABLE IF EXISTS %1;").arg(table)))
            return false;
    }
    return true;
}

inline int rowCount(EnableSqlManager *mgr, const QString &table)
{
    QSqlQuery q(mgr->m_db);
    if (!q.exec(QStringLiteral("SELECT COUNT(*) FROM %1;").arg(table)))
        return -1;
    return q.next() ? q.value(0).toInt() : -1;
}

inline bool tableHas(EnableSqlManager *mgr, const QString &table, const QString &where = QString())
{
    QSqlQuery q(mgr->m_db);
    const QString sql = where.isEmpty()
            ? QStringLiteral("SELECT COUNT(*) FROM %1;").arg(table)
            : QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2;").arg(table).arg(where);
    if (!q.exec(sql))
        return false;
    return q.next() && q.value(0).toInt() > 0;
}

} // namespace UTDB

#endif // UT_TESTDBHELPER_H
