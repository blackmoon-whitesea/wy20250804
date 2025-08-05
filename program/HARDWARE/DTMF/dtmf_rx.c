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


//������ʼ������
void DTMF_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE); //ʹ��GPIOE��ʱ��	

//PE11,12,13,14 = D0~3 input
//PE15 DV

	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;  //����
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;  //��������
//	GPIO_Init(GPIOE,&GPIO_InitStructure);

//PE10 = OE out
	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;			//���
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  	//�������
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //����
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //����GPIO
	GPIO_Init(GPIOE,&GPIO_InitStructure);
}

/*
��ȡ�����е�ʱ��
*/
void DTMF_CID(void)
{	

}


/*
 1. DV = 1: ������Ч
 2. OEһֱ���ָ���Ч
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
//				if(gDtmfRxParseFlag){		//ֻ���յ������ſ�������(��ֻ������)
//					DTMF_rx_parse(dtmf_val);
//				}
//				if(gRingingFlag&&(dtmf_val ==11)){	//�������յ�*
//						gCidFlag =1;		//�������յ�*
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
--------DTMFϵͳ��������
DTMFֵ: 1~9,10(0),11(*),12(#)
1. *01~10: ����10��һ�����ź��룬�15λ
	 *01~10xxxxxxxxxxxxxxx#
2. *51x#: ���ü������Զ�ժ�� (x = 0~9)
		0: ��ʾ�ر��Զ�ժ��
3. *52xx#��ѡ��15������ (xx = 01~15)
4. *53x#����������������С(x = 0~7)
5. *54x#���������Ƿ�����0�رգ�1����(x = 0��1)
6. *55xxx#����������ʱ���� (xxx = 000~999��)
7. *5600#���ָ���������
8. *57xxxx#��ʱ������(xxxx = xx:xx)
9. *58xx#���Զ��һ�ʱ�� (xx = 00~99��)
10. *59xx#��ѡ��15�ּ������� (xx =00����ʾû�м������ܣ������Զ�ժ��	 	01~15����ʾ���� )
11. *50xx#����������ʱ�� (xx = 00~99��)

*/

void DTMF_rx_parse(u8 data)
{
	
}


