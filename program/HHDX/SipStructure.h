#ifndef __SIP_STRUCTURE_H__
#define __SIP_STRUCTURE_H__

#include "sys.h"

#define MAX_NUM_LEN     16
#define MAX_SERVER_NUMBER   5

//typedef struct stUaInfo stUaInfo;
struct stUaInfo
{
//------基本信息	
	char host_ip[16];
	unsigned int host_port;
	
	char server_ip[16];
	unsigned int server_port;
	
	char dst_ip[16];
	unsigned int dst_port;
	
	char src_ip[16];
	unsigned int src_port;	
	
	char rtp_ip[16];
	unsigned int rtp_port;	
	
	char calling_number[MAX_NUM_LEN];
	char called_number[MAX_NUM_LEN];

//------SIP信息

//branch：所有请求在Via中都产生自己的branch。必须要以“z9hG4bK”开头
//但有两种情况例外：
//	1. CANCEL请求，CANCEL请求的branch参数与它所要取消的那个请求的branch参数是一样
//	2. 对非2xx响应的ACK请求，这种情况下ACK请求与相关的INVITE请求有着上相同的branch ID，它所要确认的就是该INVITE的响应。
//	char invite_branch[25];//建立INVITE时要记录branch=z9hG4bKxxxxxxxxxxxxxxxx\0

//	char callid[64];	
//	char tag[10];
	unsigned char cseq_value;
	int  expires;

//	struct sdp_media_l host_media[5];
//	struct sdp_key_l   host_key[5];

// 新增重传机制字段
    u8 retry_count;          // 当前重传次数
    u8 max_retry;            // 最大重传次数 (3次)
    u16 retry_timeout;       // 重传超时时间 (2秒 = 2000ms)
    u16 retry_timer;         // 重传计时器
    u8 last_msg_type;        // 最后发送的消息类型
    u8 waiting_response;     // 等待回复标志

// 新增动态端口协商字段
    u16 dynamic_port;        // 服务器分配的动态端口
    u8 port_negotiated;      // 端口协商完成标志 (0=未协商, 1=已协商)
    u8 port_verified;        // 端口验证完成标志 (0=未验证, 1=已验证)
    u16 port_request_timer;  // 端口请求超时计时器
    u8 port_request_retry;   // 端口请求重试次数

};

/*
 1. SIP消息结构体
 2. 所有字段都是记录整行
 3. 因为每字段大小未知(via可以超出200)，所以用指针动态方式
*/
struct stMsg
{ 
	char via_num;
	char re_route_num;
	unsigned int cseq_num;
	char *via[MAX_SERVER_NUMBER];
	char *from;
	char *to;
	char *callid;
	char *re_route[MAX_SERVER_NUMBER];
	char *cseq;
};


struct stSipRxFlag
{
	unsigned short invite:	1;		
	unsigned short ack:			1;		
	unsigned short bye:			1;
	unsigned short cancel:	1;
		
	unsigned short ringing:	1;
	unsigned short ok_inv:	1;
	unsigned short ok_bye:	1;
	unsigned short ok_reg:	1;
	unsigned short ok_cel:	1;
	unsigned short proxy:		1;
		
	unsigned short sdp:			1;		
	unsigned short rec:			1;
	unsigned short c:	1;
	unsigned short d:	1;
	unsigned short e:	1;
	unsigned short f:	1;
};

struct stSipTxFlag
{
	unsigned short invite:	1;		
	unsigned short ack:			1;		
	unsigned short bye:			1;
	unsigned short cancel:	1;
		
	unsigned short ringing:	1;
	unsigned short ok_inv:	1;
	unsigned short ok_bye:	1;
	unsigned short ok_reg:	1;
	unsigned short ok_cel:	1;
	unsigned short proxy:		1;
		
	unsigned short rtp:			1;		
	unsigned short retry:		1;		// 重传标志
	unsigned short c:	1;
	unsigned short d:	1;
	unsigned short e:	1;
	unsigned short f:	1;
};

struct stDate
{
	unsigned char sec;
	unsigned char min;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned char year;
};

extern struct stDate gsDate;
extern struct stMsg gsMsg;

extern struct stUaInfo *gpsUaInfo;
extern struct stSipRxFlag gsSipRxFlag;
extern struct stSipTxFlag gsSipTxFlag;

#endif
