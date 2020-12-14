#include "ZmqOrder.h"
#include "ZMQBase.h"

#include <QDebug>
#include <QDateTime>

ZmqOrder::ZmqOrder(QObject *parent)
    : mp_Req(new ZMQBase())
    , m_EndPoint("tcp://127.0.0.1:8700")
{

}

ZmqOrder::~ZmqOrder()
{
    // mp_Req没有父指针需要手动释放
    if (mp_Req != nullptr) {
        delete mp_Req;
        mp_Req = nullptr;
    }
}

bool ZmqOrder::connect()
{
    mp_Req->init(ZMQ_REQ);
    mp_Req->setID();      // 更新接口，删除未使用id
    char end[64];
    sprintf(end, "%s", m_EndPoint.toStdString().c_str());
    return mp_Req->connect(end);
}

bool ZmqOrder::connectTo(char *end)
{
    m_EndPoint = QString::fromLocal8Bit(end);
    mp_Req->init(ZMQ_REQ);
    mp_Req->setID();      // 更新接口，删除未使用id
    return mp_Req->connect(end);
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


