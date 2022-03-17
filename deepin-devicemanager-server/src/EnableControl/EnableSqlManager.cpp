#include "EnableSqlManager.h"

#include <QDebug>
#include <QDir>
#include <QSqlError>

#define DB_PATH "/usr/share/deepin-devicemanager/"
#define DB_FILE "enable.db"
#define DB_CONNECT_NAME "device-enable"
#define DB_TABLE_AUTHORIZED "authorized"
#define DB_TABLE_REMOVE "remove"
#define DB_TABLE_PRINTER "printer"
#define DB_TABLE_WAKEUP "wake"

std::atomic<EnableSqlManager *> EnableSqlManager::s_Instance;
std::mutex EnableSqlManager::m_mutex;

void EnableSqlManager::insertDataToRemoveTable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id, const QString strDriver)
{
    QString sql = QString("INSERT INTO %1 (class, name, path, unique_id, driver) VALUES ('%2', '%3', '%4', '%5', '%6');")
            .arg(DB_TABLE_REMOVE).arg(hclass).arg(name).arg(path).arg(unique_id).arg(strDriver);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::removeDateFromRemoveTable(const QString& path)
{
    QString sql = QString("DELETE FROM %1 WHERE path='%2';").arg(DB_TABLE_REMOVE).arg(path);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::insertDataToAuthorizedTable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id, bool exist, const QString strDriver)
{
    // 数据库已经存在该设备记录
    if(uniqueIDExistedEX(unique_id)){
        return;
    }

    // 数据库没有该设备记录，则直接插入
    QString sql = QString("INSERT INTO %1 (class, name, path, unique_id, exist, driver) VALUES ('%2', '%3', '%4', '%5', '%6', '%7');")
            .arg(DB_TABLE_AUTHORIZED).arg(hclass).arg(name).arg(path).arg(unique_id).arg(exist).arg(strDriver);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::removeDataFromAuthorizedTable(const QString& key)
{
    QString sql = QString("DELETE FROM %1 WHERE unique_id='%2';").arg(DB_TABLE_AUTHORIZED).arg(key);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::updateDataToAuthorizedTable(const QString& unique_id, const QString& path)
{
    QString sql = QString("UPDATE %1 SET path='%2' WHERE unique_id='%3';").arg(DB_TABLE_AUTHORIZED).arg(path).arg(unique_id);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::updateDataToAuthorizedTable(const QString& unique_id, bool enable_device)
{
    QString sql = QString("UPDATE %1 SET enable='%2' WHERE unique_id='%3';").arg(DB_TABLE_AUTHORIZED).arg(enable_device).arg(unique_id);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::clearEnableFromAuthorizedTable()
{
    QString sql = QString("DELETE FROM %1 WHERE enable='%2';").arg(DB_TABLE_AUTHORIZED).arg(true);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::insertDataToPrinterTable(const QString& hclass, const QString& name, const QString& path)
{
    QString sql = QString("INSERT INTO %1 (class, name, path) VALUES ('%2', '%3', '%4');").arg(DB_TABLE_PRINTER).arg(hclass).arg(name).arg(path);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

void EnableSqlManager::removeDataFromPrinterTable(const QString& name)
{
    QString sql = QString("DELETE FROM %1 WHERE name='%2';").arg(DB_TABLE_PRINTER).arg(name);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << m_sqlQuery.lastError();
    }
}

bool EnableSqlManager::uniqueIDExisted(const QString& key)
{
    QString sql = QString("SELECT COUNT(*) FROM %1 WHERE unique_id='%2';").arg(DB_TABLE_AUTHORIZED).arg(key);
    if (m_sqlQuery.exec(sql) && m_sqlQuery.next()) {
        return m_sqlQuery.value(0).toInt() > 0;
    }
    return false;
}

bool EnableSqlManager::uniqueIDExistedEX(const QString& key)
{
    QString sql = QString("SELECT COUNT(*) FROM %1 WHERE unique_id='%2';").arg(DB_TABLE_AUTHORIZED).arg(key);
    if (m_sqlQuery.exec(sql) && m_sqlQuery.next()) {
        return m_sqlQuery.value(0).toInt() > 0;
    }
    return false;
}

bool EnableSqlManager::isUniqueIdEnabled(const QString& key)
{
    QString sql = QString("SELECT enable FROM %1 WHERE unique_id='%2';").arg(DB_TABLE_AUTHORIZED).arg(key);
    if(m_sqlQuery.exec(sql) && m_sqlQuery.next()){
        return m_sqlQuery.value(0).toBool();
    }
    return false;
}

QString EnableSqlManager::removedInfo()
{
    QString info = "";
    QString sql = QString("SELECT class,name,path,unique_id,driver FROM %1;").arg(DB_TABLE_REMOVE);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
        return info;
    }

    while (m_sqlQuery.next()) {
        info += "Hardware Class : " + m_sqlQuery.value(0).toString() + "\n";
        info += "name : " + m_sqlQuery.value(1).toString() + "\n";
        info += "path : " + m_sqlQuery.value(2).toString() + "\n";
        info += "unique_id : " + m_sqlQuery.value(3).toString() + "\n";
        info += "driver : " + m_sqlQuery.value(4).toString() + "\n\n";
    }
    return info;
}

QString EnableSqlManager::authorizedInfo()
{
    QString info = "";
    QString sql = QString("SELECT class,name,path,unique_id,driver FROM %1;").arg(DB_TABLE_AUTHORIZED);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
        return info;
    }

    while(m_sqlQuery.next()){
        info += "Hardware Class : " + m_sqlQuery.value(0).toString() + "\n";
        info += "name : " + m_sqlQuery.value(1).toString() + "\n";
        info += "path : " + m_sqlQuery.value(2).toString() + "\n";
        info += "unique_id : " + m_sqlQuery.value(3).toString() + "\n";
        info += "driver : " + m_sqlQuery.value(4).toString() + "\n\n";
    }
    return info;
}

QString EnableSqlManager::authorizedPath(const QString& unique_id)
{
    QString sql = QString("SELECT path FROM %1 WHERE unique_id='%2';").arg(DB_TABLE_AUTHORIZED).arg(unique_id);
    if (m_sqlQuery.exec(sql) && m_sqlQuery.next()) {
        return m_sqlQuery.value(0).toString();
    }
    return "";
}

void EnableSqlManager::removePathList(QStringList& lsPath)
{
    QString sql = QString("SELECT path FROM %1;").arg(DB_TABLE_REMOVE);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
        return;
    }
    while (m_sqlQuery.next()) {
        lsPath.append(m_sqlQuery.value(0).toString());
    }
}

void EnableSqlManager::insertWakeupData(const QString& unique_id, const QString& path, bool wakeup)
{
    QString sql = QString("INSERT INTO %1 (unique_id, path, wakeup) VALUES ('%2', '%3', '%4');").arg(DB_TABLE_WAKEUP).arg(unique_id).arg(path).arg(wakeup);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
    }
}

bool EnableSqlManager::isWakeupUniqueIdExisted(const QString& unique_id)
{
    QString sql = QString("SELECT COUNT(*) FROM %1 WHERE unique_id='%2';").arg(DB_TABLE_WAKEUP).arg(unique_id);
    if (m_sqlQuery.exec(sql) && m_sqlQuery.next()) {
        return m_sqlQuery.value(0).toInt() > 0;
    }
    return false;
}

void EnableSqlManager::updateWakeData(const QString& unique_id, const QString& path, bool wakeup)
{
    QString sql = QString("UPDATE %1 SET path='%2', wakeup='%3' WHERE unique_id='%4';").arg(DB_TABLE_WAKEUP).arg(path).arg(wakeup).arg(unique_id);
    if (!m_sqlQuery.exec(sql)) {
        qInfo() << m_sqlQuery.lastError();
    }
}

QString EnableSqlManager::wakeupPath(const QString& unique_id)
{
    QString sql = QString("SELECT path FROM %1 WHERE unique_id='%2';").arg(DB_TABLE_WAKEUP).arg(unique_id);
    if (m_sqlQuery.exec(sql) && m_sqlQuery.next()) {
        return m_sqlQuery.value(0).toString();
    }
    return "";
}

bool EnableSqlManager::isWakeup(const QString& unique_id)
{
    QString sql = QString("SELECT wakeup FROM %1 WHERE unique_id='%2';").arg(DB_TABLE_WAKEUP).arg(unique_id);
    if (m_sqlQuery.exec(sql) && m_sqlQuery.next())
        return m_sqlQuery.value(0).toBool();
    return false;
}

EnableSqlManager::EnableSqlManager(QObject *parent)
    :QObject (parent)
{
    initDB();
}

void EnableSqlManager::initDB()
{
    //初始化数据库
    QDir dbDir;
    if (!dbDir.exists(DB_PATH)) {
        dbDir.mkpath(DB_PATH);
    }
    m_db = QSqlDatabase::addDatabase("QSQLITE", DB_CONNECT_NAME);
    m_db.setDatabaseName(QString("%1%2").arg(DB_PATH).arg(DB_FILE));
    if (!m_db.open()) {
        qDebug() << Q_FUNC_INFO << "local db open error!";
        return;
    }

    // 初始化查询接口
    m_sqlQuery = QSqlQuery(m_db);

    // 创建数据库表
    QStringList tableStrList = m_db.tables();

    if(!tableStrList.contains(DB_TABLE_AUTHORIZED)){
        QString sql = QString("CREATE TABLE %1 (class text, name text, path text, unique_id text, exist boolean, driver text);").arg(DB_TABLE_AUTHORIZED);
        bool res = m_sqlQuery.exec(sql);
        if(!res){
            qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
        }
    }
    if(!tableStrList.contains(DB_TABLE_REMOVE)){
        QString sql = QString("CREATE TABLE %1 (class text, name text, path text, unique_id text, driver text);").arg(DB_TABLE_REMOVE);
        bool res = m_sqlQuery.exec(sql);
        if(!res){
            qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
        }
    }
    if(!tableStrList.contains(DB_TABLE_PRINTER)){
        QString sql = QString("CREATE TABLE %1 (class text, name text, path text)").arg(DB_TABLE_PRINTER);
        bool res = m_sqlQuery.exec(sql);
        if(!res){
            qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
        }
    }
    if(!tableStrList.contains(DB_TABLE_WAKEUP)){
        QString sql = QString("CREATE TABLE %1 (unique_id text, path text, wakeup boolean)").arg(DB_TABLE_WAKEUP);
        bool res = m_sqlQuery.exec(sql);
        if(!res){
            qInfo() << Q_FUNC_INFO << m_sqlQuery.lastError();
        }
    }
}
