/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     kongyunzhen <kongyunzhen@uniontech.com>
*
* Maintainer: kongyunzhen <kongyunzhen@uniontech.com>
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
#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <QObject>

namespace QApt {
class Backend;
class Transaction;
}

class DebInstaller: public QObject
{
    Q_OBJECT
public:
    explicit DebInstaller(QObject *parent = nullptr);
    //判断当前DebInstaller对象是否生效，如果无效继续调用可能导致crash
    bool isValid();

    /**
     * @brief isArchMatched 判断架构是否匹配
     * @param path
     * @return
     */
    bool isArchMatched(QString path);

private:
    void initBackend();
    void reset();
    void doOperate(const QString &package, bool binstall);


public slots:
    void installPackage(const QString &filepath);
    void uninstallPackage(const QString &packagename);

signals:
    void installFinished(bool bsuccess);
    void errorOccurred(const QString &errmsg);
    void progressChanged(int progress);

private:
    QApt::Backend *m_backend = nullptr;
    QApt::Transaction *m_pTrans = nullptr;
    bool m_bValid = false;
    int m_iRuningTestCount = 0;
};

#endif // DEBINSTALLER_H
