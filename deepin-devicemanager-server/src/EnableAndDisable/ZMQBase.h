
///////////////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) SGRI
/// All rights reserved
///
/// @file      ZMQClient.h
/// @brief     Encapsulates ZeroMQ, the client of ZeroMQ
/// @author    LiuJun
/// @date      2019/7/11
//////////////////////////////////////////////////////////////////////////////////
#ifndef ZMQBASE_H
#define ZMQBASE_H

#include "zmq.h"
#include "zhelpers.h"

class ZMQBase
{
public:
    ZMQBase();
    ~ZMQBase();

    /*
    * @brief:  Creates socket
    * @param: _type(ZMQ_REQ,ZMQ_SUB ... )
    * @param: _context(External context)
    */
    void init(int _type);
    void init(int _type, void *_context);

    /** @brief 标识消息的来源 */
    bool setID(/*int _id*/);

    /*
    * @brief: connects to the specified IP and port
    * @param: _endpoint("tcp://IP:port")
    */
    bool connect(char *_endpoint);

    /*
    * @brief: bind to the specified IP and port
    * @param: _endpoint("tcp://IP:port")
    */
    bool bind(char *_endpoint);

    /** @brief: send message */
    void sendMsg(char *_msg);
    void sendMsg(int _value);

    /** @brief: send multi-frame message */
    void sendMoreMsg(char *_msg);
    void sendMoreMsg(int _value);

    /** @brief: recv message */
    char *recvMsg();
    int   recvIntMsg();

    /** @brief: Processing multi-frame messages */
    int64_t more();

    /** @brief: Get socket */
    void *socket();

    /*
    * @brief: 设置ZMQ socket的属性
    * @param: _name(ZMQ_SNDHWM,ZMQ_CONFLATE,ZMQ_SUBSCRIBE ....)
    * @param: _value(Just value)
    * @param: _len(The len of value)
    */
    void setsockopt(int _name, const void *_value, size_t _len);

protected:
    void           *mpContext;             //<! The context of ZeroMQ
    void           *mpSocket;              //<! Server Socket
};

#endif //
