#include "ZmqManager.h"
#include "ZMQBase.h"

ZmqManager *ZmqManager::s_Instance = nullptr;

ZmqManager::ZmqManager(QObject *parent)
    : QObject(parent)
    , mp_Req(new ZMQBase)
{

}

bool ZmqManager::connectTo(char *end)
{
    mp_Req->init(ZMQ_REQ);
    mp_Req->setID(rand());
    return mp_Req->connect(end);
}

int ZmqManager::updateData()
{
    char ch[128] = "UPDATE";
    mp_Req->sendMsg(ch);
    mp_Req->recvMsg();
    return 1;
}


