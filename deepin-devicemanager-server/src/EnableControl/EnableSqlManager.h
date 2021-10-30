/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ENABLECONFIG_H
#define ENABLECONFIG_H

#include <QMap>
#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <mutex>

class EnableSqlManager : public QObject
{
    Q_OBJECT
public:

    inline static EnableSqlManager *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        EnableSqlManager *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new EnableSqlManager();
                s_Instance.store(sin);
            }
        }
        return sin;
    }

    /**
     * @brief insertDataToRemove 将数据插入到remove表
     * @param path 插入的路径
     * @param hclass 类型
     * @param name 名称
     */
    void insertDataToRemoveTable(const QString& hclass, const QString& name, const QString& path);

    /**
     * @brief removeDateFromRemoveTable 从数据库里面删除数据
     * @param path
     */
    void removeDateFromRemoveTable(const QString& path);

    /**
     * @brief insertDataToAuthorizedTable 将数据插入remove表格
     * @param key
     */
    void insertDataToAuthorizedTable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id);

    /**
     * @brief removeDataFromAuthorizedTable 从数据库里面删除数据
     * @param key
     */
    void removeDataFromAuthorizedTable(const QString& key);

    /**
     * @brief updateDataToAuthorizedTable
     */
    void updateDataToAuthorizedTable(const QString& unique_id, const QString& path);

    /**
     * @brief insertDataToAuthorizedTable 讲数据插入打印机表格
     * @param hclass 数据类型
     * @param name 名称
     * @param path 路径
     */
    void insertDataToPrinterTable(const QString& hclass, const QString& name, const QString& path);

    /**
     * @brief removeDataFromPrinterTable
     * @param name
     */
    void removeDataFromPrinterTable(const QString& name);

    /**
     * @brief uniqueIDExisted 判断数据库中可否查到数据
     * @param key
     * @return
     */
    bool uniqueIDExisted(const QString& key);

    /**
     * @brief removeInfo 返回数据库里面的所有信息
     * @return
     */
    QString removedInfo();

    /**
     * @brief authorizedInfo 获取没有被授权的设备的信息
     * @return
     */
    QString authorizedInfo();

    /**
     * @brief authorizedPath
     * @return
     */
    QString authorizedPath(const QString& unique_id);

    /**
     * @brief removePathList 获取所有被remove的数据
     * @param lsPath
     */
    void removePathList(QStringList& lsPath);

protected:
    explicit EnableSqlManager(QObject *parent = nullptr);

private:
    void initDB();

private:
    static std::atomic<EnableSqlManager *> s_Instance;
    static std::mutex                  m_mutex;
    QSqlDatabase                       m_db;
    QSqlQuery                          m_sqlQuery;
};

#endif // ENABLECONFIG_H
