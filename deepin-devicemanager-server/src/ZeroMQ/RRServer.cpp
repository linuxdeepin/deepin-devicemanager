#include "RRServer.h"
#include"ZMQServer.h"
#include <QDebug>
#include <QDateTime>

RRServer::RRServer(QObject *parent)
    : QThread(parent)
    , mpRep(new ZMQServer)
    , mpContext(nullptr)
    , m_Waiting(false)
    , m_ReturnStr("")
{

}

RRServer::~RRServer()
{

}

bool RRServer::initTo(char *_end)
{
    mpRep->init(ZMQ_REP);
    return mpRep->bind(_end);
}

void RRServer::connectToDealer(char *_endpoint)
{
    mpRep->init(ZMQ_REP);
    mpRep->connect(_endpoint);
}

void RRServer::connectToDealer(char *_endpoint, void *_context)
{
    mpRep->init(ZMQ_REP, _context);
    mpRep->connect(_endpoint);
}

void RRServer::run()
{
    qint64 begin = 0, end = 0;
    while (1) {
        if (!m_Waiting) {
            begin = QDateTime::currentMSecsSinceEpoch();
            char *msg = mpRep->recvMsg();
            emit instruction(QString::fromLocal8Bit(msg));
            m_Waiting = true;
        } else {
            if (!m_ReturnStr.isEmpty()) {
                sendMsg(m_ReturnStr);
                m_ReturnStr = "";
                m_Waiting = false;
                end = QDateTime::currentMSecsSinceEpoch();
                qDebug() << " ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ " << end - begin;
            }
        }
        usleep(100);
    }
}

void RRServer::sendMsg(const QString &info)
{
    char ch[256];
    sprintf(ch, "%s", info.toStdString().c_str());
    mpRep->sendMsg(ch);
}

void RRServer::setWaiting(bool waiting)
{
    m_Waiting = waiting;
}

void RRServer::setReturnStr(const QString &str)
{
    m_ReturnStr = str;
}
