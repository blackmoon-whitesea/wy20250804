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


//话机摘挂机功能
void TEL_OnOffFunc(void)
{
	
//----------- 手柄、免提任意哪个提机
	if(gHfFlag||gHandsetFlag){
	//----------- 从挂机切换到摘机，且只执行一次
		if(gsTelFlag.tel_on ==0){	//只一次提机				
			gsTelFlag.tel_on =1;
			AUDIO_play_mode();	//放拨号音或RTP
			if(gsSipRxFlag.invite ==1){	//收到invite提机
				gSipTxFlow = TX_INVITE_200_OK;	//被叫振铃提机
			}
			else{	//主叫提机呼叫	
				if(gDialNumLen){		//有号码才直接呼叫(预拨号)					
						gSipTxFlow = TX_INVITE;	
				}
			}
		}
	//----------- 处在摘机状态下
	
	}

//----------- 手柄、免提两个都挂机
	else{
		if(gsTelFlag.tel_on ==1){	
			gsTelFlag.tel_on =0;
			if((gsSipRxFlag.ok_inv==1)||(gsSipRxFlag.ack ==1)){	//主叫收到invite 200 Ok或被叫收到ack
				gSipTxFlow = TX_BYE;			//主被叫发bye挂机
			}
			else{
				gSipTxFlow = TX_CANCEL;		//主被叫发cancel挂机
			}
			TEL_OffInit();
		}
	}
}

/*
 初始化所有tel function flag
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
//PE4(xpt8871_SD): 开关免提功放
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE); //ê1?üGPIODμ?ê±?ó	
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//ê?3?
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //í?íìê?3?
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //é?à-
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //???ùGPIO
	GPIO_Init(GPIOE,&GPIO_InitStructure);	
	
	//PG2(mic_in): 切换免提/手柄MIC
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE); //ê1?üGPIODμ?ê±?ó	
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//ê?3?
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //í?íìê?3?
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //é?à-
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //???ùGPIO
	GPIO_Init(GPIOG,&GPIO_InitStructure);	

	//PG0(LINK_LED): 检测NET线有没插入
	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //输入
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //上拉输入
	GPIO_Init(GPIOG,&GPIO_InitStructure);
}	

//开免提喇叭
void TEL_HandfreeSPK_on(void)	
{
	GPIO_ResetBits(GPIOE,GPIO_Pin_4); //0.开
}
//关免提喇叭
void TEL_HandfreeSPK_off(void)
{
	GPIO_SetBits(GPIOE,GPIO_Pin_4); //1.关
}
