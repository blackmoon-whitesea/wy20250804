#include "delay.h"
#include "usart.h"
#include "malloc.h"
#include "net_rxtx.h"
#include "stdio.h"
#include "string.h" 
#include "timer.h"
#include "led.h"
#include "key.h"
#include "SipFunction.h"
#include "tel_func.h"
#include "Audio.h" 
#include "i2s.h"
#include "wm8978.h"
#include "HhdxMsg.h"
#include "rtp.h"

struct stTelFlag gsTelFlag;
u8 gDialNumLen =0;


//����ժ�һ�����
void TEL_OnOffFunc(void)
{
	
//----------- �ֱ������������ĸ����
	if(gHfFlag||gHandsetFlag){
	//----------- �ӹһ��л���ժ������ִֻ��һ��
		if(gsTelFlag.tel_on ==0){	//ֻһ�����				
			gsTelFlag.tel_on =1;
			AUDIO_play_mode();	//�Ų�������RTP
			if(gsSipRxFlag.invite ==1){	//�յ�invite���
				gSipTxFlow = TX_INVITE_200_OK;	//�����������
			}
			else{	//�����������	
				if(gDialNumLen){		//�к����ֱ�Ӻ���(Ԥ����)					
						gSipTxFlow = TX_INVITE;	
				}
			}
		}
	//----------- ����ժ��״̬��
	
	}

//----------- �ֱ��������������һ�
	else{
		if(gsTelFlag.tel_on ==1){	
			gsTelFlag.tel_on =0;
			if((gsSipRxFlag.ok_inv==1)||(gsSipRxFlag.ack ==1)){	//�����յ�invite 200 Ok�򱻽��յ�ack
				gSipTxFlow = TX_BYE;			//�����з�bye�һ�
			}
			else{
				gSipTxFlow = TX_CANCEL;		//�����з�cancel�һ�
			}
			TEL_OffInit();
		}
	}
}

/*
 ��ʼ������tel function flag
*/
void TEL_OffInit(void)
{
//sip
	ANALYSIS_RxParamInit();
	
//rtp	
	gsAudio.rtp_rx_num =0;
	gRtpRxNum =0;
	gsAudio.rtp_tx_num =0;
	gRtpTxNum =0;

//tel onoff	
	gHfFlag =0;
	gHandsetFlag =0;	

//tone	
	TEL_HandfreeSPK_off();	
	AUDIO_stop_mode();	

//Called Number
	gDialNumLen=0;
	gpsUaInfo->called_number[gDialNumLen]='\0';
}

void TEL_PinInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
//PE4(xpt8871_SD): �������Ṧ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE); //��1?��GPIOD��?����?��	
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//��?3?
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //��?������?3?
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //��?��-
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //???��GPIO
	GPIO_Init(GPIOE,&GPIO_InitStructure);	
	
	//PG2(mic_in): �л�����/�ֱ�MIC
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE); //��1?��GPIOD��?����?��	
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//��?3?
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //��?������?3?
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //��?��-
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //???��GPIO
	GPIO_Init(GPIOG,&GPIO_InitStructure);	

	//PG0(LINK_LED): ���NET����û����
	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //��������
	GPIO_Init(GPIOG,&GPIO_InitStructure);
}	

//����������
void TEL_HandfreeSPK_on(void)	
{
	GPIO_ResetBits(GPIOE,GPIO_Pin_4); //0.��
}
//����������
void TEL_HandfreeSPK_off(void)
{
	GPIO_SetBits(GPIOE,GPIO_Pin_4); //1.��
}
