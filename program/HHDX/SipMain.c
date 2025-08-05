#include "usart.h"
#include "malloc.h"
#include "net_rxtx.h"
#include "stdio.h"
#include "string.h" 
#include "timer.h"
#include "SipFunction.h"
#include "24cxx.h" 
#include "tel_func.h"
#include "wm8978.h"	 
#include "Audio.h"
#include "i2s.h"
#include "HhdxMsg.h"
#include "usart3_FSK.h"

unsigned int gSipRamdom1=0x8a2e4b6e;
unsigned int gSipRamdom2=0x7b5c1f39;
//2023年01月01日 12:18:30

struct stUaInfo *gpsUaInfo;
struct stDate gsDate;

u8 gSipMechanism =0;
u8 gSipTxFlow =0;



//SIP发送流程
void SIP_TxFlow(void)
{

// 检查重传标志
    if(gsSipTxFlag.retry)
    {
        gsSipTxFlag.retry = 0;
        
        // 根据最后发送的消息类型重发
        switch(gpsUaInfo->last_msg_type)
        {
            case MSG_TYPE_START:
                BUILD_CID_start();
                break;
            case MSG_TYPE_OFFHOOK:
                BUILD_CID_offhook();
                break;
            case MSG_TYPE_STOP:
                BUILD_CID_stop();
                break;
            case MSG_TYPE_PORT_REQUEST:
                BUILD_port_request();
                break;
            case MSG_TYPE_PORT_VERIFY:
                BUILD_port_verify(gpsUaInfo->dynamic_port);
                break;
        }
        return;
    }

	switch(gSipTxFlow)
	{
		case 0:
			break;
//----------------- calling ------------------					
		case TX_INVITE:		
			BUILD_invite();			
//			ANALYSIS_RxParamInit();
			gSipMechanism =TX_INVITE;			
			gsSipTxFlag.invite =1;
			gSipTxFlow =0;
			break;		
		case TX_ACK:
			BUILD_ack();
			gSipMechanism =TX_ACK;			
			gsSipTxFlag.ack =1;
			gSipTxFlow =0;
			break;	
//----------------- called ------------------		
		case TX_INVITE_100_TRYING:
			BUILD_trying();
			gSipMechanism =TX_INVITE_100_TRYING;
			gSipTxFlow =TX_180_RINGING;
			break;
		case TX_180_RINGING:
			BUILD_ringing();
			gSipMechanism =TX_180_RINGING;
			gsSipTxFlag.ringing =1;
			gSipTxFlow =0;		
//			I2S2_SampleRate_Set(44100);	//设置采样
			AUDIO_play_mode();		//免提放Music
			TEL_HandfreeSPK_on();	//打开免提SPK	
			break;
		case TX_INVITE_200_OK:
			BUILD_invite_ok();
			gSipMechanism =TX_INVITE_200_OK;			
			gsSipTxFlag.ok_inv =1;
			gSipTxFlow =0;
			break;
			
//----------------- calling/ed ------------------				
		case TX_BYE:
			BUILD_bye();
			gSipMechanism =TX_BYE;			
			gsSipTxFlag.bye =1;
			gSipTxFlow =0;			
			break;	
		case TX_CANCEL:
			BUILD_cancel();
			gSipMechanism =TX_CANCEL;			
			gsSipTxFlag.cancel =1;
			gSipTxFlow =0;			
			break;	
		
		case TX_BYE_200_OK:
			BUILD_bye_cancel_ok();
			gSipMechanism =TX_BYE_200_OK;			
			gsSipTxFlag.ok_bye =1;
			gSipTxFlow =0;
			TEL_OffInit();
			break;
		
		case TX_CANCEL_200_OK:
			BUILD_bye_cancel_ok();
			BUILD_487();
			gSipMechanism =TX_CANCEL_200_OK;
			gSipTxFlow =0;
			TEL_OffInit();
			break;
		
		default:
			break;
	}
}


//-----------------------------------------------------------------------------
//【  版          本  】v1.0
//【  函  数  名  称  】
//【 创建人及创建时间 】snowwind 2010-01-08
//【 修改人及修改时间 】
//【  修  改  原  因  】
//【  功  能  描  述  】Count date-time
//-----------------------------------------------------------------------------
void SIP_date(void)
{
//second	
	gsDate.sec++;
	if(gsDate.sec>=120){	//500ms ++
		gsDate.sec=0;
//minute		
		gsDate.min++;	
		if(gsDate.min>=60){
//hour			
			gsDate.min=0;
			gsDate.hour++;
			if(gsDate.hour>=24){
//day			
				gsDate.hour=0;
				gsDate.day++;
				if(gsDate.day>=31){
//month			
					gsDate.day=0;
					gsDate.month++;
					if(gsDate.month>=12){
//year			
						gsDate.month=0;
						gsDate.year++;					
					}
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------
//【  版          本  】v1.0
//【  函  数  名  称  】
//【 创建人及创建时间 】snowwind 2010-01-08
//【 修改人及修改时间 】
//【  修  改  原  因  】
//【  功  能  描  述  】Call_ID:(Mac后4字节+gSipRamdom1)-年月日时分秒-(IP address+gSipRamdom2)
//						如: Call_ID:xxxxxxxx-xxxxxxxxxxxx-xxxxxxxx
//-----------------------------------------------------------------------------
void SIP_CountCallid(char *call_id)
{
	char hex_buf[17];	//Length max=16+'\0'=17
	unsigned int int_value=0;
	unsigned short short_value=0;
	*call_id='\0';
//(Mac后4字节+gSipRamdom1)	
	int_value=gSipRamdom1+((unsigned int)((lwipdev.mac[2]<<24)|(lwipdev.mac[3]<<16)|(lwipdev.mac[4]<<8)|(lwipdev.mac[5]<<0)));
	Hex2Asc(&int_value,hex_buf,(sizeof(unsigned int)<<1));
	strcat(call_id,hex_buf);
	strcat(call_id,"-");

//年月日时分秒
	//年月日时
	int_value=((unsigned int)((gsDate.year<<24)|(gsDate.month<<16)|(gsDate.day<<8)|(gsDate.hour<<0)));
	Hex2Asc(&int_value,hex_buf,(sizeof(unsigned int)<<1));
	strcat(call_id,hex_buf);
		
	//分秒
	short_value=((unsigned short)((gsDate.min<<8)|gsDate.sec));
	Hex2Asc(&short_value,hex_buf,(sizeof(unsigned short)<<1));
	strcat(call_id,hex_buf);
	strcat(call_id,"-");	

//(IP address+gSipRamdom2)	
	int_value=gSipRamdom2+HOST_IP;	
	Hex2Asc(&int_value,hex_buf,(sizeof(unsigned int)<<1));
	strcat(call_id,hex_buf);

}

//-----------------------------------------------------------------------------
//【  版          本  】v1.0
//【  函  数  名  称  】
//【 创建人及创建时间 】snowwind 2010-01-08
//【 修改人及修改时间 】
//【  修  改  原  因  】
//【  功  能  描  述  】From/To tag=(gSipRamdom1+ntohl(gSipRamdom2))(gSipRamdom1-ntohl(gSipRamdom2))
//						如: tag=xxxxxxxxxxxxxxxx
//-----------------------------------------------------------------------------
void SIP_CountTag(char *tag)
{	
	char hex_buf[17];	//Length max=16+'\0'=17
	unsigned int int_value=0;
	*tag='\0';
	int_value=gSipRamdom1+ntohl(gSipRamdom2);
	Hex2Asc(&int_value,hex_buf,(sizeof(unsigned int)<<1));
	strcat(tag,hex_buf);
	
	int_value=gSipRamdom1-ntohl(gSipRamdom2);
	Hex2Asc(&int_value,hex_buf,(sizeof(unsigned int)<<1));
	strcat(tag,hex_buf);
}

//-----------------------------------------------------------------------------
//【  版          本  】v1.0
//【  函  数  名  称  】
//【 创建人及创建时间 】snowwind 2010-01-08
//【 修改人及修改时间 】
//【  修  改  原  因  】
//【  功  能  描  述  】Via: branch=(gSipRamdom1+gSipRamdom2+ntohl(gSipRamdom1))
//						如: branch=z9hG4bKxxxxxxxxxxxxxxxx
//-----------------------------------------------------------------------------
void SIP_CountBranch(char *branch)
{
	char hex_buf[17];	//Length max=16+'\0'=17
	unsigned int int_value=0;
	*branch='\0';
	strcat(branch,"z9hG4bK");
	int_value=gSipRamdom1+gSipRamdom2+ntohl(gSipRamdom1);
	Hex2Asc(&int_value,hex_buf,(sizeof(unsigned int)<<1));
	strcat(branch,hex_buf);
	int_value=gSipRamdom1+gSipRamdom2+ntohl(gSipRamdom2);
	Hex2Asc(&int_value,hex_buf,(sizeof(unsigned int)<<1));
	strcat(branch,hex_buf);	
}


/*
REGISTER sip:172.16.1.133 SIP/2.0
Via: SIP/2.0/UDP 172.16.180.112:5060;rport;branch=z9hG4bK2e793bd9ea
From: "9012" <sip:9012@172.16.1.133>;tag=762ea54f
To: "9012" <sip:9012@172.16.1.133>
Call-ID: 652ff2ba2ee8c53c33ecfe330c24e0e7@172.16.180.112
Contact: <sip:9012@172.16.180.112:5060>
CSeq: 1 REGISTER
Max-Forwards: 70
Expires: 1200
Allow: INVITE,CANCEL,ACK,BYE,NOTIFY,REFER,OPTIONS,INFO,MESSAGE,UPDATE
User-Agent: CM5K-PHONE  (908240)
Content-Length: 0
*/
void SIP_UainfoInit(void)
{
	char ip_string[16];	
	
//Host IP =	192.168.0.30
	IpInt2Str(ip_string,gpsEeprom->own_ip);	
	strcpy(gpsUaInfo->host_ip,ip_string);	
//Server IP = 192.168.0.5	
	IpInt2Str(ip_string,gpsEeprom->sv_ip);			
	strcpy(gpsUaInfo->server_ip,ip_string);	
//Host port = 5060
	gpsUaInfo->host_port=SIP_PORT;		//5060
//Server port = 5060	
	gpsUaInfo->server_port=SIP_PORT;	//5060
	
	strcpy(gpsUaInfo->calling_number,(const char*)gpsEeprom->own_num);	//9012
	strcpy((char *)gFskOwnBuf,(char *)gpsEeprom->own_num);
	printf("num=%s\r\n",gpsUaInfo->calling_number);
	gDialNumLen=0;
	gpsUaInfo->called_number[gDialNumLen]='\0';
//	strcpy(gpsUaInfo->called_number,"9004");	//9004//
	
	gpsUaInfo->expires=SIP_EXPIRES;	//1200s
	
}




//-----------------------------------------------------------------------------
//1.如果注册成功则每隔60s注册一次
//2.如果没有注册成功每隔3秒注册一次，注册20次后，停120s
//-----------------------------------------------------------------------------
void SIP_RegisterTx(void)
{
	static char reg_times=0;	
	char flag;
	
	flag =0;
//	gsSipRxFlag.ok_reg =1;//test?????? 收到OK =1	
//1:register successful
	if(gsSipRxFlag.ok_reg){
		if(gpsT3->sip_reg > 600){	//600*100ms =60s
			flag=1;
		}
	}
//0:register fail
	else{
		if(reg_times>=20){//register is more than 20 times
			if(gpsT3->sip_reg>=1200){	//1200*100ms =120s time interval			
				reg_times=0;
				flag=1;
			}
		}
		else{
			if(gpsT3->sip_reg>=30){	//30*100ms =3s time interval
				flag=1;
			}
		}
	}
	
	if(flag ==1){//Send register
		gpsT3->sip_reg =0;
		BUILD_register();
	}	
}


