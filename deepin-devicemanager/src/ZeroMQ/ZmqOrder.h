#ifndef ZMQORDER_H
#define ZMQORDER_H

#include <QObject>
class ZMQBase;

class ZmqOrder : QObject
{
    Q_OBJECT

public:
    explicit ZmqOrder(QObject *parent = nullptr);

    // 析构
    ~ZmqOrder();

    /**
     * @brief connectTo
     * @param end
     * @return
     */
    bool connect();
    bool connectTo(char *end);

    /**
     * @brief execDriverOrder
     * @param cmd
     * @return
     */
    bool execDriverOrder(const QString &cmd);

    /**
     * @brief execIfconfigOrder
     * @param cmd
     * @return
     */
    bool execIfconfigOrder(const QString &cmd);

private:
    ZMQBase                   *mp_Req;
    QString                   m_EndPoint;
};

#endif // ZMQORDER_H
