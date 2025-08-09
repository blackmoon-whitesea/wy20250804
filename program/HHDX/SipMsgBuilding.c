
#include <stdio.h>
#include "usart.h"
#include "malloc.h"
#include "net_rxtx.h"
#include "stdio.h"
#include "string.h" 
#include "timer.h"
#include "HhdxMsg.h"
#include "24cxx.h" 
#include "SipFunction.h"
#include "usart3_FSK.h"

// ��������
//void SEND_CID_JSON(const char* msg_type, const char* status, const char* call_number);

char *gpDeviceVer = "2301091740";
//char gDeviceId[16] = "1222";  // Ĭ���豸IDΪ1222����ͨ�����������޸�
/***********************************************************************
"DTMF=" + ����(1λ)
***********************************************************************/
void BUILD_DIAL_DTMF(u8 dtmf_val)
{
	int j;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
			
	j=sprintf(SendMsg,"DTMF=%d\r\n",dtmf_val);
		
	NET_TxPcbLenWr(SIP_TX,j);
}


/***********************************************************************
"start" + ���(4λ) + ������(16λ) + �����(16λ) 
***********************************************************************/
void BUILD_CID_start(void)
{
	int i,j;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	strcpy((char *)gFskCidBuf,gpsUaInfo->called_number);	//9012
	for(i =strlen((char *)gFskCidBuf); i <MAX_NUM_LEN; i++){
		gFskCidBuf[i] =0x20;
	}
	gFskCidBuf[MAX_NUM_LEN] =0;
	
	strcpy((char *)gFskOwnBuf,(const char*)gpsEeprom->own_num);	//9012
	for(i =strlen((char *)gFskOwnBuf); i <MAX_NUM_LEN; i++){
		gFskOwnBuf[i] =0x20;
	}
	gFskOwnBuf[MAX_NUM_LEN] =0;
		
	j=sprintf(SendMsg,"start1001%s%s\r\n",gFskOwnBuf,gFskCidBuf);
		
	NET_TxPcbLenWr(SIP_TX,j);
}

/***********************************************************************
"stop" + ���(4λ)
***********************************************************************/
void BUILD_CID_stop(void)
{
	int j;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
		
	j=sprintf(SendMsg,"stop1001\r\n");
		
	NET_TxPcbLenWr(SIP_TX,j);
}

/***********************************************************************
"stop" + ���(4λ)
***********************************************************************/
void BUILD_CID_offhook(void)
{
	int j;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
		
	j=sprintf(SendMsg,"offhook1001\r\n");
		
	NET_TxPcbLenWr(SIP_TX,j);
}



// /***********************************************************************
// "start" + ���(4λ) + ������(16λ) + �����(16λ) 
// ***********************************************************************/
// void BUILD_CID_start(void)
// {
// 	int i;
	
// 	strcpy((char *)gFskCidBuf,gpsUaInfo->called_number);	//�������
// 	for(i =strlen((char *)gFskCidBuf); i <MAX_NUM_LEN; i++){
// 		gFskCidBuf[i] =0x20;
// 	}
// 	gFskCidBuf[MAX_NUM_LEN] =0;
	
// 	// ÿ�ζ���EEPROM��ȡ���µı�������
// 	strcpy((char *)gFskOwnBuf,(const char*)gpsEeprom->own_num);	//��������
// 	for(i =strlen((char *)gFskOwnBuf); i <MAX_NUM_LEN; i++){
// 		gFskOwnBuf[i] =0x20;
// 	}
// 	gFskOwnBuf[MAX_NUM_LEN] =0;
	
// 	// ʹ���µ�JSON���ͺ���
// 	SEND_CID_JSON("incoming_call", "start", (char*)gFskCidBuf);

// 	//�ش�����
// 	gpsUaInfo->waiting_response = 1;
//     gpsUaInfo->retry_count = 0;
//     gpsUaInfo->max_retry = 3;
//     gpsUaInfo->retry_timeout = 2000;  // 2��
//     gpsUaInfo->retry_timer = 2000;
//     gpsUaInfo->last_msg_type = MSG_TYPE_START;
// }

// /***********************************************************************
// "stop" + ���(4λ)
// ***********************************************************************/
// void BUILD_CID_stop(void)
// {
// 	// ʹ���µ�JSON���ͺ���
// 	SEND_CID_JSON("call_end", "stop", NULL);

// 	//�ش�����
// 	gpsUaInfo->waiting_response = 1;
//     gpsUaInfo->retry_count = 0;
//     gpsUaInfo->max_retry = 3;
//     gpsUaInfo->retry_timeout = 2000;  // 2��
//     gpsUaInfo->retry_timer = 2000;
//     gpsUaInfo->last_msg_type = MSG_TYPE_STOP;
// }

// /***********************************************************************
// "stop" + ���(4λ)
// ***********************************************************************/
// void BUILD_CID_offhook(void)
// {
// 	// ʹ���µ�JSON���ͺ���
// 	SEND_CID_JSON("call_answer", "offhook", NULL);

// 	//�ش�����
// 	gpsUaInfo->waiting_response = 1;
//     gpsUaInfo->retry_count = 0;
//     gpsUaInfo->max_retry = 3;
//     gpsUaInfo->retry_timeout = 2000;  // 2��
//     gpsUaInfo->retry_timer = 2000;
//     gpsUaInfo->last_msg_type = MSG_TYPE_OFFHOOK;
// }


/***********************************************************************
;REGISTER sip:server_ip SIP/2.0
;Via: SIP/2.0/UDP SourceIP:5060;branch=z9hG4bKxxxxxxxxxxxxxxxx
;From: <sip:CallingNumber@server_ip>;tag=xxxxxxxxxxxxxxxx
;To: <sip:CallingNumber@server_ip>
;Call-ID: CallID@server_ip
;Contact: <sip:CallingNumber@SourceIP:5060>
;CSeq: xx REGISTER
;Max-Forwards: 70
;Expires: 3600
;Allow: INVITE,CANCEL,ACK,BYE,NOTIFY,REFER,OPTIONS,INFO,MESSAGE,UPDATE
;User-Agent: HHDX-IP-PHONE-2010051301
;Content-Length: 0
************************************************************************/
void BUILD_register(void)
{
	static unsigned short reg_cseq_val=1;
	char reg_ramdom[33];
	int j;
	char *SendMsg;		
	struct stUaInfo *UAinfo;
	UAinfo =gpsUaInfo;
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
//REGISTER sip:172.16.1.133 SIP/2.0	
	j=sprintf(SendMsg,"REGISTER sip:%s SIP/2.0\r\n",UAinfo->server_ip);
//Via: SIP/2.0/UDP 172.16.180.112:5060;rport;branch=z9hG4bK2e793bd9ea
	SIP_CountBranch(reg_ramdom);	
	j+=sprintf(SendMsg+j,"Via: SIP/2.0/UDP %s:%d;branch=%s\r\n",UAinfo->host_ip,UAinfo->host_port,reg_ramdom);
//From: "9012" <sip:9012@172.16.1.133>;tag=762ea54f	
	SIP_CountTag(reg_ramdom);	
	j+=sprintf(SendMsg+j,"From: <sip:%s@%s>;tag=%s\r\n",UAinfo->calling_number,UAinfo->server_ip,reg_ramdom);
//To: "9012" <sip:9012@172.16.1.133>
	j+=sprintf(SendMsg+j,"To: <sip:%s@%s>\r\n",UAinfo->calling_number,UAinfo->server_ip);
//Call-ID: 652ff2ba2ee8c53c33ecfe330c24e0e7@172.16.180.112	
	SIP_CountCallid(reg_ramdom);
	j+=sprintf(SendMsg+j,"Call-ID: %s@%s\r\n",reg_ramdom,UAinfo->host_ip);
//CSeq: 1 REGISTER	
	j+=sprintf(SendMsg+j,"CSeq: %d REGISTER\r\n",reg_cseq_val);
	reg_cseq_val++;
	if(reg_cseq_val==0)
		reg_cseq_val=1;
//Contact: <sip:9012@172.16.180.112:5060>	
	j+=sprintf(SendMsg+j,"Contact: <sip:%s@%s:%d>\r\n",UAinfo->calling_number,UAinfo->host_ip,UAinfo->host_port);
//Max-Forwards: 70	
	j+=sprintf(SendMsg+j,"Max-Forwards: 70\r\n");
//Expires: 1200	
	j+=sprintf(SendMsg+j,"Expires: %d\r\n",UAinfo->expires);
//Allow: INVITE,CANCEL,ACK,BYE,NOTIFY,REFER,OPTIONS,INFO,MESSAGE,UPDATE	
	j+=sprintf(SendMsg+j,"Allow: INVITE,CANCEL,ACK,BYE,NOTIFY,REFER,OPTIONS,INFO,MESSAGE,UPDATE\r\n");
//User-Agent: HHDX-IPPHONE-10011201	
	j+=sprintf(SendMsg+j,"User-Agent: HHDX-ARM-IPPHONE-%s\r\n",gpDeviceVer);
//Content-Length: 0
	j+=sprintf(SendMsg+j,"Content-Length: 0\r\n\r\n");
	
	NET_TxPcbLenWr(SIP_TX,j);
}


/***********************************************************************
;����ժ������INVITE(sip_mechanism=#TX_INVITE):
;	INVITE sip:Called@server_ip:5060 SIP/2.0
;1).  ����Viaͷ�ֶ�Via: SIP/2.0/UDP ԴIP:5060;branch=z9hG4bKxxxxxx
;2).  ����Fromͷ�ֶ� From:sip:���к�@������IP:tag= xxxxxx
;3).  ����Toͷ�ֶ�To:sip:���к�@������IP
;4).  ����Call-IDͷ�ֶ�Call-ID:xxxxxx@������IP
;5).  ����Contactͷ�ֶ�Contact:< sip:������@����IP:5060>
;6).  ����Cseqͷ�ֶ�CSeq: xxx INVITE
;7).  ����Max-Fowordsͷ�ֶ�Max-Forwards: 70
;8).  ����Allowͷ�ֶ�Allow: INVITE,CANCEL,ACK,BYE,NOTIFY,REFER,OPTIONS,INFO,MESSAGE,UPDATE
;9).  ����User-Agentͷ�ֶ�User-Agent:HHDX-IP-PHON-09040801
;10). ������Record-Routeͷ�ֶ�
;12). ������Routeͷ�ֶ�Route: <sip:������IP:5060;lr>
;13). ����Content-Typeͷ�ֶ�Content-Type: application/sdp
;14). ����Content-Lengthͷ�ֶ�Content-Length: xx
;15). ����SDPЭ��
;		v=0
;		o=������ 0246810 1357911 IN IP4 ����IP
;		s=A call
;		c=IN IP4 ����IP
;		t=0 0
;		m=audio 5000 RTP/AVP 8
;		a=rtpmap:8 PCMA/8000
;		a=sendrecv
;***********************************************************************/
void BUILD_invite(void)
{
	int j,bp;
	char *SendMsg;	
	char reg_ramdom[33];
	char ip_string[16];
	struct stUaInfo *UAinfo;
	UAinfo=gpsUaInfo;
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);	
	
	j=sprintf(SendMsg,"INVITE sip:%s@%s:%d SIP/2.0\r\n",UAinfo->called_number,UAinfo->server_ip,UAinfo->server_port);
	SIP_CountBranch(reg_ramdom);	
	gsMsg.via[0] =MymallocCopy(gsMsg.via[0],24,reg_ramdom);		//cancel��inviteһ��
	j+=sprintf(SendMsg+j,"Via: SIP/2.0/UDP %s:%d;branch=%s\r\n",UAinfo->host_ip,UAinfo->host_port,reg_ramdom);
	SIP_CountTag(reg_ramdom);
	gsMsg.from =MymallocCopy(gsMsg.from,17,reg_ramdom);		//cancel��inviteһ��
	j+=sprintf(SendMsg+j,"From: sip:%s@%s;tag=%s\r\n",UAinfo->calling_number,UAinfo->server_ip,reg_ramdom);
  j+=sprintf(SendMsg+j,"To: sip:%s@%s\r\n",UAinfo->called_number,UAinfo->server_ip);  
	SIP_CountCallid(reg_ramdom);
	gsMsg.callid =MymallocCopy(gsMsg.callid,31,reg_ramdom);//cancel��inviteһ��
	j+=sprintf(SendMsg+j,"Call-ID: %s@%s\r\n",reg_ramdom,UAinfo->host_ip);
	j+=sprintf(SendMsg+j,"Contact: sip:%s@%s:%d\r\n",UAinfo->calling_number,UAinfo->host_ip,UAinfo->host_port);	
	UAinfo->cseq_value ++;
	gsMsg.cseq_num =UAinfo->cseq_value;	//cancel��inviteһ��
//	if(UAinfo->cseq_value ==0)
//		UAinfo->cseq_value =1;
	j+=sprintf(SendMsg+j,"CSeq: %d INVITE\r\n",UAinfo->cseq_value);
	j+=sprintf(SendMsg+j,"Max-Forwards: 70");
	j+=sprintf(SendMsg+j,"Allow: INVITE,CANCEL,ACK,BYE,NOTIFY,REFER,OPTIONS,INFO,MESSAGE,UPDATE\r\n");
	j+=sprintf(SendMsg+j,"User-Agent: HHDX-ARM-IPPHONE-%s\r\n",gpDeviceVer);
//	j+=sprintf(SendMsg+j,"%s: <sip:%s>\r\n","Route",UAinfo->server_ip);
	j+=sprintf(SendMsg+j,"Content-Type: application/sdp\r\n");
	j+=sprintf(SendMsg+j,"Content-Length:    \r\n\r\n");
  bp=j;
	IpInt2Str(ip_string,gpsEeprom->own_ip);	
	strcpy(UAinfo->src_ip,ip_string);	
  j=SDP_building(SendMsg,UAinfo,j);
	//Int2String(SendMsg+bp-7,j-bp);
	sprintf(ip_string,"%d",j-bp);
	strncpy(SendMsg+bp-7,ip_string,strlen(ip_string));	//strncpy����׷��"\0"
	
	NET_TxPcbLenWr(SIP_TX,j);
}
/***********************************************************************
Session Initiation Protocol (CANCEL)
    Request-Line: CANCEL sip:8000@192.168.0.10 SIP/2.0
    Message Header
        Via: SIP/2.0/UDP 192.168.0.6:5060;branch=z9hG4bK3411913063216358294;rport
        From: <sip:8001@192.168.0.10>;tag=3563447350
        To: <sip:8000@192.168.0.10>
        Call-ID: 5124135718178-102105366121757@192.168.0.6
        [Generated Call-ID: 5124135718178-102105366121757@192.168.0.6]
        CSeq: 1 CANCEL
        Max-Forwards: 70
        User-Agent: VoIP IP Phone 2.4.11.2 00a859f3dc13
        Mac: 00:a8:59:f3:dc:13
        Content-Length: 0
***********************************************************************/
void BUILD_cancel(void)
{
	int j;
	char *SendMsg;
	struct stUaInfo *UAinfo;
	UAinfo=gpsUaInfo;
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);	
	
	j=sprintf(SendMsg,"CANCEL sip:%s@%s:%d SIP/2.0\r\n",UAinfo->called_number,UAinfo->server_ip,UAinfo->server_port);
	j+=sprintf(SendMsg+j,"Via: SIP/2.0/UDP %s:%d;branch=%s\r\n",UAinfo->host_ip,UAinfo->host_port,gsMsg.via[0]);
	j+=sprintf(SendMsg+j,"From: sip:%s@%s;tag=%s\r\n",UAinfo->calling_number,UAinfo->server_ip,gsMsg.from);
  j+=sprintf(SendMsg+j,"To: sip:%s@%s\r\n",UAinfo->called_number,UAinfo->server_ip);  
	j+=sprintf(SendMsg+j,"Call-ID: %s@%s\r\n",gsMsg.callid,UAinfo->host_ip);
	j+=sprintf(SendMsg+j,"Contact: sip:%s@%s:%d\r\n",UAinfo->calling_number,UAinfo->host_ip,UAinfo->host_port);	
	j+=sprintf(SendMsg+j,"CSeq: %d CANCEL\r\n",gsMsg.cseq_num);
	j+=sprintf(SendMsg+j,"Max-Forwards: 70");
	j+=sprintf(SendMsg+j,"User-Agent: HHDX-ARM-IPPHONE-%s\r\n",gpDeviceVer);
	j+=sprintf(SendMsg+j,"Content-Length: 0\r\n\r\n");	
	
	NET_TxPcbLenWr(SIP_TX,j);
}


/***********************************************************************
ACK sip:7000@172.16.33.133:5060 SIP/2.0
Via: SIP/2.0/UDP 172.16.66.185:5060;rport;branch=z9hG4bKdd9a5236f4
From: 5000<sip:5000@172.16.33.133>;tag=7c93ee50
To: <sip:7000@172.16.33.133>;tag=gwy3h1-rs1IJ0
Call-ID: 34e523f62f9c069b2c75c28867368585@172.16.33.133
Route: <sip:172.16.33.133:5060;lr;lr>
Contact: <sip:5000@172.16.66.185:5060>
CSeq: 1 ACK
Max-Forwards: 70
Content-Length: 0
***********************************************************************/
void BUILD_ack(void)
{
	int j;
	char reg_ramdom[33];
	char *SendMsg;	
	struct stUaInfo *UAinfo;
	UAinfo=gpsUaInfo;
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	j=sprintf(SendMsg,"ACK sip:%s@%s:%d SIP/2.0\r\n",UAinfo->called_number,UAinfo->server_ip,UAinfo->server_port);
	SIP_CountBranch(reg_ramdom);	
	j+=sprintf(SendMsg+j,"Via: SIP/2.0/UDP %s:%d;branch=%s\r\n",UAinfo->host_ip,UAinfo->host_port,reg_ramdom);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.from);	
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.to);	
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.callid);
	j+=sprintf(SendMsg+j,"CSeq: %d ACK\r\n",gsMsg.cseq_num);
	j+=sprintf(SendMsg+j,"Route: <sip:%s:%d;lr>\r\n",UAinfo->server_ip,UAinfo->server_port);
	j+=sprintf(SendMsg+j,"Contact: sip:%s@%s:%d\r\n",UAinfo->calling_number,UAinfo->host_ip,UAinfo->host_port);
	j+=sprintf(SendMsg+j,"Content-Length: 0\r\n\r\n");
	
	NET_TxPcbLenWr(SIP_TX,j);
}

/***********************************************************************
BYE sip:7000@172.16.33.133:5060 SIP/2.0
Via: SIP/2.0/UDP 172.16.66.185:5060;rport;branch=z9hG4bK4eb0dfef01
From: 5000<sip:5000@172.16.33.133>;tag=7c93ee50
To: <sip:7000@172.16.33.133>;tag=gwy3h1-rs1IJ0
Call-ID: 34e523f62f9c069b2c75c28867368585@172.16.33.133
Route: <sip:172.16.33.133:5060;lr;lr>
Contact: <sip:5000@172.16.66.185:5060>
CSeq: 2 BYE
Max-Forwards: 70
Content-Length: 0
***********************************************************************/
void BUILD_bye(void)
{
	int j;
	char reg_ramdom[33];
	char *SendMsg;	
	struct stUaInfo *UAinfo;
	UAinfo=gpsUaInfo;
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	j=sprintf(SendMsg,"BYE sip:%s@%s:%d SIP/2.0\r\n",UAinfo->called_number,UAinfo->server_ip,UAinfo->server_port);
	SIP_CountBranch(reg_ramdom);	
	j+=sprintf(SendMsg+j,"Via: SIP/2.0/UDP %s:%d;branch=%s\r\n",UAinfo->host_ip,UAinfo->host_port,reg_ramdom);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.from);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.to);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.callid);
	j+=sprintf(SendMsg+j,"CSeq: %d BYE\r\n",gsMsg.cseq_num+1);
	j+=sprintf(SendMsg+j,"Route: <sip:%s:%d;lr>\r\n",UAinfo->server_ip,UAinfo->server_port);
	j+=sprintf(SendMsg+j,"Contact: sip:%s@%s:%d\r\n",UAinfo->calling_number,UAinfo->host_ip,UAinfo->host_port);
	j+=sprintf(SendMsg+j,"Content-Length: 0\r\n\r\n");
	
	NET_TxPcbLenWr(SIP_TX,j);
}


/***********************************************************************
SIP/2.0 100 Trying
From: 5000<sip:5000@172.16.33.133>;tag=7c93ee50
To: sip:7000@172.16.33.133
Call-ID: 34e523f62f9c069b2c75c28867368585@172.16.33.133
CSeq: 1 INVITE
Via: SIP/2.0/UDP 172.16.33.133:5060;branch=z9hG4bKde319c9fefef3bac.1
Via: SIP/2.0/UDP 172.16.66.185:5060;rport=5060;branch=z9hG4bKd218357ed5
Record-Route: <sip:172.16.33.133:5060;lr>
Content-Length: 0
***********************************************************************/
void BUILD_trying(void)
{
	int j;
	int i;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	j=sprintf(SendMsg,"SIP/2.0 100 Trying\r\n");
	for(i =0; i <gsMsg.via_num; i++){
		j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.via[i]);
	}
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.from);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.to);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.callid);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.cseq);
	for(i =0; i <gsMsg.re_route_num; i++){
		j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.re_route[i]);
	}	
	j+=sprintf(SendMsg+j,"Content-Length: 0\r\n\r\n");
	
	NET_TxPcbLenWr(SIP_TX,j);
}

/***********************************************************************
;����180 Ringing
;	SIP/2.0 180 Ringing
;1).  ����Viaͷ�ֶ�(�����ж��)Via: SIP/2.0/UDP  ԴIP:5060;branch=z9hG4bKxxxxxx
;2).  ����Fromͷ�ֶ� From:sip:���к�@������IP:tag= xxxxxx
;3).  ����Toͷ�ֶ�To:sip:���к�@������IP;tag= xxxxxx
;4).  ����Call-IDͷ�ֶ�Call-ID:xxxxxx@������IP
;5).  �ж�sip_route_num��=0������Routeͷ�ֶ�
;6).  ����Contactͷ�ֶ�Contact:< sip:������@����IP:5060>
;7).  ����Cseqͷ�ֶ�CSeq: xxx INVITE
;8).  ����Max-Fowordsͷ�ֶ�Max-Forwards: 70
;9).  ����Content-Lengthͷ�ֶ�Content-Length: 0
;***********************************************************************/
void BUILD_ringing(void)
{	
	int j;
	int i;
	char *SendMsg;
	struct stUaInfo *UAinfo;
	UAinfo=gpsUaInfo;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	j=sprintf(SendMsg,"SIP/2.0 180 Ringing\r\n");
	for(i =0; i <gsMsg.via_num; i++){
		j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.via[i]);
	}	
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.from);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.to);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.callid);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.cseq);
	j+=sprintf(SendMsg+j,"Contact: sip:%s@%s:%d\r\n",UAinfo->calling_number,UAinfo->host_ip,UAinfo->host_port);
	j+=sprintf(SendMsg+j,"User-Agent: HHDX-ARM-IPPHONE-%s\r\n",gpDeviceVer);
	for(i =0; i <gsMsg.re_route_num; i++){
		j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.re_route[i]);
	}		
	j+=sprintf(SendMsg+j,"Content-Length: 0\r\n\r\n");
	
	NET_TxPcbLenWr(SIP_TX,j);
}


/***********************************************************************
;����ժ������200 OK(sip_mechanism=#TX_INVITE_200_OK):
;	SIP/2.0 200 OK
;1).  ����Viaͷ�ֶ�Via: SIP/2.0/UDP  ԴIP:5060;branch=z9hG4bKxxxxxx
;2).  ����Fromͷ�ֶ� From:sip:���к�@������IP;tag= xxxxxx
;3).  ����Toͷ�ֶ�To:sip:���к�@������IP;tag= xxxxxx
;4).  ����Call-IDͷ�ֶ�Call-ID:xxxxxx@������IP
;5).  ����Contactͷ�ֶ�Contact:< sip:������@����IP:5060>
;6).  ����Cseqͷ�ֶ�CSeq: xxx INVITE
;7).  ����Max-Fowordsͷ�ֶ�Max-Forwards: 70
;8).  ����Allowͷ�ֶ�Allow: INVITE,CANCEL,ACK,BYE,NOTIFY,REFER,OPTIONS,INFO,MESSAGE,UPDATE
;9).  ����User-Agentͷ�ֶ�User-Agent:HHDX-IP-PHON-09040801
;10). ������Record-Routeͷ�ֶ�
;12). ������Routeͷ�ֶ�Route: <sip:������IP:5060;lr>
;13). ����Content-Typeͷ�ֶ�Content-Type: application/sdp
;14). ����Content-Lengthͷ�ֶ�Content-Length: xx
;15). ����SDPЭ��
;		v=0
;		o=������ 0246810 1357911 IN IP4 ����IP
;		s=A call
;		c=IN IP4 ����IP
;		t=0 0
;		m=audio 5000 RTP/AVP 8
;		a=rtpmap:8 PCMA/8000
;		a=sendrecv
;***********************************************************************/
void BUILD_invite_ok(void)
{		
	int j,bp;
	int i;
	char *SendMsg;	
	char ip_string[16];
	struct stUaInfo *UAinfo;
	UAinfo=gpsUaInfo;
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	j=sprintf(SendMsg,"SIP/2.0 200 OK\r\n");
	for(i =0; i <gsMsg.via_num; i++){
		j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.via[i]);
	}	
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.from);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.to);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.callid);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.cseq);
	j+=sprintf(SendMsg+j,"Contact: sip:%s@%s:%d\r\n",UAinfo->calling_number,UAinfo->host_ip,UAinfo->host_port);
	j+=sprintf(SendMsg+j,"User-Agent: HHDX-ARM-IPPHONE-%s\r\n",gpDeviceVer);
	j+=sprintf(SendMsg+j,"Allow: INVITE,CANCEL,ACK,BYE,NOTIFY,REFER,OPTIONS,INFO,MESSAGE,UPDATE\r\n");
	for(i =0; i <gsMsg.re_route_num; i++){
		j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.re_route[i]);
	}		
	j+=sprintf(SendMsg+j,"Content-Type: application/sdp\r\n");
	j+=sprintf(SendMsg+j,"Content-Length:    \r\n\r\n");
  bp=j;
	IpInt2Str(ip_string,gpsEeprom->own_ip);	
	strcpy(UAinfo->src_ip,ip_string);	
  j=SDP_building(SendMsg,UAinfo,j);
	//Int2String(SendMsg+bp-7,j-bp);
	sprintf(ip_string,"%d",j-bp);
	strncpy(SendMsg+bp-7,ip_string,strlen(ip_string));	//strncpy����׷��"\0"
	
	NET_TxPcbLenWr(SIP_TX,j);
}

/***********************************************************************
SIP/2.0 200 OK
From: 5000<sip:5000@172.16.33.133>;tag=7c93ee50
To: sip:7000@172.16.33.133;tag=gwy3h1-rs1IJ0
Call-ID: 34e523f62f9c069b2c75c28867368585@172.16.33.133
CSeq: 2 BYE
Via: SIP/2.0/UDP 172.16.33.133:5060;branch=z9hG4bK97e1a88b0633bbac.1
Via: SIP/2.0/UDP 172.16.66.185:5060;rport=5060;branch=z9hG4bK4eb0dfef01
User-Agent: ShangHaiHuaHeng 091130VR4X
Content-Length: 0
***********************************************************************/
void BUILD_bye_cancel_ok(void)
{
	int j;
	int i;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	j=sprintf(SendMsg,"SIP/2.0 200 OK\r\n");
	
	for(i =0; i <gsMsg.via_num; i++){
		j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.via[i]);
	}
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.from);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.to);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.callid);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.cseq);
	for(i =0; i <gsMsg.re_route_num; i++){
		j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.re_route[i]);
	}	
	j+=sprintf(SendMsg+j,"Content-Length: 0\r\n\r\n");
	
	NET_TxPcbLenWr(SIP_TX,j);
}

/***********************************************************************
Session Initiation Protocol (487)
    Status-Line: SIP/2.0 487 Request Terminated
    Message Header
        Via: SIP/2.0/UDP 192.168.0.10:5060;branch=z9hG4bK-bda4c0f20
        To: <sip:8000@192.168.0.22>;tag=2f31df36
        From: <sip:8001@192.168.0.10>;tag=56bda4c0f20
        Call-ID: bda520a0-00E066-0000-B06F43-bda4c0f2@192.168.0.10
        [Generated Call-ID: bda520a0-00E066-0000-B06F43-bda4c0f2@192.168.0.10]
        CSeq: 0 INVITE
        User-Agent: eyeBeam release 1011d stamp 40820
        Content-Length: 0
***********************************************************************/
void BUILD_487(void)
{
	int j;
	int i;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	j=sprintf(SendMsg,"SIP/2.0 487 Request Terminated\r\n");
	
	for(i =0; i <gsMsg.via_num; i++){
		j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.via[i]);
	}
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.from);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.to);
	j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.callid);
	j+=sprintf(SendMsg+j,"CSeq: %d INVITE\r\n",gsMsg.cseq_num);
	j+=sprintf(SendMsg+j,"User-Agent: HHDX-ARM-IPPHONE-%s\r\n",gpDeviceVer);
	for(i =0; i <gsMsg.re_route_num; i++){
		j+=sprintf(SendMsg+j,"%s\r\n",gsMsg.re_route[i]);
	}	
	j+=sprintf(SendMsg+j,"Content-Length: 0\r\n\r\n");
	
	NET_TxPcbLenWr(SIP_TX,j);
}

// /***********************************************************************
// ��̬�˿�������
// ***********************************************************************/
// void BUILD_port_request(void)
// {
// 	int j;
// 	char *SendMsg;	
// 	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
// 	/*�˿���������
// 	�ն�>>��������id=�ն�Ψһ��š�type=�������ͣ�
// 	Port_Request={"id":"1222","type":"port_request"}
// 	������>>�նˣ�port=��̬����Ķ˿ںţ�
// 	Port_Response={"port":"8888","status":"success"}*/

// 	j=sprintf(SendMsg,"{\"id\":\"%s\",\"type\":\"port_request\"}\r\n", gpsEeprom->own_id);

// 	NET_TxPcbLenWr(SIP_TX,j);
	
// 	// ���ö˿�����״̬
// 	gpsUaInfo->port_request_timer = PORT_REQUEST_TIMEOUT;
// 	gpsUaInfo->port_request_retry++;
// }

// /***********************************************************************
// �˿���֤����
// ***********************************************************************/
// void BUILD_port_verify(u16 port)
// {
// 	int j;
// 	char *SendMsg;	
// 	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
// 	/*�˿���֤����
// 	�ն�>>��������ͨ����̬�˿ڷ�����֤��Ϣ��
// 	Port_Verify={"id":"1222","type":"port_verify","port":"8888"}*/

// 	j=sprintf(SendMsg,"{\"id\":\"%s\",\"type\":\"port_verify\",\"port\":\"%d\"}\r\n", gpsEeprom->own_id, port);

// 	NET_TxPcbLenWr(SIP_TX,j);
// }

// /***********************************************************************
// �ļ����俪ʼ����
// ***********************************************************************/
// void BUILD_file_start(const char* filename, u32 filesize)
// {
// 	int j;
// 	char *SendMsg;	
// 	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
// 	/*�ļ����俪ʼ����
// 	File_Start={"id":"1222","type":"file_start","filename":"record.wav","size":"102400"}*/
	
// 	j=sprintf(SendMsg,"{\"id\":\"%s\",\"type\":\"file_start\",\"filename\":\"%s\",\"size\":\"%lu\"}\r\n", 
// 		gpsEeprom->own_id, filename, filesize);

// 	NET_TxPcbLenWr(SIP_TX,j);
// }

// /***********************************************************************
// �ļ����ݿ鴫�亯��
// ***********************************************************************/
// void BUILD_file_data(u16 block_num, u8* data, u16 data_len)
// {
// 	int j;
// 	char *SendMsg;	
// 	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
// 	/*�ļ����ݿ鴫�� - �򻯰棬ʵ��Ӧ���ö����ƴ���
// 	File_Data={"id":"1222","type":"file_data","block":"1","data":"base64_encoded_data"}*/
	
// 	j=sprintf(SendMsg,"{\"id\":\"%s\",\"type\":\"file_data\",\"block\":\"%d\",\"length\":\"%d\"}\r\n", 
// 		gDeviceId, block_num, data_len);
	
// 	// ע�⣺����򻯴�����ʵ��Ӧ�ý����������ݽ���Base64�����ֱ�ӷ��Ͷ�����
// 	memcpy(SendMsg + j, data, data_len);
// 	j += data_len;
	
// 	NET_TxPcbLenWr(SIP_TX,j);
// }

// /***********************************************************************
// �ļ������������
// ***********************************************************************/
// void BUILD_file_end(u16 total_blocks, u32 total_size)
// {
// 	int j;
// 	char *SendMsg;	
// 	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
// 	/*�ļ������������
// 	File_End={"id":"1222","type":"file_end","total_blocks":"100","total_size":"102400"}*/
	
// 	j=sprintf(SendMsg,"{\"id\":\"%s\",\"type\":\"file_end\",\"total_blocks\":\"%d\",\"total_size\":\"%lu\"}\r\n", 
// 		gDeviceId, total_blocks, total_size);
	
// 	NET_TxPcbLenWr(SIP_TX,j);
// }

// // ¼���ļ�����ʾ������
// void transfer_recording_file(const char* filename)
// {
//     // �������������ں�����ͷ��C89��׼��
//     u8 sample_data[] = "Recording file content data block...";
//     u32 file_size = 1024;  // ����1KB¼���ļ�
//     u16 total_blocks;
//     u16 block;
//     u16 data_len;
    
//     // �����ܿ���
//     total_blocks = (file_size + FILE_BLOCK_SIZE - 1) / FILE_BLOCK_SIZE;
    
//     // 1. ��������̬�˿�
//     if(!gpsUaInfo->port_negotiated)
//     {
//         printf("Requesting dynamic port for file transfer...\r\n");
//         BUILD_port_request();
//         gpsUaInfo->port_request_timer = PORT_REQUEST_TIMEOUT;
//         gpsUaInfo->port_request_retry = 0;
//         return;
//     }
    
//     // 2. ��֤�˿ڿ�����
//     if(!gpsUaInfo->port_verified)
//     {
//         printf("Verifying port %d...\r\n", gpsUaInfo->dynamic_port);
//         BUILD_port_verify(gpsUaInfo->dynamic_port);
//         return;
//     }
    
//     // 3. ��ʼ�ļ����� (ģ��¼���ļ�)
//     printf("Starting file transfer: %s\r\n", filename);
    
//     // �����ļ���ʼ��Ϣ
//     BUILD_file_start(filename, file_size);
    
//     // �ֿ鷢�����ݣ�C89 forѭ����ʽ��
//     for(block = 1; block <= total_blocks; block++)
//     {
//         data_len = (block == total_blocks) ? 
//                    (file_size % FILE_BLOCK_SIZE) : FILE_BLOCK_SIZE;
//         BUILD_file_data(block, sample_data, data_len);
//     }
    
//     // �����ļ�������Ϣ
//     BUILD_file_end(total_blocks, file_size);
    
//     printf("File transfer completed: %d blocks, %d bytes\r\n", 
//            total_blocks, file_size);
// }

// // JSON��ʽ������Ϣ���ͺ���
// void SEND_CID_JSON(const char* msg_type, const char* status, const char* call_number)
// {
//     char json_msg[256];
//     char timestamp[16];
//     char *SendMsg;
//     int msg_len;
    
//     // ��ȡ��ǰʱ���
//     sprintf(timestamp, "%04d%02d%02d%02d%02d%02d", 
//         gsDate.year, gsDate.month, gsDate.day, 
//         gsDate.hour, gsDate.min, gsDate.sec);
    
//     // ����JSON��Ϣ
//     if(call_number != NULL)
//     {
//         // ��������������Ϣ�������¼���
//         msg_len = sprintf(json_msg, 
//             "{\"id\":\"%s\",\"number\":\"%s\",\"call\":\"%s\",\"timestamp\":\"%s\",\"type\":\"%s\",\"status\":\"%s\"}\r\n",
//             gpsEeprom->own_id, gpsEeprom->own_num, call_number, timestamp, msg_type, status);
//     }
//     else
//     {
//         // ����������������Ϣ������/�Ҷ��¼���
//         msg_len = sprintf(json_msg, 
//             "{\"id\":\"%s\",\"number\":\"%s\",\"timestamp\":\"%s\",\"type\":\"%s\",\"status\":\"%s\"}\r\n",
//             gpsEeprom->own_id, gpsEeprom->own_num, timestamp, msg_type, status);
//     }
    
//     // ���緢��
//     SendMsg = (char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
//     strcpy(SendMsg, json_msg);
//     NET_TxPcbLenWr(SIP_TX, msg_len);
    
//     // ���ڷ���
//     printf("CID_JSON: %s", json_msg);
// }

// // ��ʼ����̬�˿�Э��
// void init_dynamic_port_negotiation(void)
// {
//     printf("Initializing dynamic port negotiation...\r\n");
    
//     // �����˿�����
//     if(gpsUaInfo->dynamic_port == 0)
//     {
//         transfer_recording_file("recording_001.wav");
//     }
// }


