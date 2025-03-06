// SPDX-FileCopyrightText: 2019 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APTINSTALLER_H
#define APTINSTALLER_H

#include <QObject>
#include <QProcess>

class AptInstaller : public QObject {
    Q_OBJECT
public:
    explicit AptInstaller(QObject *parent = nullptr);
    
    // 检查是否可用
    bool isValid();
    
    // 检查架构是否匹配
    bool isArchMatched(const QString &path);
    
    // 检查deb包是否有效
    bool isDebValid(const QString &path);

public slots:
    void installPackage(const QString &filepath);
    void uninstallPackage(const QString &packagename);

signals:
    void installFinished(bool success);
    void errorOccurred(const QString &errmsg);
    void progressChanged(int progress);

private:
    void doOperate(const QString &package, bool isInstall);
    bool checkDpkgLock();
    void parseAptOutput(const QString &output);
    
private:
    QProcess *m_process;
    bool m_bValid;
    int m_iRuningTestCount;
    static const int MAX_DPKGRUNING_TEST = 20;
    static const int TEST_TIME_INTERVAL = 2000;
};

#endif // APTINSTALLER_H 