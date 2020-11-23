#include "ZMQBase.h"

//#include "ZMQClient.h"
#include"QDebug"

ZMQBase::ZMQBase()
    : mpContext(nullptr)
    , mpSocket(nullptr)
{
}

ZMQBase::~ZMQBase()
{
    if (mpSocket) {
        zmq_close(mpSocket);
        mpSocket = nullptr;
    }

    if (mpContext) {
        zmq_term(mpContext);
        mpContext = nullptr;
    }

}

void ZMQBase::init(int _type)
{
    //  初始化ZMQ
    mpContext = zmq_init(1);
    mpSocket = zmq_socket(mpContext, _type);
}

void ZMQBase::init(int _type, void *_context)
{
    mpSocket = zmq_socket(_context, _type);
}

bool ZMQBase::setID(/*int _id*/)
{
    if (!mpSocket) {
        return false;
    } else {
        s_set_id(mpSocket/*, _id*/);
        return true;
    }

}

bool ZMQBase::connect(char *_endpoint)
{
    if (0 == zmq_connect(mpSocket, _endpoint))
        return true;
    else
        return false;
}

bool ZMQBase::bind(char *_endpoint)
{
    if (0 == zmq_bind(mpSocket, _endpoint))
        return true;
    else
        return false;
}

void ZMQBase::sendMsg(char *_msg)
{
    s_send(mpSocket, _msg);
}

void ZMQBase::sendMsg(int _value)
{
    char ch[16];
    sprintf(ch, "%d", _value);
    s_send(mpSocket, ch);

}

void ZMQBase::sendMoreMsg(char *_msg)
{
    s_sendmore(mpSocket, _msg);
}

void ZMQBase::sendMoreMsg(int _value)
{
    char ch[16];
    sprintf(ch, "%d", _value);
    s_sendmore(mpSocket, ch);
}

char *ZMQBase::recvMsg()
{
    return s_recv(mpSocket);
}

int   ZMQBase::recvIntMsg()
{
    char *str = s_recv(mpSocket);
    return atoi(str);
}

int64_t ZMQBase::more()
{
    int64_t more;
    size_t more_size = sizeof(more);
    zmq_getsockopt(mpSocket, ZMQ_RCVMORE, &more, &more_size);
    return more;
}

void *ZMQBase::socket()
{
    return mpSocket;
}

void ZMQBase::setsockopt(int _name, const void *_value, size_t _len)
{
    zmq_setsockopt(mpSocket, _name, _value, _len);
}
