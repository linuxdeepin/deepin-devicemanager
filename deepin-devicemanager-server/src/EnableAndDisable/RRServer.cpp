#include "RRServer.h"
#include"ZMQServer.h"
#include <QDebug>
#include <QDateTime>

#include "MainJob.h"

RRServer::RRServer(QObject *parent)
    : QThread(parent)
    , mpRep(new ZMQServer)
    , mpContext(nullptr)
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
    while (1) {
        char *msg = mpRep->recvMsg();
        if (!mp_MainJob) {
            sendMsg("0");
            continue;
        }

        // Request update
        if (QString::fromLocal8Bit(msg).startsWith("UPDATE_UI")) {
            if (mp_MainJob->isServerRunning())
                sendMsg(QString::number(IR_SUCCESS));
            else
                sendMsg(QString::number(IR_UPDATE));
            continue;
        }

        INSTRUCTION_RES res = mp_MainJob->executeClientInstruction(QString::fromLocal8Bit(msg));
        sendMsg(QString::number(res));
        continue;
    }
}

void RRServer::sendMsg(const QString &info)
{
    // 发送信息
    char ch[256];
    sprintf(ch, "%s", info.toStdString().c_str());
    mpRep->sendMsg(ch);
}
