#ifndef MAINJOB_H
#define MAINJOB_H

#include <QObject>
#include <QMutex>
#include <QTimer>

class ThreadPool;
class DetectThread;
class DBusInterface;
class DriverDBusInterface;

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
     * @brief isServerRunning
     * @return server running
     */
    bool isServerRunning();

private slots:
    /**
     * @brief slotUsbChanged
     */
    void slotUsbChanged();

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

private:
    ThreadPool            *mp_Pool;               //<! 生成文件的线程池
    DetectThread          *mp_DetectThread;       //<! 检测usb的线程
    QTimer                *mp_Timer;              //<! 定时器
    DBusInterface         *mp_IFace;              //<! Dbus interface
    DriverDBusInterface   *mp_DriverOperateIFace; //<! 驱动操作Dbus服务接口
    bool                  m_ClientIsUpdating;     //<! 前台正在更新中
    bool                  m_ServerIsUpdating;     //<! 后台正在更新中
    bool                  m_FirstUpdate;          //<! 是否是第一次更新

};

#endif // MAINJOB_H
