#ifndef THREADEXECLSHWNETWORK_H
#define THREADEXECLSHWNETWORK_H

#include <QThread>

class ThreadExecLshwNetwork : public QThread
{
public:
    ThreadExecLshwNetwork();

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
     * @brief getMapInfoFromCmd:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));
};

#endif // THREADEXECLSHWNETWORK_H
