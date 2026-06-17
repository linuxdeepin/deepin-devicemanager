// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISABLE_DRIVER

#ifndef DRIVERINSTALLER_H
#define DRIVERINSTALLER_H

//class PackageInstaller
//{
//public:
//    PackageInstaller();
//};

#include <QObject>

namespace QApt {
class Backend;
class Transaction;
class DebFile;
}

class DriverInstaller : public QObject
{
    Q_OBJECT
public:
    explicit DriverInstaller(QObject *parent = nullptr);

    // 驱动仓库源文件路径
    static const QString DEVICE_REPO_PATH;
    static const QString DRIVER_REPO_PATH;

public slots:
    /**
     * @brief installPackage 安装包
     * @param package 包名
     * @param version 包版本
     */
    void installPackage(const QString &package, const QString &version);

    /**
     * @brief undoInstallDriver 停止任务
     */
    void undoInstallDriver();

signals:
    void errorOccurred(int error);
    void installProgressChanged(int progress);//安装进度
    void installProgressFinished(bool bsuccess);

private:
    /**
     * @brief initBackend 初始化工作
     * @return
     */
    bool initBackend();

    /**
     * @brief aptClean
     */
    void aptClean();

    /**
     * @brief isNetworkOnline 判断网络是否在线
     * @return
     */
    bool isNetworkOnline(uint sec = 2000000);

    /**
     * @brief doOperate 开始操作
     * @param package
     * @param version
     */
    void doOperate(const QString &package, const QString &version);
    /**
     * @brief ensureDriverRepoSource 按需创建驱动仓库源文件
     *        仅在 driver.list 中不存在 pro-driver-packages 时创建 devicemanager.list
     */
    void ensureDriverRepoSource();

    /**
     * @brief cleanupTempSource 清理本次创建的临时源文件
     */
    void cleanupTempSource();

private:
    QApt::Backend *mp_Backend = nullptr;
    QApt::Transaction *mp_Trans = nullptr;
    int m_iRuningTestCount = 0;
    bool m_Cancel;
    bool m_tempSourceCreated = false;  // 是否在本次安装中创建了临时源
};

#endif // DRIVERINSTALLER_H
#endif // DISABLE_DRIVER
