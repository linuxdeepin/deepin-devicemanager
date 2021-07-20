
#pragma once


/** @brief Macro Definition */
#define       PB01_ENDPOINT        "inproc://ps01"      // 最简单的请求应答模式
#define       ZP01PS_ENDPOINT      "inproc://zp01ps"    // zmq_poll 
#define       ZP01RR_ENDPOINT      "inproc://zp01rr"    // zmq_poll

#define       PSDEVICE_FRONT       "inproc://pub_sub_front"             // 装置的前端端点
#define       PSDEVICE_BACE        "inproc://pub_sub_back"              // 装置的后端端点 

#define       PSENVELOPE           "inproc://envelope"                  // 信封



/** @brief Rep-Req 相关*/
#define      RR01              "inproc://rr01"                 // 简单的请求应答
#define      RR02_FRONT        "inproc://rr02_front"           // 简单装置的请求应答
#define      RR02_BACK         "inproc://rr02_back"            // 简单装置的请求应答



/** @brief Global variable */
extern void*  gContext;
