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
#ifndef DETECTTHREAD_H
#define DETECTTHREAD_H

#include <QThread>

class DetectJob;

/**
 * @brief The DetectThread class
 */
class DetectThread : public QThread
{
    Q_OBJECT
public:
    DetectThread(QObject *parent);

    /**
     * @brief run
     */
    void run() override;

    /**
     * @brief setUSleepElapse
     * @param elapse
     */
    void setUSleepElapse(unsigned int elapse);

signals:
    /**
     * @brief usbChanged
     */
    void usbChanged();

private slots:
    /**
     * @brief slotUsbChanged
     */
    void slotUsbChanged();

private:
    DetectJob    *mp_DectectJob;          //<! 检测任务
};

#endif // DETECTTHREAD_H
