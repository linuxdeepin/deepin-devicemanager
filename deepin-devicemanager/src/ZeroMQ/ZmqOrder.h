#ifndef ZMQORDER_H
#define ZMQORDER_H

#include <QThread>
class ZMQBase;

class ZmqOrder : public QThread
{
    Q_OBJECT

public:
    explicit ZmqOrder(QObject *parent = nullptr);

    /**
     * @brief connectTo
     * @param end
     * @return
     */
    bool connect();
    bool connectTo(char *end);

    /**
     * @brief updateData
     * @return
     */
    void updateData();

protected:
    void run() override;

private:
    ZMQBase                   *mp_Req;
    QString                   m_EndPoint;
};

#endif // ZMQORDER_H
