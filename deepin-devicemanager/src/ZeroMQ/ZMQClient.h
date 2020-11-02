
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
#ifndef ZMQCLIENT_H
#define ZMQCLIENT_H

#include"ZMQBase.h"

class ZMQClient:public ZMQBase
{
public:
	ZMQClient();
	~ZMQClient();
};

#endif // ZMQCLIENT_H
