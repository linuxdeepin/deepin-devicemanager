#include "ZmqOrder.h"
#include "ZMQBase.h"

#include <QDebug>
#include <QDateTime>

ZmqOrder::ZmqOrder(QObject *parent)
    : QThread(parent)
    , mp_Req(new ZMQBase)
    , m_EndPoint("tcp://127.0.0.1:8700")
{

}

bool ZmqOrder::connect()
{
    mp_Req->init(ZMQ_REQ);
    mp_Req->setID(rand());
    char end[64];
    sprintf(end, "%s", m_EndPoint.toStdString().c_str());
    return mp_Req->connect(end);
}

bool ZmqOrder::connectTo(char *end)
{
    m_EndPoint = QString::fromLocal8Bit(end);
    mp_Req->init(ZMQ_REQ);
    mp_Req->setID(rand());
    return mp_Req->connect(end);
}

void ZmqOrder::updateData()
{
    char ch[128] = "UPDATE";
    mp_Req->sendMsg(ch);
    mp_Req->recvMsg();
    char end[64];
    sprintf(end, "%s", m_EndPoint.toStdString().c_str());
    mp_Req->disConnect(end);
}

bool ZmqOrder::execDriverOrder(const QString &cmd)
{
    char ch[128];
    sprintf(ch, "DRIVER#%s", cmd.toStdString().c_str());
    mp_Req->sendMsg(ch);
    // 2:success   1:failed
    char *msg = mp_Req->recvMsg();

    char end[64];
    sprintf(end, "%s", m_EndPoint.toStdString().c_str());
    mp_Req->disConnect(end);

    if (QString::fromLocal8Bit(msg) == "2") {
        return true;
    } else {
        return false;
    }
}

bool ZmqOrder::execIfconfigOrder(const QString &cmd)
{
    char ch[128];
    sprintf(ch, "IFCONFIG#%s", cmd.toStdString().c_str());
    mp_Req->sendMsg(ch);

    qDebug() << "ch " << ch;

    // 2:success   1:failed
    char *msg = mp_Req->recvMsg();
    qDebug() << "msg" << msg;

    char end[64];
    sprintf(end, "%s", m_EndPoint.toStdString().c_str());
    mp_Req->disConnect(end);

    qDebug() << "end " << end;

    if (QString::fromLocal8Bit(msg) == "2") {
        qDebug() << "QString::fromLocal8Bit(msg) == 2";
        return true;
    } else {
        qDebug() << "QString::fromLocal8Bit(msg) != 2";
        return false;
    }
}



void ZmqOrder::run()
{
    updateData();
}


