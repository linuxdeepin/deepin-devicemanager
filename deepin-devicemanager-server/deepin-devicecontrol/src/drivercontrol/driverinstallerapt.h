// SPDX-FileCopyrightText: 2019 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISABLE_DRIVER

#ifndef DRIVERINSTALLER_APT_H
#define DRIVERINSTALLER_APT_H

#include <QObject>

class QProcess;

class DriverInstallerApt : public QObject
{
    Q_OBJECT
public:
    explicit DriverInstallerApt(QObject *parent = nullptr);
    ~DriverInstallerApt();

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
     * @brief aptClean 清理apt缓存
     */
    void aptClean();

    /**
     * @brief isNetworkOnline 判断网络是否在线
     * @param sec 超时时间（微秒）
     * @return 是否在线
     */
    bool isNetworkOnline(uint sec = 2000000);

    /**
     * @brief doOperate 执行安装操作
     * @param package 包名
     * @param version 版本号
     */
    void doOperate(const QString &package, const QString &version);

    /**
     * @brief executeCommand 执行shell命令
     * @param cmd 命令字符串
     * @return 命令输出
     */
    QString executeCommand(const QString &cmd, const QStringList &args);

private:
    QProcess *m_process;        // apt进程指针
    int m_iRuningTestCount;     // dpkg运行测试计数
    bool m_Cancel;              // 取消标志
};

#endif // DRIVERINSTALLER_APT_H
#endif // DISABLE_DRIVER 