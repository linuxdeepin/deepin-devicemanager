#ifndef MAINJOB_H
#define MAINJOB_H

#include <QObject>

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

private slots:
    void slotUsbChanged();
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
    ThreadPool   *mp_Pool;
    RRServer     *mp_ZmqServer;
    DetectThread *mp_DetectThread;
};

#endif // MAINJOB_H
