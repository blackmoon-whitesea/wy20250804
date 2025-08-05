
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

// 函数声明
void SEND_CID_JSON(const char* msg_type, const char* status, const char* call_number);

char *gpDeviceVer = "2301091740";
/***********************************************************************
"DTMF=" + 号码(1位)
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
"start" + 编号(4位) + 本机号(16位) + 来电号(16位) 
***********************************************************************/
void BUILD_CID_start(void)
{
	int i;
	
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
	
	// 使用新的JSON发送函数
	SEND_CID_JSON("incoming_call", "start", (char*)gFskCidBuf);

	//重传设置
	gpsUaInfo->waiting_response = 1;
    gpsUaInfo->retry_count = 0;
    gpsUaInfo->max_retry = 3;
    gpsUaInfo->retry_timeout = 2000;  // 2秒
    gpsUaInfo->retry_timer = 2000;
    gpsUaInfo->last_msg_type = MSG_TYPE_START;
}

/***********************************************************************
"stop" + 编号(4位)
***********************************************************************/
void BUILD_CID_stop(void)
{
	// 使用新的JSON发送函数
	SEND_CID_JSON("call_end", "stop", NULL);

	//重传设置
	gpsUaInfo->waiting_response = 1;
    gpsUaInfo->retry_count = 0;
    gpsUaInfo->max_retry = 3;
    gpsUaInfo->retry_timeout = 2000;  // 2秒
    gpsUaInfo->retry_timer = 2000;
    gpsUaInfo->last_msg_type = MSG_TYPE_STOP;
}

/***********************************************************************
"stop" + 编号(4位)
***********************************************************************/
void BUILD_CID_offhook(void)
{
	// 使用新的JSON发送函数
	SEND_CID_JSON("call_answer", "offhook", NULL);

	//重传设置
	gpsUaInfo->waiting_response = 1;
    gpsUaInfo->retry_count = 0;
    gpsUaInfo->max_retry = 3;
    gpsUaInfo->retry_timeout = 2000;  // 2秒
    gpsUaInfo->retry_timer = 2000;
    gpsUaInfo->last_msg_type = MSG_TYPE_OFFHOOK;
}


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
;主叫摘机发送INVITE(sip_mechanism=#TX_INVITE):
;	INVITE sip:Called@server_ip:5060 SIP/2.0
;1).  产生Via头字段Via: SIP/2.0/UDP 源IP:5060;branch=z9hG4bKxxxxxx
;2).  产生From头字段 From:sip:主叫号@服务器IP:tag= xxxxxx
;3).  产生To头字段To:sip:被叫号@服务器IP
;4).  产生Call-ID头字段Call-ID:xxxxxx@服务器IP
;5).  产生Contact头字段Contact:< sip:本机号@本机IP:5060>
;6).  产生Cseq头字段CSeq: xxx INVITE
;7).  产生Max-Fowords头字段Max-Forwards: 70
;8).  产生Allow头字段Allow: INVITE,CANCEL,ACK,BYE,NOTIFY,REFER,OPTIONS,INFO,MESSAGE,UPDATE
;9).  产生User-Agent头字段User-Agent:HHDX-IP-PHON-09040801
;10). 不产生Record-Route头字段
;12). 不产生Route头字段Route: <sip:服务器IP:5060;lr>
;13). 产生Content-Type头字段Content-Type: application/sdp
;14). 产生Content-Length头字段Content-Length: xx
;15). 产生SDP协议
;		v=0
;		o=本机号 0246810 1357911 IN IP4 本机IP
;		s=A call
;		c=IN IP4 本机IP
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
	gsMsg.via[0] =MymallocCopy(gsMsg.via[0],24,reg_ramdom);		//cancel与invite一致
	j+=sprintf(SendMsg+j,"Via: SIP/2.0/UDP %s:%d;branch=%s\r\n",UAinfo->host_ip,UAinfo->host_port,reg_ramdom);
	SIP_CountTag(reg_ramdom);
	gsMsg.from =MymallocCopy(gsMsg.from,17,reg_ramdom);		//cancel与invite一致
	j+=sprintf(SendMsg+j,"From: sip:%s@%s;tag=%s\r\n",UAinfo->calling_number,UAinfo->server_ip,reg_ramdom);
  j+=sprintf(SendMsg+j,"To: sip:%s@%s\r\n",UAinfo->called_number,UAinfo->server_ip);  
	SIP_CountCallid(reg_ramdom);
	gsMsg.callid =MymallocCopy(gsMsg.callid,31,reg_ramdom);//cancel与invite一致
	j+=sprintf(SendMsg+j,"Call-ID: %s@%s\r\n",reg_ramdom,UAinfo->host_ip);
	j+=sprintf(SendMsg+j,"Contact: sip:%s@%s:%d\r\n",UAinfo->calling_number,UAinfo->host_ip,UAinfo->host_port);	
	UAinfo->cseq_value ++;
	gsMsg.cseq_num =UAinfo->cseq_value;	//cancel与invite一致
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
	strncpy(SendMsg+bp-7,ip_string,strlen(ip_string));	//strncpy不会追加"\0"
	
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
;发送180 Ringing
;	SIP/2.0 180 Ringing
;1).  产生Via头字段(可能有多个)Via: SIP/2.0/UDP  源IP:5060;branch=z9hG4bKxxxxxx
;2).  产生From头字段 From:sip:主叫号@服务器IP:tag= xxxxxx
;3).  产生To头字段To:sip:被叫号@服务器IP;tag= xxxxxx
;4).  产生Call-ID头字段Call-ID:xxxxxx@服务器IP
;5).  判断sip_route_num否=0？产生Route头字段
;6).  产生Contact头字段Contact:< sip:本机号@本机IP:5060>
;7).  产生Cseq头字段CSeq: xxx INVITE
;8).  产生Max-Fowords头字段Max-Forwards: 70
;9).  产生Content-Length头字段Content-Length: 0
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
;被叫摘机发送200 OK(sip_mechanism=#TX_INVITE_200_OK):
;	SIP/2.0 200 OK
;1).  产生Via头字段Via: SIP/2.0/UDP  源IP:5060;branch=z9hG4bKxxxxxx
;2).  产生From头字段 From:sip:主叫号@服务器IP;tag= xxxxxx
;3).  产生To头字段To:sip:被叫号@服务器IP;tag= xxxxxx
;4).  产生Call-ID头字段Call-ID:xxxxxx@服务器IP
;5).  产生Contact头字段Contact:< sip:本机号@本机IP:5060>
;6).  产生Cseq头字段CSeq: xxx INVITE
;7).  产生Max-Fowords头字段Max-Forwards: 70
;8).  产生Allow头字段Allow: INVITE,CANCEL,ACK,BYE,NOTIFY,REFER,OPTIONS,INFO,MESSAGE,UPDATE
;9).  产生User-Agent头字段User-Agent:HHDX-IP-PHON-09040801
;10). 不产生Record-Route头字段
;12). 不产生Route头字段Route: <sip:服务器IP:5060;lr>
;13). 产生Content-Type头字段Content-Type: application/sdp
;14). 产生Content-Length头字段Content-Length: xx
;15). 产生SDP协议
;		v=0
;		o=本机号 0246810 1357911 IN IP4 本机IP
;		s=A call
;		c=IN IP4 本机IP
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
	strncpy(SendMsg+bp-7,ip_string,strlen(ip_string));	//strncpy不会追加"\0"
	
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

/***********************************************************************
动态端口请求函数
***********************************************************************/
void BUILD_port_request(void)
{
	int j;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	/*端口请求命令
	终端>>服务器（id=终端唯一编号、type=请求类型）
	Port_Request={"id":"1222","type":"port_request"}
	服务器>>终端（port=动态分配的端口号）
	Port_Response={"port":"8888","status":"success"}*/
	
	j=sprintf(SendMsg,"{\"id\":\"%s\",\"type\":\"port_request\"}\r\n", "1222");
	
	NET_TxPcbLenWr(SIP_TX,j);
	
	// 设置端口请求状态
	gpsUaInfo->port_request_timer = PORT_REQUEST_TIMEOUT;
	gpsUaInfo->port_request_retry++;
}

/***********************************************************************
端口验证函数
***********************************************************************/
void BUILD_port_verify(u16 port)
{
	int j;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	/*端口验证命令
	终端>>服务器（通过动态端口发送验证消息）
	Port_Verify={"id":"1222","type":"port_verify","port":"8888"}*/
	
	j=sprintf(SendMsg,"{\"id\":\"%s\",\"type\":\"port_verify\",\"port\":\"%d\"}\r\n", "1222", port);
	
	NET_TxPcbLenWr(SIP_TX,j);
}

/***********************************************************************
文件传输开始函数
***********************************************************************/
void BUILD_file_start(const char* filename, u32 filesize)
{
	int j;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	/*文件传输开始命令
	File_Start={"id":"1222","type":"file_start","filename":"record.wav","size":"102400"}*/
	
	j=sprintf(SendMsg,"{\"id\":\"%s\",\"type\":\"file_start\",\"filename\":\"%s\",\"size\":\"%lu\"}\r\n", 
		"1222", filename, filesize);
	
	NET_TxPcbLenWr(SIP_TX,j);
}

/***********************************************************************
文件数据块传输函数
***********************************************************************/
void BUILD_file_data(u16 block_num, u8* data, u16 data_len)
{
	int j;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	/*文件数据块传输 - 简化版，实际应该用二进制传输
	File_Data={"id":"1222","type":"file_data","block":"1","data":"base64_encoded_data"}*/
	
	j=sprintf(SendMsg,"{\"id\":\"%s\",\"type\":\"file_data\",\"block\":\"%d\",\"length\":\"%d\"}\r\n", 
		"1222", block_num, data_len);
	
	// 注意：这里简化处理，实际应该将二进制数据进行Base64编码或直接发送二进制
	memcpy(SendMsg + j, data, data_len);
	j += data_len;
	
	NET_TxPcbLenWr(SIP_TX,j);
}

/***********************************************************************
文件传输结束函数
***********************************************************************/
void BUILD_file_end(u16 total_blocks, u32 total_size)
{
	int j;
	char *SendMsg;	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	/*文件传输结束命令
	File_End={"id":"1222","type":"file_end","total_blocks":"100","total_size":"102400"}*/
	
	j=sprintf(SendMsg,"{\"id\":\"%s\",\"type\":\"file_end\",\"total_blocks\":\"%d\",\"total_size\":\"%lu\"}\r\n", 
		"1222", total_blocks, total_size);
	
	NET_TxPcbLenWr(SIP_TX,j);
}

// 录音文件传输示例函数
void transfer_recording_file(const char* filename)
{
    // 变量声明必须在函数开头（C89标准）
    u8 sample_data[] = "Recording file content data block...";
    u32 file_size = 1024;  // 假设1KB录音文件
    u16 total_blocks;
    u16 block;
    u16 data_len;
    
    // 计算总块数
    total_blocks = (file_size + FILE_BLOCK_SIZE - 1) / FILE_BLOCK_SIZE;
    
    // 1. 首先请求动态端口
    if(!gpsUaInfo->port_negotiated)
    {
        printf("Requesting dynamic port for file transfer...\r\n");
        BUILD_port_request();
        gpsUaInfo->port_request_timer = PORT_REQUEST_TIMEOUT;
        gpsUaInfo->port_request_retry = 0;
        return;
    }
    
    // 2. 验证端口可用性
    if(!gpsUaInfo->port_verified)
    {
        printf("Verifying port %d...\r\n", gpsUaInfo->dynamic_port);
        BUILD_port_verify(gpsUaInfo->dynamic_port);
        return;
    }
    
    // 3. 开始文件传输 (模拟录音文件)
    printf("Starting file transfer: %s\r\n", filename);
    
    // 发送文件开始消息
    BUILD_file_start(filename, file_size);
    
    // 分块发送数据（C89 for循环格式）
    for(block = 1; block <= total_blocks; block++)
    {
        data_len = (block == total_blocks) ? 
                   (file_size % FILE_BLOCK_SIZE) : FILE_BLOCK_SIZE;
        BUILD_file_data(block, sample_data, data_len);
    }
    
    // 发送文件结束消息
    BUILD_file_end(total_blocks, file_size);
    
    printf("File transfer completed: %d blocks, %d bytes\r\n", 
           total_blocks, file_size);
}

// JSON格式来电信息发送函数
void SEND_CID_JSON(const char* msg_type, const char* status, const char* call_number)
{
    char json_msg[256];
    char timestamp[16];
    char *SendMsg;
    int msg_len;
    
    // 获取当前时间戳
    sprintf(timestamp, "%04d%02d%02d%02d%02d%02d", 
        gsDate.year, gsDate.month, gsDate.day, 
        gsDate.hour, gsDate.min, gsDate.sec);
    
    // 构建JSON消息
    if(call_number != NULL)
    {
        // 包含来电号码的消息（呼入事件）
        msg_len = sprintf(json_msg, 
            "{\"id\":\"%s\",\"number\":\"%s\",\"call\":\"%s\",\"timestamp\":\"%s\",\"type\":\"%s\",\"status\":\"%s\"}\r\n",
            "1222", gFskOwnBuf, call_number, timestamp, msg_type, status);
    }
    else
    {
        // 不包含来电号码的消息（接听/挂断事件）
        msg_len = sprintf(json_msg, 
            "{\"id\":\"%s\",\"number\":\"%s\",\"timestamp\":\"%s\",\"type\":\"%s\",\"status\":\"%s\"}\r\n",
            "1222", gFskOwnBuf, timestamp, msg_type, status);
    }
    
    // 网络发送
    SendMsg = (char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
    strcpy(SendMsg, json_msg);
    NET_TxPcbLenWr(SIP_TX, msg_len);
    
    // 串口发送
    printf("CID_JSON: %s", json_msg);
}

// 初始化动态端口协商
void init_dynamic_port_negotiation(void)
{
    printf("Initializing dynamic port negotiation...\r\n");
    
    // 启动端口请求
    if(gpsUaInfo->dynamic_port == 0)
    {
        transfer_recording_file("recording_001.wav");
    }
}


