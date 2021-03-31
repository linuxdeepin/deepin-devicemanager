#ifndef MAINJOB_H
#define MAINJOB_H

#include <QObject>
#include <QMutex>
#include <QTimer>

class ThreadPool;
class RRServer;
class DetectThread;
class DBusInterface;

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
    void executeClientInstruction(const QString &instructions);

    /**
     * @brief isZhaoXin
     * @return
     */
    bool isZhaoXin();

private slots:
    /**
     * @brief slotUsbChanged
     */
    void slotUsbChanged();

    /**
     * @brief slotExecuteClientInstructions
     * @param instructions
     */
    void slotExecuteClientInstructions(const QString &instructions);

    /**
     * @brief onFirstUpdate
     */
    void onFirstUpdate();

private:

    /**
     * @brief handleInstruction : 处理命令的方法
     * @param instruction : 需要处理的命令
     */
    void handleInstruction(const QString &instruction);

    /**
     * @brief updateAllDevice
     */
    void updateAllDevice();

    /**
     * @brief nullInstruction
     */
    void nullInstruction();

    /**
     * @brief driverInstruction
     * @param instruction
     */
    void driverInstruction(const QString &instruction);

    /**
     * @brief ifconfigInstruction
     * @param instruction
     */
    void ifconfigInstruction(const QString &instruction);

    /**
     * @brief reqUpdateInstruction
     */
    void reqUpdateInstruction();

    /**
     * @brief initDBus : 初始化dbus
     * @return : 返回bool
     */
    bool initDBus();

private:
    ThreadPool            *mp_Pool;               //<! 生成文件的线程池
    RRServer              *mp_ZmqServer;          //<! 监听后台的服务端
    DetectThread          *mp_DetectThread;       //<! 检测usb的线程
    bool                  m_UpdateUI;             //<! 客户端请求更新
    QTimer                *mp_Timer;              //<! 定时器
    DBusInterface         *mp_IFace;              //<! Dbus interface
    bool                  m_FirstUpdate;

};

#endif // MAINJOB_H
