/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
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

#ifndef DETECTJOB_H
#define DETECTJOB_H

#include <QObject>

/**
 * @brief The DetectJob class
 */
class DetectJob : public QObject
{
    Q_OBJECT
public:
    explicit DetectJob(QObject *parent = nullptr);

    /**
     * @brief working : main job
     */
    [[noreturn]]void working();

    /**
     * @brief setUSleepElapse : 设置while循环sleep时间
     * @param elapse
     */
    void setUSleepElapse(unsigned int elapse);

signals:
    /**
     * @brief usbChanged
     */
    void usbChanged();

private:
    qint64          m_LaskTime;           //<! last update time
    unsigned int    m_Elapse;             //<! while循环中的sleep时间
};

#endif // DETECTJOB_H
