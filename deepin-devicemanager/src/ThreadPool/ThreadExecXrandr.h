#ifndef THREADEXECXRANDR_H
#define THREADEXECXRANDR_H

#include <QThread>

class ThreadExecXrandr : public QThread
{
public:
    ThreadExecXrandr(bool gpu);

    /**
     * @brief run
     */
    virtual void run() override;

private:
    /**
     * @brief runCmd
     * @param info
     * @param cmd
     */
    void runCmd(QString &info, const QString &cmd);

    /**
     * @brief loadXrandrInfo:加载xrandr获取的信息
     * @param lstMap
     * @param cmd:调试文件名
     */
    void loadXrandrInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd);

    /**
     * @brief loadXrandrVerboseInfo:加载xrandr --verbose获取的信息
     * @param lstMap
     * @param cmd:调试文件名
     */
    void loadXrandrVerboseInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd);

    /**
     * @brief getMonitorInfoFromXrandrVerbose:从xrandr --verbose获取显示设备信息
     */
    void getMonitorInfoFromXrandrVerbose();

    /**
     * @brief getMonitorRefreshRateFromXrandr:从xrandr获取显示设备刷新率
     */
    void getMonitorRefreshRateFromXrandr();

    /**
     * @brief getGpuInfoFromXrandr:从xrandr获取显卡信息
     */
    void getGpuInfoFromXrandr();


private:
    bool m_Gpu;    //<!  判断是否是gpu
};

#endif // THREADEXECXRANDR_H