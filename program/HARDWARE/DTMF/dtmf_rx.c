#include "usart.h"
#include "usart3_FSK.h"
#include "tel_func.h"
#include "HhdxMsg.h"
#include "SipFunction.h"
#include "cw2015.h"
#include "dtmf_rx.h"
#include "timer.h"

uint8_t gDtmfRxFlag0 =0;
uint8_t gDtmfRxFlag1 =0;
uint8_t gDtmfRxStep =0;
uint8_t gCidFlag =0;


//按键初始化函数
void DTMF_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE); //使能GPIOE的时钟	

//PE11,12,13,14 = D0~3 input
//PE15 DV

	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //输入
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //上拉输入
//	GPIO_Init(GPIOE,&GPIO_InitStructure);

//PE10 = OE out
	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;			//输出
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  	//推挽输出
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //上拉
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //高速GPIO
	GPIO_Init(GPIOE,&GPIO_InitStructure);
}

/*
提取来电中的时间
*/
void DTMF_CID(void)
{	

}


/*
 1. DV = 1: 号码有效
 2. OE一直保持高有效
*/
void DTMF_rx(void)
{
//	static u8 i=0;
	u8 dtmf_val;
	if(DTMF_DV ==1){
		gpsT3->dtmf_low =0;
		if(gpsT3->dtmf_hi >5){
			if(gDtmfRxFlag0 ==0){
				gDtmfRxFlag0 =1;
				gDtmfRxFlag1 =0;
				dtmf_val =DTMF_D0_3();
				printf("DTMF=%d\r\n",dtmf_val);
				BUILD_DIAL_DTMF(dtmf_val);
//				if(gDtmfRxParseFlag){		//只有收到振铃后才可以设置(即只当被叫)
//					DTMF_rx_parse(dtmf_val);
//				}
//				if(gRingingFlag&&(dtmf_val ==11)){	//振铃下收到*
//						gCidFlag =1;		//振铃下收到*
//	//						PC_printf("*\r\n");
//				}
//				LCD_DtmfDisp(dtmf_val);
	//			PC_printf("Hi\r\n");
			}
		}
	}
	else{
		gpsT3->dtmf_hi =0;
		if(gpsT3->dtmf_low >5){	//20ms			
			if(gDtmfRxFlag1 ==0){
				gDtmfRxFlag1 =1;
				gDtmfRxFlag0 =0;
//				printf("%d",i);			
//				i++;
//				if(i ==9)
//					i =0;
			}
		}
	}
}

/*
--------DTMF系统参数设置
DTMF值: 1~9,10(0),11(*),12(#)
1. *01~10: 设置10个一键拨号号码，最长15位
	 *01~10xxxxxxxxxxxxxxx#
2. *51x#: 设置几声铃自动摘机 (x = 0~9)
		0: 表示关闭自动摘机
3. *52xx#：选择15种音乐 (xx = 01~15)
4. *53x#：设置音乐音量大小(x = 0~7)
5. *54x#：照明灯是否开启，0关闭，1开启(x = 0，1)
6. *55xxx#：话机心跳时间间隔 (xxx = 000~999秒)
7. *5600#：恢复出厂设置
8. *57xxxx#：时间设置(xxxx = xx:xx)
9. *58xx#：自动挂机时长 (xx = 00~99分)
10. *59xx#：选择15种急呼音乐 (xx =00：表示没有急呼功能，两声自动摘机	 	01~15：表示急呼 )
11. *50xx#：设置热线时长 (xx = 00~99秒)

*/

void DTMF_rx_parse(u8 data)
{
	
}


