#ifndef __SIP_STRUCTURE_H__
#define __SIP_STRUCTURE_H__

#include "sys.h"

#define MAX_NUM_LEN     16
#define MAX_SERVER_NUMBER   5

//typedef struct stUaInfo stUaInfo;
struct stUaInfo
{
//------������Ϣ	
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

//------SIP��Ϣ

//branch������������Via�ж������Լ���branch������Ҫ�ԡ�z9hG4bK����ͷ
//��������������⣺
//	1. CANCEL����CANCEL�����branch����������Ҫȡ�����Ǹ������branch������һ��
//	2. �Է�2xx��Ӧ��ACK�������������ACK��������ص�INVITE������������ͬ��branch ID������Ҫȷ�ϵľ��Ǹ�INVITE����Ӧ��
//	char invite_branch[25];//����INVITEʱҪ��¼branch=z9hG4bKxxxxxxxxxxxxxxxx\0

//	char callid[64];	
//	char tag[10];
	unsigned char cseq_value;
	int  expires;

//	struct sdp_media_l host_media[5];
//	struct sdp_key_l   host_key[5];

// �����ش������ֶ�
    u8 retry_count;          // ��ǰ�ش�����
    u8 max_retry;            // ����ش����� (3��)
    u16 retry_timeout;       // �ش���ʱʱ�� (2�� = 2000ms)
    u16 retry_timer;         // �ش���ʱ��
    u8 last_msg_type;        // ����͵���Ϣ����
    u8 waiting_response;     // �ȴ��ظ���־

// ������̬�˿�Э���ֶ�
    u16 dynamic_port;        // ����������Ķ�̬�˿�
    u8 port_negotiated;      // �˿�Э����ɱ�־ (0=δЭ��, 1=��Э��)
    u8 port_verified;        // �˿���֤��ɱ�־ (0=δ��֤, 1=����֤)
    u16 port_request_timer;  // �˿�����ʱ��ʱ��
    u8 port_request_retry;   // �˿��������Դ���

};

/*
 1. SIP��Ϣ�ṹ��
 2. �����ֶζ��Ǽ�¼����
 3. ��Ϊÿ�ֶδ�Сδ֪(via���Գ���200)��������ָ�붯̬��ʽ
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
	unsigned short retry:		1;		// �ش���־
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
