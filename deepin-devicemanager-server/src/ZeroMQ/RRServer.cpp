#include "RRServer.h"
#include"ZMQServer.h"
#include <QDebug>
#include <QDateTime>

#include "MainJob.h"

RRServer::RRServer(QObject *parent)
    : QThread(parent)
    , mpRep(new ZMQServer)
    , mpContext(nullptr)
    , m_Waiting(false)
    , m_ReturnStr("")
    , mp_MainJob(dynamic_cast<MainJob *>(parent))
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
            m_Waiting = true;
            qDebug() << " ************** begin 001";
            char *msg = mpRep->recvMsg();
            qDebug() << " ************** begin 002";
            begin = QDateTime::currentMSecsSinceEpoch();
            emit instruction(QString::fromLocal8Bit(msg));
        } else {
            if (!m_ReturnStr.isEmpty()) {
                qDebug() << " ************** begin 003";
                sendMsg(m_ReturnStr);
                qDebug() << " ************** begin 004";
                m_ReturnStr = "";
                m_Waiting = false;
                end = QDateTime::currentMSecsSinceEpoch();
                qDebug() << " ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ " << end - begin;
            }
        }
//        usleep(100);
    }


//    while (true) {
//        if (!mp_MainJob) {
//            qDebug() << "**************** 重大问题 mp_MainJob = nullptr *********************************";
//            break;
//        }
//        char *msg = mpRep->recvMsg();
//        mp_MainJob->executeClientInstruction(QString::fromLocal8Bit(msg));
//        char ch[] = "1";
//        mpRep->sendMsg(ch);
//    }
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
