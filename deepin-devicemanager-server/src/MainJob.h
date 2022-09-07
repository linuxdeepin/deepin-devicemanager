// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINJOB_H
#define MAINJOB_H

#include <QObject>
#include <QMutex>
#include <QTimer>

class ThreadPool;
class DetectThread;
class DBusInterface;
class DriverDBusInterface;
class DBusEnableInterface;
class DBusWakeupInterface;
class QThread;

enum INSTRUCTION_RES {
    IR_NULL = 0,
    IR_FAILED = 1,
    IR_SUCCESS = 2,
    IR_UPDATE = 3
};

class MainJob : public QObject
{
    Q_OBJECT
public:
    explicit MainJob(QObject *parent = nullptr);
    ~MainJob();

    void working();

    /**
     * @brief executeClientInstruction
     * @param instructions
     */
    INSTRUCTION_RES executeClientInstruction(const QString &instructions);

    /**
     * @brief isZhaoXin
     * @return
     */
    bool isZhaoXin();

    /**
     * @brief serverIsRunning
     * @return
     */
    static bool serverIsRunning();

    /**
     * @brief clientIsRunning
     * @return
     */
    static bool clientIsRunning();

private slots:
    /**
     * @brief slotUsbChanged
     */
    void slotUsbChanged();

    /**
     * @brief slotUsbChanged
     * @param usbchanged
     */
    void slotDriverControl(bool success);

    /**
     * @brief onFirstUpdate
     */
    void onFirstUpdate();

private:

    /**
     * @brief updateAllDevice
     */
    void updateAllDevice();

    /**
     * @brief initDBus : 初始化dbus
     * @return : 返回bool
     */
    bool initDBus();

    /**
     * @brief initDBus : 初始化Driver相关dbus
     * @return : 返回bool
     */
    bool initDriverDbus();

    /**
     * @brief initDriverRepoSource : 初始化驱动仓库
     * @return : 无
     */
    void initDriverRepoSource();

private:
    ThreadPool            *mp_Pool = nullptr;                 //<! 生成文件的线程池
    DetectThread          *mp_DetectThread = nullptr;         //<! 检测usb的线程
    QTimer                *mp_Timer = nullptr;                //<! 定时器
    DBusInterface         *mp_IFace = nullptr;                //<! Dbus interface
    DriverDBusInterface   *mp_DriverOperateIFace = nullptr;   //<! 驱动操作Dbus服务接口
    DBusEnableInterface   *mp_Enable = nullptr;               //<! 启用禁用dbus
    DBusWakeupInterface   *mp_Wakeup = nullptr;               //<! 唤醒
    static bool           s_ClientIsUpdating;                 //<! 前台正在更新中
    static bool           s_ServerIsUpdating;                 //<! 后台正在更新中
    bool                  m_FirstUpdate;                      //<! 是否是第一次更新

};

#endif // MAINJOB_H
