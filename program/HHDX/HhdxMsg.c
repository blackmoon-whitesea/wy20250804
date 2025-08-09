#include "stdio.h"
#include "string.h" 
#include "timer.h"
#include "usart.h" 
#include "HhdxMsg.h" 
#include "SipConstant.h" 
#include "SipFunction.h"
#include "24cxx.h" 
#include "malloc.h"
#include "delay.h"
#include "net_rxtx.h"
#include "key.h"

struct stHhdxFlag gsHhdxFlag;

int rec_recive_cmd=0;
int dynamic_port=0;	
/*
1. TEL_param_fromPC={......}
2. TEL_func_fromPC={......}
*/
//��������������
int HHDX_Command(char *p_line)
{
	unsigned int i;
	int j=0;	//�ֶγ���
	char ibuffer[18]; // �����л����� 
	for(i=0;i<18;i++)
	{
//1. ��һ���ֽڿ�ʼ��ibuffer		
		if(j==0) ibuffer[0]='\0';
//2. ��һ��'=' 			
    if(*(p_line+i)=='=')
		{
			ibuffer[j]='\0';	//�����ָ���'='ֹͣ
			gTempPointer =(p_line+i+1);	//'='��һ����ַ
			break;
		}
//3. ��¼"="ǰ�ַ�
    ibuffer[j]= *(p_line+i);
		j++;
	}
//ȡ��TEL_param_fromPC��TEL_func_fromPC����
	if(i!=18)
	{
		if(!strcmp(ibuffer,"TEL_func_fromPC")){
			printf("F=%s\r\n",ibuffer);
			return HHDX_FUNC;
		}
		if(!strcmp(ibuffer,"TEL_param_fromPC")){			
			printf("P=%s\r\n",ibuffer);
			return HHDX_PARAM;
		}
		if(!strcmp(ibuffer,"Send_Start")){			
			printf("P=%s\r\n",ibuffer);
			rec_recive_cmd=HHDX_REC_START;
			return HHDX_REC_START;
		}	
		if(!strcmp(ibuffer,"Send_Stop")){
			printf("P=%s\r\n",ibuffer);
			rec_recive_cmd=HHDX_REC_STOP;
			return HHDX_REC_STOP;
		}
		if(!strcmp(ibuffer,"Send_Offhook")){
			printf("P=%s\r\n",ibuffer);
			rec_recive_cmd=HHDX_REC_OFFHOOK;
			return HHDX_REC_OFFHOOK;
		}

	}		
	return 0;
}

/*
��ȡÿ���ַ�
{"key":"value",......}
*/
int HHDX_DivideJson(int cmd_type,char *p_msg,int msg_len)
{
	int i;		
	int j=0;	//ÿ�ֶγ��ȼ����������ָ���','����
	char *value_addr;
	char ibuffer[50]; //ÿ�ַ�"key":"value"������(���ᳬ��50) 
	for(i=0;i<msg_len;i++)
	{
		if((*(p_msg+i))=='{'){	//json��ʼ
			j=0;
			ibuffer[0]='\0';
		}
		else if((*(p_msg+i))==','){	//json�ֶμ�ָ���
			value_addr=HHDX_GetFieldName(cmd_type,ibuffer);
			HHDX_GetFieldValue(gTempChar,value_addr);
			j=0;
			ibuffer[0]='\0';		//��һ���ַ�			
		}
		else if((*(p_msg+i))=='}'){	//json����
			value_addr=HHDX_GetFieldName(cmd_type,ibuffer);
			HHDX_GetFieldValue(gTempChar,value_addr);
			return i;
		}	
		else{
	    ibuffer[j]= *(p_msg+i); // ��ÿ���ַ����ưᵽibuffer[j]��
			j++;
			if(j==50)		//��ֹmsg_len>50,����ibuffer����
				j=0;
		}
	}
	return 0;			//��������
}

/*
��{"key":"value"}����ȡÿ���ַ���"key"
*/
char *HHDX_GetFieldName(int cmd_type,char *p_msg)
{
	int i;		
	int j=0;	//ÿ�ֶγ��ȼ����������ָ���','����
	char ibuffer[20]; //"key"�ַ��Ļ���������Ϊ20
	for(i=0;i<20;i++)
	{
		if((j==0)&&(*(p_msg+i)=='"')){	//�ַ�����ʼ
			ibuffer[0]='\0';
		}
		else if((j!=0)&&(*(p_msg+i)=='"')){	//�ַ�������
			ibuffer[j] ='\0';
//			printf("N=%s\r\n",ibuffer);
			gTempChar =HHDX_AnalyseJsonName(cmd_type,ibuffer);
			return 	p_msg+i+2;		//����value��ַ
		}
		else{
	    ibuffer[j]= *(p_msg+i); // ��ÿ���ַ����ưᵽibuffer[j]��
			j++;
		}
	}
	return 0;			//��������
}


//��������PC�������command type
char HHDX_AnalyseJsonName(int cmd_type,char *p_msg)
{
	char name=0;
	if(cmd_type ==HHDX_FUNC){
		name = (char)HHDX_FuncRx(cmd_type,p_msg);
	}
	else if(cmd_type ==HHDX_REC_START){
		name = (char)HHDX_FuncRx(cmd_type,p_msg);
	}
	else if(cmd_type ==HHDX_REC_STOP){
		name = (char)HHDX_FuncRx(cmd_type,p_msg);
	}
	else if(cmd_type ==HHDX_PARAM){
		name = (char)HHDX_ParamRx(p_msg);
		gsNetFlag.bind =0;
		gsHhdxFlag.eepromwr =1;		//1: ��Ҫдeeprom		0: д���
	}
	else{
		name =0;
	}
	return name;
}


/*
//��������PC�ĵ绰����ָ��:  TEL_func_fromPC={����}
{"tel_onoff":"x"}
{"dial_nums":"xxx����"}
{"dial_num":"x"}
*/
int HHDX_FuncRx(int cmd_type,char *p_msg_name)
{
	if(!strcmp(p_msg_name,"tel_onoff"))
		return JS_TEL_ONOFF;
    if(!strcmp(p_msg_name,"dial_nums"))
		return JS_DIAL_NUMS;
	if(!strcmp(p_msg_name,"dial_num"))
		return JS_DIAL_NUM;	
	if(cmd_type==HHDX_REC_START && !strcmp(p_msg_name,"port"))
		return JS_REC_START;	
	if(cmd_type==HHDX_REC_STOP && !strcmp(p_msg_name,"id"))
		return JS_REC_STOP;		
	return 0;
}
/* 
//����PC���ò�����JSON��ʽ����
{
	"local_ip":"192.168.0.22",
	"mask":"255.255.255.0",
	"gw_ip":"192.168.0.1",
	"sv_ip":"192.168.0.10",
	"local_num":"8006",
	"hf_spk_vol":"50",
	"hf_mic_vol":"50",
	"music_name":"1",
	"ans_time":"4",
	"ans_onoff":"0",
	"alarm_led_onoff":"1"							
}
*/
int HHDX_ParamRx(char *p_msg_name)
{
	if(!strcmp(p_msg_name,"local_ip")){
		return JS_LOCAL_IP;
	}
  if(!strcmp(p_msg_name,"mask")){
		return JS_MASK;
	}
  if(!strcmp(p_msg_name,"gw_ip"))
		return JS_GW_IP;
	if(!strcmp(p_msg_name,"sv_ip"))
		return JS_SV_IP;
	if(!strcmp(p_msg_name,"local_num"))
		return JS_LOCAL_NUM;
  if(!strcmp(p_msg_name,"hf_spk_vol"))
		return JS_HF_SKP_VOL;
  if(!strcmp(p_msg_name,"hf_mic_vol"))
		return JS_HF_MIC_VOL;	
  if(!strcmp(p_msg_name,"music_name"))
		return JS_MUSIC_NAME;
	if(!strcmp(p_msg_name,"ans_time"))
		return JS_ANS_TIME;
  if(!strcmp(p_msg_name,"ans_onoff"))
		return JS_ANS_ONOFF;		
  if(!strcmp(p_msg_name,"alarm_led_onoff"))
		return JS_ALARM_LED_ONOFF;	
		
	if(!strcmp(p_msg_name,"restore"))
		return JS_RESET;	
		
	if(!strcmp(p_msg_name,"param_read"))
		return JS_PARAM_READ;		
	return 0;
}
/*
��{"key":"value"}����ȡÿ���ַ���"value"
*/
char *HHDX_GetFieldValue(char name,char *p_msg)
{
	int i=0,j=0;
	char ibuffer[16]; //message header������	
	for(i=0;i<16;i++)
	{
		if((j==0)&&(*(p_msg+i)=='"')){	//�ַ�����ʼ
			ibuffer[0]='\0';
		}
		else if((j!=0)&&(*(p_msg+i)=='"')){	//�ַ�������
			ibuffer[j] ='\0';
			HHDX_AnalyseJsonValue(name,ibuffer);
			return p_msg+i+2;		//����value��ַ
		}
		else{
	    ibuffer[j]= *(p_msg+i); // ��ÿ���ַ����ưᵽibuffer[j]��
			j++;
		}
	}
	return 0;			//��������
}

/*
*/
void HHDX_AnalyseJsonValue(char name,char *p_msg)
{	
	u8 i;
	switch(name)
	{
		case 0:
			break;
//------------- function -------------
		case JS_TEL_ONOFF:	

			break;
		case JS_DIAL_NUMS:	
			break;			
	   	case JS_DIAL_NUM:
			break;
		case JS_REC_START:
			//rec_recive_flag=HHDX_REC_START;
			dynamic_port = atoi(p_msg);
			printf("dynamic_port=%d\r\n",dynamic_port);
			NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,dynamic_port,RTP_LOCAL_PORT);
			break;
		case JS_REC_STOP:
			//rec_recive_flag=HHDX_REC_STOP;
			if(gpsaUdppcb[1]){	
				udp_remove(gpsaUdppcb[1]);	//�Ƴ�UDP����
				gpsaUdppcb[1] =NULL;
			}	
			printf("rec stop\r\n");
			break;

//------------- parameter -------------
		case JS_LOCAL_IP:			
			Str2IpArray(gpsEeprom->own_ip,p_msg);
			LWIP_NetConfigUpdate();
			break;
		case JS_MASK:	
			Str2IpArray(gpsEeprom->mask_ip,p_msg);
			break;
		case JS_GW_IP:
			Str2IpArray(gpsEeprom->gw_ip,p_msg);
			break;
	   case JS_SV_IP:
	   	Str2IpArray(gpsEeprom->sv_ip,p_msg);
	   	gsNetFlag.bind =0;
			break;				
		case JS_LOCAL_NUM:
			if(strlen(p_msg)<16){			//���Ȳ�����15
				strcpy((char *)gpsEeprom->own_num,p_msg);
				LWIP_NetConfigUpdate();
			}
			break;
		case JS_HF_SKP_VOL:
			String2Int((unsigned int *)&gpsEeprom->spk_vol,p_msg);
			break; 		
	 	case JS_HF_MIC_VOL:
	 		String2Int((unsigned int *)&gpsEeprom->mic_vol,p_msg);
			break;
	 	case JS_MUSIC_NAME:
	 		String2Int((unsigned int *)&gpsEeprom->music_name,p_msg);
			break;
		case JS_ANS_TIME:	
			String2Int((unsigned int *)&gpsEeprom->ans_time,p_msg);	
			break;
		case JS_ANS_ONOFF:
			gpsEeprom->flag.ans_onoff= (*p_msg)& 0x0f;
			break;
		case JS_ALARM_LED_ONOFF:
			gpsEeprom->flag.alarm_led_onoff = (*p_msg)& 0x0f;
			break;
		case JS_RESET:				
			HHDX_VariateInit();
			HHDX_FactoryReset();
			gsHhdxFlag.eepromwr =1;		
			
			gsNetFlag.bind =0;
			//lwip_Set_ip();
			LWIP_NetConfigUpdate();
		//	break;	
		case JS_PARAM_READ:
			HHDX_ParamTxBuild();			
			printf("Len =%d\r\n",gEeprom_len);
			for(i=0; i<gEeprom_len; i++){
				printf("%d,",(*(((u8*)gpsEeprom+i))));
			}
			printf("\r\n");
			break;	
		default:
			break;
	}	
}





/*
;1.����IP	= 192.168.0.30
;2.��������	= 8002
;3.������IP	= 192.168.0.10
;4.����		= 192.168.0.1
;5.��������	= 255.255.255.0
;6.����������	= 9999
*/
void HHDX_FactoryReset(void)
{
//1.����IP	= 192.168.0.30	
	gpsEeprom->own_ip[0] =HOST_IP0;
	gpsEeprom->own_ip[1] =HOST_IP1;
	gpsEeprom->own_ip[2] =HOST_IP2;
	gpsEeprom->own_ip[3] =HOST_IP3;
	
//2.��������	= 8002
	gpsEeprom->own_num[0] ='8';
	gpsEeprom->own_num[1] ='0';
	gpsEeprom->own_num[2] ='0';
	gpsEeprom->own_num[3] ='2';
	gpsEeprom->own_num[4] =0;

//3.SIP������IP	= 192.168.0.10	
	gpsEeprom->sv_ip[0] =SV_IP0;
	gpsEeprom->sv_ip[1] =SV_IP1;
	gpsEeprom->sv_ip[2] =SV_IP2;
	gpsEeprom->sv_ip[3] =SV_IP3;

//4.����		= 192.168.0.1	
	gpsEeprom->gw_ip[0] =GW_IP0;
	gpsEeprom->gw_ip[1] =GW_IP1;
	gpsEeprom->gw_ip[2] =GW_IP2;
	gpsEeprom->gw_ip[3] =GW_IP3;

//5.��������	= 255.255.0.0
	gpsEeprom->mask_ip[0] =NET_MASK0;
	gpsEeprom->mask_ip[1] =NET_MASK1;
	gpsEeprom->mask_ip[2] =NET_MASK2;
	gpsEeprom->mask_ip[3] =NET_MASK3;

//6.Speaker volume level = 0~63
	gpsEeprom->spk_vol = 50;

//7.MIC volume level = 0~63
	gpsEeprom->mic_vol = 50;
	
//8.music name =1;	
	gpsEeprom->music_name =1;

//9.Automatic response time=0~9(max=9*3=27s)	
	gpsEeprom->ans_time =3;
	
//10.16λ��־λ	
	gpsEeprom->flag.ans_onoff =0;
	gpsEeprom->flag.alarm_led_onoff =0;
//11.
    gpsEeprom->own_id[0]='1';
	gpsEeprom->own_id[1]='1';
	gpsEeprom->own_id[2]='2';
	gpsEeprom->own_id[3]='2';
	gpsEeprom->own_id[0]=0;
//12.
   gpsEeprom->nophone_hv=168;
   gpsEeprom->ring_lv=280;
   gpsEeprom->onhook_lv=236;
   gpsEeprom->onhook_hv=264;
   gpsEeprom->offhook_lv=170;
   gpsEeprom->offhook_hv=190;

   gpsEeprom->port=RTP_LOCAL_PORT;
}	


/*
//��PC���Ͳ�����ѯ�ظ�
	TEL_param_toPC={
	"local_ip":"192.168.0.12",
	"mask":"255.255.255.0",
	"gw_ip":"192.168.0.1",
	"sv_ip":"192.168.0.10",
	"local_num":"8006",
	"hf_spk_vol":"50",
	"hf_mic_vol":"50",
	"music_name":"1", 
	"ans_time":"4",
	"ans_onoff":"0",
	"alarm_led_onoff":"1",
	"ver":"V2023061301"
}
*/
void HHDX_ParamTxBuild(void)
{
	int j;
	char *SendMsg;		

//1. aoi_beat	
	SendMsg =(char *)(gpNetTxBuf + gpsNetTx->wr*NET_BUF_SIZE);
	
	j=sprintf(SendMsg,"TEL_func_toPC={\"local_ip\":\"%d.%d.%d.%d\",",	\
		gpsEeprom->own_ip[0],gpsEeprom->own_ip[1],gpsEeprom->own_ip[2],gpsEeprom->own_ip[3]);	
	j+=sprintf(SendMsg+j,"\"mask\":\"%d.%d.%d.%d\",",	\
		gpsEeprom->mask_ip[0],gpsEeprom->mask_ip[1],gpsEeprom->mask_ip[2],gpsEeprom->mask_ip[3]);
	j+=sprintf(SendMsg+j,"\"gw_ip\":\"%d.%d.%d.%d\",",	\
		gpsEeprom->gw_ip[0],gpsEeprom->gw_ip[1],gpsEeprom->gw_ip[2],gpsEeprom->gw_ip[3]);
	j+=sprintf(SendMsg+j,"\"sv_ip\":\"%d.%d.%d.%d\",",	\
		gpsEeprom->sv_ip[0],gpsEeprom->sv_ip[1],gpsEeprom->sv_ip[2],gpsEeprom->sv_ip[3]);		
	j+=sprintf(SendMsg+j,"\"local_num\":\"%s\",",gpsEeprom->own_num);
	j+=sprintf(SendMsg+j,"\"hf_spk_vol\":\"%d\",",gpsEeprom->spk_vol);
	j+=sprintf(SendMsg+j,"\"hf_mic_vol\":\"%d\",",gpsEeprom->mic_vol);
	j+=sprintf(SendMsg+j,"\"music_name\":\"%d\",",gpsEeprom->music_name);
	j+=sprintf(SendMsg+j,"\"ans_time\":\"%d\",",gpsEeprom->ans_time);
	j+=sprintf(SendMsg+j,"\"ans_onoff\":\"%d\",",gpsEeprom->flag.ans_onoff);
	j+=sprintf(SendMsg+j,"\"alarm_led_onoff\":\"%d\",",gpsEeprom->flag.alarm_led_onoff);
	
	j+=sprintf(SendMsg+j,"\"ver\":\"%s\",","V2023070201");
	
	
	NET_TxPcbLenWr(MSW_TX,j);
}


void HHDX_VariateInit(void)
{
	gsHhdxFlag.eepromwr =0;	//1: ��ҪдEEPROM
}

//	
//	LCALL	Flash128Erase
//	LCALL	Flash128Write
//	LJMP	SysSetReset


