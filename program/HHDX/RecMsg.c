#include <stdio.h>
#include "usart.h"
#include "malloc.h"
#include "net_rxtx.h"
#include "stdio.h"
#include "string.h" 
#include "timer.h"
#include "HhdxMsg.h"
#include "24cxx.h" 
#include "RecMsgBuilding.h"


//struct stHhdxFlag gsHhdxFlag;

extern int rec_recive_cmd;

char gRecTempChar;
/*
1. Send_Start={......}
2. Send_Stop={......}
3. Send_Offhook={......}
*/
//��������������
int HHDX_RecCommand(char *p_line)
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

	if(i!=18)
	{  
		printf("P=%s\r\n",ibuffer);
		printf("waiting_response=%d\r\n", gpsRecInfo->waiting_response);

		if(gpsRecInfo->waiting_response==0){
			return 0;		
		}

		if(!strcmp(ibuffer,"Send_Start")){			
			//printf("P=%s\r\n",ibuffer);
			gpsRecInfo->waiting_response=0;
            gpsRecInfo->retry_count = 0;
			gsRecTxFlag.retry = 0;
            rec_recive_cmd=HHDX_REC_START;
			return HHDX_REC_START;
		}	
		if(!strcmp(ibuffer,"Send_Stop")){
			//printf("P=%s\r\n",ibuffer);
            gpsRecInfo->waiting_response=0;
            gpsRecInfo->retry_count = 0;
			gsRecTxFlag.retry = 0;
			rec_recive_cmd=HHDX_REC_STOP;
			return HHDX_REC_STOP;
		}
		if(!strcmp(ibuffer,"Send_Offhook")){
			//printf("P=%s\r\n",ibuffer);
            gpsRecInfo->waiting_response=0;
            gpsRecInfo->retry_count = 0;
			gsRecTxFlag.retry = 0;
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
int HHDX_RecDivideJson(int cmd_type,char *p_msg,int msg_len)
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
		else if((*(p_msg+i))==','){	//json�ֶμ�ָ���?
			value_addr=HHDX_RecGetFieldName(cmd_type,ibuffer);
			HHDX_RecGetFieldValue(gTempChar,value_addr);
			j=0;
			ibuffer[0]='\0';		//��һ���ַ�			
		}
		else if((*(p_msg+i))=='}'){	//json����
			value_addr=HHDX_RecGetFieldName(cmd_type,ibuffer);
			HHDX_RecGetFieldValue(gTempChar,value_addr);
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

char *HHDX_RecGetFieldName(int cmd_type,char *p_msg)
{
	int i;		
	int j=0;	//???��?????????????????','????
	char ibuffer[20]; //"key"???????????????20
	for(i=0;i<20;i++)
	{
		if((j==0)&&(*(p_msg+i)=='"')){	//????????
			ibuffer[0]='\0';
		}
		else if((j!=0)&&(*(p_msg+i)=='"')){	//?????????
			ibuffer[j] ='\0';
//			printf("N=%s\r\n",ibuffer);
			gRecTempChar =HHDX_RecAnalyseJsonName(cmd_type,ibuffer);
			return 	p_msg+i+2;		//????value???
		}
		else{
	    ibuffer[j]= *(p_msg+i); // ?????????????ibuffer[j]??
			j++;
		}
	}
	return 0;			//????????
}


//????????PC???????command type
char HHDX_RecAnalyseJsonName(int cmd_type,char *p_msg)
{
	char name=0;
	if(cmd_type ==HHDX_REC_START){
		name = (char)HHDX_RecFuncRx(cmd_type,p_msg);
	}
	else if(cmd_type ==HHDX_REC_STOP){
		name = (char)HHDX_RecFuncRx(cmd_type,p_msg);
	}
    else if(cmd_type ==HHDX_REC_OFFHOOK){
		name = (char)HHDX_RecFuncRx(cmd_type,p_msg);
	}
	else{
		name =0;
	}
	return name;
}


/*
//????????PC??��???????:  TEL_func_fromPC={????}
{"tel_onoff":"x"}
{"dial_nums":"xxx????"}
{"dial_num":"x"}
*/
int HHDX_RecFuncRx(int cmd_type,char *p_msg_name)
{
	if(cmd_type==HHDX_REC_START && !strcmp(p_msg_name,"port"))
		return JS_REC_START;	
	if(cmd_type==HHDX_REC_STOP && !strcmp(p_msg_name,"id"))
		return JS_REC_STOP;	
    if(cmd_type==HHDX_REC_OFFHOOK && !strcmp(p_msg_name,"id"))
		return JS_REC_OFFHOOK;
	return 0;
}

/*
??{"key":"value"}?????????????"value"
*/
char *HHDX_RecGetFieldValue(char name,char *p_msg)
{
	int i=0,j=0;
	char ibuffer[16]; //message header??????	
	for(i=0;i<16;i++)
	{
		if((j==0)&&(*(p_msg+i)=='"')){	//????????
			ibuffer[0]='\0';
		}
		else if((j!=0)&&(*(p_msg+i)=='"')){	//?????????
			ibuffer[j] ='\0';
			HHDX_AnalyseJsonValue(name,ibuffer);
			return p_msg+i+2;		//????value???
		}
		else{
	    ibuffer[j]= *(p_msg+i); // ?????????????ibuffer[j]??
			j++;
		}
	}
	return 0;			//????????
}

/*
*/
void HHDX_RecAnalyseJsonValue(char name,char *p_msg)
{	
	u8 i;
	switch(name)
	{
		case 0:
			break;
		case JS_REC_START:
			//rec_recive_flag=HHDX_REC_START;
			gpsRecInfo->dynamic_port = atoi(p_msg);
			printf("dynamic_port=%d\r\n",gpsRecInfo->dynamic_port);

	        if(gpsaUdppcb[1]){	
				udp_remove(gpsaUdppcb[1]);	//???UDP????
				gpsaUdppcb[1] =NULL;
			}	
			printf("rec disconnect\r\n");

			NET_UdppcbConnect(gpsaUdppcb[1],gpsEeprom->sv_ip,gpsRecInfo->dynamic_port,RTP_LOCAL_PORT);
            printf("rec connected\r\n");
            break;
		case JS_REC_STOP:
			//rec_recive_flag=HHDX_REC_STOP;
			if(gpsaUdppcb[1]){	
				udp_remove(gpsaUdppcb[1]);	//???UDP????
				gpsaUdppcb[1] =NULL;
			}	
			printf("rec disconnect\r\n");
			    
        case JS_REC_OFFHOOK:

            break;
		default:
			break;
	}	
}


