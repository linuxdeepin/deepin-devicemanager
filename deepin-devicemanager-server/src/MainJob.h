#ifndef MAINJOB_H
#define MAINJOB_H

#include <QObject>
#include <QMutex>

class ThreadPool;
class RRServer;
class DetectThread;

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
     * @brief updateDevice : 更新设备
     */
    void updateMonitor();

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
     * @brief getDriverPath
     * @param driver
     * @return
     */
    QString getDriverPath(const QString &driver);

private:
    ThreadPool   *mp_Pool;               //<! 生成文件的线程池
    RRServer     *mp_ZmqServer;          //<! 监听后台的服务端
    DetectThread *mp_DetectThread;       //<! 检测usb的线程
    qint64       m_UpdateTime;           //<! 更新时间
    bool         m_Delay;                //<! 延迟时间
};

#endif // MAINJOB_H
